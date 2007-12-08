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
#ifndef BTHTTPTRACKER_H
#define BTHTTPTRACKER_H

#include <qtimer.h>
#include <btcore_export.h>
#include "tracker.h"

class KJob;

namespace KIO
{
	class MetaData;
}

namespace bt
{
	

	/**
	 * @author Joris Guisson
	 * @brief Communicates with the tracker
	 *
	 * This class uses the HTTP protocol to communicate with the tracker.
	 */
	class BTCORE_EXPORT HTTPTracker : public Tracker
	{
		Q_OBJECT
	public:
		HTTPTracker(const KUrl & url,TorrentInterface* tor,const PeerID & id,int tier);
		virtual ~HTTPTracker();
		
		virtual void start();
		virtual void stop(WaitJob* wjob = 0);
		virtual void completed();
		virtual void manualUpdate();
		virtual Uint32 failureCount() const {return failures;}
		virtual void scrape();
		
		static void setProxy(const QString & proxy);
		static void setProxyEnabled(bool on);
		
	private slots:
		void onAnnounceResult(KJob* j);
		void onScrapeResult(KJob* j);
		void emitInvalidURLFailure();

	private:
		void doRequest(WaitJob* wjob = 0);
		bool updateData(const QByteArray & data);
		void setupMetaData(KIO::MetaData & md);
 		void doAnnounceQueue();
 		void doAnnounce(const KUrl & u);

	private:
		KJob* active_job;
		KUrl::List announce_queue;
		QString event;
		Uint32 failures;
		
		static bool proxy_on;
		static QString proxy;
	};

}

#endif