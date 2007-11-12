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
#ifndef KTWEEKSCENE_H
#define KTWEEKSCENE_H

#include <QGraphicsScene>

namespace kt
{
	struct ScheduleItem;

	/**
		@author
	*/
	class WeekScene : public QGraphicsScene
	{
		Q_OBJECT
	public:
		WeekScene(QObject* parent);
		virtual ~WeekScene();
		
		/**
		 * Add an item to the schedule.
		 * @param item The item to add
		 */
		QGraphicsItem* addScheduleItem(const ScheduleItem & item);
		
		/**
		 * Update the text of the status line
		 * @param up Up speed
		 * @param down Down speed
		 * @param paused Paused or not
		 */
		void updateStatusText(int up,int down,bool paused);
		
	signals:
		/**
		 * Emitted when an item has been double clicked.
		 * @param gi Item double clicked
		 */
		void itemDoubleClicked(QGraphicsItem* gi);

	private:
		void addCalendar();
		virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* ev);
	//	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* ev);
		virtual void mousePressEvent(QGraphicsSceneMouseEvent* ev);
		
	private:
		qreal xoff;
		qreal yoff;
		qreal day_width;
		qreal hour_height;
		QGraphicsTextItem* status;
	};

}

#endif