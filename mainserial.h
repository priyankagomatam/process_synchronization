#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <semaphore.h>  /* Semaphore */


typedef int cond_t;

//node for waiting queue  and crowds
typedef struct node_t {
	pthread_t id;
	struct node_t *next;
	struct node_t *prev;
	sem_t *sema;
	cond_t (*func)();
	int priority;
} node_t;

//crowds
typedef struct crowd_t {
    node_t *head;
    node_t *tail;
    int size;
} crowd_t;

//crowdList Node
typedef struct crowdNode_t {
	crowd_t *element;
	struct crowdNode_t *next;
}crowdNode_t;

//crowd List
typedef struct crowdList_t {
	crowdNode_t *head;
	crowdNode_t *tail;
    int size;
} crowdList_t;

//waiting queues
typedef struct queue_t {
    node_t *head;
    node_t *tail;
    int size;
} queue_t;

//queueList Node
typedef struct queueNode_t {
	queue_t *element;
	struct queueNode_t *next;
}queueNode_t;

//queue List
typedef struct queueList_t {
	queueNode_t *head;
	queueNode_t *tail;
    int size;
} queueList_t;


//serializer
typedef struct serial_t{
	crowdList_t cList;
	queueList_t qList;
	sem_t enter_exit;
}serial_t;

serial_t* Create_Serial(); 

int Serial_Enter(serial_t*); 

int Serial_Exit(serial_t*); 

queue_t* Create_Queue(serial_t*);
 
crowd_t* Create_Crowd(serial_t*); 

int Queue_Empty(serial_t*, queue_t*); 

int Crowd_Empty(serial_t*, crowd_t*); 

void Serial_Enqueue(serial_t*, queue_t*, cond_t(*func)());

void Serial_Enqueue_ds(serial_t*, queue_t*, cond_t(*func)(), int);
 
void Serial_Join_Crowd(serial_t *s,crowd_t *c, void*(*func)());

void serial_dequeue(serial_t*);

void crowdList_enqueue(serial_t *s, crowd_t *c);

void queueList_enqueue(serial_t *s, queue_t *q);



