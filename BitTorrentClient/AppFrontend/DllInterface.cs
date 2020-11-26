using System;
using System.Collections.Generic;
//using System.Linq;
using System.Text;
using System.Runtime.InteropServices;


using TorrentHandle = System.Int32;


static public class DllInterface
{
    // if the function is extern c then it won't be mangled and you can do this ...
//    [DllImport("bt.dll")]
//    public static extern int foo1();

    // note the mangled name, use the dependency walker tool to view the mangled names
//        [DllImport("DllTest.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "?foo2@@YGHXZ")]
//        public static extern int foo2();

//        [DllImport("DllTest.dll")]
//        public static extern void ?fnDllTest@@YAHH@Z(int x);


	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "DebugDll")]
	public static extern void DebugDll();

	//[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "SetStdOutHandle")]
	//public static extern void SetStdOutHandle(IntPtr h);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void DebugString_Delegate(String s);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "SetDebugStringOutputCb")]
	public static extern void SetDebugStringOutputCb(DebugString_Delegate cb);

	
	
	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "InitTorrentManager")]
	public static extern void InitTorrentManager(String appDataFolder);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "DeInitTorrentManager")]
	public static extern void DeInitTorrentManager();

    [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "AddTorrent")]
    public static extern int AddTorrent(string filename, String rootDir);

    [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "RemoveTorrent")]
    public static extern bool RemoveTorrent(TorrentHandle torrentId);

    [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "UpdateTorrentManager")]
    public static extern void UpdateTorrentManager();



	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "SaveTorrentManagerState")]
	public static extern bool SaveTorrentManagerState();

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "LoadTorrentManagerState")]
	public static extern bool LoadTorrentManagerState();


	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "TorrentValid")]
	public static extern bool TorrentValid(TorrentHandle torrentId);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "TorrentForceRecheck")]
	public static extern void TorrentForceRecheck(TorrentHandle torrentId);


	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "IsDhtRunning")]
	public static extern bool IsDhtRunning();

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "NumberOfDhtNodes")]
	public static extern UInt32 NumberOfDhtNodes();


	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "SetListenerPort")]
	public static extern UInt32 SetListenerPort(UInt16 port);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "GetListenerPort")]
	public static extern UInt16 GetListenerPort();

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "StartTorrent")]
	public static extern void StartTorrent(TorrentHandle torrentId);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "StopTorrent")]
	public static extern void StopTorrent(TorrentHandle torrentId);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "PauseTorrent")]
	public static extern void PauseTorrent(TorrentHandle torrentId);



	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "TotalDownloadRate")]
	public static extern UInt32 TotalDownloadRate();

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "TotalUploadRate")]
	public static extern UInt32 TotalUploadRate();



	//////////////////////////////////////////////////////////////////////////
	// Torrent Client Options

	[StructLayout(LayoutKind.Sequential)]
	public struct sTorrentClientOptions
	{
		public Byte mAllConnectionsMustBeEncrypted;
		public Byte mStopOnCompletion;
		public Byte mRecheckOnCompletion;
		public Byte mUseDht;
		public Byte mUseTrackers;
		public Byte mCheckForLatestBuild;
		public UInt32 mMaxUploadRate;
	}
	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "GetTorrentClientOptions")]
	public static extern void GetTorrentClientOptions(ref sTorrentClientOptions options);
	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "SetTorrentClientOptions")]
	public static extern void SetTorrentClientOptions(sTorrentClientOptions options);


	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "SetTorrentOnlyUsesEncryptedConnections")]
	public static extern void SetTorrentOnlyUsesEncryptedConnections(TorrentHandle torrentId, bool allow);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "DoesTorrentOnlyUsesEncryptedConnections")]
	public static extern bool DoesTorrentOnlyUsesEncryptedConnections(TorrentHandle torrentId);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "DisconnectAllUnencryptedPeers")]
	public static extern void DisconnectAllUnencryptedPeers(TorrentHandle torrentId);


	//////////////////////////////////////////////////////////////////////////
	// Get Torrent Handles

	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
	public struct TorrentHandles
	{
		[MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 64)]
		public Int32[] mHandles;
		public UInt32 mNumHandles;
	}
	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "GetAllTorrentHandles")]
	public static extern void GetAllTorrentHandles(ref TorrentHandles handles);


	//////////////////////////////////////////////////////////////////////////
	// Get Torrent Meta Data

	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)] 
    public struct TorrentMetaData
    {
		public TorrentHandle mHandle;

		public enum TorrentState
		{
			Stopped = 0,
			CreateFiles,
			PeerMode,
			SeedMode,
			Queued,
			Rechecking
		};
		public TorrentState mState;

		[MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 256)]
		public string mName;

		[MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 256)]
		public string mFileName;

		[MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 1024)]
		public string mTargetFolder;

		[MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 512)]
		public string mComment;

		[MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 20)]
		public Byte[] mInfoHash;

		public UInt32 mCreationDate;
		public UInt32 mTotalPieces;
		public UInt32 mPiecesDownloaded;
		public UInt32 mPieceSize;
		public Int64  mTotalSize;
		public UInt32 mTimeSinceStarted;
		public Int64  mEta;
	    public UInt32 mDownloadSpeed;
	    public UInt32 mUploadSpeed;
		public UInt32 mEventFlags;
		public UInt32 mNumEncryptedConnections;
		public UInt32 mNumUnencryptedConnections;
		public UInt32 mNumSeeds;
		public UInt32 mNumPeers;
    };
	[DllImport("bt.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, EntryPoint = "GetTorrentMetaData")]
	public static extern void GetTorrentMetaData(TorrentHandle torrentId, ref TorrentMetaData meta);



	//////////////////////////////////////////////////////////////////////////
	// Peer Info

	[StructLayout(LayoutKind.Sequential)]
	public struct sPeerInfo
	{
		[MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 20)]
		public Byte[]  mPeerId;

		public UInt32 mDlRate;
		public UInt32 mUlRate;
		public UInt32 mTotalBytesDownloaded;
		public UInt32 mTotalBytesUploaded;
		public UInt32 mOustandingDownloadRequests;
		public UInt32 mOustandingUploadRequests;
		public UInt32 mConnectionLengthInSeconds;

		[MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 4)]
		public Byte[] mIp;

		public UInt16 mPort;

		public Byte mAmChoking;
		public Byte mIsChokingMe;
		public Byte mAmInterested;
		public Byte mIsInterestedInMe;

		public float mPercentageDone;

		public bool mHandshakeRecvd;
		public bool mIsSeed;

		public enum ConnectionFlag 
		{ 
			INCOMING_CONNECTION  = 1 << 0,
			ENCRYPTED_CONNECTION = 1 << 1
		};
		public Byte  mConnectionFlags;
	}

	//[DllImport("DllTest.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "ArrayTest")]
	//public static extern int ArrayTest(ref IntPtr d);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "GetConnectedPeersInfo")]
	public static extern UInt32 GetConnectedPeersInfo(TorrentHandle torrentId, IntPtr[] d, UInt32 maxItems);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "NumberOfConnectedPeers")]
	public static extern UInt32 NumberOfConnectedPeers(TorrentHandle torrentId);



	//////////////////////////////////////////////////////////////////////////
	// Announce Info

	[StructLayout(LayoutKind.Sequential)]
	public struct sAnnounceInfo
	{
		[MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 512)]
		public string mUrl;

		public UInt32 mAnnounceInterval;
		public UInt32 mNextUpdateMs;
		public UInt32 mNumberOfPeersFound;
		public UInt32 mLastResult;		
	}
	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "GetAnnounceTargetsInfo")]
	public static extern UInt32 GetAnnounceTargetsInfo(TorrentHandle torrentId, IntPtr[] d, UInt32 maxItems);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "NumberOfAnnounceTargets")]
	public static extern UInt32 NumberOfAnnounceTargets(TorrentHandle torrentId);



	//////////////////////////////////////////////////////////////////////////
	// Pieces Info

	[StructLayout(LayoutKind.Sequential)]
	public struct sPiecesInfo
	{
		public UInt32  mPieceNumber;
		public UInt32  mPieceSize;
		public UInt32  mNumberOfBlocks;
		public UInt32  mCompletedBlocks;
		public UInt32  mOutstandingBlocks;
		public UInt32  mRequestedBlocks;
		//public bool mIsFinalPiece;
	}
	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "GetActivePiecesInfo")]
	public static extern UInt32 GetActivePiecesInfo(TorrentHandle torrentId, IntPtr[] d, UInt32 maxItems);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "NumberOfActivePieces")]
	public static extern UInt32 NumberOfActivePieces(TorrentHandle torrentId);



	//////////////////////////////////////////////////////////////////////////
	// Files Info

	[StructLayout(LayoutKind.Sequential)]
	public struct sFileInfo
	{
		[MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 512)]
		public string mFilename;

		public Int64 mSize;
		public float mPercentageComplete;
		public UInt32 mNumberOfPieces;
	}
	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "GetTorrentFilesInfo")]
	public static extern UInt32 GetTorrentFilesInfo(TorrentHandle torrentId, IntPtr[] d, UInt32 maxItems);

	[DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "NumberOfFilesInTorrent")]
	public static extern UInt32 NumberOfFilesInTorrent(TorrentHandle torrentId);
}
