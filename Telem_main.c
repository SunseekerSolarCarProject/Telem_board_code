//
// Telemetry
//
//
/* New code 2021 B. Bazuin
 * - Can0 functional
 * - UART - RS232 functional (may be RX TX reversed?)
 * - RTC IC function with polling (jumper wire no battery)
 * - I2C interrupt driven operation
 *
 * - Operational to perform required functions for Telemetry in 2021
 * - can is placed in a 16 message queue
 * - can from queue decoded into telemetry messages
 * - messages are periodically transmitted as commanded
 * - interrupt driven UART transmission
 *
 */

#include "Sunseeker2021.h"

// structures
can_message_fifo can0_queue;
//char_fifo USB_FIFO, MODEM_FIFO;

can_struct TX_can0_message;
can_struct can_MPPT;

hf_packet pckHF;
lf_packet pckLF;
status_packet pckST;

unsigned volatile char forceread;

unsigned int can_mask0, can_mask1;
unsigned char can_CANINTF, can0_FLAGS[3];

enum MODE {INIT, DECODE, MODEMTX, USBTX, LOWP, LOOP} ucMODE;
unsigned volatile char can_status_test, can_rcv_status_test;
unsigned long can_msg_count = 0, can_stall_cnt = 0, can_no_int_cnt=0;
unsigned long can_err_count = 0, can_read_cnt = 0;

char CAN0_INT_FLAG = FALSE;
char CAN1_INT_FLAG = FALSE;

unsigned char byear, bmonth, bdate, bwkday, bhrs, bmin, bsec; //MCP7940M format
int thrs, tmin, tsec; // BCD representation
unsigned char i2c_RX, i2c_TX;
unsigned char *PTxData;
unsigned char I2CTXByteCtr;
char get_status_RTCIC = FALSE;
char end_RTCIC_RX = FALSE;

char ucFLAG;

volatile unsigned char status_flag = FALSE;
volatile unsigned char hs_comms_flag = FALSE;
volatile unsigned char ls_comms_flag = FALSE;
volatile unsigned char st_comms_flag = FALSE;
volatile unsigned char mppt_comm_flag = FALSE;
volatile unsigned char AC_comm_flag = FALSE;


static char init_msg_data[21] = "0xHHHHHHHH,0xHHHHHHHH";
//static char init_time_msg[17] = "TL_TIM,HH:MM:SS\r\n";

char time_test_msg[18] = "TL_TIM,HH:MM:SS\r\n\0";
char cr_lf_msg[3] = "\r\n\0";

// General Variables

// CAN0 Communication Variables
volatile unsigned char cancomm_flag = FALSE;	//used for CAN transmission timing
volatile unsigned char send_can = FALSE;	//used for CAN transmission timing
volatile unsigned char rcv_can = FALSE;	//used for CAN transmission timing
volatile unsigned char can_full = FALSE;	//used for CAN transmission status
volatile unsigned char can_fifo_full = FALSE;	//used for CAN transmission status
int can0_comms_event_count = 0;

// CAN1 Communication Variables
unsigned int mppt_can1_rx_cnt, mppt_can1_tx_cnt;
unsigned int can1_buf_addr[3] = {0xFFFF, 0xFFFF, 0xFFFF};

//Modem_RS232 Variables
char put_status_MODEM = FALSE;
char end_Modem_TX = FALSE;
char *Modem_TX_ptr;

char command[32];								//stores rs232 commands
char buff[32];									//buff array to hold sprintf string
unsigned char modem_count = 0;					//counts characters received
char modem_var1[32] = "var1\r";					//command to read batt temperatures
char modem_var1_status = 0;						//TRUE if battery temps is sent
char modem_var2[32] = "var2\r";					//command to read cell volts
char modem_var2_status = 0;						//TRUE if battery volts is sent

//Modem_USB Variables
char put_status_USB = FALSE;
char end_USB_TX = FALSE;
char *USB_TX_ptr;

// MPPT Variables
unsigned char mppt1_turn_on  = FALSE;
unsigned char mppt1_turn_off = FALSE;
unsigned char mppt2_turn_on  = FALSE;
unsigned char mppt2_turn_off = FALSE;
unsigned char MPPT_test_cnt = 0x00;
unsigned char main_comm_cnt = 0x00;

unsigned int mppt_av[2];
unsigned int mppt_ac[2];
unsigned int mppt_bv[2];
unsigned int mppt_temp[2];
unsigned char mppt_status[2];

