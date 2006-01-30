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
#ifndef BTHTTPREQUEST_H
#define BTHTTPREQUEST_H

#include <qobject.h>
#include <kurl.h>
#include <kstreamsocket.h>
#include "constants.h"

namespace bt 
{
	
	/**
	 * @author Joris Guisson
	 * 
	 * Just create one, fill in the fields,
	 * connect to the right signals and forget about it. After the reply has been recieved or
	 * an error occured, the appropriate signal will be emitted.
	*/
	class HTTPRequest : public QObject
	{
		Q_OBJECT
	public:
		/**
		 * Constructor, set the url and the request header.
		 * @param hdr The http request header
		 * @param payload The payload
		 * @param host The host
		 * @param port THe port
		 */
		HTTPRequest(const QString & hdr,const QString & payload,const QString & host,Uint16 port);
		virtual ~HTTPRequest();
		
		/**
		 * Open a connetion and send the request.
		 */
		void start();
		
	signals:
		/**
		 * An OK reply was sent.
	     * @param r The sender of the request
		 * @param data The data of the reply
		 */
		void replyOK(bt::HTTPRequest* r,const QString & data);
		
		/**
		 * Anything else but an 200 OK was sent.
		 * @param r The sender of the request
		 * @param data The data of the reply
		 */
		void replyError(bt::HTTPRequest* r,const QString & data);
		
		/**
		 * No reply was sent and an error or timeout occured.
		 * @param r The sender of the request
		 * @param timeout Wether or not a timeout occured
		 */
		void error(bt::HTTPRequest* r,bool timeout);
		
	private slots:
		void onReadyRead();
		void onError(int);
		void onTimeout();
		
	private:
		KNetwork::KStreamSocket* sock;
		QString hdr,payload;
	};

}

#endif
