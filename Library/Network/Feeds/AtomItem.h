// Jon Bellamy 26-01-2008


#ifndef ATOM_ITEM_H
#define ATOM_ITEM_H

#include <string>
#include "xml/tinyxml.h"
#include "FeedItem.h"

namespace net {


class cAtomItem : public cFeedItem
{
public:
	
	cAtomItem();


	bool Parse(TiXmlElement *pXmlRoot);


private:

};


} // namespace net


#endif // ATOM_ITEM_H