unsigned int mppt_bsum,max_mppt_temp;
unsigned int mppt_av_avg[2];
unsigned int mppt_ac_avg[2];

volatile float bat_voltage;

int main(void) {
	int ii;

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	_DINT();     		    	//disables interrupts

	ucMODE = INIT;
	ucFLAG = 0x80;

	clock_init();								//Configure HF and LF clocks
	delay();

	io_init();
	delay();

	timerB_init();	//init timer B
	delay();

	// Setup on-chip RTC
	init_RTC();
	delay();

	// Setup USB port and Modem
	Modem_USB_init();
	delay();

	// Setup RS232 port and Modem
	Modem_UART_init();
	delay();

    MODEM_command_puts(RFModemH);
    Modem_UART_puts(cr_lf_msg);
	delay();

	// Prepare and deliver Modem tets message
	getRTCTime(&thrs,&tmin,&tsec);
	delay();

	insert_time(&time_test_msg[0]);
	Modem_UART_puts(time_test_msg);
	delay();

	Modem_USB_puts(time_test_msg);
	delay();

	// MSP430 RTC init
	init_i2c();
	delay();

	init_MCP7940M();
	delay();

	get_MCP7940M(&bhrs,&bmin,&bsec);
	thrs = bhrs;
	tmin = bmin;
	tsec = bsec;;
	setRTChms(thrs,tmin,tsec);

	delay();
	insert_time_2(&time_test_msg[0]);
	Modem_UART_puts(time_test_msg);
	delay();

	get_MCP7940M(&bhrs,&bmin,&bsec);
	delay();
	insert_time_2(&time_test_msg[0]);
	Modem_UART_puts(time_test_msg);
	delay();

	get_MCP7940M(&bhrs,&bmin,&bsec);
	delay();
	insert_time_2(&time_test_msg[0]);
	Modem_UART_puts(time_test_msg);
	delay();

	get_MCP7940M(&bhrs,&bmin,&bsec);
	delay();
	insert_time_2(&time_test_msg[0]);
	Modem_UART_puts(time_test_msg);
	delay();

	// Set up both Can Controllers
	can0spi_init();
	delay();

	can0_init();
	delay();

	can1spi_init();
	delay();

	can1_init();
	delay();

    while(1)
    {
    	if(ucMODE != INIT){
            // The following is to recover after a breakpoint
    		// Based on timing, it happens periodically.
            CAN0_INT_FLAG = ((P2IN & CAN0_INTn)==0);

            if(CAN0_INT_FLAG){
            	CAN0_INT_FLAG = FALSE;
            	can0_flag_check();	//could read CANSTAT instead
            	if(can_CANINTF& 0x03){
            		can_stall_cnt++;
            		can0_receive();
            		can_no_int_cnt++;
            	}

            }

            if (can_fifo_STAT(&can0_queue)){
        		ucMODE = DECODE;
            }
            else
            	 ucMODE = LOOP;

    	}

        switch (ucMODE)
        {
          case (INIT):
		    can_fifo_INIT();
            packet_init();
            pckHF.msg_filled = 0;
            pckLF.msg_filled = 0;
            pckST.msg_filled = 0;

            can0_init();
            can1_init();

		    for(ii=0;ii<=2;ii++) mppt_av[ii]=0;
		    for(ii=0;ii<=2;ii++) mppt_av_avg[ii]=0;
		    for(ii=0;ii<=2;ii++) mppt_ac[ii]=0;
		    for(ii=0;ii<=2;ii++) mppt_ac_avg[ii]=0;
		    for(ii=0;ii<=2;ii++) mppt_bv[ii]=0;
		    for(ii=0;ii<=2;ii++) mppt_temp[ii]=0;

            P2IES = CAN0_INTn;
            P2IE  = CAN0_INTn;	// Enable can0 Interrupts

		    ucMODE = LOOP;
		    WDTCTL = WDT_ARST_1000; 	// Start watchdog timer to prevent time out reset
			_EINT(); 	//enable global interrupts
			__no_operation();			//for compiler

		    break;
          case (DECODE):
		    decode();
            if(can_fifo_STAT(&can0_queue)){
            	ucFLAG |= 0x20;  //if queue is not empty then continue to decode
            }
            else {
            	ucMODE = LOOP;
            }
		    break;
          case (MODEMTX):

		    break;
          case (LOOP):

		    break;
          default:

        	break;

        }


        if(can_fifo_full){
    		P8OUT ^= BIT4;                          // Toggle LED
    		can_fifo_INIT();
        }

        if(end_Modem_TX){
        	end_Modem_TX = FALSE;
            pckHF.msg_filled = 0;
            hs_comms_flag = FALSE;
            for(ii =0;ii<HF_MSG_PACKET;ii++){
            	strncpy(&pckHF.xmit[ii].message[7],init_msg_data,21);
            }

        }

    	if(status_flag){
    		status_flag = FALSE;
    		P8OUT ^= BIT3;                          // Toggle LED

    		//get_MCP7940M(&bhrs,&bmin,&bsec);
    		get_MCP7940M_int();
    		insert_time_2(&pckHF.timexmit.time_msg[0]);

        	getRTCTime(&thrs,&tmin,&tsec);
            //insert_time(&pckHF.timexmit.time_msg[0]);

            Modem_TX_ptr = &pckHF.prexmit.pre_msg[0]; 	// set TX pointer
            Modem_UART_puts_int();						// Start int modem TX

   			// Transmit our ID frame at a slower rate (every 10 events = 1/second)
    		can0_comms_event_count++;
    		if(can0_comms_event_count >=0)
    		{
    			can0_comms_event_count = 0;
    			TX_can0_message.address = TM_CAN_BASE;
    			TX_can0_message.data.data_u8[7] = 'T';
    			TX_can0_message.data.data_u8[6] = 'M';
    			TX_can0_message.data.data_u8[5] = 'v';
    			TX_can0_message.data.data_u8[4] = '1';
    			TX_can0_message.data.data_u32[0] = DEVICE_SERIAL;
    			can0_transmit();
   			}

    	}  // End periodic communications

    	if(mppt_comm_flag == TRUE){
    		mppt_comm_flag = FALSE;

    		switch(MPPT_test_cnt){
    			case 0x00:
    				//MPPT CAN RTR Send
    				can_MPPT.address = MPPT_CAN_BASE + MPPT_CAN_ADDRESS1;
    				mppt_status[0] |= 0x02;
    				break;
    			case 0x01:
    				//MPPT CAN RTR Send
    				can_MPPT.address = MPPT_CAN_BASE + MPPT_CAN_ADDRESS2;
    				mppt_status[1] |= 0x02;
    				break;
   			}

    		can1_sendRTR(); //Send RTR request

    		MPPT_test_cnt++;
    		if(MPPT_test_cnt >= 0x02) MPPT_test_cnt = 0x00;

    	}

    	/*Check for CAN packet reception on CAN_MPPT (Polling)*/
    	if((P2IN & CAN1_INTn) == 0x00)
    	{    //IRQ flag is set, so run the receive routine to either get the message, or the error
    		can1_receive();
    	    // Check the status
    	    // Modification: case based updating of actual current and velocity added
    	    // - messages received at 5 times per second 16/(2*5) = 1.6 sec smoothing
    	    if(can_MPPT.status == CAN_OK)
    	    {
    	        switch(can_MPPT.address){
    	      	    case MPPT_CAN_BASE + MPPT_CAN_ADDRESS1:
    					mppt_av[0] = can_MPPT.data.data_u16[0];
    					mppt_ac[0] = can_MPPT.data.data_u16[1];
    					mppt_bv[0] = can_MPPT.data.data_u16[2];
    					mppt_temp[0] = can_MPPT.data.data_u16[3];

    					mppt_av_avg[0] = 15 * mppt_av_avg[0] + mppt_av[0];
    					mppt_av_avg[0] = mppt_av_avg[0]>>4;
    					mppt_ac_avg[0] = 15*mppt_ac_avg[0] + mppt_ac[0];
    					mppt_ac_avg[0] = mppt_ac_avg[0]>>4;

    					if(mppt_temp[0]>7000) {
    						mppt_status[0] |= 0x10;
    					}

    			   		mppt_status[0] &= ~(0x02);
    			   		break;
    	    		case MPPT_CAN_BASE + MPPT_CAN_ADDRESS2:
    		   			mppt_av[1] = can_MPPT.data.data_u16[0];
    		   			mppt_ac[1] = can_MPPT.data.data_u16[1];
    		   			mppt_bv[1] = can_MPPT.data.data_u16[2];
    		   			mppt_temp[1] = can_MPPT.data.data_u16[3];

    			   		mppt_av_avg[1] = 15*mppt_av_avg[1] + mppt_av[1];
    			   		mppt_av_avg[1] = mppt_av_avg[1]>>4;
    			   		mppt_ac_avg[1] = 15*mppt_ac_avg[1] + mppt_ac[1];
    			   		mppt_ac_avg[1] = mppt_ac_avg[1]>>4;

    			   		if(mppt_temp[1]>7000) {
    			   			mppt_status[1] |= 0x10;
    			   		}
    			   		mppt_status[1] &= ~(0x02);
    	    	   	    break;
    	    	}
    	    }
    	    if(can_MPPT.status == CAN_RTR)
    	    {
    	       	//do nothing
    	    	P8OUT ^= BIT6;                          // Toggle LED
    	    }
    	    if(can_MPPT.status == CAN_ERROR)
    	    {
    	    	P8OUT ^= BIT6;                          // Toggle LED
    	    }
    	}

    	if(AC_comm_flag == TRUE){
    		AC_comm_flag = FALSE;

    		mppt_bsum = mppt_bv[0] + mppt_bv[1];
    		bat_voltage = (float) mppt_bsum;
			bat_voltage /= 200.0;

    		// Transmit shunt current
    		TX_can0_message.address = AC_CAN_BASE + AC_ISH;
    		TX_can0_message.data.data_fp[1] = 0.0;
    		TX_can0_message.data.data_fp[0] = bat_voltage;
			can0_transmit();

			switch(main_comm_cnt)
			{
			case 0x00:
			case 0x03:
			case 0x06:
    		// Transmit MPPT 1 Info
				TX_can0_message.address = AC_CAN_BASE + AC_M1;
	    		TX_can0_message.data.data_fp[1] = (float) mppt_av_avg[0];
	    		TX_can0_message.data.data_fp[0] = (float) mppt_ac_avg[0];
				can0_transmit();

				// Transmit MPPT 2 Info
				TX_can0_message.address = AC_CAN_BASE + AC_M2;
	    		TX_can0_message.data.data_fp[1] = (float) mppt_av_avg[1];
	    		TX_can0_message.data.data_fp[0] = (float) mppt_ac_avg[1];
				can0_transmit();

/*
	    		// Transmit MPPT 1 Info
				can_MAIN.address = AC_CAN_BASE + AC_M1;
	    		can_MAIN.data.data_u16[3] = mppt_temp[0];
	    		can_MAIN.data.data_u16[2] = mppt_bv[0];
	    		can_MAIN.data.data_u16[1] = mppt_ac[0];
	    		can_MAIN.data.data_u16[0] = mppt_av[0];
				can0_transmit();

				// Transmit MPPT 2 Info
				can_MAIN.address = AC_CAN_BASE + AC_M2;
	    		can_MAIN.data.data_u16[3] = mppt_temp[1];
	    		can_MAIN.data.data_u16[2] = mppt_bv[1];
	    		can_MAIN.data.data_u16[1] = mppt_ac[1];
	    		can_MAIN.data.data_u16[0] = mppt_av[1];
				can0_transmit();
				*/
				break;
			case 0x01:
			case 0x04:
			case 0x07:
	    		// Transmit maximum temperatures
				max_mppt_temp = mppt_temp[0];
				if(mppt_temp[1]>=max_mppt_temp) max_mppt_temp = mppt_temp[1];

	    		TX_can0_message.address = AC_CAN_BASE + AC_TMAX;
	    		TX_can0_message.data.data_u32[1] = 0.0;
	    		TX_can0_message.data.data_fp[0] = (float) max_mppt_temp;
				can0_transmit();

				// AC-BP_Charge Information
	    		TX_can0_message.address = AC_CAN_BASE + AC_BP_CHARGE;
				TX_can0_message.data.data_u8[7] = 'A';
				TX_can0_message.data.data_u8[6] = 'C';
				TX_can0_message.data.data_u8[5] = 'v';
				TX_can0_message.data.data_u8[4] = '1';
				TX_can0_message.data.data_u32[0] = DEVICE_SERIAL;
				can0_transmit();
				break;

			case 0x02:
			case 0x05:
			case 0x08:
	    		// Transmit MPPT Temp Values
	    		TX_can0_message.address = AC_CAN_BASE + AC_TVAL1;
	    		TX_can0_message.data.data_fp[1] = (float) mppt_temp[0];
	    		TX_can0_message.data.data_fp[0] = (float) mppt_temp[1];
				can0_transmit();
				break;

			case 0x09:
	    		// Transmit Board ID
	    		TX_can0_message.address = AC_CAN_BASE;
				TX_can0_message.data.data_u8[7] = 'A';
				TX_can0_message.data.data_u8[6] = 'C';
				TX_can0_message.data.data_u8[5] = 'v';
				TX_can0_message.data.data_u8[4] = '1';
				TX_can0_message.data.data_u32[0] = DEVICE_SERIAL;
				can0_transmit();

				break;
			}

			main_comm_cnt++;
			if(main_comm_cnt == 10) main_comm_cnt = 0x00;

    	}

    	WDTCTL = WDT_ARST_1000; // Reset watchdog timer to prevent time out reset
    }  // end while(TRUE)
	return 0;
}

