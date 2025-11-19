#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <signal.h>
#include "main.h"
#include "tests.h"
#include "utils.h"
#include "temps.h"
#include "power.h"
#include "fans.h"


void handle_exit(int sig) {
    log_info("Received signal %s, exiting...", sig);
    set_power_profile(POWER_SAVER);
    set_fan_mode(AUTO);
    exit(EXIT_SUCCESS);
}

void exit_failure(char* reason) {
    log_failure("[EXITING] reason: %s", reason);
    set_power_profile(POWER_SAVER);
    set_fan_mode(AUTO);
    exit(EXIT_FAILURE);
}

int main() {

    bool power_ctrl = 1;
    bool fans_ctrl = 1;

    struct sigaction sa = {0};
    sa.sa_handler = handle_exit;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    int test_status = test();
    if (!test_status) 
        log_info("ALL TESTS PASS, all functionality can be enabled");
    if ((test_status & FANS_FAILED)) {
        log_warning("fans failed, vicuts-power-ctrl won't control them");
        fans_ctrl = false;
    } 
    if ((test_status & THERMAL_ZONES_FAILED) || (test_status & POWER_PROFILES_FAILED)) {
        log_warning("thermal zones or power profiles failed, victus-power-ctrl won't control them");
        power_ctrl = false;
    } 

    if (!fans_ctrl && !power_ctrl)
        exit_failure("no functionality left");

    // set up linear fan curve
    int cpu_fan_curve[110]; // for each degree separate point
    double factor;
    for (int i=0;i<110;i++) {
        factor = (1.16 * i) - 46.4;
        cpu_fan_curve[i] = ((int)factor) * 100;
        if (cpu_fan_curve[i] > CPU_FAN_RPM_MAX)
            cpu_fan_curve[i] = CPU_FAN_RPM_MAX;
        else if (cpu_fan_curve[i] < 0) 
            cpu_fan_curve[i] = 0;
    }
    int gpu_fan_curve[110]; // for each degree separate point
    for (int i=0;i<110;i++) {
        factor = (1.22 * i) - 48.8;
        gpu_fan_curve[i] = ((int)factor) * 100;
        if (gpu_fan_curve[i] > GPU_FAN_RPM_MAX)
            gpu_fan_curve[i] = GPU_FAN_RPM_MAX;
        else if (gpu_fan_curve[i] < 0) 
            gpu_fan_curve[i] = 0;
    }

    
    // controll loop
    while (1) {
        // power_profile_t curProfile = get_current_pwp();
        thermal_zones_t curThermalZone = get_thermal_zone();
        // PERORMANCE   <->   BALANCED   <->   LOW_POWER
        //         CPU_BALANCED     CPU_POWERSAVE
        if (power_ctrl) {
            switch (curThermalZone)
            {
                case COOL: set_power_profile(PERFORMANCE); break;
                case MID:  set_power_profile(BALANCED);    break;
                case HOT:  set_power_profile(POWER_SAVER); break;
                default:
                    exit(EXIT_FAILURE);
                    break;
            }
        }

        int status = 0;

        int cpu_temp = read_cpu_temp();
        int gpu_temp = read_gpu_temp();

        if (fans_ctrl) {
            set_fan_mode_if_unset(MANUAL);
            if (cpu_temp >= 93 || gpu_temp >= 93) {
                set_cpu_fan_if_unset(CPU_FAN_RPM_MAX);
                set_gpu_fan_if_unset(GPU_FAN_RPM_MAX);
                log_warning("DANGEROUS TEMPERATURE");
            } else {
                status = set_cpu_fan_if_unset(cpu_fan_curve[cpu_temp]);
                if (status < 0)
                    log_warning("[control_loop] set_cpu_fan_if_unset failed");
                status = set_gpu_fan_if_unset(gpu_fan_curve[gpu_temp]);

                if (status < 0)
                    log_warning("[control_loop] set_gpu_fan_if_unset failed");
            }
        }


        sleep(CYCLE_SLEEP_TIME);
    }
}
