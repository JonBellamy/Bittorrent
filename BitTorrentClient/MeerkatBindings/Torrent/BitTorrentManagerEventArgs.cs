using System;

namespace MeerkatBindings
{
    public class TorrentActionEventArgs : EventArgs
    {
        public TorrentActionEventArgs(UInt32 handle)
        {
            this.Handle = handle;
        }
        public UInt32 Handle { get; set; }
    }

    public delegate void TorrentActionHandler(object sender, TorrentActionEventArgs args);
}