/*
* Initialise Timer B
*	- Provides timer tick timebase at 100 Hz
*/
void timerB_init( void )
{
  TBCTL = CNTL_0 | TBSSEL_1 | ID_3 | TBCLR;		// ACLK/8, clear TBR
  TBCCR0 = (ACLK_RATE/8/TICK_RATE);				// Set timer to count to this value = TICK_RATE overflow
  TBCCTL0 = CCIE;								// Enable CCR0 interrrupt
  TBCTL |= MC_1;								// Set timer to 'up' count mode
}

/*
* Timer B CCR0 Interrupt Service Routine
*	- Interrupts on Timer B CCR0 match at 10Hz
*	- Sets Time_Flag variable
*/
/*
* GNU interropt symantics
* interrupt(TIMERB0_VECTOR) timer_b0(void)
*/
#pragma vector = TIMER0_B0_VECTOR
__interrupt void timer_b0(void)
{
	  static unsigned int status_count = TELEM_STATUS_COUNT;
	  static unsigned int hs_comms_count = HS_COMMS_SPEED;
	  static unsigned int ls_comms_count = LS_COMMS_SPEED;
	  static unsigned int st_comms_count = ST_COMMS_SPEED;
	  static unsigned int mppt_comm_count = MPPT_COMMS_SPEED;
	  static unsigned int AC_comm_count = AC_COMMS_SPEED;

	  if (ucMODE != INIT)
	  {
	  // Trigger comms events (hs command packet transmission)
	  	hs_comms_count--;
	  	if( hs_comms_count == 0 ){
	   		hs_comms_count = HS_COMMS_SPEED;
	    	hs_comms_flag = TRUE;
	  	}

	  	ls_comms_count--;
	  	if( ls_comms_count == 0 ){
	    	ls_comms_count = LS_COMMS_SPEED;
	    	ls_comms_flag = TRUE;
	  	}

	  	st_comms_count--;
	  	if( st_comms_count == 0 ){
	    	st_comms_count = ST_COMMS_SPEED;
	    	st_comms_flag = TRUE;
	  	}

	  	mppt_comm_count--;
	  	if( mppt_comm_count == 0 ){
	  		mppt_comm_count = MPPT_COMMS_SPEED;
	    	mppt_comm_flag = TRUE;
	  	}
	  	AC_comm_count--;
	  	if( AC_comm_count == 0 ){
	  		AC_comm_count = AC_COMMS_SPEED;
	  		AC_comm_flag = TRUE;
	  	}
}

	  // Primary System Heart beat - always on
	  status_count--;
	  if( status_count == 0 )
	  {
		  status_count = TELEM_STATUS_COUNT;
		  status_flag = TRUE;
	  }

}

