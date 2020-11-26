using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Diagnostics;

using ZedGraph;

using TorrentHandle = System.Int32;



public class BitTorrentManager
{
	private System.Windows.Forms.Timer oneSecondTimer;
	private System.Windows.Forms.Timer oneMinuteTimer;

	List<Torrent> torrentList;
	
	public Int32 TorrentCount { get { return torrentList.Count; } }
		
	private AddTorrentAdded_Delegate torrentAddedDelegate;
	private AddTorrentRemoved_Delegate torrentRemovedDelegate;


	private PointPairList downloadGraphData;
	public PointPairList DownloadGraphData { get { return downloadGraphData; } }

	private PointPairList uploadGraphData;
	public PointPairList UploadGraphData { get { return uploadGraphData; } }

	public enum BtmState
	{
		Stopped = 0,
		Initializing,
		Running
	}
	public BtmState State { get { return BitTorrentManagerState(); } }

	public BitTorrentManager()
	{
		torrentList = new List<Torrent>();

		downloadGraphData = new PointPairList();
		uploadGraphData = new PointPairList();
		
		// Dll callbacks
		torrentAddedDelegate = new AddTorrentAdded_Delegate(TorrentAddedCallback);
		torrentRemovedDelegate = new AddTorrentRemoved_Delegate(TorrentRemovedCallback);
		AddTorrentAddedCallback(torrentAddedDelegate);
		AddTorrentRemovedCallback(torrentRemovedDelegate);

		oneSecondTimer = new System.Windows.Forms.Timer();
		oneSecondTimer.Tick += new EventHandler(TimerEventOneSecond);
		oneSecondTimer.Interval = 1000;
		oneSecondTimer.Start();


		oneMinuteTimer = new System.Windows.Forms.Timer();
		oneMinuteTimer.Tick += new EventHandler(TimerEventOneMinute);
		oneMinuteTimer.Interval = 1000 * 60;
		oneMinuteTimer.Start();	
	}


	private void TimerEventOneSecond(Object myObject, EventArgs myEventArgs)
	{
		if(State != BtmState.Running)
		{
			return;
		}

		DllInterface.TorrentHandles handles = new DllInterface.TorrentHandles();
		DllInterface.GetAllTorrentHandles(ref  handles);


		for (Int32 i = 0; i < handles.mNumHandles; i++)
		{
			TorrentHandle handle = handles.mHandles[i];


			// THREADING ISSUE???!!?!??!?!

			DllInterface.TorrentMetaData meta = new DllInterface.TorrentMetaData();
			DllInterface.GetTorrentMetaData(handle, ref meta);

			Torrent torrent = GetTorrent(handle);
			if(torrent != null)
			{
				torrent.MetaData = meta;
			}
		}
	}


	private void TimerEventOneMinute(Object myObject, EventArgs myEventArgs)
	{
		if (State != BtmState.Running)
		{
			return;
		}

		foreach (Torrent torrent in torrentList)
		{
			torrent.SubmitGraphData((TimeTorrentManagerRunning() / (60000)), (torrent.MetaData.mDownloadSpeed / 1024), (TimeTorrentManagerRunning() / 60000), (torrent.MetaData.mUploadSpeed / 1024));
		}


		float dlSpeed = ((float)DllInterface.TotalDownloadRate() / 1024.0f);
		float ulSpeed = ((float)DllInterface.TotalUploadRate() / 1024.0f);

		downloadGraphData.Add(TimeTorrentManagerRunning() / 60000, dlSpeed);
		uploadGraphData.Add(TimeTorrentManagerRunning() / 60000, ulSpeed);

		if(MainForm.mBandwidthGraph != null)
		{
			MainForm.mBandwidthGraph.UpdateCurves();
		}
	}
	

	private bool StoreTorrent(TorrentHandle handle)
	{
		if (GetTorrent(handle) != null)
		{
			Debug.Assert(false);
			return false;
		}

		Torrent torrent = new Torrent(handle);
		torrentList.Add(torrent);
		return true;
	}
	

	private bool DeleteTorrent(TorrentHandle handle)
	{
		Torrent torrent = GetTorrent(handle);
		
		if(torrent == null)
		{
			Debug.Assert(false);
			return false;
		}

		torrentList.Remove(torrent);
		return true;
	}


	// TODO : Use a proper lookup
	public Torrent GetTorrent(TorrentHandle handle)
	{
		for (int i = 0; i < torrentList.Count; i++)
		{
			Torrent torrent = (Torrent) torrentList[i];
			if (torrent.Handle == handle)
			//if (contact.Jid.Equals(jid))
			{
				return torrent;
			}
		}
		return null;
	}


	public Torrent TorrentAtIndex(Int32 index)
	{
		return torrentList[index];
	}


	public void TorrentAddedCallback(TorrentHandle handle, IntPtr param)
	{
		StoreTorrent(handle);
	}


	public void TorrentRemovedCallback(TorrentHandle handle, IntPtr param)
	{
		DeleteTorrent(handle);
	}


	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void AddTorrentAdded_Delegate(TorrentHandle handle, IntPtr param);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void AddTorrentRemoved_Delegate(TorrentHandle handle, IntPtr param);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "TimeTorrentManagerRunning")]
	public static extern UInt32 TimeTorrentManagerRunning();


	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "AddTorrentAddedCallback")]
	private static extern void AddTorrentAddedCallback(AddTorrentAdded_Delegate cb);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "AddTorrentRemovedCallback")]
	private static extern void AddTorrentRemovedCallback(AddTorrentRemoved_Delegate cb);


	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "BitTorrentManagerState")]
	private static extern BtmState BitTorrentManagerState();
}