/*
 *  Originally Written for TELEMETRY 2021 PCB Development
 * 
 *  Modem_UART to Modem UCA3 Interface
 *
 *  Clock and microcontroller Initialization RS232
 */

// Include files
#include "Modem_RS232.h"

#include <msp430x54xa.h>
#include "Sunseeker2021.h"

/*********************************************************************************/
// Modem_UART to Modem UCA3 Interface (voltage isolated)
/*********************************************************************************/
void Modem_UART_init(void)
{
    UCA3CTL1 |= UCSWRST | UCSSEL_2;	//put state machine in reset & SMCLK
    UCA3CTL0 |= UCMODE_0;
    UCA3BRW = (int)(SMCLK_RATE/MODEM_BR1);	//10MHZ/9600
    UCA3MCTL |= UCBRF_0 + (char)MODEM_UCBRS1; //modulation UCBRSx=computed from baud rate UCBRFx=0
	
	UCA3IFG &= ~UCTXIFG;			//Clear Xmit and Rec interrupt flags
	UCA3IFG &= ~UCRXIFG;
	UCA3CTL1 &= ~UCSWRST;			// initalize state machine
	UCA3ABCTL |= UCABDEN;			// automatic baud rate

//  UCA3IE |= UCRXIE|UCTXIE;		//enable TX & RX interrupt
}

/*********************************************************************************/
// Special MODEM routines
/*********************************************************************************/
void MODEM_command_puts(char data[])
{
  int ii;
  Modem_UART_puts(MODEMCmd);
  for (ii=0;ii<0x100;ii++) delay();

  Modem_UART_puts(data);
  // Note: after the completion of the data send
  // an "OK" is sent of the command is executed
  // an "ERROR" is sent if there is an error
}



/*********************************************************************************/
// Typical polling based getchr & gets and putchr & puts
/*********************************************************************************/
void Modem_UART_putchar(char data)
{
    while((UCA3IFG & UCTXIFG) == 0);
    UCA3TXBUF = data; 
}

unsigned char Modem_UART_getchar(void)
{
	char i = 100; //avoid infinite loop

    while((UCA3IFG & UCRXIFG) == 0 && i != 0)
    {
    	i--;
    }
    return(UCA3RXBUF);
}

int Modem_UART_puts(char *str)
{
	int i;
    char ch;
    i = 0;

    while((ch=*str)!= '\0')
    {
    	Modem_UART_putchar(ch);
    	str++;
    	i++;
    }
    
    return(i);
}

int Modem_UART_gets(char *ptr)
{
    int i;
    i = 0;
    while (1) {
          *ptr = Modem_UART_getchar();
          if (*ptr == 0x0D){
             *ptr = 0;
             return(i);
          }
          else 
          {
          	ptr++;
          	i++;
          }
     }
}

/*********************************************************************************/
// Typical interrupt based puts
/*********************************************************************************/

void Modem_UART_puts_int(void)
{
    extern char *Modem_TX_ptr;
    extern char put_status_MODEM;
    char ch;
    
    ch = *Modem_TX_ptr++;

	if (ch == '\0')
	{
		UCA3IE &= ~UCTXIE;
		put_status_MODEM = FALSE;
	}
	else
	{
		UCA3TXBUF = ch;
		UCA3IE |= UCTXIE;
		put_status_MODEM = TRUE;
	}
}

