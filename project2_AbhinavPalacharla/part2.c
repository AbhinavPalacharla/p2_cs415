#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "MCP.h"


int main(int argc, char **argv) {
    char *input_file = argv[1];

    FILE *f = fopen(input_file, "r");

    size_t len = 128; char *line = malloc(sizeof(char) * len); ssize_t read;
    
    int N = 0;

    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);

    for(; (read = getline(&line, &len, f)) != -1; N++);

    fseek(f, 0, SEEK_SET);

    int *pids = malloc(sizeof(int) * N);

    //create child processes
    for(int i = 0; i < N; i++) {
        getline(&line, &len, f);
        command_line cl = str_filler(line, " ");


        pids[i] = fork();

        if(pids[i] == 0) {

            if(sigwait(&set, &sig) == 0) {

                if(execvp(cl.command_list[0], cl.command_list) != 0) {
                    printf("Error executing command: %s\n", cl.command_list[0]);
                    exit(1);
                }

                exit(0);
            }
        }
        else if(pids[i] < 0) {
            printf("Error creating child process.\n");
            exit(1);
        }
    }

    free(line);

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
        waitpid(pids[i], NULL, 0);
    }

    free(pids);

    fclose(f);
    return 0;
}