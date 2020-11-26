partial class MainForm
{
    /// <summary>
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
    protected override void Dispose(bool disposing)
    {
        if (disposing && (components != null))
        {
            components.Dispose();
        }
        base.Dispose(disposing);
    }

    #region Windows Form Designer generated code

    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent()
    {
        this.components = new System.ComponentModel.Container();
        System.Windows.Forms.TextBox textBox1;
        System.Windows.Forms.Label label1;
        System.Windows.Forms.Label label3;
        System.Windows.Forms.Label label5;
        System.Windows.Forms.Label label6;
        System.Windows.Forms.TextBox textBox3;
        System.Windows.Forms.Label label15;
        System.Windows.Forms.Label label16;
        System.Windows.Forms.Label label17;
        System.Windows.Forms.Label label18;
        System.Windows.Forms.Label label19;
        System.Windows.Forms.Label label20;
        System.Windows.Forms.ColumnHeader ColumnHeaderName;
        System.Windows.Forms.ColumnHeader ColumnHeaderEta;
        System.Windows.Forms.ColumnHeader ColumnHeaderId;
        System.Windows.Forms.ColumnHeader ColumnHeaderDownSpeed;
        System.Windows.Forms.ColumnHeader ColumnHeaderUpSpeed;
        System.Windows.Forms.ColumnHeader ColumnHeaderDone;
        System.Windows.Forms.ColumnHeader Ip;
        System.Windows.Forms.ColumnHeader DownloadRate;
        System.Windows.Forms.ColumnHeader UploadRate;
        System.Windows.Forms.ColumnHeader Requests;
        System.Windows.Forms.ColumnHeader PeerId;
        System.Windows.Forms.ColumnHeader columnHeader1;
        System.Windows.Forms.ColumnHeader columnHeader2;
        System.Windows.Forms.ColumnHeader columnHeader3;
        System.Windows.Forms.ColumnHeader columnHeader4;
        System.Windows.Forms.ColumnHeader columnHeader5;
        System.Windows.Forms.ColumnHeader Flags;
        System.Windows.Forms.ColumnHeader ConnectionType;
        System.Windows.Forms.ColumnHeader ColumnHeaderStatus;
        System.Windows.Forms.ColumnHeader ColumnHeaderSeeds;
        System.Windows.Forms.ColumnHeader ColumnHeaderPeers;
        System.Windows.Forms.ColumnHeader ColumnHeaderSize;
        System.Windows.Forms.ColumnHeader mNumberColumn;
        System.Windows.Forms.ColumnHeader mSizeColumn;
        System.Windows.Forms.ColumnHeader mNumberOfCompletedBlocksColumn;
        System.Windows.Forms.ColumnHeader mNumberOfRequestedBlocksColumn;
        System.Windows.Forms.ColumnHeader NumberOfBlocksColumn;
        System.Windows.Forms.ColumnHeader mFileListViewColumnName;
        System.Windows.Forms.ColumnHeader mFileListViewColumnSize;
        System.Windows.Forms.ColumnHeader mFileListViewColumnPercentageDone;
        System.Windows.Forms.ColumnHeader mFileListViewColumnNumPieces;
        System.Windows.Forms.ColumnHeader Downloaded;
        System.Windows.Forms.ColumnHeader ConnectionLength;
        System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
        this.mAppMenu = new System.Windows.Forms.MenuStrip();
        this.mMenuFileRoot = new System.Windows.Forms.ToolStripMenuItem();
        this.addTorrentToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
        this.exitToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
        this.mMenuToolsRoot = new System.Windows.Forms.ToolStripMenuItem();
        this.mMenuToolsOptions = new System.Windows.Forms.ToolStripMenuItem();
        this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
        this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
        this.mAppToolStrip = new System.Windows.Forms.ToolStrip();
        this.AddTorrentButton = new System.Windows.Forms.ToolStripButton();
        this.RemoveTorrentButton = new System.Windows.Forms.ToolStripButton();
        this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
        this.StartTorrentButton = new System.Windows.Forms.ToolStripButton();
        this.PauseTorrentButton = new System.Windows.Forms.ToolStripButton();
        this.StopTorrentButton = new System.Windows.Forms.ToolStripButton();
        this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
        this.OptionsButton = new System.Windows.Forms.ToolStripButton();
        this.toolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
        this.mAppStatusStrip = new System.Windows.Forms.StatusStrip();
        this.mStatusStripDhtLabel = new System.Windows.Forms.ToolStripStatusLabel();
        this.mStatusStripDlLabel = new System.Windows.Forms.ToolStripStatusLabel();
        this.mStatusStripUlLabel = new System.Windows.Forms.ToolStripStatusLabel();
        this.mSplitContainer = new System.Windows.Forms.SplitContainer();
        this.mListView = new System.Windows.Forms.ListView();
        this.mMainListViewRightClickMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
        this.openContainingFolderToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
        this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
        this.removeTorrentToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
        this.removeAndDeleteTorrentToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
        this.toolStripSeparator5 = new System.Windows.Forms.ToolStripSeparator();
        this.toolStripSeparator6 = new System.Windows.Forms.ToolStripSeparator();
        this.forceRecheckToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
        this.toolStripSeparator7 = new System.Windows.Forms.ToolStripSeparator();
        this.mMenuItemAllowUnencryptedComs = new System.Windows.Forms.ToolStripMenuItem();
        this.mAppImageList = new System.Windows.Forms.ImageList(this.components);
        this.mAppTabControl = new System.Windows.Forms.TabControl();
        this.mTabPageGeneral = new System.Windows.Forms.TabPage();
        this.mGeneralTabLabelLocalName = new System.Windows.Forms.TextBox();
        this.mGeneralTabLabelComment = new System.Windows.Forms.Label();
        this.mGeneralTabLabelHash = new System.Windows.Forms.Label();
        this.mGeneralTabLabelPieces = new System.Windows.Forms.Label();
        this.mGeneralTabLabelTotalSize = new System.Windows.Forms.Label();
        this.mGeneralTabLabelCreatedOn = new System.Windows.Forms.Label();
        this.mGeneralTabLabelShareRatio = new System.Windows.Forms.Label();
        this.mGeneralTabLabelRemaining = new System.Windows.Forms.Label();
        this.mGeneralTabLabelDownloaded = new System.Windows.Forms.Label();
        this.mGeneralTabLabelTimeElapsed = new System.Windows.Forms.Label();
        this.mTabPageTracker = new System.Windows.Forms.TabPage();
        this.mTrackerListView = new System.Windows.Forms.ListView();
        this.mTabPagePeers = new System.Windows.Forms.TabPage();
        this.mPeersListView = new System.Windows.Forms.ListView();
        this.PercentDone = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        this.Uploaded = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        this.mTabPagePieces = new System.Windows.Forms.TabPage();
        this.mPiecesListView = new System.Windows.Forms.ListView();
        this.mTabPageFiles = new System.Windows.Forms.TabPage();
        this.mFilesListView = new System.Windows.Forms.ListView();
        this.mTabPageBandwidth = new System.Windows.Forms.TabPage();
        this.mZedGraphControl = new ZedGraph.ZedGraphControl();
        this.mTabPageOutput = new System.Windows.Forms.TabPage();
        this.mAppNotifyIcon = new System.Windows.Forms.NotifyIcon(this.components);
        this.mAppSystemTrayMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
        this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
        textBox1 = new System.Windows.Forms.TextBox();
        label1 = new System.Windows.Forms.Label();
        label3 = new System.Windows.Forms.Label();
        label5 = new System.Windows.Forms.Label();
        label6 = new System.Windows.Forms.Label();
        textBox3 = new System.Windows.Forms.TextBox();
        label15 = new System.Windows.Forms.Label();
        label16 = new System.Windows.Forms.Label();
        label17 = new System.Windows.Forms.Label();
        label18 = new System.Windows.Forms.Label();
        label19 = new System.Windows.Forms.Label();
        label20 = new System.Windows.Forms.Label();
        ColumnHeaderName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        ColumnHeaderEta = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        ColumnHeaderId = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        ColumnHeaderDownSpeed = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        ColumnHeaderUpSpeed = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        ColumnHeaderDone = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        Ip = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        DownloadRate = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        UploadRate = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        Requests = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        PeerId = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        Flags = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        ConnectionType = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        ColumnHeaderStatus = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        ColumnHeaderSeeds = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        ColumnHeaderPeers = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        ColumnHeaderSize = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        mNumberColumn = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        mSizeColumn = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        mNumberOfCompletedBlocksColumn = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        mNumberOfRequestedBlocksColumn = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        NumberOfBlocksColumn = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        mFileListViewColumnName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        mFileListViewColumnSize = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        mFileListViewColumnPercentageDone = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        mFileListViewColumnNumPieces = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        Downloaded = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        ConnectionLength = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
        this.mAppMenu.SuspendLayout();
        this.mAppToolStrip.SuspendLayout();
        this.mAppStatusStrip.SuspendLayout();
        ((System.ComponentModel.ISupportInitialize)(this.mSplitContainer)).BeginInit();
        this.mSplitContainer.Panel1.SuspendLayout();
        this.mSplitContainer.Panel2.SuspendLayout();
        this.mSplitContainer.SuspendLayout();
        this.mMainListViewRightClickMenu.SuspendLayout();
        this.mAppTabControl.SuspendLayout();
        this.mTabPageGeneral.SuspendLayout();
        this.mTabPageTracker.SuspendLayout();
        this.mTabPagePeers.SuspendLayout();
        this.mTabPagePieces.SuspendLayout();
        this.mTabPageFiles.SuspendLayout();
        this.mTabPageBandwidth.SuspendLayout();
        this.mAppSystemTrayMenu.SuspendLayout();
        this.SuspendLayout();
        // 
        // textBox1
        // 
        textBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                    | System.Windows.Forms.AnchorStyles.Right)));
        textBox1.BackColor = System.Drawing.SystemColors.MenuBar;
        textBox1.BorderStyle = System.Windows.Forms.BorderStyle.None;
        textBox1.Enabled = false;
        textBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
        textBox1.Location = new System.Drawing.Point(11, 37);
        textBox1.Name = "textBox1";
        textBox1.ReadOnly = true;
        textBox1.Size = new System.Drawing.Size(950, 13);
        textBox1.TabIndex = 0;
        textBox1.Text = "Transfer";
        // 
        // label1
        // 
        label1.AutoSize = true;
        label1.Location = new System.Drawing.Point(8, 57);
        label1.Name = "label1";
        label1.Size = new System.Drawing.Size(74, 13);
        label1.TabIndex = 1;
        label1.Text = "Time Elapsed:";
        // 
        // label3
        // 
        label3.AutoSize = true;
        label3.Location = new System.Drawing.Point(8, 82);
        label3.Name = "label3";
        label3.Size = new System.Drawing.Size(70, 13);
        label3.TabIndex = 3;
        label3.Text = "Downloaded:";
        // 
        // label5
        // 
        label5.AutoSize = true;
        label5.Location = new System.Drawing.Point(197, 57);
        label5.Name = "label5";
        label5.Size = new System.Drawing.Size(60, 13);
        label5.TabIndex = 5;
        label5.Text = "Remaining:";
        // 
        // label6
        // 
        label6.AutoSize = true;
        label6.Location = new System.Drawing.Point(398, 57);
        label6.Name = "label6";
        label6.Size = new System.Drawing.Size(66, 13);
        label6.TabIndex = 6;
        label6.Text = "Share Ratio:";
        // 
        // textBox3
        // 
        textBox3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                    | System.Windows.Forms.AnchorStyles.Right)));
        textBox3.BackColor = System.Drawing.SystemColors.MenuBar;
        textBox3.BorderStyle = System.Windows.Forms.BorderStyle.None;
        textBox3.Enabled = false;
        textBox3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
        textBox3.Location = new System.Drawing.Point(8, 126);
        textBox3.Name = "textBox3";
        textBox3.ReadOnly = true;
        textBox3.Size = new System.Drawing.Size(953, 13);
        textBox3.TabIndex = 10;
        textBox3.Text = "General";
        // 
        // label15
        // 
        label15.AutoSize = true;
        label15.Location = new System.Drawing.Point(8, 151);
        label15.Name = "label15";
        label15.Size = new System.Drawing.Size(67, 13);
        label15.TabIndex = 17;
        label15.Text = "Local Name:";
        // 
        // label16
        // 
        label16.AutoSize = true;
        label16.Location = new System.Drawing.Point(8, 228);
        label16.Name = "label16";
        label16.Size = new System.Drawing.Size(54, 13);
        label16.TabIndex = 18;
        label16.Text = "Comment:";
        // 
        // label17
        // 
        label17.AutoSize = true;
        label17.Location = new System.Drawing.Point(601, 151);
        label17.Name = "label17";
        label17.Size = new System.Drawing.Size(35, 13);
        label17.TabIndex = 19;
        label17.Text = "Hash:";
        // 
        // label18
        // 
        label18.AutoSize = true;
        label18.Location = new System.Drawing.Point(8, 176);
        label18.Name = "label18";
        label18.Size = new System.Drawing.Size(42, 13);
        label18.TabIndex = 20;
        label18.Text = "Pieces:";
        // 
        // label19
        // 
        label19.AutoSize = true;
        label19.Location = new System.Drawing.Point(400, 151);
        label19.Name = "label19";
        label19.Size = new System.Drawing.Size(64, 13);
        label19.TabIndex = 21;
        label19.Text = "Created On:";
        // 
        // label20
        // 
        label20.AutoSize = true;
        label20.Location = new System.Drawing.Point(8, 202);
        label20.Name = "label20";
        label20.Size = new System.Drawing.Size(57, 13);
        label20.TabIndex = 22;
        label20.Text = "Total Size:";
        // 
        // ColumnHeaderName
        // 
        ColumnHeaderName.Text = "Name";
        ColumnHeaderName.Width = 300;
        // 
        // ColumnHeaderEta
        // 
        ColumnHeaderEta.Text = "Eta";
        ColumnHeaderEta.Width = 223;
        // 
        // ColumnHeaderId
        // 
        ColumnHeaderId.Text = "Id";
        ColumnHeaderId.Width = 50;
        // 
        // ColumnHeaderDownSpeed
        // 
        ColumnHeaderDownSpeed.Text = "Down Speed";
        ColumnHeaderDownSpeed.Width = 75;
        // 
        // ColumnHeaderUpSpeed
        // 
        ColumnHeaderUpSpeed.Text = "Up Speed";
        ColumnHeaderUpSpeed.Width = 75;
        // 
        // ColumnHeaderDone
        // 
        ColumnHeaderDone.Text = "Done";
        // 
        // Ip
        // 
        Ip.Text = "Ip";
        Ip.Width = 180;
        // 
        // DownloadRate
        // 
        DownloadRate.Text = "Down Speed";
        DownloadRate.Width = 85;
        // 
        // UploadRate
        // 
        UploadRate.Text = "Up Speed";
        UploadRate.Width = 80;
        // 
        // Requests
        // 
        Requests.Text = "Reqs";
        // 
        // PeerId
        // 
        PeerId.Text = "Peer Id";
        PeerId.Width = 180;
        // 
        // columnHeader1
        // 
        columnHeader1.Text = "Url";
        columnHeader1.Width = 504;
        // 
        // columnHeader2
        // 
        columnHeader2.Text = "Update In";
        columnHeader2.Width = 113;
        // 
        // columnHeader3
        // 
        columnHeader3.Text = "Seeds";
        columnHeader3.Width = 85;
        // 
        // columnHeader4
        // 
        columnHeader4.Text = "Peers";
        columnHeader4.Width = 80;
        // 
        // columnHeader5
        // 
        columnHeader5.Text = "Interval";
        columnHeader5.Width = 119;
        // 
        // Flags
        // 
        Flags.Text = "Flags";
        // 
        // ConnectionType
        // 
        ConnectionType.Text = "Connection Type";
        ConnectionType.Width = 130;
        // 
        // ColumnHeaderStatus
        // 
        ColumnHeaderStatus.Text = "Status";
        ColumnHeaderStatus.Width = 100;
        // 
        // ColumnHeaderSeeds
        // 
        ColumnHeaderSeeds.Text = "Seeds";
        // 
        // ColumnHeaderPeers
        // 
        ColumnHeaderPeers.Text = "Peers";
        // 
        // ColumnHeaderSize
        // 
        ColumnHeaderSize.Text = "Size";
        ColumnHeaderSize.Width = 83;
        // 
        // mNumberColumn
        // 
        mNumberColumn.Text = "#";
        // 
        // mSizeColumn
        // 
        mSizeColumn.Text = "Size";
        // 
        // mNumberOfCompletedBlocksColumn
        // 
        mNumberOfCompletedBlocksColumn.Text = "Downloaded Blocks";
        mNumberOfCompletedBlocksColumn.Width = 120;
        // 
        // mNumberOfRequestedBlocksColumn
        // 
        mNumberOfRequestedBlocksColumn.Text = "Requested Blocks";
        mNumberOfRequestedBlocksColumn.Width = 100;
        // 
        // NumberOfBlocksColumn
        // 
        NumberOfBlocksColumn.Text = "# Blocks";
        // 
        // mFileListViewColumnName
        // 
        mFileListViewColumnName.Text = "Name";
        mFileListViewColumnName.Width = 507;
        // 
        // mFileListViewColumnSize
        // 
        mFileListViewColumnSize.Text = "Size";
        // 
        // mFileListViewColumnPercentageDone
        // 
        mFileListViewColumnPercentageDone.Text = "% Done";
        // 
        // mFileListViewColumnNumPieces
        // 
        mFileListViewColumnNumPieces.Text = "# Pieces";
        // 
        // Downloaded
        // 
        Downloaded.Text = "Downloaded";
        Downloaded.Width = 100;
        // 
        // ConnectionLength
        // 
        ConnectionLength.Text = "Connection Length";
        ConnectionLength.Width = 109;
        // 
        // mAppMenu
        // 
        this.mAppMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mMenuFileRoot,
            this.mMenuToolsRoot,
            this.helpToolStripMenuItem});
        this.mAppMenu.Location = new System.Drawing.Point(0, 0);
        this.mAppMenu.Name = "mAppMenu";
        this.mAppMenu.Size = new System.Drawing.Size(1198, 24);
        this.mAppMenu.TabIndex = 0;
        this.mAppMenu.Text = "AppMenu";
        // 
        // mMenuFileRoot
        // 
        this.mMenuFileRoot.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addTorrentToolStripMenuItem,
            this.exitToolStripMenuItem1});
        this.mMenuFileRoot.Name = "mMenuFileRoot";
        this.mMenuFileRoot.Size = new System.Drawing.Size(37, 20);
        this.mMenuFileRoot.Text = "File";
        // 
        // addTorrentToolStripMenuItem
        // 
        this.addTorrentToolStripMenuItem.Name = "addTorrentToolStripMenuItem";
        this.addTorrentToolStripMenuItem.Size = new System.Drawing.Size(138, 22);
        this.addTorrentToolStripMenuItem.Text = "&Add Torrent";
        this.addTorrentToolStripMenuItem.Click += new System.EventHandler(this.AddTorrentToolStripMenuItem_Click);
        // 
        // exitToolStripMenuItem1
        // 
        this.exitToolStripMenuItem1.Name = "exitToolStripMenuItem1";
        this.exitToolStripMenuItem1.Size = new System.Drawing.Size(138, 22);
        this.exitToolStripMenuItem1.Text = "E&xit";
        this.exitToolStripMenuItem1.Click += new System.EventHandler(this.ExitMenu_Click);
        // 
        // mMenuToolsRoot
        // 
        this.mMenuToolsRoot.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mMenuToolsOptions});
        this.mMenuToolsRoot.Name = "mMenuToolsRoot";
        this.mMenuToolsRoot.Size = new System.Drawing.Size(48, 20);
        this.mMenuToolsRoot.Text = "&Tools";
        // 
        // mMenuToolsOptions
        // 
        this.mMenuToolsOptions.Name = "mMenuToolsOptions";
        this.mMenuToolsOptions.Size = new System.Drawing.Size(116, 22);
        this.mMenuToolsOptions.Text = "&Options";
        this.mMenuToolsOptions.Click += new System.EventHandler(this.OptionsMenuItem_Click);
        // 
        // helpToolStripMenuItem
        // 
        this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem});
        this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
        this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
        this.helpToolStripMenuItem.Text = "&Help";
        // 
        // aboutToolStripMenuItem
        // 
        this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
        this.aboutToolStripMenuItem.Size = new System.Drawing.Size(107, 22);
        this.aboutToolStripMenuItem.Text = "&About";
        this.aboutToolStripMenuItem.Click += new System.EventHandler(this.AboutToolStripMenuItem_Click);
        // 
        // mAppToolStrip
        // 
        this.mAppToolStrip.ImageScalingSize = new System.Drawing.Size(24, 24);
        this.mAppToolStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.AddTorrentButton,
            this.RemoveTorrentButton,
            this.toolStripSeparator1,
            this.StartTorrentButton,
            this.PauseTorrentButton,
            this.StopTorrentButton,
            this.toolStripSeparator2,
            this.OptionsButton,
            this.toolStripSeparator4});
        this.mAppToolStrip.Location = new System.Drawing.Point(0, 24);
        this.mAppToolStrip.Name = "mAppToolStrip";
        this.mAppToolStrip.Size = new System.Drawing.Size(1198, 31);
        this.mAppToolStrip.TabIndex = 1;
        this.mAppToolStrip.Text = "toolStrip1";
        // 
        // AddTorrentButton
        // 
        this.AddTorrentButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
        this.AddTorrentButton.Image = global::AppFrontend.Properties.Resources.add_file;
        this.AddTorrentButton.ImageTransparentColor = System.Drawing.Color.Magenta;
        this.AddTorrentButton.Name = "AddTorrentButton";
        this.AddTorrentButton.Size = new System.Drawing.Size(28, 28);
        this.AddTorrentButton.Text = "Add Torrent";
        this.AddTorrentButton.Click += new System.EventHandler(this.AddTorrentButton_Click);
        // 
        // RemoveTorrentButton
        // 
        this.RemoveTorrentButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
        this.RemoveTorrentButton.Image = global::AppFrontend.Properties.Resources.delete;
        this.RemoveTorrentButton.ImageTransparentColor = System.Drawing.Color.Magenta;
        this.RemoveTorrentButton.Name = "RemoveTorrentButton";
        this.RemoveTorrentButton.Size = new System.Drawing.Size(28, 28);
        this.RemoveTorrentButton.Text = "Remove Torrent";
        this.RemoveTorrentButton.Click += new System.EventHandler(this.RemoveTorrentButton_Click);
        // 
        // toolStripSeparator1
        // 
        this.toolStripSeparator1.Name = "toolStripSeparator1";
        this.toolStripSeparator1.Size = new System.Drawing.Size(6, 31);
        // 
        // StartTorrentButton
        // 
        this.StartTorrentButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
        this.StartTorrentButton.Image = global::AppFrontend.Properties.Resources.play;
        this.StartTorrentButton.ImageTransparentColor = System.Drawing.Color.Magenta;
        this.StartTorrentButton.Name = "StartTorrentButton";
        this.StartTorrentButton.Size = new System.Drawing.Size(28, 28);
        this.StartTorrentButton.Text = "Start Torrent";
        this.StartTorrentButton.Click += new System.EventHandler(this.StartTorrentButton_Click);
        // 
        // PauseTorrentButton
        // 
        this.PauseTorrentButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
        this.PauseTorrentButton.Image = global::AppFrontend.Properties.Resources.pause;
        this.PauseTorrentButton.ImageTransparentColor = System.Drawing.Color.Magenta;
        this.PauseTorrentButton.Name = "PauseTorrentButton";
        this.PauseTorrentButton.Size = new System.Drawing.Size(28, 28);
        this.PauseTorrentButton.Text = "Pause Torrent";
        this.PauseTorrentButton.Click += new System.EventHandler(this.PauseTorrentButton_Click);
        // 
        // StopTorrentButton
        // 
        this.StopTorrentButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
        this.StopTorrentButton.Image = global::AppFrontend.Properties.Resources.stop;
        this.StopTorrentButton.ImageTransparentColor = System.Drawing.Color.Magenta;
        this.StopTorrentButton.Name = "StopTorrentButton";
        this.StopTorrentButton.Size = new System.Drawing.Size(28, 28);
        this.StopTorrentButton.Text = "Stop Torrent";
        this.StopTorrentButton.Click += new System.EventHandler(this.StopTorrentButton_Click);
        // 
        // toolStripSeparator2
        // 
        this.toolStripSeparator2.Name = "toolStripSeparator2";
        this.toolStripSeparator2.Size = new System.Drawing.Size(6, 31);
        // 
        // OptionsButton
        // 
        this.OptionsButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
        this.OptionsButton.Image = global::AppFrontend.Properties.Resources.Options;
        this.OptionsButton.ImageTransparentColor = System.Drawing.Color.Magenta;
        this.OptionsButton.Name = "OptionsButton";
        this.OptionsButton.Size = new System.Drawing.Size(28, 28);
        this.OptionsButton.Text = "Options";
        this.OptionsButton.Click += new System.EventHandler(this.OptionsMenuItem_Click);
        // 
        // toolStripSeparator4
        // 
        this.toolStripSeparator4.Name = "toolStripSeparator4";
        this.toolStripSeparator4.Size = new System.Drawing.Size(6, 31);
        // 
        // mAppStatusStrip
        // 
        this.mAppStatusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mStatusStripDhtLabel,
            this.mStatusStripDlLabel,
            this.mStatusStripUlLabel});
        this.mAppStatusStrip.Location = new System.Drawing.Point(0, 576);
        this.mAppStatusStrip.Name = "mAppStatusStrip";
        this.mAppStatusStrip.Size = new System.Drawing.Size(1198, 24);
        this.mAppStatusStrip.TabIndex = 2;
        this.mAppStatusStrip.Text = "AppStatusStrip";
        // 
        // mStatusStripDhtLabel
        // 
        this.mStatusStripDhtLabel.Name = "mStatusStripDhtLabel";
        this.mStatusStripDhtLabel.Size = new System.Drawing.Size(1067, 19);
        this.mStatusStripDhtLabel.Spring = true;
        this.mStatusStripDhtLabel.Text = "Dht Nodes: 0";
        this.mStatusStripDhtLabel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
        // 
        // mStatusStripDlLabel
        // 
        this.mStatusStripDlLabel.Name = "mStatusStripDlLabel";
        this.mStatusStripDlLabel.Size = new System.Drawing.Size(59, 19);
        this.mStatusStripDlLabel.Text = "D:128kB/s";
        this.mStatusStripDlLabel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
        // 
        // mStatusStripUlLabel
        // 
        this.mStatusStripUlLabel.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right)));
        this.mStatusStripUlLabel.BorderStyle = System.Windows.Forms.Border3DStyle.Etched;
        this.mStatusStripUlLabel.Name = "mStatusStripUlLabel";
        this.mStatusStripUlLabel.RightToLeft = System.Windows.Forms.RightToLeft.No;
        this.mStatusStripUlLabel.Size = new System.Drawing.Size(57, 19);
        this.mStatusStripUlLabel.Text = "U:64kB/s";
        // 
        // mSplitContainer
        // 
        this.mSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
        this.mSplitContainer.Location = new System.Drawing.Point(0, 55);
        this.mSplitContainer.Name = "mSplitContainer";
        this.mSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
        // 
        // mSplitContainer.Panel1
        // 
        this.mSplitContainer.Panel1.Controls.Add(this.mListView);
        // 
        // mSplitContainer.Panel2
        // 
        this.mSplitContainer.Panel2.Controls.Add(this.mAppTabControl);
        this.mSplitContainer.Size = new System.Drawing.Size(1198, 521);
        this.mSplitContainer.SplitterDistance = 225;
        this.mSplitContainer.TabIndex = 3;
        // 
        // mListView
        // 
        this.mListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            ColumnHeaderName,
            ColumnHeaderId,
            ColumnHeaderSize,
            ColumnHeaderDone,
            ColumnHeaderStatus,
            ColumnHeaderSeeds,
            ColumnHeaderPeers,
            ColumnHeaderDownSpeed,
            ColumnHeaderUpSpeed,
            ColumnHeaderEta});
        this.mListView.ContextMenuStrip = this.mMainListViewRightClickMenu;
        this.mListView.Dock = System.Windows.Forms.DockStyle.Fill;
        this.mListView.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
        this.mListView.FullRowSelect = true;
        this.mListView.HideSelection = false;
        this.mListView.Location = new System.Drawing.Point(0, 0);
        this.mListView.MultiSelect = false;
        this.mListView.Name = "mListView";
        this.mListView.Size = new System.Drawing.Size(1198, 225);
        this.mListView.SmallImageList = this.mAppImageList;
        this.mListView.TabIndex = 0;
        this.mListView.UseCompatibleStateImageBehavior = false;
        this.mListView.View = System.Windows.Forms.View.Details;
        this.mListView.SelectedIndexChanged += new System.EventHandler(this.mListView_SelectedIndexChanged);
        // 
        // mMainListViewRightClickMenu
        // 
        this.mMainListViewRightClickMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openContainingFolderToolStripMenuItem,
            this.toolStripSeparator3,
            this.removeTorrentToolStripMenuItem,
            this.removeAndDeleteTorrentToolStripMenuItem,
            this.toolStripSeparator5,
            this.toolStripSeparator6,
            this.forceRecheckToolStripMenuItem,
            this.toolStripSeparator7,
            this.mMenuItemAllowUnencryptedComs});
        this.mMainListViewRightClickMenu.Name = "mMainListViewRightClickMenu";
        this.mMainListViewRightClickMenu.Size = new System.Drawing.Size(326, 160);
        this.mMainListViewRightClickMenu.Opening += new System.ComponentModel.CancelEventHandler(this.mMainListViewRightClickMenu_Opening);
        // 
        // openContainingFolderToolStripMenuItem
        // 
        this.openContainingFolderToolStripMenuItem.Name = "openContainingFolderToolStripMenuItem";
        this.openContainingFolderToolStripMenuItem.Size = new System.Drawing.Size(325, 22);
        this.openContainingFolderToolStripMenuItem.Text = "Open Containing Folder";
        this.openContainingFolderToolStripMenuItem.Click += new System.EventHandler(this.openContainingFolderToolStripMenuItem_Click);
        // 
        // toolStripSeparator3
        // 
        this.toolStripSeparator3.Name = "toolStripSeparator3";
        this.toolStripSeparator3.Size = new System.Drawing.Size(322, 6);
        // 
        // removeTorrentToolStripMenuItem
        // 
        this.removeTorrentToolStripMenuItem.Name = "removeTorrentToolStripMenuItem";
        this.removeTorrentToolStripMenuItem.Size = new System.Drawing.Size(325, 22);
        this.removeTorrentToolStripMenuItem.Text = "Remove Torrent";
        this.removeTorrentToolStripMenuItem.Click += new System.EventHandler(this.RemoveTorrentToolStripMenuItem_Click);
        // 
        // removeAndDeleteTorrentToolStripMenuItem
        // 
        this.removeAndDeleteTorrentToolStripMenuItem.Name = "removeAndDeleteTorrentToolStripMenuItem";
        this.removeAndDeleteTorrentToolStripMenuItem.Size = new System.Drawing.Size(325, 22);
        this.removeAndDeleteTorrentToolStripMenuItem.Text = "Remove and Delete Torrent";
        this.removeAndDeleteTorrentToolStripMenuItem.Click += new System.EventHandler(this.RemoveAndDeleteTorrentToolStripMenuItem_Click);
        // 
        // toolStripSeparator5
        // 
        this.toolStripSeparator5.Name = "toolStripSeparator5";
        this.toolStripSeparator5.Size = new System.Drawing.Size(322, 6);
        // 
        // toolStripSeparator6
        // 
        this.toolStripSeparator6.Name = "toolStripSeparator6";
        this.toolStripSeparator6.Size = new System.Drawing.Size(322, 6);
        // 
        // forceRecheckToolStripMenuItem
        // 
        this.forceRecheckToolStripMenuItem.Name = "forceRecheckToolStripMenuItem";
        this.forceRecheckToolStripMenuItem.Size = new System.Drawing.Size(325, 22);
        this.forceRecheckToolStripMenuItem.Text = "Force Recheck";
        this.forceRecheckToolStripMenuItem.Click += new System.EventHandler(this.ForceRecheckToolStripMenuItem_Click);
        // 
        // toolStripSeparator7
        // 
        this.toolStripSeparator7.Name = "toolStripSeparator7";
        this.toolStripSeparator7.Size = new System.Drawing.Size(322, 6);
        // 
        // mMenuItemAllowUnencryptedComs
        // 
        this.mMenuItemAllowUnencryptedComs.Name = "mMenuItemAllowUnencryptedComs";
        this.mMenuItemAllowUnencryptedComs.Size = new System.Drawing.Size(325, 22);
        this.mMenuItemAllowUnencryptedComs.Text = "Allow UnEncrypted connections for this Torrent";
        this.mMenuItemAllowUnencryptedComs.Click += new System.EventHandler(this.AllowNonEncryptedConnectionsForThisTorrentToolStripMenuItem_Click);
        // 
        // mAppImageList
        // 
        this.mAppImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("mAppImageList.ImageStream")));
        this.mAppImageList.TransparentColor = System.Drawing.Color.Transparent;
        this.mAppImageList.Images.SetKeyName(0, "Tab_General.ico");
        this.mAppImageList.Images.SetKeyName(1, "Tab_Announce.ico");
        this.mAppImageList.Images.SetKeyName(2, "Tab_Peers.ico");
        this.mAppImageList.Images.SetKeyName(3, "Tab_Files.ico");
        this.mAppImageList.Images.SetKeyName(4, "Tab_TTY.ico");
        this.mAppImageList.Images.SetKeyName(5, "Pieces.ico");
        this.mAppImageList.Images.SetKeyName(6, "SecureConnection.ico");
        this.mAppImageList.Images.SetKeyName(7, "UnSecureConnection.ico");
        // 
        // mAppTabControl
        // 
        this.mAppTabControl.Controls.Add(this.mTabPageGeneral);
        this.mAppTabControl.Controls.Add(this.mTabPageTracker);
        this.mAppTabControl.Controls.Add(this.mTabPagePeers);
        this.mAppTabControl.Controls.Add(this.mTabPagePieces);
        this.mAppTabControl.Controls.Add(this.mTabPageFiles);
        this.mAppTabControl.Controls.Add(this.mTabPageBandwidth);
        this.mAppTabControl.Controls.Add(this.mTabPageOutput);
        this.mAppTabControl.Dock = System.Windows.Forms.DockStyle.Fill;
        this.mAppTabControl.ImageList = this.mAppImageList;
        this.mAppTabControl.Location = new System.Drawing.Point(0, 0);
        this.mAppTabControl.Name = "mAppTabControl";
        this.mAppTabControl.SelectedIndex = 0;
        this.mAppTabControl.Size = new System.Drawing.Size(1198, 292);
        this.mAppTabControl.TabIndex = 0;
        // 
        // mTabPageGeneral
        // 
        this.mTabPageGeneral.AutoScroll = true;
        this.mTabPageGeneral.Controls.Add(this.mGeneralTabLabelLocalName);
        this.mTabPageGeneral.Controls.Add(this.mGeneralTabLabelComment);
        this.mTabPageGeneral.Controls.Add(this.mGeneralTabLabelHash);
        this.mTabPageGeneral.Controls.Add(this.mGeneralTabLabelPieces);
        this.mTabPageGeneral.Controls.Add(this.mGeneralTabLabelTotalSize);
        this.mTabPageGeneral.Controls.Add(this.mGeneralTabLabelCreatedOn);
        this.mTabPageGeneral.Controls.Add(label20);
        this.mTabPageGeneral.Controls.Add(label19);
        this.mTabPageGeneral.Controls.Add(label18);
        this.mTabPageGeneral.Controls.Add(label17);
        this.mTabPageGeneral.Controls.Add(label16);
        this.mTabPageGeneral.Controls.Add(label15);
        this.mTabPageGeneral.Controls.Add(textBox3);
        this.mTabPageGeneral.Controls.Add(this.mGeneralTabLabelShareRatio);
        this.mTabPageGeneral.Controls.Add(this.mGeneralTabLabelRemaining);
        this.mTabPageGeneral.Controls.Add(label6);
        this.mTabPageGeneral.Controls.Add(label5);
        this.mTabPageGeneral.Controls.Add(this.mGeneralTabLabelDownloaded);
        this.mTabPageGeneral.Controls.Add(label3);
        this.mTabPageGeneral.Controls.Add(this.mGeneralTabLabelTimeElapsed);
        this.mTabPageGeneral.Controls.Add(label1);
        this.mTabPageGeneral.Controls.Add(textBox1);
        this.mTabPageGeneral.ImageIndex = 0;
        this.mTabPageGeneral.Location = new System.Drawing.Point(4, 31);
        this.mTabPageGeneral.Name = "mTabPageGeneral";
        this.mTabPageGeneral.Padding = new System.Windows.Forms.Padding(3);
        this.mTabPageGeneral.Size = new System.Drawing.Size(1190, 257);
        this.mTabPageGeneral.TabIndex = 0;
        this.mTabPageGeneral.Text = "General";
        this.mTabPageGeneral.UseVisualStyleBackColor = true;
        // 
        // mGeneralTabLabelLocalName
        // 
        this.mGeneralTabLabelLocalName.BackColor = System.Drawing.SystemColors.Window;
        this.mGeneralTabLabelLocalName.BorderStyle = System.Windows.Forms.BorderStyle.None;
        this.mGeneralTabLabelLocalName.Location = new System.Drawing.Point(91, 151);
        this.mGeneralTabLabelLocalName.Name = "mGeneralTabLabelLocalName";
        this.mGeneralTabLabelLocalName.ReadOnly = true;
        this.mGeneralTabLabelLocalName.Size = new System.Drawing.Size(291, 13);
        this.mGeneralTabLabelLocalName.TabIndex = 29;
        // 
        // mGeneralTabLabelComment
        // 
        this.mGeneralTabLabelComment.AutoSize = true;
        this.mGeneralTabLabelComment.Location = new System.Drawing.Point(85, 228);
        this.mGeneralTabLabelComment.Name = "mGeneralTabLabelComment";
        this.mGeneralTabLabelComment.Size = new System.Drawing.Size(64, 13);
        this.mGeneralTabLabelComment.TabIndex = 28;
        this.mGeneralTabLabelComment.Text = "                   ";
        // 
        // mGeneralTabLabelHash
        // 
        this.mGeneralTabLabelHash.AutoSize = true;
        this.mGeneralTabLabelHash.Location = new System.Drawing.Point(642, 151);
        this.mGeneralTabLabelHash.Name = "mGeneralTabLabelHash";
        this.mGeneralTabLabelHash.Size = new System.Drawing.Size(64, 13);
        this.mGeneralTabLabelHash.TabIndex = 27;
        this.mGeneralTabLabelHash.Text = "                   ";
        // 
        // mGeneralTabLabelPieces
        // 
        this.mGeneralTabLabelPieces.AutoSize = true;
        this.mGeneralTabLabelPieces.Location = new System.Drawing.Point(85, 176);
        this.mGeneralTabLabelPieces.Name = "mGeneralTabLabelPieces";
        this.mGeneralTabLabelPieces.Size = new System.Drawing.Size(64, 13);
        this.mGeneralTabLabelPieces.TabIndex = 26;
        this.mGeneralTabLabelPieces.Text = "                   ";
        // 
        // mGeneralTabLabelTotalSize
        // 
        this.mGeneralTabLabelTotalSize.AutoSize = true;
        this.mGeneralTabLabelTotalSize.Location = new System.Drawing.Point(85, 202);
        this.mGeneralTabLabelTotalSize.Name = "mGeneralTabLabelTotalSize";
        this.mGeneralTabLabelTotalSize.Size = new System.Drawing.Size(64, 13);
        this.mGeneralTabLabelTotalSize.TabIndex = 25;
        this.mGeneralTabLabelTotalSize.Text = "                   ";
        // 
        // mGeneralTabLabelCreatedOn
        // 
        this.mGeneralTabLabelCreatedOn.AutoSize = true;
        this.mGeneralTabLabelCreatedOn.Location = new System.Drawing.Point(466, 151);
        this.mGeneralTabLabelCreatedOn.Name = "mGeneralTabLabelCreatedOn";
        this.mGeneralTabLabelCreatedOn.Size = new System.Drawing.Size(64, 13);
        this.mGeneralTabLabelCreatedOn.TabIndex = 24;
        this.mGeneralTabLabelCreatedOn.Text = "                   ";
        // 
        // mGeneralTabLabelShareRatio
        // 
        this.mGeneralTabLabelShareRatio.AutoSize = true;
        this.mGeneralTabLabelShareRatio.Location = new System.Drawing.Point(470, 57);
        this.mGeneralTabLabelShareRatio.Name = "mGeneralTabLabelShareRatio";
        this.mGeneralTabLabelShareRatio.Size = new System.Drawing.Size(64, 13);
        this.mGeneralTabLabelShareRatio.TabIndex = 8;
        this.mGeneralTabLabelShareRatio.Text = "                   ";
        // 
        // mGeneralTabLabelRemaining
        // 
        this.mGeneralTabLabelRemaining.AutoSize = true;
        this.mGeneralTabLabelRemaining.Location = new System.Drawing.Point(263, 57);
        this.mGeneralTabLabelRemaining.Name = "mGeneralTabLabelRemaining";
        this.mGeneralTabLabelRemaining.Size = new System.Drawing.Size(64, 13);
        this.mGeneralTabLabelRemaining.TabIndex = 7;
        this.mGeneralTabLabelRemaining.Text = "                   ";
        // 
        // mGeneralTabLabelDownloaded
        // 
        this.mGeneralTabLabelDownloaded.AutoSize = true;
        this.mGeneralTabLabelDownloaded.Location = new System.Drawing.Point(88, 82);
        this.mGeneralTabLabelDownloaded.Name = "mGeneralTabLabelDownloaded";
        this.mGeneralTabLabelDownloaded.Size = new System.Drawing.Size(64, 13);
        this.mGeneralTabLabelDownloaded.TabIndex = 4;
        this.mGeneralTabLabelDownloaded.Text = "                   ";
        // 
        // mGeneralTabLabelTimeElapsed
        // 
        this.mGeneralTabLabelTimeElapsed.AutoSize = true;
        this.mGeneralTabLabelTimeElapsed.Location = new System.Drawing.Point(88, 57);
        this.mGeneralTabLabelTimeElapsed.Name = "mGeneralTabLabelTimeElapsed";
        this.mGeneralTabLabelTimeElapsed.Size = new System.Drawing.Size(64, 13);
        this.mGeneralTabLabelTimeElapsed.TabIndex = 2;
        this.mGeneralTabLabelTimeElapsed.Text = "                   ";
        // 
        // mTabPageTracker
        // 
        this.mTabPageTracker.Controls.Add(this.mTrackerListView);
        this.mTabPageTracker.ImageIndex = 1;
        this.mTabPageTracker.Location = new System.Drawing.Point(4, 31);
        this.mTabPageTracker.Name = "mTabPageTracker";
        this.mTabPageTracker.Padding = new System.Windows.Forms.Padding(3);
        this.mTabPageTracker.Size = new System.Drawing.Size(1190, 257);
        this.mTabPageTracker.TabIndex = 4;
        this.mTabPageTracker.Text = "Tracker";
        this.mTabPageTracker.UseVisualStyleBackColor = true;
        // 
        // mTrackerListView
        // 
        this.mTrackerListView.AllowColumnReorder = true;
        this.mTrackerListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            columnHeader1,
            columnHeader2,
            columnHeader3,
            columnHeader4,
            columnHeader5});
        this.mTrackerListView.Dock = System.Windows.Forms.DockStyle.Fill;
        this.mTrackerListView.FullRowSelect = true;
        this.mTrackerListView.Location = new System.Drawing.Point(3, 3);
        this.mTrackerListView.MultiSelect = false;
        this.mTrackerListView.Name = "mTrackerListView";
        this.mTrackerListView.Size = new System.Drawing.Size(1114, 218);
        this.mTrackerListView.TabIndex = 1;
        this.mTrackerListView.UseCompatibleStateImageBehavior = false;
        this.mTrackerListView.View = System.Windows.Forms.View.Details;
        // 
        // mTabPagePeers
        // 
        this.mTabPagePeers.Controls.Add(this.mPeersListView);
        this.mTabPagePeers.ImageIndex = 2;
        this.mTabPagePeers.Location = new System.Drawing.Point(4, 31);
        this.mTabPagePeers.Name = "mTabPagePeers";
        this.mTabPagePeers.Padding = new System.Windows.Forms.Padding(3);
        this.mTabPagePeers.Size = new System.Drawing.Size(1190, 257);
        this.mTabPagePeers.TabIndex = 3;
        this.mTabPagePeers.Text = "Peers";
        this.mTabPagePeers.UseVisualStyleBackColor = true;
        // 
        // mPeersListView
        // 
        this.mPeersListView.AllowColumnReorder = true;
        this.mPeersListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            Ip,
            PeerId,
            this.PercentDone,
            DownloadRate,
            UploadRate,
            Requests,
            Flags,
            ConnectionType,
            Downloaded,
            this.Uploaded,
            ConnectionLength});
        this.mPeersListView.Dock = System.Windows.Forms.DockStyle.Fill;
        this.mPeersListView.FullRowSelect = true;
        this.mPeersListView.Location = new System.Drawing.Point(3, 3);
        this.mPeersListView.MultiSelect = false;
        this.mPeersListView.Name = "mPeersListView";
        this.mPeersListView.Size = new System.Drawing.Size(1184, 232);
        this.mPeersListView.TabIndex = 0;
        this.mPeersListView.UseCompatibleStateImageBehavior = false;
        this.mPeersListView.View = System.Windows.Forms.View.Details;
        this.mPeersListView.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.mPeersListView_ColumnClick);
        // 
        // PercentDone
        // 
        this.PercentDone.Text = "%";
        this.PercentDone.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
        // 
        // Uploaded
        // 
        this.Uploaded.Text = "Uploaded";
        this.Uploaded.Width = 100;
        // 
        // mTabPagePieces
        // 
        this.mTabPagePieces.Controls.Add(this.mPiecesListView);
        this.mTabPagePieces.ImageIndex = 5;
        this.mTabPagePieces.Location = new System.Drawing.Point(4, 31);
        this.mTabPagePieces.Name = "mTabPagePieces";
        this.mTabPagePieces.Padding = new System.Windows.Forms.Padding(3);
        this.mTabPagePieces.Size = new System.Drawing.Size(1190, 257);
        this.mTabPagePieces.TabIndex = 5;
        this.mTabPagePieces.Text = "Pieces";
        this.mTabPagePieces.UseVisualStyleBackColor = true;
        // 
        // mPiecesListView
        // 
        this.mPiecesListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            mNumberColumn,
            mSizeColumn,
            NumberOfBlocksColumn,
            mNumberOfRequestedBlocksColumn,
            mNumberOfCompletedBlocksColumn});
        this.mPiecesListView.Dock = System.Windows.Forms.DockStyle.Fill;
        this.mPiecesListView.FullRowSelect = true;
        this.mPiecesListView.Location = new System.Drawing.Point(3, 3);
        this.mPiecesListView.Name = "mPiecesListView";
        this.mPiecesListView.Size = new System.Drawing.Size(1114, 218);
        this.mPiecesListView.TabIndex = 0;
        this.mPiecesListView.UseCompatibleStateImageBehavior = false;
        this.mPiecesListView.View = System.Windows.Forms.View.Details;
        // 
        // mTabPageFiles
        // 
        this.mTabPageFiles.Controls.Add(this.mFilesListView);
        this.mTabPageFiles.ImageIndex = 3;
        this.mTabPageFiles.Location = new System.Drawing.Point(4, 31);
        this.mTabPageFiles.Name = "mTabPageFiles";
        this.mTabPageFiles.Padding = new System.Windows.Forms.Padding(3);
        this.mTabPageFiles.Size = new System.Drawing.Size(1190, 257);
        this.mTabPageFiles.TabIndex = 2;
        this.mTabPageFiles.Text = "Files";
        this.mTabPageFiles.UseVisualStyleBackColor = true;
        // 
        // mFilesListView
        // 
        this.mFilesListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            mFileListViewColumnName,
            mFileListViewColumnSize,
            mFileListViewColumnNumPieces,
            mFileListViewColumnPercentageDone});
        this.mFilesListView.Dock = System.Windows.Forms.DockStyle.Fill;
        this.mFilesListView.FullRowSelect = true;
        this.mFilesListView.Location = new System.Drawing.Point(3, 3);
        this.mFilesListView.Name = "mFilesListView";
        this.mFilesListView.Size = new System.Drawing.Size(1114, 218);
        this.mFilesListView.TabIndex = 0;
        this.mFilesListView.UseCompatibleStateImageBehavior = false;
        this.mFilesListView.View = System.Windows.Forms.View.Details;
        // 
        // mTabPageBandwidth
        // 
        this.mTabPageBandwidth.Controls.Add(this.mZedGraphControl);
        this.mTabPageBandwidth.ImageIndex = 4;
        this.mTabPageBandwidth.Location = new System.Drawing.Point(4, 31);
        this.mTabPageBandwidth.Name = "mTabPageBandwidth";
        this.mTabPageBandwidth.Padding = new System.Windows.Forms.Padding(3);
        this.mTabPageBandwidth.Size = new System.Drawing.Size(1190, 257);
        this.mTabPageBandwidth.TabIndex = 6;
        this.mTabPageBandwidth.Text = "Bandwidth";
        this.mTabPageBandwidth.UseVisualStyleBackColor = true;
        // 
        // mZedGraphControl
        // 
        this.mZedGraphControl.Dock = System.Windows.Forms.DockStyle.Fill;
        this.mZedGraphControl.Location = new System.Drawing.Point(3, 3);
        this.mZedGraphControl.Name = "mZedGraphControl";
        this.mZedGraphControl.ScrollGrace = 0D;
        this.mZedGraphControl.ScrollMaxX = 0D;
        this.mZedGraphControl.ScrollMaxY = 0D;
        this.mZedGraphControl.ScrollMaxY2 = 0D;
        this.mZedGraphControl.ScrollMinX = 0D;
        this.mZedGraphControl.ScrollMinY = 0D;
        this.mZedGraphControl.ScrollMinY2 = 0D;
        this.mZedGraphControl.Size = new System.Drawing.Size(1184, 232);
        this.mZedGraphControl.TabIndex = 0;
        // 
        // mTabPageOutput
        // 
        this.mTabPageOutput.ImageIndex = 4;
        this.mTabPageOutput.Location = new System.Drawing.Point(4, 31);
        this.mTabPageOutput.Name = "mTabPageOutput";
        this.mTabPageOutput.Padding = new System.Windows.Forms.Padding(3);
        this.mTabPageOutput.Size = new System.Drawing.Size(1190, 257);
        this.mTabPageOutput.TabIndex = 1;
        this.mTabPageOutput.Text = "Output";
        this.mTabPageOutput.UseVisualStyleBackColor = true;
        // 
        // mAppNotifyIcon
        // 
        this.mAppNotifyIcon.BalloonTipIcon = System.Windows.Forms.ToolTipIcon.Info;
        this.mAppNotifyIcon.BalloonTipText = "Test Tip";
        this.mAppNotifyIcon.BalloonTipTitle = "My Notify Title";
        this.mAppNotifyIcon.ContextMenuStrip = this.mAppSystemTrayMenu;
        this.mAppNotifyIcon.Icon = ((System.Drawing.Icon)(resources.GetObject("mAppNotifyIcon.Icon")));
        this.mAppNotifyIcon.DoubleClick += new System.EventHandler(this.NotifyIcon_DoubleClick);
        // 
        // mAppSystemTrayMenu
        // 
        this.mAppSystemTrayMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.exitToolStripMenuItem});
        this.mAppSystemTrayMenu.Name = "SystemTrayMenu";
        this.mAppSystemTrayMenu.Size = new System.Drawing.Size(93, 26);
        // 
        // exitToolStripMenuItem
        // 
        this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
        this.exitToolStripMenuItem.Size = new System.Drawing.Size(92, 22);
        this.exitToolStripMenuItem.Text = "E&xit";
        this.exitToolStripMenuItem.Click += new System.EventHandler(this.ExitMenu_Click);
        // 
        // MainForm
        // 
        this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
        this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
        this.ClientSize = new System.Drawing.Size(1198, 600);
        this.Controls.Add(this.mSplitContainer);
        this.Controls.Add(this.mAppStatusStrip);
        this.Controls.Add(this.mAppToolStrip);
        this.Controls.Add(this.mAppMenu);
        this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
        this.Name = "MainForm";
        this.Text = "Meerkat";
        this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MainForm_FormClosed);
        this.Load += new System.EventHandler(this.MainForm_Load);
        this.Resize += new System.EventHandler(this.MainForm_Resize);
        this.mAppMenu.ResumeLayout(false);
        this.mAppMenu.PerformLayout();
        this.mAppToolStrip.ResumeLayout(false);
        this.mAppToolStrip.PerformLayout();
        this.mAppStatusStrip.ResumeLayout(false);
        this.mAppStatusStrip.PerformLayout();
        this.mSplitContainer.Panel1.ResumeLayout(false);
        this.mSplitContainer.Panel2.ResumeLayout(false);
        ((System.ComponentModel.ISupportInitialize)(this.mSplitContainer)).EndInit();
        this.mSplitContainer.ResumeLayout(false);
        this.mMainListViewRightClickMenu.ResumeLayout(false);
        this.mAppTabControl.ResumeLayout(false);
        this.mTabPageGeneral.ResumeLayout(false);
        this.mTabPageGeneral.PerformLayout();
        this.mTabPageTracker.ResumeLayout(false);
        this.mTabPagePeers.ResumeLayout(false);
        this.mTabPagePieces.ResumeLayout(false);
        this.mTabPageFiles.ResumeLayout(false);
        this.mTabPageBandwidth.ResumeLayout(false);
        this.mAppSystemTrayMenu.ResumeLayout(false);
        this.ResumeLayout(false);
        this.PerformLayout();

    }

    #endregion

    private System.Windows.Forms.MenuStrip mAppMenu;
    private System.Windows.Forms.ToolStripMenuItem mMenuFileRoot;
    private System.Windows.Forms.ToolStrip mAppToolStrip;
    private System.Windows.Forms.ToolStripButton AddTorrentButton;
    private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
    private System.Windows.Forms.StatusStrip mAppStatusStrip;
    private System.Windows.Forms.ToolStripStatusLabel mStatusStripDlLabel;
    private System.Windows.Forms.SplitContainer mSplitContainer;
    private System.Windows.Forms.ListView mListView;
    private System.Windows.Forms.TabControl mAppTabControl;
    private System.Windows.Forms.TabPage mTabPageGeneral;
	private System.Windows.Forms.TabPage mTabPageOutput;
    private System.Windows.Forms.ImageList mAppImageList;
    private System.Windows.Forms.NotifyIcon mAppNotifyIcon;
	private System.Windows.Forms.ContextMenuStrip mAppSystemTrayMenu;
	private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
	private System.Windows.Forms.ToolStripMenuItem mMenuToolsRoot;
	private System.Windows.Forms.ToolStripMenuItem mMenuToolsOptions;
	private System.Windows.Forms.ToolStripStatusLabel mStatusStripUlLabel;
	private System.Windows.Forms.Label mGeneralTabLabelTimeElapsed;
	private System.Windows.Forms.Label mGeneralTabLabelShareRatio;
	private System.Windows.Forms.Label mGeneralTabLabelRemaining;
	private System.Windows.Forms.Label mGeneralTabLabelDownloaded;
	private System.Windows.Forms.Label mGeneralTabLabelComment;
	private System.Windows.Forms.Label mGeneralTabLabelHash;
	private System.Windows.Forms.Label mGeneralTabLabelPieces;
	private System.Windows.Forms.Label mGeneralTabLabelTotalSize;
	private System.Windows.Forms.Label mGeneralTabLabelCreatedOn;
	private System.Windows.Forms.TabPage mTabPageFiles;
	private System.Windows.Forms.ListView mFilesListView;
	private System.Windows.Forms.TabPage mTabPagePeers;
	private System.Windows.Forms.TabPage mTabPageTracker;
	private System.Windows.Forms.ListView mPeersListView;
	private System.Windows.Forms.ToolStripButton RemoveTorrentButton;
	private System.Windows.Forms.ToolStripButton StartTorrentButton;
	private System.Windows.Forms.ToolStripButton PauseTorrentButton;
	private System.Windows.Forms.ToolStripButton StopTorrentButton;
	private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
	private System.Windows.Forms.ToolStripButton OptionsButton;
	private System.Windows.Forms.ToolStripSeparator toolStripSeparator4;
	private System.Windows.Forms.ContextMenuStrip mMainListViewRightClickMenu;
	private System.Windows.Forms.ToolStripMenuItem removeTorrentToolStripMenuItem;
	private System.Windows.Forms.ToolStripMenuItem removeAndDeleteTorrentToolStripMenuItem;
	private System.Windows.Forms.ToolStripMenuItem openContainingFolderToolStripMenuItem;
	private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
    private System.Windows.Forms.ToolStripSeparator toolStripSeparator5;
	private System.Windows.Forms.ToolStripSeparator toolStripSeparator6;
	private System.Windows.Forms.ToolStripMenuItem forceRecheckToolStripMenuItem;
	private System.Windows.Forms.ToolStripMenuItem addTorrentToolStripMenuItem;
	private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem1;
	private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
	private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
	private System.Windows.Forms.TabPage mTabPagePieces;
	private System.Windows.Forms.ListView mTrackerListView;
	private System.Windows.Forms.ToolStripStatusLabel mStatusStripDhtLabel;
	private System.Windows.Forms.ToolStripSeparator toolStripSeparator7;
	private System.Windows.Forms.ToolStripMenuItem mMenuItemAllowUnencryptedComs;
	private System.Windows.Forms.ColumnHeader PercentDone;
	private System.Windows.Forms.ListView mPiecesListView;
	private System.Windows.Forms.TextBox mGeneralTabLabelLocalName;
	private System.Windows.Forms.TabPage mTabPageBandwidth;
	private ZedGraph.ZedGraphControl mZedGraphControl;
	private System.Windows.Forms.ColumnHeader Uploaded;

}

