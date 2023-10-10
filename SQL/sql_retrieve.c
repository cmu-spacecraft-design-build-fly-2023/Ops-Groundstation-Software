/**
 * sql_retrieve.c
 * 
 * Code to retrieve data from SQL database. Code from zetcode.com.
 * Link -> https://zetcode.com/db/sqlitec/
 * 
 * Purpose: Retrieving various metadata from Cubesat SQL database. 
 * 
 * Receive payload information, such as message names and expected 
 * message lengths based on header APID
 * 
 * Receieve signal information, such as start bits, end bits, factors, 
 * and offsets based on a signal name.
 * 
 * Files edited by: D.J. Morvay (dmorvay@andrew.cmu.edu)
*/

/* Includes */
#include <sqlite3.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sql.h"

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
int main() {
    header rec_header;
    message rec_msg;
    signal rec_signal;

    rec_header.APID = 20;
    strncpy(rec_signal.signal_name, "vCell_1", sizeof(rec_signal.signal_name));

    get_payload_message(rec_header, &rec_msg);
    get_start_end(rec_msg, &rec_signal);

    return 0;
}

/**
 * @name: get_payload_message
 * 
 * @details: Retrieve message payload information from SQL database.
 * Returns the message name and message length based on the APID. 
 * 
 * Inputs:
 *  cube_header - Received header containing the APID
 * 
 * Outputs
 *  cube_msg - Messaged information such as message name and length
 * 
 * @return: 
 *  -1 == Error
 *   0 == No match
 *   1 == Match
 * 
 * Saved States: NONE
*/
int get_payload_message(header cube_header, message* cube_msg) {

    /* SQL variable declaration */
    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;
    
    /* Open SQL DB */
    int rc = sqlite3_open("DATABASE/cubesat.db", &db);
    
    /* Check if SQL database exists */
    /* Return -1 if cannot open */
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return -1;
    }

    /* Setup SQL search */
    char req_sb[256];
    snprintf(req_sb, sizeof(req_sb), "SELECT \"message name\", \"message length\" FROM \"message definitions\" WHERE APID = ?;");

    sqlite3_stmt* stmt;

    /* Prepare the SQL query */
    /* Return -1 if statement has an error */
    rc = sqlite3_prepare_v2(db, req_sb, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);

        return -1;
    }

    /* Bind the APID parameter */
    /* Return -1 if APID cannot bind */
    rc = sqlite3_bind_int(stmt, 1, cube_header.APID);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);

        return -1;
    }

    /* Execute the query and retrieve the result */
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        strncpy((*cube_msg).message_name, (char*)sqlite3_column_text(stmt, 0), sizeof((*cube_msg).message_name));
        (*cube_msg).message_length = (uint16_t)sqlite3_column_int(stmt, 1);
        /* Process the retrieved value here */
        printf("Message name: %s\n", (*cube_msg).message_name);
        printf("Message length: %u\n", (*cube_msg).message_length);
    } else {
        fprintf(stderr, "No matching rows found.\n");
        return 0;
    }

    /* Cleanup */
    sqlite3_finalize(res);
    sqlite3_close(db);

    return 1;
}

/**
 * @name: get_start_end
 * 
 * @details: Retrieve signal information from SQL database.
 * Returns the signal start and end bits based on the signal name. 
 * 
 * Inputs:
 *  cube_msg - Message which contains the lookup signal. 
 * 
 * Outputs
 *  cube_signal - Signal information such as start bit and end bit
 * 
 * @return: 
 *  -1 == Error
 *   0 == No match
 *   1 == Match
 * 
 * Saved States: NONE
*/
int get_start_end(message cube_msg, signal* cube_signal) {
    
    /* SQL variable declaration */
    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;
    
    /* Open SQL DB */
    int rc = sqlite3_open("DATABASE/cubesat.db", &db);
    
    /* Check if SQL database exists */
    /* Return -1 if cannot open */
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    /* Setup SQL search */
    char req_sb[256];
    snprintf(req_sb, sizeof(req_sb), "SELECT \"start bit\", \"end bit\" FROM %s WHERE signal = ?;", cube_msg.message_name);

    sqlite3_stmt* stmt;

    /* Prepare the SQL query */
    /* Return -1 if statement has an error */
    rc = sqlite3_prepare_v2(db, req_sb, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }

    /* Bind the signal name parameter */
    /* Return -1 if signal name cannot bind */
    rc = sqlite3_bind_text(stmt, 1, (*cube_signal).signal_name, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }

    /* Execute the query and retrieve the result */
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        (*cube_signal).start_bit = (uint16_t)sqlite3_column_int(stmt, 0);
        (*cube_signal).end_bit = (uint16_t)sqlite3_column_int(stmt, 1);
        /* Process the retrieved value here */
        printf("Start bit: %u\n", (*cube_signal).start_bit);
        printf("End bit: %u\n", (*cube_signal).end_bit);
    } else {
        fprintf(stderr, "No matching rows found.\n");
        return 0;
    }

    /* Cleanup */
    sqlite3_finalize(res);
    sqlite3_close(db);

    return 1;
}