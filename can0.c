/*
 * Tritium MCP2515 CAN Interface
 * Copyright (c) 2006, Tritium Pty Ltd.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *	- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer 
 *	  in the documentation and/or other materials provided with the distribution.
 *	- Neither the name of Tritium Pty Ltd nor the names of its contributors may be used to endorse or promote products 
 *	  derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE. 
 *
 * Last Modified: J.Kennedy, Tritium Pty Ltd, 18 December 2006
 *
 * - Implements the following CAN interface functions
 *	- can0_init
 *	- can0_transmit
 *	- can0_receive
 *
 * Modified for tranmist errors, B. Bazuin 7/2012
 *
 */

// Include files
#include "can.h"
#include "Sunseeker2021.h"

// Public variables
can_struct			can;
extern can_struct TX_can0_message;
can_struct RX_can0_message;

extern unsigned long can_msg_count;
extern unsigned long can_err_count;
extern unsigned long can_read_cnt;


// Private variables
unsigned char 			buffer[16];

/**************************************************************************************************
 * PUBLIC FUNCTIONS
 *************************************************************************************************/

/*
 * Initialises MCP2515 CAN controller
 *	- Resets MCP2515 via SPI port (switches to config mode, clears errors)
 *	- Changes CLKOUT to /4 rate (4 MHz)
 *	- Sets up bit timing
 *      - originally 1 Mbit operation
 *      - modified for 250 kbps operation (buffer[2] setting below)
 *	- Sets up receive filters and masks
 *		- Rx Filter 0 = Motor controller velocity
 *		- Rx Filter 1 = Unused
 *		- Rx Filter 2 = Driver controls packets (for remote frame requests)
 *		- Rx Filter 3 = Unused
 *		- Rx Filter 4 = Unused
 *		- Rx Filter 5 = Unused
 *		- Rx Mask 0   = Exact message must match (all 11 bits)
 *		- Rx Mask 1   = Block address must match (upper 6 bits)
 *	- Enables ERROR and RX interrupts on IRQ pin
 *	- Switches to normal (operating) mode
 */
