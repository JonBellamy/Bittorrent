// Jon Bellamy 26-01-2008


#include "stdafx.h"

#include "FeedManager.h"

#include "Global.h"



namespace net 
{



cFeedManager::cFeedManager()
{
}// END cFeedManager



cFeedManager::~cFeedManager()
{
	for(u32 i=0; i < mFeeds.size(); i++)
	{
		cFeed* pFeed = mFeeds[i];
		assert(pFeed);
		delete pFeed;
		pFeed=NULL;
	}
	mFeeds.clear();
}// END cFeedManager



void cFeedManager::AddFeed(const cUrl& url)
{
	mFeeds.push_back(new cFeed(url));
}// END AddFeed



void cFeedManager::RefreshAllFeeds()
{
	for(u32 i=0; i < mFeeds.size(); i++)
	{
		cFeed* pFeed = mFeeds[i];
		pFeed->Refresh();
	}
}// END RefreshAllFeeds



void cFeedManager::Process()
{
	for(u32 i=0; i < mFeeds.size(); i++)
	{
		cFeed* pFeed = mFeeds[i];
		pFeed->Process();
	}


	// TODO : REMOVE BAD FEEDS


}


} // namespace net

