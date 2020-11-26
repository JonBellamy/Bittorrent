// Jon Bellamy 26-01-2008
// RSS feed class, basically just checks the date and decides weather or not to download the latest xml


#ifndef _FEED_H
#define _FEED_H

#include <vector>
#include "Network/Url.h"
#include "Network/Http/HttpClient.h"
#include "FeedItem.h"
#include "xml/tinyxml.h"


namespace net {


class cFeed
{
public:
	
	cFeed(const cUrl& url);

	// used to check for the newest version of the rss xml file
	//operator<

	bool IsValid() const { return mValid; }

	virtual void Process();

	virtual bool Refresh();

	virtual void DumpToTty();

	const cFeedItem& GetFeedItem(u32 index) const { return mFeedItems[index]; }

	u32 NumberOfFeedItems() const { return mFeedItems.size(); }

	const std::string& Title() const { return mTitle; }

	const cUrl& Source() const { return mFeedUrl; }

protected:

	static void HttpHeadRcvdCb(bool success, const net::cHttpMessage& message, void* param);
	static void HttpContentRcvdCb(bool success, const net::cHttpMessage& message, void* param);

	bool ParseRss(const char* szFn);
	//bool ParseAtom(const char* szFn);

	bool mValid;

	cUrl mFeedUrl;
	net::cHttpClient mHttpClient;



	// feed header items
	std::string mTitle;
	std::string mPubDate;
	std::string mDescription;

	/* all rss 2.0 none item fields
	<title>Lift Off News</title>
	<link>http://liftoff.msfc.nasa.gov/</link>
	<description>Liftoff to Space Exploration.</description>
	<language>en-us</language>
	<pubDate>Tue, 10 Jun 2003 04:00:00 GMT</pubDate>
	<lastBuildDate>Tue, 10 Jun 2003 09:41:01 GMT</lastBuildDate>
	<docs>http://blogs.law.harvard.edu/tech/rss</docs>
	<generator>Weblog Editor 2.0</generator>
	<managingEditor>editor@example.com</managingEditor>
	<webMaster>webmaster@example.com</webMaster>

	// also noticed in some feeds...
	<image>
	<title>Telegraph News | All</title>
	<width>124</width>
	<height>18</height>
	<link>http://news.telegraph.co.uk</link>
	<url>http://www.telegraph.co.uk/newsfeed/rss/tcuk_main.gif</url>
	</image>
	*/

	std::vector<cFeedItem> mFeedItems;
};


} // namespace net

#endif // _FEED_H