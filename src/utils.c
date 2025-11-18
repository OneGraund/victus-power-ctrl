#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <dirent.h>
#include <stdbool.h>

void _remove_newline(char* str) {
    char* nl_ptr = strchr(str, '\n');
    if (nl_ptr == NULL) 
        return;
     
    int nl_pos = nl_ptr - str;
    str[nl_pos] = '\0';
    return;
}

// printf-style helper used as: logMessageFmt("value=%d\n", x)
void _logmessagefmt(FILE* stream, const char* fmt, ...) { // ... means take as many arguments
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
    if (logLevel < LOG_LEVEL)
        return;
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

void logMessage(int logLevel, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    logMessageVARG(logLevel, fmt, ap);
    va_end(ap);
}

int get_hwmon_path(char* dest, size_t size) {
    struct dirent *de;

    // hwmon path then has hwmon*, where * can be any number
    DIR *dr = opendir(hwmon_base_path);
    if (!dr) {
        log_error("Could not open hwmon path");
        return -1;
    }

    char ftf[] = "hwmon"; // folder tofind
    char hwmon_found = false;
    while ((de = readdir(dr))) {
        if (!strncmp(de->d_name, ftf, strlen(ftf))) {
            hwmon_found = true;
            break;
        } 
    }
    if (!hwmon_found) {
        log_error("No base hwmon dir found");
        return -1;
    }

    char hwmon_dir[strlen(de->d_name)+1];
    strcpy(hwmon_dir, de->d_name);
    closedir(dr);

    char *hwmon_path;
    size_t hwmon_path_size = 256; 
    hwmon_path = malloc(hwmon_path_size * sizeof(char));
    if (!hwmon_path) {
        log_error("Could not allocate memory for hwmon path");
        return -1;
    }
    // compose hwmoun path by combining base and found directory
    strcpy(hwmon_path, hwmon_base_path);
    strcat(hwmon_path, hwmon_dir);

    size_t hwmon_len = strnlen(hwmon_path, hwmon_path_size);
    if (size < hwmon_len) {
        log_error("[get_hwmon_path] passed buffer is too small. increase size by %d bytes", hwmon_len-size);
        free(hwmon_path);
        return -1;
    }
    strcpy(dest, hwmon_path);
    free(hwmon_path);
    return 0;
}
