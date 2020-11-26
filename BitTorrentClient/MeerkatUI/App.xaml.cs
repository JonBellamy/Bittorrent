using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;

namespace MeerkatUI
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        public App()
        {
            // Ensure the btm gets created straight away
            MeerkatBindings.BitTorrentManager.Instance();
        }
    }
}