void can0_init( void )
{
	// Set up reset and clocking
	can0_reset();
	delay();
	can0_mod( CANCTRL, 0x03, 0x02 );			// CANCTRL register, modify lower 2 bits, CLK = /4
	
	// Set up bit timing & interrupts
	buffer[0] = 0x02;						// CNF3 register: PHSEG2 = 3Tq, No wakeup, CLKOUT = CLK
	buffer[1] = 0xC9;						// CNF2 register: set PHSEG2 in CNF3, Triple sample, PHSEG1= 2Tq, PROP = 2Tq
//	buffer[2] = 0x00;						// CNF1 register: SJW = 1Tq, BRP = 0 > 1Mbps
	buffer[2] = 0x03;						// CNF1 register: SJW = 1Tq, BRP = 3 > 250 kbps
//	buffer[3] = 0xA3;						// CANINTE register: enable MERRE, ERROR, RX0 & RX1 interrupts on IRQ pin
	buffer[3] = 0x23;						// CANINTE register: enable ERROR, RX0 & RX1 interrupts on IRQ pin
	buffer[4] = 0x00;						// CANINTF register: clear all IRQ flags
	buffer[5] = 0x00;						// EFLG register: clear all user-changable error flags
	can0_write( CNF3, &buffer[0], 6);		// Write to registers

	// Set up external interrupt pins RX_BF and RX1BF
	buffer[0] = 0x0F;			    // BFPCTRL register: enabele RXxBF as interrupts
	buffer[1] = 0x00;			    // TXRTSCTRL register: request to send TX
	can0_write(BFPCTRL, &buffer[0], 2);// Write to registers

	// Set up receive filtering & masks
	// RXF0 - Buffer 0
	buffer[ 0] = (unsigned char)((DC_CAN_BASE + DC_SWITCH) >> 3);
	buffer[ 1] = (unsigned char)((DC_CAN_BASE + DC_SWITCH) << 5);
	buffer[ 2] = 0x00;
	buffer[ 3] = 0x00;
	// RXF1 - Buffer 0
	buffer[ 4] = (unsigned char)((AC_CAN_BASE + AC_BP_CHARGE) >> 3);
	buffer[ 5] = (unsigned char)((AC_CAN_BASE + AC_BP_CHARGE) >> 3);
	buffer[ 6] = 0x00;
	buffer[ 7] = 0x00;
	// RXF2 - Buffer 1
	  buffer[ 8] = (unsigned char)((MC_CAN_BASE1) >> 3);
	  buffer[ 9] = (unsigned char)((MC_CAN_BASE1) << 5);
	buffer[10] = 0x00;
	buffer[11] = 0x00;
	can0_write( RXF0SIDH, &buffer[0], 12 );
	
	// RXF3 - Buffer 1
	buffer[ 0] = 0x00;
	buffer[ 1] = 0x00;
	buffer[ 2] = 0x00;
	buffer[ 3] = 0x00;
	// RXF4 - Buffer 1
	buffer[ 4] = 0x00;
	buffer[ 5] = 0x00;
	buffer[ 6] = 0x00;
	buffer[ 7] = 0x00;
	// RXF5 - Buffer 1
	buffer[ 8] = 0x00;
	buffer[ 9] = 0x00;
	buffer[10] = 0x00;
	buffer[11] = 0x00;
	can0_write( RXF3SIDH, &buffer[0], 12 );

	// RXM0 - Buffer 0
	//buffer[ 0] = 0xFF;						// Match entire 11 bit ID (ID is left-justified in 32-bit mask register)
	//buffer[ 1] = 0xE0;
	buffer[ 0] = 0xC0;     // Match 2 MSB of 11 bit ID (ID is left-justified in 32-bit mask register)
	buffer[ 1] = 0x00;
	buffer[ 2] = 0x00;
	buffer[ 3] = 0x00;
	// RXM1 - Buffer 1
	//buffer[ 4] = 0xFC;						// Match upper 6 bits of ID - don't care about lower 5 bits (block address)
	//buffer[ 5] = 0x00;
	buffer[ 4] = 0xC0;		//Match 2 MSB of 11 bit ID (ID is left-justified in 32-bit mask register)
	buffer[ 5] = 0x00;
	buffer[ 6] = 0x00;
	buffer[ 7] = 0x00;
	can0_write( RXM0SIDH, &buffer[0], 8 );
	
/*	buffer[0] = 0x04;	//enable filters & rollover
 *  can0_write(RXB0CTRL, &buffer[0], 1);
 *  buffer[0] = 0x04;	//enable filters
 *  can0_write(RXB1CTRL, &buffer[0], 1);
 *  */

	// Switch out of config mode into normal operating mode
//	can0_mod( CANCTRL, 0x08, 0x08 );			// CANCTRL register,One-SHot Mode for Transmission

	// Switch out of config mode into normal operating mode
	can0_mod( CANCTRL, 0xE0, 0x00 );			// CANCTRL register, modify upper 3 bits, mode = Normal
}

/*
 * Receives a CAN message from the MCP2515
 *	- Run this routine when an IRQ is received
 *	- Query the controller to identify the source of the IRQ
 *		- If it was an ERROR IRQ, read & clear the Error Flag register, and return it
 *		- If it was an RX IRQ, read the message and address, and return them
 *		- If both, handle the error preferentially to the message
 *	- Clear the appropriate IRQ flag bits
 */
