using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Input;

namespace MeerkatUI
{
     public sealed class TorrentRecheckCommand : TorrentCommand, ICommand
    {
        private readonly MeerkatWindowViewModel vm;

        public TorrentRecheckCommand(MeerkatWindowViewModel vm)
        {
            this.vm = vm;
        }
        

        public bool CanExecute(object parameter)
        {
            return (vm.SelectedTorrent != null);
        }


        public void Execute(object parameter)
        {
        }
    }
}
