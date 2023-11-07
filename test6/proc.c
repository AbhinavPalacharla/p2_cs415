#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv) {
    int max_iters = (atoi(argv[1]) * 10);
    int i = 0;

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 100000000;

    while((i++) < max_iters) {
        printf("Process Executing... (%d/%d)\n", i, max_iters);
        nanosleep(&ts, NULL);
    }


    return 0;
}