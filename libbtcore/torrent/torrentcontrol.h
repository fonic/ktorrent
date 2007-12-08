/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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
#ifndef BTTORRENTCONTROL_H
#define BTTORRENTCONTROL_H

#include <qobject.h>
#include <qtimer.h>
#include <kurl.h>
#include "globals.h"
#include <util/timer.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/trackerslist.h>
#include <btcore_export.h>

class QStringList;
class QString;



namespace bt
{
	class Choker;
	class Torrent;
	class PeerSourceManager;
	class ChunkManager;
	class PeerManager;
	class Downloader;
	class Uploader;
	class Peer;
	class BitSet;
	class QueueManagerInterface;
	class PreallocationThread;
	class TimeEstimator;
	class DataCheckerThread;
	class WaitJob;
	class MonitorInterface;
	
	/**
	 * @author Joris Guisson
	 * @brief Controls just about everything
	 * 
	 * This is the interface which any user gets to deal with.
	 * This class controls the uploading, downloading, choking,
	 * updating the tracker and chunk management.
	 */
	class BTCORE_EXPORT TorrentControl : public TorrentInterface
	{
		Q_OBJECT
	public:
		TorrentControl();
		virtual ~TorrentControl();

		/**
		 * Get a BitSet of the status of all Chunks
		 */
		const BitSet & downloadedChunksBitSet() const;

		/**
		 * Get a BitSet of the availability of all Chunks
		 */
		const BitSet & availableChunksBitSet() const;

		/**
		 * Get a BitSet of the excluded Chunks
		 */
		const BitSet & excludedChunksBitSet() const;
		
		/**
		 * Get a BitSet of the only seed chunks 
		 */
		const BitSet & onlySeedChunksBitSet() const;
		
		/**
		 * Initialize the TorrentControl. 
		 * @param qman The QueueManager
		 * @param torrent The filename of the torrent file
		 * @param tmpdir The directory to store temporary data
		 * @param datadir The directory to store the actual file(s)
		 * 		(only used the first time we load a torrent)
		 * @param default_save_dir Default save directory (null if not set)
		 * @throw Error when something goes wrong
		 */
		void init(QueueManagerInterface* qman,
				const QString & torrent,
				const QString & tmpdir,
				const QString & datadir,
				const QString & default_save_dir);
		
		/**
		 * Initialize the TorrentControl. 
		 * @param qman The QueueManager
		 * @param data The data of the torrent
		 * @param tmpdir The directory to store temporary data
		 * @param datadir The directory to store the actual file(s)
		 * 		(only used the first time we load a torrent)
		 * @param default_save_dir Default save directory (null if not set)
		 * @throw Error when something goes wrong
		 */
		void init(QueueManagerInterface* qman,
				  const QByteArray & data,
				  const QString & tmpdir,
				  const QString & datadir,
				  const QString & default_save_dir);

		/**
		 * Change to a new data dir. If this fails
		 * we will fall back on the old directory.
		 * @param new_dir The new directory
		 * @return true upon succes
		 */
		bool changeTorDir(const QString & new_dir);
		
		
		/**
		 * Change torrents output directory. If this fails we will fall back on the old directory.
		 * @param new_dir The new directory
		 * @param move_files Wether or not to move the files
		 * @return true upon success.
		 */
		bool changeOutputDir(const QString& new_dir,bool move_files = true);

		/**
		 * Roll back the previous changeDataDir call.
		 * Does nothing if there was no previous changeDataDir call.
		 */
		void rollback();
	
		/// Gets the TrackersList interface
		TrackersList* getTrackersList();
		
		/// Gets the TrackersList interface
		const TrackersList* getTrackersList() const;
	
		/// Get the data directory of this torrent
		QString getDataDir() const {return outputdir;}

		/// Get the torX dir.
		QString getTorDir() const {return tordir;}

		/// Set the monitor
		void setMonitor(MonitorInterface* tmo);

		/// Get the Torrent.
		const Torrent & getTorrent() const {return *tor;}
		
		/**
		 * Get the download running time of this torrent in seconds
		 * @return Uint32 - time in seconds
		 */
		Uint32 getRunningTimeDL() const;
		
