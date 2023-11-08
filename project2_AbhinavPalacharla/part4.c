#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "MCP.h"

#define QUANTUM 2

typedef struct Process {
    int pid;
    int status;
} Process;

int N = 0;
int waiting = 0;
int numRunning = 0;
Process *processes = NULL;
int current_process = 0;

void next_process() {
    current_process = (current_process + 1) % N;
}

void processes_status_overview() {
    printf("\n------- PROCESSES: ---------------------------------------------------------------------------------------------------------------------\n\n");
    for(int i = 0; i < N; i++) {
        if(processes[i].status == 1) {continue;}

        char *fpath;
        asprintf(&fpath, "/proc/%d/status", processes[i].pid);

        FILE *f; 
        
        if((f = fopen(fpath, "r")) == NULL) {
            printf("Error opening file: %s\n", fpath);
            exit(1);
        }

        char *line = NULL;
        size_t len = 128;
        ssize_t read;

        char *name;
        char *state = NULL;
        int threads = 0;
        int voluntary_ctxt_switches = 0;
        int nonvoluntary_ctxt_switches = 0;

        while((read = getline(&line, &len, f)) != -1) {
            if(strncmp(line, "Name:", 5) == 0) {
                name = malloc(sizeof(char) * (strlen(line) + 1));
                strcpy(name, line);
                removeSubstring(name, "Name:\t");
                name[strlen(name) - 1] = '\0';
            } else if(strncmp(line, "State:", 6) == 0) {
                state = malloc(sizeof(char) * (strlen(line) + 1));
                strcpy(state, line);
                removeSubstring(state, "State:\t");
                state[strlen(state) - 1] = '\0';
            } else if(strncmp(line, "Threads:", 8) == 0) {
                threads = atoi(line + 9);
            } else if(strncmp(line, "voluntary_ctxt_switches:", 24) == 0) {
                voluntary_ctxt_switches = atoi(line + 25);
            } else if(strncmp(line, "nonvoluntary_ctxt_switches:", 27) == 0) {
                nonvoluntary_ctxt_switches = atoi(line + 28);
            }
        }

        printf("PROC (ID: %d) (STAT: %d) (CMD: %s) | STATE: %s ", processes[i].pid, processes[i].status, name, state);
        printf("| THREADS: %d ", threads);
        printf("| VOLUNTARY CTX SWITCHES: %d ", voluntary_ctxt_switches);
        printf("| NON VOLUNTARY CTX SWITCHES: %d\n\n", nonvoluntary_ctxt_switches);

        free(name);
        free(state);
        free(line);

        fclose(f);

        free(fpath);
    }

    printf("---------------------------------------------------------------------------------------------------------------------------------------\n\n");
}

void sigalrm_handler() {

    /*************************************************/

    //process overview
    processes_status_overview();

    /*************************************************/
    
    //stop current process
    kill(processes[current_process].pid, SIGSTOP);
    printf("\n(OS) >>> (ID: %d) Process Paused.\n\n", processes[current_process].pid);

    /*************************************************/

    //check if current process is finished
    int status;
    pid_t child_pid = waitpid(processes[current_process].pid, &status, WNOHANG);

    if(child_pid != 0) {
        if(WIFEXITED(status)) {
            printf("(OS) >>> (ID: %d) Process Finished.\n", processes[current_process].pid);
            printf("(OS) >>> %d Processes Remaining.\n\n", numRunning - 1);
            
            processes[current_process].status = 1;
            numRunning--;

            if(numRunning == 0) {
                return;
            }
        }
    }

    //move to next process
    do {
        next_process();
    } while(processes[current_process].status == 1);

    //start next process
    printf("\n\n(OS) >>> (ID: %d) Process Resumed.\n\n", processes[current_process].pid);

    kill(processes[current_process].pid, SIGCONT);

    if (numRunning > 0) {
        alarm(QUANTUM);
    }
}

int main(int argc, char **argv) {
    char *input_file = argv[1];

    FILE *f = fopen(input_file, "r");

    size_t len = 128; char *line = malloc(sizeof(char) * len);  ssize_t read;
    
    for(; (read = getline(&line, &len, f)) != -1; N++);

    free(line);

    fclose(f);

    processes = malloc(sizeof(Process) * N);

    f = fopen(input_file, "r");

    /************************************************/
    signal(SIGALRM, sigalrm_handler);

    sigset_t set; int sig;

    sigemptyset(&set);
    sigaddset(&set, SIGCONT);

    sigprocmask(SIG_BLOCK, &set, NULL);

    /*************************************************/

    for(int i = 0; i < N; i++) {
        size_t len = 128; char *line = malloc(sizeof(char) * len); 

        getline(&line, &len, f);
        command_line cl = str_filler(line, " ");

        processes[i].pid = fork();

        if(processes[i].pid == 0) {
            // kill(getppid(), SIGUSR2);

            if(sigwait(&set, &sig) == 0) {
                if(execvp(cl.command_list[0], cl.command_list) != 0) {
                    printf("Error executing command: %s\n", cl.command_list[0]);
                    exit(1);
                }

            }
        } else if(processes[i].pid < 0) {
            printf("Error creating process %d\n", i);
            exit(1);
        }

        free_command_line(&cl);
        free(line);
    }

    fclose(f);

    /************************************************/

    //setup all processes status
    for(int i = 0; i < N; i++) {
        processes[i].status = 0;
    }

    numRunning = N;

    /************************************************/

    printf("\n(OS) >>> (ID: %d) Process Started.\n\n", processes[current_process].pid);

    kill(processes[current_process].pid, SIGCONT);

    alarm(QUANTUM);

    /************************************************/

    while(numRunning > 0);

    printf("\n\n(OS) >>> ALL PROCESSES FINISHED.\n\n");

    free(processes);

    return 0;
}