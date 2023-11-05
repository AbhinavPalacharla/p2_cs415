#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>







//FIX LINE 88










int main(int argc, char **argv) {
    freopen("input.txt", "r", stdin);

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int N = 0;

    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);

    for(; (read = getline(&line, &len, stdin)) != -1; N++) {
    }

    fseek(stdin, 0, SEEK_SET);

    int *pids = malloc(sizeof(int) * N);

    //create child processes
    for(int i = 0; i < N; i++) {
        pids[i] = fork();

        if(pids[i] == 0) {
            printf("(%d) Waiting...\n", getpid());

            if(sigwait(&set, &sig) == 0) {
                getline(&line, &len, stdin);

                printf("(%d) Process === %s\n", getpid(), line);

                exit(0);
            }
        }
        else if(pids[i] < 0) {
            printf("Error creating child process.\n");
            exit(1);
        }
    }

    //send signal to all processes
    for(int i = 0; i < N; i++) {
        sleep(3);
        kill(pids[i], SIGUSR1);
    }

    //suspend child processes
    for(int i = 0; i < N; i++) {
        printf("(%d) Suspending...\n", pids[i]);
        kill(pids[i], SIGSTOP);
    }

    //resume child processes
    for(int i = 0; i < N; i++) {
        printf("(%d) Resuming...\n", pids[i]);
        kill(pids[i], SIGCONT);
    }

    //wait for processes to finish
    for(int i = 0; i < N; i++) {
        /***************************************************/
        /***************************************************/
        /***************************************************/
        /***************************************************/
        wait(NULL); //NEED TO REPLACE WITH WAITPID EVENTUALLY
        /***************************************************/
        /***************************************************/
        /***************************************************/
        /***************************************************/
    }

    free(pids);

    fclose(stdin);
    return 0;
}