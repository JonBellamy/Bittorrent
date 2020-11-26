using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;


namespace MeerkatUI
{
    public interface IAboutWindowViewModel
    {
        string Description { get; set; }
    }


    public sealed class AboutWindowViewModel : INotifyPropertyChanged, IAboutWindowViewModel
    {
        private readonly Dispatcher currentDispatcher;

        public event PropertyChangedEventHandler PropertyChanged;

        public string Description { get; set; }


        public AboutWindowViewModel()
        {
            // store the current dispatcher ref
            currentDispatcher = Dispatcher.CurrentDispatcher;
        }

     

        public void OnPropertyChanged(string prop)
        {
            PropertyChangedEventArgs args = new PropertyChangedEventArgs(prop);
            if (PropertyChanged != null)
            {
                PropertyChanged(this, args);
            }
        }
         
    }
}
