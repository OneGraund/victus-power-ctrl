#ifndef MAIN_HEADER_H
#define MAIN_HEADER_H

#define CPU_BALANCED_TEMP_THRESHHOLD 77
#define CPU_POWERSAVE_TEMP_THRESHHOLD 80
#define CYCLE_SLEEP_TIME 1 // only integer, since temp updates min 1 sec
#define CYCLE_CHECK_BAT_TIME 5

typedef enum power_profile_t {POWER_SAVER, BALANCED, PERFORMANCE, UNKNOWN} power_profile_t;

// hwmon stuff
static const char hwmon_base_path[] = "/sys/devices/platform/hp-wmi/hwmon/";

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
//  NOTE: considering to remove this, no use case found
int is_on_ac();

// choose profile from enum and set, 0 for success, -1 for failure
int set_power_profile(power_profile_t profile);

// returns thermal_zones_t either COOL, MID or HOT. zones are defined via 
//    CPU_BALANCED_TEMP_THRESHHOLD and CPU_POWERSAVE_TEMP_THRESHHOLD
thermal_zones_t get_thermal_zone();

// signal handler to catch SIGINT and SIGTERM
//  on signal, set power profile to POWER_SAVER and exit
void handle_exit(int sig);

int get_fan_speed(int fan_num);

int set_fan_speed(int fan_num, int RPM);

#endif // MAIN_HEADER_H