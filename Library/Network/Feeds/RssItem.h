// Jon Bellamy 26-01-2008


#ifndef RSS_ITEM_H
#define RSS_ITEM_H

#include <string>
#include "xml/tinyxml.h"
#include "FeedItem.h"


namespace net {


class cRssItem : public cFeedItem
{
public:
	
	cRssItem();

	bool Parse(TiXmlElement *pXmlRoot);

private:

};


} // namespace net

#endif // RSS_ITEM_H