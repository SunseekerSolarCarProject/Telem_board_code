#ifndef DECODE_PACKET_H
#define DECODE_PACKET_H

#define HF_MSG_PACKET  9    //number of messages per packet in high frequency
#define LF_MSG_PACKET  0   //number of messages per packet in low frequency
#define ST_MSG_PACKET  0    //number of messages that we receive and don't send out
#define No_MSG_PACKET  25    //number of messages that we receive and don't send out
#define LOOKUP_ROWS HF_MSG_PACKET+LF_MSG_PACKET+ST_MSG_PACKET+No_MSG_PACKET
#define NAME_LOOKUP_ROWS LOOKUP_ROWS
//#define TIME_SIZE 30        //number of characters in time
#define MSG_SIZE  30        //number of characters in single message

// char xmit[638];
//char hf_flash[638] = "ABCDE\r\nTIME MO/DY/YEAR HH:MM.SS    /r/nMC_LIM,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_BUS,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_VEL,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_PHA,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_VVC,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_IVC,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_BEM,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_RL1,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_RL2,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_FAN,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_TP1,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_TP2,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_TP3,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_CML,0xZZZZZZZZ,OxZZZZZZZZ\r\nDC_CML,0xZZZZZZZZ,OxZZZZZZZZ\r\nDC_DRV,0xZZZZZZZZ,OxZZZZZZZZ\r\nDC_PWR,0xZZZZZZZZ,OxZZZZZZZZ\r\nDC_RET,0xZZZZZZZZ,OxZZZZZZZZ\r\nDC_SWT,0xZZZZZZZZ,OxZZZZZZZZ\r\n\0";
//char lf_flash[638] = "ABCDE\r\nTIME MO/DY/YEAR HH:MM.SS    /r/nMC_LIM,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_BUS,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_VEL,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_PHA,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_VVC,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_IVC,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_BEM,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_RL1,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_RL2,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_FAN,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_TP1,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_TP2,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_TP3,0xZZZZZZZZ,OxZZZZZZZZ\r\nMC_CML,0xZZZZZZZZ,OxZZZZZZZZ\r\nDC_CML,0xZZZZZZZZ,OxZZZZZZZZ\r\nDC_DRV,0xZZZZZZZZ,OxZZZZZZZZ\r\nDC_PWR,0xZZZZZZZZ,OxZZZZZZZZ\r\nDC_RET,0xZZZZZZZZ,OxZZZZZZZZ\r\nDC_SWT,0xZZZZZZZZ,OxZZZZZZZZ\r\n\0";


typedef struct _pre_message
{
  char pre_msg[8];// = "ABCDEF\r\n";
} pck_pre_message;

typedef struct _time_message
{
  char time_msg[17];// = "TL_TIM,HH:MM:SS\r\n";
} pck_time_message;

typedef struct _message
{
  char message[MSG_SIZE];// = "XXXXXX,0x00000000,0x00000000\r\n";
} pck_message;


typedef struct _post_message
{
  char post_msg[9];// = "UVWXYZ\r\n\0";
} pck_post_message;

typedef struct _hf_packet
{
  unsigned int msg_filled;                            //each bit represents a msg that needs to be filled 1-filled 0-empty
  pck_pre_message prexmit;
  pck_message xmit[HF_MSG_PACKET];                //character array to be sent to modem/USB
  pck_time_message timexmit; 
  pck_post_message postxmit; 
} hf_packet;

typedef struct _lf_packet
{
  unsigned int msg_filled;                            //each bit represents a msg that needs to be filled 1-filled 0-empty
  pck_pre_message prexmit;
  pck_message xmit[LF_MSG_PACKET];                //character array to be sent to modem/USB
  pck_time_message timexmit; 
  pck_post_message postxmit; 
} lf_packet;
      
typedef struct _status_packet
{
  unsigned int msg_filled;                            //each bit represents a msg that needs to be filled 1-filled 0-empty
  pck_pre_message prexmit;
  pck_message xmit[ST_MSG_PACKET];                //character array to be sent to modem/USB
  pck_time_message timexmit; 
  pck_post_message postxmit; 
} status_packet;
      
typedef struct _no_packet
{
  unsigned int msg_filled;                            //each bit represents a msg that needs to be filled 1-filled 0-empty
  pck_pre_message prexmit;
  pck_message xmit[No_MSG_PACKET];                //character array to be sent to modem/USB
  pck_time_message timexmit; 
  pck_post_message postxmit; 
} no_packet;
   
#endif
