#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    INFO,
    DEBUG,
    WARNING,
    ERROR
} LogLevel;

void Logger(int logLevel, const char *txt, ...);

#endif // LOGGER_H
