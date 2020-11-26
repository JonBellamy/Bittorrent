// Jon Bellamy 31/12/2006

#include "stdafx.h"

#include <stdio.h>
#include "cassert.h"

#if defined( _DEBUG )


bool CustomAssertFunction( bool exp, char* description, int line, char* file, bool* ignoreAlways)
{
	return true;
	/*
	if(exp==false)
	{
		char sz[512];
		sprintf(sz, "ASSERT : %s	FILE %s LINE %d", description, file, line);
		printf(sz);
		int ret = AfxMessageBox(sz, MB_ABORTRETRYIGNORE);

		switch(ret)
		{
		case IDABORT:
		case IDRETRY:
			return true;

		case IDIGNORE:
			*ignoreAlways=true;
			return false;
		}

		return false;
	}

	return false;
	*/

	/*
	if( OpenClipboard( NULL ) )
	{
		HGLOBAL hMem;
		char szAssert[256];
		char *pMem;

		sprintf( szAssert, "%s", description);
		hMem = GlobalAlloc( GHND|GMEM_DDESHARE, strlen( szAssert )+1 );

		if( hMem ) 
		{
			pMem = (char*)GlobalLock( hMem );
			strcpy( pMem, szAssert );
			GlobalUnlock( hMem );
			EmptyClipboard();
			SetClipboardData( CF_TEXT, hMem );
		}

		CloseClipboard();
	}
	*/
}// END CustomAssertFunction


#endif





