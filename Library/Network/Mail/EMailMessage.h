// Jon Bellamy 13-08-2007

#ifndef __MAILMESSAGE_H
#define __MAILMESSAGE_H


#include "file.h"
#include "Network/Mail/Mime.h"

namespace net {


class cEmailMessage
{
public:

	cEmailMessage(const char* filename);
	cEmailMessage(const cEmailMessage&);

	~cEmailMessage();



private:	
	const cEmailMessage& operator=(const cEmailMessage&);

public:

	// UID : YYMMDDHHMM
	u32 TimeReceived() const;

	bool ValidMessage() const;

	bool IsMultipart() { return mMimeText.IsMultiPart(); }

	std::string Body(bool bPreferHtml);

	const char* From() const;
	const char* To() const;
	const char* Subject() const;
	const char* Date() const;


private:	

	char szMsgFileName[256];

	char mMsgBody[1024];

	cFile mMsgFile;

	CMimeMessage mMimeText;
	u32 mMessageSize;
};



} // namespace net


#endif 