// Jon Bellamy 26-01-2008
// RSS feed class, basically just checks the date and decides weather or not to download the latest xml


#ifndef ATOM_FEED_H
#define ATOM_FEED_H

#include <vector>
#include "Network/Url.h"
#include "Network/Http/HttpClient.h"
#include "AtomItem.h"
#include "feed.h"
#include "xml/tinyxml.h"


namespace net {


class cAtomFeed : public cFeed
{
public:
	
	cAtomFeed(const cUrl& url);

	// used to check for the newest version of the rss xml file
	//operator<

	void DumpToTty();

private:

	bool Parse(const char* szFn);

	// feed header items
	std::string mTitle;
	std::string mPubDate;
	std::string mDescription;


	std::vector<cAtomItem> mFeedItems;
};


} // namespace net

#endif // ATOM_FEED_H