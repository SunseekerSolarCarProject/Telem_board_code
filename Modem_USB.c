/*
 *  Originally Written for TELEMETRY 2021 PCB Development
 *
 *  Modem_UART to USB UCA2 Interface
 *
 *  Clock and microcontroller Initialization USB
 */

// Include files
#include "Modem_USB.h"

#include <msp430x54xa.h>
#include "Sunseeker2021.h"

/*********************************************************************************/
// Modem_USB to Modem UCA2 Interface (voltage isolated)
/*********************************************************************************/
void Modem_USB_init(void)
{
    UCA2CTL1 |= UCSWRST | UCSSEL_3;	//put state machine in reset & SMCLK
    UCA2CTL0 |= UCMODE_0;
    UCA2BRW = (int)(SMCLK_RATE/MODEM_BR1);	//10MHZ/9600
    UCA2MCTL |= UCBRF_0 + (char)MODEM_UCBRS1; //modulation UCBRSx=computed from baud rate UCBRFx=0

	UCA2IFG &= ~UCTXIFG;			//Clear Xmit and Rec interrupt flags
	UCA2IFG &= ~UCRXIFG;
	UCA2CTL1 &= ~UCSWRST;			// initalize state machine
	UCA2ABCTL |= UCABDEN;			// automatic baud rate

//  UCA2IE |= UCRXIE|UCTXIE;		//enable TX & RX interrupt
}

/*********************************************************************************/
// Typical polling based getchr & gets and putchr & puts
/*********************************************************************************/
void Modem_USB_putchar(char data)
{
    while((UCA2IFG & UCTXIFG) == 0);
    UCA2TXBUF = data;
}

unsigned char Modem_USB_getchar(void)
{
	char i = 100; //avoid infinite loop

    while((UCA2IFG & UCRXIFG) == 0 && i != 0)
    {
    	i--;
    }
    return(UCA2RXBUF);
}

int Modem_USB_puts(char *str)
{
	int i;
    char ch;
    i = 0;

    while((ch=*str)!= '\0')
    {
    	Modem_USB_putchar(ch);
    	str++;
    	i++;
    }

    return(i);
}

int Modem_USB_gets(char *ptr)
{
    int i;
    i = 0;
    while (1) {
          *ptr = Modem_USB_getchar();
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

void Modem_USB_puts_int(void)
{
    extern char *USB_TX_ptr;
    extern char put_status_USB;
    char ch;

    ch = *USB_TX_ptr++;

	if (ch == '\0')
	{
		UCA2IE &= ~UCTXIE;
		put_status_USB = FALSE;
	}
	else
	{
		UCA2TXBUF = ch;
		UCA2IE |= UCTXIE;
		put_status_USB = TRUE;
	}
}



