#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int *pids;
int N = 0;

void rotate_pids() {
    int temp = pids[0];

    for(int i = 0; i < N - 1; i++) {
        pids[i] = pids[i + 1];
    }

    pids[N - 1] = temp;
}

void sigalrm_handler(int sig) {
    printf("\nSIGALRM received.\n");

    //stop running process
    kill(pids[0], SIGSTOP);
    printf("\n(OS) >>> (%d) Process Stopped.\n", pids[0]);

    //rotate pids
    rotate_pids();

    printf("\n(OS) >>> (%d) Process Resumed.\n\n", pids[0]);
    //resume next process
    kill(pids[0], SIGCONT);

    alarm(1);
}

int main(int argc, char **argv) {
    freopen("input.txt", "r", stdin);

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGCONT);

    signal(SIGALRM, sigalrm_handler);

    for(; (read = getline(&line, &len, stdin)) != -1; N++);

    fseek(stdin, 0, SEEK_SET);

    pids = (int *) malloc(sizeof(int) * N);

    alarm(1);

    //create child processes
    for(int i = 0; i < N; i++) {
        pids[i] = fork();

        if(pids[i] == 0) {
            printf("(%d) Waiting...\n", getpid());

            if(sigwait(&set, &sig) == 0) {
                getline(&line, &len, stdin);

                // printf("(%d) Process === %s\n", getpid(), line);

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

    for(int i = 0; i < N - 1; i++) {
        rotate_pids();
    }

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

    //wait for processes to finish
    for(int i = 0; i < N; i++) {
        wait(NULL);
    }

    free(pids);

    fclose(stdin);
    return 0;
}