//------------------------------------------------------------------------------
// The i2c_B1TX_isr is structured such that it can be used to transmit any
// number of bytes by pre-loading TXByteCtr with the byte count. Also, TXData
// points to the next byte to transmit.
//------------------------------------------------------------------------------

#pragma vector=PORT2_VECTOR
__interrupt void P2_ISR(void)
{
  //int i;
  switch(__even_in_range(P2IV,16))
  {
  case 0:break;                             // Vector 0 - no interrupt
  case 2:                                   // Vector Pin 2.0 - CAN0_INTn
    can_stall_cnt++;
    can0_receive();
    P2IFG &= 0xFE;
    break;
  case 4:                                   // Vector Pin 2.1 - CAN0_RXB0n
    break;
  case 6:                                   // Vector Pin 2.2 - CAN0_RXB1n
    break;
  case 8:                                   // Vector Pin 2.3 - CAN1_INTn
	//P2IFG &= 0xF7;
    break;
  case 10:                                  // Vector Pin 2.4 - CAN1_RXB0n
    break;
  case 12:                                  // Vector Pin 2.5 - CAN1_RXB1n
    break;
  case 14:                                  // Vector Pin 2.6 - GPS_INTn
    break;
  case 16:                                  // Vector Pin 2.7 - UNUSED
    break;
  default:
    break;
  }
}

