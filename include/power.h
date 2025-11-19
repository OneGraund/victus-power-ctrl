#ifndef POWER_HEADER_H
#define POWER_HEADER_H
#include "main.h" // for thresholds

typedef enum power_profile_t {POWER_SAVER, BALANCED, PERFORMANCE, PWP_UNKNOWN} power_profile_t;

// acpi/platform_profile  has different name for power saving mode, therefore two sets
static const char str_read_profiles[4][20] = {"low-power", "balanced", "performance", "unknown"};
static const char str_write_profiles[4][20] = {"power-saver", "balanced", "performance", "unknown"};

static const int thermal_zones[3][2] = {
    {0, CPU_BALANCED_TEMP_THRESHHOLD}, 
    {CPU_BALANCED_TEMP_THRESHHOLD, CPU_POWERSAVE_TEMP_THRESHHOLD}, 
    {CPU_POWERSAVE_TEMP_THRESHHOLD, 200}
};

// returns pointers to power profiles
char** read_power_profiles();

// returns enum for 0-2 if okay profile, -1 if unknown
power_profile_t get_current_pwp();

// choose profile from enum and set, 0 for success, -1 for failure
int set_power_profile(power_profile_t profile);

// returns 1 if yes, 0 if no, -1 if failed
//  NOTE: considering to remove this, no use case found
int is_on_ac();

#endif // POWER_HEADER_H