#ifndef UTILS_HEADER_H
#define UTILS_HEADER_H
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#define LOG_LEVEL_FAILURE 4
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_DEBUG 0

#define LOG_LEVEL LOG_LEVEL_WARNING

#define log_debug(...)  logMessage(LOG_LEVEL_DEBUG,   __VA_ARGS__)
#define log_info(...)  logMessage(LOG_LEVEL_INFO,   __VA_ARGS__)
#define log_warning(...)  logMessage(LOG_LEVEL_WARNING,   __VA_ARGS__)
#define log_error(...)  logMessage(LOG_LEVEL_ERROR,   __VA_ARGS__)
#define log_failure(...)  logMessage(LOG_LEVEL_FAILURE,   __VA_ARGS__)


// pass a string to remove '\n' if exists
void _remove_newline(char* str);

void _logmessagefmt(FILE* stream, const char* fmt, ...);
void logMessageVARG(int logLevel, const char* fmt, va_list ap);
void logMessage(int logLevel, const char* fmt, ...);

// hwmon stuff
static const char hwmon_base_path[] = "/sys/devices/platform/hp-wmi/hwmon/";

// gives out: /sys/devices/platform/hp-wmi/hwmon/hwmon6/
// size_t size is to prevent buffer overflow (specifies dest size)
// returns 0 on success, -1 on failure
int get_hwmon_path(char* dest, size_t size);

#endif // UTILS_HEADER_H
