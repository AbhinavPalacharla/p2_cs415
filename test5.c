#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define N 3
#define QUANTUM 5

int *pids;
// int iters[N] = {27, 17, 37}; //2.7 sec, 1.7 sec, 3.7 sec QUANTUM 2
int iters[N] = {80, 30, 130}; //QUANTUM 5
int proc_status[N] = {0, 0, 0};
// int iters[3] = {80, 80, 80};
int first = 1;
int waiting = 0;
int proc_finished_count = 0;

void rotate_pids() {
    int temp = pids[0];
    int temp_status = proc_status[0];
    int temp_iters = iters[0];

    for(int i = 0; i < N - 1; i++) {
        pids[i] = pids[i + 1];
        proc_status[i] = proc_status[i + 1];
        iters[i] = iters[i + 1];
    }

    pids[N - 1] = temp;
    proc_status[N - 1] = temp_status;
    iters[N - 1] = temp_iters;
}

void print_processes() {
    printf("\n------- PROCESSES: -------\n");
    for(int i = 0; i < N; i++) {
        printf("(%d) | STAT: %d | IT: %d |\n", pids[i], proc_status[i], iters[i]);
    }

    printf("--------------------------\n");
}

void sigalrm_handler(int sig) {
    if(proc_finished_count == N) {
        printf("All processes finished, skipping alarm.\n");
        return;
    }

    printf("In alarm handler...\n");
    
    //print processes
    print_processes();

    //stop running process
    kill(pids[0], SIGSTOP);
    printf("\n(OS) >>> (ID: %d) (STAT: %d) (IT: %d) Process Stopped.\n", pids[0], proc_status[0], iters[0]);
    
    // // rotate pids
    // rotate_pids();

    printf("Rotating pids...\n");

    //rotate pids until we find a process that hasn't finished
    do {
        rotate_pids();
    } while(proc_status[0] == 1); //This loop will never end if all processes have finished FIX THIS

    //print processes
    print_processes();

    printf("\n(OS) >>> (ID: %d) (STAT: %d) (IT: %d) Process Started.\n", pids[0], proc_status[0], iters[0]);

    //resume next process
    kill(pids[0], SIGCONT);

    alarm(QUANTUM);
}

void sigchld_handler(int sig) {
    int status;
    pid_t child_pid;

    signal(SIGALRM, sigalrm_handler);

    // Check if any child process has terminated
    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            // A child process has exited, mark it as finished
            printf("\n\n(SIGCHLHAND) >>> Process Finished: %d\n\n", child_pid);
            for(int i = 0; i < N; i++) {
                if(pids[i] == child_pid) {
                    proc_status[0] = 1; //Mark process as finished
                    proc_finished_count++;
                    break;
                }
            }

            // Start the next process if there is one
            printf("Exit early so starting next process...\n");

            //reset alarm
            unsigned int alarm_left = alarm(0);
            printf("Alarm left: %d\n", alarm_left);
            // alarm(QUANTUM);
            sigalrm_handler(0);
            printf("Alarm set to 2...\n");
        }
    }
}

void sigusr2_handler() {
    waiting++;
}

int main(int argc, char **argv) {
    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGCONT);

    signal(SIGALRM, sigalrm_handler);
    signal(SIGUSR2, sigusr2_handler);
    signal(SIGCHLD, sigchld_handler);

    pids = (int *) malloc(sizeof(int) * N);

    //create child processes
    for(int i = 0; i < N; i++) {
        pids[i] = fork();

        if(pids[i] == 0) {
            int j = 0;
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 100000000;
            // ts.tv_nsec = 500000000;
            
            printf("(%d) Waiting...\n", getpid());
            // kill(getppid(), SIGUSR2);

            if(sigwait(&set, &sig) == 0) {
                // printf("(%d) Doing Stuff...\n", getpid());
                while((j++) < iters[i]) {
                    printf("(%d) Process Executing... (%d/%d)\n", getpid(), j, iters[i]);

                    //sleep for 1/10th of a second
                    nanosleep(&ts, NULL);
                }
                printf("(%d) Process Finished.\n", getpid());
                kill(getppid(), SIGCHLD);

                exit(0);
            }
        }
        else if(pids[i] < 0) {
            printf("Error creating child process.\n");
            exit(1);
        }
    }

    // while(waiting < N);
    sleep(2);
    printf("\n");

    printf("(OS) >>> (ID: %d) (STAT: %d) (IT: %d) Process Started.\n", pids[0], proc_status[0], iters[0]);
    kill(pids[0], SIGCONT);
    alarm(QUANTUM);

    printf("Before wait...\n");

    //wait for processes to finish
    for(int i = 0; i < N; i++) {
        // waitpid(pids[i], NULL, 0);
        wait(NULL);
    }

    printf("All processes finished.\n");

    free(pids);

    fclose(stdin);
    return 0;
}