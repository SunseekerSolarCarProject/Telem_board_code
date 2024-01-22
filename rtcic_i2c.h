/*
 * rtcic_i2c.h
 *
 *  Created on: Jun 3, 2021
 *      Author: ecelab
 *      for MCP7940 RTC IC
 */

#ifndef RTCIC_I2C_H_
#define RTCIC_I2C_H_

#define SLV_Addr  0x6F; // Address 0x6F <<1 lsb is R/Wn
#define ADDR_RTCC_WRITE 0xDE; // Address 0x6F <<1 lsb is R/Wn
#define ADDR_RTCC_READ  0xDF; // Address 0x6F <<1 lsb is R/Wn

// User defined functions
void init_i2c(void);
void init_MCP7940M(void);
void init_MCP7940M_int(void);
void set_all_MCP7940M(char h, char m, char s, char mo, char d, char y);
void set_MCP7940M(char h, char m, char s);
void get_MCP7940M(unsigned char *hr, unsigned char *min, unsigned char *sec);
void get_MCP7940M_int(void);

int insert_time_2(char *time_string);

static char SetRTC_TxData[] =              // Table of data to transmit
{
  0x00, // Register 0 Address
  0x80, // turn on internal osc, sec = 0
  0x00, // min = 0
  0x92, // 24-hour format, hours = 12
  0x01, // weekday = Monday
  0x01, // January 1
  0x01, //
  0x21,   // xx21 (BCD)
  0x40  // output 1 Hz, no alarm, use crystal osc
};

#define SetRTC_TXByteCtr  9


// Register MAP values
#define MRTCSEC		0x00 //Values are BCD
#define MRTCMIN		0x01
#define MRTCHOUR	0x02
#define MRTCWKDAY	0x03
#define MRTCDATE	0x04
#define MRTCMTH		0x05
#define MRTCYEAR	0x06
#define CONTROL		0x07
#define OSCTRIM		0x08
//#define reserved	0x09
#define ALM0SEC		0x0A
#define ALM0MIN		0x0B
#define ALM0HOUR	0x0C
#define ALM0WKDAY	0x0D
#define ALM0DATE	0x0E
#define ALM0MTH		0x0F
//#define reserved	0x10
#define ALM1SEC		0x11
#define ALM1MIN		0x12
#define ALM1HOUR	0x13
#define ALM1WKDAY	0x14
#define ALM1DATE	0x15
#define ALM1MTH		0x16

#define SUNYEAR		0x21
#define SUNMONTH	0x07
#define SUNDATE		0X01
#define SUNWKDAY    0x04

#endif /* RTCIC_I2C_H_ */
