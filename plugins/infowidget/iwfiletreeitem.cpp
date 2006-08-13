/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <klocale.h>
#include <kglobal.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <util/bitset.h>
#include "iwfiletreeitem.h"
#include "iwfiletreediritem.h"
#include "functions.h"

using namespace kt;

namespace kt
{
				
	IWFileTreeItem::IWFileTreeItem(IWFileTreeDirItem* item,const QString & name,kt::TorrentFileInterface & file)
		: FileTreeItem(item,name,file)
	{
		perc_complete = 0.0;
		connect(&file,SIGNAL(downloadPercentageChanged( float )),this,SLOT(onPercentageUpdated( float )));
		connect(&file,SIGNAL(previewAvailable( bool )),this,SLOT(onPreviewAvailable( bool )));
	}
	
	IWFileTreeItem::~IWFileTreeItem()
	{
	}
	
	int IWFileTreeItem::compare(QListViewItem* i, int col, bool ascending) const
	{
		if (col == 4)
		{
			IWFileTreeItem* other = dynamic_cast<IWFileTreeItem*>(i);
			if (!other)
				return 0;
			else
				return CompareVal(perc_complete,other->perc_complete);
		}
		else
		{
			return FileTreeItem::compare(i, col, ascending);
		}
	}

	
	void IWFileTreeItem::updatePreviewInformation(kt::TorrentInterface* tc)
	{
		if (file.isMultimedia())
		{
			if (tc->readyForPreview(file.getFirstChunk(), file.getFirstChunk()+1) )
			{
				setText(3, i18n("Available"));
			}
			else
			{
				setText(3, i18n("Pending"));
			}
		}
		else
			setText(3, i18n("No"));
	}

	void IWFileTreeItem::updatePercentageInformation()
	{
		onPercentageUpdated(file.getDownloadPercentage());
	}
	
	void IWFileTreeItem::onPercentageUpdated(float p)
	{
		double percent = p;
		if (percent < 0.0)
			percent = 0.0;
		else if (percent > 100.0)
			percent = 100.0;
		KLocale* loc = KGlobal::locale();
		setText(4,i18n("%1 %").arg(loc->formatNumber(percent,2)));
		perc_complete = percent;
	}
	
	void IWFileTreeItem::onPreviewAvailable(bool av)
	{
		if (av)
		{
			setText(3, i18n("Available"));
		}
		else if (file.isMultimedia())
		{
			setText(3, i18n("Pending"));
		}
		else
		{
			setText(3, i18n("No"));
		}
	}

	void IWFileTreeItem::updatePriorityInformation(kt::TorrentInterface* tc)
	{
		switch(file.getPriority())
		{
			case FIRST_PRIORITY:
				setText(2, i18n("Yes, First"));
				setChecked(true);
				break;
			case LAST_PRIORITY:
				setText(2, i18n("Yes, Last"));
				setChecked(true);
				break;
			case EXCLUDED:
				setText(2, i18n("No"));
				setChecked(false);
				break;
			case PREVIEW_PRIORITY:
				break;
			default:
				setText(2, i18n("Yes"));
				setChecked(true);
				break;
		}
	}
	
	void IWFileTreeItem::updateDNDInformation()
	{
		if (file.doNotDownload() && isOn())
		{
			setChecked(false);
			setText(2, i18n("No"));
		}
	}
}

#include "iwfiletreeitem.moc"