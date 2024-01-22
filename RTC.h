/******************************************************************************
/ Program to set and keep track of a clock/calendar updated by the RTC
/
/
/
/
/
/
/
/
*******************************************************************************/
#ifndef RTC_H_
#define RTC_H_

typedef struct _Time { int Year, Month, DayOfWeek, Day, Hour, Minute, Second; }time;

// User defined functions
extern void init_RTC(void); 	
extern int setRTC(int h, int m, int s, int mo, int d, int y);
void setRTChms(int h, int m, int s);
extern void getRTCTime(int *h, int *m, int *s);
extern int insert_time(char *time_string);

 // Provided functions

extern int SetRTCYEAR(int year); 	
extern int SetRTCMON(int month);
extern int SetRTCDAY(int day);
extern int SetRTCDOW(int dow);
extern int SetRTCHOUR(int hour);
extern int SetRTCMIN(int min);
extern int SetRTCSEC(int sec);

extern int GetRTCTIM0(void); 	
extern int GetRTCTIM1(void); 	
extern int GetRTCDATE(void); 	
extern int GetRTCYEAR(void); 	

extern int GetRTCMON(void);
extern int GetRTCDOW(void);
extern int GetRTCDAY(void);
extern int GetRTCHOUR(void);
extern int GetRTCMIN(void);
extern int GetRTCSEC(void);

int TestRTCYear(time TaD);
int TestRTCMonth(time TaD);
int TestRTCDow(time TaD);
int TestRTCDay(time TaD);
int TestRTCHour(time TaD);
int TestRTCMinute(time TaD);
int TestRTCSecond(time TaD);


#endif /*RTC_H_*/