void can0_receive( void )
{
	can_struct *RXPtr_can_message;
	unsigned char flags;
	
	can_msg_count++;

	// Set up pointer to receive queue

	RXPtr_can_message=&(*can0_queue.PutPt);

	// Read out the interrupt flags register
	can0_read( CANINTF, &flags, 1 );

	// Check for errors
	if(( flags & MCP_IRQ_ERR ) != 0x00 ){
		// Read error flags and counters
		can0_read( EFLAG, &buffer[0], 1 );
		can0_read( TEC, &buffer[1], 2 );
		// Clear error flags
		can0_mod( EFLAG, buffer[0], 0x00 );	// Modify (to '0') all bits that were set
		// Return error code, a blank address field, and error registers in data field
		RXPtr_can_message->status = CAN_ERROR;
		RXPtr_can_message->address = 0x0000;
		RXPtr_can_message->data.data_u8[0] = flags;		// CANINTF
		RXPtr_can_message->data.data_u8[1] = buffer[0];	// EFLG
		RXPtr_can_message->data.data_u8[2] = buffer[1];	// TEC
		RXPtr_can_message->data.data_u8[3] = buffer[2];	// REC
		// Clear the IRQ flag
		can0_mod( CANINTF, MCP_IRQ_ERR, 0x00 );
	}
	// No error, check for received messages, buffer 0
	else if(( flags & MCP_IRQ_RXB0 ) != 0x00 ){
		// Read in the info, address & message data
		can0_read( RXB0CTRL, &buffer[0], 14 );
		// Clear the IRQ flag as fast as possible
		can0_mod( CANINTF, MCP_IRQ_RXB0, 0x00 );

		// Fill out return structure
		// check for Remote Frame requests and indicate the status correctly
		if(( buffer[0] & MCP_RXB0_RTR ) == 0x00 ){
			// We've received a standard data packet
			RXPtr_can_message->status = CAN_OK;
			// Fill in the data
			RXPtr_can_message->data.data_u8[1] = buffer[ 7];
			RXPtr_can_message->data.data_u8[2] = buffer[ 8];
			RXPtr_can_message->data.data_u8[3] = buffer[ 9];
			RXPtr_can_message->data.data_u8[4] = buffer[10];
			RXPtr_can_message->data.data_u8[5] = buffer[11];
			RXPtr_can_message->data.data_u8[6] = buffer[12];
			RXPtr_can_message->data.data_u8[7] = buffer[13];
		}
		else{
			// We've received a remote frame request
			// Data is irrelevant with an RTR
			RXPtr_can_message->status = CAN_RTR;
		}
		// Fill in the address
		RXPtr_can_message->address = ((int)(buffer[1]) << 3) | ((int)(buffer[2]) >> 5);
		can_read_cnt++;

		//add message to queue to be decoded
		can_fifo_PUT(&can0_queue, *RXPtr_can_message);

	}
	// No error, check for received messages, buffer 1
	else if(( flags & MCP_IRQ_RXB1 ) != 0x00 ){
		// Read in the info, address & message data
		can0_read( RXB1CTRL, &buffer[0], 14 );
		// Clear the IRQ flag as fast as possible
		can0_mod( CANINTF, MCP_IRQ_RXB1, 0x00 );

		// Fill out return structure
		// check for Remote Frame requests and indicate the status correctly
		if(( buffer[0] & MCP_RXB1_RTR ) == 0x00 ){
			// We've received a standard data packet
			RXPtr_can_message->status = CAN_OK;
			// Fill in the data
			RXPtr_can_message->data.data_u8[0] = buffer[ 6];
			RXPtr_can_message->data.data_u8[1] = buffer[ 7];
			RXPtr_can_message->data.data_u8[2] = buffer[ 8];
			RXPtr_can_message->data.data_u8[3] = buffer[ 9];
			RXPtr_can_message->data.data_u8[4] = buffer[10];
			RXPtr_can_message->data.data_u8[5] = buffer[11];
			RXPtr_can_message->data.data_u8[6] = buffer[12];
			RXPtr_can_message->data.data_u8[7] = buffer[13];
		}
		else{
			// We've received a remote frame request
			// Data is irrelevant with an RTR
			RXPtr_can_message->status = CAN_RTR;
		}
		// Fill in the address
		RXPtr_can_message->address = ((int)(buffer[1]) << 3) | ((int)(buffer[2]) >> 5);
		can_read_cnt++;

		//add message to queue to be decoded
		can_fifo_PUT(&can0_queue, *RXPtr_can_message);

	}
	// If multiple receive then clear that error
	else if(( flags & MCP_IRQ_MERR) != 0x00 ){
		// Read error flags and counters
		can0_read( EFLAG, &buffer[0], 1 );
		can0_read( TEC, &buffer[1], 2 );
		// Clear error flags
		can0_mod( EFLAG, buffer[0], 0x00 );	// Modify (to '0') all bits that were set
		// Return error code, a blank address field, and error registers in data field
		RXPtr_can_message->status = CAN_MERROR;
		RXPtr_can_message->address = 0x0000;
		RXPtr_can_message->data.data_u8[0] = flags;		// CANINTF
		RXPtr_can_message->data.data_u8[1] = buffer[0];	// EFLG
		RXPtr_can_message->data.data_u8[2] = buffer[1];	// TEC
		RXPtr_can_message->data.data_u8[3] = buffer[2];	// REC
		// Clear the IRQ flag
		can0_mod( CANINTF, MCP_IRQ_MERR, 0x00 );
	}
	//not handled by buffers or errors
	else{
		RXPtr_can_message->status = CAN_FERROR;
		RXPtr_can_message->address = 0x0001;
		RXPtr_can_message->data.data_u8[0] = flags;		// CANINTF
		// Clear all IRQ flag
		can0_mod( CANINTF, 0xFF, 0x00 );
		can_err_count++;
	}
	
//added to tritum code to account for the fact that we could have more then one intrrupt pending at a given time
//NOTE: YOU MUST CLEAR THE MAIN_MODE STATUS BEFORE ENTERING THIS FUNCTION OR YOU WILL CLEAR THE RETURN FLAG
  can0_read(CANINTF, &flags, 1 );
  if((( flags & MCP_IRQ_ERR ) ||( flags & MCP_IRQ_MERR )) != 0x00 )
  {
    can0_read(EFLAG, &buffer[0], 1 );
    can0_read(TEC, &buffer[1], 2 );
    // Clear error flags
    can0_mod(EFLAG, buffer[0], 0x00 );	// Modify (to '0') all bits that were set
    // Clear the IRQ flag
    can0_mod(CANINTF, 0XA3, 0x00 );
  }

}

