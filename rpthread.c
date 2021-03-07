// File:	rpthread.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "rpthread.h"

#define STACK_SIZE SIGSTKSZ

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE

static ucontext_t mainCntx;

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
	
	tcb* newBlock = malloc(sizeof(tcb));
	
	void* stack = malloc(STACK_SIZE);
	idNum++;
	
	if(stack == NULL){ 
		perror("no mem");
	}
	ucontext_t cntx;
	getcontext(&cntx);
	
	cntx.uc_link = NULL;	//should point to successor function
	cntx.uc_stack.ss_sp = stack;
	cntx.uc_stack.ss_size = STACK_SIZE;
	cntx.uc_stack.ss_flags = 0;
	
	newBlock->threadid = idNum;
	thread = &newBlock->threadid;
	
	newBlock->status =0;
	newBlock->priority = 4;
	
	makecontext(&cntx, (void*)function, 0);	
	
	newBlock->context = &cntx;
	appendToQ(newBlock); 
	
	return atexit( (void *)(*schedule));
};

void appendToQ(tcb* block) {
	
	node* newNode = malloc(sizeof(node));
	

	newNode->TCB = *block;
	newNode->next = NULL;
	if(runqueue == NULL){ 
		runqueue  = newNode;
		return;
	} 

	node * ptr = runqueue; 
	while(ptr->next!= NULL){ 
		ptr = ptr->next; 
	} 
	ptr->next = newNode;
	
	return;
}

/* give CPU possession to other user-level threads voluntarily */
int rpthread_yield() {

	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// wwitch from thread context to scheduler context

	// YOUR CODE HERE
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
	
	if(runqueue == NULL){ 
		return;
	}

	node* ptr = runqueue; 
		
			
		


//				printf("one \n" );

				if(ptr->TCB.status == 1 ){ 
							printf("one");
						//	free(ptr->TCB.context->uc_stack.ss_sp); 
							node * hold = runqueue; 
							
							runqueue = runqueue->next;
							
							free(hold);
							return;
						}



				while(ptr->next!=NULL){ 
							printf("two\n");

						if(ptr->next->TCB.status == 1 ){ 
							//free(ptr->next->TCB.context->uc_stack.ss_sp); 
							free(ptr->next); 
							
							ptr->next= NULL;
							return;							
						} 

							ptr->next= ptr->next->next;

				}

					if(ptr->TCB.status == 1 ){ 
								printf("three\n");

							//free(ptr->TCB.context->uc_stack.ss_sp); 
							free(ptr); 
							return;
												} 


			
		

		
			
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
	puts("we made it");
	

		node * ptr = runqueue; 

		while( ptr != NULL ){ 
			if(ptr->TCB.status == 0 ){ 
				ptr->TCB.status = 1; 
				setcontext(ptr->TCB.context);
			 	return;
			 }
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
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

//this method and main are just for testing purposes
void doTheThing() {
	printf("printing...\n");
	rpthread_exit(NULL);
//	return;
	
}

int main() {
	//ucontext_t mainCntx;
	getcontext(&mainCntx);
	rpthread_t thread1, thread2, thread3;	 
	int a = rpthread_create(&thread1, NULL, (void*)doTheThing, NULL);
	printf("success, on %d \n",a);
//	runqueue = NULL;
	int b = rpthread_create(&thread2, NULL, (void*)doTheThing, NULL); 
		printf("success, on %d \n", b); 
//	runqueue = NULL;
	rpthread_create(&thread3, NULL, (void*)doTheThing, NULL);	
	
	return 0;
}

// Feel free to add any other functions you need

// YOUR CODE HERE

