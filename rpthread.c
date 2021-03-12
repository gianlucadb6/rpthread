// File:	rpthread.c

// List all group member's name: Gianluca Delbarba and Charles Rodriguez
// username of iLab: cd.cs.rutgers
// iLab Server: 

#include "rpthread.h"

#define STACK_SIZE SIGSTKSZ

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE

ucontext_t * schedCntx;
ucontext_t* mainCntx; 
rpthread_t mainThread;

int idNum = 0;

//int * schedule_init = &idNum; // value used to assit in initializing saved context for  scheduler 

int flip_main = 0;

node* runqueue = NULL;
node * runqueue_tail = NULL;
node * L2 = NULL;
node * L3 = NULL;
node * L4 = NULL;

//node* blockedQ = NULL;

void timer_interupt(int signum){
	printf("RING RING! The timer has gone off\n");
	if(runqueue != NULL){ 
		node * ptr = runqueue; 

		while(ptr!= NULL){ 
			if( ptr->TCB.status == 1){ 
				swapcontext(ptr->TCB.context, schedCntx);
				break;
			} 
			ptr = ptr->next;
		}  

	}
	else if( L2 != NULL){ 
		node * ptr = L2; 

		while(ptr!= NULL){ 
			if( ptr->TCB.status == 1){ 
				swapcontext(ptr->TCB.context, schedCntx);
				break;
			} 
			ptr = ptr->next;
		}  

	}

	else if(L3 != NULL){ 
		node * ptr = L3; 

		while(ptr!= NULL){ 
			if( ptr->TCB.status == 1){ 
				swapcontext(ptr->TCB.context, schedCntx);
				break;
			} 
			ptr = ptr->next;
		}  

	}

	else if(L4 != NULL){ 
		node * ptr = L4; 

		while(ptr!= NULL){ 
			if( ptr->TCB.status == 1){ 
				swapcontext(ptr->TCB.context, schedCntx);
				break;
			} 
			ptr = ptr->next;
		}  

	}
};


// And more ...
/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	// create Thread Control Block
	// create and initialize the context of this thread
	// allocate space of stack for this thread to run
	// after everything is all set, push this thread int
	// YOUR CODE HERE
	struct sigaction sa; 
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &timer_interupt;
	sigaction (SIGPROF, &sa, NULL);

	// Create timer struct
	struct itimerval timer;

	timer.it_interval.tv_usec = 0;  
	timer.it_interval.tv_sec = 0;

	// Set up the current timer to go off in 1 second
	// Note: if both of the following values are zero
	//       the timer will not be active, and the timer
	//       will never go off even if you set the interval value
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 1;

	if(idNum == 0) {
		mainCntx = malloc(sizeof(ucontext_t));
		if(getcontext(mainCntx)<0) {
			printf("unable to set up main context\n");
			return -1;
		}
		void* mStack = malloc(STACK_SIZE);
		mainCntx->uc_link = NULL;
		mainCntx->uc_stack.ss_sp = mStack;
		mainCntx->uc_stack.ss_size = STACK_SIZE;
		mainCntx->uc_stack.ss_flags = 0;

		tcb* mainBlock = malloc(sizeof(tcb));

		void* mainStack = malloc(STACK_SIZE);

		if(mainStack == NULL){
			perror("no mem\n");
			return -1;
		}

		mainBlock->threadid = idNum;
		//thread = &newBlock->threadid;

		mainBlock->status = 1;
		mainBlock->priority = 1;
		mainBlock->join = 0;

		mainBlock->context = mainCntx;
		enqueue(mainBlock);

		//make schedCntx?
		schedCntx = malloc(sizeof(ucontext_t));
		if(getcontext(schedCntx) < 0) {
			printf("unable to set up scheduler context\n");
			return -1;
		}
		void* sStack = malloc(STACK_SIZE);
		schedCntx->uc_link = mainCntx;		
		schedCntx->uc_stack.ss_sp = sStack;
		schedCntx->uc_stack.ss_size = STACK_SIZE;
		schedCntx->uc_stack.ss_flags = 0;
		makecontext(schedCntx, (void*)schedule, 0);
	}
	puts("in pthread create()\n"); 

	++idNum;
	printf("idNum = %d\n", idNum);
	tcb* newBlock = malloc(sizeof(tcb));

	void* stack = malloc(STACK_SIZE);

	if(stack == NULL){ 
		perror("no mem\n");
		return -1;
	}
	ucontext_t* cntx = malloc(sizeof(ucontext_t));

	if(getcontext(cntx)) {
		printf("unable to get contrxt\n");
		return -1;
	}

	cntx->uc_link = mainCntx;	
	cntx->uc_stack.ss_sp = stack;
	cntx->uc_stack.ss_size = STACK_SIZE;
	cntx->uc_stack.ss_flags = 0;

	newBlock->threadid = idNum;
	//thread = malloc(sizeof(uint));
	*thread = idNum;
	printf("thread = %d\n", *thread);

	newBlock->status = 0;
	newBlock->priority = 1;
	newBlock->join = 0;
	newBlock->joiner = malloc(sizeof(tcb));

	makecontext(cntx, (void*)function, 1, arg); //last param should be arg value from create() signature	

	newBlock->context = cntx;
	enqueue(newBlock); 

	//whe timer is set up, we can return 0 and on timer interupt, swap from main context/thread
	setitimer(ITIMER_PROF, &timer, NULL);
	return 0;
	//return atexit((void *)(*schedule));
};

