using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Threading;

//using ZedGraph;

namespace MeerkatBindings
{
    public interface IBitTorrentManager
    {
        event TorrentActionHandler TorrentAdded;
        event TorrentActionHandler TorrentRemoved;
        event TorrentActionHandler TorrentMataDataUpdated;

        bool AddTorrent(string filename, string downloadFolder);
        bool RemoveTorrent(UInt32 handle);
        void StartTorrent(UInt32 handle);
        void StopTorrent(UInt32 handle);
        void PauseTorrent(UInt32 handle);

        Torrent GetTorrent(UInt32 handle);
    }

    public sealed class BitTorrentManager : IDisposable, IBitTorrentManager
    {
        // Singleton
        private readonly static BitTorrentManager instance = new BitTorrentManager();
        public static BitTorrentManager Instance()
        {
            return instance;
        }

        private bool disposed;

        private bool exitThreads;

        public event TorrentActionHandler TorrentAdded;
        public event TorrentActionHandler TorrentRemoved;
        public event TorrentActionHandler TorrentMataDataUpdated;

        List<Torrent> torrentList;

        public Int32 TorrentCount { get { return torrentList.Count; } }

        private AddTorrentAdded_Delegate torrentAddedDelegate;
        private AddTorrentRemoved_Delegate torrentRemovedDelegate;


        //private PointPairList downloadGraphData;
        //public PointPairList DownloadGraphData { get { return downloadGraphData; } }

        //private PointPairList uploadGraphData;
        //public PointPairList UploadGraphData { get { return uploadGraphData; } }

        Thread coreUpdateThread;
        Thread metaDataThread;

        public enum BtmState
        {
            Stopped = 0,
            Initializing,
            Running
        }
        public BtmState State { get { return BitTorrentManagerState(); } }


        private BitTorrentManager()
        {
            disposed = false;

            torrentList = new List<Torrent>();

            InitTorrentManager(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData));

            //downloadGraphData = new PointPairList();
            //uploadGraphData = new PointPairList();

            // Dll callbacks
            torrentAddedDelegate = new AddTorrentAdded_Delegate(TorrentAddedCallback);
            torrentRemovedDelegate = new AddTorrentRemoved_Delegate(TorrentRemovedCallback);
            AddTorrentAddedCallback(torrentAddedDelegate);
            AddTorrentRemovedCallback(torrentRemovedDelegate);

            exitThreads = false;
     
            // Use threads instead of timers so we don't get calls queuing up if an update takes too long
            coreUpdateThread = new Thread(new ThreadStart(CoreUpdate));
            coreUpdateThread.Start();

            metaDataThread = new Thread(new ThreadStart(MetaDataThreadProc));
            metaDataThread.Start();        
        }


        ~BitTorrentManager()
        {
            Dispose(false);
        }


		public void Dispose()
		{
			Dispose(true);
			GC.SuppressFinalize(this);
		}


		private void Dispose(bool disposing)
		{
			if (!this.disposed)
			{                
				if (disposing)
                {                   
                }

                exitThreads = true;
                coreUpdateThread.Join();
                metaDataThread.Join();
                DeInitTorrentManager();

				disposed = true;
			}
		}


        private void CoreUpdate()
        {
            while (exitThreads == false)
            {
                UpdateTorrentManager();

                if (State != BtmState.Running)
                {
                    continue;
                }

                Thread.Sleep(5);
            }
        }


        private void MetaDataThreadProc()
        {
            while (exitThreads == false)
            {
                if (State != BtmState.Running)
                {
                    continue;
                }


                TorrentHandles handles = new TorrentHandles();
                GetAllTorrentHandles(ref  handles);


                for (Int32 i = 0; i < handles.mNumHandles; i++)
                {
                    Int32 handle = handles.mHandles[i];

                    Torrent torrent = GetTorrent((UInt32) handle);
                    if (torrent != null)
                    {
                        torrent.RefreshMetaData();
  
                        if (TorrentMataDataUpdated != null)
                        {
                            TorrentMataDataUpdated(this, new TorrentActionEventArgs((UInt32)handle));
                        }                        
                    }
                }


                /* TODO : One MINUTE NOT one second
                foreach (Torrent torrent in torrentList)
                {
                    torrent.SubmitGraphData((TimeTorrentManagerRunning() / (60000)), (torrent.MetaData.DownloadSpeed / 1024), (TimeTorrentManagerRunning() / 60000), (torrent.MetaData.UploadSpeed / 1024));
                }


                float dlSpeed = ((float)TotalDownloadRate() / 1024.0f);
                float ulSpeed = ((float)TotalUploadRate() / 1024.0f);

                //downloadGraphData.Add(TimeTorrentManagerRunning() / 60000, dlSpeed);
                //uploadGraphData.Add(TimeTorrentManagerRunning() / 60000, ulSpeed);

                //if (MainForm.mBandwidthGraph != null)
                //{
                //    MainForm.mBandwidthGraph.UpdateCurves();
                //}
                 */

                Thread.Sleep(1000);
            }
        }


