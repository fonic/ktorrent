 /***************************************************************************
 *   Copyright (C) 2006 by Diego R. Brogna                                 *
 *   dierbro@gmail.com                                               	   *
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
#include <qcstring.h>
#include <qdatetime.h>

#include <torrent/queuemanager.h>

#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
#include <util/log.h>
#include <net/socketmonitor.h>
#include <qfileinfo.h>
#include <kmdcodec.h>
#include <qsocketnotifier.h>
#include <kmessagebox.h>
#include <net/portlist.h>

#include <sys/mman.h>
#include "httpserver.h"

#include "webinterfacepluginsettings.h"

#define RAWREAD_BUFF_SIZE	2048

#define HTTP_404_ERROR "<html><head><title>HTTP/1.1 404 Not Found</title></head><body>HTTP/1.1 404 Not Found</body</html>"
#define HTTP_500_ERROR "<html><head><title>HTTP/1.1 500 Internal Server Error</title></head><body>HTTP/1.1 Internal Server Error</body</html>"

using namespace bt;

namespace kt{
	
	void ServerThread::run()
	{
		mutex.lock();
		HttpServer *server;
		server=0;
		int i=0, port=WebInterfacePluginSettings::port();
		
		do{
			if(!server)
				delete server;
			server=new HttpServer(core, port+i);
			i++;
		}while(!server->ok() && i<10);

		if(server->ok()){
			if(WebInterfacePluginSettings::forward())
				bt::Globals::instance().getPortList().addNewPort(server->port(),net::TCP,true);
			Out(SYS_WEB|LOG_ALL) << "Web server listen on port "<< server->port() << endl;
		}
		else{
			Out(SYS_WEB|LOG_ALL) << "Cannot bind to port " << port <<" or the 10 following ports. WebInterface plugin cannot be loaded." << endl;
			return;
		}
		p=server->port();
		mutex.unlock();
		running = true;
		while (running)
			usleep(1000);
		running = false;
		delete server;
		return;
	}

	
	void ServerThread::stop()
	{
		running = false;
	}

	HttpServer::HttpServer(CoreInterface *core, int port) : QServerSocket(port, 5){
		php_i=new PhpInterface(core);
		php_h=new PhpHandler(php_i, WebInterfacePluginSettings::phpExecutablePath());
		imgCache.setAutoDelete(true);
		rootDir=WebInterfacePluginSettings::rootDir();
		sessionTTL=WebInterfacePluginSettings::sessionTTL();
		session.logged=false;
	}
	HttpServer::~HttpServer()
	{
		delete php_i;
		delete php_h;
	}

	void HttpServer::newConnection(int s){
		QSocket* socket= new QSocket(this);
		connect(socket, SIGNAL(readyRead()), this, SLOT(slotSocketReadyToRead()));
		connect(socket, SIGNAL(delayedCloseFinished()), this, SLOT(slotConnectionClosed()));
		socket->setSocket(s);
		Out(SYS_WEB|LOG_DEBUG) << "connection from "<< socket->peerAddress().toString()  << endl;
	}

	void HttpServer::slotSocketReadyToRead(){
		QString request, data;
		unsigned int size=0;
		QSocket* socket = (QSocket*)sender();
		
		data.append(socket->readAll()); 

        	if ( !data.isEmpty() ) {
			QStringList headerLines = QStringList::split("\r\n", data);
			QStringList requestLine = QStringList::split( QRegExp("[ \r\n][ \r\n]*"), headerLines[0]);
			
			if(requestLine[0]=="GET" || requestLine[0]=="POST")
			{
				if(requestLine[1].isEmpty() || !requestLine[1].startsWith("/")){
					socket->close();
					return;
					}
				request=requestLine[1];

				if(requestLine[0]=="POST"){
					request.append('?');

					for ( QStringList::Iterator it = headerLines.begin(); it != headerLines.end(); ++it )
						if((*it).contains("Content-Length:")){
							QStringList token=QStringList::split(":", (*it));
							size=token[1].toInt();
						}

					if(size>0){
						if(headerLines.last().length()==size)
							request.append(headerLines.last());
						else
							request.append("");
					}
					else
						request.append(waitPostData(socket));
				}
				Out(SYS_WEB| LOG_DEBUG) << "request from "<< socket->peerAddress().toString() << ": " <<  request << endl;

				parseRequest(request);
				
				parseHeaderFields(headerLines);
				
				processRequest(socket);
				

			}
			else 
			{
				Out(SYS_WEB| LOG_DEBUG) << "Sorry method "<< requestLine[0].latin1() <<"not yet supported." << endl;
			}
		}
		socket->close();

	}
	
	QString HttpServer::waitPostData(QSocket* s)
	{
			unsigned int size=0;
			QString data;
			do{
			data.append(s->readAll());
			}while(s->waitForMore(500)>0);

			QStringList tmp = QStringList::split("\r\n", data);
			for ( QStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it )
				if((*it).contains("Content-Length:")){
					QStringList token=QStringList::split(":", (*it));
					size=token[1].toInt();
				}

			if(size>0){
				if(tmp.last().length()==size)
					return tmp.last();
			}
			
			return QString("");
	}

	void HttpServer::slotConnectionClosed()
	{
		QSocket* socket= (QSocket*)sender();
        	delete socket;
		Out(SYS_WEB| LOG_DEBUG) << "connection_closed" << endl;
	}


	void HttpServer::parseRequest(QString request)
	{
		//remove old data
		requestedFile="";
		requestParams.clear();

		requestedFile=request.left(request.find("?"));
		
		
		request.remove(0,requestedFile.length()+1);
		QStringList tokens = QStringList::split("&",request);
		for ( QStringList::Iterator it = tokens.begin(); it != tokens.end(); ++it ) {
			QStringList req=QStringList::split( '=', *it );
			requestParams[req[0]]=req[1];
			Out(SYS_WEB| LOG_DEBUG) << "Request key [" << req[0].latin1() << "] value [" << req[1].latin1() <<"]" << endl;
		}
	}

	void HttpServer::parseHeaderFields(QStringList headerLines)
	{
		headerField.keepAlive=false;
		headerField.gzip=false;
		headerField.ifModifiedSince=false;
		headerField.sessionId=0;
		
		for ( QStringList::Iterator it = headerLines.begin(); it != headerLines.end(); ++it ) {
			if((*it).contains("Connection:")){
				if((*it).contains("keep-alive"))
					headerField.keepAlive=false;
			}
			else if((*it).contains("Cookie:")){
				QStringList tokens = QStringList::split('=', (*it).remove("Cookie: "));
				if(tokens[0]=="SESSID")
					headerField.sessionId=tokens[1].toInt();
					
			}
			else if((*it).contains("Content-Type:")){
				if((*it).contains("gzip"))
					headerField.gzip=true;
			}
			else if((*it).contains("If-Modified-Since:")){
				headerField.ifModifiedSince=true;
			}		
		}
	}
	void HttpServer::processRequest(QSocket* s)
	{	
		QFile f(rootDir+'/'+requestedFile);
		QFileInfo finfo(f);

		//Logout
		if(requestedFile=="/login.html")
			session.logged=false;

		if(headerField.sessionId==session.sessionId){
			if(session.last_access.secsTo(QTime::currentTime())<sessionTTL){
				Out(SYS_WEB| LOG_DEBUG) << "Session valid" << endl;
				session.last_access=QTime::currentTime();
				}
			else{
				Out(SYS_WEB| LOG_DEBUG) << "Session expired" << endl;
				session.logged=false;
			}
		}
		else
			session.logged=false;

		if(!session.logged){
			if(requestParams.contains("username") && requestParams.contains("password")){
				KMD5 context(requestParams["password"]);
				if(requestParams["username"]==WebInterfacePluginSettings::username() && context.hexDigest().data()==WebInterfacePluginSettings::password()){
					session.logged=true;
					Out(SYS_WEB|LOG_ALL) << s->peerAddress().toString() << " logged in"  << endl;
					session.sessionId=rand();
					session.last_access=QTime::currentTime();
					requestParams.remove("password");
				}
				else
					session.logged=false;
			}
		}

		if(!session.logged){
			if(finfo.extension()!="ico" && finfo.extension()!="png" && finfo.exists()){
				requestedFile="login.html";
				f.setName(rootDir+'/'+requestedFile);
				finfo.setFile(f);
				session.sessionId=0;
			}
		}

		//execute request
		if(session.logged)
			php_i->exec(requestParams);
		QString header;
		
		if ( !f.open(IO_ReadOnly) || (finfo.extension()!="php" && finfo.extension()!="html" && finfo.extension()!="png" && finfo.extension()!="ico") ){
			QString data;
			header="HTTP/1.1 404 Not Found\r\n";
			header+="Server: ktorrent\r\n";
			header+="Cache-Control: private\r\n";
			header+="Connection: close\r\n";
			header+=QDateTime::currentDateTime(Qt::UTC).toString("Date: ddd, dd MMM yyy hh:mm:ss UTC");
			data=HTTP_404_ERROR;
			header+=QString("Content-Length: %1\r\n\r\n").arg(data.length());
			sendHtmlPage(s, QString(header+data).latin1());
			return;
		}
		
		if(finfo.extension()=="html"){
			QString dataFile;
			dataFile=QString(f.readAll().data());
			header="HTTP/1.1 200 OK\r\n";
			header+="Server: ktorrent\r\n";
			header+="Cache-Control: private\r\n";
			header+="Connection: close\r\n";
			header+=QDateTime::currentDateTime(Qt::UTC).toString("Date: ddd, dd MMM yyy hh:mm:ss UTC");
			header+="Content-Type: text/html\r\n";
			header+=QString("Set-Cookie: SESSID=%1\r\n").arg(session.sessionId);
			header+=QString("Content-Length: %1\r\n\r\n").arg(f.size());
			sendHtmlPage(s, QString(header+dataFile).latin1());
		}
		else if(finfo.extension()=="php"){
			QString dataFile;
			dataFile=QString(f.readAll().data());
			dataFile.truncate(f.size());
			if(php_h->executeScript(dataFile, requestParams)){
				header="HTTP/1.1 200 OK\r\n";
				header+="Server: ktorrent\r\n";
				header+="Cache-Control: private\r\n";
				header+="Connection: close\r\n";
				header+=QDateTime::currentDateTime(Qt::UTC).toString("Date: ddd, dd MMM yyy hh:mm:ss UTC");
				header+="Content-Type: text/html\r\n";
				header+=QString("Set-Cookie: SESSID=%1\r\n").arg(session.sessionId);
				header+=QString("Content-Length: %1\r\n\r\n").arg(php_h->getOutput().length());
				sendHtmlPage(s, QString(header+php_h->getOutput()).latin1());
				}
			else{
				Out(SYS_WEB|LOG_DEBUG) << "PHP executable error" << endl;
				QString data;
				header="HTTP/1.1 500 OK\r\n";
				header+="Server: ktorrent\r\n";
				header+="Cache-Control: private\r\n";
				header+="Connection: close\r\n";
				header+=QDateTime::currentDateTime(Qt::UTC).toString("Date: ddd, dd MMM yyy hh:mm:ss UTC");
				data=HTTP_500_ERROR;
				data+="\nPHP executable error";
				header+=QString("Content-Length: %1\r\n\r\n").arg(data.length());
				sendHtmlPage(s, QString(header+data).latin1());
				return;

			}
		}
		else if(finfo.extension()=="ico" || finfo.extension()=="png"){
			if(!headerField.ifModifiedSince){
				header="HTTP/1.1 200 OK\r\n";
				header+="Server: ktorrent\r\n";
				header+=QString("Set-Cookie: SESSID=%1\r\n").arg(session.sessionId);
				header+=QString("Date: ")+QDateTime::currentDateTime(Qt::UTC).toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");
				header+=QString("Last-Modified: ")+finfo.lastModified().toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");
				header+=QString("Expires: ")+QDateTime::currentDateTime(Qt::UTC).addSecs(sessionTTL).toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");
				header+="Cache-Control: private\r\n";
				header+=QString("Content-Type: image/%1\r\n").arg(finfo.extension());
				header+=QString("Content-Length: %1\r\n\r\n").arg(finfo.size());
				sendRawData(s, header, &f);
			}
			else{
				header="HTTP/1.1 304 Not Modified\r\n";
				header+="Server: ktorrent\r\n";
				header+=QString("Set-Cookie: SESSID=%1\r\n").arg(session.sessionId);
				header+=QString("Date: ")+QDateTime::currentDateTime(Qt::UTC).toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");
				header+="Cache-Control: max-age=0\r\n";
				header+=QString("If-Modified-Since: ")+finfo.lastModified().toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");
				header+=QString("Content-Type: text/html\r\n");
				header+=QString("Content-Length: 0\r\n\r\n");
				sendHtmlPage(s, QString(header).latin1());
			}
			
		}

		f.close();
	}

	void HttpServer::sendHtmlPage(QSocket* s, const char *data)
	{
		if(!s->isOpen()){
			return;
		}

		QTextStream os(s);
                os.setEncoding( QTextStream::UnicodeUTF8 );
		os << data;
	}
	
	void HttpServer::sendRawData(QSocket* s,QString header, QFile *file)
	{
		if(!s->isOpen()){
			return;
		}

		QTextStream os(s);
                os.setEncoding( QTextStream::UnicodeUTF8 );
		os << header.latin1();
		Image *im;
		im=imgCache.find(file->name(), true);
		if(im==NULL){
			Image *image= new Image();
			image->data=mmap(0, file->size(), PROT_READ, MAP_PRIVATE, file->handle(), 0);
			Out(SYS_WEB|LOG_DEBUG) << file->name() << " mmaped\n"  << endl;
			if(imgCache.insert(file->name(), image)){
				Out(SYS_WEB|LOG_DEBUG) << file->name() << " added in cache\n"  << endl;
				im=imgCache.find(file->name(), true);
			}
			else{
				Out(SYS_WEB|LOG_DEBUG) << file->name() << " not added in cache\n"  << endl;
				void *data;
				unsigned int count=0, r_size;
				data=malloc(RAWREAD_BUFF_SIZE);
				while(file->size() > count){
					bzero(data,RAWREAD_BUFF_SIZE);
					r_size=file->readBlock((char *)data, RAWREAD_BUFF_SIZE);
					s->writeBlock((const char *)data, r_size);
					s->flush();
					count+=r_size;
				}
				free(data);
				delete image;
				return;
			}

		}
		s->writeBlock((const char *)im->data, file->size());
	}





}
