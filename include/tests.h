#ifndef TESTS_HEADER_H
#define TESTS_HEADER_H
#include "power.h"

#define FANS_FAILED 1 << 1
#define THERMAL_ZONES_FAILED 1 << 2
#define READ_GPU_FAILED 1 << 3
#define READ_CPU_FAILED 1 << 4
#define POWER_PROFILES_FAILED 1 << 5


// util function for test_power_profiles
int test_profile(power_profile_t profile);

// test function to test that all power profiles are correctly set
int test_power_profiles();

// see if temp can be read
int test_read_cpu_temp();

// see if temp can be read
int test_read_gpu_temp();

int test_thermal_zones();

int test_fans();

// run all tests
int test();


#endif // TESTS_HEADER_H