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

void sigalrm_handler() {
    
    //stop current process
    kill(processes[current_process].pid, SIGSTOP);
    printf("\n(OS) >>> (ID: %d) Process Paused.\n\n", processes[current_process].pid);

    /*************************************************/

    //check if current process is finished
    int status;
    pid_t child_pid = waitpid(processes[current_process].pid, &status, WNOHANG);

    if(child_pid != 0) {
        printf("Child PID != 0 | (%d)\n", child_pid);
        printf("Status: %d\n", status);
        if(WIFEXITED(status)) {
            printf("(OS) >>> (ID: %d) Process Finished.", processes[current_process].pid);
            
            processes[current_process].status = 1;
            numRunning--;
            printf("Num Running: %d\n", numRunning);
            if(numRunning == 0) {
                return;
            }
        }
    }

    /*************************************************/

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

    /*************************************************/

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
            kill(getppid(), SIGUSR2);

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

    //Kick off first process
    printf("\n(OS) >>> (ID: %d) Process Started.\n\n", processes[current_process].pid);

    kill(processes[current_process].pid, SIGCONT);

    alarm(QUANTUM);

    /************************************************/

    while(numRunning > 0);

    printf("\n\n(OS) >>> ALL PROCESSES FINISHED.\n\n");

    free(processes);

    return 0;
}