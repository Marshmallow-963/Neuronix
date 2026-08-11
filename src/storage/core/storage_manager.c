#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <string.h>
#include <sqlite3.h>

#include "log/logger.h"

#include "storage/sql/sql_enums.h"
#include "storage/sql/sql_queries.h"

#include "storage/core/storage_manager.h"

#include "storage/config/storage_config.h"
#include "storage/config/storage_strings.h"

sqlite3 *db = NULL;

static inline void StorageExecuteSQL(sqlite3 *database, const char *sql, const char *tableName);

bool StorageSessionDelete(size_t sessionId) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, SQL_DELETE_SESSION_BY_ID, SQL_AUTO_LENGTH, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, SQL_BIND_IDX_1, (int64)sessionId);
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) fprintf(stderr, MSG_DELETE_SUCCESS, sessionId);
        else Logger(ERROR, MSG_DELETE_ERROR, sqlite3_errmsg(db));
    }
    else fprintf(stderr, MSG_PREPARE_ERROR, sqlite3_errmsg(db));

    sqlite3_finalize(stmt);

    // --- Reset de AutoIncrement ---
    // Verifica se o banco ficou totalmente vazio
    sqlite3_stmt *stmtCount;
    if (sqlite3_prepare_v2(db, SQL_COUNT_SESSIONS, SQL_AUTO_LENGTH, &stmtCount, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmtCount) == SQLITE_ROW) {
            size_t remaining = (size_t)sqlite3_column_int(stmtCount, SQL_COL_0);

            if (remaining == DB_EMPTY_COUNT) {
                char sqlClean[SQL_CMD_BUFFER_SIZE];

                for (size_t i = 0; i < SEQUENCE_RESET_TABLES.count; i++) {
                    snprintf(sqlClean, sizeof(sqlClean), SQL_RESET_SEQUENCE, SEQUENCE_RESET_TABLES.names[i]);
                    sqlite3_exec(db, sqlClean, NULL, NULL, NULL);
                }

                fprintf(stderr, "%s", MSG_CLEAN_COMPLETE);
            }
        }
    }
    sqlite3_finalize(stmtCount);
    return true;
}

bool StorageSessionDataTableDelete(const char *tableName, size_t sessionId) {
    if (!db || !tableName) return false;

    // 1. Validação de Segurança: Garante que a tabela está na lista de tabelas de trace
    bool isValid = (strcmp(tableName, TBL_TAU) == 0);
    for (size_t i = 0; i < ALL_TRACE_TABLES.count; i++) {
        if (strcmp(tableName, ALL_TRACE_TABLES.names[i]) == 0) {
            isValid = true;
            break;
        }
    }

    if (!isValid) {
        Logger(ERROR, "Segurança: Tentativa de deletar dados de tabela não autorizada: %s\n", tableName);
        return false;
    }

    // 2. Construção da Query (O nome da tabela é concatenado, o ID é via bind)
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM %s WHERE session_id = ?;", tableName);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, SQL_AUTO_LENGTH, &stmt, NULL) != SQLITE_OK) {
        Logger(ERROR, "Falha ao preparar remoção de tabela: %s\n", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, SQL_BIND_IDX_1, (int)sessionId);

    // 3. Execução
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        Logger(ERROR, "Erro ao remover dados da tabela %s (Sessão %zu): %s\n", tableName, sessionId, sqlite3_errmsg(db));
        return false;
    }

    return true;
}

bool StorageSessionNotesGet(size_t sessionId, char *buffer, size_t bufferSize) {
    if (!db || !buffer) return false;

    bool sucess = false;
    buffer[0] = '\0';
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, SQL_SELECT_NOTES_BY_ID, SQL_AUTO_LENGTH, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, SQL_BIND_IDX_1, (int)sessionId);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *dbTxt = (const char*)sqlite3_column_text(stmt, SQL_COL_0);
            if (dbTxt) {
                snprintf(buffer, bufferSize, "%s", dbTxt);
                sucess = true;
            }
        }
    }

    sqlite3_finalize(stmt);
    return sucess;
}

