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
    public class SecondsFromUnixEpochConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            Int64 secondsFromUnixEpoch = System.Convert.ToInt64(value);
            DateTime unixEpoch = new DateTime(1970, 1, 1);
            unixEpoch = unixEpoch.AddSeconds(secondsFromUnixEpoch);
            return unixEpoch.ToString("D");         
        }
        

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException("JB: Converter not in use");
        }
    }

}
