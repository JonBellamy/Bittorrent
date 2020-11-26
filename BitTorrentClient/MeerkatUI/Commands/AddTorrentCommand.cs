using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Input;

namespace MeerkatUI
{
     public sealed class AddTorrentCommand : TorrentCommand, ICommand
    {
        private readonly MeerkatWindowViewModel vm;

        public AddTorrentCommand(MeerkatWindowViewModel vm)
        {
            this.vm = vm;
        }
        

        public bool CanExecute(object parameter)
        {
            return true;
        }


        public void Execute(object parameter)
        {
            vm.AddTorrent();
        }
    }
}
