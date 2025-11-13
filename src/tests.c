#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include "tests.h"
#include "main.h"

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
            fprintf(stderr, "[TEST FAILED] Expected profile: %s, but is: %s", 
                str_read_profiles[i], str_read_profiles[get_current_pwp()]);
            return -1;
        }
    }
    printf("[TEST OK] All test profiles successfuly can be set and unset\n");
    set_power_profile(before_test);
    return 0;
}

int test_read_temp() {
    if (read_temp() == -1) {
        fprintf(stderr, "[TEST FAILED] Cannot read temperature\n");
        return 1;
    }
    printf("[TEST OK] Can read temperature\n");
    return 0;
}

int test() {
    int status = 0;
    status  += test_power_profiles();
    status  += test_read_temp();
    return status;
}