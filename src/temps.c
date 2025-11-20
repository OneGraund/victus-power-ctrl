#include "temps.h"
#include "utils.h"
#include "main.h"
#include "power.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int read_cpu_temp() {
    FILE *fptr;
    char tmpFileLoc[] = "/sys/class/thermal/thermal_zone0/temp";
    fptr = fopen(tmpFileLoc, "r");
    if (fptr == NULL) {
        log_error("[read_temp] fopen temp file error message:");
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

int read_gpu_temp() {
    int stdout_pipe[2];
    int stderr_pipe[2];
    const int BSIZE = 256;
    char stdout_buf[BSIZE];
    char stderr_buf[BSIZE];
    ssize_t nbytes_out, nbytes_err;
    int status;

    if (pipe(stdout_pipe) == -1) { // stdout_pipe[0] for reading, [1] for writing
        log_error("[read_gpu_temp] stdout pipe failed");
        return -1;
    }
    if (pipe(stderr_pipe) == -1) { // stderr_pipe[0] for reading, [1] for writing
        log_error("[read_gpu_temp] stderr pipe failed");
        close(stdout_pipe[0]); // on failure, close pipes
        close(stdout_pipe[1]);
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        log_error("[read_gpu_temp] couldn't fork");
        close(stdout_pipe[0]); // on failure close pipes
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);
        return -1; 
    } else if (pid == 0) {
        // both stdout_pipe[1] and STDOUT_FILENO point to stdout kernel object
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);

        // close unnesessary read ends 
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        // stdout points to stdout_pipe write end and stdout_pipe write end points to stdout_pipe[1]
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);
        // close them so that only stdout points to stdout_pipe write end

        char *args[] = {"nvidia-smi", "--query-gpu=temperature.gpu", "--format=csv,noheader", NULL};
        execvp(args[0], args);
        // if execvp returns, it failed
        perror("execvp nvidia-smi");
        exit(127); // command not found convention
    }

    // parent process
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    // read stdout
    nbytes_out = read(stdout_pipe[0], stdout_buf, BSIZE - 1);
    if (nbytes_out > 0) {
        stdout_buf[nbytes_out] = '\0';
    } else {
        stdout_buf[0] = '\0';
    }

    // read stderr
    nbytes_err = read(stderr_pipe[0], stderr_buf, BSIZE - 1);
    if (nbytes_err > 0) {
        stderr_buf[nbytes_err] = '\0';
    } else {
        stderr_buf[0] = '\0';
    }

    int child_status;
    waitpid(pid, &child_status, 0);
    
    // close read ends after child finishes to avoid SIGPIPE
    close(stdout_pipe[0]);
    close(stderr_pipe[0]);

    // evaluates to a non-zero value if status was returned for a child process that terminated normally
    if (WIFEXITED(child_status)) { 
        int exit_code = WEXITSTATUS(child_status); // lower 8 bits of status returned by child _exit()
        if (exit_code != 0) {
            log_error("[read_gpu_temp] nvidia-smi exited with code %d", exit_code);
            if (exit_code == 127) {
                log_error("[read_gpu_temp] nvidia-smi not found in PATH");
            }
            if (nbytes_err > 0) {
                log_error("[read_gpu_temp] stderr: %s", stderr_buf);
            }
            return -1;
        } else {
            log_debug("[read_gpu_temp] nvidia-smi exited successfully");
        }
    } else if (WIFSIGNALED(child_status)) { // terminated due to the receipt of a signal that was not caught
        log_error("[read_gpu_temp] nvidia-smi killed by signal %d", WTERMSIG(child_status)); //sig that caused
        return -1;
    } else {
        // WIFEXITED is zero if child process terminated abnormally
        log_error("[read_gpu_temp] nvidia-smi terminated abnormally");
        return -1;
    }

    if (nbytes_out <= 0) {
        log_error("[read_gpu_temp] nvidia-smi produced no output");
        if (nbytes_err > 0) {
            log_error("[read_gpu_temp] stderr: %s", stderr_buf);
        }
        return -1;
    }

    log_debug("[read_gpu_temp] nvidia-smi output: %s", stdout_buf);
    
    int ret = atoi(stdout_buf); // well in theory we can have 0 temperature
    if (ret == 0 && stdout_buf[0] != '0') {
        log_error("[read_gpu_temp] failed to parse temperature from: %s", stdout_buf);
        return -1;
    }
    
    log_debug("[read_gpu_temp] GPU temp: %d C", ret);
    return ret;
}

thermal_zones_t get_thermal_zone() {
    int curTemp = read_cpu_temp();
    if (curTemp == -1) {
        log_error("[get_thermal_zone] cannot read CPU temp");
        return TZ_UNKNOWN;
    }
    for (int i = 0; i < 3; i++) {
        if (curTemp > thermal_zones[i][0] && curTemp <= thermal_zones[i][1])
            return (thermal_zones_t) i;
    }
    log_error("[get_thermal_zone] temperature %d out of range", curTemp);
    return TZ_UNKNOWN;
}