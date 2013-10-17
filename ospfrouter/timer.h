/***************************************************************************
                  timer.h  -  OSPF timer handling
                             -------------------
    begin        : Sun Apr 7 2002
    copyright    : (C) 2002 by Ashish Puri, Anurag Damani, Jignesh Patel
    email        : ospfrouter@how.to
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef	__TIMER_H
#define	__TIMER_H

#define SINGLESHOT 1
#define INTERVAL 0
#define FIRED 2
#define NUMOFIFTIMER 2

#define IFTIMER (MAXIF * NUMOFIFTIMER)

int numoftimers = 0;
int numof_nb_sstimers = IFTIMER + 1;

struct timer
{
	int interval;
	int index;
	int status;
	int started;
	void(*f)(int );
};

struct timer timerinterval[10];
unsigned int numofticks = 0;
struct inactivitytimer
{
	struct in_addr nb_ip;////UNIQUELY IDENTIFY NB
	int index;//index======identifies interface to which nb belongs
	int cur_interval;
	void (*cur_f)(struct in_addr  ,int );
	int status;
	int started;
};
struct inactivitytimer nb_timer[10];
int num_of_nb_timers = 0;

void handler(int signum)
{
	int i;
	numofticks += 1;
	for(i=0;i<numoftimers;i++)
	{	
		if((numofticks % timerinterval[i].interval ==
			timerinterval[i].started % timerinterval[i].interval)
		&&(timerinterval[i].status!=FIRED))
		{////HANDLE IF TIMERS
		timerinterval[i].f(timerinterval[i].index/NUMOFIFTIMER);
		}
	}
	for(i = 0; i<num_of_nb_timers;i++)
	{	
		if((numofticks % nb_timer[i].cur_interval ==
			nb_timer[i].started % nb_timer[i].cur_interval)
		&&(nb_timer[i].status!=FIRED))	
		{	
			//display(2,0,5,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$ %d ",i);
			nb_timer[i].cur_f(nb_timer[i].nb_ip,
								nb_timer[i].index);
		}
	}
}

int  addsstimer(int interval,void (* functobecalled)(int))
{
	timerinterval[numoftimers].interval = interval;
	timerinterval[numoftimers].status   = SINGLESHOT;
	timerinterval[numoftimers].f        = functobecalled;
	timerinterval[numoftimers].index    = numoftimers;
	timerinterval[numoftimers].started  = numofticks;
	numoftimers++;
	return  numoftimers -1;
}

int addintervaltimer(int interval,void (* functobecalled)(int ))
{
	timerinterval[numoftimers].interval = interval;
	timerinterval[numoftimers].f        = functobecalled;
	timerinterval[numoftimers].status   = INTERVAL;
	timerinterval[numoftimers].index    = numoftimers;
	timerinterval[numoftimers].started  = numofticks;
	numoftimers++;
	return  numoftimers -1;
}
int stopitimer(int index)
{
	timerinterval[index].status = FIRED;
	
}

int remove_nb_ss_timer(int index)
{	int j=0;
	for(j = index;j< numof_nb_sstimers;j++)
	{
		timerinterval[j] = timerinterval[j + 1];
	}
	numof_nb_sstimers --;
}

int add_nb_sstimer(int interval,void(*f)(struct in_addr nb_ip,int),struct in_addr nb_ip)
{
	nb_timer[num_of_nb_timers].cur_interval = interval;
	nb_timer[num_of_nb_timers].status   = SINGLESHOT;
	nb_timer[num_of_nb_timers].cur_f    = f;
	nb_timer[num_of_nb_timers].index    = num_of_nb_timers;
	nb_timer[num_of_nb_timers].started  = numofticks;
	nb_timer[num_of_nb_timers].nb_ip    = nb_ip;
	num_of_nb_timers++;
	return  num_of_nb_timers -1;
}
int add_nb_itimer(int interval,void(*f)(struct in_addr nb_ip,int),struct in_addr nb_ip)
{
	nb_timer[num_of_nb_timers].cur_interval = interval;
	nb_timer[num_of_nb_timers].status   = INTERVAL;
	nb_timer[num_of_nb_timers].cur_f    = f;
	nb_timer[num_of_nb_timers].index    = num_of_nb_timers;
	nb_timer[num_of_nb_timers].started  = numofticks;
	nb_timer[num_of_nb_timers].nb_ip    = nb_ip;
	num_of_nb_timers++;
	return  num_of_nb_timers -1;
}

int stop_nb_itimer(int index)
{
	nb_timer[index].status = FIRED;
}

int restart_nb_sstimer(int index)
{	
	nb_timer[index].started  = numofticks;
}

int restart_nb_itimer(int index)
{	
	nb_timer[index].started  = numofticks - 1;
}


#endif // __TIMER_H

