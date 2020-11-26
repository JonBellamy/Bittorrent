
#include "TorrentOptions.h"

#include <assert.h>

#include "BitTorrentValues.h"


void sTorrentOptions::SetDefault()
{
	mAllConnectionsMustBeEncrypted = true;
	mStopOnCompletion = true;
	mUseDht = true;
	mUseTrackers = true;
	mCheckForLatestBuild = true;
	mRecheckOnCompletion = false;
	mMaxUploadRate = DEFAULT_UPLOAD_LIMIT;
}
/*
cTorrentOptions::cTorrentOptions()
: mAllConnectionsMustBeEncrypted(false)
{
}// END cTorrentOptions



cTorrentOptions::~cTorrentOptions()
{
}// END ~cTorrentOptions
*/