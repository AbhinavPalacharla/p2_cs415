#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>

#define N 3
#define QUANTUM 5

int iters[N] = {8, 3, 13};
int waiting = 0;

typedef struct Command {
    char *command;
    char **args;
} Command;

typedef struct Process {
    int pid;
    int status;
    Command *command;
} Process;

Process *processes = NULL;

int current_process = 0;

void next_process() {
    current_process = (current_process + 1) % N;
}

void print_processes() {
    printf("\n------- PROCESSES: -------\n");
    for(int i = 0; i < N; i++) {
        printf("(%d) | STAT: %d | IT: %d |\n", processes[i].pid, processes[i].status, iters[i]);
    }

    printf("--------------------------\n");
}


void sigusr2_handler() {
    waiting++;
}

void sigalrm_handler() {
    
    //stop current process
    kill(processes[current_process].pid, SIGSTOP);
    printf("\n(OS) >>> (ID: %d) (STAT: %d) (IT: %d) Process Paused.\n\n", processes[current_process].pid , processes[current_process].status, iters[current_process - N]);

    /*************************************************/

    //check if current process is finished
    int status;
    pid_t child_pid = waitpid(processes[current_process].pid, &status, WNOHANG);

    if(child_pid != 0) {
        if(WIFEXITED(status)) {
            printf("(OS) >>> (ID: %d) (STAT: %d) (IT: %d) Process Finished.", processes[current_process].pid , processes[current_process].status, iters[current_process - N]);
            
            processes[current_process].status = 1;
        }
    }

    /*************************************************/

    //move to next process
    do {
        next_process();
    } while(processes[current_process].status == 1);

    //start next process
    printf("\n\n(OS) >>> (ID: %d) (STAT: %d) (IT: %d) Process Resumed.\n\n", processes[current_process].pid , processes[current_process].status, iters[current_process - N]);
    kill(processes[current_process].pid, SIGCONT);

    alarm(QUANTUM);
}

int main() {
    signal(SIGUSR2, sigusr2_handler);
    signal(SIGALRM, sigalrm_handler);

    /*************************************************/

    sigset_t set; int sig;

    sigemptyset(&set);
    sigaddset(&set, SIGCONT);

    sigprocmask(SIG_BLOCK, &set, NULL);

    /*************************************************/

    processes = malloc(sizeof(Process) * N);

    /************************************************/

    for(int i = 0; i < N; i++) {
        processes[i].pid = fork();

        if(processes[i].pid == 0) {
            printf("(PROC: %d) >>  Waiting...\n", getpid());

            kill(getppid(), SIGUSR2);

            if(sigwait(&set, &sig) == 0) {
                switch(i) {
                    case 0:
                        execvp("./proc", (char *[]){"./proc", "8", NULL});
                        break;
                    case 1:
                        execvp("./proc", (char *[]){"./proc", "3", NULL});
                        break;
                    case 2:
                        execvp("./proc", (char *[]){"./proc", "13", NULL});
                        break;
                }
            }
        } else if(processes[i].pid < 0) {
            printf("Error creating process %d\n", i);
            exit(1);
        }
    }
    /************************************************/

    //setup all processes status
    for(int i = 0; i < N; i++) {
        processes[i].status = 0;
    }

    /************************************************/

    while(waiting < N);

    printf("\n(OS) >>> (ID: %d) (STAT: %d) (IT: %d) Process Started.\n\n", processes[current_process].pid , processes[current_process].status, iters[current_process - N]);

    kill(processes[current_process].pid, SIGCONT);

    alarm(QUANTUM);

    /************************************************/

    for(int i = 0; i < N; i++) {
        waitpid(processes[i].pid, NULL, 0);
    }
    
    free(processes);

    return 0;
}