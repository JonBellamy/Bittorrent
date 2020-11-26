
#ifndef SQLITE_DB__H
#define SQLITE_DB__H


#include "Database/SQLite/sqlite3.h"


class cSQLiteDb
{
public:

	cSQLiteDb(const char* fn);
	virtual ~cSQLiteDb();

private:
	cSQLiteDb(const cSQLiteDb& rhs);
	const cSQLiteDb& operator= (const cSQLiteDb& rhs);


public:



private:

	sqlite3* mpDb;

};





#endif // SQLITE_DB__H
