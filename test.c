#include "rpthread.c"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <ucontext.h>

#define STACK_SIZE SIGSTKSZ

void func(ucontext_t n) {
	printf("yooooo\n");
}



int main() {
        struct threadControlBlock* newBlock = malloc(sizeof(struct threadControlBlock));
        newBlock->threadid = 1;
        newBlock->status = 0;
        newBlock->priority = 4;
	void* stack = malloc(STACK_SIZE);	
	ucontext_t cntx, ncntx;
        newBlock->context = cntx;
        if(getcontext(&cntx) < 0) {
                printf("No bueno\n");
                exit(1);
        }
	cntx.uc_link=NULL;
	cntx.uc_stack.ss_sp=stack;
	cntx.uc_stack.ss_size=STACK_SIZE;
	cntx.uc_stack.ss_flags=0;
        makecontext(&cntx, func, 0);
	setcontext(&cntx);
	free(newBlock);
        return 0;
}

