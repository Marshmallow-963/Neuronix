#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#include "log/logger.h"

static const char *PATH_LOG = "data/neurolab.log";

static void vLogger(int logLevel, const char *txt, va_list args);

void Logger(int logLevel, const char *txt, ...) {
    va_list args;

    va_start(args, txt);
    vLogger(logLevel, txt, args);
    va_end(args);
}

static void vLogger(int logLevel, const char *txt, va_list args) {
    if (!txt) return;

    FILE *logFile = fopen(PATH_LOG, "a");
    if (!logFile) return;

    char timeStr[32];
    time_t now = time(NULL);

    struct tm *tmInfo = localtime(&now);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tmInfo);

    const char *levelStr[] = {"INFO", "DEBUG", "WARNING", "ERROR"};
    const char *label = (logLevel >= 0 && logLevel <= 3) ? levelStr[logLevel] : "UNKNOWN";

    fprintf(logFile, "[%s] [%s] ", timeStr, label);
    vfprintf(logFile, txt, args);
    fprintf(logFile, "\n");
    fclose(logFile);
}
