#ifndef TESTS_HEADER_H
#define TESTS_HEADER_H
#include "main.h"

// util function for test_power_profiles
int test_profile(power_profile_t profile);

// test function to test that all power profiles are correctly set
int test_power_profiles();

// see if temp can be read
int test_read_temp();

// run all tests
int test();

#endif // TESTS_HEADER_H