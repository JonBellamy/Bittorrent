// Jon Bellamy 26-01-2008
// Feed base

#include "stdafx.h"

#include "Feed.h"

#include "Network/Dns.h"
#include "Global.h"


namespace net {



cFeed::cFeed(const cUrl& url)
: mFeedUrl(url)
, mValid(false)
{
}// END cFeed



void cFeed::Process()
{
	mHttpClient.Process();
}


bool cFeed::Refresh()
{	
	mFeedItems.clear();
	return mHttpClient.GET(mFeedUrl, HttpHeadRcvdCb, this, HttpContentRcvdCb, this);
}// END Refresh



void cFeed::HttpHeadRcvdCb(bool success, const net::cHttpMessage& message, void* param)
{
	cFeed* pThis = reinterpret_cast<cFeed*> (param);
	assert(pThis);

	pThis->mValid = success;
	if(!pThis->mValid)
	{
		return;
	}

#if HTTP_DEBUG_MESSAGES
	printf("cRssFeed::HttpHeadRcvdCb\n");
#endif
}// END HttpHeadRcvdCb



void cFeed::HttpContentRcvdCb(bool success, const net::cHttpMessage& message, void* param)
{
	cFeed* pThis = reinterpret_cast<cFeed*> (param);
	assert(pThis);

	pThis->mValid = success;
	if(!pThis->mValid)
	{
		return;
	}

#if HTTP_DEBUG_MESSAGES
	printf("*** cRssFeed::HttpContentRcvdCb\n");
#endif

	// TODO : this code needs to be in a func somewhere
	std::string path = "./HttpCache/" + pThis->mFeedUrl.HostName() + pThis->mFeedUrl.ResPathOnly() + "/";
	std::string fn = path + pThis->mFeedUrl.ResourceName();

	pThis->ParseRss(fn.c_str());

	//pThis->DumpToTty();
}// END HttpContentRcvdCb



bool cFeed::ParseRss(const char* szFn)
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

	TiXmlDeclaration* dec1 = (TiXmlDeclaration*) doc.FirstChild();
	//TiXmlNode* dec2 = (TiXmlNode*) dec1->NextSiblingElement();

	// get the root element from the doc <AVI_PROJECT>
	root = doc.RootElement();


	//root = (TiXmlElement*) root->FirstChild("?xml-stylesheet");
	


	// TODO : check RSS tag and bail if its not there


	// skipping pass the rss version here, maybe a bad move 
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
	printf("*** WARNING No publicaion date\n");
	//return false;
	}
	*/

	// item
	// load all the assets
	itemRoot = (TiXmlElement*) root->FirstChild("item");
	while(itemRoot)
	{
		cFeedItem item;

		if(!item.ParseRss(itemRoot))
		{
			printf("Badly formed rss item\n");
		}

		mFeedItems.push_back(item);

		itemRoot = (TiXmlElement*) itemRoot->NextSiblingElement("item");
	}

	mValid = true;

	return true;
}// END ParseRss




void cFeed::DumpToTty()
{
	printf("Title : %s\n", mTitle.c_str());
	//GlobalConsoleOutput(String::Format("Pub date : {0}\n", gcnew String(mPubDate.c_str())));
	printf("Description : %s\n", mDescription.c_str());

	for(u32 i=0; i < mFeedItems.size(); i++)
	{
		const cFeedItem& item = mFeedItems[i];

		printf("Item %d :\n", i);
		printf("Title : %s\n", item.Title().c_str());
		printf("Description : %s\n", item.Description().c_str());
	}
}// END DumpToTty


} // namespace net

