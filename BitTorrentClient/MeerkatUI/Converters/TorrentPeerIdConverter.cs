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
    [ValueConversion(typeof(Byte[]), typeof(string))]
    public class TorrentPeerIdConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value == DependencyProperty.UnsetValue)
            {
                return null;
            }

            Byte[] byteArray = value as Byte[];
            if (byteArray == null)
            {
                return null;
            }

            Byte[] clientIdBytes = new Byte[8];
            Byte[] peerIdBytes = new Byte[12];
            Array.Copy(byteArray, 0, clientIdBytes, 0, 8);
            Array.Copy(byteArray, 8, peerIdBytes, 0, 12);

            Encoding ascii = Encoding.ASCII;                      

            StringBuilder sb = new StringBuilder("", 64);
            sb.Append(ascii.GetString(clientIdBytes));
            sb.Append(BitConverter.ToString(peerIdBytes));
            return sb.ToString();
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException("JB: Converter not in use");
        }
    }

}
