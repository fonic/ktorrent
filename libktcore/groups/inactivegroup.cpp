/***************************************************************************
 *   Copyright (C) 2007 by Ivan Vasić   								   *
 *   ivasic@gmail.com   												   *
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
#include "inactivegroup.h"

#include <klocale.h>
#include <interfaces/torrentinterface.h>

namespace kt
{
	
InactiveGroup::InactiveGroup()
		: Group(i18n("Inactive torrents"),MIXED_GROUP,"/all/inactive")
{
	setIconByName("network-disconnect");
}


InactiveGroup::~InactiveGroup()
{}

}

bool kt::InactiveGroup::isMember(TorrentInterface * tor)
{
	if(!tor)
		return false;
	
	const bt::TorrentStats& s = tor->getStats();
	return !s.running;
}

