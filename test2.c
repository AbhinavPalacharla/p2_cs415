#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int *pids;
int N = 3;
int first = 1;
int waiting = 0;

void rotate_pids() {
    int temp = pids[0];

    for(int i = 0; i < N - 1; i++) {
        pids[i] = pids[i + 1];
    }

    pids[N - 1] = temp;
}

void print_pids() {
    printf("\n/// PIDS: ");
    for(int i = 0; i < N; i++) {
        printf("%d ", pids[i]);
    }

    printf("///\n");
}

void sigalrm_handler(int sig) {
    //stop running process
    kill(pids[0], SIGSTOP);
    printf("\n(OS) >>> (%d) Process Stopped.\n", pids[0]);
    
    //rotate pids
    rotate_pids();

    printf("\n(OS) >>> (%d) Process Started.\n\n", pids[0]);
    //resume next process
    kill(pids[0], SIGCONT);

    alarm(2);
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

    pids = (int *) malloc(sizeof(int) * N);

    //create child processes
    for(int i = 0; i < N; i++) {
        pids[i] = fork();

        if(pids[i] == 0) {
            printf("(%d) Waiting...\n", getpid());
            // kill(getppid(), SIGUSR2);

            if(sigwait(&set, &sig) == 0) {
                while(1) {
                    printf("(%d) Process Executing...\n", getpid());
                    // sleep(1);
                }

                exit(0);
            }
        }
        else if(pids[i] < 0) {
            printf("Error creating child process.\n");
            exit(1);
        }
    }

    // while(waiting < N);
    // sleep(2);

    kill(pids[0], SIGCONT);
    alarm(2);

    //wait for processes to finish
    for(int i = 0; i < N; i++) {
        wait(NULL);
    }

    free(pids);

    fclose(stdin);
    return 0;
}