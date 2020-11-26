using MeerkatUI;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

using Moq;
using MeerkatBindings;

namespace MeerkatUITest
{
    
    [TestClass()]
    public class MeerkatWindowViewModelTest
    {
        [TestMethod()]
        public void MeerkatWindowViewModelConstructorTest()
        {
            /*
            var mock = new Mock<IVm>();

            // WOW! No record/replay weirdness?! :)
            mock.Setup(mockVm => mockVm.MockTest("2.0.0.0"))
                .Returns(true);
                //.AtMostOnce();

            // Hand mock.Object as a collaborator and exercise it, like calling methods on it...
            IVm vm = mock.Object;
            bool ret = vm.MockTest("2.0.0.0");

            // Verify that the given method was indeed called with the expected value, throw an exception if it wasn't
            mock.Verify(framework => framework.MockTest("2.0.0.0"));
             */
        }


        [TestMethod()]
        public void AddTorrentTest()
        {
            // Create our mocks for the torrent manager and the open dialog services

            var btmMock = new Mock<IBitTorrentManager>();
            btmMock.Setup(mock => mock.AddTorrent("my.torrent", @"c:\downloads")).Returns(true);
            IBitTorrentManager bitTorrentManager = btmMock.Object;

            var openFileServiceMock = new Mock<IOpenFileService>();
            openFileServiceMock.Setup(mock => mock.OpenFileDialog()).Returns(true);
            openFileServiceMock.SetupProperty(f => f.FileName, "my.torrent");
            IOpenFileService openFileService = openFileServiceMock.Object;

            var folderBrowserServiceMock = new Mock<IFolderBrowerService>();
            folderBrowserServiceMock.Setup(mock => mock.OpenFolderBrowserDialog()).Returns(true);
            folderBrowserServiceMock.SetupProperty(f => f.FolderName, @"c:\downloads");
            IFolderBrowerService folderBrowserService = folderBrowserServiceMock.Object;

            // Create the vm (not mocked!) and call the function we are testing
            MeerkatWindowViewModel vm = new MeerkatWindowViewModel(openFileService, folderBrowserService, bitTorrentManager);
            bool ret = vm.AddTorrent();
            Assert.AreEqual(true, ret);

            // Verify the BitTorrentManager->AddTorrent & dialog show functions were called & called with the correct values
            openFileServiceMock.Verify(framework => framework.OpenFileDialog());
            folderBrowserServiceMock.Verify(framework => framework.OpenFolderBrowserDialog());
            btmMock.Verify(framework => framework.AddTorrent("my.torrent", @"c:\downloads"));          
        }



        [TestMethod()]
        public void RemoveTorrentTest()
        {
            UInt32 handle = 50;

            // Create our mocks
            var btmMock = new Mock<IBitTorrentManager>();
            btmMock.Setup(mock => mock.RemoveTorrent(handle)).Returns(true);
            IBitTorrentManager bitTorrentManager = btmMock.Object;

            // Create the vm (not mocked!) and call the function we are testing
            MeerkatWindowViewModel vm = new MeerkatWindowViewModel(null, null, bitTorrentManager);
            vm.SelectedTorrent = new Torrent(handle);
            bool ret = vm.RemoveTorrent();
            Assert.AreEqual(true, ret);

            // Verify the BitTorrentManager.RemoveTorrent is called with the correct values
            btmMock.Verify(framework => framework.RemoveTorrent(handle));
        }
    }
}