//RS232 Interrupt
#pragma vector = USCI_A3_VECTOR
__interrupt void USCI_A3_ISR(void)
{
    extern char *Modem_TX_ptr;
    extern char put_status_MODEM;
	char ch;
	int ii;

	switch(__even_in_range(UCA3IV,16))
	{
	  case 0:break;                             // no interrupt
	  case 2:                                   // UCRXIFG
		command[modem_count] = Modem_UART_getchar();		//get character
		//Modem_UART_putchar(command[modem_count]);		// put char
		if(command[modem_count] == 0x0D)				//if return
		{
			modem_count = 0x00;							//reset counter
			if(strcmp(modem_var1,command) == 0) modem_var1_status = 1;
			else if(strcmp(modem_var2,command) == 0) modem_var2_status = 1;
			else
			{
				modem_var1_status = 0;
				modem_var2_status = 0;
			}

			for(ii = 0; ii < 31; ii++)
			{
				command[ii] = 0;							//reset command
			}
//			Modem_UART_putchar(0x0A);						//new line
//			Modem_UART_putchar(0x0D);						//return
		}
		else if(command[modem_count] != 0x7F)			//if not backspace
		{
			modem_count++;
		}
		else
		{
			modem_count--;
		}
	    break;
	  case 4:                                   // UCTXIFG
		ch = *Modem_TX_ptr++;

		if (ch == '\0')
		{
			UCA3IE &= ~UCTXIE;
			put_status_MODEM = FALSE;
			end_Modem_TX = TRUE;
		}
		else
		{
			UCA3TXBUF = ch;
			UCA3IE |= UCTXIE;
			put_status_MODEM = TRUE;
		}
	  	break;
    default:
  	break;
	  }
}

