#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "MCP.h"

int main(int argc, char **argv) {
    char *input_file = argv[1];

    FILE *f = fopen(input_file, "r");

    size_t len = 128; char *line = malloc(sizeof(char) * len); ssize_t read;
    
    int numLines = 0;

    for(; (read = getline(&line, &len, f)) != -1; numLines++);

    fseek(f, 0, SEEK_SET);

    int *pids = malloc(sizeof(int) * numLines);

    //create child processes
    for(int i = 0; i < numLines; i++) {
        getline(&line, &len, f);    
        command_line cl = str_filler(line, " ");

        pids[i] = fork();

        if(pids[i] == 0) {
            
            if(execvp(cl.command_list[0], cl.command_list) != 0) {
                printf("Error executing command: %s\n", cl.command_list[0]);
                exit(1);
            }

            exit(0);
        }
        else if(pids[i] < 0) {
            printf("Error creating child process.\n");
            exit(1);
        }

        free_command_line(&cl);
    }

    free(line);

    //wait for all processes to finish
    for(int i = 0; i < numLines; i++) {
        wait(NULL);
    }

    free(pids);

    fclose(f);
    return 0;
}