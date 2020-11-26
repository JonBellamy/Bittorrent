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

    public class TorrentFlagsConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            if (values[0] == DependencyProperty.UnsetValue || values[1] == DependencyProperty.UnsetValue || values[2] == DependencyProperty.UnsetValue || values[3] == DependencyProperty.UnsetValue)
            {
                return null;
            }
            byte amChoking = System.Convert.ToByte(values[0]);
            byte isChokingMe = System.Convert.ToByte(values[1]);
            byte amInterested = System.Convert.ToByte(values[2]);
            byte isInterestedInMe = System.Convert.ToByte(values[3]);
                
            string flags = String.Empty;
            if (amChoking == 1)
			{
				flags = flags + "C";
			}
			if (isChokingMe == 1)
			{
				flags = flags + "c";
			}
			if (amInterested == 1)
			{
				flags = flags + "I";
			}
			if (isInterestedInMe == 1)
			{
				flags = flags + "i";
			}

            return flags;
        }
        
        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException("JB: Converter not in use");
        }
    }
}
