/*
 * - Implements the following UCA0 and UCA1 interface functions
 *	- init
 *	- transmit
 *	- exchange
 *
 */

// Include files
#include<msp430x54xa.h>

/*
 * Initialise UCA0 port can0
 */

// Public Function prototypes
void can0spi_init(void)
{
	UCA0CTL1 |= UCSWRST;					//software reset
	UCA0CTL0 |= UCCKPH | UCMSB | UCMST | UCMODE_0 | UCSYNC; //data-capt then change; MSB first; Master; 3-pin SPI; sync
	UCA0CTL1 |= UCSSEL_2;	//set SMCLK
	UCA0BR0 = 0x01;			//set clk prescaler to 1 - SM_CLK transfer rate
	UCA0BR1 = 0x00;
	UCA0STAT = 0x00;		//not in loopback mode
	UCA0CTL1 &= ~UCSWRST;	//SPI enable turn off software reset
	// UCA0IE |= UCTXIE | UCRXIE;		// Interrupt Enable
}

/*
* Transmits data on UCA0 connection
*	- Busy waits until entire shift is complete
*/
void can0spi_transmit(unsigned char data)
{
  //unsigned char forceread;
  UCA0TXBUF = data;
  while((UCA0IFG & UCRXIFG) == 0x00);	// Wait for Rx completion (implies Tx is also complete)
  //forceread = UCA0RXBUF;
}

/*
* Exchanges data on UCA0 connection
*	- Busy waits until entire shift is complete
*	- This function is safe to use to control hardware lines that rely on shifting being finalised
*/
unsigned char can0spi_exchange(unsigned char data)
{
  UCA0TXBUF = data;
  while((UCA0IFG & UCRXIFG) == 0x00);	// Wait for Rx completion (implies Tx is also complete)
  return(UCA0RXBUF);
}

// Can1 SPI

/*
 * Initialise UCA1 port can1
 */
 
// Public Function prototypes
void can1spi_init(void)
{
	UCA1CTL1 |= UCSWRST;					//software reset
	UCA1CTL0 |= UCCKPH | UCMSB | UCMST | UCMODE_0 | UCSYNC; //data-capt then change; MSB first; Master; 3-pin SPI; sync
	UCA1CTL1 |= UCSSEL_2;	//set SMCLK
	UCA1BR0 = 0x01;			//set clk prescaler to 1
	UCA1BR1 = 0x00;
	UCA1STAT = 0x00;		//not in loopback mode
	UCA1CTL1 &= ~UCSWRST;	//SPI enable turn off software reset
	// UCA1IE |= UCTXIE | UCRXIE;		// Interrupt Enable
}

/*
* Transmits data on UCA1 connection
*	- Busy waits until entire shift is complete
*/
void can1spi_transmit(unsigned char data)
{
  //unsigned char forceread;
  UCA1TXBUF = data;
  while((UCA1IFG & UCRXIFG) == 0x00);	// Wait for Rx completion (implies Tx is also complete)
  //forceread = UCA1RXBUF;
}

/*
* Exchanges data on UCA1 connection
*	- Busy waits until entire shift is complete
*	- This function is safe to use to control hardware lines that rely on shifting being finalised
*/
unsigned char can1spi_exchange(unsigned char data)
{
  UCA1TXBUF = data;
  while((UCA1IFG & UCRXIFG) == 0x00);	// Wait for Rx completion (implies Tx is also complete)
  return(UCA1RXBUF);
}
 

