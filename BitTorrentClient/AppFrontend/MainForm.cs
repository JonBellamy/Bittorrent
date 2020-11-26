using System;
using System.Net;
using System.Threading;
using System.IO;
using System.Windows.Forms;
//using Microsoft.VisualBasic.FileIO;
using System.Diagnostics;
using System.Runtime.InteropServices;


using TorrentHandle = System.Int32;


public partial class MainForm : Form
{
	//////////////////////////////////////////////////////////////////////////
	// Data members

#if DEBUG
	private static ConsoleOutputWindow mOutputWnd;
#endif

    private bool versionChecked;

	static System.Windows.Forms.Timer mUiTimer = new System.Windows.Forms.Timer();
	private ListViewColumnSorter lvwColumnSorter;

	static System.Windows.Forms.Timer mCoreUpdateTimer = new System.Windows.Forms.Timer();

	public static TorrentHandle mSelectedTorrentId;

	public static BandwidthGraph mBandwidthGraph;
	public static BitTorrentManager torrentManager;

    private System.ComponentModel.BackgroundWorker backgroundWorker;

	// Bit Torrent Flags, this needs to be kept in sync with the bittorrentvalues.h enum, hardly ideal
	enum BtEventFlags
	{
		BT_COMPLETE = 1 << 0,
		NUM_EVENT_FLAGS = 1
	};


	private static System.Object mThreadLock = new System.Object();
	
	// used to call our event from the main thread
	private readonly SynchronizationContext mSyncContext;

	private static DllInterface.DebugString_Delegate debugStringDelegate;

	public enum ColumnId
	{
		COLUMN_NAME = 0,
		COLUMN_ID,
		COLUMN_DOWNSPEED, 
		COLUMN_UPSPEED,
		COLUMN_ETA		
	};



    public MainForm()
    {
        mSelectedTorrentId = (TorrentHandle) Torrent.TorrentValues.INVALID_TORRENT;
        versionChecked = false;

		mSyncContext = System.ComponentModel.AsyncOperationManager.SynchronizationContext;
        InitializeComponent();

#if DEBUG
		mOutputWnd = new ConsoleOutputWindow();
		mOutputWnd.AddString("App Startup\n");
#endif

		debugStringDelegate = new DllInterface.DebugString_Delegate(DebugStringCallback);
		DllInterface.SetDebugStringOutputCb(debugStringDelegate);

		torrentManager = new BitTorrentManager();
		DllInterface.InitTorrentManager(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData));

		// Create an instance of a ListView column sorter and assign it to the ListView control.
		lvwColumnSorter = new ListViewColumnSorter();
		this.mPeersListView.ListViewItemSorter = lvwColumnSorter;
    }// END MainForm


	private static void DebugStringCallback(String s)
	{
#if DEBUG
		lock (mThreadLock)
		{
			mOutputWnd.AddString(s);
		}
#endif
	}

    private delegate void InvokeDelegate(String version);
    private void ShowNewVersionDialog(String version)
    {
        // Marshall the call to the ui thread
        if (InvokeRequired)
        {
            object[] o = new object[] { version };
            InvokeDelegate d = new InvokeDelegate(ShowNewVersionDialog);
            Invoke(d, o);
        }
        else
        {
            DllInterface.sTorrentClientOptions options = new DllInterface.sTorrentClientOptions();
            DllInterface.GetTorrentClientOptions(ref options);

            if (options.mCheckForLatestBuild != 0 &&
                Application.ProductVersion.Equals(version) == false)
            {
                AppFrontend.AboutBox dialog = new AppFrontend.AboutBox();
                dialog.ShowDialog(AppFrontend.AboutBox.AboutMode.NewBuildAvailable);
            }
        }
    }

