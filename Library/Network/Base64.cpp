// Jon Bellamy 18-04-2008
// Base64 is a simple encoding method used to encode any binary stream using text data. Certain formats such as mime
// only allow text data (no binary) and therefore must encode binary sections using text ...



#include "Base64.h"

#include <assert.h>



namespace net {



static const u8 base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";



// determines if a particular character is an alphanumeric character
static bool isalnum(u8 c)
{
	return (c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
}// END isalnum



static inline bool is_base64(unsigned char c) 
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}// END is_base64



static s32 FindChar(const u8 val, const u8* buf, u32 bufLen)
{
	for(u32 i=0; i < bufLen; i++)
	{
		if(buf[i] == val)
		{
			return i;
		}
	}
	//ASSERTMSG(0, "bad base64 data");
	return -1;
}// END FindChar



// remember that pBufOut needs to have a capacity larger than the bytes to encode as b64 does increase the size
// returns bytes written
int EncodeBase64(u8 const* bytes_to_encode, u32 in_len, u8* pBufOut, u32 outBufSize)
{
	// FFS
	//return DWC_Base64Encode((const char*) bytes_to_encode, in_len, (char*) pBufOut, outBufSize);
	


	u32 writeIndex=0;
	int i = 0;
	int j = 0;
	u8 char_array_3[3];
	u8 char_array_4[4];

	while (in_len--) 
	{
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) 
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++)
			{
				pBufOut[writeIndex] = base64_chars[char_array_4[i]];
				writeIndex++;
			}
			i = 0;
		}
	}

	if (i)
	{
		for(j = i; j < 3; j++)
		{
			char_array_3[j] = '\0';
		}

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
		{
			pBufOut[writeIndex] = base64_chars[char_array_4[j]];
			writeIndex++;
		}

		while((i++ < 3))
		{
			pBufOut[writeIndex] = '=';
			writeIndex++;
		}

	}

	assert(writeIndex < outBufSize);
	return writeIndex;
}// END EncodeBase64



int DecodeBase64(const u8* encoded_string, u32 in_len, u8* pBufOut, u32 outBufSize)
{
	//return DWC_Base64Decode((const char*) encoded_string, in_len, (char*) pBufOut, outBufSize);
	

	u32 writeIndex=0;
	int i = 0;
	int j = 0;
	int in_ = 0;
	u8 char_array_4[4];
	u8 char_array_3[3];

	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) 
	{
		char_array_4[i++] = encoded_string[in_]; 
		in_++;
		if (i == 4) 
		{
			for (i = 0; i <4; i++)
			{				
				char_array_4[i] = static_cast<u8> (FindChar(char_array_4[i], base64_chars, 64));
			}

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
			{
				pBufOut[writeIndex] = char_array_3[i];
				writeIndex++;
			}
			i = 0;
		}
	}

	if (i) 
	{
		for (j = i; j <4; j++)
		{
			char_array_4[j] = 0;
		}

		for (j = 0; j <4; j++)
		{		
			char_array_4[j] = static_cast<u8> (FindChar(char_array_4[j], base64_chars, 64));
		}

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) 
		{
			pBufOut[writeIndex] = char_array_3[j];
			writeIndex++;
		}
	}

	assert(writeIndex < outBufSize);
	return writeIndex;
}// END DecodeBase64



} // namespace net