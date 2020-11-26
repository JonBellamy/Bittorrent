using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace MeerkatBindings
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct TorrentMetaData
    {
        public Int32 Handle;

        public enum TorrentState
        {
            Stopped = 0,
            CreateFiles,
            PeerMode,
            SeedMode,
            Queued,
            Rechecking
        };
        public TorrentState State;

        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string Name;

        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string FileName;

        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 1024)]
        public string TargetFolder;

        [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 512)]
        public string Comment;

        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 20)]
        public Byte[] InfoHash;

        public UInt32 CreationDate;
        public UInt32 TotalPieces;
        public UInt32 PiecesDownloaded;
        public UInt32 PieceSize;
        public Int64 TotalSize;
        public UInt32 TimeSinceStarted;
        public Int64 Eta;
        public UInt32 DownloadSpeed;
        public UInt32 UploadSpeed;
        public UInt32 EventFlags;
        public UInt32 NumEncryptedConnections;
        public UInt32 NumUnencryptedConnections;
        public UInt32 NumSeeds;
        public UInt32 NumPeers;

        [DllImport("bt.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, EntryPoint = "GetTorrentMetaData")]
        public static extern void GetTorrentMetaData(Int32 torrentId, ref TorrentMetaData meta);
    }
}