//this method takes a tcb struct, creates a node srtuct out of it, pushes it to
//the back of a a queue. The queue number to push to is determined by the tcb* 
//parameter's priority.
void enqueue(tcb* block) {

	node* newNode = malloc(sizeof(node));

	newNode->TCB = *block;
	newNode->next = NULL;
	if(newNode->TCB.priority == 1) {
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
	}else if(newNode->TCB.priority == 2) {
		if(L2 == NULL){ 
			L2  = newNode;
			return;
		}   

		node* ptr = L2; 
		while(ptr->next!= NULL){ 
			ptr = ptr->next; 
		}   
		ptr->next = newNode;

		return;
	}else if(newNode->TCB.priority == 3) {
		if(L3 == NULL){ 
			L3  = newNode;
			return;
		}   

		node* ptr = L3; 
		while(ptr->next!= NULL){ 
			ptr = ptr->next; 
		}   
		ptr->next = newNode;

		return;
	}else {
		if(L4 == NULL){ 
			L4  = newNode;
			return;
		}   

		node* ptr = L4; 
		while(ptr->next!= NULL){ 
			ptr = ptr->next; 
		}   
		ptr->next = newNode;

		return;
	}

};

//dequeue method which pops the currently running node from its queue
//the queue to pop from is determined by checking the priority of the
//currently running thread
node* dequeue() {
	if(findCurThread()->TCB.priority == 1) {
		node* ptr = runqueue;
		if(ptr->next != NULL) {
			runqueue = runqueue->next;
		}
		return ptr;
	}else if(findCurThread()->TCB.priority == 2) {
		node* ptr = L2;
		if(ptr->next != NULL) {
			L2 = L2->next;
		}   
		return ptr;
	}else if(findCurThread()->TCB.priority == 3) {
		node* ptr = L3;
		if(ptr->next != NULL) {
			L3 = L3->next;
		}   
		return ptr;
	}else {
		node* ptr = L4;
		if(ptr->next != NULL) {
			L4 = L4->next;
		}
		return ptr;
	}	

};

//this method determined the currently running thread by searching all queues for
//a thread with priority 1, which is then returned.
node* findCurThread() {
	if(runqueue != NULL) {
		node* ptr = runqueue;
		while(ptr!=NULL) {
			if(ptr->TCB.status == 1) {
				return ptr;
			}
			ptr = ptr->next;
		}
		//return NULL;
	}else if(L2 != NULL) {
		node* ptr = L2;
		while(ptr!=NULL) {
			if(ptr->TCB.status == 1) {
				return ptr;
			}   
			ptr = ptr->next;
		}   
		//return NULL;
	}else if(L3 != NULL) {	
		node* ptr = L3;
		while(ptr!=NULL) {
			if(ptr->TCB.status == 1) {
				return ptr;
			}
			ptr = ptr->next;
		}
		//return NULL;
	}else {
		node* ptr = L4;
		while(ptr!=NULL) {
			if(ptr->TCB.status == 1) {
				return ptr;
			}   
			ptr = ptr->next;
		}   
		//return NULL;
	}
	return NULL;
}
/* give CPU possession to other user-level threads voluntarily */
int rpthread_yield() {
	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// witch from thread context to scheduler context
	node* ptr = dequeue();
	if(ptr != NULL) {
		printf("this thread was running, gona throw it on back of runqueue\n");
		ptr->TCB.status = 0;
		//dequeue();
		enqueue(&ptr->TCB);
		printf("yielded...\n");
		swapcontext(ptr->TCB.context, schedCntx);
	}
	return 0;
}

