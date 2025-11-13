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

void _remove_newline(char* str) {
    char* nl_ptr = strchr(str, '\n');
    if (nl_ptr == NULL) 
        return;
     
    int nl_pos = nl_ptr - str;
    str[nl_pos] = '\0';
    return;
}

int read_temp() {
    FILE *fptr;
    char tmpFileLoc[] = "/sys/class/thermal/thermal_zone0/temp";
    fptr = fopen(tmpFileLoc, "r");
    if (fptr == NULL) {
        log_error("[read_temp] fopen temp file error message:");
        fclose(fptr);
        return -1;
    }
    char strTemp[7];
    fgets(strTemp, 7, fptr);
    fclose(fptr);
    int intTemp = atoi(strTemp) / 1000;
    if (!intTemp) {
        log_error("Error reading from CPU temperature file");
        return -1;
    }
    log_debug("CPU temp: %d C", intTemp);
    return intTemp;
}

char** read_power_profiles() {
    char** power_profiles = malloc(3 * sizeof(char*));
    FILE *fptr;
    char pwpFileLoc[] = "/sys/firmware/acpi/platform_profile_choices";
    fptr = fopen(pwpFileLoc, "r");
    if (fptr == NULL) {
        log_error("[read_power_profiles] fopen error message:");
        fclose(fptr);
        exit(EXIT_FAILURE);
    }
    char strProfiles[100];
    fgets(strProfiles, 100, fptr);
    log_info("Available power profiles: %s", strProfiles);
    fclose(fptr);

    char* profile = strtok(strProfiles, " ");
    for (int i = 0; i < 3; i++) {
        power_profiles[i] = malloc(20*sizeof(char));
        if (!strcpy(power_profiles[i], profile)) {
            for (int j = i; j >= 0; j--)
                free(power_profiles[j]);
            log_error("[read_power_profiles] strcpy failed");
            exit(EXIT_FAILURE);
        }
        profile = strtok(NULL, " ");
        
        if (power_profiles[i] == NULL) {
            for (int j = i; j >= 0; j--)
                free(power_profiles[j]);
            log_error("Expected amount of power profiles = 3, but is = %d", (i+1));
            exit(EXIT_FAILURE);
        } else {
            log_info("Found profile: %s", power_profiles[i]);
        }
    }
    
    // last profile has '\n' at then end
    _remove_newline(power_profiles[2]);
    return power_profiles;
}

power_profile_t get_current_pwp() {
    FILE *fptr;
    char cur_pfp_fileLoc[] = "/sys/firmware/acpi/platform_profile";
    fptr = fopen(cur_pfp_fileLoc, "r"); 
    if (!fptr) {
        log_error("[get_current_pwp] fopen error message:");
        exit(EXIT_FAILURE);
    }

    char str_pfp[20]; 
    fgets(str_pfp, 20, fptr);
    fclose(fptr);
    // always reads with an '\n' at the end, so remove
    _remove_newline(str_pfp);
    for (int i=0;i<3;i++) {
        int comp_res = strcmp(str_pfp, str_read_profiles[i]);
        if (!comp_res)
            return i; // enum type has the same idx as str_profiles 
    }
    return UNKNOWN;
}

int is_on_ac() {
    bool out = 0;
    FILE *fptr;
    char file_loc[] = "/sys/class/power_supply/ACAD/online";
    char str_acOnline[2];
    fptr = fopen(file_loc, "r");
    if (!fptr) {
        log_error("[is_on_ac] fopen error message:");
        return -1;
   }
    fgets(str_acOnline, 2, fptr);
    
    int res = atoi(str_acOnline);    
    if (out) {
        if (res)
            log_info("AC is connected to the latptop");
        else
            log_info("Laptop working from Battery power");
    }
    return res;
}


