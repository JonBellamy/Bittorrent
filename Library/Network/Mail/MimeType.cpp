//////////////////////////////////////////////////////////////////////
//
// MIME message encoding/decoding
//
// Jeff Lee
// Dec 22, 2000
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Mime.h"

// Content-Type header field specifies the media type of a body part. it coule be:
// text/image/audio/vedio/application (discrete type) or message/multipart (composite type).
// the default Content-Type is: text/plain; charset=us-ascii (RFC 2046)