/*node* prev = NULL;
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
};*/


/* terminate a thread */
void rpthread_exit(void *value_ptr) {
	/*  
Assumptions:  
- Only one read can run at a time  
- If this function is called it was done by a thread that was running

idea: if this function is called find the first thread in runqueue that has status running, 
go to that thread and dequeue it. Once dequeued, we can then free it's stack and remove it's  
tcb*/
	node * ptr = findCurThread();  
	if(ptr->TCB.priority == 1) {
		if(runqueue->TCB.status == 1 ){ 
			node * hold = runqueue;
			runqueue = runqueue->next;
			if(hold->TCB.join == 1) {
				--hold->TCB.joiner->waitNum;
			}
			free(hold); 
			hold = NULL;
			setcontext(schedCntx);
			return;
		}
	}else if(ptr->TCB.priority == 2) {
		if(L2->TCB.status == 1 ){  
			node * hold = L2;
			L2 = L2->next;
			if(hold->TCB.join == 1) {
				--hold->TCB.joiner->waitNum;
			}   
			free(hold); 
			hold = NULL;
			setcontext(schedCntx);
			return; //value_ptr;
		}   
	}else if(ptr->TCB.priority == 3) {
		if(L3->TCB.status == 1 ){  
			node * hold = L3;
			L3 = L3->next;
			if(hold->TCB.join == 1) {
				--hold->TCB.joiner->waitNum;
			}   
			free(hold); 
			hold = NULL;
			setcontext(schedCntx);
			return; //value_ptr;
		}   
	}else {
		if(L4->TCB.status == 1 ){  
			node * hold = L4;
			L4 = L4->next;
			if(hold->TCB.join == 1) {
				--hold->TCB.joiner->waitNum;
			}   
			free(hold); 
			hold = NULL;
			setcontext(schedCntx);
			return; //value_ptr;
		}   
	}

};


/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {
	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread
	node* curThread = findCurThread();
	if(curThread->TCB.threadid > thread) {
		printf("can't join on a parent\n");
		return -1;
	}else {
		int i = 0;
		while(i < 4) {
			node* ptr;
			if(i == 1) {
				ptr = runqueue;
			}else if(i == 2) {
				ptr == L2;
			}else if(i == 3) {
				ptr = L3;
			}else {
				ptr = L4;
			}
			while(ptr!=NULL) {
				//find thread to join on
				if(ptr->TCB.threadid == thread) {
					//if its already set to one, it shouldnt be joined on again
					if(ptr->TCB.join != 1) {
						printf("this thread can properly be joined on\n");
						//this thread is now being joined on
						ptr->TCB.join = 1;
						++curThread->TCB.waitNum;
						curThread->TCB.status = 2;
						//joiner is the thread that called join; used to know whick thread to decr waitNum on when a thread is done
						ptr->TCB.joiner = &(curThread->TCB);
						//enqueue(&curThread->TCB);
						swapcontext(curThread->TCB.context, schedCntx);		
						//--curThread->TCB.waitNum;
						if(curThread->TCB.waitNum == 0) {
							curThread->TCB.status = 0;
						}
						return 0;
						//setcontext(schedCntx);
					}else {
						printf("can't join on the same thread 2x\n");
						//or just return
						return -1;
					}
				}
				ptr = ptr->next;
			}
			++i;
		}
		//the thread to join on already finished
		swapcontext(curThread->TCB.context, schedCntx);
		return 0;
	}
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
	/*struct sigaction sa;
	  memset (&sa, 0, sizeof (sa));
	  sa.sa_handler = &timer_interupt;
	  sigaction (SIGPROF, &sa, NULL);

	// Create timer struct
	struct itimerval timer;

	timer.it_interval.tv_usec = 0; 
	timer.it_interval.tv_sec = 0;

	// Set up the current timer to go off in 1 second
	// Note: if both of the following values are zero
	//       the timer will not be active, and the timer
	//       will never go off even if you set the interval value
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 1;

	//do we need to get sched context every time?
	//what does it actually do 
	//i believe getcontext() is pointless here
	//getcontext(&schedCntx);
	 */
	//puts("Entering scheduler...\n");
	sched_rr();
	//sched_mlfq();
	/*
	   node* thread = runqueue;
	//if list is empty, should reset timer, and let the thread that just got switched out run again
	if(thread == NULL) {
	setitimer(ITIMER_PROF, &timer, NULL);
	setcontext(mainCntx);
	}
	//find curr thread that caused the switch to scheduler
	//could be from 1)timer interupt or 2)thread finishing its work
	//will cur always be the front?
	while(thread!=NULL) {
	if(thread->TCB.status == 1) {

	break;
	}
	thread = thread->next;
	}
	printf("runqueue is not empty\n");
	//this will pop the thread that just finished running 
	//if(thread->next == NULL) {
	//	printf("only one thing in the runqueue\n");
	//}
	node* ptr = dequeue();
	enqueue(&ptr->TCB);

	//this will pop the next thing in line and should be run
	ptr = dequeue();
	 */	
	//printf("here2, post pop put to back, and pop again\n");
	// Use sigaction to register signal handler
	// Set the timer up (start the timer)

	// Kill some time

	//	if(flip_main == 0){
	//		flip_main =1 ;

	//		node * ptr2 = runqueue;
	//	while(ptr != NULL){

	//	if( ptr->TCB.status == 0){
	//		while(ptr2!= NULL){ 
	//			if(ptr2->TCB.status ==1){ 
	//			ptr2->TCB.status = 0;
	//	} 
	//		ptr2 = ptr2->next;
	//}  

	//ptr->TCB.status = 1;
	/*if(ptr!=NULL) {
	  printf("ptr was not null so we can set the next thing to run\n");
	  if(ptr->TCB.waitNum == 0) {
	  ptr->TCB.status = 1;
	  setitimer(ITIMER_PROF, &timer, NULL);
	  printf("setting context now\n");
	  setcontext(ptr->TCB.context); 
	  }else {
	  printf("this thread is currently blocked\n");
	  enqueue(&ptr->TCB);
	  setcontext(schedCntx);
	  }
	  }
	  printf("ptr was null so queue is empty and can return\n");
	 */
	//set main context or return?
	//setcontext(mainCntx);



	//	return;
	//	}
	//	ptr = ptr->next;

	//		}
	//} 
	//else{ 
	//	flip_main = 0;

	///}




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
};

