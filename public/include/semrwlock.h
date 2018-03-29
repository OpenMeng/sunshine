#ifndef __SEMRWLOCK_H__
#define __SEMRWLOCK_H__

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/sem.h>
#include <iostream>

#include "dstream.h"
#include "macro.h"
#include "user_exception.h"
#include "object.h"

using namespace std;

namespace util
{

class semrwlock
{
private:
	enum enLockType
	{
		WRITER_COUNT = 0,
    READER_COUNT = 1,
		MUTEX_LOCK = 2,
    LOCK_NUM
	};

	int   _nSemId[LOCK_NUM];
	key_t _nIpcKey;
	BOOL  _bServer;
	
	int lock(int nSemNo);
	int unlock(int nSemNo);
	int get_value(int nSemNo);
	
public:
	semrwlock(int nIpcKey);
	~semrwlock();
	
	int create_rwlock();
	int get_rwlock();
	
	int read_lock();
	int read_unlock();
	int write_lock();
	int write_unlock();
	void print_time(const char* func_name);
};

}
#endif


