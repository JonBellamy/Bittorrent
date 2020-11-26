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
    // Displays pieces in the form of '100 x 1mb (Have 10)'
    //[ValueConversion(typeof(double), typeof(string))]    
    public class TorrentPiecesConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            if (values[0] == DependencyProperty.UnsetValue || values[1] == DependencyProperty.UnsetValue || values[2] == DependencyProperty.UnsetValue)
            {
                return null;
            }
            UInt32 totalPieces = System.Convert.ToUInt32(values[0]);
            UInt32 pieceSize = System.Convert.ToUInt32(values[1]);
            UInt32 numPiecesDownloaded = System.Convert.ToUInt32(values[2]);

            BytesToMegaBytesConverter conv = new BytesToMegaBytesConverter();
            string strPieceSize = (string) conv.Convert(pieceSize, typeof(string), null, culture);

            return String.Format("{0} x {1} (have {2})", totalPieces, strPieceSize, numPiecesDownloaded);
        }
        
        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException("JB: Converter not in use");
        }
    }

}
