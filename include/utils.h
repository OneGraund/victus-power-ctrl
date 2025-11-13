#ifndef UTILS_HEADER_H
#define UTILS_HEADER_H
#include <stdarg.h>

#define LOG_LEVEL_FAILURE 4
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_DEBUG 0

#define log_debug(...)  logMessage(LOG_LEVEL_DEBUG,   __VA_ARGS__)
#define log_info(...)  logMessage(LOG_LEVEL_INFO,   __VA_ARGS__)
#define log_warning(...)  logMessage(LOG_LEVEL_WARNING,   __VA_ARGS__)
#define log_error(...)  logMessage(LOG_LEVEL_ERROR,   __VA_ARGS__)
#define log_failure(...)  logMessage(LOG_LEVEL_FAILURE,   __VA_ARGS__)


#define LOG_LEVEL LOG_LEVEL_DEBUG

void logMessageVARG(int logLevel, const char* fmt, va_list ap);
void logMessage(int logLevel, const char* fmt, ...);

#endif // UTILS_HEADER_H