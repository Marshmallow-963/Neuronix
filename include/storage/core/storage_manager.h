#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <stddef.h>
#include <stdbool.h>

#include "storage/core/storage_state.h"

bool StorageSessionDataTableDelete(const char *tableName, size_t sessionId);
bool StorageSessionDelete(size_t sessionId);
bool StorageSessionMetadataUpdate(const char *notes, double totalTime, size_t sessionId);
bool StorageSessionNotesGet(size_t sessionId, char *buffer, size_t bufferSize);

bool StorageSessionExport(const char *filePath, size_t sessionID);
bool StorageTableExport(const char *tableName, const char *filePath, size_t sessionID);

size_t StorageSessionDataTablesGet(char outTableNames[][64], size_t maxCount, size_t sessionId);
size_t StorageSessionsFetch(SessionEntry *list, size_t maxCount);
size_t StorageSpikeCountGet(size_t sessionId);

void StorageClose(void);
void StorageInit(const char *dbName);

#endif // STORAGE_MANAGER_H
