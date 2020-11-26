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
    public class BytesToKbsConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            //Int32 speedBytes = System.Convert.ToInt32(value);
            //double speed = ((double)speedBytes / 1024.0d);
            //return String.Format("{0:0.0} kB/s", speed);

            Int64 bytes = System.Convert.ToInt32(value);
            const int scale = 1024;
            string[] orders = new string[] { "GB/s", "MB/s", "KB/s", "Bytes/s" };
            long max = (long)Math.Pow(scale, orders.Length - 1);
            foreach (string order in orders)
            {
                if (bytes > max)
                {
                    return string.Format("{0:0.0#} {1}", decimal.Divide(bytes, max), order);
                }
                max /= scale;
            }
            return "0";
        }
        
        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            /*
            string price = value.ToString(culture);
            decimal result;
            if (Decimal.TryParse(price, NumberStyles.Any, culture, out result))
            {
                return result;
            }
            return value;
            */
            throw new NotImplementedException("JB: Converter not in use");
        }
    }

}
