#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int *pids;
int N = 3;
int first = 1;

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
    if(!first) {
        kill(pids[0], SIGSTOP);
        printf("\n(OS) >>> (%d) Process Stopped.\n", pids[0]);
    }
    else {
        first = 0;
    }
    

    // print_pids();
    // //rotate pids
    rotate_pids();
    // printf("\nPIDs Rotated.\n");
    // print_pids();

    printf("\n(OS) >>> (%d) Process Started.\n\n", pids[0]);
    //resume next process
    kill(pids[0], SIGCONT);

    alarm(2);
}

int main(int argc, char **argv) {
    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGCONT);

    signal(SIGALRM, sigalrm_handler);

    pids = (int *) malloc(sizeof(int) * N);

    //create child processes
    for(int i = 0; i < N; i++) {
        pids[i] = fork();

        if(pids[i] == 0) {
            printf("(%d) Waiting...\n", getpid());

            if(sigwait(&set, &sig) == 0) {
                while(1) {
                    printf("(%d) Process Executing...\n", getpid());
                    sleep(1);
                }

                exit(0);
            }
        }
        else if(pids[i] < 0) {
            printf("Error creating child process.\n");
            exit(1);
        }
    }

    // for(int i = 0; i < N - 1; i++) {
    //     rotate_pids();
    // }

    //send signal to all processes
    // for(int i = 0; i < N; i++) {
    //     sleep(3);
    //     kill(pids[i], SIGUSR1);
    // }

    // //suspend child processes
    // for(int i = 0; i < N; i++) {
    //     printf("(%d) Suspending...\n", pids[i]);
    //     kill(pids[i], SIGSTOP);
    // }

    // //resume child processes
    // for(int i = 0; i < N; i++) {
    //     printf("(%d) Resuming...\n", pids[i]);
    //     kill(pids[i], SIGCONT);
    // }

    alarm(2);

    //wait for processes to finish
    for(int i = 0; i < N; i++) {
        wait(NULL);
    }

    free(pids);

    fclose(stdin);
    return 0;
}