using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Runtime.InteropServices;


//using ZedGraph;

namespace MeerkatBindings
{
    // TODO : Needs to be IDisposable so that we can remove callbacks after we cleanup
    public class Torrent : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public UInt32 Handle { get; set; }
        //public TorrentMetaData MetaData { get; set; }

        private UInt32 id;
        public UInt32 Id { get { return id; } set { id = value; OnPropertyChanged("Id"); } }

        private string name;
        public string Name { get { return name; } set { name = value; OnPropertyChanged("Name"); } }

        private Byte[] infoHash;
        public Byte[] InfoHash { get { return infoHash; } set { infoHash = value; OnPropertyChanged("InfoHash"); } }

        private Int64 totalSize;
        public Int64 TotalSize { get { return totalSize; } set { totalSize = value; OnPropertyChanged("TotalSize"); } }

        private TorrentMetaData.TorrentState state;
        public TorrentMetaData.TorrentState State { get { return state; } set { state = value; OnPropertyChanged("State"); } }

        private UInt32 numPeers;
        public UInt32 NumPeers { get { return numPeers; } set { numPeers = value; OnPropertyChanged("NumPeers"); } }

        private UInt32 numSeeds;
        public UInt32 NumSeeds { get { return numSeeds; } set { numSeeds = value; OnPropertyChanged("NumSeeds"); } }

        private UInt32 downloadSpeed;
        public UInt32 DownloadSpeed { get { return downloadSpeed; } set { downloadSpeed = value; OnPropertyChanged("DownloadSpeed"); } }

        private UInt32 uploadSpeed;
        public UInt32 UploadSpeed { get { return uploadSpeed; } set { uploadSpeed = value; OnPropertyChanged("UploadSpeed"); } }

        private Int64 eta;
        public Int64 Eta { get { return eta; } set { eta = value; OnPropertyChanged("Eta"); } }

        private UInt32 totalPieces;
        public UInt32 TotalPieces { get { return totalPieces; } set { totalPieces = value; OnPropertyChanged("TotalPieces"); } }

        private UInt32 pieceSize;
        public UInt32 PieceSize { get { return pieceSize; } set { pieceSize = value; OnPropertyChanged("PieceSize"); } }

        private UInt32 numPiecesDownloaded;
        public UInt32 NumPiecesDownloaded { get { return numPiecesDownloaded; } set { numPiecesDownloaded = value; OnPropertyChanged("NumPiecesDownloaded"); } }

        private string torrentFilename;
        public string TorrentFilename { get { return torrentFilename; } set { torrentFilename = value; OnPropertyChanged("TorrentFilename"); } }

        private string downloadFolder;
        public string DownloadFolder { get { return downloadFolder; } set { downloadFolder = value; OnPropertyChanged("DownloadFolder"); } }

        private string comment;
        public string Comment { get { return comment; } set { comment = value; OnPropertyChanged("Comment"); } }

        private UInt32 timeElapsedSinceStarted;
        public UInt32 TimeElapsedSinceStarted { get { return timeElapsedSinceStarted; } set { timeElapsedSinceStarted = value; OnPropertyChanged("TimeElapsedSinceStarted"); } }

        private UInt32 creationDate;
        public UInt32 CreationDate { get { return creationDate; } set { creationDate = value; OnPropertyChanged("CreationDate"); } }


        public void OnPropertyChanged(string prop)
        {
            PropertyChangedEventArgs args = new PropertyChangedEventArgs(prop);
            if (PropertyChanged != null)
            {
                PropertyChanged(this, args);
            }
        }


        public List<Peer> Peers { get; private set; }


        private PeerConnectedCallback peerConnectedDelegate;
        private PeerDisonnectedCallback peerDisconnectedDelegate;

        //private PointPairList downloadGraphData;
        //public PointPairList DownloadGraphData { get { return downloadGraphData; } }

        //private PointPairList uploadGraphData;
        //public PointPairList UploadGraphData { get { return uploadGraphData; } }


