/* Minimal SQLite3 stub implementation for testing */
/* This provides a basic in-memory implementation without persistence */

#include "sqlite3.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Simple in-memory stub - just returns OK/DONE for all operations */
/* This is NOT a real database - just enough to make tests compile/link */

struct sqlite3 {
    char dummy;
};

struct sqlite3_stmt {
    char dummy;
};

int sqlite3_open(const char* filename, sqlite3** ppDb) {
    *ppDb = (sqlite3*)malloc(sizeof(sqlite3));
    return SQLITE_OK;
}

int sqlite3_close(sqlite3* db) {
    if (db) free(db);
    return SQLITE_OK;
}

int sqlite3_exec(sqlite3* db, const char* sql, 
                  int (*callback)(void*,int,char**,char**), 
                  void* arg, char** errmsg) {
    /* Stub: accept all SQL */
    (void)db; (void)sql; (void)callback; (void)arg; (void)errmsg;
    return SQLITE_OK;
}

const char* sqlite3_errmsg(sqlite3* db) {
    (void)db;
    return "stub implementation";
}

int sqlite3_prepare_v2(sqlite3* db, const char* zSql, int nByte, 
                        sqlite3_stmt** ppStmt, const char** pzTail) {
    (void)db; (void)zSql; (void)nByte; (void)pzTail;
    *ppStmt = (sqlite3_stmt*)malloc(sizeof(sqlite3_stmt));
    return SQLITE_OK;
}

int sqlite3_step(sqlite3_stmt* pStmt) {
    (void)pStmt;
    return SQLITE_DONE; /* No rows */
}

int sqlite3_finalize(sqlite3_stmt* pStmt) {
    if (pStmt) free(pStmt);
    return SQLITE_OK;
}

sqlite3_int64 sqlite3_last_insert_rowid(sqlite3* db) {
    (void)db;
    static int id_counter = 0;
    return ++id_counter;
}

int sqlite3_changes(sqlite3* db) {
    (void)db;
    return 1;
}

void sqlite3_free(void* p) {
    free(p);
}

int sqlite3_bind_int(sqlite3_stmt* pStmt, int index, int value) {
    (void)pStmt; (void)index; (void)value;
    return SQLITE_OK;
}

int sqlite3_bind_int64(sqlite3_stmt* pStmt, int index, sqlite3_int64 value) {
    (void)pStmt; (void)index; (void)value;
    return SQLITE_OK;
}

int sqlite3_bind_double(sqlite3_stmt* pStmt, int index, double value) {
    (void)pStmt; (void)index; (void)value;
    return SQLITE_OK;
}

int sqlite3_bind_text(sqlite3_stmt* pStmt, int index, const char* value, int n, void (*destructor)(void*)) {
   (void)pStmt; (void)index; (void)value; (void)n; (void)destructor;
    return SQLITE_OK;
}

int sqlite3_bind_null(sqlite3_stmt* pStmt, int index) {
    (void)pStmt; (void)index;
    return SQLITE_OK;
}

int sqlite3_column_type(sqlite3_stmt* pStmt, int iCol) {
    (void)pStmt; (void)iCol;
    return SQLITE_NULL;
}

int sqlite3_column_int(sqlite3_stmt* pStmt, int iCol) {
    (void)pStmt; (void)iCol;
    return 0;
}

sqlite3_int64 sqlite3_column_int64(sqlite3_stmt* pStmt, int iCol) {
    (void)pStmt; (void)iCol;
    return 0;
}

double sqlite3_column_double(sqlite3_stmt* pStmt, int iCol) {
    (void)pStmt; (void)iCol;
    return 0.0;
}

const unsigned char* sqlite3_column_text(sqlite3_stmt* pStmt, int iCol) {
    (void)pStmt; (void)iCol;
    return (const unsigned char*)"";
}

const void* sqlite3_column_blob(sqlite3_stmt* pStmt, int iCol) {
    (void)pStmt; (void)iCol;
    return NULL;
}

int sqlite3_column_bytes(sqlite3_stmt* pStmt, int iCol) {
    (void)pStmt; (void)iCol;
    return 0;
}
