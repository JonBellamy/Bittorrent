// Jon Bellamy 16/04/2010


#ifndef BITTORRENT_OPTIONS_H
#define BITTORRENT_OPTIONS_H

#if USE_PCH
#include "stdafx.h"
#endif



struct sTorrentOptions
{
	// Because we want C linkage so this struct can go through the dll, we cannot use the private keyword or have a ctor in here.

	void SetDefault();

	void AllConnectionsMustBeEncrypted(bool b) { mAllConnectionsMustBeEncrypted = b; }
	bool AllConnectionsMustBeEncrypted() const { return mAllConnectionsMustBeEncrypted!=0; }

	void StopOnCompletion(bool b) { mStopOnCompletion = b; }
	bool StopOnCompletion() const { return mStopOnCompletion!=0; }

	void RecheckOnCompletion(bool b) { mRecheckOnCompletion = b; }
	bool RecheckOnCompletion() const { return mRecheckOnCompletion!=0; }

	void UseDht(bool b) { mUseDht = b; }
	bool UseDht() const { return mUseDht!=0; }

	void UseTrackers(bool b) { mUseTrackers = b; }
	bool UseTrackers() const { return mUseTrackers!=0; }

	void CheckForLatestBuild(bool b) { mCheckForLatestBuild = b; }
	bool CheckForLatestBuild() const { return mCheckForLatestBuild!=0; }

	void MaxUploadRate(u32 i) { mMaxUploadRate = i; }
	u32 MaxUploadRate() const { return mMaxUploadRate; }

	u8 mAllConnectionsMustBeEncrypted;
	u8 mStopOnCompletion;
	u8 mRecheckOnCompletion;
	u8 mUseDht;
	u8 mUseTrackers;
	u8 mCheckForLatestBuild;
	u32 mMaxUploadRate;
};








#endif // BITTORRENT_OPTIONS_H