/*
 * Transmits a CAN message to the bus
 *	- Accepts address and data payload via can_interface structure
 *	- Busy waits while message is sent to CAN controller
 *	- Uses all available transmit buffers (3 available in CAN controller) to maximise throughput
 *	- Only modifies address information if it's different from what is already set up in CAN controller
 *	- Assumes constant 8-byte data length value
 */
int can0_transmit( void )
{
	unsigned int wait_count;
	static unsigned int buf_addr[3] = {0xFFFF, 0xFFFF, 0xFFFF};
	extern unsigned char can_full;	//used for CAN transmission status
	
	P2IE &= ~CAN0_INTn;
	// Fill data into buffer, it's used by any address
	// Allow room at the start of the buffer for the address info if needed
	buffer[ 5] = TX_can0_message.data.data_u8[0];
	buffer[ 6] = TX_can0_message.data.data_u8[1];
	buffer[ 7] = TX_can0_message.data.data_u8[2];
	buffer[ 8] = TX_can0_message.data.data_u8[3];
	buffer[ 9] = TX_can0_message.data.data_u8[4];
	buffer[10] = TX_can0_message.data.data_u8[5];
	buffer[11] = TX_can0_message.data.data_u8[6];
	buffer[12] = TX_can0_message.data.data_u8[7];

	// Check if the incoming address has already been configured in a mailbox
	if( TX_can0_message.address == buf_addr[0] ){
		// Mailbox 0 setup matches our new message
		// Write to TX Buffer 0, start at data registers, and initiate transmission
		can0_write_tx( 0x01, &buffer[5] );
		can0_rts( 0 );
	}
	else if( TX_can0_message.address == buf_addr[1] ){
		// Mailbox 1 setup matches our new message
		// Write to TX Buffer 1, start at data registers, and initiate transmission
		can0_write_tx( 0x03, &buffer[5] );
		can0_rts( 1 );
	}
	else if( TX_can0_message.address == buf_addr[2] ){
		// Mailbox 2 setup matches our new message
		// Write to TX Buffer 2, start at data registers, and initiate transmission
		can0_write_tx( 0x05, &buffer[5] );
		can0_rts( 2 );
	}
	else{
		// No matches in existing mailboxes
		// No mailboxes already configured, so we'll need to load an address - set it up
		buffer[0] = (unsigned char)(TX_can0_message.address >> 3);
		buffer[1] = (unsigned char)(TX_can0_message.address << 5);
		buffer[2] = 0x00;						// EID8
		buffer[3] = 0x00;						// EID0
		buffer[4] = 0x08;						// DLC = 8 bytes
		
		// Check if we've got any un-setup mailboxes free and use them
		// Otherwise, find a non-busy mailbox and set it up with our new address
		if( buf_addr[0] == 0xFFFF ){			// Mailbox 0 is free
			// Write to TX Buffer 0, start at address registers, and initiate transmission
			can0_write_tx( 0x00, &buffer[0] );
			can0_rts( 0 );
			buf_addr[0] = TX_can0_message.address;
		}									
		else if( buf_addr[1] == 0xFFFF ){		// Mailbox 1 is free
			// Write to TX Buffer 1, start at address registers, and initiate transmission
			can0_write_tx( 0x02, &buffer[0] );
			can0_rts( 1 );
			buf_addr[1] = TX_can0_message.address;
		}
		else if( buf_addr[2] == 0xFFFF ){		// Mailbox 2 is free
			// Write to TX Buffer 2, start at address registers, and initiate transmission
			can0_write_tx( 0x04, &buffer[0] );
			can0_rts( 2 );
			buf_addr[2] = TX_can0_message.address;
		}
		else {					
	
			// No mailboxes free, wait until at least one is not busy
			wait_count = 0;
			 while((( can0_read_status() & 0x54 ) == 0x54) && (wait_count<2))
				 {
				 delay();
				 wait_count++;
				 }
			// Continue without sending ...
			//	if(( can0_read_status() & 0x54 ) == 0x54) {
			//		can0_full = TRUE;
			//	}
			//	else {
					// Is it mailbox 0?
					if(( can0_read_status() & 0x04 ) == 0x00) {
						// Setup mailbox 0 and send the message
						can0_write_tx( 0x00, &buffer[0] );
						can0_rts( 0 );
						buf_addr[0] = TX_can0_message.address;
					}
					// Is it mailbox 1?
					else if(( can0_read_status() & 0x10 ) == 0x00) {
						// Setup mailbox 1 and send the message
						can0_write_tx( 0x02, &buffer[0] );
						can0_rts( 1 );
						buf_addr[1] = TX_can0_message.address;
					}
					// Is it mailbox 2?
					else if(( can0_read_status() & 0x40 ) == 0x00) {
						// Setup mailbox 2 and send the message
						can0_write_tx( 0x04, &buffer[0] );
						can0_rts( 2 );
						buf_addr[2] = TX_can0_message.address;
					}
			//	}
		}
	}
	P2IE |= ~CAN0_INTn;

	return(0);
}

