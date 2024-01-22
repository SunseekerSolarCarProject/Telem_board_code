#ifndef message_FIFO_H
#define message_FIFO_H

#ifndef msg_fifo_size
#define msg_fifo_size 16
#endif

//structure to hold incoming can messages before decoding
typedef struct _can_message_fifo
{
	can_struct msg_fifo[msg_fifo_size];
	can_struct *PutPt;
	can_struct *GetPt;
} can_message_fifo;


//public structure
extern can_message_fifo can0_queue;

//public functions
extern void can_fifo_INIT(void);
extern int can_fifo_PUT(can_message_fifo *queue, can_struct toPut);
extern int can_fifo_GET(can_message_fifo *queue, can_struct *toGet);
extern int can_fifo_STAT(can_message_fifo *queue);

#endif

