#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "main.h"

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
        perror("fopen temp file error message:");
        fclose(fptr);
        return -1;
    }
    char strTemp[7];
    fgets(strTemp, 7, fptr);
    fclose(fptr);
    int intTemp = atoi(strTemp) / 1000;
    if (!intTemp) {
        fprintf(stderr, "%s", "Error reading from CPU temperature file");
        return -1;
    }
    // printf("CPU temp: %d C\n", intTemp);
    return intTemp;
}

char** read_power_profiles() {
    char** power_profiles = malloc(3 * sizeof(char*));
    FILE *fptr;
    char pwpFileLoc[] = "/sys/firmware/acpi/platform_profile_choices";
    fptr = fopen(pwpFileLoc, "r");
    if (fptr == NULL) {
        perror("Power profile fopen error message:");
        fclose(fptr);
        exit(EXIT_FAILURE);
    }
    char strProfiles[100];
    fgets(strProfiles, 100, fptr);
    printf("Available power profiles: %s\n", strProfiles);
    fclose(fptr);

    char* profile = strtok(strProfiles, " ");
    for (int i = 0; i < 3; i++) {
        power_profiles[i] = malloc(20*sizeof(char));
        if (!strcpy(power_profiles[i], profile)) {
            for (int j = i; j >= 0; j--)
                free(power_profiles[j]);
            fprintf(stderr, "strcpy failed");
            exit(EXIT_FAILURE);
        }
        profile = strtok(NULL, " ");
        
        if (power_profiles[i] == NULL) {
            for (int j = i; j >= 0; j--)
                free(power_profiles[j]);
            fprintf(stderr, "Expected amount of power profiles = 3, but is = %d\n", (i+1));
            exit(EXIT_FAILURE);
        } else {
            printf("Found profile: %s\n", power_profiles[i]);
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
        perror("current power profile");
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
    FILE *fptr;
    char file_loc[] = "/sys/class/power_supply/ACAD/online";
    char str_acOnline[2];
    fptr = fopen(file_loc, "r");
    if (!fptr) {
        perror("reading ac file");
        return -1;
   }
    fgets(str_acOnline, 2, fptr);
    
    int res = atoi(str_acOnline);    
    if (res)
        printf("AC is connected to the latptop\n");
    else
        printf("Laptop working from Battery power\n");
    return res;
}


int set_power_profile(power_profile_t profile) {
    // ignore if already same power profile
    if (get_current_pwp() == profile)
        return 0;
    // use powerprofilectl to set 
    printf("changing power profile to %s\n", str_write_profiles[profile]);
    pid_t pid = fork();

    if (pid == -1) {
        perror("couldn't fork");
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
        perror("child process powerprofilectl failed");
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


int main() {
    if (test() != 0)
        exit(EXIT_FAILURE);
    
    power_profile_t currentProfile;
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
