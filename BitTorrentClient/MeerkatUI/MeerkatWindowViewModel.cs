using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;

using MeerkatBindings;


namespace MeerkatUI
{
    public interface IMeerkatWindowViewModel
    {
        bool AddTorrent();
        bool RemoveTorrent();

        bool StartTorrent();
        bool StopTorrent();
        bool PauseTorrent();
    }


    public sealed class MeerkatWindowViewModel : INotifyPropertyChanged, IMeerkatWindowViewModel
    {
        private readonly Dispatcher currentDispatcher;

        public ObservableCollection<Torrent> Torrents { get; set; }
        public ObservableCollection<Peer> Peers { get; set; }

        private Torrent selectedTorrent;
        public Torrent SelectedTorrent 
        { 
            get { return selectedTorrent; } 
            set 
            {
                if (selectedTorrent != null)
                {
                    selectedTorrent.PropertyChanged -= TorrentPropertyChangedEventHandler;
                }

                selectedTorrent = value;

                if (selectedTorrent != null)
                {
                    selectedTorrent.PropertyChanged += TorrentPropertyChangedEventHandler;
                    OnSelectedTorrentChanged();  
                }
                
                OnPropertyChanged("SelectedTorrent"); 
            } 
        }

        public event PropertyChangedEventHandler PropertyChanged;

        // Services
        private IBitTorrentManager bitTorrentManager;
        private IOpenFileService openFileService;
        private IFolderBrowerService folderBrowerService;

        // Commands
        public ICommand AddTorrentCommand { get; set; }
        public ICommand RemoveTorrentCommand { get; set; }
        public ICommand RemoveAndDeleteTorrentCommand { get; set; }
        public ICommand OpenContainingFolderCommand { get; set; }
        public ICommand TorrentRecheckCommand { get; set; }
        public ICommand AllowUnencryptedConnectionsCommand { get; set; }
        public ICommand StartTorrentCommand { get; set; }
        public ICommand StopTorrentCommand { get; set; }
        public ICommand PauseTorrentCommand { get; set; }
      



        public MeerkatWindowViewModel(IOpenFileService openFileService, IFolderBrowerService folderBrowerService, IBitTorrentManager bitTorrentManager)
        {            
            // store the current dispatcher ref
            currentDispatcher = Dispatcher.CurrentDispatcher;

            this.bitTorrentManager = bitTorrentManager;
            this.openFileService = openFileService;
            this.folderBrowerService = folderBrowerService;

            this.AddTorrentCommand = new AddTorrentCommand(this);
            this.RemoveTorrentCommand = new RemoveTorrentCommand(this);
            this.RemoveAndDeleteTorrentCommand = new RemoveAndDeleteTorrentCommand(this);
            this.OpenContainingFolderCommand = new OpenContainingFolderCommand(this);
            this.TorrentRecheckCommand = new TorrentRecheckCommand(this);
            this.AllowUnencryptedConnectionsCommand = new AllowUnencryptedConnectionsCommand(this);
            this.StartTorrentCommand = new StartTorrentCommand(this);
            this.StopTorrentCommand = new StopTorrentCommand(this);
            this.PauseTorrentCommand = new PauseTorrentCommand(this);
            
            Torrents = new ObservableCollection<Torrent>();
            Peers = new ObservableCollection<Peer>();

            bitTorrentManager.TorrentAdded += OnTorrentAdded;
            bitTorrentManager.TorrentRemoved += OnTorrentRemoved;
        }


        public void OnTorrentAdded(object sender, TorrentActionEventArgs args)
        {
            Torrent torrent = bitTorrentManager.GetTorrent(args.Handle);

            // Marshall the collection call back to the window thread, torrent will be captured
            Action dispatchAction = () => Torrents.Add(torrent);
            this.currentDispatcher.BeginInvoke(dispatchAction);
        }


        public void OnTorrentRemoved(object sender, TorrentActionEventArgs args)
        {
            Torrent torrent = bitTorrentManager.GetTorrent(args.Handle);

            // Marshall the collection call back to the window thread, torrent will be captured
            Action dispatchAction = () => Torrents.Remove(torrent);
            this.currentDispatcher.BeginInvoke(dispatchAction);  
        }


        private void OnSelectedTorrentChanged()
        {
            SyncPeersCollection();
        }



        private void TorrentPropertyChangedEventHandler(object sender, PropertyChangedEventArgs e)
        {
            if(e.PropertyName.Equals("Peers"))
            {
                SyncPeersCollection();
            }
        }


        private void SyncPeersCollection()
        {
            if (this.currentDispatcher.CheckAccess())
            {
                if (SelectedTorrent != null)
                {
                    Peers.Clear();
                    foreach (Peer peer in SelectedTorrent.Peers)
                    {
                        Peers.Add(peer);
                    }
                }
            }
            else
            {
                Action dispatchAction = () => SyncPeersCollection();
                this.currentDispatcher.BeginInvoke(dispatchAction);                
            }
        }


        public bool AddTorrent()        
        {
            if(openFileService.OpenFileDialog() && folderBrowerService.OpenFolderBrowserDialog())
            {
                return bitTorrentManager.AddTorrent(openFileService.FileName, folderBrowerService.FolderName);
            }
            return false;
        }


        public bool RemoveTorrent()
        {
            if (SelectedTorrent != null)
            {
                return bitTorrentManager.RemoveTorrent(SelectedTorrent.Handle);
            }
            return false;         
        }


        public bool StartTorrent()
        {
            if (SelectedTorrent != null)
            {
                bitTorrentManager.StartTorrent(SelectedTorrent.Handle);
                return true;
            }
            return false; 
        }


        public bool StopTorrent()
        {
            if (SelectedTorrent != null)
            {
                bitTorrentManager.StopTorrent(SelectedTorrent.Handle);
                return true;
            }
            return false; 
        }


        public bool PauseTorrent()
        {
            if (SelectedTorrent != null)
            {
                bitTorrentManager.PauseTorrent(SelectedTorrent.Handle);
                return true;
            }
            return false; 
        }


        public void OnPropertyChanged(string prop)
        {
            PropertyChangedEventArgs args = new PropertyChangedEventArgs(prop);
            if (PropertyChanged != null)
            {
                PropertyChanged(this, args);
            }
        }
         
    }
}
