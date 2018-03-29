#ifndef __SEMMUTEX_H__
#define __SEMMUTEX_H__

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/sem.h>

#include "macro.h"
#include "user_exception.h"

namespace util
{

class  semmutex
{
private:
	int _nsemid;

public:
	semmutex();
	virtual ~semmutex();
	
	void lock(void);
	bool try_lock(void);
	void unlock(void);
	int get_value(void);
};

}
#endif