/*
* I2B UCB2 Interrupt Service Routine
*/
/*
* GNU interrupt symantics
* interrupt(UCB1IV) i2c_B2_isr(void)
*/
#pragma vector = USCI_B2_VECTOR
__interrupt void i2c_B2_isr(void)
{
	unsigned char sec_raw, min_raw, hrs_raw;
	extern unsigned char *PTxData;                     // Pointer to TX data
	extern unsigned char I2CTXByteCtr;
	extern unsigned char i2c_RX;
	extern unsigned char i2c_TX;
	extern unsigned char bhrs, bmin, bsec;


	  switch(__even_in_range(UCB2IV,16))
	  {
	  case 0:break;                             // no interrupt
	  case 2:                                   // UCALIFG;
	    break;
	  case 4:                                   // UCNACKIFG
		break;
	  case 6:                                   // UCSTTIFG
		  UCB2IFG &= ~UCSTTIFG;                  // Clear start condition int flag
		break;
	  case 8:                                   // UCSTPIFG
		break;
	  case 10:                                   // UCRXIFG
		  if(i2c_RX==0x02)
		  {
			  sec_raw = UCB2RXBUF;
			  bsec = sec_raw & 0x7F;
			  i2c_RX++;
		  }
		  else if(i2c_RX==0x03)
		  {
		      min_raw = UCB2RXBUF;
		      bmin = min_raw;
			  i2c_RX++;
		      UCB2CTL1 |= UCTXSTP;                    // I2C stop condition
		  }
		  else if(i2c_RX==0x04)
		  {
		      hrs_raw = UCB2RXBUF;
		      bhrs = hrs_raw & 0x3F;
		      UCB2IE &= ~UCRXIE;                  // Clear USCI_B2 TX int flag
			  i2c_RX=0x00;
		  }
		break;
	  case 12:                                   // UCTXIFG;
		  if (i2c_RX==0x01)
		  {
			  UCB2CTL1 &= ~UCTR;             // I2C RX
			  UCB2CTL1 |= UCTXSTT;           // start condition
			  i2c_RX = 0x02;
			  UCB2IE &= ~UCTXIE;             // I2C TX interrupt enable

		  }
		  else if(i2c_TX)
		  {
			  if (I2CTXByteCtr)                          // Check TX byte counter
			  {
			      UCB2TXBUF = *PTxData++;               // Load TX buffer
			      I2CTXByteCtr--;                          // Decrement TX byte counter
			  }
			  else
			  {
			      UCB2CTL1 |= UCTXSTP;                  // I2C stop condition
			      UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B2 TX int flag
			      UCB2IE &= ~UCTXIE;                  // Clear USCI_B2 TX int flag
			      i2c_TX = FALSE;
			  }
 		  }
		break;
      default:
    	break;
	  }

}

