#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <time.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <errno.h>
#include <cstdlib>
#include <regex.h>
#include <dirent.h>

#include "object.h"
#include "user_exception.h"
#include <sys/ioctl.h>

namespace util
{
#define TIMEOUT_INF ~((timeout_t) 0)

class threadrwlock
{
private:
	pthread_rwlock_t _lock;
public:
	threadrwlock();
	~threadrwlock();

	void read_lock(void);
	void write_lock(void);
	bool try_read_lock(void);
	bool try_write_lock(void);
	void unlock(void);
};

class threadmutex
{
protected:
	pthread_mutex_t	 _mutex;
public:
	threadmutex();
	~threadmutex();

	void enter_mutex(void);
	bool try_enter_mutex(void);
	void leave_mutex(void);
};


class threadcond : public threadmutex
{
protected:
	pthread_cond_t _cond;
public:
	threadcond();
	~threadcond();

	void signal(bool broadcast = false);
	void wait(timeout_t timer = 0,int unit = 0);
};

class workthreadKey
{
private:
	pthread_key_t key;

public:
	workthreadKey();
	~workthreadKey();
	void *get_key(void);
	void set_key(void *);
};

extern "C"
{
	struct timespec* gettimeout(struct timespec *spec, timeout_t timeout);
};


class mutexbool
{
public:
	mutexbool(bool bValue = false);
	~mutexbool();

	void operator = (bool bValue);
	bool operator == (bool bValue);
	bool operator != (bool bValue);
  bool operator !();
private:
	bool _bValue;
	pthread_mutex_t	 _mutex;
};

class workthread
{
private:
	pthread_t _tid;
  int _nCallBack;
	pthread_attr_t _attr;
	object* pBaseObj;
public:
	workthread(object* pObj,int pri = 0, size_t stack = 0);
	virtual ~workthread();

	int start(int nCallBack = 0);
	int stop(void);
	int join(void);
	int detach(void);
	bool is_running(void);
	bool is_thread(void);
	void terminate(void);
	object* get_object();
	static void* call_back(void* pParam);
  static void* call_back_ext(void* pParam);
	static void* call_back_service(void* pParam);
};

}

#endif
