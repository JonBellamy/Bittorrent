using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;


namespace MeerkatUI
{
    public partial class MeerkatWindow : Window
    {
        private MeerkatWindowViewModel vm;

        public MeerkatWindow()
        {
            InitializeComponent();

            // TODO : Look into Unity container to hold this stuff, see video tutorial!
            OpenFileService openFileService = new OpenFileService();
            FolderBrowerService folderBrowser = new FolderBrowerService();

            vm = new MeerkatWindowViewModel(openFileService, folderBrowser, MeerkatBindings.BitTorrentManager.Instance());

            // All of our Data Bindings will now be in looking at our ViewModel by default
            this.DataContext = vm;
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            MeerkatBindings.BitTorrentManager.Instance().Dispose();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            AboutWindow win = new AboutWindow();
            win.ShowDialog();
        }
    }
}
