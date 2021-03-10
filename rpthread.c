// File:	rpthread.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "rpthread.h"

#define STACK_SIZE SIGSTKSZ

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE

static ucontext_t mainCntx, schedCntx, curCntx;


int idNum = 0;

node* runqueue = NULL; 


// And more ...
/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	// create Thread Control Block
	// create and initialize the context of this thread
	// allocate space of stack for this thread to run
	// after everything is all set, push this thread int
	// YOUR CODE HERE
	if(idNum == 0) {
		if(getcontext(&mainCntx)<0) {
			printf("unable to set up main context\n");
			return -1;
		}
		//make schedCntx?
		if(getcontext(&schedCntx) < 0) {
			printf("unable to set up scheduler context\n");
			return -1;
		}
		void* stack = malloc(STACK_SIZE);
		schedCntx.uc_link = NULL;		
		schedCntx.uc_stack.ss_sp = stack;
		schedCntx.uc_stack.ss_size = STACK_SIZE;
		schedCntx.uc_stack.ss_flags = 0;
		makecontext(&schedCntx, (void*)schedule, 0);
	}
	++idNum;

	tcb* newBlock = malloc(sizeof(tcb));

	void* stack = malloc(STACK_SIZE);

	if(stack == NULL){ 
		perror("no mem\n");
		return -1;
	}
	ucontext_t cntx;
	
	if(getcontext(&cntx)) {
		printf("unable to get contrxt\n");
		return -1;
	}

	cntx.uc_link = NULL;	
	cntx.uc_stack.ss_sp = stack;
	cntx.uc_stack.ss_size = STACK_SIZE;
	cntx.uc_stack.ss_flags = 0;

	newBlock->threadid = idNum;
	thread = &newBlock->threadid;

	newBlock->status = 0;
	newBlock->priority = 4;

	makecontext(&cntx, (void*)function, 0); //last param should be arg value from create() signature	

	newBlock->context = &cntx;
	enqueue(newBlock); 
	
	//whe timer is set up, we can return 0 and on timer interupt, swap from main context/thread
	//return 0;
	return atexit((void *)(*schedule));
};


void enqueue(tcb* block) {

	node* newNode = malloc(sizeof(node));

	newNode->TCB = *block;
	newNode->next = NULL;
	if(runqueue == NULL){ 
		runqueue  = newNode;
		return;
	} 

	node* ptr = runqueue; 
	while(ptr->next!= NULL){ 
		ptr = ptr->next; 
	} 
	ptr->next = newNode;

	return;
};

tcb* dequeue(node* runqueue) {
	node* ptr = runqueue;
	runqueue = runqueue->next;
	return &ptr->TCB;
};



/* give CPU possession to other user-level threads voluntarily */
int rpthread_yield() {
	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// witch from thread context to scheduler context
	node* ptr = runqueue;
	node* prev = NULL;
	while(ptr!=NULL) {
		if(ptr->TCB.status == 1) {
			//set status to 0, readjust list, and swapcontexts
			prev->next = ptr->next;
			ptr->TCB.status = 0;
			enqueue(&ptr->TCB);
			//free ptr? might messt up ptr->TCB
			//chane enqueue to take node* as param?
			printf("yielded...\n");
			swapcontext(ptr->TCB.context, &schedCntx);
			break;	
		}
	}
	return 0;
};


/* terminate a thread */
void rpthread_exit(void *value_ptr) {
	/*  
Assumptions:  
- Only one read can run at a time  
- If this function is called it was done by a thread that was running

idea: if this function is called find the first thread in runqueue that has status running, 
go to that thread and dequeue it. Once dequeued, we can then free it's stack and remove it's  
tcb
	 */
	// YOUR CODE HERE
	/*
	if(runqueue == NULL){ 
		printf("nothing to schedule\n");
		return;
	}
	runqueue;
	while(1) {
		node* ptr = dequeue(runqueue);
		printf("%d = done\n", ptr->TCB.threadid);
		return;
	}
	//printf("one \n" );
	if(ptr->TCB.status == 1 ){ 
		//printf("one\n");
		//free(ptr->TCB.context->uc_stack.ss_sp); 
		node * hold = runqueue; 

		runqueue = runqueue->next;

		free(hold);
		return;
	}
	while(ptr!=NULL){ 
		//printf("two\n");

		if(ptr->next->TCB.status == 1){ 
			//free(ptr->next->TCB.context->uc_stack.ss_sp); 
			free(ptr->next); 

			ptr->next= NULL;
			return;							
		} 

		ptr->next= ptr->next->next;

	}
//	if(ptr->TCB.status == 1 ){ 
//		printf("three\n");

		//free(ptr->TCB.context->uc_stack.ss_sp); 
//		free(ptr); 
//		return;
//	}*/ 

};


/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {

	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread

	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int rpthread_mutex_init(rpthread_mutex_t *mutex, 
		const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int rpthread_mutex_lock(rpthread_mutex_t *mutex) {
	// use the built-in test-and-set atomic function to test the mutex
	// if the mutex is acquired successfully, enter the critical section
	// if acquiring mutex fails, push current thread into block list and //  
	// context switch to the scheduler thread

	// YOUR CODE HERE
	return 0;
};

/* release the mutex lock */
int rpthread_mutex_unlock(rpthread_mutex_t *mutex) {
	// Release mutex and make it available again. 
	// Put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int rpthread_mutex_destroy(rpthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in rpthread_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	puts("Entering scheduler...\n");
	int  i = 1;	
	node* ptr = dequeue(&runqueue); 
	while(ptr != NULL){//1?
		//printf("%d\n", i);
		//++i; 
		if(ptr->TCB.status == 0 ){ 
			ptr->TCB.status = 1; 
			swapcontext(&mainCntx, ptr->TCB.context);
			printf("returning from thread...\n");
			//getcontext(&mainCntx);
			//ptr = runqueue;
			//continue;
		}
		ptr = ptr->next;
	}
	ptr = runqueue;
	while(ptr != NULL) {
		swapcontext(&mainCntx, ptr->TCB.context);
		ptr = ptr->next;
	}

	// Every time when timer interrup happens, your thread library 
	// should be contexted switched from thread context to this 
	// schedule function

	// Invoke different actual scheduling algorithms
	// according to policy (RR or MLFQ)

	// if (sched == RR)
	//		sched_rr();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

	// schedule policy
#ifndef MLFQ
	// Choose RR
	// CODE 1
#else 
	// Choose MLFQ
	// CODE 2
#endif
	return;
}

/* Round Robin (RR) scheduling algorithm */
static void sched_rr() {
	// Your own implementation of RR
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
};

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
};

//this method and main are just for testing purposes
/*void doTheThing() {
	printf("printing...\n");
	rpthread_yield();
	printf("...back to printing\n");
	//rpthread_exit(NULL);
	//	return;

}

int main() {
	//ucontext_t mainCntx;
	//getcontext(&mainCntx);
	rpthread_t thread1, thread2, thread3;	 
	int a = rpthread_create(&thread1, NULL, (void*)doTheThing, NULL);
	printf("success, on %d \n",a);
	//	runqueue = NULL;
	int b = rpthread_create(&thread2, NULL, (void*)doTheThing, NULL); 
	printf("success, on %d \n", b); 
	//	runqueue = NULL;
	//rpthread_create(&thread3, NULL, (void*)doTheThing, NULL);	

	return 0;
}*/

// Feel free to add any other functions you need

// YOUR CODE HERE

