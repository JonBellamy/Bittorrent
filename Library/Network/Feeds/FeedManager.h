// Jon Bellamy 26-01-2008

// List of thousands of feeds : http://www.syndic8.com


#ifndef FEED_MANAGER_H
#define FEED_MANAGER_H

#include <vector>

#include "Feed.h"
#include "Network/Url.h"


namespace net 
{


class cFeedManager
{
public:
	
	cFeedManager();
	~cFeedManager();

	void AddFeed(const cUrl& url);	

	void RefreshAllFeeds();

	void Process();

	u32 NumberOfFeeds() const { return mFeeds.size(); }

	const cFeed& GetFeed(u32 index) const { return *(mFeeds[index]); }

private:

	// TODO : callbacks


	std::vector<cFeed*> mFeeds;
};


} // namespace net



#endif // FEED_MANAGER_H