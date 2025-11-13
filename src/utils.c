#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// printf-style helper used as: logMessageFmt("value=%d\n", x)
void _logMessageFmt(FILE* stream, const char* fmt, ...) { // ... means take as many arguments
    if (!fmt) return;
    va_list ap;
    va_start(ap, fmt); // init ap so that it points to the first argument of fmt
    vfprintf(stream, fmt, ap);
    va_end(ap);
    fflush(stream);
}

//             4
// log(LOG_LEVEL_FAILURE, "I wanna die")
void logMessageVARG(int logLevel, const char* fmt, va_list ap) {
    if (logLevel >= LOG_LEVEL) {
        char tag[20];
        FILE* stream;
        switch (logLevel) {
            case LOG_LEVEL_DEBUG:   strcpy(tag, "  [DEBUG] "); stream = stdout; break;
            case LOG_LEVEL_INFO:    strcpy(tag, "   [INFO] "); stream = stdout; break;
            case LOG_LEVEL_WARNING: strcpy(tag, "[WARNING] "); stream = stdout; break;
            case LOG_LEVEL_ERROR:   strcpy(tag, "  [ERROR] "); stream = stderr; break;
            case LOG_LEVEL_FAILURE: strcpy(tag, "[FAILURE] "); stream = stderr; break;
            default:
                fprintf(stderr, "[UTILS] Invalid logLevel value...");
                break;
        }
        fprintf(stream, "%s", tag);
        vfprintf(stream, fmt, ap);
        fprintf(stream, "\n");
        fflush(stream);
    }
}

void logMessage(int logLevel, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    logMessageVARG(logLevel, fmt, ap);
    va_end(ap);
}
