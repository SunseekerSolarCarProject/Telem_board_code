#ifndef Modem_RS232_PORTS_H_
#define Modem_RS232_PORTS_H_

//public declarations constants

static char MODEMCmd[5] = "+++\r\0";
static char RS232_Test1[13] = "Sunseeker \n\r\0";
static char RS232_Test2[9] = "2021. \n\r\0";
static char Parse_header[6][5] = {"LTC \0","ADC \0","ISH \0","ERR \0","BPS \0","BPC \0"};

static char RFCommand[5] = "+++\r\0";
// 2008 Initialization using 9600 baud
//  Sunseeker telemetry PCB initialization
//	ATAM, MY 786, DT 786 <Enter>
//	ATRR 3, RN 4<Enter>
//	ATPK (message length),RB 474,RO 1B4<Enter>
//	ATPL 2<Enter>
//  ATBD 3<Enter>
//	ATCN<Enter>
//static char RFModem[74] = "ATAM,MY 786,DT 786\rATRR 3,RN 4\rATPK 474,RB 474,RO 1B4\rATPL 2\rATBD 3\rATCN\r";
// 2010 Initialization
static char RFModemH[76] = "ATAM,MY 786,DT 786\rATRR 3,RN 4\rATPK 201,RB 12F,RO 128\rATPL 2\rATBD 3\rATCN\r\0\0";
static char RFModemL[76] = "ATAM,MY 786,DT 786\rATRR 3,RN 4\rATPK 201,RB 12F,RO 128\rATPL 2\rATBD 3\rATCN\r\0\0";
static char RFModemS[76] = "ATAM,MY 786,DT 786\rATRR 3,RN 4\rATPK 201,RB 1A7,RO 128\rATPL 2\rATBD 3\rATCN\r\0\0";
//Baud rate change command
static char RFModemBaud[14] = "ATBD 7\rATCN\r\0\0";


/*********************************************************************************/
// BPS to PC External RS-232 (voltage isolated)
/*********************************************************************************/

void Modem_UART_init();
void Modem_UART_putchar(char data);
unsigned char Modem_UART_getchar(void);

int Modem_UART_gets(char *ptr);
int Modem_UART_puts(char *str);

void Modem_UART_puts_int(void);

void MODEM_command_puts (char data[]);



#endif /*Modem_RS232_PORTS_H_*/