int set_power_profile(power_profile_t profile) {
    // ignore if already same power profile
    if (get_current_pwp() == profile)
        return 0;
    // use powerprofilectl to set 
    log_info("changing power profile to %s", str_write_profiles[profile]);
    pid_t pid = fork();

    if (pid == -1) {
        log_error("[set_power_profile] couldn't fork");
        return -1;
    } else if (pid == 0) {
        // child process
        char *args[] = {"powerprofilesctl", "set", (char*)str_write_profiles[profile], NULL};
        if (execvp(args[0], args) == -1)
            exit(EXIT_FAILURE);
        else 
            exit(EXIT_SUCCESS);
    }

    // parent process
    int child_status;
    waitpid(pid, &child_status, 0);
    if (!WIFEXITED(child_status)) {
        // child failed
        log_error("[set_power_profile] child process powerprofilectl failed");
        return -1;
    }

    return 0;
}

thermal_zones_t get_thermal_zone() {
    int curTemp = read_temp();
    for (int i = 0; i < 3; i++) {
        if (curTemp > thermal_zones[i][0] && curTemp <= thermal_zones[i][1])
            return (thermal_zones_t) i;
    }
}

int set_fan_speed(int fan_num, int RPM) {
    struct dirent *de;

    // hwmon path then has hwmon*, where * can be any number
    DIR *dr = opendir(hwmon_base_path);
    if (!dr) {
        log_error("Could not open hwmon path");
        return -1;
    }

    char ftf[] = "hwmon"; // folder tofind
    char hwmon_found = false;
    while (de = readdir(dr)) {
        log_debug("%s", de->d_name);
        if (!strncmp(de->d_name, ftf, strlen(ftf))) {
            hwmon_found = true;
            break;
        } 
    }

    if (!hwmon_found) {
        log_error("No base hwmon dir found");
        return -1;
    }

    char hwmon_dir[strlen(de->d_name)+1];
    strcpy(hwmon_dir, de->d_name);
    closedir(dr);

    char *hwmon_path;
    size_t hwmon_path_size = strlen(hwmon_base_path)  + strlen(hwmon_dir) + strlen("/fan1_target") + 1; // \0
    hwmon_path = malloc(hwmon_path_size * sizeof(char));
    if (!hwmon_path) {
        log_error("Could not allocate memory for hwmon path");
        return -1;
    }
    // compose hwmoun path by combining base and found directory
    strcpy(hwmon_path, hwmon_base_path);
    strcat(hwmon_path, hwmon_dir);

    log_debug("Full hwmon path: %s", hwmon_path);

    // append fan file
    if (fan_num == 1)
        strcat(hwmon_path, "/fan1_target");
    else if (fan_num == 2)
        strcat(hwmon_path, "/fan2_target");
    else {
        log_error("Invalid fan number %d, only 1 or 2 supported", fan_num);
        free(hwmon_path);
        return -1;
    }

    log_debug("full fan path: %s", hwmon_path);
    FILE *hwmon_fptr = fopen(hwmon_path, "w");
    if (!hwmon_fptr) {
        log_error("Could not open fan target file");
        free(hwmon_path);
        return -1;
    }
    char str_temp[12];
    snprintf(str_temp, sizeof(str_temp), "%d", RPM);
    fputs(str_temp, hwmon_fptr);
    fclose(hwmon_fptr);

    log_info("Set fan %d speed to %d RPM", fan_num, RPM);
    free(hwmon_path);
    return -1;
}


void handle_exit(int sig) {
    log_info("Received signal %d, exiting...", sig);
    set_power_profile(POWER_SAVER);
    exit(EXIT_SUCCESS);
}

int main() {
    struct sigaction sa = {0};
    sa.sa_handler = handle_exit;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    if (test() != 0) {
        exit(EXIT_FAILURE);
    }

    set_fan_speed(1, 0);
    return 0;
    
    while (1) {
        power_profile_t curProfile = get_current_pwp();
        thermal_zones_t curThermalZone = get_thermal_zone();
        // PERORMANCE   <->   BALANCED   <->   LOW_POWER
        //         CPU_BALANCED     CPU_POWERSAVE
        switch (curThermalZone)
        {
            case COOL:
                set_power_profile(PERFORMANCE);
                break;
            case MID:
                set_power_profile(BALANCED);
                break;
            case HOT:
                set_power_profile(POWER_SAVER);
                break;
            
            default:
                exit(EXIT_FAILURE);
                break;
        }
        sleep(CYCLE_SLEEP_TIME);
    }
}
