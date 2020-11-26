// Jon Bellamy 13-08-2007


#include "stdafx.h"
#include "EmailMessage.h"

#include <assert.h>

#include "Network/Mail/Mime.h"

// app specific
#include "file.h"
#include "Global.h"


namespace net {



cEmailMessage::cEmailMessage(const char* filename) :
mMsgFile(filename, cFile::READ)
{
	assert(filename);
	strcpy_s(szMsgFileName, 256, filename);
	mMimeText.Load((const char*) (mMsgFile.GetFileContents()), mMsgFile.Size());
}// END cEmailMessage



cEmailMessage::cEmailMessage(const cEmailMessage& rhs) :
mMsgFile(rhs.szMsgFileName, cFile::READ)
{
	strcpy_s(szMsgFileName, 256, rhs.szMsgFileName);
	mMimeText.Load((const char*) (mMsgFile.GetFileContents()), mMsgFile.Size());
}// END cEmailMessage



cEmailMessage::~cEmailMessage()
{
}// END ~cEmailMessage



bool cEmailMessage::ValidMessage() const
{
	return true;
}// END ValidMessage


const char* cEmailMessage::From() const
{
	const char* pszField;
	pszField = mMimeText.GetFrom();
	return pszField;
}// END From



const char* cEmailMessage::To() const
{
	const char* pszField;
	pszField = mMimeText.GetTo();
	return pszField;
}// END To



const char* cEmailMessage::Subject() const
{
	const char* pszField;
	pszField = mMimeText.GetSubject();
	return pszField;
}// END Subject



const char* cEmailMessage::Date() const
{
	const char* pszField;
	pszField = mMimeText.GetDate();
	return pszField;
}// END Date



std::string cEmailMessage::Body(bool bPreferHtml)
{
	CMimeBody::CBodyList bodies;
	int nCount = mMimeText.GetBodyPartList(bodies);
	CMimeBody::CBodyList::const_iterator it;
	for (it=bodies.begin(); it!=bodies.end(); it++)
	{
		CMimeBody* pBP = *it;

		// Iterate all the header fields of this body part:
		CMimeHeader::CFieldList& fds = pBP->Fields();
		CMimeHeader::CFieldList::const_iterator itfd;
		for (itfd=fds.begin(); itfd!=fds.end(); itfd++)
		{
			const CMimeField& fd = *itfd;
			//printf("%s: %s\r\n", fd.GetName(), fd.GetValue());
		}

		if (pBP->IsText() ||
			pBP->IsMessage())
		{
			if(pBP->GetContentType() == NULL)
			{
				continue;
			}

			string strText = pBP->GetContentType();

			string strContentType;
			
			if(bPreferHtml)
			{
				strContentType = "text/html";
			}
			else
			{
				strContentType = "text/plain";
			}
						
			u32 index = strText.find(strContentType.c_str(), 0, strContentType.size());

			if(index != std::string::npos)
			{
				string strBody;
				pBP->GetText(strBody);
				return strBody;
			}
		}
		/*
		else if (pBP->IsAttachment())
		{
			string strName = pBP->GetName();
			printf("File name: %s\r\n", strName.c_str());
			printf("File size: %d\r\n", pBP->GetContentLength());
			strName = "d:\\download\\" + strName;
			pBP->WriteToFile(strName.c_str());
		}
		*/
	}
	
	if(bPreferHtml)
	{
		return Body(false);
	}
	else
	{
		return "";
	}
}


/*
const char* cEmailMessage::Body()
{
	//mMimeText.GetText(&mMsgBody[0], 1024);
	
	//return (const char*) mMimeText.GetContent();

	
	// Iterate all the descendant body parts
	CMimeBody::CBodyList bodies;
	int nCount = mMimeText.GetBodyPartList(bodies);
	CMimeBody::CBodyList::const_iterator it;
	for (it=bodies.begin(); it!=bodies.end(); it++)
	{
		CMimeBody* pBP = *it;

		// Iterate all the header fields of this body part:
		CMimeHeader::CFieldList& fds = pBP->Fields();
		CMimeHeader::CFieldList::const_iterator itfd;
		for (itfd=fds.begin(); itfd!=fds.end(); itfd++)
		{
			const CMimeField& fd = *itfd;
			printf("%s: %s\r\n", fd.GetName(), fd.GetValue());
		}

		if (pBP->IsText())
		{
			string strText;
			pBP->GetText(strText);
			printf("Content: %s\r\n", strText.c_str());
		}
		else if (pBP->IsAttachment())
		{
			string strName = pBP->GetName();
			printf("File name: %s\r\n", strName.c_str());
			printf("File size: %d\r\n", pBP->GetContentLength());
			strName = "d:\\download\\" + strName;
			pBP->WriteToFile(strName.c_str());
		}
	}
	
	return NULL;
}// END Body
*/



} // namespace net