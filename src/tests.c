#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include "tests.h"
#include "main.h"
#include "utils.h"
#include "temps.h"
#include "power.h"
#include "fans.h"

int test_profile(power_profile_t profile) {
    set_power_profile(profile);
    if (get_current_pwp() != profile)
        return -1;
    return 0;
}

int test_power_profiles() {
    power_profile_t before_test = get_current_pwp();
    for (int i = 0; i < 3; i++) {
        if (test_profile((power_profile_t)i) == -1) {
            log_error("[TEST FAILED] Expected profile: %s, but is: %s", 
                str_read_profiles[i], str_read_profiles[get_current_pwp()]);
            return -1;
        }
    }
    log_info("[TEST OK] All test profiles successfuly can be set and unset");
    set_power_profile(before_test);
    return 0;
}

int test_read_cpu_temp() {
    int temp;
    if ((temp = read_cpu_temp()) == -1) {
        log_error("[TEST FAILED] Cannot read temperature");
        return 1;
    }
    log_info("[TEST OK] Can read CPU temperature: %d", temp);
    return 0;
}

int test_read_gpu_temp() { 
    int temp;
    if ((temp = read_gpu_temp()) == -1) {
        log_error("[TEST FAILED] Cannot read GPU temperature");
        return -1;
    }
    log_info("[TEST OK] Can read GPU temperature: %d", temp);
    return 0;
}

int test_fans() {
    if (get_fan_mode() == FAN_UNKNOWN) {
        log_error("[TEST FAILED] Cannot read fan mode");
        return -1;
    }
    log_info("[TEST OK] Can read fan mode");
    if (set_fan_mode(MANUAL) != 0) {
        log_error("[TEST FAILED] Cannot set fan mode to MANUAL");
        return -1;
    }
    sleep(1);
    if (get_fan_mode() != MANUAL) {
        log_error("[TEST FAILED] Fan mode is not MANUAL after setting it to MANUAL");
        return -1;
    }
    log_info("[TEST OK] Can set and read MANUAL fan mode");
    if (set_fan_mode(MAX) != 0) {
        log_error("[TEST FAILED] Cannot set fan mode to MAX");
        return -1;
    }
    sleep(1);
    if (get_fan_mode() != MAX) {
        log_error("[TEST FAILED] Fan mode is not MAX after setting it to MAX");
        return -1;
    }
    log_info("[TEST OK] Can set and read MAX fan mode");
    if (set_fan_mode(AUTO) != 0) {
        log_error("[TEST FAILED] Cannot set fan mode to AUTO");
        return -1;
    }
    sleep(1);
    if (get_fan_mode() != AUTO) {
        log_error("[TEST FAILED] Fan mode is not AUTO after setting it to AUTO");
        return -1;
    }
    log_info("[TEST OK] Can set and read AUTO fan mode");
    return 0;
}

int test_thermal_zones() {
    thermal_zones_t zone = get_thermal_zone();
    if (zone == TZ_UNKNOWN) {
        log_error("[TEST FAILED] Cannot read thermal zone");
        return -1;
    }
    log_info("[TEST OK] Can read thermal zone: %d", zone);
    return 0;
}

int test() {
    int status = 0;
    status += test_power_profiles();
    status <<= 1;
    status += test_read_cpu_temp();
    status <<= 1;
    status += test_read_gpu_temp();
    status <<= 1;
    status += test_thermal_zones();
    status <<= 1;
    status += test_fans();
    status <<= 1;
    log_info("[TEST RESULT] errors: %u", status);
    return status;
}