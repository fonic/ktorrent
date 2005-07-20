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
#ifndef HTMLPART_H
#define HTMLPART_H

#include <khtml_part.h>

/**
@author Joris Guisson
*/
class HTMLPart : public KHTMLPart
{
	Q_OBJECT
public:
	HTMLPart(QWidget *parent = 0);
	virtual ~HTMLPart();

public slots:
	void back();
	void reload();
	void copy();

private slots:
	void openURLRequest(const KURL &url, const KParts::URLArgs &args);
	void addToHistory(const KURL & url);
//	void download(const KURL & u);

signals:
	void backAvailable(bool yes);
	void openTorrent(const KURL & url);

private:
	KURL::List history;
};

#endif
