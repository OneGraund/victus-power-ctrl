#ifndef MAIN_HEADER_H
#define MAIN_HEADER_H

#define CPU_BALANCED_TEMP_THRESHHOLD 77
#define CPU_POWERSAVE_TEMP_THRESHHOLD 80
#define CYCLE_SLEEP_TIME 1 // only integer, since temp updates min 1 sec
#define CYCLE_CHECK_BAT_TIME 5

// signal handler to catch SIGINT and SIGTERM
//  on signal, set power profile to POWER_SAVER and exit
void handle_exit(int sig);

void exit_failure(char* reason);


#endif // MAIN_HEADER_H