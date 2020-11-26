using MeerkatBindings;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace MeerkatBindingsTest
{
    [TestClass()]
    public class TorrentTest
    {
        [TestMethod()]
        public void TorrentConstructorTest()
        {
            UInt32 handle = 100;
            Torrent target = new Torrent(handle);
            Assert.AreEqual(target.Handle, handle);
        }


        [TestMethod()]
        public void TorrentPropertiesTest()
        {
            UInt32 handle = 101;
            string name = "TorrentName";
            Int64 totalSize = (1024 * 1024);
            //double percentDone = 47.7d;
            TorrentMetaData.TorrentState state = TorrentMetaData.TorrentState.PeerMode;
            UInt32 numPeers = 10;
            UInt32 numSeeds = 5;
            UInt32 downloadSpeed = 2048;
            UInt32 uploadSpeed = 512;
            Int64 eta = 100000;

            Torrent target = new Torrent(handle) { Id = handle, Name = name, TotalSize = totalSize, /*PercentDone = percentDone, */State = state, NumPeers = numPeers, NumSeeds = numSeeds, DownloadSpeed = downloadSpeed, UploadSpeed = uploadSpeed, Eta = eta };
            Assert.AreEqual(target.Handle, handle);
            Assert.AreEqual(target.Name, name);
            Assert.AreEqual(target.TotalSize, totalSize);
            //Assert.AreEqual(target.PercentDone, percentDone);
            Assert.AreEqual(target.State, state);
            Assert.AreEqual(target.NumPeers, numPeers);
            Assert.AreEqual(target.NumSeeds, numSeeds);
            Assert.AreEqual(target.DownloadSpeed, downloadSpeed);
            Assert.AreEqual(target.UploadSpeed, uploadSpeed);
            Assert.AreEqual(target.Eta, eta);
        }
    }
}
