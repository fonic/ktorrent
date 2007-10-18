/***************************************************************************
 *   Copyright © 2007 by Krzysztof Kundzicz                                *
 *   athantor@gmail.com                                                    *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef PluginPage_H_
#define PluginPage_H_

#include <QWidget>
#include <utility>

#include <interfaces/plugin.h>

namespace kt {

/** \brief Base class for plugin's tabs in the main UI
\author Krzysztof Kundzicz <athantor@gmail.com>
*/
class PluginPage : public QWidget
{
	Q_OBJECT

	protected:
		///Setups UI
		virtual void SetupUi() = 0;
	public:
		//sum , msmnts
		/** \brief Type used for computing average
		* 
		* Layout:
		* - First: Sum of measurements
		* - Second: Amount
		*/
		typedef std::pair<long double, long double> avg_t;
		
		/** \brief Constructor
		\param p Parent
		*/
		PluginPage(QWidget * p);
		///Destructor
		virtual ~PluginPage();
		
	public slots:
		///Applies settings
		virtual void ApplySettings() = 0;
		///Updates all charts
		virtual void UpdateAllCharts() = 0;
		/** \brief Gathers data
		\param pP Plugin interface
		*/
		virtual void GatherData(Plugin * pP) = 0;
		///Resets average
		virtual void ResetAvg() = 0;
};

} // ns end

#endif