/*
 * Read CAN Status flags
 */
void can0_flag_check( void )
{
	extern unsigned char can_CANINTF, can0_FLAGS[3];

	can0_read( CANINTF, &can_CANINTF, 1 );
	// Check for errors
	can0_read( EFLAG, &can0_FLAGS[0], 1 );
	can0_read( TEC, &can0_FLAGS[1], 2 );
}

/**************************************************************************************************
 * PRIVATE FUNCTIONS
 *************************************************************************************************/

/*
 * Resets MCP2515 CAN controller via SPI port
 *	- SPI port must be already initialised
 */
void can0_reset( void )
{
	can0_select;
	can0spi_transmit( MCP_RESET );
	can0_deselect;
}
 
/*
 * Reads data bytes from the MCP2515
 *	- Pass in starting address, pointer to array of bytes for return data, and number of bytes to read
 */
void can0_read( unsigned char address, unsigned char *ptr, unsigned char bytes )
{
	unsigned char i;
	
	can0_select;
	can0spi_transmit( MCP_READ );
	can0spi_transmit( address );
	for( i = 0; i < bytes; i++ ) *ptr++ = can0spi_exchange( 0x00 );
	can0_deselect;
}

/*
 * Reads data bytes from receive buffers
 *	- Pass in buffer number and start position as defined in MCP2515 datasheet
 *		- For starting at data, returns 8 bytes
 *		- For starting at address, returns 13 bytes
 */
