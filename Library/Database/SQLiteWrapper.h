// defines all the functions we will use in the SQLite dll
// If you need to use an SQLite function that isn't in this list, add it and call through the wrapper. 
// You cannot simply call the function or the silly .net linker will cry like the whinny little girl it is.



#ifndef __SQLITE_WRAP_H
#define __SQLITE_WRAP_H


using namespace System;
using namespace System::Windows::Forms;
using namespace System::Runtime::InteropServices;


#include "Database/SQLite/sqlite3.h"


public ref class SQLiteWrapper
{
public:

	//////////////////////////////////////////////////////////////////////////
	// sqlite3.dll imports

	[DllImport("sqlite3.dll", EntryPoint="sqlite3_open")]
	static int sqlite3_open(const char *filename, sqlite3 **ppDb);

	[DllImport("sqlite3.dll", EntryPoint="sqlite3_close")]
	static int sqlite3_close(sqlite3 *);

	[DllImport("sqlite3.dll", EntryPoint="sqlite3_exec")]
	static int sqlite3_exec(sqlite3*, const char *sql, int (*callback)(void*,int,char**,char**), void *, char **errmsg);

	[DllImport("sqlite3.dll", EntryPoint="sqlite3_get_table")]
	static int sqlite3_get_table(sqlite3*,	const char *sql, char ***pResult, int *nrow, int *ncolumn, char **errmsg);

	[DllImport("sqlite3.dll", EntryPoint="sqlite3_free_table")]
	static void sqlite3_free_table(char **result);

	[DllImport("sqlite3.dll", EntryPoint="sqlite3_malloc")]
	static void *sqlite3_malloc(int);

	[DllImport("sqlite3.dll", EntryPoint="sqlite3_realloc")]
	static void *sqlite3_realloc(void*, int);

	[DllImport("sqlite3.dll", EntryPoint="sqlite3_free")]
	static void sqlite3_free(void*);

}; 









#endif  // __SSL_WRAP_H