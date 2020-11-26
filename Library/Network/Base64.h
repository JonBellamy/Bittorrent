// Jon Bellamy 18-04-2008
// Base64 is a simple encoding method used to encode any binary stream using text data. Certain formats such as mime
// only allow text data (no binary) and therefore must encode binary sections using text ...

#ifndef _BASE64_H
#define _BASE64_H


namespace net {


// remember that pBufOut needs to have a capacity larger than the bytes to encode as b64 does increase the size
// returns bytes written
extern int EncodeBase64(u8 const* bytes_to_encode, u32 in_len, u8* pBufOut, u32 outBufSize);

extern int DecodeBase64(const u8* encoded_string, u32 in_len, u8* pBufOut, u32 outBufSize);



} // namespace net


#endif 