//
// Telemetry Messgae FIFO
//
#include "Sunseeker2021.h"
#include "can_FIFO.h"

void can_fifo_INIT(void)
{
	can_struct *temptestPt;
  
  temptestPt = &can0_queue.msg_fifo[0];
  can0_queue.PutPt = temptestPt;
  can0_queue.GetPt = temptestPt;
} 

int can_fifo_PUT(can_message_fifo *queue, can_struct toPut)
{
	extern volatile unsigned char can_fifo_full;

	can_struct *tempPt;
  
  tempPt = queue->PutPt;
  *(tempPt++) = toPut;
  if(tempPt == &queue->msg_fifo[msg_fifo_size])
  {
    tempPt = &queue->msg_fifo[0];	//need to wrap around
  }
  if(tempPt == queue->GetPt)
  {
	can_fifo_full = TRUE;
    return(0);		//Failure FIFO Full
  }
  else
  {
    queue->PutPt = tempPt;
    can_fifo_full = FALSE;
    return(1);		//successful
  }
}

int can_fifo_GET(can_message_fifo *queue, can_struct *toGet)
{
	extern volatile unsigned char can_fifo_full;

  if(queue->PutPt == queue->GetPt)
  {
	  can_fifo_full = FALSE;
	  return(0);	//failure FIFO empty
  }
  else
  {
    *toGet = *(queue->GetPt++);
    if(queue->GetPt == &queue->msg_fifo[msg_fifo_size])
    {
      queue->GetPt = &queue->msg_fifo[0];	//need to wrap around
    }
    can_fifo_full = FALSE;
    return(1);
  }
}

int can_fifo_STAT(can_message_fifo *queue)
{
  return (queue->GetPt != queue->PutPt);
}

