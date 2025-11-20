#include "utils.h"
#include "fans.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


FILE* _get_pwm_fp(const char* mode) {
    char pwm_path[256];
    if (get_hwmon_path(pwm_path, 256) < 0) {
        return NULL;
    }

    strcat(pwm_path, "/pwm1_enable");
    log_debug("[_get_pwm_fp] full pwm path: %s", pwm_path);
    FILE *pwm_fptr = fopen(pwm_path, mode);
    if (!pwm_fptr) {
        log_error("[_get_pwm_fp] Could not open fan mode file");
        perror("fopen error message:");
        // Do NOT call fclose on NULL
        return NULL;
    }

    return pwm_fptr;  
}

int set_fan_speed(int fan_num, int RPM) {

    char hwmon_path[256];
    if (get_hwmon_path(hwmon_path, 256) < 0)
        return -1;
    log_debug("Full hwmon path: %s", hwmon_path);

    // append fan file
    if (fan_num == 1)
        strcat(hwmon_path, "/fan1_target");
    else if (fan_num == 2)
        strcat(hwmon_path, "/fan2_target");
    else {
        log_error("Invalid fan number %d, only 1 or 2 supported", fan_num);
        return -1;
    }

    log_debug("full fan path: %s", hwmon_path);
    FILE *hwmon_fptr = fopen(hwmon_path, "w");
    if (!hwmon_fptr) {
        log_error("Could not open fan target file");
        return -1;
    }
    char str_temp[12];
    snprintf(str_temp, sizeof(str_temp), "%d", RPM);
    fputs(str_temp, hwmon_fptr);
    fclose(hwmon_fptr);

    log_info("Set fan %d speed to %d RPM", fan_num, RPM);
    return 0;
}

int get_fan_speed(int fan_num) {
    char fan_path[256];
    if (get_hwmon_path(fan_path, 256) < 0)
        return -1;

    switch (fan_num) {
        case 1: strcat(fan_path, "/fan1_input");
            break;
        case 2:
            strcat(fan_path, "/fan2_input");
            break;
        default:
            log_error("[get_fan_speed] incorrect fan_num");
            return -1;
    }

    FILE *fan_ptr = fopen(fan_path, "r");
    log_debug("%s", fan_path);
    if (!fan_ptr) {
        log_error("[get_fan_speed] error opening %s", fan_path);
        return -1;
    }

    char fan_speed[5];
    fgets(fan_speed, 5, fan_ptr);
    log_debug("%s", fan_speed);
    int res = atoi(fan_speed);
    fclose(fan_ptr);
    return res;
}

fan_mode_t get_fan_mode() {
    FILE* pwm_fptr = _get_pwm_fp("r");
    if (!pwm_fptr) {
        log_error("[get_fan_mode] failed, pwm not open");
        return FAN_UNKNOWN;
    }

    char fan_mode_raw[2];
    fgets(fan_mode_raw, 2, pwm_fptr);
    fclose(pwm_fptr);

    switch (fan_mode_raw[0]) {
        case '2': return AUTO;
        case '1': return MANUAL;
        case '0': return MAX;
        default:
            log_error("[get_fan_mode] Unknown %s fan mode", fan_mode_raw);
            return FAN_UNKNOWN;
            break;
    }
}


int set_fan_mode(fan_mode_t fan_mode) {
    if (fan_mode == FAN_UNKNOWN) {
        log_error("[set_fan_mode] Cannot set to UNKNOWN mode");
        return -1;
    }

    FILE* pwm_ptr = _get_pwm_fp("w");
    if (!pwm_ptr) {
        log_error("[set_fan_mode] failed, pwm not open");
        return -1;
    }

    char str_fanmod[2];
    snprintf(str_fanmod, sizeof(str_fanmod), "%d", fan_mode);
    fputs(str_fanmod, pwm_ptr);
    fclose(pwm_ptr);
    log_info("Set fan mode to %d", fan_mode);
    return 0;
}

int set_fan_mode_if_unset(fan_mode_t fan_mode) {
    fan_mode_t cur_mode = get_fan_mode();
    if (cur_mode == FAN_UNKNOWN)
        return -1;

    if (get_fan_mode() != fan_mode) {
        if (set_fan_mode(fan_mode) < 0)
            return -1;
    }

    return 0;
}

int set_cpu_fan_if_unset(int RPM) {
    int cur_RPM = get_fan_speed(1);
    if (cur_RPM == -1) {
        log_error("[set_cpu_fan_if_unset] get_fan_speed failed");
        return -1;
    }
    
    if (cur_RPM != RPM) {
        int status;
        if ((status = set_fan_speed(1, RPM)) < 0) {
            log_error("[set_cpu_fan_if_unset] set_fan_speed failed: %d", status);
            return -1;
        }
    }
    return 0;
}

int set_gpu_fan_if_unset(int RPM) {
    int cur_RPM = get_fan_speed(2);
    if (cur_RPM == -1) {
        log_error("[set_gpu_fan_if_unset] get_fan_speed failed");
        return -1;
    }
    if (cur_RPM != RPM) {
        int status;
        if ((status = set_fan_speed(2, RPM)) < 0) {
            log_error("[set_gpu_fan_if_unset] set_fan_speed failed: %d", status);
            return -1;
        }
    }
    return 0;
}