using System;
using System.Threading;
using System.IO;
using System.Windows.Forms;
using System.Diagnostics;
using System.Runtime.InteropServices;



using TorrentHandle = System.Int32;



public partial class MainForm : Form
{
	public ListViewItem FindTorrentItemInMainListView(TorrentHandle torrentId)
	{
		for(Int32 i=0; i < mListView.Items.Count; i++)
		{
			TorrentHandle handle = (TorrentHandle)mListView.Items[i].Tag;
			if (handle == torrentId)
			{
				return mListView.Items[i];
			}
		}
		return null;
	}// END FindTorrentItemInMainListView


	public void RefreshListView()
    {
		// remove old torrents
		bool removed=false;
		do
		{
			removed = false;
			for (Int32 i = 0; i < mListView.Items.Count; i++)
			{
				TorrentHandle handle = (TorrentHandle) mListView.Items[i].Tag;

				if (torrentManager.GetTorrent(handle) == null)
				{
					mListView.Items.Remove(mListView.Items[i]);
					removed = true;
					break;
				}
			}
		}
		while(removed==true);


		for (Int32 i = 0; i < torrentManager.TorrentCount; i++)
		{
			Torrent torrent = torrentManager.TorrentAtIndex(i);

			DllInterface.TorrentMetaData meta = torrent.MetaData;
		

			ListViewItem listViewItem = FindTorrentItemInMainListView(torrent.Handle);
			ListViewItem.ListViewSubItem idItem;
			ListViewItem.ListViewSubItem sizeItem;
			ListViewItem.ListViewSubItem doneItem;
			ListViewItem.ListViewSubItem statusItem;
			ListViewItem.ListViewSubItem seedsItem;
			ListViewItem.ListViewSubItem peersItem;
			ListViewItem.ListViewSubItem downSpeedItem;
			ListViewItem.ListViewSubItem upSpeedItem;
			ListViewItem.ListViewSubItem etaItem;
			

			bool addToControl = false;
			if(listViewItem == null)
			{
				addToControl = true;
			}

			if (addToControl)
			{
				// create the new item and pass the torrent name, all other details are sub items
				listViewItem = new System.Windows.Forms.ListViewItem(meta.mName);
				
				idItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(idItem);

				sizeItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(sizeItem);

				doneItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(doneItem);

				statusItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(statusItem);

				seedsItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(seedsItem);

				peersItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(peersItem);

				downSpeedItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(downSpeedItem);

				upSpeedItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(upSpeedItem);

				etaItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(etaItem);

				mListView.Items.AddRange(new System.Windows.Forms.ListViewItem[] { listViewItem });

				// custom data 
				listViewItem.Tag = torrent.Handle;
			}
			else
			{
				idItem = listViewItem.SubItems[1];
				sizeItem = listViewItem.SubItems[2];
				doneItem = listViewItem.SubItems[3];
				statusItem = listViewItem.SubItems[4];
				seedsItem = listViewItem.SubItems[5];
				peersItem = listViewItem.SubItems[6];
				downSpeedItem = listViewItem.SubItems[7];
				upSpeedItem = listViewItem.SubItems[8];
				etaItem = listViewItem.SubItems[9];
			}

			// set the image depending on if the connection is encrypted or not
			if (DllInterface.DoesTorrentOnlyUsesEncryptedConnections(torrent.Handle))
			{
				// fully encrypted, padlock icon
				listViewItem.ImageIndex = 6;
			}
			else
			{
				// some connections are plain text
				listViewItem.ImageIndex = 7;
			}


			String strNewValue;

			// Name
			if (listViewItem.Text != meta.mName)
			{
				listViewItem.Text = meta.mName;
			}

			strNewValue = torrent.Handle.ToString();
			if (idItem.Text != strNewValue)
			{
				idItem.Text = strNewValue;
			}

			// size
			Int64 totalSize = meta.mTotalSize;
			strNewValue = FormatBytes(totalSize); //String.Format("{0:0} MB", (totalSize / (1024.0f * 1024.0f))); 
			if (sizeItem.Text != strNewValue)
			{				
				sizeItem.Text = strNewValue;
			}


			// done
			float onePercent = (meta.mTotalPieces / 100.0f);
			float done = meta.mPiecesDownloaded / onePercent;
			if (float.IsNaN(done)) done = 0.0f;
			strNewValue = String.Format("{0:0.0}%", done);
			if (doneItem.Text != strNewValue)
			{
				doneItem.Text = strNewValue;
			}

			// status 
			switch(meta.mState)
			{
				case DllInterface.TorrentMetaData.TorrentState.Stopped:
					strNewValue = String.Format("Stopped");
					break;

				case DllInterface.TorrentMetaData.TorrentState.CreateFiles:
					strNewValue = String.Format("Creating Files");
					break;

				case DllInterface.TorrentMetaData.TorrentState.PeerMode:
					strNewValue = String.Format("Downloading");
					break;

				case DllInterface.TorrentMetaData.TorrentState.SeedMode:
					strNewValue = String.Format("Seeding");
					break;

				case DllInterface.TorrentMetaData.TorrentState.Queued:
					strNewValue = String.Format("Queued");
					break;

				case DllInterface.TorrentMetaData.TorrentState.Rechecking:
					strNewValue = String.Format("Checking");
					break;
			}
			if (statusItem.Text != strNewValue)
			{
				statusItem.Text = strNewValue;
			}

			// Seeds
			UInt32 numConnectedSeeds = meta.mNumSeeds;
			//UInt32 totalSeeds = 0;
			//strNewValue = String.Format("{0} ({1})", numConnectedSeeds, totalSeeds);
			strNewValue = String.Format("{0}", numConnectedSeeds);
			if (seedsItem.Text != strNewValue)
			{
				seedsItem.Text = strNewValue;
			}

			// Peers 
			UInt32 numConnectedPeers = meta.mNumPeers;
			//UInt32 totalPeers = 0;
			strNewValue = String.Format("{0}", numConnectedPeers);
			if (peersItem.Text != strNewValue)
			{
				peersItem.Text = strNewValue;
			}


			// down speed
			float speed = ((float)meta.mDownloadSpeed / 1024.0f);
			strNewValue = String.Format("{0:0.0} kB/s", speed);
			if (downSpeedItem.Text != strNewValue)
			{
				downSpeedItem.Text = strNewValue;
			}
	
			// up speed
			speed = ((float)meta.mUploadSpeed / 1024.0f);
			strNewValue = String.Format("{0:0.0} kB/s", speed);
			if (upSpeedItem.Text != strNewValue)
			{
				upSpeedItem.Text = strNewValue;
			}


			// TODO : if state is done, leave the following field blank...


			// eta
			if (meta.mDownloadSpeed == 0)
			{
				char chInfinity = (char)0x221E;

				strNewValue = String.Format("{0}", chInfinity);
				if (etaItem.Text != strNewValue)
				{
					etaItem.Text = strNewValue;
				}
			}
			else
			{
				TimeSpan t = TimeSpan.FromSeconds(meta.mEta);
				if (t.Days > 0)
				{
					strNewValue = string.Format("{0}d {1:D2}h {2:D2}m", t.Days, t.Hours, t.Minutes);
				}
				else if (t.Hours > 0)
				{
					strNewValue = string.Format("{0}h {1:D2}m", t.Hours, t.Minutes);
				}
				else if (t.Minutes > 0)
				{
					strNewValue = string.Format("{0} minute", t.Minutes);
					if (t.Minutes > 1) strNewValue = strNewValue + "s";
				}
				else
				{
					strNewValue = string.Format("{0} seconds", t.Seconds);
				}
				
				if (etaItem.Text != strNewValue)
				{
					etaItem.Text = strNewValue;
				}
			}
		

	
		}
    }// END RefreshListView



