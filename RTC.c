//
// Telemetry Real Time Clock
//
#include "Sunseeker2021.h"

/*************************************************************
/ Name: setRTC
/ IN: int hour, int minute, int second, int month, int day, int year
/ OUT:  1 if succesful
/ DESC:  This function is used to set the Real Time Clock Values
************************************************************/
void init_RTC(void)
{
  RTCCTL01 |= RTCBCD+RTCHOLD+RTCMODE;       // RTC enable, BCD mode,
                                            // Calendar Mode
  setRTC(12,0,0,7,1,2021);
  RTCCTL01 &= ~RTCHOLD;       // Enable counting
}

//
//   char time_msg[17];// = "TL_TIM,HH:MM:SS\r\n";
//
int insert_time(char *time_string)
{
  char h1, h0, m1, m0, s1, s0;
  extern int thrs, tmin, tsec;
  
  h1 = ((thrs>>4) & 0x0F)+'0';
  h0 = (thrs & 0x0F)+'0';
  time_string[7] = h1;
  time_string[8] = h0;

  m1 = ((tmin>>4) & 0x0F)+'0';
  m0 = (tmin & 0x0F)+'0';
  time_string[10] = m1;
  time_string[11] = m0;

  s1 = ((tsec>>4) & 0x0F)+'0';
  s0 = (tsec & 0x0F)+'0';
  time_string[13] = s1;
  time_string[14] = s0;
  
  return 1;
}

/*************************************************************
/ Name: setRTC
/ IN: int hour, int minute, int second, int month, int day, int year
/ OUT:  1 if succesful
/ DESC:  This function is used to set the Real Time Clock Values
************************************************************/
int setRTC(int h, int m, int s, int mo, int d, int y)
{
  SetRTCYEAR(y);
  delay();

 // P1OUT ^= (LED5);
  SetRTCMON(mo);
  delay();

  //P1OUT ^= (LED5 | LED6);
  SetRTCDAY(d);
  delay();

 // P1OUT ^= (LED5);
  SetRTCHOUR(h);
  delay();

  //P1OUT ^= (LED5 | LED6 | LED7);
  SetRTCMIN(m);
  delay();

 // P1OUT ^= (LED5 );
  SetRTCSEC(s);
  delay();

  //P1OUT ^= (LED5 | LED6 );
  return 1;
}

void setRTChms(int h, int m, int s)
{
  SetRTCHOUR(h);
  delay();

  SetRTCMIN(m);
  delay();

  SetRTCSEC(s);
  delay();
}

/*************************************************************
/ Name: getRTCTime
/ IN: int *hour, int *minute, int *second
/ OUT:  void
/ DESC:  This function returns the current time of the Real time clock
************************************************************/
void getRTCTime(int *h, int *m, int *s)
{
  int min_sec;
  while((RTCCTL0 && RTCRDYIFG) == 0);
  *h = GetRTCHOUR();
  while((RTCCTL0 && RTCRDYIFG) == 0);
  min_sec = GetRTCTIM0();
  *m = (min_sec & 0xFF00)>>8;
  *s = min_sec & 0x00FF;
}
