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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef KTORRENTVIEWITEM_H
#define KTORRENTVIEWITEM_H

#include <klistview.h>

namespace kt
{
	class TorrentInterface;
}

/**
@author Joris Guisson
*/
class KTorrentViewItem : public KListViewItem
{
	kt::TorrentInterface* tc;
public:
	KTorrentViewItem(QListView* parent,kt::TorrentInterface* tc);
	virtual ~KTorrentViewItem();

	kt::TorrentInterface* getTC() {return tc;}
	void update();

private:
	int compare(QListViewItem * i,int col,bool ascending) const;
	void paintCell(QPainter* p,const QColorGroup & cg,int column,int width,int align);

};

#endif
