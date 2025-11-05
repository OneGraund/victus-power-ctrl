#ifndef MAIN_HEADER_H
#define MAIN_HEADER_H

#define CPU_BALANCED_TEMP_THRESHHOLD 79
#define CPU_POWERSAVE_TEMP_THRESHHOLD 81
#define CYCLE_SLEEP_TIME 1 // only integer, since temp updates min 1 sec

typedef enum power_profile_t {POWER_SAVER, BALANCED, PERFORMANCE, UNKNOWN} power_profile_t;

// acpi/platform_profile  has different name for power saving mode, therefore two sets
static const char str_read_profiles[4][20] = {"low-power", "balanced", "performance", "unknown"};
static const char str_write_profiles[4][20] = {"power-saver", "balanced", "performance", "unknown"};

static const int thermal_zones[3][2] = {
    {0, CPU_BALANCED_TEMP_THRESHHOLD}, 
    {CPU_BALANCED_TEMP_THRESHHOLD, CPU_POWERSAVE_TEMP_THRESHHOLD}, 
    {CPU_POWERSAVE_TEMP_THRESHHOLD, 200}
};
typedef enum thermal_zones_t {COOL, MID, HOT} thermal_zones_t;

// pass a string to remove '\n' if exists
void _remove_newline(char* str);

// returns cpu_temp in degrees (int)
//         -1 on error
int read_temp();

// returns pointers to power profiles
char** read_power_profiles();

// returns enum for 0-2 if okay profile, -1 if unknown
power_profile_t get_current_pwp();

// returns 1 if yes, 0 if no, -1 if failed
int is_on_ac();

// choose profile from enum and set, 0 for success, -1 for failure
int set_power_profile(power_profile_t profile);

// returns thermal_zones_t either COOL, MID or HOT. zones are defined via 
//    CPU_BALANCED_TEMP_THRESHHOLD and CPU_POWERSAVE_TEMP_THRESHHOLD
thermal_zones_t get_thermal_zone();

// util function for test_power_profiles
int test_profile(power_profile_t profile);

// test function to test that all power profiles are correctly set
int test_power_profiles();

// see if temp can be read
int test_read_temp();

#endif // MAIN_HEADER_H
