/*
 *  Printf.h
 *  Halloumi
 *
 *  Created by Ian Johnson on 13/05/2010.
 *  Copyright 2010 Varsity Media Group. All rights reserved.
 *
 */

typedef void (*DebugStringOutputCb)	 (const char* str);
extern void SetPrintfHandler(DebugStringOutputCb cb);

extern bool gSuppressPrintf;

extern "C" void Printf(const char* szStr, ...);
