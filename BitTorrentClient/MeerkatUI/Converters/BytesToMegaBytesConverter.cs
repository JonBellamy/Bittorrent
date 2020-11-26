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
    [ValueConversion(typeof(Int64), typeof(string))]
    public class BytesToMegaBytesConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            Int64 bytes = System.Convert.ToInt64(value);
            const int scale = 1024;
            string[] orders = new string[] { "GB", "MB", "KB", "Bytes" };
            long max = (long)Math.Pow(scale, orders.Length - 1); 
            foreach (string order in orders)
            {
                if (bytes > max)
                {
                    return string.Format("{0:##.##} {1}", decimal.Divide(bytes, max), order); 
                }
                max /= scale;
            }
            return "0 Bytes";
        }
        
        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException("JB: Convertor not in use");
        }
    }


    // Takes 2 params, multiplies then together, then converts to xMb xGb etc
    //[ValueConversion(typeof(Int64), typeof(string))]   
    public class BytesToMegaBytesMultiConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            if (values[0] == DependencyProperty.UnsetValue || values[1] == DependencyProperty.UnsetValue)
            {
                return null;
            }
            UInt32 p1 = System.Convert.ToUInt32(values[0]);
            UInt32 p2 = System.Convert.ToUInt32(values[1]);
            Int64 bytes = p1 * p2;

            BytesToMegaBytesConverter converter = new BytesToMegaBytesConverter();
            return converter.Convert(bytes, typeof(String), parameter, culture);
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException("JB: Converter not in use");
        }
    }

}