/*
	private void DownloadVersionStringCompleted(object sender, DownloadStringCompletedEventArgs e)
    {
        //String str ="Result: " + e.Result + " State:" + e.UserState.ToString() + "\r\n";
		if (e.Error == null)
		{
			String version = e.Result;
            ShowNewVersionDialog(version);
		}
    }
*/

    private void DownloadVersionString(object sender, System.ComponentModel.DoWorkEventArgs e)
    {
        if (versionChecked == false)
        {
            versionChecked = true;
            WebClient wc = new WebClient();
            //wc.DownloadStringCompleted += new DownloadStringCompletedEventHandler(DownloadVersionStringCompleted);
            //wc.DownloadStringAsync(new Uri("http://themeerkat.net/version.txt"));
            String version = wc.DownloadString(new Uri("http://themeerkat.net/version.txt"));
            ShowNewVersionDialog(version);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Overrides

    protected override void OnCreateControl()
    {
		base.OnCreateControl();
        

		mUiTimer.Tick += new EventHandler(TimerEventProcessor);
		mUiTimer.Interval = 1000;
		mUiTimer.Start();

		mCoreUpdateTimer.Tick += new EventHandler(CoreProcessEventProcessor);
		mCoreUpdateTimer.Interval = 10;
		mCoreUpdateTimer.Start();
			
		// system tray icon
		mAppNotifyIcon.Visible = true;
		
#if DEBUG
		mTabPageOutput.Controls.Add(mOutputWnd);
		mOutputWnd.Dock = System.Windows.Forms.DockStyle.Fill;
		//mOutputWnd.SetTextColour(255, 0, 0, 255);
#else
        mAppTabControl.TabPages.Remove(mTabPageOutput);
#endif

		// Grab the latest version number from the web server
#if !DEBUG
        backgroundWorker = new System.ComponentModel.BackgroundWorker();
        backgroundWorker.DoWork += new System.ComponentModel.DoWorkEventHandler(DownloadVersionString);
        backgroundWorker.RunWorkerAsync();
#endif
	}// END OnCreateControl


/*
	void TorrentWorkerThread()
	{
		while (true)
		{
			// Wait until it is safe to enter.
			//mut.WaitOne();

			lock (mThreadLock)
			{
				DllInterface.UpdateTorrentManager();
			}
			Thread.Sleep(5);

			// Release the Mutex.
			//mut.ReleaseMutex();
		}
	}
*/

	public string FormatBytes(Int64 bytes)
	{
		const int scale = 1024;
		string[] orders = new string[] { "GB", "MB", "KB", "Bytes" };
		long max = (long)Math.Pow(scale, orders.Length - 1); foreach (string order in orders)
		{
			if (bytes > max)
				return string.Format("{0:##.##} {1}", decimal.Divide(bytes, max), order); max /= scale;
		}
		return "0 Bytes";
	}


    //////////////////////////////////////////////////////////////////////////
    // Events


	private void Application_Idle(Object sender, EventArgs e)
	{
		//DllInterface.UpdateTorrentManager();
	}// END Application_Idle


	private void RefreshView()
	{
		// Wait until it is safe to enter.
		//mut.WaitOne();

		lock (mThreadLock)
		{
			RefreshListView();
			
			mStatusStripDhtLabel.Text = String.Format("Dht Nodes: {0}", DllInterface.NumberOfDhtNodes());

			if (mListView.SelectedItems.Count == 1)
			{
				Torrent torrent = torrentManager.GetTorrent(mSelectedTorrentId);
				if(torrent != null)
				{
					if (mTabPageGeneral.Visible)
					{
						RefreshGeneralTabPage(mSelectedTorrentId, torrent.MetaData);
					}

					if (mTabPageTracker.Visible)
					{
						RefreshTrackerTab(torrent.Handle);
					}

					if (mTabPagePeers.Visible)
					{
						RefreshPeersTab(torrent.Handle);
					}

					if (mTabPagePieces.Visible)
					{
						RefreshPiecesTab(torrent.Handle);
					}

					if (mTabPageFiles.Visible)
					{
						RefreshFileTab(torrent.Handle);
					}


					//if (mTabPageBandwidth.Visible)
					//{
					//	mBandwidthGraph.Refresh(torrent);
					//}
					

					//if (mPeersListView.Visible == true)
					//{
					//	this.mPeersListView.Sort();
					//}				

					ProcessTorrentEventFlags();
				}
			}
		}
		// Release the Mutex.
		//mut.ReleaseMutex();
	}// END RefreshView


	public void ProcessTorrentEventFlags()
	{
		for(Int32 i=0; i < mListView.Items.Count; i++)
		{
			Torrent torrent = torrentManager.GetTorrent((TorrentHandle)mListView.Items[i].Tag);

			if (torrent != null)
			{
	
				for (Int32 j = 0; j < (Int32) BtEventFlags.NUM_EVENT_FLAGS; j++)
				{
					if ((torrent.MetaData.mEventFlags & (Int32)BtEventFlags.BT_COMPLETE) != 0)
					{
						ShowDownloadCompleteBalloon(torrent.MetaData.mFileName);
					}
				}
				
				//BtEventFlags
//				if(id == torrentId)
//				{
//					return mListView.Items[i];
//				}
			}
		}
	}// END ProcessTorrentEventFlags



	public void UpdateStatusStripValues()
	{
		lock (mThreadLock)
		{
			float dlSpeed = ((float)DllInterface.TotalDownloadRate() / 1024.0f);
			float ulSpeed = ((float)DllInterface.TotalUploadRate() / 1024.0f);

			mStatusStripDlLabel.Text = String.Format("D:{0:0.0}kB/s", dlSpeed);
			mStatusStripUlLabel.Text = String.Format("U:{0:0.0}kB/s", ulSpeed);

			// sort the system tray balloon tip while we are here as the BalloonShown msg does not work
			mAppNotifyIcon.Text = String.Format("D:{0:0.0}kB/s\nU:{1:0.0}kB/s", dlSpeed, ulSpeed);
		}
	}// END UpdateStatusStripValues


	public void ShowDownloadCompleteBalloon(String torrentName)
	{
		// NB the balloon tip is NOT the same thing as the tool tip!!
		if (mAppNotifyIcon.Visible)
		{
			mAppNotifyIcon.BalloonTipTitle = "Download Complete";
			//mAppNotifyIcon.BalloonTipText = "aaa.bin. Dl time 1h 31m";
			mAppNotifyIcon.BalloonTipText = torrentName;
			mAppNotifyIcon.ShowBalloonTip(5000);
		}
	}



	// This is the method to run when the timer is raised.
	private void TimerEventProcessor(Object myObject,EventArgs myEventArgs)
	{
		RefreshView();
		UpdateStatusStripValues();
	}// END TimerEventProcessor


	private void CoreProcessEventProcessor(Object myObject, EventArgs myEventArgs)
	{
		//DllInterface.UpdateTorrentManager();
	}// END CoreProcessEventProcessor



    private void MainForm_Load(object sender, EventArgs e)
    {
		mBandwidthGraph = new BandwidthGraph(mZedGraphControl);
		//mBandwidthGraph.CreateGraph();


		/*
		// Column sorter...
		ColumnHeader columnheader;		// Used for creating column headers.
		ListViewItem listviewitem;		// Used for creating listview items.

		// Ensure that the view is set to show details.
		//mListView.View = View.Details;

		// Create some listview items consisting of first and last names.
		listviewitem = new ListViewItem("John");
		listviewitem.SubItems.Add("Smith");
		this.mListView.Items.Add(listviewitem);

		listviewitem = new ListViewItem("Bob");
		listviewitem.SubItems.Add("Taylor");
		this.mListView.Items.Add(listviewitem);

		listviewitem = new ListViewItem("Kim");
		listviewitem.SubItems.Add("Zimmerman");
		this.mListView.Items.Add(listviewitem);

		listviewitem = new ListViewItem("Olivia");
		listviewitem.SubItems.Add("Johnson");
		this.mListView.Items.Add(listviewitem);

		// Create some column headers for the data. 
		columnheader = new ColumnHeader();
		columnheader.Text = "First Name";
		this.mListView.Columns.Add(columnheader);

		columnheader = new ColumnHeader();
		columnheader.Text = "Last Name";
		this.mListView.Columns.Add(columnheader);

		// Loop through and size each column header to fit the column header text.
		foreach (ColumnHeader ch in this.mListView.Columns)
		{
			ch.Width = -2;
		}
		*/

	}// END MainForm_Load



    private void mListView_SelectedIndexChanged(object sender, EventArgs e)
    {
		
		if(mListView.SelectedItems.Count == 0)
		{
			mSelectedTorrentId = (TorrentHandle) Torrent.TorrentValues.INVALID_TORRENT;

			mBandwidthGraph.SetCurrentTorrent(null);
			
			mPeersListView.Items.Clear();

			mGeneralTabLabelTimeElapsed.Text = "";
			mGeneralTabLabelShareRatio.Text = "";
			mGeneralTabLabelRemaining.Text = "";
			mGeneralTabLabelDownloaded.Text = "";
			mGeneralTabLabelComment.Text = "";
			mGeneralTabLabelHash.Text = "";
			mGeneralTabLabelPieces.Text = "";
			mGeneralTabLabelTotalSize.Text = "";
			mGeneralTabLabelCreatedOn.Text = "";
			mGeneralTabLabelLocalName.Text = "";

			return;
		}

		mSelectedTorrentId = (TorrentHandle) mListView.SelectedItems[0].Tag;
		
		mBandwidthGraph.SetCurrentTorrent(torrentManager.GetTorrent(mSelectedTorrentId));

		RefreshView();
	}// END mListView_SelectedIndexChanged



    private void NotifyIcon_DoubleClick(object sender, EventArgs e)
    {
        // Show the form when the user double clicks on the notify icon.

        // Set the WindowState to normal if the form is minimized.
        if (this.WindowState == FormWindowState.Minimized)
        {
            this.WindowState = FormWindowState.Normal;
            
        }

        // Activate the form.
        this.Activate();
    }// END NotifyIcon_DoubleClick



    private void MainForm_Resize(object sender, EventArgs e)
    {
        if (WindowState == FormWindowState.Minimized)
        {
			// Very strange bug where the listview items all become a copy of the first is minimized while an item is selected.
			// Clear the dam list and it will sort itself out when it updates again.
			mListView.Items.Clear();
			mSelectedTorrentId = (TorrentHandle) Torrent.TorrentValues.INVALID_TORRENT;

            // system tray icon
            mAppNotifyIcon.Visible = true;

            //for the main form
            this.ShowIcon = false;
            this.ShowInTaskbar = false;

        }
        else
        {
            //mAppNotifyIcon.Visible = false;
            this.ShowIcon = true;
            this.ShowInTaskbar = true;

        }

		
		if (mTabPageGeneral.VerticalScroll.Visible == false)
		{
			//mTabPageGeneral.AutoScroll = false;
			mTabPageGeneral.VerticalScroll.Enabled = false;
			mTabPageGeneral.VerticalScroll.Visible = true;
		}
		else
		{
			//mTabPageGeneral.AutoScroll = true;
			mTabPageGeneral.VerticalScroll.Enabled = true;			
		}
    }// END MainForm_Resize



	private void OptionsMenuItem_Click(object sender, EventArgs e) 
	{
		lock (mThreadLock)
		{
			// show the modal dialog until the OK Cancel is	pressed
			OptionsDialog optionsDialog = new OptionsDialog();
			if (optionsDialog.ShowDialog() == DialogResult.OK)
			{
				DllInterface.SetTorrentClientOptions(optionsDialog.mOptions);
			}
		}
	}// END OptionsMenuItem_Click



	private void MainForm_FormClosed(object sender,FormClosedEventArgs e) 
	{
		lock (mThreadLock)
		{
			DllInterface.SetDebugStringOutputCb(null);
			DllInterface.DeInitTorrentManager();
			mAppNotifyIcon.Dispose();
			//mTorrentWorkerThread.Abort("MK Shutdown Abort.");
		}
	}// END MainForm_FormClosed



	private void AddTorrentButton_Click(object sender, EventArgs e)
	{
		if (torrentManager.State != BitTorrentManager.BtmState.Running)
		{
			return;
		}

		OpenFileDialog openFileDialog = new OpenFileDialog();
		//openFileDialog1.InitialDirectory = "c:\\" ;
		openFileDialog.Filter = "All files (*.*)|*.*|torrent files (*.torrent)|*.torrent";
		openFileDialog.FilterIndex = 2;
		openFileDialog.RestoreDirectory = true;
		openFileDialog.Multiselect = true;

		if (openFileDialog.ShowDialog() == DialogResult.OK)
		{
			lock (mThreadLock)
			{
				for (Int32 i = 0; i < openFileDialog.FileNames.Length; i++)
				{
					FolderBrowserDialog folderDialog = new System.Windows.Forms.FolderBrowserDialog();
					folderDialog.Description = "Select where to download the torrent files.";
					// Default to the My Documents folder.
					//folderDialog.RootFolder = Environment.SpecialFolder.Personal;

					DialogResult result = folderDialog.ShowDialog();
					if (result == DialogResult.OK)
					{
						String folderName = folderDialog.SelectedPath;

						Int32 handle = DllInterface.AddTorrent(openFileDialog.FileNames[i], folderName);
						if (handle < 0)
						{
							MessageBox.Show(String.Format("The torrent: {0} is invalid", openFileDialog.SafeFileNames[i]), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
						}
					}
				}
			}
		}
	}// END AddTorrentButton_Click



	private void RemoveTorrentButton_Click(object sender, EventArgs e)
	{
		if (torrentManager.State != BitTorrentManager.BtmState.Running)
		{
			return;
		}

		lock (mThreadLock)
		{
			if (mSelectedTorrentId != (TorrentHandle) Torrent.TorrentValues.INVALID_TORRENT)
			{
				DllInterface.RemoveTorrent(mSelectedTorrentId);
			}
		}
	}// END RemoveTorrentButton_Click



	private void StartTorrentButton_Click(object sender, EventArgs e)
	{
		if (torrentManager.State != BitTorrentManager.BtmState.Running)
		{
			return;
		}

		lock (mThreadLock)
		{
			if (mSelectedTorrentId != (TorrentHandle) Torrent.TorrentValues.INVALID_TORRENT)
			{
				DllInterface.StartTorrent(mSelectedTorrentId);
			}
		}
	}// END StartTorrentButton_Click



	private void PauseTorrentButton_Click(object sender, EventArgs e)
	{
		if (torrentManager.State != BitTorrentManager.BtmState.Running)
		{
			return;
		}

		lock (mThreadLock)
		{
			if (mSelectedTorrentId != (TorrentHandle) Torrent.TorrentValues.INVALID_TORRENT)
			{
				DllInterface.PauseTorrent(mSelectedTorrentId);
			}
		}
	}// END PauseTorrentButton_Click



	private void StopTorrentButton_Click(object sender, EventArgs e)
	{
		if (torrentManager.State != BitTorrentManager.BtmState.Running)
		{
			return;
		}

		lock (mThreadLock)
		{
			if (mSelectedTorrentId != (TorrentHandle) Torrent.TorrentValues.INVALID_TORRENT)
			{
				DllInterface.StopTorrent(mSelectedTorrentId);
			}
		}
	}// END StopTorrentButton_Click



	private void RemoveTorrentToolStripMenuItem_Click(object sender, EventArgs e)
	{
		RemoveTorrentButton_Click(this, null);
	}// END RemoveTorrentToolStripMenuItem_Click



	private void RemoveAndDeleteTorrentToolStripMenuItem_Click(object sender, EventArgs e)
	{
		if (torrentManager.State != BitTorrentManager.BtmState.Running)
		{
			return;
		}

		if (MessageBox.Show("Are you sure you wish to remove the selected torrent and delete its associated .torrent file?", "Confirmation required", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2) == DialogResult.Yes)
		{
			Torrent torrent = torrentManager.GetTorrent(mSelectedTorrentId);
			if(torrent != null)
			{
				string torrentFn = torrent.MetaData.mFileName;
				string torrentMkFn = torrent.MetaData.mFileName + ".mk";

				RemoveTorrentButton_Click(this, null);

				// Move file(s) to recycle bin
				//FileSystem.DeleteFile(torrentFn, UIOption.AllDialogs, RecycleOption.SendToRecycleBin);
				//FileSystem.DeleteFile(torrentMkFn, UIOption.AllDialogs, RecycleOption.SendToRecycleBin);
				File.Delete(torrentFn);
				File.Delete(torrentMkFn);
			}
		}
	}// END RemoveAndDeleteTorrentToolStripMenuItem_Click



	private void ForceRecheckToolStripMenuItem_Click(object sender, EventArgs e)
	{
		lock (mThreadLock)
		{
			if (mSelectedTorrentId != (TorrentHandle) Torrent.TorrentValues.INVALID_TORRENT)
			{
				DllInterface.TorrentForceRecheck(mSelectedTorrentId);
			}
		}
	}// END ForceRecheckToolStripMenuItem_Click



	private void AddTorrentToolStripMenuItem_Click(object sender, EventArgs e)
	{
		AddTorrentButton_Click(this, null);
	}// END AddTorrentToolStripMenuItem_Click



	private void ExitMenu_Click(object sender, EventArgs e)
	{
		DllInterface.SetDebugStringOutputCb(null);
		DllInterface.DeInitTorrentManager();
		mAppNotifyIcon.Dispose();
		Application.Exit();
	}// END ExitMenu_Click



	private void AboutToolStripMenuItem_Click(object sender, EventArgs e)
	{
		// TODO : you cannot embed links in the properly, create a new dialog
		//MessageBox.Show(String.Format("Meerkat BitTorrent Client v{0}\r\n\r\nBuild Date: {1}\r\n\r\nFor more info visit:\r\n\r\nhttp://www.themeerkat.net", Application.ProductVersion, RetrieveLinkerTimestamp().ToString()), "About", MessageBoxButtons.OK, MessageBoxIcon.Information);

		// show the modal dialog until the OK Cancel is	pressed
		AppFrontend.AboutBox optionsDialog = new AppFrontend.AboutBox();
        optionsDialog.ShowDialog(AppFrontend.AboutBox.AboutMode.About);		
	}// END AboutToolStripMenuItem_Click



	private void ShutdownWhenAllDownloadsAreCompleteToolStripMenuItem_Click(object sender, EventArgs e)
	{
		//ExitWindowsEx(EWX_SHUTDOWN, 0);

		//System.Diagnostics.Process.Start("shutdown.exe", "-s -t 00");

	}// END ShutdownWhenAllDownloadsAreCompleteToolStripMenuItem_Click



	private void StopAllDownloadsToolStripMenuItem_Click(object sender, EventArgs e)
	{
		// TODO ...
	}// END StopAllDownloadsToolStripMenuItem_Click



	private void AllowNonEncryptedConnectionsForThisTorrentToolStripMenuItem_Click(object sender, EventArgs e)
	{
		lock (mThreadLock)
		{
			if (mSelectedTorrentId != (TorrentHandle) Torrent.TorrentValues.INVALID_TORRENT)
			{
				bool connectionsMustbeSecure = DllInterface.DoesTorrentOnlyUsesEncryptedConnections(mSelectedTorrentId);
				DllInterface.SetTorrentOnlyUsesEncryptedConnections(mSelectedTorrentId, !connectionsMustbeSecure);
			}
		}
	}// END AllowNonEncryptedConnectionsForThisTorrentToolStripMenuItem_Click



	private void mMainListViewRightClickMenu_Opening(object sender, System.ComponentModel.CancelEventArgs e)
	{
		lock (mThreadLock)
		{
			if (mSelectedTorrentId != (TorrentHandle) Torrent.TorrentValues.INVALID_TORRENT)
			{
				bool connectionsMustbeSecure = DllInterface.DoesTorrentOnlyUsesEncryptedConnections(mSelectedTorrentId);
				mMenuItemAllowUnencryptedComs.Checked = !connectionsMustbeSecure;
			}
		}
	}

	private void mPeersListView_ColumnClick(object sender, ColumnClickEventArgs e)
	{
		// Determine if clicked column is already the column that is being sorted.
		if (e.Column == lvwColumnSorter.SortColumn)
		{
			// Reverse the current sort direction for this column.
			if (lvwColumnSorter.Order == SortOrder.Ascending)
			{
				lvwColumnSorter.Order = SortOrder.Descending;
			}
			else
			{
				lvwColumnSorter.Order = SortOrder.Ascending;
			}
		}
		else
		{
			// Set the column number that is to be sorted; default to ascending.
			lvwColumnSorter.SortColumn = e.Column;
			lvwColumnSorter.Order = SortOrder.Ascending;
		}

		// Perform the sort with these new sort options.
		this.mPeersListView.Sort();

	}

	private void openContainingFolderToolStripMenuItem_Click(object sender, EventArgs e)
	{
		lock (mThreadLock)
		{
			if (mListView.SelectedItems.Count == 1)
			{
				Torrent torrent = torrentManager.GetTorrent(mSelectedTorrentId);
				if(torrent != null)
				{
					string folderpath = torrent.MetaData.mTargetFolder;
					string windir = Environment.GetEnvironmentVariable("WINDIR");
					System.Diagnostics.Process prc = new System.Diagnostics.Process();
					prc.StartInfo.FileName = windir + @"\explorer.exe";
					prc.StartInfo.Arguments = folderpath;
					prc.Start();
				}
			}
		}
	}



}

