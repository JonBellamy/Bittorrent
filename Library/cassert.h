/* Copyright (C) Steve Rabin, 2000. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Steve Rabin, 2000"
 */

#ifndef CASSERT_H
#define CASSERT_H



#if defined( _DEBUG )
extern bool CustomAssertFunction( bool, char*, int, char*, bool* );

#define Assert( exp, description ) \
   {  static bool ignoreAlways = false; \
      if( !ignoreAlways ) { \
         if( CustomAssertFunction( exp!=0, description, \
                                   __LINE__, __FILE__, &ignoreAlways ) ) \
         { _asm { int 3 } } \
      } \
   }

#else
#define Assert( exp, description )
#endif



#endif // CASSERT_H