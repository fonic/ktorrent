/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <klocale.h>
#include <kstandardguiitem.h>
#include <util/constants.h>
#include <util/log.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>
#include <settings.h>
#include "core.h"
#include "speedlimitsmodel.h"
#include "speedlimitsdlg.h"
#include "spinboxdelegate.h"
#include <torrent/queuemanager.h>

		
using namespace bt;

namespace kt
{

	
	
	
	SpeedLimitsDlg::SpeedLimitsDlg(bt::TorrentInterface* current,Core* core,QWidget* parent)
			: QDialog(parent),core(core),current(current)
	{
		setupUi(this);
		setWindowIcon(KIcon("kt-speed-limits"));
		
		model = new SpeedLimitsModel(core,this);
		QSortFilterProxyModel* pm = new QSortFilterProxyModel(this);
		pm->setSourceModel(model);
		pm->setSortRole(Qt::UserRole);
		
		m_speed_limits_view->setModel(pm);
		m_speed_limits_view->setItemDelegate(new SpinBoxDelegate(this));
		m_speed_limits_view->setUniformRowHeights(true);
		m_speed_limits_view->setSortingEnabled(true);
		m_speed_limits_view->sortByColumn(0,Qt::AscendingOrder);
		m_speed_limits_view->header()->setSortIndicatorShown(true);
		m_speed_limits_view->header()->setClickable(true);
		m_speed_limits_view->setAlternatingRowColors(true);
		
		
		m_ok->setGuiItem(KStandardGuiItem::ok());
		m_apply->setGuiItem(KStandardGuiItem::apply());
		m_cancel->setGuiItem(KStandardGuiItem::cancel());
		
		connect(m_ok,SIGNAL(clicked()),this,SLOT(accept()));
		connect(m_apply,SIGNAL(clicked()),this,SLOT(apply()));
		connect(m_cancel,SIGNAL(clicked()),this,SLOT(reject()));
		
		m_apply->setEnabled(false);
		connect(model,SIGNAL(enableApply(bool)),m_apply,SLOT(setEnabled(bool)));
		
		m_upload_rate->setValue(Settings::maxUploadRate());
		m_download_rate->setValue(Settings::maxDownloadRate());
		connect(m_upload_rate,SIGNAL(valueChanged(int)),this,SLOT(spinBoxValueChanged(int)));
		connect(m_download_rate,SIGNAL(valueChanged(int)),this,SLOT(spinBoxValueChanged(int)));
		
		loadState();
		
		// if current is specified, select it and scroll to it
		if (current)
		{
			kt::QueueManager* qman = core->getQueueManager();
			int idx = 0;
			QList<bt::TorrentInterface*>::iterator itr = qman->begin();
			while (itr != qman->end())
			{
				if (*itr == current)
					break;
				
				idx++;
				itr++;
			}
			
			if (itr != qman->end())
			{
				QItemSelectionModel* sel = m_speed_limits_view->selectionModel();
				QModelIndex midx = pm->mapFromSource(model->index(idx,0));
				QModelIndex midx2 = pm->mapFromSource(model->index(idx,4));
				sel->select(QItemSelection(midx,midx2),QItemSelectionModel::Select);
				m_speed_limits_view->scrollTo(midx);
			}
		}
	}

	SpeedLimitsDlg::~SpeedLimitsDlg()
	{}
	
	void SpeedLimitsDlg::saveState()
	{
		KConfigGroup g = KGlobal::config()->group("SpeedLimitsDlg");
		QByteArray s = m_speed_limits_view->header()->saveState();
		g.writeEntry("view_state",s.toBase64());
		g.writeEntry("size",size());
	}
	
	void SpeedLimitsDlg::loadState()
	{
		KConfigGroup g = KGlobal::config()->group("SpeedLimitsDlg");
		QByteArray s = QByteArray::fromBase64(g.readEntry("view_state",QByteArray()));
		if (!s.isNull())
		{
			m_speed_limits_view->header()->restoreState(s);
			m_speed_limits_view->header()->setSortIndicatorShown(true);
			m_speed_limits_view->header()->setClickable(true);
		}
		
		QSize ws = g.readEntry("size",size());
		resize(ws);
	}

	void SpeedLimitsDlg::accept()
	{
		apply();
		saveState();
		QDialog::accept();
	}
	
	void SpeedLimitsDlg::reject()
	{
		saveState();
		QDialog::reject();
	}
	
	void SpeedLimitsDlg::apply()
	{
		model->apply();
		m_apply->setEnabled(false);
		
		bool apply = false;
		if (Settings::maxUploadRate() != m_upload_rate->value())
		{
			Settings::setMaxUploadRate(m_upload_rate->value());
			apply = true;
		}
		
		if (Settings::maxDownloadRate() != m_download_rate->value())
		{
			Settings::setMaxDownloadRate(m_download_rate->value());
			apply = true;
		}
		
		if (apply)
		{
			kt::ApplySettings();
			Settings::self()->writeConfig();
		}
	}
	
	void SpeedLimitsDlg::spinBoxValueChanged(int)
	{
		m_apply->setEnabled(true);
	}

}

#include "speedlimitsdlg.moc"
