using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace MeerkatBindings
{
    [StructLayout(LayoutKind.Sequential)]
    public struct PeerMetaData
    {
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 20)]
        public Byte[] PeerId;

        public UInt32 DlRate;
        public UInt32 UlRate;
        public UInt32 TotalBytesDownloaded;
        public UInt32 TotalBytesUploaded;
        public UInt32 OustandingDownloadRequests;
        public UInt32 OustandingUploadRequests;
        public UInt32 ConnectionLengthInSeconds;

        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 4)]
        public Byte[] Ip;

        public UInt16 Port;

        public Byte AmChoking;
        public Byte IsChokingMe;
        public Byte AmInterested;
        public Byte IsInterestedInMe;

        public float PercentageDone;

        public bool HandshakeRecvd;
        public bool IsSeed;

        public Byte ConnectionFlags;


        [DllImport("bt.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, EntryPoint = "GetPeerMetaData")]
        public static extern bool GetPeerMetaData(Int32 torrentId, UInt32 peerIp, UInt16 peerPort, ref PeerMetaData meta);
    }

}
