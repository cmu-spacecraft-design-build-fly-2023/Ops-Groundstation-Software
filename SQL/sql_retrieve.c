/**
 * sql_retrieve.c
 * 
 * Code to retrieve data from SQL database. Code from zetcode.com
 * Link -> https://zetcode.com/db/sqlitec/
 * 
 * Purpose: Retrieving layout of messages from SQL database
 * 
 * Files edited by: D.J. Morvay (dmorvay@andrew.cmu.edu)
*/

/* Includes */
#include <sqlite3.h>
#include <stdio.h>

/* Function Prototypes */
int callback(void *NotUsed, int argc, char **argv, char **azColName);

/**
 * main
 * 
 * Purpose: Main routine for SQL retrieve.
 * Gathers information based on the APID received in a header.  
 * 
 * Inputs: NONE
 * 
 * Return: NONE
 * 
 * Saved States: NONE
*/
int main(void) {
    
    /* SQL variable declaration */
    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;
    
    /* Open SQL DB */
    int rc = sqlite3_open("DATABASE/cubesat.db", &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }
    
    /* Get message name based on APID */
    char *APID_select = "SELECT APID, \"message name\" FROM \"message definitions\" WHERE APID = 20";
        
    rc = sqlite3_prepare_v2(db, APID_select, -1, &res, 0);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(res, 1, 3);
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }
    
    int step = sqlite3_step(res);
    
    /* Print message name associated to APID */
    if (step == SQLITE_ROW) {
        printf("%s: ", sqlite3_column_text(res, 0));
        printf("%s\n\n", sqlite3_column_text(res, 1));
    } 

    /* Select full message declaration based on name */
    unsigned char* msg_name = (unsigned char*)sqlite3_column_text(res, 1);
    char msg_select[256];
    snprintf(msg_select, sizeof(msg_select), "SELECT * from \"%s\"", msg_name);

    rc = sqlite3_exec(db, msg_select, callback, 0, &err_msg);
    
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        
        return 1;
    } 

    /* Cleanup */
    sqlite3_finalize(res);
    sqlite3_close(db);
    
    return 0;
}

/**
 * callback
 * 
 * Purpose: Gathers the entire contents of an SQL Table 
*/
int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    
    NotUsed = 0;
    
    for (int i = 0; i < argc; i++) {

        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    
    printf("\n");
    
    return 0;
}