size_t StorageSessionsFetch(SessionEntry *list, size_t maxCount) {
    if (!db) return 0;

    size_t count = 0;
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, timestamp, notes, total_time FROM Simulation_Sessions ORDER BY id DESC LIMIT ?;";

    if (sqlite3_prepare_v2(db, sql, SQL_AUTO_LENGTH, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, SQL_BIND_IDX_1, (int)maxCount);

        while (sqlite3_step(stmt) == SQLITE_ROW && count < maxCount) {
            list[count].id = (size_t)sqlite3_column_int(stmt, SQL_COL_0);

            // Tratamento de NULL
            const char *ts = (const char*)sqlite3_column_text(stmt, SQL_COL_1);
            const char *nt = (const char*)sqlite3_column_text(stmt, SQL_COL_2);

            snprintf(list[count].timestamp, 32, "%s", ts ? ts : "Unknow");
            snprintf(list[count].notes, 32, "%s", nt ? nt : "");

            list[count].totalTime = (float)sqlite3_column_double(stmt, SQL_COL_3);

            count++;
        }
    }
    else Logger(ERROR, "DB Error: %s\n", sqlite3_errmsg(db));

    sqlite3_finalize(stmt);
    return count;
}

size_t StorageSessionDataTablesGet(char outTableNames[][64], size_t maxCount, size_t sessionId) {
    if (!db) return 0;

    char sqlQuery[128];
    sqlite3_stmt *stmt;
    size_t foundCount = 0;

    // 1. Verificação explícita para a tabela Tau
    if (foundCount < maxCount) {
        snprintf(sqlQuery, sizeof(sqlQuery), "SELECT 1 FROM %s WHERE session_id = ? LIMIT 1", TBL_TAU);

        if (sqlite3_prepare_v2(db, sqlQuery, SQL_AUTO_LENGTH, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(stmt, SQL_BIND_IDX_1, (int)sessionId);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                snprintf(outTableNames[foundCount], 64, "%s", TBL_TAU);
                foundCount++;
            }
        }
        sqlite3_finalize(stmt);
    }

    // 2. Iteração sobre as demais tabelas de trace
    for (size_t i = 0; i < ALL_TRACE_TABLES.count; i++) {
        if (foundCount >= maxCount) break;

        snprintf(sqlQuery, sizeof(sqlQuery), "SELECT 1 FROM %s WHERE session_id = ? LIMIT 1", ALL_TRACE_TABLES.names[i]);

        if (sqlite3_prepare_v2(db, sqlQuery, SQL_AUTO_LENGTH, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(stmt, SQL_BIND_IDX_1, (int)sessionId);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                snprintf(outTableNames[foundCount], 64, "%s", ALL_TRACE_TABLES.names[i]);
                foundCount++;
            }
        }
        sqlite3_finalize(stmt);
    }
    return foundCount;
}

bool StorageSessionMetadataUpdate(const char *notes, double totalTime, size_t sessionId) {
    if (!db) return false;

    bool sucess = false;
    sqlite3_stmt *stmt;

    const char *sql = "UPDATE Simulation_Sessions SET notes = ?, total_time = ? WHERE id = ?";

    if (sqlite3_prepare_v2(db, sql, SQL_AUTO_LENGTH, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, SQL_BIND_IDX_1, notes ? notes : "", SQL_AUTO_LENGTH, SQLITE_STATIC);
        sqlite3_bind_double(stmt, SQL_BIND_IDX_2, totalTime);
        sqlite3_bind_int64(stmt, SQL_BIND_IDX_3, (int64)sessionId);

        if (sqlite3_step(stmt) == SQLITE_DONE) sucess = true;
        else {
            Logger(ERROR, "Erro ao atualizar metadados da sessao %zu: %s\n", sessionId,
            sqlite3_errmsg(db));
        }
    }
    else Logger(ERROR, "Falha ao preparar update de metadados: %s\n", sqlite3_errmsg(db));

    sqlite3_finalize(stmt);
    return sucess;
}

