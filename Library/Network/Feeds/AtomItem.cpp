// Jon Bellamy 26-01-2008


#include "stdafx.h"
#include "AtomItem.h"



namespace net {



cAtomItem::cAtomItem()
{
}// END cRssFeed




bool cAtomItem::Parse(TiXmlElement *pXmlRoot)
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
}// END Parse



} // namespace net

