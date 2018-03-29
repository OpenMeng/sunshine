#ifndef __CRONTAB_H__
#define __CRONTAB_H__

#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "dstream.h"
#include "memleak.h"

using namespace std;

namespace util
{

const int TASK_NUM = 256;
struct crontask
{
	int nMinute;
	int nHour;
	int nMonthDay;
	int nMonth;
	int nWeekDay;

	int (*call_proc)(void);
};

class crontab
{
public:
	static crontask pCronTask[TASK_NUM];
	static int 		 taskstatus[TASK_NUM];
	static int		 nCronTaskNum;
	static timer_t   timerid;
private:
	int	_nInterval;

	static void signal_handler(int signo);
	static bool to_be_run(crontask task,time_t  nNowTime);
public:
	crontab(int nInterval);
	~crontab();

	int add_task(crontask task);
	int start();	
	int stop();
};

}
#endif

