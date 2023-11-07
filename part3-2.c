#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

typedef struct {
    char** command_list;
    int num_token;
} command_line;

int count_token(char* buf, const char* delim)
{
	if (buf == NULL) {
		return 0;
	}

	int numTokens = 0;

	char *bufCopy = (char *) malloc(sizeof(char) * (strlen(buf) + 1));

	strcpy(bufCopy, buf);

	if (bufCopy[strlen(buf) - 1] == '\n') {
		bufCopy[strlen(buf) - 1] = '\0';
	}

	char *token = strtok(bufCopy, delim);

	while(token != NULL) {
		token = strtok(NULL, delim);
		numTokens++;
	}

	free(bufCopy);

	return numTokens;
}

command_line str_filler (char* buf, const char* delim)
{
	command_line command;

	command.num_token = count_token(buf, delim);
	command.command_list = (char **) malloc(sizeof(char *) * (command.num_token + 1));

	char *bufCopy = (char *) malloc(sizeof(char) * (strlen(buf) + 1));

	strcpy(bufCopy, buf);

	if (bufCopy[strlen(buf) - 1] == '\n') {
		bufCopy[strlen(buf) - 1] = '\0';
	}

	char *token = strtok(bufCopy, delim);

	int i = 0;

	while(token != NULL) {
		command.command_list[i] = (char *) malloc(sizeof(char) * (strlen(token) + 1));
		strcpy(command.command_list[i], token);
		token = strtok(NULL, delim);
		i++;
	}

	command.command_list[i] = NULL;

	free(bufCopy);

	return command;
}

void free_command_line(command_line* command)
{
	for(int i = 0; i < command->num_token; i++) {
		free(command->command_list[i]);
	}

	free(command->command_list);
}

// #define N 3
#define QUANTUM 2

// int iters[N] = {8, 3, 13};
int N = 0;
int waiting = 0;
int numRunning = 0;

typedef struct Process {
    int pid;
    int status;
    char **command;
} Process;

Process *processes = NULL;

int current_process = 0;

void next_process() {
    current_process = (current_process + 1) % N;
}

void print_processes() {
    printf("\n------- PROCESSES: -------\n");
    for(int i = 0; i < N; i++) {
        printf("(%d) | STAT: %d\n", processes[i].pid, processes[i].status);
    }

    printf("--------------------------\n");
}


void sigusr2_handler() {
    waiting++;
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
        }
    }

    /*************************************************/

    //move to next process
    do {
        next_process();
    } while(processes[current_process].status == 1);

    //start next process
    // printf("\n\n(OS) >>> (ID: %d) Process Resumed.\n\n", processes[current_process].pid);
    printf("\n(OS) >>> (ID: %d) (CMD: %s) Process Started.\n\n", processes[current_process].pid, processes[current_process].command[0]);

    kill(processes[current_process].pid, SIGCONT);

    alarm(QUANTUM);
}

int main() {
    FILE *f = fopen("input.txt", "r");

    char *line = NULL; size_t len = 0; ssize_t read;
    
    for(; (read = getline(&line, &len, f)) != -1; N++);

    fclose(f);

    processes = malloc(sizeof(Process) * N);

    for(int i = 0; i < N; i++) {
        processes[i].command = malloc(sizeof(command_line));
    }

    f = fopen("input.txt", "r");

    for(int i = 0; (getline(&line, &len, f) != -1); i++) {
        command_line *cl = malloc(sizeof(command_line));
        *cl = str_filler(line, " "); // fill command list
        // memcpy(processes[i].command, cl, sizeof(command_line));

        processes[i].command = malloc(sizeof(char*) * (cl->num_token + 1));
        
        for(int j = 0; j < cl->num_token; j++) {
            processes[i].command[j] = malloc(sizeof(char) * (strlen(cl->command_list[j]) + 1));
            strcpy(processes[i].command[j], cl->command_list[j]);
        }

        processes[i].command[cl->num_token] = NULL;

        free(cl);
    }


    /************************************************/

    signal(SIGUSR2, sigusr2_handler);
    signal(SIGALRM, sigalrm_handler);

    /*************************************************/

    sigset_t set; int sig;

    sigemptyset(&set);
    sigaddset(&set, SIGCONT);

    sigprocmask(SIG_BLOCK, &set, NULL);

    /*************************************************/

    for(int i = 0; i < N; i++) {
        processes[i].pid = fork();

        if(processes[i].pid == 0) {
            // printf("(PROC: %d) >>  Waiting...\n", getpid());

            kill(getppid(), SIGUSR2);

            if(sigwait(&set, &sig) == 0) {
                // if(execvp(processes[i].command->command_list[0], processes[i].command->command_list) != 0) {
                //     printf("Error executing command: %s\n", processes[i].command->command_list[0]);
                //     exit(1);
                // }

                if(execvp(processes[i].command[0], processes[i].command) != 0) {
                    printf("Error executing command: %s\n", processes[i].command[0]);
                    exit(1);
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

    // while(waiting < N);

    printf("\n(OS) >>> (ID: %d) (CMD: %s) Process Started.\n\n", processes[current_process].pid, processes[current_process].command[0]);

    kill(processes[current_process].pid, SIGCONT);

    alarm(QUANTUM);

    /************************************************/

    for(int i = 0; i < N; i++) {
        // waitpid(processes[i].pid, NULL, 0);
        wait(NULL);
    }

    // while(numRunning > 0);

    printf("\n\n(OS) >>> ALL PROCESSES FINISHED.\n\n");

    for(int i = 0; i < N; i++) {
        for(int j = 0; processes[i].command[j] != NULL; j++) {
            free(processes[i].command[j]);
        }

        free(processes[i].command);
    }

    free(processes);

    return 0;
}