void can0_read_rx( unsigned char address, unsigned char *ptr )
{
	unsigned char i;
	
	address &= 0x03;						// Force upper bits of address to be zero (they're invalid)
	address <<= 1;							// Shift input bits to correct location in command byte
	address |= MCP_READ_RX;					// Construct command byte for MCP2515
	
	can0_select;
	can0spi_transmit( address );
	
	if(( address & 0x02 ) == 0x00 ){		// Start at address registers
		for( i = 0; i < 13; i++ ){
			*ptr++ = can0spi_exchange( 0x00 );
		}
	}
	else{									// Start at data registers
		for( i = 0; i < 8; i++ ){
			*ptr++ = can0spi_exchange( 0x00 );
		}
	}
	can0_deselect;
}

/*
 * Writes data bytes to the MCP2515
 *	- Pass in starting address, pointer to array of bytes, and number of bytes to write
 */
void can0_write( unsigned char address, unsigned char *ptr, unsigned char bytes )
{
	unsigned char i;
	
	can0_select;
	can0spi_transmit( MCP_WRITE );
	can0spi_transmit( address );
	for( i = 0; i < (bytes-1); i++ ){
		can0spi_transmit( *ptr++ );
	}
	can0spi_transmit( *ptr );
	can0_deselect;
}

/*
 * Writes data bytes to transmit buffers
 *	- Pass in buffer number and start position as defined in MCP2515 datasheet
 *		- For starting at data, accepts 8 bytes
 *		- For starting at address, accepts 13 bytes
 */
void can0_write_tx( unsigned char address, unsigned char *ptr )
{
	unsigned char i;
	
	address &= 0x07;						// Force upper bits of address to be zero (they're invalid)
	address |= MCP_WRITE_TX;				// Construct command byte for MCP2515
	
	can0_select;
	can0spi_transmit( address );
	
	if(( address & 0x01 ) == 0x00 ){		// Start at address registers
		for( i = 0; i < 13; i++ ){
			can0spi_transmit( *ptr++ );
		}
	}
	else{									// Start at data registers
		for( i = 0; i < 8; i++ ){
			can0spi_transmit( *ptr++ );
		}
	}
	can0_deselect;
}

/*
 * Request to send selected transmit buffer
 *	- Pass in address of buffer to transmit: 0, 1 or 2
 */
void can0_rts( unsigned char address )
{
	unsigned char i;
	
	// Set up address bits in command byte
	i = MCP_RTS;
	if( address == 0 ) i |= 0x01;
	else if( address == 1 ) i |= 0x02;
	else if( address == 2 ) i |= 0x04;
	
	// Write command
	can0_select;
	can0spi_transmit( i );
	can0_deselect;
}

/*
 * Reads MCP2515 status register
 */
unsigned char can0_read_status( void )
{
	unsigned char status;
	
	can0_select;
	can0spi_transmit( MCP_STATUS );
	status = can0spi_exchange( 0x00 );
	can0_deselect;
	return status;
}

/*
 * Reads MCP2515 RX status (filter match) register
 */
unsigned char can0_read_filter( void )
{
	unsigned char status;
	
	can0_select;
	can0spi_transmit( MCP_FILTER );
	status = can0spi_exchange( 0x00 );
	can0_deselect;
	return status;
}
 
/*
 * Modifies selected register in MCP2515
 *	- Pass in register to be modified, bit mask, and bit data
 */
void can0_mod( unsigned char address, unsigned char mask, unsigned char data )
{
	can0_select;
	can0spi_transmit( MCP_MODIFY );
	can0spi_transmit( address );
	can0spi_transmit( mask );
	can0spi_transmit( data );
	can0_deselect;
}
