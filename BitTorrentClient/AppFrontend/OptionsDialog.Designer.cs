partial class OptionsDialog 
{
	/// <summary>
	/// Required designer variable.
	/// </summary>
	private System.ComponentModel.IContainer components = null;

	/// <summary>
	/// Clean up any resources being used.
	/// </summary>
	/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
	protected override void Dispose(bool disposing) {
		if(disposing && (components != null)) {
			components.Dispose();
		}
		base.Dispose(disposing);
	}

	#region Windows Form Designer generated code

	/// <summary>
	/// Required method for Designer support - do not modify
	/// the contents of this method with the code editor.
	/// </summary>
	private void InitializeComponent() {
		System.Windows.Forms.GroupBox groupBox1;
		System.Windows.Forms.GroupBox groupBox2;
		this.mOkButton = new System.Windows.Forms.Button();
		this.mCancelButton = new System.Windows.Forms.Button();
		this.mOptionCheck_UseTrackers = new System.Windows.Forms.CheckBox();
		this.label2 = new System.Windows.Forms.Label();
		this.mEditbox_ListenPort = new System.Windows.Forms.TextBox();
		this.label1 = new System.Windows.Forms.Label();
		this.mEditbox_MaxUploadRate = new System.Windows.Forms.TextBox();
		this.mOptionCheck_EnableDht = new System.Windows.Forms.CheckBox();
		this.mOptionCheck_StopOnCompletionTorrents = new System.Windows.Forms.CheckBox();
		this.mOptionCheck_ForceEncryption = new System.Windows.Forms.CheckBox();
		this.mOptionCheck_CheckForBuild = new System.Windows.Forms.CheckBox();
		groupBox1 = new System.Windows.Forms.GroupBox();
		groupBox2 = new System.Windows.Forms.GroupBox();
		groupBox1.SuspendLayout();
		groupBox2.SuspendLayout();
		this.SuspendLayout();
		// 
		// groupBox1
		// 
		groupBox1.Controls.Add(this.mOkButton);
		groupBox1.Controls.Add(this.mCancelButton);
		groupBox1.Location = new System.Drawing.Point(-14, 473);
		groupBox1.Name = "groupBox1";
		groupBox1.Size = new System.Drawing.Size(539, 91);
		groupBox1.TabIndex = 2;
		groupBox1.TabStop = false;
		// 
		// mOkButton
		// 
		this.mOkButton.DialogResult = System.Windows.Forms.DialogResult.OK;
		this.mOkButton.Location = new System.Drawing.Point(328, 21);
		this.mOkButton.Name = "mOkButton";
		this.mOkButton.Size = new System.Drawing.Size(75, 23);
		this.mOkButton.TabIndex = 1;
		this.mOkButton.Text = "OK";
		this.mOkButton.UseVisualStyleBackColor = true;
		this.mOkButton.Click += new System.EventHandler(this.mOkButton_Click);
		// 
		// mCancelButton
		// 
		this.mCancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
		this.mCancelButton.Location = new System.Drawing.Point(409, 21);
		this.mCancelButton.Name = "mCancelButton";
		this.mCancelButton.Size = new System.Drawing.Size(75, 23);
		this.mCancelButton.TabIndex = 0;
		this.mCancelButton.Text = "Cancel";
		this.mCancelButton.UseVisualStyleBackColor = true;
		// 
		// groupBox2
		// 
		groupBox2.Controls.Add(this.mOptionCheck_CheckForBuild);
		groupBox2.Controls.Add(this.mOptionCheck_UseTrackers);
		groupBox2.Controls.Add(this.label2);
		groupBox2.Controls.Add(this.mEditbox_ListenPort);
		groupBox2.Controls.Add(this.label1);
		groupBox2.Controls.Add(this.mEditbox_MaxUploadRate);
		groupBox2.Controls.Add(this.mOptionCheck_EnableDht);
		groupBox2.Controls.Add(this.mOptionCheck_StopOnCompletionTorrents);
		groupBox2.Controls.Add(this.mOptionCheck_ForceEncryption);
		groupBox2.Location = new System.Drawing.Point(15, 19);
		groupBox2.Name = "groupBox2";
		groupBox2.Size = new System.Drawing.Size(454, 438);
		groupBox2.TabIndex = 3;
		groupBox2.TabStop = false;
		groupBox2.Text = "Meerkat Options";
		// 
		// mOptionCheck_UseTrackers
		// 
		this.mOptionCheck_UseTrackers.AutoSize = true;
		this.mOptionCheck_UseTrackers.Checked = true;
		this.mOptionCheck_UseTrackers.CheckState = System.Windows.Forms.CheckState.Checked;
		this.mOptionCheck_UseTrackers.Location = new System.Drawing.Point(7, 102);
		this.mOptionCheck_UseTrackers.Name = "mOptionCheck_UseTrackers";
		this.mOptionCheck_UseTrackers.Size = new System.Drawing.Size(90, 17);
		this.mOptionCheck_UseTrackers.TabIndex = 7;
		this.mOptionCheck_UseTrackers.Text = "Use Trackers";
		this.mOptionCheck_UseTrackers.UseVisualStyleBackColor = true;
		// 
		// label2
		// 
		this.label2.AutoSize = true;
		this.label2.Location = new System.Drawing.Point(4, 174);
		this.label2.Name = "label2";
		this.label2.Size = new System.Drawing.Size(132, 13);
		this.label2.TabIndex = 6;
		this.label2.Text = "Incoming connections port";
		// 
		// mEditbox_ListenPort
		// 
		this.mEditbox_ListenPort.Location = new System.Drawing.Point(152, 174);
		this.mEditbox_ListenPort.Name = "mEditbox_ListenPort";
		this.mEditbox_ListenPort.Size = new System.Drawing.Size(100, 20);
		this.mEditbox_ListenPort.TabIndex = 5;
		this.mEditbox_ListenPort.Leave += new System.EventHandler(this.mEditbox_ListenPort_Leave);
		// 
		// label1
		// 
		this.label1.AutoSize = true;
		this.label1.Location = new System.Drawing.Point(4, 148);
		this.label1.Name = "label1";
		this.label1.Size = new System.Drawing.Size(139, 13);
		this.label1.TabIndex = 4;
		this.label1.Text = "Maximum upload rate (kB/s)";
		// 
		// mEditbox_MaxUploadRate
		// 
		this.mEditbox_MaxUploadRate.Location = new System.Drawing.Point(152, 148);
		this.mEditbox_MaxUploadRate.Name = "mEditbox_MaxUploadRate";
		this.mEditbox_MaxUploadRate.Size = new System.Drawing.Size(100, 20);
		this.mEditbox_MaxUploadRate.TabIndex = 3;
		this.mEditbox_MaxUploadRate.Leave += new System.EventHandler(this.mEditbox_MaxUploadRate_Leave);
		// 
		// mOptionCheck_EnableDht
		// 
		this.mOptionCheck_EnableDht.AutoSize = true;
		this.mOptionCheck_EnableDht.Checked = true;
		this.mOptionCheck_EnableDht.CheckState = System.Windows.Forms.CheckState.Checked;
		this.mOptionCheck_EnableDht.Location = new System.Drawing.Point(7, 81);
		this.mOptionCheck_EnableDht.Name = "mOptionCheck_EnableDht";
		this.mOptionCheck_EnableDht.Size = new System.Drawing.Size(79, 17);
		this.mOptionCheck_EnableDht.TabIndex = 2;
		this.mOptionCheck_EnableDht.Text = "Enable Dht";
		this.mOptionCheck_EnableDht.UseVisualStyleBackColor = true;
		// 
		// mOptionCheck_StopOnCompletionTorrents
		// 
		this.mOptionCheck_StopOnCompletionTorrents.AutoSize = true;
		this.mOptionCheck_StopOnCompletionTorrents.Location = new System.Drawing.Point(7, 58);
		this.mOptionCheck_StopOnCompletionTorrents.Name = "mOptionCheck_StopOnCompletionTorrents";
		this.mOptionCheck_StopOnCompletionTorrents.Size = new System.Drawing.Size(189, 17);
		this.mOptionCheck_StopOnCompletionTorrents.TabIndex = 1;
		this.mOptionCheck_StopOnCompletionTorrents.Text = "Always stop torrents on completion";
		this.mOptionCheck_StopOnCompletionTorrents.UseVisualStyleBackColor = true;
		// 
		// mOptionCheck_ForceEncryption
		// 
		this.mOptionCheck_ForceEncryption.AutoSize = true;
		this.mOptionCheck_ForceEncryption.Location = new System.Drawing.Point(7, 34);
		this.mOptionCheck_ForceEncryption.Name = "mOptionCheck_ForceEncryption";
		this.mOptionCheck_ForceEncryption.Size = new System.Drawing.Size(188, 17);
		this.mOptionCheck_ForceEncryption.TabIndex = 0;
		this.mOptionCheck_ForceEncryption.Text = "All connections must be encrypted";
		this.mOptionCheck_ForceEncryption.UseVisualStyleBackColor = true;
		// 
		// mOptionCheck_CheckForBuild
		// 
		this.mOptionCheck_CheckForBuild.AutoSize = true;
		this.mOptionCheck_CheckForBuild.Checked = true;
		this.mOptionCheck_CheckForBuild.CheckState = System.Windows.Forms.CheckState.Checked;
		this.mOptionCheck_CheckForBuild.Location = new System.Drawing.Point(7, 125);
		this.mOptionCheck_CheckForBuild.Name = "mOptionCheck_CheckForBuild";
		this.mOptionCheck_CheckForBuild.Size = new System.Drawing.Size(175, 17);
		this.mOptionCheck_CheckForBuild.TabIndex = 8;
		this.mOptionCheck_CheckForBuild.Text = "Check for latest build on startup";
		this.mOptionCheck_CheckForBuild.UseVisualStyleBackColor = true;
		// 
		// OptionsDialog
		// 
		this.AcceptButton = this.mOkButton;
		this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
		this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
		this.CancelButton = this.mCancelButton;
		this.ClientSize = new System.Drawing.Size(482, 529);
		this.ControlBox = false;
		this.Controls.Add(groupBox2);
		this.Controls.Add(groupBox1);
		this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
		this.MaximizeBox = false;
		this.MinimizeBox = false;
		this.Name = "OptionsDialog";
		this.ShowInTaskbar = false;
		this.Text = "Options";
		this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.OptionsDialog_FormClosing);
		this.Load += new System.EventHandler(this.OptionsDialog_Load);
		groupBox1.ResumeLayout(false);
		groupBox2.ResumeLayout(false);
		groupBox2.PerformLayout();
		this.ResumeLayout(false);

	}

	#endregion

	private System.Windows.Forms.Button mCancelButton;
	private System.Windows.Forms.Button mOkButton;
	private System.Windows.Forms.CheckBox mOptionCheck_EnableDht;
	private System.Windows.Forms.CheckBox mOptionCheck_StopOnCompletionTorrents;
	private System.Windows.Forms.CheckBox mOptionCheck_ForceEncryption;
	private System.Windows.Forms.Label label1;
	private System.Windows.Forms.TextBox mEditbox_MaxUploadRate;
	private System.Windows.Forms.Label label2;
	private System.Windows.Forms.TextBox mEditbox_ListenPort;
	private System.Windows.Forms.CheckBox mOptionCheck_UseTrackers;
	private System.Windows.Forms.CheckBox mOptionCheck_CheckForBuild;
}