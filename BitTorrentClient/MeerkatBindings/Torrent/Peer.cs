using System;
using System.ComponentModel;
//using ZedGraph;

namespace MeerkatBindings
{
    public class Peer : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private SockAddr addr;
        public SockAddr Addr { get { return addr; } set { addr = value; OnPropertyChanged("Addr"); } }

        public Byte[] peerId;
        public Byte[] PeerId { get { return peerId; } set { peerId = value; OnPropertyChanged("PeerId"); } }

        private UInt32 downloadRate;
        public UInt32 DownloadRate { get { return downloadRate; } set { downloadRate = value; OnPropertyChanged("DownloadRate"); } }

        private UInt32 uploadRate;
        public UInt32 UploadRate { get { return uploadRate; } set { uploadRate = value; OnPropertyChanged("UploadRate"); } }

        public UInt32 totalBytesDownloaded;
        public UInt32 TotalBytesDownloaded { get { return totalBytesDownloaded; } set { totalBytesDownloaded = value; OnPropertyChanged("TotalBytesDownloaded"); } }

        public UInt32 totalBytesUploaded;
        public UInt32 TotalBytesUploaded { get { return totalBytesUploaded; } set { totalBytesUploaded = value; OnPropertyChanged("TotalBytesUploaded"); } }

        public UInt32 connectionLengthInSeconds;
        public UInt32 ConnectionLengthInSeconds { get { return connectionLengthInSeconds; } set { connectionLengthInSeconds = value; OnPropertyChanged("ConnectionLengthInSeconds"); } }

        public ConnectionFlag connectionFlags;
        public ConnectionFlag ConnectionFlags { get { return connectionFlags; } set { connectionFlags = value; OnPropertyChanged("ConnectionFlags"); } }

        public UInt32 oustandingDownloadRequests;
        public UInt32 OustandingDownloadRequests { get { return oustandingDownloadRequests; } set { oustandingDownloadRequests = value; OnPropertyChanged("OustandingDownloadRequests"); } }

        public UInt32 oustandingUploadRequests;
        public UInt32 OustandingUploadRequests { get { return oustandingUploadRequests; } set { oustandingUploadRequests = value; OnPropertyChanged("OustandingUploadRequests"); } }

        public double percentageDone;
        public double PercentageDone { get { return percentageDone; } set { percentageDone = value; OnPropertyChanged("PercentageDone"); } }


        public Byte amChoking;
        public Byte AmChoking { get { return amChoking; } set { amChoking = value; OnPropertyChanged("AmChoking"); } }

        public Byte isChokingMe;
        public Byte IsChokingMe { get { return isChokingMe; } set { isChokingMe = value; OnPropertyChanged("IsChokingMe"); } }

        public Byte amInterested;
        public Byte AmInterested { get { return amInterested; } set { amInterested = value; OnPropertyChanged("AmInterested"); } }

        public Byte isInterestedInMe;
        public Byte IsInterestedInMe { get { return isInterestedInMe; } set { isInterestedInMe = value; OnPropertyChanged("IsInterestedInMe"); } }


        public enum ConnectionFlag
        {
            IncomingConnection = 1 << 0,
            EncryptedConnection = 1 << 1
        };


        public void OnPropertyChanged(string prop)
        {
            PropertyChangedEventArgs args = new PropertyChangedEventArgs(prop);
            if (PropertyChanged != null)
            {
                PropertyChanged(this, args);
            }
        }

        public Peer(SockAddr addr)
        {
            this.Addr = addr;
        }


  
        public void RefreshMetaData(UInt32 torrentHandle)
        {
            PeerMetaData meta = new PeerMetaData();
            PeerMetaData.GetPeerMetaData((Int32)torrentHandle, Addr.Ip, Addr.Port, ref meta);

            DownloadRate = meta.DlRate;
            UploadRate = meta.UlRate;
            PeerId = meta.PeerId;
            TotalBytesDownloaded = meta.TotalBytesDownloaded;
            TotalBytesUploaded = meta.TotalBytesUploaded;
            ConnectionLengthInSeconds = meta.ConnectionLengthInSeconds;
            ConnectionFlags = (ConnectionFlag) meta.ConnectionFlags;
            OustandingDownloadRequests = meta.OustandingDownloadRequests;
            OustandingUploadRequests = meta.OustandingUploadRequests;
            PercentageDone = meta.PercentageDone;
            AmChoking = meta.AmChoking;
            IsChokingMe = meta.IsChokingMe;
            AmInterested = meta.AmInterested;
            IsInterestedInMe = meta.IsInterestedInMe;
        }
    }

}