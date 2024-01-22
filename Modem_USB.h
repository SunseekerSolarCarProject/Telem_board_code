#ifndef Modem_USB_PORTS_H_
#define Modem_USB_PORTS_H_

//public declarations constants

static char MODEM_USBCmd[5] = "+++\r\0";
static char USB_Test1[13] = "Sunseeker \n\r\0";
static char USB_Test2[9] = "2021. \n\r\0";

static char USBCommand[5] = "+++\r\0";

/*********************************************************************************/
// Telemetry to PC External USB (voltage isolated)
/*********************************************************************************/

void Modem_USB_init();

void Modem_USB_putchar(char data);
unsigned char Modem_USB_getchar(void);

int Modem_USB_gets(char *ptr);
int Modem_USB_puts(char *str);

void Modem_USB_puts_int(void);

#endif /*Modem_USB_PORTS_H_*/
