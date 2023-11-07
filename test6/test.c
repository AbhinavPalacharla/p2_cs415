#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int pid = fork();
    int sec = 3;

    execvp("./proc", (char *[]){"./proc", sec, NULL});

    // waitpid(pid, NULL, 0);

    while(1);

    return 0;
}