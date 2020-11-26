using System;


namespace MeerkatUI
{
    // This can be passed into our VM via the ctor or through dependency injection, it can then be mocked for testing
    public interface IOpenFileService
    {
        string FileName { get; set; }
        bool OpenFileDialog();
    }


    public sealed class OpenFileService : IOpenFileService
    {
        public string FileName { get; set; }
       
        public bool OpenFileDialog()
        {
            // Create OpenFileDialog
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

            // Set filter for file extension and default file extension
            dlg.DefaultExt = ".torrent";
            dlg.Filter = "All files (*.*)|*.*|torrent files (*.torrent)|*.torrent";

            // Display OpenFileDialog
            Nullable<bool> result = dlg.ShowDialog();
            if (result == true)
            {
                FileName = dlg.FileName;
                return true;
            }
            return false;
        }
    }
}
