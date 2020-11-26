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
    public class TorrentConnectionTypeConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            Int32 connectionFlags = System.Convert.ToInt32(value);

            string result = String.Empty;
            if ((connectionFlags & (Int32)Peer.ConnectionFlag.IncomingConnection) != 0)
            {
                result = "Incoming";
            }
            else
            {
                result = "Outgoing";
            }
            if ((connectionFlags & (Int32)Peer.ConnectionFlag.EncryptedConnection) != 0)
            {
                result += " Encrypted";
            }
            else
            {
                result += " Unencrypted";
            }
            return result;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException("JB: Converter not in use");
        }
    }

}
