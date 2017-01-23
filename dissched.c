#include "ds_serial.h"
// Authors: Priyanka Gomatam, Ikenna Okafor
void Init_ds(int disk_size, float seek_time){
	if(seek_time >= 0 || disk_size > 0 ) {
		serial = Create_Serial();
		rec.upqueue = Create_Queue(serial);
		rec.downqueue = Create_Queue(serial);
		rec.crowd = Create_Crowd(serial);
		rec.current=0;
		rec.direction=1;
		seqNo=1;
	} else {
		printf("Please input a disk size  greater than 0 and a seek time greater than or equal to 0\n");
		exit(1);
	}
}

int Disk_Request(int CylNo, void* (*func)(), int* SeekedCylinders, int id) {
int priority;

	Serial_Enter(serial);		//someone gets the serializer
	if(CylNo > rec.current || (CylNo == rec.current && rec.direction==0)) {
		priority =  CylNo - rec.current;
		Serial_Enqueue_ds(serial, rec.upqueue, condition1, priority);
	} else if (CylNo < rec.current || (CylNo == rec.current && rec.direction==1)) {
		priority = rec.current - CylNo;
		Serial_Enqueue_ds(serial, rec.downqueue, condition2, priority);
	}
	
	//set number of cylinders from head
	SeekedCylinders[id] = abs(CylNo - rec.current);
	
	Serial_Join_Crowd(serial, rec.crowd, NULL);
	
	//call thread routine
	func(id, SeekedCylinders[id]);
	
	//change current head position
	rec.current = CylNo;
	
	//check to see if we should start going the other way
	release(id);
	
	Serial_Exit(serial);
	seqNo++;
	return (seqNo-1);
}

cond_t condition1() {
	
	return(Crowd_Empty(serial, rec.crowd) && (Queue_Empty(serial,rec.downqueue)  ||  rec.direction==1));
}

cond_t condition2() {
	
	return(Crowd_Empty(serial, rec.crowd) && (Queue_Empty(serial,rec.upqueue)  ||  rec.direction==0));
}

cond_t up_is_done() {
	
	return(Queue_Empty(serial,rec.upqueue)  &&  rec.direction==1);
}

cond_t down_is_done() {
	
	return (Queue_Empty(serial,rec.downqueue)  &&  rec.direction==0);
}

void release(int a) {
	if(up_is_done()) {
		 rec.direction = 0;
	} else if(down_is_done()) {
		 rec.direction=1;
	}
}