bool StorageSessionExport(const char *filePath, size_t sessionID) {
    if (!db || !filePath || filePath[0] == '\0') return false;

    FILE *file = fopen(filePath, "w");
    if (!file) {
        Logger(ERROR, "Erro ao criar arquivo para sessão: %s\n", filePath);
        return false;
    }

    // 1. Lista com as tabelas fixas do sistema
    const char *staticTables[] = {
        "Simulation_Sessions",
        "Network_Neurons",
        "Network_Synapses",
        "Spike_Events",
        "Tau"
    };
    size_t staticCount = sizeof(staticTables) / sizeof(staticTables[0]);
    size_t totalTables = staticCount + ALL_TRACE_TABLES.count;

    // 2. Iteração sobre todas as tabelas (fixas + tabelas de trace)
    for (size_t i = 0; i < totalTables; i++) {
        const char *tName = (i < staticCount) ? staticTables[i] : ALL_TRACE_TABLES.names[i - staticCount];

        char sql[256];
        // Simulation_Sessions usa a chave 'id', as demais tabelas usam 'session_id'
        if (strcmp(tName, "Simulation_Sessions") == 0) {
            snprintf(sql, sizeof(sql), "SELECT * FROM %s WHERE id = ?;", tName);
        } else {
            snprintf(sql, sizeof(sql), "SELECT * FROM %s WHERE session_id = ?;", tName);
        }

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql, SQL_AUTO_LENGTH, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, SQL_BIND_IDX_1, (sqlite3_int64)sessionID);

            int colCount = sqlite3_column_count(stmt);
            bool headerWritten = false;

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                // Escreve o nome da tabela e os cabeçalhos apenas se houver registros
                if (!headerWritten) {
                    fprintf(file, "=== TABLE: %s ===\n", tName);
                    for (int c = 0; c < colCount; c++) {
                        fprintf(file, "\"%s\"%s", sqlite3_column_name(stmt, c), (c == colCount - 1) ? "\n" : ",");
                    }
                    headerWritten = true;
                }

                // Escreve os dados linha por linha dinamicamente
                for (int c = 0; c < colCount; c++) {
                    const char *val = (const char*)sqlite3_column_text(stmt, c);
                    fprintf(file, "\"%s\"%s", val ? val : "", (c == colCount - 1) ? "\n" : ",");
                }
            }

            if (headerWritten) {
                fprintf(file, "\n"); // Quebra de linha entre tabelas
            }

            sqlite3_finalize(stmt);
        } else {
            Logger(ERROR, "Erro ao preparar SQL para exportação da tabela %s: %s\n", tName, sqlite3_errmsg(db));
        }
    }

    fclose(file);
    Logger(INFO, "Sessão %zu exportada com sucesso para %s", sessionID, filePath);
    return true;
}

bool StorageTableExport(const char *tableName, const char *filePath, size_t sessionID) {
    if (!db || !tableName || !filePath || filePath[0] == '\0') return false;

    FILE *file = fopen(filePath, "w");
    if (!file) {
        Logger(ERROR, "Erro ao criar o arquivo CSV: %s\n", filePath);
        return false;
    }

    char sql[256];
    if (strcmp(tableName, TBL_SESSIONS) == 0 || strcmp(tableName, "Simulation_Sessions") == 0) {
        snprintf(sql, sizeof(sql), "SELECT * FROM %s WHERE id = ?;", tableName);
    } else {
        snprintf(sql, sizeof(sql), "SELECT * FROM %s WHERE session_id = ?;", tableName);
    }

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, SQL_AUTO_LENGTH, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, SQL_BIND_IDX_1, (int64)sessionID);

        int colCount = sqlite3_column_count(stmt);

        // 1. Escreve os cabeçalhos das colunas
        for (int i = 0; i < colCount; i++) {
            fprintf(file, "\"%s\"%s", sqlite3_column_name(stmt, i), (i == colCount - 1) ? "\n" : ",");
        }

        // 2. Escreve os dados linha por linha
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            for (int i = 0; i < colCount; i++) {
                const char *val = (const char*)sqlite3_column_text(stmt, i);
                fprintf(file, "\"%s\"%s", val ? val : "", (i == colCount - 1) ? "\n" : ",");
            }
        }

        sqlite3_finalize(stmt);
    } else {
        Logger(ERROR, "Erro ao preparar SQL para exportação da tabela %s: %s\n", tableName, sqlite3_errmsg(db));
        fclose(file);
        return false;
    }

    fclose(file);
    Logger(INFO, "Tabela %s exportada com sucesso para %s", tableName, filePath);
    return true;
}

