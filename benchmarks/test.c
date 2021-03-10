#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../rpthread.h"

/* A scratch program template on which to call and
 * test rpthread library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

void func() {
//	printf("printing...\n"); 
	int i = 0;
	while(1) {
		i++; 
		printf("%d \n", i);
	//		puts("never made it ");	
	}
//	printf("yerrrr\n");
	return;
}

void func2() {
//	int i = 0;
	//while(1) {
	//	printf("%d\n", i);
//		printf("yielding func2\n");
	//	rpthread_yield();
//		printf("back from func1, now done\n");
	while(1);
	//	++i;
	//	printf("%d\n", i);
	//	break;
//	}
	return;
}

int main(int argc, char **argv) {
	rpthread_t t1, t2;
	rpthread_create(&t1, NULL, (void*)func, NULL);  
//	rpthread_create(&t2, NULL, (void*)func2, 0);
	printf("here\n");
	return 0;
}
