// Jon Bellamy 26-01-2008


#include "stdafx.h"
#include "FeedItem.h"



namespace net {



cFeedItem::cFeedItem()
{
}// END cRssFeed



const cFeedItem& cFeedItem::operator= (const cFeedItem& rhs)
{
	mTitle = rhs.mTitle;
	mDescription = rhs.mDescription;
	return *this;
}// END operator=




bool cFeedItem::ParseRss(TiXmlElement *pXmlRoot)
{
	TiXmlElement	*currentElement;
	TiXmlText		*assetText;

	currentElement = (TiXmlElement*) pXmlRoot->FirstChild("title");
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
	currentElement = (TiXmlElement*) pXmlRoot->FirstChild("description");
	if (currentElement)
	{
		assetText = (TiXmlText *) currentElement->FirstChild();
		if (assetText)
		{
			mDescription = assetText->Value();
		}
	}
	else 
	{
		return false;
	}

	return true;
}// END ParseRss




} // namespace net

