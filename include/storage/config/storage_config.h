#ifndef STORAGE_CONFIG_H
#define STORAGE_CONFIG_H

#include <stddef.h>
#include <sqlite3.h>

typedef sqlite3_int64 int64;

// --- Query Building Limits ---
#define MAX_SQL_QUERY_LEN   512

// --- SQLite Standard Values ---
// Valor padrão do SQLite para ler a string até o terminador nulo '\0'
#define SQL_AUTO_LENGTH     (-1)

// --- Buffer & Field Sizes ---
#define DDL_BUF_SIZE        ((size_t)1024)
#define DB_EMPTY_COUNT      ((size_t)0)
#define SQL_CMD_BUFFER_SIZE ((size_t)256)
#define SQL_TXT_FIELD_SIZE  ((size_t)32)

#endif // STORAGE_CONFIG_H
