// iLab Server:

#include "rpthread.h"

#define STACK_SIZE SIGSTKSZ

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE

ucontext_t * scheudle_hold;
static ucontext_t mainCntx, schedCntx, curCntx;
rpthread_t mainThread;

int idNum = 0;

//int * schedule_init = &idNum; // value used to assit in initializing saved context for  scheduler 

int flip_main = 0;


node* runqueue = NULL; 
//node* blockedQ = NULL;

void timer_interupt(int signum){
        printf("RING RING! The timer has gone off\n");
        node* ptr = findCurThread();
        if(ptr!=NULL) {
                swapcontext(ptr->TCB.context, &schedCntx);
        }else {
                //or at this point is the main done?
                //can we terminate here?
                //should we set context to main?
                setcontext(&schedCntx);
        }   
}


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
                if(getcontext(&mainCntx)<0) {
                        printf("unable to set up main context\n");
                        return -1;
                }
                void* mStack = malloc(STACK_SIZE);
                mainCntx.uc_link = NULL;
                mainCntx.uc_stack.ss_sp = mStack;
                mainCntx.uc_stack.ss_size = STACK_SIZE;
                mainCntx.uc_stack.ss_flags = 0;

                tcb* mainBlock = malloc(sizeof(tcb));

                void* mainStack = malloc(STACK_SIZE);

                if(mainStack == NULL){
                        perror("no mem\n");
                        return -1;
                }

                mainBlock->threadid = idNum;
                //thread = &newBlock->threadid;

                mainBlock->status = 1;
                mainBlock->priority = 4;
                mainBlock->join = 0;

                mainBlock->context = &mainCntx;
                enqueue(mainBlock);

                //make schedCntx?
                if(getcontext(&schedCntx) < 0) {
                        printf("unable to set up scheduler context\n");
                        return -1;
                }
                void* sStack = malloc(STACK_SIZE);
                schedCntx.uc_link = NULL;
                schedCntx.uc_stack.ss_sp = sStack;
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
        newBlock->join = 0;

        makecontext(&cntx, (void*)function, 0); //last param should be arg value from create() signature        

        newBlock->context = &cntx;
        enqueue(newBlock);

        //whe timer is set up, we can return 0 and on timer interupt, swap from main context/thread
        setitimer(ITIMER_PROF, &timer, NULL);
        return 0;
        //return atexit((void *)(*schedule));
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

node* dequeue() {
        node* ptr = runqueue;
        runqueue = runqueue->next;
        return ptr;
};


//is this useless because shouldnt the currently running thread be at the front
node* findCurThread() {
        node* ptr = runqueue;
        while(ptr!=NULL) {
                if(ptr->TCB.status == 1) {
                        return ptr;
                }
                ptr = ptr->next;
        }
        return NULL;
}
/* give CPU possession to other user-level threads voluntarily */
int rpthread_yield() {
        // change thread state from Running to Ready
        // save context of this thread to its thread control block
        // witch from thread context to scheduler context
        node* ptr = runqueue;
        if(ptr->TCB.status == 1) {
                ptr->TCB.status = 0;
                dequeue();
                enqueue(&ptr->TCB);
                printf("yielded...\n");
                swapcontext(ptr->TCB.context, &schedCntx);
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
        //      if(ptr->TCB.status == 1 ){ 
        //              printf("three\n");

        //free(ptr->TCB.context->uc_stack.ss_sp); 
        //              free(ptr); 
        //              return;
        //      }*/

};
/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {

        // wait for a specific thread to terminate
        // de-allocate any dynamic memory created by the joining thread
        node* ptr = runqueue;
        node* curThread = NULL;
        while(ptr!=NULL) {
                if(ptr->TCB.status == 1) {
                        curThread = ptr;
                        break;
                }
        }
        if(curThread->TCB.threadid > thread) {
                printf("can't join on a parent\n");
                return -1;
        }else {
                //just check in scheduler if a thread is joining on any others
                /*if(blockedQ == NULL) {
                  blockedQ = curThread;
                  }else {
                  node* n = blockedQ;
                  while(n->next!=NULL) {
                  n = n->next;
                  }
                  n->next = curThread;
                  }*/
                ptr = runqueue;
                while(ptr!=NULL) {
                        if(ptr->TCB.threadid == thread) {
                                if(ptr->TCB.join != 1) {
                                        ptr->TCB.join = 1;
                                        //ptr->TCB.joiner = curThread->TCB;
                                        //no need bc can just check waitNum
                                        break;
                                }else {
                                        printf("can't join on the same thread 2x\n");
                                        return -1;
                                }
                        }
                }
                ++curThread->TCB.waitNum;
                swapcontext(curThread->TCB.context, &schedCntx);
                return 0;
        }
        // YOUR CODE HERE
        printf("thread does not exits\n");
        return -1;
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

        //do we need to get sched context every time?
        //what does it actually do 
        //i believe getcontext() is pointless here
        getcontext(&schedCntx);
        puts("Entering scheduler...\n");

        node* thread = runqueue;
        //if list is empty, should reset timer, and let the thread that just got switched out run again
        if(thread == NULL) {
                setitimer(ITIMER_PROF, &timer, NULL);
                return;
        }
  //find curr thread that caused the switch to scheduler
        //could be from 1)timer interupt or 2)thread finishing its work
        //will cur always be the front?
        /*while(thread!=NULL) {
          if(thread->TCB.status == 1) {

          break;
          }
          thread = thread->next;
          }*/
        printf("here1\n");
        //this will pop the thread that just finished running 
        node* ptr = dequeue();
        enqueue(&ptr->TCB);

        //this will pop the next thing in line and should be run
        ptr = dequeue();
        printf("here2, post pop put to back, and pop again\n");
        // Use sigaction to register signal handler
        // Set the timer up (start the timer)

        // Kill some time

        //      if(flip_main == 0){
        //              flip_main =1 ;

        //              node * ptr2 = runqueue;
        //      while(ptr != NULL){

        //      if( ptr->TCB.status == 0){
        //              while(ptr2!= NULL){ 
        //                      if(ptr2->TCB.status ==1){ 
        //                      ptr2->TCB.status = 0;
        //      } 
        //              ptr2 = ptr2->next;
        //}  

        //ptr->TCB.status = 1;
        if(ptr!=NULL) {
                printf("ptr was not null so we can set the next thing to run\n");
                if(ptr->TCB.waitNum == 0) {
                        ptr->TCB.status = 1;
                        setitimer(ITIMER_PROF, &timer, NULL);
                        printf("problem is swap()\n");
                        swapcontext(&schedCntx, ptr->TCB.context);
                }else {
                        enqueue(&ptr->TCB);
                        setcontext(&schedCntx);
                }
        }
        printf("ptr was null so queue is empty and can return\n");
        //      return;
        //      }
        //      ptr = ptr->next;

        //              }
        //} 
  //else{ 
        //      flip_main = 0;

        ///}




        // Every time when timer interrup happens, your thread library 
        // should be contexted switched from thread context to this 
        // schedule function

        // Invoke different actual scheduling algorithms
        // according to policy (RR or MLFQ)

        // if (sched == RR)
        //              sched_rr();
        // else if (sched == MLFQ)
        //              sched_mlfq();

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
