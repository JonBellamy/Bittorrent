using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Input;

namespace MeerkatUI
{
    public sealed class RemoveTorrentCommand : TorrentCommand, ICommand
    {
        private readonly MeerkatWindowViewModel vm;

        public RemoveTorrentCommand(MeerkatWindowViewModel vm)
        {
            this.vm = vm;
        }


        public bool CanExecute(object parameter)
        {
            return (vm.SelectedTorrent != null);
        }


        public void Execute(object parameter)
        {
            vm.RemoveTorrent();
        }
    }
}
