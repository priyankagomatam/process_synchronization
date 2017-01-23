#include "serial.h"

serial_t* Create_Serial(){
	serial_t *construct;
	construct = (serial_t*)malloc(sizeof(serial_t));
	sem_init(&(construct->enter_exit), 0, 1);
	construct->cList.size=0;
	construct->cList.head=NULL;
	construct->cList.tail=NULL;
	construct->qList.size=0;
	construct->qList.head=NULL;
	construct->qList.tail=NULL;
	return construct;
}

queue_t* Create_Queue(serial_t *s){
	queue_t *q;
	q = (queue_t*)malloc(sizeof(queue_t));
	q->size=0;
	q->head=NULL;
	q->tail=NULL;
	queueList_enqueue(s, q);
	return q;
}

crowd_t* Create_Crowd(serial_t *s){
	crowd_t *c;
	c = (crowd_t*)malloc(sizeof(crowd_t));
	c->size=0;
	c->head=NULL;
	c->tail=NULL;
	crowdList_enqueue(s, c);
	return c;
}

int Serial_Enter(serial_t *s){	

	//check to see if thread can get serializer
	sem_wait(&(s->enter_exit));
	
	return 0;
}

void crowdList_enqueue(serial_t *s, crowd_t *c){
	crowdNode_t *node;

	//Create new node
	node = (crowdNode_t*)malloc(sizeof(crowdNode_t));

	//if this list is new
	if(s->cList.size==0)	
		s->cList.head=node;
	else		
		//Append to the end
		s->cList.tail->next=node;

	//set info
	node->element=c;

	//Nothing on the right of this node so....
	node->next = NULL;

	//Now the end of the queue points to..
	s->cList.tail = node;

	//increment size
	s->cList.size = s->cList.size + 1;

}

void queueList_enqueue(serial_t *s, queue_t *q){
	queueNode_t *node;

	//Create new node
	node = (queueNode_t*)malloc(sizeof(queueNode_t));

	//if this list is new
	if(s->qList.size==0)	
		s->qList.head=node;
	else		
		//Append to the end
		s->qList.tail->next=node;

	//set info
	node->element=q;

	//Nothing on the right of this node so....
	node->next = NULL;

	//Now the end of the queue points to..
	s->qList.tail = node;

	//increment size
	s->qList.size = s->qList.size + 1;

}

int Queue_Empty(serial_t *s, queue_t *q){
	if(q->size == 0)
		return 1;
	else
		return 0;
}

int Crowd_Empty(serial_t *s, crowd_t *c){
	if(c->size == 0)
		return 1;
	else
		return 0;
} 

void Serial_Enqueue(serial_t *s, queue_t *q, cond_t (*func)()){
node_t *node;

	if(!func()){			
		//Create new node
		node = (node_t*)malloc(sizeof(node_t));

		//if this list is new
		if(q->size==0){	
			q->head=node;
		} else {
			//Append to the end	
			q->tail->next=node;
		}

		//set information		
		node->sema=(sem_t*)malloc(sizeof(sem_t));
		sem_init(node->sema, 0, 0); 
		node->func=func;

		//Nothing on the right of this node so....
		node->next = NULL;

		//Now the end of the queue points to..
		q->tail = node;
		
		//increment size; do this after giving serializer away in case the list is new
		q->size = q->size+1;
		
		//when we call enqueue we releases the serializer so we need
		//to check if head of queue can get it, else in serial_dequeue give it to thread entering or exiting
		serial_dequeue(s);
				
		
		//put the thread to sleep
		sem_wait(node->sema);
		//			<--dequeued threads wake up here
		
	} 
}

void Serial_Enqueue_ds(serial_t *s, queue_t *q, cond_t (*func)(), int priority){
node_t *node;
node_t *current;

	if(!func()){			
		//Create new node
		node = (node_t*)malloc(sizeof(node_t));

		//if this list is new
		if(q->size==0){	
			q->head=node;

			//Now the end of the queue points to..
			q->tail = node;
		} else {
			current = q->head;
			while(current!=NULL){
				if(current->next!=NULL) {
					if( priority >= current->priority && priority >= current->next->priority) 					
						break;
				}
				current=current->next;
			}
			if(current==NULL) {	//append at the end
				q->tail->next = node;

				//Nothing on the right of this node so....
				node->next = NULL;

				//Now the end of the queue points to..
				q->tail = node;

			} else {
				node->next=current->next;
				current->next = node;
			}
		}

		//set information		
		node->sema=(sem_t*)malloc(sizeof(sem_t));
		sem_init(node->sema, 0, 0); 
		node->func=func;
		node->priority = priority;

		//increment size; do this after giving serializer away in case the list is new
		q->size = q->size+1;
		
		//when we call enqueue we releases the serializer so we need
		//to check if head of queue can get it, else in serial_dequeue give it to thread entering or exiting
		serial_dequeue(s);
			

		//put the thread to sleep
		sem_wait(node->sema);
		//			<--dequeued threads wake up here
		
	} 
}

