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
    //[ValueConversion(typeof(double), typeof(string))]
    public class PercentageConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            if (values[0] == DependencyProperty.UnsetValue || values[1] == DependencyProperty.UnsetValue)
            {
                return null;
            }
            UInt32 current = System.Convert.ToUInt32(values[0]);
            UInt32 total = System.Convert.ToUInt32(values[1]);
            double onePercent = ((double)current / 100.0d);
            double done = (double)(total) / (double)(onePercent);
            if (double.IsNaN(done) || done < 0.0d) done = 0.0d;
            if (done > 100.0d) done = 100.0d;
            return String.Format("{0:0.0#}%", done);
        }
        
        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException("JB: Converter not in use");
        }
    }

}
