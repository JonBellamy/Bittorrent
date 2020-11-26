using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
//using System.Linq;
using System.Text;
using System.Windows.Forms;


public partial class OptionsDialog:Form 
{
	public DllInterface.sTorrentClientOptions mOptions;

	public OptionsDialog() 	
	{
		InitializeComponent();
	}

	private void OptionsDialog_Load(object sender, EventArgs e)
	{
		DllInterface.GetTorrentClientOptions(ref mOptions);
		mOptionCheck_ForceEncryption.Checked = (mOptions.mAllConnectionsMustBeEncrypted != 0);
		mOptionCheck_EnableDht.Checked = (mOptions.mUseDht != 0);
		mOptionCheck_UseTrackers.Checked = (mOptions.mUseTrackers != 0);
		mOptionCheck_CheckForBuild.Checked = (mOptions.mCheckForLatestBuild != 0);		
		mOptionCheck_StopOnCompletionTorrents.Checked = (mOptions.mStopOnCompletion != 0);
		mEditbox_MaxUploadRate.Text = (mOptions.mMaxUploadRate /1024).ToString();
		mEditbox_ListenPort.Text = DllInterface.GetListenerPort().ToString();
	}
	

	private void OptionsDialog_FormClosing(object sender, FormClosingEventArgs e)
	{
	}


	private void mOkButton_Click(object sender, EventArgs e)
	{
		// write all options back to the struct
		mOptions.mAllConnectionsMustBeEncrypted = (byte)(mOptionCheck_ForceEncryption.Checked ? 1 : 0);
		mOptions.mStopOnCompletion = (byte)(mOptionCheck_StopOnCompletionTorrents.Checked ? 1 : 0);
		mOptions.mUseDht = (byte)(mOptionCheck_EnableDht.Checked ? 1 : 0);
		mOptions.mUseTrackers = (byte)(mOptionCheck_UseTrackers.Checked ? 1 : 0);
		mOptions.mCheckForLatestBuild = (byte)(mOptionCheck_CheckForBuild.Checked ? 1 : 0);		
		mOptions.mMaxUploadRate = (Convert.ToUInt32(mEditbox_MaxUploadRate.Text) * 1024);
		DllInterface.SetListenerPort((Convert.ToUInt16(mEditbox_ListenPort.Text)));
	}

	// Only allow numbers in the upload rate textbox
	private void mEditbox_MaxUploadRate_Leave(object sender, EventArgs e)
	{
		try
		{
			mEditbox_MaxUploadRate.Text = Convert.ToUInt32(mEditbox_MaxUploadRate.Text).ToString();
		}
		catch
		{
			MessageBox.Show("You can only type a number here.");
			mEditbox_MaxUploadRate.Text = mOptions.mMaxUploadRate.ToString();
			mEditbox_MaxUploadRate.Focus();
		}
	}

	private void mEditbox_ListenPort_Leave(object sender, EventArgs e)
	{
		try
		{
			mEditbox_ListenPort.Text = Convert.ToUInt16(mEditbox_ListenPort.Text).ToString();
		}
		catch
		{
			MessageBox.Show("You can only type a number here (0-65535). You should use a port number higher than 1024.");
			mEditbox_ListenPort.Text = DllInterface.GetListenerPort().ToString();
			mEditbox_ListenPort.Focus();
		}
	}


}

