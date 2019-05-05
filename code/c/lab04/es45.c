#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "utils.h"

int terminated = 0;

void sig_usr1_handler(int s) {
        zprintf(1, "received SIGUSR1!\n");
}

void sig_usr2_handler(int s) {
        zprintf(1, "received SIGUSR2! quitting...!\n");
        terminated = 1;
}

/* child function */
int child() {
    zprintf(1, "[%d] child started...\n", getpid());
    signal(SIGUSR1, sig_usr1_handler);
    signal(SIGUSR2, sig_usr2_handler);
    
    while(!terminated) {
        /* avoids cpu waste */
        sleep(1);
    }
    exit(EXIT_SUCCESS);
}


/* father function */
int father(int child_n, int *pids) {
    int i, pid, status;

    zprintf(1, "[%d] father started...\n", getpid());
    
    /* wait for children */
    sleep(1);
    
    for (i = 0; i < child_n ; i++) { 
        zprintf(1, "[%d] sending SIGUSR1 to child (%d, %d)...\n", getpid(), i, pids[i]);
        kill(pids[i], SIGUSR1);
    }
    
    /* wait for children */
    sleep(1);
    
    for (i = 0; i < child_n ; i++) { 
        zprintf(1, "[%d] sending SIGUSR2 to child (%d, %d)...\n", getpid(), i, pids[i]);
        kill(pids[i], SIGUSR2);
    }
    
    /* wait child before exit */
    if ((pid = wait(&status)) == -1) {
        zprintf(1, "error: wait()\n");
        exit(EXIT_FAILURE);
    }
    if (!WIFEXITED(status)) {
        zprintf(1, "[%d] Child exited abnormally\n", pid);
        exit(EXIT_FAILURE);
    }
    zprintf(1, "[%d] Child pid=%d exit=%d\n", getpid(), pid, WEXITSTATUS(status));
    exit(EXIT_SUCCESS);
}


/* main function */
int main(int argc, char **argv) {
    int i, *pids, child_n;
    
    /* arguments check */
    if (argc != 2) {
        zprintf(1, "error: %s n_children\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    /* get child_n from command line */
    child_n = atoi(argv[1]);
    if (child_n <= 0) { 
        zprintf(1, "error: n_children must be positive\n");
        exit(EXIT_FAILURE);
    }
    
    /* allocate memory for pids */
    pids = (int *)malloc(sizeof(int) * child_n);
    
    /* init child processes */
    for (i=0; i < child_n; i++) { 
        pids[i] = fork(); 
        switch(pids[i]) {
            case -1:
                zprintf(1, "[%d] error fork()\n", getpid());
                exit(EXIT_FAILURE);
            case 0: 
                child();
        }
    }
    father(child_n, pids);
}

