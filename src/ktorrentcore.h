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
#ifndef KTORRENTCORE_H
#define KTORRENTCORE_H

#include <qobject.h>
#include <qtimer.h>

namespace bt
{
	class TorrentControl;
}

class KProgress;

/**
 * @author Joris Guisson
 * @brief Keeps track of all TorrentControl objects
 *
 * This class keeps track of all TorrentControl objects.
 */
class KTorrentCore : public QObject
{
	Q_OBJECT
public:
	KTorrentCore();
	virtual ~KTorrentCore();
	
	
	/**
	 * Load all torrents from the data dir.
	 */
	void loadTorrents();
	
	/**
	 * Set the maximum number of simultanious downloads.
	 * @param max The max num (0 == no limit)
	 */
	void setMaxDownloads(int max);
	
	/**
	 * Set wether or not we should keep seeding after
	 * a download has finished.
	 * @param ks Keep seeding yes or no
	 */
	void setKeepSeeding(bool ks);
	
	/**
	 * Change the data dir. This involves copying
	 * all data from the old dir to the new.
	 * This can offcourse go horribly wrong, therefore
	 * if it doesn't succeed it returns false
	 * and leaves everything where it supposed to be.
	 * @param new_dir The new directory
	 */
	bool changeDataDir(const QString & new_dir);
	
	/**
	 * Save active torrents on exit.
	 */
	void onExit();

	/**
	 * Start all, takes into account the maximum number of downloads.
	 */
	void startAll();

	/**
	 * Stop all torrents.
	 */
	void stopAll();

	/**
	 * Start a torrent, takes into account the maximum number of downloads.
	 * @param tc The TorrentControl
	 */
	void start(bt::TorrentControl* tc);

	/**
	 * Stop a torrent, may start another download if it hasn't been started.
	 * @param tc The TorrentControl
	 */
	void stop(bt::TorrentControl* tc);

	/**
	 * Make a torrent file
	 * @param file The file or dir to make a torrent of
	 * @param trackers A list of trackers
	 * @param chunk_size The size of each chunk (in KB)
	 * @param name The torrents name (usually filename)
	 * @param comments The comments
	 * @param seed Wether or not to start seeding or not
	 * @param output_file File to store the torrent file
	 * @param prog Progress bar to update
	 */
	void makeTorrent(const QString & file,const QStringList & trackers,
					 int chunk_size,const QString & name,const QString & comments,
					 bool seed,const QString & output_file,KProgress* prog);
public slots:
	/**
	 * Load a torrent file. Pops up an error dialog
	 * if something goes wrong.
	 * @param file The torrent file
	 */
	void load(const QString & file);
	
	/**
	 * Remove a download.This will delete all temp
	 * data from this TorrentControl And delete the
	 * TorrentControl itself. It can also potentially
	 * start a new download (when one is waiting to be downloaded).
	 * @param tc 
	 */
	void remove(bt::TorrentControl* tc);

	/**
	 * Update all torrents.
	 */
	void update();
	
signals:
	/**
	 * A TorrentControl was added
	 * @param tc 
	 */
	void torrentAdded(bt::TorrentControl* tc);

	
	/**
	 * A TorrentControl was removed
	 * @param tc
	 */
	void torrentRemoved(bt::TorrentControl* tc);
	
	/**
	 * A TorrentControl has finished downloading.
	 * @param tc
	 */
	void finished(bt::TorrentControl* tc);
	
private:
	QString findNewTorrentDir() const;
	int getNumRunning() const;
	void rollback(const QPtrList<bt::TorrentControl> & success);
	
private slots:
	void torrentFinished(bt::TorrentControl* tc);
	
private:
	QPtrList<bt::TorrentControl> downloads;
	QString data_dir;
	int max_downloads;
	bool keep_seeding;
	QTimer update_timer;
};

#endif
