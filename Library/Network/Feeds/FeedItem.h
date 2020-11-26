// Jon Bellamy 26-01-2008


#ifndef FEED_ITEM_H
#define FEED_ITEM_H

#include <string>
#include "xml/tinyxml.h"


namespace net {


class cFeedItem
{
public:
	
	cFeedItem();

	const cFeedItem& operator= (const cFeedItem& rhs);

	bool ParseRss(TiXmlElement *pXmlRoot);

	const std::string& Title() const { return mTitle; }
	const std::string& Description() const { return mDescription; }

protected:

	std::string mTitle;
	std::string mDescription;

};


} // namespace net

#endif // FEED_ITEM_H