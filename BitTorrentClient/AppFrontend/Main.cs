using System;
using System.Collections.Generic;
//using System.Linq;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Threading;


static class Program
{
    [STAThread]
    static void Main()
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);

		// Make sure no other instances of this app are running
		bool mutexIsAvailable = false;
		Mutex m = null;
		try
		{
			string mutexName = System.Reflection.Assembly.GetExecutingAssembly().GetName().Name;
			m = new Mutex(true, mutexName);
			mutexIsAvailable = m.WaitOne(1, false); // wait only 1 ms
		}
		catch (AbandonedMutexException)
		{
			// don't worry about the abandonment;
			// the mutex only guards app instantiation
			mutexIsAvailable = true;
		}
		if (mutexIsAvailable)
		{
			try
			{				
				Application.Idle += new EventHandler(OnApplicationIdle);

				Application.Run(new MainForm());
			}
			catch (Exception e)
			{
				MessageBox.Show(String.Format("Unhandled Exception: {0}  {1}", System.IO.Directory.GetCurrentDirectory(), e.ToString()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Information);
			}
			finally
			{
				m.ReleaseMutex();
			}
		}
    }


	private static void OnApplicationIdle(object sender, EventArgs e)
	{
		while (AppStillIdle)
		{
			// idle time (no messages are waiting)
			DllInterface.UpdateTorrentManager();
            Thread.Sleep(1);
		}
	}

	private static bool AppStillIdle
	{
		get
		{
			NativeMessage message = new NativeMessage();
			return !PeekMessage(out message, IntPtr.Zero, 0, 0, 0);
		}
	}


	[StructLayout(LayoutKind.Sequential)]
	public struct NativeMessage
	{
		public IntPtr handle;
		public uint msg;
		public IntPtr wParam;
		public IntPtr lParam;
		public uint time;
		public System.Drawing.Point p;
	}

	[DllImport("user32.dll")]
	[return: MarshalAs(UnmanagedType.Bool)]
	static extern bool PeekMessage(out NativeMessage lpMsg, IntPtr hWnd, uint wMsgFilterMin, uint wMsgFilterMax, uint wRemoveMsg);
}
