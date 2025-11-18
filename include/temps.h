#ifndef TEMPS_HEADER_H
#define TEMPS_HEADER_H

typedef enum thermal_zones_t {COOL, MID, HOT, TZ_UNKNOWN} thermal_zones_t;
// returns cpu_temp in degrees (int)
//         -1 on error
int read_cpu_temp();

// returns gpu_temp in degrees (int)
//         -1 on error
int read_gpu_temp();

// returns thermal_zones_t either COOL, MID or HOT. zones are defined via 
//    CPU_BALANCED_TEMP_THRESHHOLD and CPU_POWERSAVE_TEMP_THRESHHOLD
thermal_zones_t get_thermal_zone();

#endif // TEMPS_HEADER_H