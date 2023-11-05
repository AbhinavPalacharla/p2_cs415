#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    freopen("input.txt", "r", stdin);

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int numLines = 0;

    for(; (read = getline(&line, &len, stdin)) != -1; numLines++) {
    }

    fseek(stdin, 0, SEEK_SET);

    int *pids = malloc(sizeof(int) * numLines);

    //create child processes
    for(int i = 0; i < numLines; i++) {
        pids[i] = fork();

        if(pids[i] == 0) {
            getline(&line, &len, stdin);
            printf("(%d) Process === %s\n", getpid(), line);

            exit(0);
        }
        else if(pids[i] < 0) {
            printf("Error creating child process.\n");
            exit(1);
        }
    }

    //wait for all processes to finish
    for(int i = 0; i < numLines; i++) {
        // waitpid(pids[i], NULL, 0);
        wait(NULL);
    }

    free(pids);

    fclose(stdin);
    return 0;
}