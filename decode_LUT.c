//
// Decode and Lookup Table Routines
//

#include "Sunseeker2021.h"

extern hf_packet pckHF;
extern lf_packet pckLF;
extern status_packet pckST;

extern unsigned int can_mask0, can_mask1; //Mask 0 should always be the lower value(higher priority)

static char init_pre_msg[8] = "ABCDEF\r\n";
static char init_msg[30] = "XXXXXX,0xHHHHHHHH,0xHHHHHHHH\r\n";
static char init_time_msg[17] = "TL_TIM,HH:MM:SS\r\n";
static char init_post_msg[9] = "UVWXYZ\r\n\0";

#define priority(row) addr_lookup[row][4]
#define address(row) addr_lookup[row][0]

unsigned int lookup_next(int pri);

void decode()
{
  int offset;
  int position;
  int pck;
  int row;
  int i;
  char a0, a1, dflag;
  static can_struct current;
  static pck_message *xmit_string;
  
  dflag = 0x00;
  
  if(can_fifo_GET(&can0_queue, &current))
  {
    if(lookup(current.address, &offset, &position, &pck, &row))
    {
      //if(row == can_mask1) can_mask1 = lookup_next(priority(row)); 
      //else if(row == can_mask0) {
      //	can_mask0 = can_mask1;
      //	can_mask1 = lookup_next(priority(can_mask0));
      //} else {
      	//shouldn't happen!
      //}
      //can1_sources(address(can_mask0), address(can_mask1));    	
   
      if(pck == 0)
      {
        if( (pckHF.msg_filled & position) == 0)
        {
          xmit_string = &(pckHF.xmit[offset]);
          pckHF.msg_filled |= position;
          dflag = 0xFF;
        }
      }
      else if(pck == 1)
      {  
        if( (pckLF.msg_filled & position) == 0)
        {
          xmit_string = &(pckLF.xmit[offset]);
          pckLF.msg_filled |= position;
          dflag = 0xFF;
        }
      }
      else if(pck == 2)
      {  
        if( (pckST.msg_filled & position) == 0)
        {
          xmit_string = &(pckST.xmit[offset]);
          pckST.msg_filled |= position;
          dflag = 0xFF;
        }
      }
      else
      {
        dflag = 0x00;
      }
      
      if (dflag == 0xFF)
      {
        for(i=0;i<4;i++)
        {
          a1 = ((current.data.data_u8[i]>>4) & 0x0F)+'0';
          if (a1 > '9')
            a1 = a1 - '0' - 0x0a + 'A';
          a0 = (current.data.data_u8[i] & 0x0F)+'0';
          if (a0 > '9')
            a0 = a0 - '0' - 0x0a + 'A';
          xmit_string->message[2*i+9]=a1;
          xmit_string->message[2*i+10]=a0;
        }
        for(i=0;i<4;i++)
        {
          a1 = ((current.data.data_u8[i+4]>>4) & 0x0F)+'0';
          if (a1 > '9')
            a1 = a1 - '0' - 0x0a + 'A';
          a0 = (current.data.data_u8[i+4] & 0x0F)+'0';
          if (a0 > '9')
            a0 = a0 - '0' - 0x0a + 'A';
          xmit_string->message[2*i+20]=a1;
          xmit_string->message[2*i+21]=a0;
        }
      }
    }
  }
}

unsigned int lookup_next(int pri)
{
	int row;
	pri++;
	
	for(row = 0; row < sizeof(lut_blacklist); row++)
	{
		if(pri == lut_blacklist[row]) pri++;
		if(pri == LOOKUP_ROWS) pri = 0;
	}
	
	for(row = 0; row < LOOKUP_ROWS; row++)
		if(addr_lookup[row][4] == pri) break;
		
	if(row == LOOKUP_ROWS) row = 0;
	
	return row; 
}

//void start_mask()
//{
//  can_mask0 = lookup_next(-1);
//  can_mask1 = lookup_next(0);
//  can0_sources(address(can_mask0), address(can_mask1));
//}

int lookup(unsigned int address, int *off, int *pos, int *pck, int *row)
{ 
  for(*row = 0; *row < LOOKUP_ROWS;(*row)++)
  {
    if(address == addr_lookup[*row][0])
    {
      *off = addr_lookup[*row][1];
      *pos = addr_lookup[*row][2];
      *pck = addr_lookup[*row][3];
      return 1;   //found address
    }
  }
  *row = 0;
 return 0;         //search failed 
}

/*************************************************************
/ Name: packet_init
/ IN: global pckHF, pckLH, pckST
/ OUT:  void
/ DESC:  This function is used to fill the packets
************************************************************/
void packet_init(void)
{
int i, pck_type, pck_offset;

  strncpy(pckHF.prexmit.pre_msg,init_pre_msg,8);
  for( i =0;i<HF_MSG_PACKET;i++){
    strncpy(pckHF.xmit[i].message,init_msg,MSG_SIZE);
  }
  strncpy(pckHF.timexmit.time_msg,init_time_msg,17);
  strncpy(pckHF.postxmit.post_msg,init_post_msg,9);
  
  strncpy(pckLF.prexmit.pre_msg,init_pre_msg,8);
  for( i =0;i<LF_MSG_PACKET;i++){
    strncpy(pckLF.xmit[i].message,init_msg,MSG_SIZE);
  }
  strncpy(pckLF.timexmit.time_msg,init_time_msg,17);
  strncpy(pckLF.postxmit.post_msg,init_post_msg,9);
  
  
  strncpy(pckST.prexmit.pre_msg,init_pre_msg,8);
  for( i =0;i<ST_MSG_PACKET;i++){
    strncpy(pckST.xmit[i].message,init_msg,MSG_SIZE);
  }
  strncpy(pckST.timexmit.time_msg,init_time_msg,17);
  strncpy(pckST.postxmit.post_msg,init_post_msg,9);
  
  for (i =0;i<LOOKUP_ROWS;i++){
    pck_type = addr_lookup[i][3];
    pck_offset =  addr_lookup[i][1];
    if(pck_type == 0){
      strncpy(pckHF.xmit[pck_offset].message,name_lookup[i],6);
    }
    else if(pck_type == 1){
      strncpy(pckLF.xmit[pck_offset].message,name_lookup[i],6);
    }
    else if(pck_type == 2){
      strncpy(pckST.xmit[pck_offset].message,name_lookup[i],6);
    }
    
  }
} 
