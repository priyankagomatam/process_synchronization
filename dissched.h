#include "serial.h"
#include <math.h>

int cylinderseektime;
serial_t *serial;
typedef struct record_t {
	queue_t *upqueue, *downqueue;
	crowd_t *crowd;
	int current;
	int direction;  //1 for up 0 for down
} record_t;

record_t rec;
int seqNo;

void Init_ds(int, float);

int Disk_Request(int a, void*(*func)(), int* b, int c);

cond_t condition1();

cond_t condition2();

cond_t up_is_done();

cond_t down_is_done();

void release(int);

