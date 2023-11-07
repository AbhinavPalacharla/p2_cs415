#define _GNU_SOURCE
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

#define QUANTUM 20

int N = 0;
int waiting = 0;
int numRunning = 0;

typedef struct Process {
    int pid;
    int status;
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

void processes_status_overview() {
    printf("\n------- PROCESSES: -------\n\n");
    for(int i = 0; i < N; i++) {
        char *fpath;
        asprintf(&fpath, "/proc/%d/status", processes[i].pid);

        printf("fpath: %s\n", fpath);

        FILE *f = fopen(fpath, "r");

        printf("Here");

        char *line = NULL;
        size_t len = 128;
        ssize_t read;

        char *state = NULL;
        int threads = 0;
        int voluntary_ctxt_switches = 0;
        int nonvoluntary_ctxt_switches = 0;

        while((read = getline(&line, &len, f)) != -1) {
            if(strncmp(line, "State:", 6) == 0) {
                state = malloc(sizeof(char) * (strlen(line) + 1));
                strcpy(state, line);
            } else if(strncmp(line, "Threads:", 8) == 0) {
                threads = atoi(line + 9);
            } else if(strncmp(line, "voluntary_ctxt_switches:", 24) == 0) {
                voluntary_ctxt_switches = atoi(line + 25);
            } else if(strncmp(line, "nonvoluntary_ctxt_switches:", 27) == 0) {
                nonvoluntary_ctxt_switches = atoi(line + 28);
            }
        }

        printf("PROC (ID: %d) (STAT: ) | STATE: %s ", processes[i].pid, state);
        printf("| THREADS: %d ", threads);
        printf("| VOLUNTARY CTX SWITCHES: %d ", voluntary_ctxt_switches);
        printf("| NON VOLUNTARY CTX SWITCHES: %d\n\n", nonvoluntary_ctxt_switches);

        free(state);
        free(line);

        fclose(f);

        free(fpath);
        // printf("PROC (ID: %d) (STAT: ) | STATE: I (idle) ", processes[i].pid, processes[i].status);
        // printf("| THREADS: 0 ");
        // printf("| VOLUNTARY CTX SWITCHES: 0 ");
        // printf("| NON VOLUNTARY CTX SWITCHES: 0\n\n");
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
        // printf("Child PID != 0 | (%d)\n", child_pid);
        // printf("Status: %d\n", status);
        if(WIFEXITED(status)) {
            printf("(OS) >>> (ID: %d) Process Finished.\n", processes[current_process].pid);
            printf("(OS) >>> %d Processes Remaining.\n\n", numRunning);
            
            processes[current_process].status = 1;
            numRunning--;
            // printf("Num Running: %d\n", numRunning);
            if(numRunning == 0) {
                return;
            }
        }
    }

    /*************************************************/

    //process overview
    processes_status_overview();

    /*************************************************/

    //move to next process
    do {
        next_process();
    } while(processes[current_process].status == 1);

    //start next process
    printf("\n\n(OS) >>> (ID: %d) Process Resumed.\n\n", processes[current_process].pid);
    // printf("\n(OS) >>> (ID: %d) (CMD: %s) Process Started.\n\n", processes[current_process].pid, processes[current_process].command[0]);

    kill(processes[current_process].pid, SIGCONT);

    if (numRunning > 0) {
        alarm(QUANTUM);
    }
}

int main() {
    FILE *f = fopen("input.txt", "r");

    size_t len = 128; char *line = malloc(sizeof(char) * len);  ssize_t read;
    
    for(; (read = getline(&line, &len, f)) != -1; N++);

    free(line);

    fclose(f);

    processes = malloc(sizeof(Process) * N);

    f = fopen("input.txt", "r");


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
        size_t len = 128; char *line = malloc(sizeof(char) * len); 

        getline(&line, &len, f);
        command_line cl = str_filler(line, " ");

        processes[i].pid = fork();

        if(processes[i].pid == 0) {
            kill(getppid(), SIGUSR2);

            if(sigwait(&set, &sig) == 0) {
                if(execvp(cl.command_list[0], cl.command_list) != 0) {
                    printf("Error executing command: %s\n", cl.command_list[0]);
                    exit(1);
                }

            }
        } else if(processes[i].pid < 0) {
            printf("Error creating process %d\n", i);
            exit(1);
        }

        free_command_line(&cl);
        free(line);
    }

    fclose(f);

    /************************************************/

    //setup all processes status
    for(int i = 0; i < N; i++) {
        processes[i].status = 0;
    }

    numRunning = N;

    /************************************************/

    // while(waiting < N);

    // printf("\n(OS) >>> (ID: %d) (CMD: %s) Process Started.\n\n", processes[current_process].pid, processes[current_process].command[0]);
    printf("\n(OS) >>> (ID: %d) Process Started.\n\n", processes[current_process].pid);

    kill(processes[current_process].pid, SIGCONT);

    alarm(QUANTUM);

    /************************************************/

    while(numRunning > 0);

    printf("\n\n(OS) >>> ALL PROCESSES FINISHED.\n\n");

    free(processes);

    return 0;
}