void serial_dequeue(serial_t *s){
node_t *node;
int dequeued=0;
queueNode_t *currentQueueNode = s->qList.head;
queue_t *currentQueue;

	if(s->qList.size > 0 ){	//if there are queues
	
		while(currentQueueNode!=NULL) {
			
			currentQueue = currentQueueNode->element;
						
			if(currentQueue->size > 0) {//if there is stuff in the queue
			
				//get the top
				node = currentQueue->head;

				if(node->func()) {
					
					dequeued = 1;
					
					//increment pointer to the next node in line
					currentQueue->head = currentQueue->head->next;
					if (currentQueue->head == NULL){		//last element in queue
						currentQueue->tail = NULL;			//so empty queue
					}	
					
					//decrement size
					currentQueue->size = currentQueue->size-1;			
					
					//wake up thread
					sem_post(node->sema);
						
					//delete old queue node	
					free(node);				
					
					break;
				} 
			}
				
			currentQueueNode=currentQueueNode->next;
		}	
			
		if(!dequeued) {
			//fcfs to enter and exiting threads
			sem_post(&(s->enter_exit));
		}
			
	} else {
		//fcfs to enter and exiting threads
		sem_post(&(s->enter_exit));
	}
}

void Serial_Join_Crowd(serial_t *s,crowd_t *c, void*(*func)()){
	
		node_t *node;
	
		crowdNode_t *currentCrowdnode = s->cList.head;
		
		//Create new node
		node = (node_t*)malloc(sizeof(node_t));

		//Find the crowd in the list
		while(currentCrowdnode!=NULL && currentCrowdnode->element != c) {
			currentCrowdnode=currentCrowdnode->next;
		}

		if(currentCrowdnode->element->size==0){	//if this list is new
			currentCrowdnode->element->head=node;
			node->prev = NULL;
		}

		//set information		
		node->id=pthread_self();

		//Append to the end
		if(currentCrowdnode->element->size > 0) {
			node->prev = currentCrowdnode->element->tail;
			currentCrowdnode->element->tail->next=node;
		}

		//Nothing on the right of this node so....
		node->next = NULL;

		//Now the end of the queue points to..
		currentCrowdnode->element->tail = node;

		//increment size
		currentCrowdnode->element->size = currentCrowdnode->element->size+1;

		//release serializer
		serial_dequeue(s);
		
		//call the thread routine
		if(func!=NULL) {
			func();
		}
}


int Serial_Exit(serial_t *s){ 

	crowdNode_t 	*currentCrowd = s->cList.head;
	node_t			*currentThread;

	//check to see if thread can get serializer
	sem_wait(&(s->enter_exit));
	
	//leave crowd		
	while(currentCrowd!=NULL){
		currentThread = currentCrowd->element->head;
		while(currentThread!=NULL){
			if(currentThread->id == pthread_self()) {
				if(currentThread == currentCrowd->element->tail && currentThread == currentCrowd->element->head) { //only element
					currentCrowd->element->tail = NULL;
					currentCrowd->element->head = NULL;
				} else if(currentThread->prev == NULL) { 	//at the head
					currentThread->next->prev = NULL;
					currentCrowd->element->head = currentThread->next;
				} else if(currentThread->next == NULL) { 	//at the end
					currentThread->prev->next = NULL;
					currentCrowd->element->tail = currentThread->prev;
				} else{						//remove from middle
					currentThread->prev->next = currentThread->next;
					currentThread->next->prev = currentThread->prev;
				}
				free(currentThread);
				currentCrowd->element->size = currentCrowd->element->size -1;
				break;
			}		
			currentThread = currentThread->next;
		}
		currentCrowd = currentCrowd->next;
	}
	
	//release serializer
	serial_dequeue(s);

	return 0;
}




