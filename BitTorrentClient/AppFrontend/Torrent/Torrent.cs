using System;
using ZedGraph;


using TorrentHandle = System.Int32;


public class Torrent
{
	public TorrentHandle Handle { get; set; }
	public DllInterface.TorrentMetaData MetaData { get; set; }

	private PointPairList downloadGraphData;
	public PointPairList DownloadGraphData { get { return downloadGraphData; } }

	private PointPairList uploadGraphData;
	public PointPairList UploadGraphData { get { return uploadGraphData; } }

/*
	public LineItem downloadGraphCurve;
	public LineItem uploadGraphCurve;
	public LineItem DownloadCurve { get { return downloadGraphCurve; } }
	public LineItem UploadCurve { get { return uploadGraphCurve; } }
*/

	public enum TorrentValues
	{
		INVALID_TORRENT = -1
	};



	public Torrent(TorrentHandle handle)
	{
		this.Handle = handle;

		downloadGraphData = new PointPairList();
		uploadGraphData = new PointPairList();

		//downloadGraphCurve = MainForm.mBandwidthGraph.Pane.AddCurve("Torrent Download Bandwidth", downloadGraphData, System.Drawing.Color.Red, SymbolType.Circle);
		//uploadGraphCurve = MainForm.mBandwidthGraph.Pane.AddCurve("Torrent Upload Bandwidth", downloadGraphData, System.Drawing.Color.Red, SymbolType.Circle);
	}


	public void SubmitGraphData(UInt32 dlMinute, UInt32 dlSpeed, UInt32 ulMinute, UInt32 ulSpeed)
	{
		downloadGraphData.Add(dlMinute, dlSpeed);
		uploadGraphData.Add(ulMinute, ulSpeed);

		//downloadGraphCurve.AddPoint(dlMinute, dlSpeed);
		//uploadGraphCurve.AddPoint(ulMinute, ulSpeed);
	}

}