        public bool AddTorrent(string filename, string downloadFolder)
        {
            Int32 handle = _AddTorrent(filename, downloadFolder);
            if (handle < 0)
            {
                return false;
            }
            return true;
        }


        public bool RemoveTorrent(UInt32 handle)
        {
            return _RemoveTorrent(handle);
        }


        public void StartTorrent(UInt32 handle)
        {
            _StartTorrent(handle);
        }


        public void StopTorrent(UInt32 handle)
        {
            _StopTorrent(handle);
        }


        public void PauseTorrent(UInt32 handle)
        {
            _PauseTorrent(handle);
        }


        private bool StoreTorrent(UInt32 handle)
        {
            if (GetTorrent(handle) != null)
            {
                Debug.Assert(false);
                return false;
            }

            Torrent torrent = new Torrent((UInt32)handle);
           
            // Get the meta data then store the torrent
            torrent.RefreshMetaData();

            torrentList.Add(torrent);
            return true;
        }


        private bool DeleteTorrent(UInt32 handle)
        {
            Torrent torrent = GetTorrent(handle);

            if (torrent == null)
            {
                Debug.Assert(false);
                return false;
            }

            torrentList.Remove(torrent);
            return true;
        }


        // TODO : Use a proper lookup
        public Torrent GetTorrent(UInt32 handle)
        {
            for (int i = 0; i < torrentList.Count; i++)
            {
                Torrent torrent = (Torrent)torrentList[i];
                if (torrent.Handle == handle)
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


        public void TorrentAddedCallback(UInt32 handle, IntPtr param)
        {
            StoreTorrent(handle);

            if (TorrentAdded != null)
            {
                TorrentAdded(this, new TorrentActionEventArgs(handle));
            }
        }


        public void TorrentRemovedCallback(UInt32 handle, IntPtr param)
        {
            if (TorrentRemoved != null)
            {
                TorrentRemoved(this, new TorrentActionEventArgs(handle));
            }

            DeleteTorrent(handle);
        }


        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void AddTorrentAdded_Delegate(UInt32 handle, IntPtr param);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void AddTorrentRemoved_Delegate(UInt32 handle, IntPtr param);



        //////////////////////////////////////////////////////////////////////////
        // Dll Calls

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "InitTorrentManager")]
        private static extern void InitTorrentManager(String appDataFolder);

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "DeInitTorrentManager")]
        private static extern void DeInitTorrentManager();

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "UpdateTorrentManager")]
        private static extern void UpdateTorrentManager();


        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "AddTorrent")]
        private static extern int _AddTorrent(string filename, String rootDir);

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "RemoveTorrent")]
        private static extern bool _RemoveTorrent(UInt32 torrentId);

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "StartTorrent")]
        private static extern void _StartTorrent(UInt32 torrentId);

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "StopTorrent")]
        private static extern void _StopTorrent(UInt32 torrentId);

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "PauseTorrent")]
        private static extern void _PauseTorrent(UInt32 torrentId);


        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "TimeTorrentManagerRunning")]
        private static extern UInt32 TimeTorrentManagerRunning();

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "AddTorrentAddedCallback")]
        private static extern void AddTorrentAddedCallback(AddTorrentAdded_Delegate cb);

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "AddTorrentRemovedCallback")]
        private static extern void AddTorrentRemovedCallback(AddTorrentRemoved_Delegate cb);

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "BitTorrentManagerState")]
        private static extern BtmState BitTorrentManagerState();

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "TotalDownloadRate")]
        private static extern UInt32 TotalDownloadRate();

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "TotalUploadRate")]
        private static extern UInt32 TotalUploadRate();


        //////////////////////////////////////////////////////////////////////////
        // Get Torrent Handles

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        private struct TorrentHandles
        {
            [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 64)]
            public Int32[] mHandles;
            public UInt32 mNumHandles;
        }
        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "GetAllTorrentHandles")]
        private static extern void GetAllTorrentHandles(ref TorrentHandles handles);
    }


}