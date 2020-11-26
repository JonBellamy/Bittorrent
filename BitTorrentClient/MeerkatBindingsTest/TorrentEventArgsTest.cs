using MeerkatBindings;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace MeerkatBindingsTest
{
    [TestClass()]
    public class TorrentEventArgsTest
    {

        [TestMethod()]
        public void TorrentActionEventArgsConstructorTest()
        {
            TorrentActionEventArgs args = new TorrentActionEventArgs(5);
            Assert.AreEqual(args.Handle, 5);
        }
    }
}
