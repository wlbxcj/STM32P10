#ifndef _SYSTIMER_H_
#define _SYSTIMER_H_

extern void Tim3Init(void);
extern void s_TimerProc(void);
extern void TimerSet(unsigned char TimerID, unsigned long Cnts);
extern unsigned long TimerCheck(unsigned char TimerID);
extern int SetTimerEvent(unsigned short uElapse, void (*TimerFunc)(void));

extern void KillTimerEvent(int handle);
extern unsigned int GetTimerCount(void);

#endif
