/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
#ifndef KTWEEKDAYMODEL_H
#define KTWEEKDAYMODEL_H

#include <QAbstractListModel>

namespace kt
{

    /**
        Model to display the days of a week in a list view. The weekdays are checkable.
        @author
    */
    class WeekDayModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        WeekDayModel(QObject* parent);
        virtual ~WeekDayModel();

        virtual int rowCount(const QModelIndex& parent) const;
        virtual QVariant data(const QModelIndex& index, int role) const;
        virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
        virtual Qt::ItemFlags flags(const QModelIndex& index) const;

        /// Get all the days which have been checked
        QList<int> checkedDays() const;
    private:
        bool checked[7];
    };

}

#endif