/* Round Robin (RR) scheduling algorithm */
static void sched_rr() {
	// Your own implementation of RR
	// (feel free to modify arguments and return types)
	// Your own implementation of RR
	// (feel free to modify arguments and return types)

	// Use sigaction to register signal handler

	node * ptr = runqueue; 
	if(runqueue == NULL){ 
		return;
	}

	struct sigaction sa;
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &timer_interupt;
	sigaction (SIGPROF, &sa, NULL);

	// Create timer struct
	struct itimerval timer;

	timer.it_interval.tv_usec = 0; 
	timer.it_interval.tv_sec = 0;

	// Set up the current timer to go off in 1 second
	// Note: if both of the following values are zero
	//       the timer will not be active, and the timer
	//       will never go off even if you set the interval value
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 1;

	node * ptr2 = runqueue;

	while(ptr != NULL){

		if( ptr->TCB.status == 0){
			/*
			   while(ptr2!= NULL){ 
			   if(ptr2->TCB.status ==1 ){ 
			   ptr2->TCB.status = 0; 
			   enqueue(&ptr2->TCB); 
			   runqueue = runqueue->next;

			   break;
			   } 
			   ptr2 = ptr2->next;
			   }  
			 */


			ptr->TCB.status = 1;

			setitimer(ITIMER_PROF, &timer, NULL);

			setcontext(ptr->TCB.context); 

		}  

		else{ 
			//	enqueue(&runqueue->TCB); 
			node * ptr2 = runqueue; 

			while(ptr2->next != NULL){ 
				ptr2 = ptr2->next;
			} 
			node * insert_val = (node *)malloc(sizeof(node)); 
			insert_val->TCB = runqueue->TCB; 
			insert_val->TCB.status = 0;
			insert_val->next= NULL;
			ptr2->next = insert_val; 

			node * hold  = runqueue;
			runqueue = runqueue->next; 
			free(hold); 
			hold = NULL;
			ptr = runqueue;	

		}

		//	ptr = ptr->next;

	} 



	// YOUR CODE HERE
};

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	puts("In MLFQ scheduler\n");
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)
	struct sigaction sa;
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &timer_interupt;
	sigaction (SIGPROF, &sa, NULL);

	// Create timer struct
	struct itimerval timer;

	timer.it_interval.tv_usec = 0; 
	timer.it_interval.tv_sec = 0;

	// Set up the current timer to go off in 1 second
	// Note: if both of the following values are zero
	//       the timer will not be active, and the timer
	//       will never go off even if you set the interval value
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 1;

	puts("In MLFQ scheduler\n");

	if(runqueue != NULL){
		node * ptr = runqueue;
		node * ptr2 = runqueue;

		while(ptr != NULL){

			if( ptr->TCB.status == 0){

				ptr->TCB.status = 1;

				setitimer(ITIMER_PROF, &timer, NULL);

				setcontext(ptr->TCB.context); 

			} 

			else{ 
				node * ptr2 = L2;


				node * insert_val = (node *)malloc(sizeof(node)); 
				insert_val->TCB = runqueue->TCB; 
				insert_val->TCB.status = 0;
				insert_val->next= NULL;
				if( ptr2!= NULL){
					while(ptr2->next!= NULL){ 
						ptr2 = ptr2->next;
					} 
					ptr2->next = insert_val; 
				} 
				else{ 
					L2 = insert_val;
				}


				node * hold  = runqueue;
				runqueue = runqueue->next; 
				free(hold); 
				hold = NULL;
				ptr = runqueue;	



			}



		}
	} 

	if(L2 != NULL){ 

		struct sigaction sa;
		memset (&sa, 0, sizeof (sa));
		sa.sa_handler = &timer_interupt;
		sigaction (SIGPROF, &sa, NULL);

		// Create timer struct
		struct itimerval timer;




		timer.it_interval.tv_usec = 0; 
		timer.it_interval.tv_sec = 0;

		// Set up the current timer to go off in 1 second
		// Note: if both of the following values are zero
		//       the timer will not be active, and the timer
		//       will never go off even if you set the interval value
		timer.it_value.tv_usec = 0;
		timer.it_value.tv_sec = 1;




		node * ptr = L2;
		node * ptr2 = L2;

		while(ptr != NULL){

			if( ptr->TCB.status == 0){

				ptr->TCB.status = 1;

				setitimer(ITIMER_PROF, &timer, NULL);

				setcontext(ptr->TCB.context); 

			} 
			else{ 
				node * ptr2 = L3;


				node * insert_val = (node *)malloc(sizeof(node)); 
				insert_val->TCB = L2->TCB; 
				insert_val->TCB.status = 0;
				insert_val->next= NULL;
				if( ptr2!= NULL){
					while(ptr2->next!= NULL){ 
						ptr2 = ptr2->next;
					} 
					ptr2->next = insert_val; 
				} 
				else{ 
					L3 = insert_val;
				}


				node * hold  = L2;
				L2 = L2->next; 
				free(hold); 
				hold = NULL;
				ptr = L2;	



			}

		} 
	}
	if(L3 != NULL){ 
		node * ptr = L3;
		node * ptr2 = L3;

		while(ptr != NULL){

			if( ptr->TCB.status == 0){
				ptr->TCB.status = 1;
				setitimer(ITIMER_PROF, &timer, NULL);
				setcontext(ptr->TCB.context); 
			} 
			else{ 
				node * ptr2 = L4;


				node * insert_val = (node *)malloc(sizeof(node)); 
				insert_val->TCB = L3->TCB; 
				insert_val->TCB.status = 0;
				insert_val->next= NULL;
				if( ptr2!= NULL){
					while(ptr2->next!= NULL){ 
						ptr2 = ptr2->next;
					} 
					ptr2->next = insert_val; 
				} 
				else{ 
					L4 = insert_val;
				}


				node * hold  = L3;
				L3 = L3->next; 
				free(hold); 
				hold = NULL;
				ptr = L3;	




			}


		} 
	}
	if(L4 != NULL){ 
		node * ptr = L4;
		node * ptr2 = L4;


		while(ptr != NULL){

			if( ptr->TCB.status == 0){


				ptr->TCB.status = 1;

				setitimer(ITIMER_PROF, &timer, NULL);

				setcontext(ptr->TCB.context); 

			}  

			else{ 
				node * ptr2 = L4; 

				while(ptr2->next != NULL){ 
					ptr2 = ptr2->next;
				} 
				node * insert_val = (node *)malloc(sizeof(node)); 
				insert_val->TCB = L4->TCB; 
				insert_val->TCB.status = 0;
				insert_val->next= NULL;
				ptr2->next = insert_val; 

				node * hold  = L4;
				L4 = L4->next; 
				free(hold); 
				hold = NULL;
				ptr = L4;	

			}


		} 
	}
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