size_t StorageSpikeCountGet(size_t sessionId) {
    if (!db) return 0;

    size_t count = 0;
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, SQL_COUNT_SPIKES_BY_SESSION, SQL_AUTO_LENGTH, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, SQL_BIND_IDX_1, (int)sessionId);

        if (sqlite3_step(stmt) == SQLITE_ROW) count = (size_t)sqlite3_column_int(stmt, SQL_COL_0);
    }

    sqlite3_finalize(stmt);
    return count;
}

void StorageClose() {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}

void StorageInit(const char *dbName) {
    int rc = sqlite3_open(dbName, &db);
    if (rc != SQLITE_OK) return;

    sqlite3_busy_timeout(db, 5000);

    // Habilita chaves estrangeiras
    sqlite3_exec(db, SQL_ENABLE_FOREIGN_KEYS, NULL, NULL, NULL);
    sqlite3_exec(db, SQL_ENABLE_WAL_MODE,     NULL, NULL, NULL);
    sqlite3_exec(db, SQL_ENABLE_SYNCHRONOUS,  NULL, NULL, NULL);

    StorageExecuteSQL(db, SQL_CREATE_NETWORK_NEURONS,  TBL_NET_NRN);
    StorageExecuteSQL(db, SQL_CREATE_NETWORK_SYNAPSES, TBL_NET_SYN);
    StorageExecuteSQL(db, SQL_CREATE_SESSIONS,         TBL_SESSIONS);
    StorageExecuteSQL(db, SQL_CREATE_SPIKES,           TBL_SPIKES);
    StorageExecuteSQL(db, SQL_CREATE_SPIKES_INDEX,     TBL_SPIKES);
    StorageExecuteSQL(db, SQL_CREATE_TAU,              TBL_TAU);
    StorageExecuteSQL(db, SQL_CREATE_TAU_INDEX,        TBL_TAU);

    char sqlBuffer[DDL_BUF_SIZE];
    for (size_t i = 0; i < ALL_TRACE_TABLES.count; i ++) {
        if (snprintf(sqlBuffer, DDL_BUF_SIZE, SQL_CREATE_TRACE_TEMPLATE, ALL_TRACE_TABLES.names[i])) {
            StorageExecuteSQL(db, sqlBuffer, ALL_TRACE_TABLES.names[i]);
        }

        if (snprintf(sqlBuffer, DDL_BUF_SIZE, SQL_CREATE_INDEX_TEMPLATE, ALL_TRACE_TABLES.names[i], ALL_TRACE_TABLES.names[i])) {
            StorageExecuteSQL(db, sqlBuffer, ALL_TRACE_TABLES.names[i]);
        }
    }

    fprintf(stderr, "%s", MSG_DB_INIT_SUCCESS);
}

static inline void StorageExecuteSQL(sqlite3 *database, const char *sql, const char *tableName) {
    char *errMsg = NULL;

    if (sqlite3_exec(database, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL Error: %s: %s\n", tableName, errMsg);
        sqlite3_free(errMsg);
    }
}
