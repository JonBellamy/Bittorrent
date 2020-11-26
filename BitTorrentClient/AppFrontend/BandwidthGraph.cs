using System;

using ZedGraph;


public class BandwidthGraph
{
	public GraphPane Pane { get; set; }
	private Torrent currentTorrent;

	ZedGraphControl zgc;
	
	LineItem currentTorrentDownloadCurve;
	LineItem currentTorrentUploadCurve;

	LineItem totalDownloadCurve;
	LineItem totalUploadCurve;


	public BandwidthGraph(ZedGraphControl zgc)
	{
		this.zgc = zgc;

		currentTorrent = null;
		totalDownloadCurve = null;
		totalUploadCurve = null;

		CreateGraph();
	}


	public void SetCurrentTorrent(Torrent torrent)
	{
		currentTorrent = torrent;

		Pane.CurveList.Clear();

		if(currentTorrent != null)
		{
			currentTorrentDownloadCurve = Pane.AddCurve("Current Torrent Download", currentTorrent.DownloadGraphData, System.Drawing.Color.Red, SymbolType.Circle);
			//currentTorrentDownloadCurve.Line.Fill = new Fill(System.Drawing.Color.White, System.Drawing.Color.Red, 45F);

			currentTorrentUploadCurve = Pane.AddCurve("Current Torrent Upload", currentTorrent.UploadGraphData, System.Drawing.Color.Blue, SymbolType.Circle);
			//currentTorrentUploadCurve.Line.Fill = new Fill(System.Drawing.Color.White, System.Drawing.Color.Blue, 45F);
		}

		if(totalDownloadCurve != null)
		{
			totalDownloadCurve = Pane.AddCurve("Total Download", MainForm.torrentManager.DownloadGraphData, System.Drawing.Color.Green, SymbolType.Circle);
			//totalDownloadCurve.Line.Fill = new Fill(System.Drawing.Color.White, System.Drawing.Color.Green, 45F);
		}

		if(totalUploadCurve != null)
		{
			totalUploadCurve = Pane.AddCurve("Total Upload", MainForm.torrentManager.UploadGraphData, System.Drawing.Color.Yellow, SymbolType.Circle);
			//totalUploadCurve.Line.Fill = new Fill(System.Drawing.Color.White, System.Drawing.Color.Yellow, 45F);
		}

		zgc.AxisChange();
		//zgc.Invalidate();
		zgc.Refresh();
	}


	public void UpdateCurves()
	{
		if(totalDownloadCurve == null)
		{
			totalDownloadCurve = Pane.AddCurve("Total Download Bandwidth", MainForm.torrentManager.DownloadGraphData, System.Drawing.Color.Green, SymbolType.Circle);

			// Fill the area under the curves
			//totalDownloadCurve.Line.Fill = new Fill(System.Drawing.Color.White, System.Drawing.Color.Green, 45F);
		}

		if(totalUploadCurve == null)
		{
			totalUploadCurve = Pane.AddCurve("Total Upload Bandwidth", MainForm.torrentManager.UploadGraphData, System.Drawing.Color.Yellow, SymbolType.Circle);

			// Fill the area under the curves
			//totalUploadCurve.Line.Fill = new Fill(System.Drawing.Color.White, System.Drawing.Color.Yellow, 45F);
		}

		if (currentTorrent != null)
		{
			currentTorrentDownloadCurve.Points = currentTorrent.DownloadGraphData;
			currentTorrentUploadCurve.Points = currentTorrent.UploadGraphData;
		}

		totalDownloadCurve.Points = MainForm.torrentManager.DownloadGraphData;
		totalUploadCurve.Points = MainForm.torrentManager.UploadGraphData;

		zgc.AxisChange();
		//zgc.Invalidate();
		zgc.Refresh();
	}



	// Build the Chart
	public void CreateGraph()
	{
		// get a reference to the GraphPane
		Pane = zgc.GraphPane;

		// Set the Titles
		Pane.Title.Text = "Bandwidth Graph";
		Pane.XAxis.Title.Text = "Time in minutes (From application start)";
		Pane.YAxis.Title.Text = "kB per sec";


		// Tell ZedGraph to refigure the axes since the data have changed
		zgc.AxisChange();
	}

}