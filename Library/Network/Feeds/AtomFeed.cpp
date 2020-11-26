// Jon Bellamy 26-01-2008
// Atom feed class, basically just checks the date and decides weather or not to download the latest xml

#include "stdafx.h"

#include "AtomFeed.h"

#include "Network/Dns.h"
#include "Global.h"

namespace net {



cAtomFeed::cAtomFeed(const cUrl& url)
: cFeed(url)
{
}// END cAtomFeed


bool cAtomFeed::Parse(const char* szFn)
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
		cAtomItem item;

		if(!item.Parse(itemRoot))
		{
			GlobalConsoleOutput("Badly formed rss item\n");
		}

		mFeedItems.push_back(item);

		itemRoot = (TiXmlElement*) itemRoot->NextSiblingElement("item");
	}
	

	return true;


}// END Parse



void cAtomFeed::DumpToTty()
{
	GlobalConsoleOutput(String::Format("Title : {0}\n", gcnew String(mTitle.c_str())));
	//GlobalConsoleOutput(String::Format("Pub date : {0}\n", gcnew String(mPubDate.c_str())));
	GlobalConsoleOutput(String::Format("Description : {0}\n", gcnew String(mDescription.c_str())));

	for(u32 i=0; i < mFeedItems.size(); i++)
	{
		const cAtomItem& item = mFeedItems[i];

		GlobalConsoleOutput(String::Format("Item {0} :\n", i));
		GlobalConsoleOutput(String::Format("Title : {0}\n", gcnew String(item.Title().c_str())));
		GlobalConsoleOutput(String::Format("Description : {0}\n", gcnew String(item.Description().c_str())));
	}
}// END DumpToTty



} // namespace net