        //public enum TorrentValues
        //{
        //    INVALID_TORRENT = -1
        //};



        public Torrent(UInt32 handle)
        {
            this.Handle = handle;

            Peers = new List<Peer>();

            //downloadGraphData = new PointPairList();
            //uploadGraphData = new PointPairList();

            peerConnectedDelegate = new PeerConnectedCallback(OnPeerConnected);
            AddPeerConnectedCallback(peerConnectedDelegate);

            peerDisconnectedDelegate = new PeerDisonnectedCallback(OnPeerDisconnected);
            AddPeerDisconnectedCallback(peerDisconnectedDelegate);
            
         }


        public void SubmitGraphData(UInt32 dlMinute, UInt32 dlSpeed, UInt32 ulMinute, UInt32 ulSpeed)
        {
           // downloadGraphData.Add(dlMinute, dlSpeed);
           // uploadGraphData.Add(ulMinute, ulSpeed);

        }


        public void RefreshMetaData()
        {
            TorrentMetaData meta = new TorrentMetaData();
            TorrentMetaData.GetTorrentMetaData((Int32)Handle, ref meta);

            // All this is ugly but needed, so that the property changed event gets fired to update our view model
            Id = Handle;
            Name = meta.Name;
            InfoHash = meta.InfoHash;
            TotalSize = meta.TotalSize;
            NumPeers = meta.NumPeers;
            NumSeeds = meta.NumSeeds;
            DownloadSpeed = meta.DownloadSpeed;
            UploadSpeed = meta.UploadSpeed;
            State = meta.State;
            Eta = meta.Eta;
            TotalPieces = meta.TotalPieces;
            PieceSize = meta.PieceSize;
            NumPiecesDownloaded = meta.PiecesDownloaded;
            TorrentFilename = meta.FileName;
            DownloadFolder = meta.TargetFolder;
            TimeElapsedSinceStarted = meta.TimeSinceStarted;
            Comment = meta.Comment;
            CreationDate = meta.CreationDate;


            for (int i = 0; i < Peers.Count; i++)
            {
                Peer peer = Peers[i];
                peer.RefreshMetaData(Handle);
            }
      
        }

        public void OnPeerConnected(UInt32 handle, UInt32 ip, UInt16 port, IntPtr param)
        {
            if (handle == Handle)
            {
                SockAddr sockAddr = new SockAddr(ip, port);
                if (Peers.Find(x => x.Addr == sockAddr) == null)
                {
                    Peer peer = new Peer(sockAddr);
                    Peers.Add(peer);
                    OnPropertyChanged("Peers");
                }
            }
        }





        public void OnPeerDisconnected(UInt32 handle, UInt32 ip, UInt16 port, IntPtr param)
        {
            if (handle == Handle)
            {
                SockAddr sockAddr = new SockAddr(ip, port);
               
                /*
                foreach (Peer peer in Peers)
                {
                    if (peer.Addr.Equals(sockAddr))
                    {
                        Peers.Remove(peer);
                        return;
                    }
                }
                */
             
                Peer peer = Peers.Find( (x) => x.Addr.Equals(sockAddr) );
                if (peer != null)
                {
                    Peers.Remove(peer);
                }
                OnPropertyChanged("Peers");           
            }
        }


        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void PeerConnectedCallback(UInt32 handle, UInt32 ip, UInt16 port, IntPtr param);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void PeerDisonnectedCallback(UInt32 handle, UInt32 ip, UInt16 port, IntPtr param);

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "AddPeerConnectedCallback")]
        private static extern void AddPeerConnectedCallback(PeerConnectedCallback cb);

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "AddPeerDisconnectedCallback")]
        private static extern void AddPeerDisconnectedCallback(PeerDisonnectedCallback cb);

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "RemovePeerConnectedCallback")]
        private static extern void RemovePeerConnectedCallback(PeerConnectedCallback cb);

        [DllImport("bt.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "RemovePeerDisconnectedCallback")]
        private static extern void RemovePeerDisconnectedCallback(PeerDisonnectedCallback cb);
    }

}