	public void RefreshGeneralTabPage(int torrentId, DllInterface.TorrentMetaData meta)
	{
		float totalFilesSize = ((float)meta.mTotalPieces * (float)meta.mPieceSize) / (1024.0f * 1024.0f);
		float totalHaveSize = ((float)meta.mPiecesDownloaded * (float)meta.mPieceSize) / (1024.0f * 1024.0f);
		float onePercentOfTorrent = totalFilesSize / 100.0f;

		mGeneralTabLabelShareRatio.Text = String.Format("{0:0.00}", totalHaveSize / onePercentOfTorrent / 100.0f);


		// remaining
		if (meta.mDownloadSpeed == 0)
		{
			char chInfinity = (char)0x221E;
			mGeneralTabLabelRemaining.Text = String.Format("{0}", chInfinity);
		}
		else
		{
			UInt32 hoursRemaining = (UInt32)(meta.mEta / (60 * 60));
			UInt32 minutesRemaining = (UInt32)((meta.mEta - (hoursRemaining * (60 * 60))) / 60);
			UInt32 secondsRemaining = (UInt32)((meta.mEta - (hoursRemaining * (60 * 60))) - (minutesRemaining * 60));
			mGeneralTabLabelRemaining.Text = String.Format("{0:0}h {1:00}m", hoursRemaining, minutesRemaining);
		}


		// time elapsed
		UInt32 hoursRunning   = (UInt32) (meta.mTimeSinceStarted / (60 * 60));
		UInt32 minutesRunning = (UInt32) ((meta.mTimeSinceStarted - (hoursRunning * (60 * 60))) / 60);
		UInt32 secondsRunning = (UInt32)((meta.mTimeSinceStarted - (hoursRunning * (60 * 60))) - (minutesRunning * 60));
		mGeneralTabLabelTimeElapsed.Text = String.Format("{0:0}h {1:00}m", hoursRunning, minutesRunning);  //"1h 31m 22s";


		mGeneralTabLabelDownloaded.Text = String.Format("{0:0.00} MB", totalHaveSize);
		mGeneralTabLabelLocalName.Text = meta.mFileName;

		DateTime unixEpoch = new DateTime(1970, 1, 1);
		unixEpoch = unixEpoch.AddSeconds(meta.mCreationDate);
		mGeneralTabLabelCreatedOn.Text = unixEpoch.ToString("D");

		mGeneralTabLabelTotalSize.Text = String.Format("{0:0.00} MB ({1:0.00} MB done)", totalFilesSize, totalHaveSize); 
		mGeneralTabLabelPieces.Text = String.Format("{0} x {1:0.00} MB (have {2})", meta.mTotalPieces, meta.mPieceSize/(1024.0f*1024.0f), meta.mPiecesDownloaded);

		String infohash = "";
		for (int i = 0, j = 0; i < meta.mInfoHash.Length; i++, j++)
		{
			infohash += String.Format("{0:X2}", meta.mInfoHash[i]);

			if (j == 3)
			{
				j=-1;
				infohash += " ";
			}
		}
		mGeneralTabLabelHash.Text = infohash;
		
		mGeneralTabLabelComment.Text = meta.mComment;
	}// END RefreshGeneralTabPage



