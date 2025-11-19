#include "power.h"
#include "utils.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


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
    return PWP_UNKNOWN;
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