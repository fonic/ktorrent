/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <QColor>
#include <QMimeData>
#include <klocale.h>
#include <util/log.h>
#include <util/functions.h>
#include <torrent/queuemanager.h>
#include <interfaces/torrentinterface.h>
#include "queuemanagermodel.h"
#include "settings.h"

using namespace bt;

namespace kt
{


	QueueManagerModel::QueueManagerModel(QueueManager* qman,QObject* parent)
			: QAbstractTableModel(parent),qman(qman)
	{
		connect(qman,SIGNAL(queueOrdered()),this,SLOT(onQueueOrdered()));
	}


	QueueManagerModel::~QueueManagerModel()
	{}
	
	void QueueManagerModel::onQueueOrdered()
	{
		reset();
	}
	
	void QueueManagerModel::onTorrentAdded(bt::TorrentInterface* tc)
	{
		insertRow(qman->count() - 1);
		stalled_times.insert(tc,0);
		reset();
	}
	
	void QueueManagerModel::onTorrentRemoved(bt::TorrentInterface* tc)
	{
		int r = 0;
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
		{
			if (tc == *i)
			{
				stalled_times.remove(tc);
				removeRow(r);
				break;
			}
			r++;
		}
	}

	int QueueManagerModel::rowCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return qman->count();
	}
	
	int QueueManagerModel::columnCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return 3;
	}
	
	QVariant QueueManagerModel::headerData(int section, Qt::Orientation orientation,int role) const
	{
		if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
			return QVariant();
		 
		switch (section)
		{
			case 0: return i18n("Name");
			case 1: return i18n("Status");
			case 2: return i18n("Time Stalled");
			case 3: return i18n("Priority");
			default: return QVariant();
		}
	}
	
	QVariant QueueManagerModel::data(const QModelIndex & index, int role) const
	{
		if (!index.isValid() || index.row() >= qman->count() || index.row() < 0)
			return QVariant(); 
		
		if (role == Qt::ForegroundRole)
		{
			const bt::TorrentInterface* tc = qman->getTorrent(index.row());
			if (index.column() == 1)
			{
				if (tc->getPriority() == 0)
					return QVariant();
				else if (tc->getStats().running)
					return QColor(40,205,40); // green
				else
					return QColor(255,174,0); // yellow
			}
			return QVariant();
		}
		else if (role == Qt::DisplayRole)
		{
			const bt::TorrentInterface* tc = qman->getTorrent(index.row());
			switch (index.column())
			{
				case 0: return tc->getStats().torrent_name;
				case 1: 
					if (tc->getPriority() == 0)
						return i18n("Not queued");
					else if (tc->getStats().running)
						return i18n("Running");
					else
						return i18n("Queued");
					break;
				case 2: 
					{
						if (!tc->getStats().running)
							return QVariant();
						
						
						Int64 stalled_time = stalled_times.value(tc);
						if (stalled_time >= 1)
							return i18n("%1",DurationToString(stalled_time));
						else
							return QVariant();
					}
					break;
				case 3:
					return tc->getPriority();
				default: return QVariant();
			}
		}
		
		return QVariant();
	}
	
	Qt::ItemFlags QueueManagerModel::flags(const QModelIndex &index) const
	{
		Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

		if (index.isValid())
			return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
		else
			return Qt::ItemIsDropEnabled | defaultFlags;
	}
	
	Qt::DropActions QueueManagerModel::supportedDropActions() const
	{
		return Qt::CopyAction | Qt::MoveAction;
	}
	
	QStringList QueueManagerModel::mimeTypes() const
	{
		QStringList types;
		types << "application/vnd.text.list";
		return types;
	}
					
	QMimeData* QueueManagerModel::mimeData(const QModelIndexList &indexes) const
	{
		QMimeData *mimeData = new QMimeData();
		QByteArray encodedData;

		dragged_items.clear();
	
		foreach (const QModelIndex &index, indexes) 
		{
			if (index.isValid() && !dragged_items.contains(index.row())) 
				dragged_items.append(index.row());
		}
		
		mimeData->setData("application/vnd.text.list", "stuff");
		return mimeData;
	}

	bool QueueManagerModel::dropMimeData(const QMimeData *data,Qt::DropAction action, int row, int column, const QModelIndex &parent)		
	{
		if (action == Qt::IgnoreAction)
			return true;

		if (!data->hasFormat("application/vnd.text.list"))
			return false;
		
		int begin_row;
		if (row != -1)
			begin_row = row;
		else if (parent.isValid())
			begin_row = parent.row();
		else
			begin_row = rowCount(QModelIndex());
		
		
		// use a copy of the list of torrents to redo the priorities
		QueuePtrList torrents;
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
			torrents.append(*i);
		
		// put the dragged ones in a new list
		QList<bt::TorrentInterface*> tcs;
		foreach (int r,dragged_items)
		{
			bt::TorrentInterface* tc = qman->getTorrent(r);
			if (tc)
			{
				tcs.append(tc);
				if (r < begin_row) // begin row will decrease when we remove this one in the next loop
					begin_row--;
				
				if (tc->getPriority() == 0) // once you drag items they become QM controlled
					tc->setPriority(1);
			}
		}
		
		// remove the dragged ones
		foreach (bt::TorrentInterface* tc,tcs)
			torrents.removeAll(tc);
		
		// reinsert them at the correct location
		foreach (bt::TorrentInterface* tc,tcs)
			torrents.insert(begin_row,tc);
		
		int prio = torrents.count();
		// redo the priorities
		foreach (bt::TorrentInterface* t,torrents)
		{
			if (t->getPriority() > 0)
				t->setPriority(prio);
			prio--;
		}
		
		qman->orderQueue();
		return true;
	}

	bool QueueManagerModel::removeRows(int row,int count,const QModelIndex & parent)
	{
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	bool QueueManagerModel::insertRows(int row,int count,const QModelIndex & parent)
	{
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	void QueueManagerModel::moveUp(int row)
	{
		if (row <= 0 || row > qman->count())
			return;
		
		int r = 0;
		int prio = qman->count();
		bt::TorrentInterface* prev = 0;
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
		{
			bt::TorrentInterface* curr = *i;
			if (r == row)
			{
				if (!prev || prev->getPriority() == 0)
				{
					curr->setPriority(prio);
				}
				else
				{
					// switch prio with previous one
					curr->setPriority(prio + 1);
					prev->setPriority(prio);
				}
			}
			else if (curr->getPriority() > 0)
			{
				curr->setPriority(prio);
			}
			prio--;
			prev = *i;
			r++;
		}
		
		// reorder the queue
		qman->orderQueue();
	}
	
	void QueueManagerModel::moveDown(int row)
	{
		if (row < 0 || row >= qman->count() - 1)
			return;
		
		int r = 0;
		int prio = qman->count();
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
		{
			bt::TorrentInterface* curr = *i;
			bt::TorrentInterface* next = 0;
			QueueManager::iterator j = i;
			j++;
			if (j != qman->end())
				next = *j;

			if (r == row)
			{
				if (!next || next->getPriority() == 0)
				{
					curr->setPriority(0);
				}
				else
				{
					// switch prio with next one
					curr->setPriority(prio - 1);
					next->setPriority(prio);
				}
			}
			else if (curr->getPriority() > 0)
			{
				curr->setPriority(prio);
			}
			prio--;
			r++;
		}
		// reorder the queue
		qman->orderQueue();
	}
	
	void QueueManagerModel::queue(int row)
	{
		if (row < 0 || row >= qman->count())
			return;
		
		bt::TorrentInterface* tc = qman->getTorrent(row);
		qman->queue(tc);
		qman->orderQueue();
	}
	
	void QueueManagerModel::update()
	{
		TimeStamp now = bt::GetCurrentTime();
		int r = 0;
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
		{
			bt::TorrentInterface* tc = *i;
			if (!tc->getStats().running)
			{
				if (stalled_times[tc] != -1)
				{
					stalled_times[tc] = -1;
					emit dataChanged(createIndex(r,2),createIndex(r,2));
				}
			}
			else
			{
				Int64 stalled_time = 0;
				if (tc->getStats().completed)
					stalled_time = (now - tc->getStats().last_upload_activity_time) / 1000;
				else
					stalled_time = (now - tc->getStats().last_download_activity_time) / 1000;
				
				if (stalled_times[tc] != stalled_time)
				{
					stalled_times[tc] = stalled_time;
					emit dataChanged(createIndex(r,2),createIndex(r,2));
				}
			}
			r++;
		}
	}
}

#include "queuemanagermodel.moc"