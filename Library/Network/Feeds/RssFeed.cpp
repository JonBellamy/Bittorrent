// Jon Bellamy 26-01-2008
// RSS feed class, basically just checks the date and decides weather or not to download the latest xml

#include "stdafx.h"

#include "RssFeed.h"

#include "Network/Dns.h"
#include "Global.h"


namespace net {



cRssFeed::cRssFeed(const cUrl& url)
: cFeed(url)
{
}// END cRssFeed



bool cRssFeed::Parse(const char* szFn)
{
	TiXmlElement	*root, *itemRoot;
	TiXmlElement	*currentElement;
	TiXmlText		*assetText;
	//const char		*szValue;

	// create the xml doc
	TiXmlDocument doc(szFn);

	// load the xml file
	if (!doc.LoadFile()) 
	{
		return false;
	}


	// get the root element from the doc <AVI_PROJECT>
	root = doc.RootElement();


	// TODO : check RSS tag and bail if its not there


	// skipping pass the rss version here, may be a bad move 
	root = (TiXmlElement*) root->FirstChild("channel");
	if (!root) 
	{
		return false;
	}

	// title
	currentElement = (TiXmlElement*) root->FirstChild("title");
	if (currentElement)
	{
		assetText = (TiXmlText *) currentElement->FirstChild();
		if (currentElement)
		{
			mTitle = assetText->Value();
		}
	}
	else 
	{
		return false;
	}

	// description
	currentElement = (TiXmlElement*) root->FirstChild("description");
	if (currentElement)
	{
		assetText = (TiXmlText *) currentElement->FirstChild();
		if (currentElement)
		{
			mDescription = assetText->Value();
		}
	}
	else 
	{
		return false;
	}

/* bullshit param
	currentElement = (TiXmlElement*) root->FirstChild("pubDate");
	if (currentElement)
	{
		assetText = (TiXmlText *) currentElement->FirstChild();
		if (currentElement)
		{
			mPubDate = assetText->Value();
		}
	}
	else 
	{
		GlobalConsoleOutput("*** WARNING No publicaion date\n");
		//return false;
	}
*/

	// item
	// load all the assets
	itemRoot = (TiXmlElement*) root->FirstChild("item");
	while(itemRoot)
	{
		cRssItem item;

		if(!item.Parse(itemRoot))
		{
			GlobalConsoleOutput("Badly formed rss item\n");
		}

		mFeedItems.push_back(item);

		itemRoot = (TiXmlElement*) itemRoot->NextSiblingElement("item");
	}
	

	return true;


}// END Parse



void cRssFeed::DumpToTty()
{
	GlobalConsoleOutput(String::Format("Title : {0}\n", gcnew String(mTitle.c_str())));
	//GlobalConsoleOutput(String::Format("Pub date : {0}\n", gcnew String(mPubDate.c_str())));
	GlobalConsoleOutput(String::Format("Description : {0}\n", gcnew String(mDescription.c_str())));

	for(u32 i=0; i < mFeedItems.size(); i++)
	{
		const cRssItem& item = mFeedItems[i];

		GlobalConsoleOutput(String::Format("Item {0} :\n", i));
		GlobalConsoleOutput(String::Format("Title : {0}\n", gcnew String(item.Title().c_str())));
		GlobalConsoleOutput(String::Format("Description : {0}\n", gcnew String(item.Description().c_str())));
	}
}// END DumpToTty



} // namespace net

