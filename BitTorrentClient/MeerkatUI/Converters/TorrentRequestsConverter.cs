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

    public class TorrentRequestsConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            if (values[0] == DependencyProperty.UnsetValue || values[1] == DependencyProperty.UnsetValue)
            {
                return null;
            }
            UInt32 oustandingDownloadRequests = System.Convert.ToUInt32(values[0]);
            UInt32 oustandingUploadRequests = System.Convert.ToUInt32(values[1]);
            return String.Format("{0} | {1}", oustandingDownloadRequests, oustandingUploadRequests);
        }
        
        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException("JB: Converter not in use");
        }
    }
}