		/**
		 * Get the upload running time of this torrent in seconds
		 * @return Uint32 - time in seconds
		 */
		Uint32 getRunningTimeUL() const;

		/**
		* Checks if torrent is multimedial and chunks needed for preview are downloaded
		* @param start_chunk The index of starting chunk to check
		* @param end_chunk The index of the last chunk to check
		* In case of single torrent file defaults can be used (0,1)
		**/
		bool readyForPreview(int start_chunk = 0, int end_chunk = 1);

		/// Get the time to the next tracker update in seconds.
		Uint32 getTimeToNextTrackerUpdate() const;

		/// Get a short error message
		QString getShortErrorMessage() const {return error_msg;}
		
		virtual Uint32 getNumFiles() const;
		virtual TorrentFileInterface & getTorrentFile(Uint32 index);
		virtual const TorrentFileInterface & getTorrentFile(Uint32 index) const;
		virtual void recreateMissingFiles();
		virtual void dndMissingFiles();
		virtual void addPeerSource(PeerSource* ps);
		virtual void removePeerSource(PeerSource* ps);
		
		int getPriority() const { return istats.priority; }
		void setPriority(int p);

		virtual bool overMaxRatio();		
		virtual void setMaxShareRatio(float ratio);
		virtual float getMaxShareRatio() const { return stats.max_share_ratio; }
 		
		virtual bool overMaxSeedTime();
		virtual void setMaxSeedTime(float hours);
		virtual float getMaxSeedTime() const {return stats.max_seed_time;}
	
		/// Tell the TorrentControl obj to preallocate diskspace in the next update
		void setPreallocateDiskSpace(bool pa) {prealloc = pa;}
		
		/// Make a string out of the status message
		virtual QString statusToString() const;
		
		/// Checks if tracker announce is allowed (minimum interval 60 seconds)
		bool announceAllowed();
		
		void startDataCheck(bt::DataCheckerListener* lst);
		
		/// Test if the torrent has existing files, only works the first time a torrent is loaded
		bool hasExistingFiles() const;
		
		/**
		 * Test all files and see if they are not missing.
		 * If so put them in a list
		 */
		bool hasMissingFiles(QStringList & sl);
		
		
		virtual Uint32 getNumDHTNodes() const;
		virtual const DHTNode & getDHTNode(Uint32 i) const;
		virtual void deleteDataFiles();
		virtual const bt::PeerID & getOwnPeerID() const;
		virtual bool updateNeeded() const;
		
		/**
		 * Called by the PeerSourceManager when it is going to start a new tracker.
		 */
		void resetTrackerStats();
		
		/**
		 * Returns estimated time left for finishing download. Returned value is in seconds.
		 * Uses TimeEstimator class to calculate this value.
		 */
		Uint32 getETA();

		/// Is a feature enabled
		bool isFeatureEnabled(TorrentFeature tf);
		
		/// Disable or enable a feature
		void setFeatureEnabled(TorrentFeature tf,bool on);
		
		/// Create all the necessary files
		void createFiles();
		
		///Checks if diskspace is low
		bool checkDiskSpace(bool emit_sig = true);
		
		virtual void setTrafficLimits(Uint32 up,Uint32 down);
		virtual void getTrafficLimits(Uint32 & up,Uint32 & down);
		virtual const SHA1Hash & getInfoHash() const;
	
		/// Get the PeerManager
		const PeerManager * getPeerMgr() const;

		/// Are we in the process of moving files
		bool isMovingFiles() const {return moving_files;}
		
	public slots:
		/**
		 * Update the object, should be called periodically.
		 */
		void update();
		
		/**
		 * Start the download of the torrent.
		 */
		void start();
		
		/**
		 * Stop the download, closes all connections.
		 * @param user wether or not the user did this explicitly
		 * @param wjob WaitJob to wait at exit for the completion of stopped requests
		 */
		void stop(bool user,WaitJob* wjob = 0);
			
		/**
		 * Update the tracker, this should normally handled internally.
		 * We leave it public so that the user can do a manual announce.
		 */
		void updateTracker();

		/**
		 * Scrape the tracker.
		 * */
		void scrapeTracker();

