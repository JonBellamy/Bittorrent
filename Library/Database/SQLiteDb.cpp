
#include "stdafx.h"
#include "SQLiteDb.h"

#include <stdio.h>
#include <assert.h>

#include "Database/SQLiteWrapper.h"
#include "Global.h"



cSQLiteDb::cSQLiteDb(const char* fn)
{
	int ret = SQLiteWrapper::sqlite3_open(fn, &mpDb);

	if(ret)
	{
		SQLiteWrapper::sqlite3_close(mpDb);
		assert(0);
	}


	{
		char* err;
		char sql[1024] = "DROP TABLE mails;";
		ret = SQLiteWrapper::sqlite3_exec(mpDb, sql, NULL, NULL, &err);

		if(ret != SQLITE_OK)
		{
			printf("SQL Error : %s\n", err);
			SQLiteWrapper::sqlite3_free(err);
		}
	}




	{
	char* err;
	char sql[1024] = "CREATE TABLE mails (uid integer primary key, fromAddr text, subject text, body text);";
	ret = SQLiteWrapper::sqlite3_exec(mpDb, sql, NULL, NULL, &err);
	
	if(ret != SQLITE_OK)
	{
		printf("SQL Error : %s\n", err);
		SQLiteWrapper::sqlite3_free(err);
	}
	}


	{
		char* err;
		char sql[1024] = "INSERT INTO mails VALUES(NULL,'jb@yahoo.co.uk','my subject','hello body');";
		ret = SQLiteWrapper::sqlite3_exec(mpDb, sql, NULL, NULL, &err);


		if(ret != SQLITE_OK)
		{
			printf("SQL Error : %s\n", err);
			SQLiteWrapper::sqlite3_free(err);
		}
	}




	{
		char* err;
		char sql[1024] = "SELECT * FROM mails;";
		char** results;
		int nRows, nCols;
		ret = SQLiteWrapper::sqlite3_get_table(mpDb, sql, &results, &nRows, &nCols, &err);

		if(ret == SQLITE_OK)
		{
			for(int i=0; i < nCols; ++i)
			{
				printf("%s ", results[i]);   /* First row heading */
			}
			
			printf("\n");

			for(int i=0; i < nCols*nRows; ++i)
			{
				printf("%s ", results[nCols+i]);
			}
		}
		else
		{
			printf("SQL Error : %s\n", err);
			SQLiteWrapper::sqlite3_free(err);
		}

		SQLiteWrapper::sqlite3_free_table(results);

		printf("\n");
	}
}// END cSQLiteDb



cSQLiteDb::~cSQLiteDb()
{
	assert(mpDb);
	SQLiteWrapper::sqlite3_close(mpDb);
}// END ~cSQLiteDb