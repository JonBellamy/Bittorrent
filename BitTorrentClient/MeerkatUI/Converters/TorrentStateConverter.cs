using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Text;
using System.Windows;
using System.Windows.Data;
using System.Globalization;

using MeerkatBindings;


namespace MeerkatUI
{
    [ValueConversion(typeof(Int32), typeof(string))]
    public class TorrentStateConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            TorrentMetaData.TorrentState state = (TorrentMetaData.TorrentState) System.Convert.ToInt32(value);
            switch (state)
            {
                case TorrentMetaData.TorrentState.Stopped:
                    return String.Format("Stopped");

                case TorrentMetaData.TorrentState.CreateFiles:
                    return String.Format("Creating Files");

                case TorrentMetaData.TorrentState.PeerMode:
                    return String.Format("Downloading");

                case TorrentMetaData.TorrentState.SeedMode:
                    return String.Format("Seeding");

                case TorrentMetaData.TorrentState.Queued:
                    return String.Format("Queued");

                case TorrentMetaData.TorrentState.Rechecking:
                    return String.Format("Checking");
            }
            throw new Exception("Invalid state");
        }
        

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException("JB: Converter not in use");
        }
    }

}
