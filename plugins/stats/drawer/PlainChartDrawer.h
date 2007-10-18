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

#ifndef PlainChartDrawer_H_
#define PlainChartDrawer_H_

#include <QWidget>
#include <QFrame>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QStyleOption>
#include <QMenu>
#include <QString>
#include <QImage>
#include <QPalette>

#include <kfiledialog.h>
#include <kurl.h>
#include <kmenu.h>

#include <utility>
#include <memory>

#include <ChartDrawer.h>
#include <ChartDrawerData.h>

namespace kt {

/** \brief Basic chart drawer widget
\author Krzysztof Kundzicz <athantor@gmail.com>
*/
class PlainChartDrawer : public QFrame, public ChartDrawer
{
	Q_OBJECT 
	private:
		///Pointer to context menu
		std::auto_ptr<KMenu> pmCtxMenu;
		
		///Height of the chart area ( \b not widget height!)
		inline wgtunit_t height() const;
		///Width of the chart area ( \b not widget width!)
		inline wgtunit_t width() const;
		
		/// Translates coords
		inline wgtunit_t TY(const wgtunit_t) const;
		
		/** \brief Translates chart X coord to screen coord
		\param xcc X chart coord
		\return Screen X coord
		*/
		inline wgtunit_t FindXScreenCoords(const wgtunit_t xcc) const;
		/** \brief Translates chart Y coord to screen coord
		\param ycc Y chart coord
		\return Screen Y coord
		*/
		inline wgtunit_t FindYScreenCoords(const wgtunit_t ycc) const;
		
		///Makes a context menu for widget
		void MakeCtxMenu();
		
		/** \brief Draws chart's scale
		\param rPnt Painter object
		*/
		void DrawScale(QPainter & rPnt);
		/** \brief Draws chart's frame
		\param rPnt Painter object
		*/
		void DrawFrame(QPainter & rPnt);
		/** \brief Draws chart
		\param rPnt Painter object
		*/
		void DrawChart(QPainter & rPnt);
		
		/** \brief Draws chart's lines
		\param rPnt Painter object
		\param rCdd Dataset to draw
		*/
		void DrawChartLine(QPainter & rPnt, const ChartDrawerData & rCdd);
		/** \brief Draws current values of the sets
		\param rPnt Painter object
		\param rCdd Dataset to draw
		\param idx Set's index
		*/
		void DrawCurrentValue(QPainter & rPnt, const ChartDrawerData & rCdd, size_t idx);
		/** \brief Marks maximum values
		\param rPnt Painter object
		\param rCdd Dataset to draw
		\param idx Set's index
		*/
		void DrawMaximum(QPainter & rPnt, const ChartDrawerData & rCdd, size_t idx);
		
	public:
		/** \brief Constructor
		\param p Parent
		*/
		PlainChartDrawer(QWidget * p = 0);
		///Destructor
		virtual ~PlainChartDrawer();
		
		/** \brief Widget's paint event
		\param pPevt Event
		*/
		virtual void paintEvent ( QPaintEvent * pPevt );
		
	public slots:
		void ShowCtxMenu(const QPoint & rP);
		void RenderToImage();
		
		void AddValue (const size_t idx, const wgtunit_t val, const bool upd=false);
		void AddDataSet (ChartDrawerData Cdd);
		void InsertDataSet (const size_t idx, ChartDrawerData Cdd);
		void RemoveDataSet (const size_t idx);
		void Zero (const size_t idx);
		void ZeroAll();
		void SetUnitName(const QString & rN);
		void SetPen (const size_t idx, const QPen &rP);
		void SetXMax (const wgtunit_t x);
		void SetYMax (const wgtunit_t y);
		void FindSetMax ();
		void SetUuid (const size_t idx, const QUuid &rQ);
		int16_t FindUuidInSet (const QUuid &rQ) const;
		void SetMaxMode (const MaxMode mm);
		const QUuid * GetUuid(const size_t idx) const;
		QString MakeLegendStr() ;
		void SetLegend(const QString & rL);
		void update ();

		void EnableAntiAlias (const bool aa);
		void EnableBgdGrid(bool bg);
		
		

};

} //ns end

#endif
