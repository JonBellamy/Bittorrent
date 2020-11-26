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
    public class SecondsToTimeSpanConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            Int64 eta = System.Convert.ToInt64(value);          
            if (eta == -1)
            {
                char chInfinity = (char)0x221E;
                return String.Format("{0}", chInfinity);
            }
            else
            {
                // TODO: This is flooding the intern pool with strings, use StringBuilder!!!!!
                TimeSpan t = TimeSpan.FromSeconds(eta);
                if (t.Days > 0)
                {
                    return string.Format("{0}d {1:D2}h {2:D2}m", t.Days, t.Hours, t.Minutes);
                }
                else if (t.Hours > 0)
                {
                    return string.Format("{0}h {1:D2}m", t.Hours, t.Minutes);
                }
                else if (t.Minutes > 0)
                {
                    string str = string.Format("{0} minute", t.Minutes);
                    if (t.Minutes > 1) str = str + "s";
                    return str;
                }
                else
                {
                    return string.Format("{0} seconds", t.Seconds);
                }
            }            
        }
        

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException("JB: Converter not in use");
        }
    }

}
