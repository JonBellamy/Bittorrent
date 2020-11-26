using System;
using System.Windows.Forms;

namespace MeerkatUI
{
    // This can be passed into our VM via the ctor or through dependency injection, it can then be mocked for testing
    public interface IFolderBrowerService
    {
        string FolderName { get; set; }
        bool OpenFolderBrowserDialog();
    }


    public sealed class FolderBrowerService : IFolderBrowerService
    {
        public string FolderName { get; set; }

        public bool OpenFolderBrowserDialog()
        {
            var dialog = new System.Windows.Forms.FolderBrowserDialog();
            System.Windows.Forms.DialogResult result = dialog.ShowDialog();
            if (result == DialogResult.OK)
            {
                FolderName = dialog.SelectedPath;
                return true;
            }
            return false;
        }
    }
}