		/**
		 * The tracker status has changed.
		 * @param ns New status
		 */
		void trackerStatusChanged(const QString & ns);

		/**
		 * A scrape has finished on the tracker.
		 * */
		void trackerScrapeDone();
		
		/**
		 * Set the move upon completion directory. 
		 * @param dir The directory an empty url disables this feature
		 */
		static void setMoveWhenCompletedDir(const KUrl & dir) {completed_dir = dir;}
		
		/**
		 * Enable or disable data check upon completion
		 * @param on 
		 */
		static void setDataCheckWhenCompleted(bool on) {completed_datacheck = on;}
		
		/**
		 * Set the minimum amount of diskspace in MB. When there is less then this free, torrents will be stopped.
		 * @param m 
		 */
		static void setMinimumDiskSpace(Uint32 m) {min_diskspace = m;}
		
		/**
		 * Enable or disable automatic datachecking when to many corrupted chunks have been found on disk.
		 * @param on 
		 */
		static void setAutoRecheck(bool on) {auto_recheck = on;}
		
		/**
		 * Set the number of corrupted chunks for a before we start an automatic recheck.
		 * @param m 
		 */
		static void setNumCorruptedForRecheck(Uint32 m) {num_corrupted_for_recheck = m;}
		
	private slots:
		void onNewPeer(Peer* p);
		void onPeerRemoved(Peer* p);
		void doChoking();
		void onIOError(const QString & msg);
		void onPortPacket(const QString & ip,Uint16 port);
		/// Update the stats of the torrent.
		void updateStats();
		void corrupted(Uint32 chunk);
		
	private:	
		void updateTracker(const QString & ev,bool last_succes = true);
		void updateStatusMsg();
		void saveStats();
		void loadStats();
		void loadOutputDir();
		void getSeederInfo(Uint32 & total,Uint32 & connected_to) const;
		void getLeecherInfo(Uint32 & total,Uint32 & connected_to) const;
		void migrateTorrent(const QString & default_save_dir);
		void continueStart();
		virtual void handleError(const QString & err);

		void initInternal(QueueManagerInterface* qman,const QString & tmpdir,
						  const QString & ddir,const QString & default_save_dir,bool first_time);
		
		void checkExisting(QueueManagerInterface* qman);
		void setupDirs(const QString & tmpdir,const QString & ddir);
		void setupStats();
		void setupData(const QString & ddir);
		void afterDataCheck();
		void preallocThreadDone();
		virtual bool isCheckingData(bool & finished) const;
		
	private:
		Torrent* tor;
		PeerSourceManager* psman;
		ChunkManager* cman;
		PeerManager* pman;
		Downloader* down;
		Uploader* up;
		Choker* choke;
		TimeEstimator* m_eta;
		MonitorInterface* tmon;
		
		Timer choker_update_timer;
		Timer stats_save_timer;
		Timer stalled_timer;
		
		QString tordir;
		QString old_tordir;
		QString outputdir;
		QString error_msg;
		
		bool prealloc;
		PreallocationThread* prealloc_thread;
		DataCheckerThread* dcheck_thread;
		TimeStamp last_diskspace_check;
		bool moving_files;
		
		struct InternalStats
		{
			QDateTime time_started_dl; 
			QDateTime time_started_ul;
			Uint32 running_time_dl;
			Uint32 running_time_ul;
			Uint64 prev_bytes_dl;
			Uint64 prev_bytes_ul;
			Uint64 trk_prev_bytes_dl;
			Uint64 trk_prev_bytes_ul;
			Uint64 session_bytes_uploaded;
			bool io_error;
			bool custom_output_name;
			Uint16 port;
			int priority;
			bool dht_on;
			TimeStamp last_announce;
			bool diskspace_warning_emitted;
		};
		
		Uint32 upload_gid; // group ID for upload
		Uint32 upload_limit; 
		Uint32 download_gid; // group ID for download
		Uint32 download_limit; 
		
		InternalStats istats;
		
		static KUrl completed_dir;
		static bool completed_datacheck;
		static Uint32 min_diskspace;
		static bool auto_recheck;
		static Uint32 num_corrupted_for_recheck;
	};
	
	
}

#endif