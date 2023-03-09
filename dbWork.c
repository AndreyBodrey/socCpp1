#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "processedPacket.h"
#include "sqlite3.h"



static void insertStrBuild(char *str);


#define DB_PATH "dataBase.db1"


static char *sqlError;
extern sqlite3 *dataBase;
extern HandledPacket hPack;


int createDbConnection() //rerurn 1 if error
{
char jou[] = "PRAGMA journal_mode = OFF;";

    char *createT = "CREATE TABLE IF NOT EXISTS PacksEr2 ("
                    "data_year INTEGER,"    //
                    "data_month INTEGER,"   //
                    "data_day INTEGER,"     //
                    "time_hour INTEGER,"    //
                    "time_min INTEGER,"     //
                    "time_sec INTEGER,"     //
                    "time_usec INTEGER,"    //
                    "protocol INTEGER,"
                    "scrabled INTEGER,"
                    "igmpMessage INTEGER,"
                    "checkSummErr INTEGER,"
                    "late INTEGER,"
                    "error INTEGER)";

    int rc = sqlite3_open(DB_PATH, &dataBase);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(dataBase));
        sqlite3_close(dataBase);
        return 1;
    }

	rc = sqlite3_exec(dataBase, jou, 0, 0, &sqlError);
	    if (rc != SQLITE_OK)
    {
        printf( "Journal mode setup error %s\n", sqlite3_errmsg(dataBase));
        sqlite3_close(dataBase);
        sqlite3_free(sqlError);
        return 1;
    }
    rc = sqlite3_exec(dataBase, createT, 0, 0, &sqlError);
    if (rc != SQLITE_OK)
    {
        printf( "Cannot  create table: %s\n", sqlite3_errmsg(dataBase));
        sqlite3_close(dataBase);
        sqlite3_free(sqlError);
        return 1;
    }
    return 0;
}
//--------------------------------------------------------------------------

int writeErrToBase() //rerurn 1 if error
{
    char sqlStr[200] = {0,};
    insertStrBuild(sqlStr);
    int rc = sqlite3_exec(dataBase, sqlStr, 0, 0, &sqlError);
    if (rc != SQLITE_OK )
    {
        printf("SQL error: %s\n", sqlError);
        sqlite3_free(sqlError);
        sqlite3_close(dataBase);
        return 1;
    }
    return -1;
}
//--------------------------------------------------------------------------

static void insertStrBuild(char *str)
 {
     char *s = str;

    memset(str,0,200);
    strcpy(str, "INSERT INTO PacksEr2 VALUES ("); // len 29
    s += 29;

        /// time insert
    sprintf(s,"%i,%i,%i,%i,%i,%i,%i,",
            hPack.localTime.tm_year+1900, hPack.localTime.tm_mon+1, hPack.localTime.tm_mday ,
            hPack.localTime.tm_hour, hPack.localTime.tm_min, hPack.localTime.tm_sec ,
            (int)hPack.timePackRecive.tv_usec );

    s = str + strlen(str);

    sprintf(s,"%i,%i,%i,%i,%i,%i);",
                hPack.protocol,
                hPack.countScrambled,
                hPack.igmpMessage,
                hPack.checkSummError,
                hPack.late,
                hPack.error );
 }
 //--------------------------------------------------------------------------

 void closeDb()
 {
     sqlite3_close(dataBase);
 }