	public ListViewItem FindPeerInPeerListView(SockAddr peerSockAddr)
	{
		for (Int32 i = 0; i < mPeersListView.Items.Count; i++)
		{
			SockAddr sockAddr = (SockAddr)mPeersListView.Items[i].Tag;
			if (sockAddr == peerSockAddr)
			{
				return mPeersListView.Items[i];
			}
		}
		return null;
	}// END FindPeerInPeerListView



	public void RefreshPeersTab(TorrentHandle torrentId)
	{
		UInt32 numConnectedPeers = DllInterface.NumberOfConnectedPeers(torrentId);

		if (numConnectedPeers == 0)
		{
			return;
		}

		// this approach allocates space for the data here, UNMANAGED needs freeing
		IntPtr[] ps = new IntPtr[numConnectedPeers];
		for (int n = 0; n < numConnectedPeers; n++)
		{
			ps[n] = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(DllInterface.sPeerInfo)));
		}

		UInt32 numWritten = DllInterface.GetConnectedPeersInfo(torrentId, ps, numConnectedPeers);

		DllInterface.sPeerInfo[] peersInfo = new DllInterface.sPeerInfo[numConnectedPeers];
		for (int i = 0; i < numConnectedPeers; i++)
		{
			peersInfo[i] = (DllInterface.sPeerInfo)Marshal.PtrToStructure(ps[i], typeof(DllInterface.sPeerInfo));
		}


		/*
		// this approach relies on the called function allocating space for the data
		IntPtr ptrArray = IntPtr.Zero;
		int count = DllInterface.ArrayTest(ref ptrArray);
		DllInterface.sPeerInfo sAddr = (DllInterface.sPeerInfo)Marshal.PtrToStructure(ptrArray, typeof(DllInterface.sPeerInfo));
		for (int i = 0; i < count; i++)
		{
			IntPtr nextPtr = new IntPtr(ptrArray.ToInt32() + i * Marshal.SizeOf(sAddr));
			sAddr = (DllInterface.sPeerInfo)Marshal.PtrToStructure(nextPtr, typeof(DllInterface.sPeerInfo));

			Console.WriteLine("{0} {1}", sAddr.addr, sAddr.port);
		}
		*/


		// remove peers we are no longer connected to
		for (Int32 i = 0; i < mPeersListView.Items.Count; i++)
		{
			SockAddr listId = (SockAddr)mPeersListView.Items[i].Tag;

			bool found = false;
			for (UInt32 j = 0; j < numConnectedPeers; j++)
			{
				SockAddr peerId = new SockAddr(peersInfo[j].mIp, peersInfo[j].mPort);
				if(listId == peerId)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				mPeersListView.Items.RemoveAt(i);
				i = -1;
			}
		}

		
		for (UInt32 i = 0; i < numConnectedPeers; i++)
		{
			if(peersInfo[i].mHandshakeRecvd == false)
			{
				continue;
			}

			// keep a copy of the ip & port as the id					
			SockAddr peersSockAddr = new SockAddr(peersInfo[i].mIp, peersInfo[i].mPort);

			ListViewItem listViewItem = FindPeerInPeerListView(peersSockAddr);
			ListViewItem.ListViewSubItem peerIdItem;
			ListViewItem.ListViewSubItem percentageItem;
			ListViewItem.ListViewSubItem downSpeedItem;
			ListViewItem.ListViewSubItem upSpeedItem;
			ListViewItem.ListViewSubItem reqsItem;
			ListViewItem.ListViewSubItem flagsItem;
			ListViewItem.ListViewSubItem connectionTypeItem;
			ListViewItem.ListViewSubItem totalDownloadedItem;
			ListViewItem.ListViewSubItem totalUploadedItem;
			ListViewItem.ListViewSubItem connectionLengthItem;

			if (listViewItem == null)
			{
				// create the new item and pass the ip string
				listViewItem = new System.Windows.Forms.ListViewItem(String.Format("{0}.{1}.{2}.{3} : {4}", peersInfo[i].mIp[0], peersInfo[i].mIp[1], peersInfo[i].mIp[2], peersInfo[i].mIp[3], peersInfo[i].mPort));

				// peer id				
				peerIdItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(peerIdItem);

				// % done
				percentageItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(percentageItem);
				

				// dl
				downSpeedItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(downSpeedItem);

				// ul
				upSpeedItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(upSpeedItem);

				// reqs
				reqsItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(reqsItem);

				// flags
				flagsItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(flagsItem);

				// connection type
				connectionTypeItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(connectionTypeItem);

				// total downloaded
				totalDownloadedItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(totalDownloadedItem);

				// total uploaded
				totalUploadedItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(totalUploadedItem);

				// Connection Length
				connectionLengthItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(connectionLengthItem);
				

				listViewItem.Tag = peersSockAddr;

				mPeersListView.Items.AddRange(new System.Windows.Forms.ListViewItem[] { listViewItem });
			}
			else
			{
				peerIdItem = listViewItem.SubItems[1];
				percentageItem = listViewItem.SubItems[2];
				downSpeedItem = listViewItem.SubItems[3];
				upSpeedItem = listViewItem.SubItems[4];
				reqsItem = listViewItem.SubItems[5];
				flagsItem = listViewItem.SubItems[6];
				connectionTypeItem = listViewItem.SubItems[7];
				totalDownloadedItem = listViewItem.SubItems[8];
				totalUploadedItem = listViewItem.SubItems[9];
				connectionLengthItem = listViewItem.SubItems[10];
			}

			String strNewValue;

			// peer id				
			System.Text.ASCIIEncoding ae = new System.Text.ASCIIEncoding();
			String strPeerId = ae.GetString(peersInfo[i].mPeerId);
			if(peerIdItem.Text != strPeerId)
			{
				peerIdItem.Text = strPeerId;
			}


			// % done
			strNewValue = String.Format("{0:0.0}%", peersInfo[i].mPercentageDone);
			if (percentageItem.Text != strNewValue)
			{
				percentageItem.Text = strNewValue;
			}

			// dl
			float speed = ((float)peersInfo[i].mDlRate / 1024.0f);
			strNewValue = String.Format("{0:0.0} kB/s", speed);
			if(downSpeedItem.Text != strNewValue)
			{
				downSpeedItem.Text = strNewValue;
			}

			// ul
			speed = ((float)peersInfo[i].mUlRate / 1024.0f);
			strNewValue = String.Format("{0:0.0} kB/s", speed);
			if (upSpeedItem.Text != strNewValue)
			{
				upSpeedItem.Text = strNewValue;
			}

			// reqs
			strNewValue = String.Format("{0} | {1}", peersInfo[i].mOustandingDownloadRequests, peersInfo[i].mOustandingUploadRequests);
			if(reqsItem.Text != strNewValue)
			{
				reqsItem.Text = strNewValue;
			}

			// Flags
			String strFlags = "";
			if (peersInfo[i].mAmChoking == 1)
			{
				strFlags = strFlags + "C";
			}
			if (peersInfo[i].mIsChokingMe == 1)
			{
				strFlags = strFlags + "c";
			}
			if (peersInfo[i].mAmInterested == 1)
			{
				strFlags = strFlags + "I";
			}
			if (peersInfo[i].mIsInterestedInMe == 1)
			{
				strFlags = strFlags + "i";
			}			
			if (flagsItem.Text != strFlags)
			{
				flagsItem.Text = strFlags;
			}
			

			// Connection type	
			if ((peersInfo[i].mConnectionFlags & (byte) DllInterface.sPeerInfo.ConnectionFlag.INCOMING_CONNECTION) != 0)
			{
				strNewValue = "Incoming";
			}
			else
			{
				strNewValue = "Outgoing";
			}
			if ((peersInfo[i].mConnectionFlags & (byte) DllInterface.sPeerInfo.ConnectionFlag.ENCRYPTED_CONNECTION) != 0)
			{
				strNewValue += " Encrypted";
			}
			else
			{
				strNewValue += " Unencrypted";
			}
			if (connectionTypeItem.Text != strNewValue)
			{
				connectionTypeItem.Text = strNewValue;
			}


			// total downloaded
			strNewValue = FormatBytes(peersInfo[i].mTotalBytesDownloaded);
			if (totalDownloadedItem.Text != strNewValue)
			{
				totalDownloadedItem.Text = strNewValue;
			}

			// total uploaded
			strNewValue = FormatBytes(peersInfo[i].mTotalBytesUploaded);
			if (totalUploadedItem.Text != strNewValue)
			{
				totalUploadedItem.Text = strNewValue;
			}


			// Connection Length
			TimeSpan t = TimeSpan.FromSeconds(peersInfo[i].mConnectionLengthInSeconds);
			if (t.Days > 0)
			{
				strNewValue = string.Format("{0}d {1:D2}h {2:D2}m", t.Days, t.Hours, t.Minutes);
			}
			else if (t.Hours > 0)
			{
				strNewValue = string.Format("{0}h {1:D2}m", t.Hours, t.Minutes);
			}
			else if (t.Minutes > 0)
			{
				strNewValue = string.Format("{0} minute", t.Minutes);
				if (t.Minutes > 1) strNewValue = strNewValue + "s";
			}
			else
			{
				strNewValue = string.Format("{0} seconds", t.Seconds);
			}

			if (connectionLengthItem.Text != strNewValue)
			{
				connectionLengthItem.Text = strNewValue;
			}
		}


		// free the unmanaged memory
		for (int n = 0; n < ps.Length; n++)
		{
			Marshal.FreeHGlobal(ps[n]);
		}
	}// END RefreshPeersTab



	public ListViewItem FindAnnounceTargetInTrackerListView(String url)
	{
		for (Int32 i = 0; i < mTrackerListView.Items.Count; i++)
		{
			String storedString = (String) mTrackerListView.Items[i].Tag;
			if (url == storedString)
			{
				return mTrackerListView.Items[i];
			}
		}
		return null;
	}// END FindAnnounceTargetInTrackerListView



	public void RefreshTrackerTab(TorrentHandle torrentId)
	{
		UInt32 numAnnounceTargets = DllInterface.NumberOfAnnounceTargets(torrentId);
		if (numAnnounceTargets == 0)
		{
			mTrackerListView.Items.Clear();
			return;
		}

		// this approach allocates space for the data here, UNMANAGED needs freeing
		IntPtr[] ps = new IntPtr[numAnnounceTargets];
		for (int n = 0; n < numAnnounceTargets; n++)
		{
			ps[n] = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(DllInterface.sAnnounceInfo)));
		}

		UInt32 numWritten = DllInterface.GetAnnounceTargetsInfo(torrentId, ps, numAnnounceTargets);

		DllInterface.sAnnounceInfo[] announceTargetsInfo = new DllInterface.sAnnounceInfo[numAnnounceTargets];
		for (int i = 0; i < numAnnounceTargets; i++)
		{
			announceTargetsInfo[i] = (DllInterface.sAnnounceInfo)Marshal.PtrToStructure(ps[i], typeof(DllInterface.sAnnounceInfo));
		}


		// remove peers we are no longer connected to
		for (Int32 i = 0; i < mTrackerListView.Items.Count; i++)
		{
			String urlTag = (string) mTrackerListView.Items[i].Tag;

			bool found = false;
			for (UInt32 j = 0; j < numAnnounceTargets; j++)
			{
				if (urlTag == announceTargetsInfo[j].mUrl)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				mTrackerListView.Items.RemoveAt(i);
				i = -1;
			}
		}


		for (UInt32 i = 0; i < numAnnounceTargets; i++)
		{
			ListViewItem listViewItem = FindAnnounceTargetInTrackerListView(announceTargetsInfo[i].mUrl);
			
			ListViewItem.ListViewSubItem urlItem;
			ListViewItem.ListViewSubItem updateInItem;
			ListViewItem.ListViewSubItem seedsItem;
			ListViewItem.ListViewSubItem peersItem;
			ListViewItem.ListViewSubItem intervalItem;

			if (listViewItem == null)
			{
				// create the new item and pass the url string
				listViewItem = new System.Windows.Forms.ListViewItem(String.Format("{0}", announceTargetsInfo[i].mUrl));

				// Url		
				//urlItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				//listViewItem.SubItems.Add(urlItem);

				// Update In
				updateInItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(updateInItem);

				// Seeds
				seedsItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(seedsItem);

				// Peers
				peersItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(peersItem);

				// Interval
				intervalItem = new ListViewItem.ListViewSubItem(listViewItem, "");
				listViewItem.SubItems.Add(intervalItem);

				listViewItem.Tag = announceTargetsInfo[i].mUrl;

				mTrackerListView.Items.AddRange(new System.Windows.Forms.ListViewItem[] { listViewItem });
			}
			else
			{
				urlItem = listViewItem.SubItems[0];
				updateInItem = listViewItem.SubItems[1];
				seedsItem = listViewItem.SubItems[2];
				peersItem = listViewItem.SubItems[3];
				intervalItem = listViewItem.SubItems[4];
			}

			

			String str;

			// Update In
			UInt32 minutes = (announceTargetsInfo[i].mNextUpdateMs / 1000 / 60);
			switch(minutes)
			{
			case 0:
				str = String.Format("Updating ...", minutes);
				break;

			default:
				str = String.Format("{0}m", minutes);
				break;
			}
			if (updateInItem.Text != str)
			{
				updateInItem.Text = str;
			}

			// Seeds
			str = String.Format("{0}", 0);
			if (seedsItem.Text != str)
			{
				seedsItem.Text = str;
			}

			// Peers
			str = String.Format("{0}", announceTargetsInfo[i].mNumberOfPeersFound);
			if (peersItem.Text != str)
			{
				peersItem.Text = str;
			}

			// Interval
			minutes = (announceTargetsInfo[i].mAnnounceInterval / 1000 / 60);
			str = String.Format("{0}m", minutes);
			if (intervalItem.Text != str)
			{
				intervalItem.Text = str;
			}
		}

		// free the unmanaged memory
		for (int n = 0; n < ps.Length; n++)
		{
			Marshal.FreeHGlobal(ps[n]);
		}
	}// END RefreshTrackerTab



	public ListViewItem FindPieceInListView(UInt32 pieceNum)
	{
		for (Int32 i = 0; i < mPiecesListView.Items.Count; i++)
		{
			UInt32 storedpieceNum = (UInt32)mPiecesListView.Items[i].Tag;
			if (pieceNum == storedpieceNum)
			{
				return mPiecesListView.Items[i];
			}
		}
		return null;
	}// END FindPieceInListView


	public void RefreshPiecesTab(TorrentHandle torrentId)
	{
		UInt32 numPieces = DllInterface.NumberOfActivePieces(torrentId);
		if (numPieces == 0)
		{
			mPiecesListView.Items.Clear();
			return;
		}

		// this approach allocates space for the data here, UNMANAGED needs freeing
		IntPtr[] ps = new IntPtr[numPieces];
		for (int n = 0; n < numPieces; n++)
		{
			ps[n] = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(DllInterface.sPiecesInfo)));
		}

		UInt32 numWritten = DllInterface.GetActivePiecesInfo(torrentId, ps, numPieces);

		DllInterface.sPiecesInfo[] piecesInfo = new DllInterface.sPiecesInfo[numPieces];
		for (int i = 0; i < numPieces; i++)
		{
			piecesInfo[i] = (DllInterface.sPiecesInfo)Marshal.PtrToStructure(ps[i], typeof(DllInterface.sPiecesInfo));
		}


		// Remove pieces we are no longer actively downloading
		for (Int32 i = 0; i < mPiecesListView.Items.Count; i++)
		{
			UInt32 pieceNumber = (UInt32)mPiecesListView.Items[i].Tag;

			bool found = false;
			for (UInt32 j = 0; j < numPieces; j++)
			{
				UInt32 storedPieceNum = piecesInfo[j].mPieceNumber;
				if(storedPieceNum == pieceNumber)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				mPiecesListView.Items.RemoveAt(i);
				i = -1;
			}
		}

		
		for (UInt32 i = 0; i < numPieces; i++)
		{
			// keep a copy of the piece number to use as the id					
			UInt32 storedPieceNum = piecesInfo[i].mPieceNumber;

			ListViewItem listViewItem = FindPieceInListView(storedPieceNum);
			ListViewItem.ListViewSubItem pieceNumItem;
			ListViewItem.ListViewSubItem sizeItem;
			ListViewItem.ListViewSubItem numBlocksItem;
			ListViewItem.ListViewSubItem requestedBlocksItem;
			ListViewItem.ListViewSubItem completedBlocksItem;

			if (listViewItem == null)
			{
				// create the new item and pass the id string
				listViewItem = new System.Windows.Forms.ListViewItem(String.Format("{0}", storedPieceNum));

				// size
				float size = (float)piecesInfo[i].mPieceSize / (1024.0f * 1024.0f);
				sizeItem = new ListViewItem.ListViewSubItem(listViewItem, String.Format("{0:0.0} MB", size));
				listViewItem.SubItems.Add(sizeItem);

				// num blocks
				numBlocksItem = new ListViewItem.ListViewSubItem(listViewItem, String.Format("{0}", piecesInfo[i].mNumberOfBlocks));
				listViewItem.SubItems.Add(numBlocksItem);
				
				// requested blocks
				requestedBlocksItem = new ListViewItem.ListViewSubItem(listViewItem, String.Format("{0}", piecesInfo[i].mRequestedBlocks));
				listViewItem.SubItems.Add(requestedBlocksItem);

				// completed blocks
				completedBlocksItem = new ListViewItem.ListViewSubItem(listViewItem, String.Format("{0}", piecesInfo[i].mCompletedBlocks));
				listViewItem.SubItems.Add(completedBlocksItem);

				listViewItem.Tag = storedPieceNum;

				mPiecesListView.Items.AddRange(new System.Windows.Forms.ListViewItem[] { listViewItem });
			}
			else
			{
				pieceNumItem = listViewItem.SubItems[0];
				sizeItem = listViewItem.SubItems[1];
				numBlocksItem = listViewItem.SubItems[2];
				requestedBlocksItem = listViewItem.SubItems[3];
				completedBlocksItem = listViewItem.SubItems[4];
			}

			String strNewValue;


			// requested blocks	
			strNewValue = String.Format("{0}", piecesInfo[i].mRequestedBlocks);
			if (requestedBlocksItem.Text != strNewValue)
			{
				requestedBlocksItem.Text = strNewValue;
			}


			// completedBlocksItem	
			strNewValue = String.Format("{0}", piecesInfo[i].mCompletedBlocks);
			if (completedBlocksItem.Text != strNewValue)
			{
				completedBlocksItem.Text = strNewValue;
			}
		}



		// free the unmanaged memory
		for (int n = 0; n < ps.Length; n++)
		{
			Marshal.FreeHGlobal(ps[n]);
		}
	}// END RefreshPiecesTab




	public ListViewItem FindFileInListView(String filename)
	{
		for (Int32 i = 0; i < mFilesListView.Items.Count; i++)
		{
			String storedFilename = (String)mFilesListView.Items[i].Tag;
			if (filename == storedFilename)
			{
				return mFilesListView.Items[i];
			}
		}
		return null;
	}// END FindPieceInListView



	public void RefreshFileTab(TorrentHandle torrentId)
	{
		UInt32 numFiles = DllInterface.NumberOfFilesInTorrent(torrentId);
		if (numFiles == 0)
		{
			//mFilesListView.Items.Clear();
			return;
		}

		// this approach allocates space for the data here, UNMANAGED needs freeing
		IntPtr[] ps = new IntPtr[numFiles];
		for (int n = 0; n < numFiles; n++)
		{
			ps[n] = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(DllInterface.sFileInfo)));
		}

		UInt32 numWritten = DllInterface.GetTorrentFilesInfo(torrentId, ps, numFiles);

		DllInterface.sFileInfo[] fileInfo = new DllInterface.sFileInfo[numFiles];
		for (int i = 0; i < numFiles; i++)
		{
			fileInfo[i] = (DllInterface.sFileInfo)Marshal.PtrToStructure(ps[i], typeof(DllInterface.sFileInfo));
		}



		// Remove file we are no longer actively downloading
		for (Int32 i = 0; i < mFilesListView.Items.Count; i++)
		{
			String filename = (String)mFilesListView.Items[i].Tag;

			bool found = false;
			for (UInt32 j = 0; j < numFiles; j++)
			{
				String storedFilename = fileInfo[j].mFilename;
				if (storedFilename == filename)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				mFilesListView.Items.RemoveAt(i);
				i = -1;
			}
		}


		for (UInt32 i = 0; i < numFiles; i++)
		{
			// keep a copy of the file name to use as the id					
			String storedfilename = fileInfo[i].mFilename;

			ListViewItem listViewItem = FindFileInListView(storedfilename);
			ListViewItem.ListViewSubItem nameItem;
			ListViewItem.ListViewSubItem sizeItem;
			ListViewItem.ListViewSubItem numPiecesItem;
			ListViewItem.ListViewSubItem percentageDoneItem;

			if (listViewItem == null)
			{
				// create the new item and pass the name string
				listViewItem = new System.Windows.Forms.ListViewItem(String.Format("{0}", storedfilename));

				// size
				float size = (float)fileInfo[i].mSize / (1024.0f * 1024.0f);
				sizeItem = new ListViewItem.ListViewSubItem(listViewItem, String.Format("{0:0.00} MB", size));
				listViewItem.SubItems.Add(sizeItem);

				// num pieces
				numPiecesItem = new ListViewItem.ListViewSubItem(listViewItem, String.Format("{0}", fileInfo[i].mNumberOfPieces));
				listViewItem.SubItems.Add(numPiecesItem);

				// % done
				percentageDoneItem = new ListViewItem.ListViewSubItem(listViewItem, String.Format("{0:0.0}%", fileInfo[i].mPercentageComplete));
				listViewItem.SubItems.Add(percentageDoneItem);

				listViewItem.Tag = storedfilename;

				mFilesListView.Items.AddRange(new System.Windows.Forms.ListViewItem[] { listViewItem });
			}
			else
			{
				nameItem = listViewItem.SubItems[0];
				sizeItem = listViewItem.SubItems[1];
				numPiecesItem = listViewItem.SubItems[2];
				percentageDoneItem = listViewItem.SubItems[3];
			}

			String strNewValue;


			// percentageDoneItem	
			strNewValue = String.Format("{0:0.0}%", fileInfo[i].mPercentageComplete);
			if (percentageDoneItem.Text != strNewValue)
			{
				percentageDoneItem.Text = strNewValue;
			}
		}


		// free the unmanaged memory
		for (int n = 0; n < ps.Length; n++)
		{
			Marshal.FreeHGlobal(ps[n]);
		}
	}// END RefreshFileTab
}

