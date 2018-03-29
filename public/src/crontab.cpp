#include "crontab.h"
#include "macro.h"
#if defined(MEM_DEBUG)
#define new 		DEBUG_NEW
#define delete 	DEBUG_DELETE
#endif
#include <string.h>
namespace util
{

	crontask	crontab::pCronTask[TASK_NUM];
	int		crontab::taskstatus[TASK_NUM];
	int		crontab::nCronTaskNum = 0;
	timer_t   	crontab::timerid = (timer_t)(-1);

	crontab::crontab(int nInterval)
	{

		struct sigaction sigact;
		struct sigevent sigev;

		nCronTaskNum = 0;
		_nInterval = nInterval;

		sigact.sa_handler = signal_handler;
		sigemptyset(&(sigact.sa_mask));
		sigact.sa_flags = 0;

		if (sigaction(SIGUSR1, &sigact, (struct sigaction *)0) == -1) 
		{
			perror("sigaction");
			exit(1);
		}

		sigev.sigev_notify = SIGEV_SIGNAL;
		sigev.sigev_signo = SIGUSR1;

		if (timer_create(CLOCK_REALTIME, &sigev, &timerid) == -1) 
		{
			perror("timer_create");
			exit(1);
		}
	}

	crontab::~crontab()
	{
		if (timer_delete(timerid) == -1) 
		{
			perror("timer_delete");
			exit(1);
		}
	}

	int crontab::add_task(crontask task)
	{
		if ( nCronTaskNum > 256 )
			return -1;
		memcpy((char*)&pCronTask[nCronTaskNum++],(char*)&task,sizeof(crontask));
		return 0;
	}

	int crontab::start()
	{
		struct itimerspec one_minute = { {_nInterval, 0}, {_nInterval, 0} };
		if (timer_settime(timerid, 0, &one_minute, (struct itimerspec*)0) == -1) 
		{
			perror("timer_create");
			return -1;
		}

		pause();
		return 0;
	}	

	int crontab::stop()
	{
		if (timer_delete(timerid) == -1) 
		{
			perror("timer_delete");
			return -1;
		}
		return 0;
	}

	bool crontab::to_be_run(crontask task,time_t  nNowTime)
	{
		return FALSE;
	}
	void crontab::signal_handler(int signo)
	{
		int overrun = timer_getoverrun(timerid);

		dout << "signal=[" << signo << "],tasknum=[" << nCronTaskNum << "]." << endl;
		if (overrun == -1) 
		{
			perror("handler: timer_getoverrun()");
			exit(1);
		}
		for( int i = 0; i < nCronTaskNum; i ++ )
		{
			if ( pCronTask[i].call_proc != 0 && !to_be_run(pCronTask[i],time(0)))
			{
				pCronTask[i].call_proc();
			}
		}
		dout << "Timer expired, overrun count was " << overrun << endl;
	}

}

