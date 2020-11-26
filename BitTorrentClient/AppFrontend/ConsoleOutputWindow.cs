// Jon Bellamy 06/10/2006
// Console Window

using System;
using System.Windows.Forms;
using System.Drawing;


public class ConsoleOutputWindow : RichTextBox
{

	public ConsoleOutputWindow()
	{
		mTextColor = Color.FromArgb(0, 0, 0, 255);

		// force the creation of the hwnd etc so the control can be used right now
		CreateControl();
	}

	public void AddString(String opStr)
	{
		try
        {
            SuspendLayout();

			if (this.Created == false)
			{
				return;
			}
			
			//this.SelectionColor = Color.FromArgb(255, 255, 0, 0);

			/* TODO : put thisback instead of clear
			while ((this.TextLength + opStr.Length) >= this.MaxLength)
			{
				OutputWindowRemoveTopLine();
			}
			*/
			if ((this.TextLength + opStr.Length) >= this.MaxLength)
			{
				Clear();
			}

			//  TODO : put thisback instead of clear
			//while(GetNumberOfLines() >= OUTPUTWINDOW_MAX_LINES)
			//{
			//	OutputWindowRemoveTopLine();
			//}

			// move Carret to end
			this.Select(this.TextLength, 1);

			// insert text
			this.SelectionColor = mTextColor;
			this.AppendText(opStr);


			// scroll new text into view
			this.ScrollToCaret();
		}
		finally
        {
            ResumeLayout();
        }

	}// END AddString



	void OutputWindowRemoveTopLine()
	{
		String text = this.Text;
		for (int i = 0; i < text.Length; i++)
		{
			if (text[i] == '\n')
			{
				this.Text = text.Substring(i+1);
			}
		}
	}



	int GetNumberOfLines()
	{
		int length = this.TextLength;
		return this.GetLineFromCharIndex(length);
	}// END GetNumberOfLines



	public void SetTextColour(byte r, byte g, byte b, byte a)
	{
		this.ForeColor = Color.FromArgb(a, r, g, b);
		this.mTextColor = Color.FromArgb(a, r, g, b);
	}// END OutputWindowSetTextColour



	protected override void OnCreateControl()
	{
		base.OnCreateControl();
		this.Multiline = true;
		this.MaxLength = 1024 * 512;
		this.Clear();
		this.SelectionFont = new System.Drawing.Font("Tahoma", 10.0F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, (System.Byte)0);
		this.Font = new System.Drawing.Font("Tahoma", 10.0F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, (System.Byte)0);
		this.WordWrap = false;	
		//this.ShowSelectionMargin = true;	// removes scroll bar for some bizare reason
		this.ScrollBars = RichTextBoxScrollBars.Vertical;
		this.ReadOnly = true;
	}



	//////////////////////////////////////////////////////////////////////////
	// member data

	//private static readonly int OUTPUTWINDOW_MAX_LINES = 10000;
	private Color mTextColor;
};
