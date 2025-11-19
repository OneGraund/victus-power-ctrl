#ifndef FANS_HEADER_H
#define FANS_HEADER_H
#include <stdio.h>

typedef enum fan_mode_t {MAX, MANUAL, AUTO, FAN_UNKNOWN} fan_mode_t;

#define CPU_FAN_RPM_MAX 5800
#define GPU_FAN_RPM_MAX 6100

// helper function for get_fan_mode/set_fan_mode
// returns NULL on failure, pwm1_enable file pointer on success
FILE* _get_pwm_fp(const char* mode);

// return -1 if failed, actual value otherwise (DON'T use !return_value)
//  NOT IMPLEMENTED YET
int get_fan_speed(int fan_num);

// return -1 on failure, 0 on success. fan_num is [1, 2]
int set_fan_speed(int fan_num, int RPM);

// return UNKNOWN if failed
fan_mode_t get_fan_mode();

// return -1 on failure, 0 on success
int set_fan_mode(fan_mode_t fan_mode);
// read set_cpu(gpu)_fan_if_unset coment
int set_fan_mode_if_unset(fan_mode_t fan_mode);

// both do pretty much what they describe. call is made to get_fan_speed
//  call to set_fan_speed only done if RPM !=
int set_cpu_fan_if_unset(int RPM);
int set_gpu_fan_if_unset(int RPM);
#endif