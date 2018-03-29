#include "workthread.h"

#if defined(MEM_DEBUG)
#define new		DEBUG_NEW
#define delete	DEBUG_DELETE
#endif

namespace util
{

#ifndef KEY_INVALID
#define KEY_INVALID	((pthread_key_t)(~0))
#endif


threadrwlock::threadrwlock()
{
	pthread_rwlockattr_t attr;

	pthread_rwlockattr_init(&attr);
	if(pthread_rwlock_init(&_lock, &attr) < 0)
		throw bad_param(__FILE__,__LINE__,316,"pthread_rwlock_init error.");
}

threadrwlock::~threadrwlock()
{
	pthread_rwlock_destroy(&_lock);
}

void threadrwlock::read_lock(void)
{
	pthread_rwlock_rdlock(&_lock);
}

void threadrwlock::write_lock(void)
{
	pthread_rwlock_wrlock(&_lock);
}

void threadrwlock::unlock(void)
{
	pthread_rwlock_unlock(&_lock);
}

bool threadrwlock::try_read_lock(void)
{
	if(pthread_rwlock_tryrdlock(&_lock))
		return false;
	return true;
}

bool threadrwlock::try_write_lock(void)
{
	if(pthread_rwlock_trywrlock(&_lock))
		return false;
	return true;
}

threadmutex::threadmutex()
{
	pthread_mutexattr_t _attr;
	pthread_mutexattr_init(&_attr);
	if(pthread_mutex_init(&_mutex, &_attr))
		throw bad_param(__FILE__,__LINE__,317,"pthread_mutex_init error.");
}

threadmutex::~threadmutex()
{
	pthread_mutex_destroy(&_mutex);
}

void threadmutex::enter_mutex(void)
{
	pthread_mutex_lock(&_mutex);
}

bool threadmutex::try_enter_mutex(void)
{
	return (pthread_mutex_trylock(&_mutex) == 0) ? true : false;
}

void threadmutex::leave_mutex(void)
{
	pthread_mutex_unlock(&_mutex);
}


threadcond::threadcond() : threadmutex()
{
	if(pthread_cond_init(&_cond, 0))
		throw bad_param(__FILE__,__LINE__,318,"pthread_cond_init error.");
}

threadcond::~threadcond()
{
	pthread_cond_destroy(&_cond);
}

void threadcond::signal(bool broadcast)
{
	if(broadcast)
		pthread_cond_broadcast(&_cond);
	else
		pthread_cond_signal(&_cond);
}

void threadcond::wait(timeout_t timeout,int unit)
{
	struct timespec ts;

	enter_mutex();
	if(!timeout)
	{
		pthread_cond_wait(&_cond, &_mutex);
		leave_mutex();
		return;
	}

	if ( unit == 0 )
	  gettimeout(&ts, timeout * 1000);
	else
	  gettimeout(&ts, timeout);

	pthread_cond_timedwait(&_cond, &_mutex, &ts);
	leave_mutex();
}

timespec *gettimeout(struct timespec *spec, timeout_t timer)
{
	static struct timespec myspec;

	if (spec == 0)
		spec = &myspec;

	struct timeval current;

	gettimeofday(&current, 0);
	spec->tv_sec = current.tv_sec + timer / 1000;
	spec->tv_nsec = (current.tv_usec + (timer % 1000) * 1000) * 1000;

	return spec;
}

workthreadKey::workthreadKey(void)
{
	if(pthread_key_create(&key, 0))
		key = KEY_INVALID;
}

workthreadKey::~workthreadKey()
{
	if(key != KEY_INVALID)
		pthread_key_delete(key);
}

void *workthreadKey::get_key(void)
{
	if(key != KEY_INVALID)
		return pthread_getspecific(key);
	else
		return 0;
}

void workthreadKey::set_key(void *ptr)
{
	if(key != KEY_INVALID)
		pthread_setspecific(key, ptr);
}

mutexbool::mutexbool(bool bValue)
{
	_bValue = bValue;
	pthread_mutexattr_t _attr;
	pthread_mutexattr_init(&_attr);
	if(pthread_mutex_init(&_mutex, &_attr))
		throw bad_param(__FILE__,__LINE__,317,"pthread_mutex_init error.");
}

mutexbool::~mutexbool()
{
	pthread_mutex_destroy(&_mutex);
}

bool mutexbool::operator !=(bool bValue)
{
	bool bRet;
	pthread_mutex_lock(&_mutex);
	bRet = (_bValue != bValue);
	pthread_mutex_unlock(&_mutex);

	return	bRet;
}

void mutexbool::operator = (bool bValue)
{
	pthread_mutex_lock(&_mutex);
	_bValue = bValue;
	pthread_mutex_unlock(&_mutex);
}

bool mutexbool::operator ==(bool bValue)
{
	bool bRet;
	pthread_mutex_lock(&_mutex);
	bRet = (_bValue == bValue);
	pthread_mutex_unlock(&_mutex);

	return	bRet;
}

bool mutexbool::operator !()
{
  bool bRet;
  pthread_mutex_lock(&_mutex);
  bRet = !_bValue;
  pthread_mutex_unlock(&_mutex);

  return	bRet;
}

workthread::workthread(object* pObj,int pri, size_t stack)
	:pBaseObj(pObj)
{
	_tid = 0;
	_nCallBack = 0;
	pthread_attr_init(&_attr);
	pthread_attr_setdetachstate(&_attr, PTHREAD_CREATE_JOINABLE);
	if ( stack > 0 )
		pthread_attr_setstacksize(&_attr, stack <= PTHREAD_STACK_MIN ? PTHREAD_STACK_MIN : stack);
}

workthread::~workthread()
{
	terminate();
}

bool workthread::is_thread(void)
{
	return (_tid == pthread_self()) ? true : false;
}

bool workthread::is_running(void)
{
#ifdef DEBUG
    if ((_tid != 0) ? (pthread_kill(_tid,0) == 0 ? true : false ) : false)
        dout << "THREAD ALIVE!" <<endl ;
    else
        dout << "THREAD IS NOT ALIVE!" << endl ;
#endif

	return (_tid != 0) ? (pthread_kill(_tid,0) == 0 ? true : false ) : false;
}

int workthread::start(int nCallBack)
{
	int nrt = -1;

	_nCallBack = nCallBack;
	dout << "before pthread_create:workthread::start()" << endl;
	switch(_nCallBack) 
	{
	case 0:
		nrt = pthread_create(&_tid, &_attr, call_back, this);
		break;
	case 1:
		nrt = pthread_create(&_tid, &_attr, call_back_ext, this);
		break;
	case 2:
		nrt = pthread_create(&_tid, &_attr, call_back_service, this);
		break;
	default:
		throw bad_msg(__FILE__,__LINE__,319,"create thread failed.");
	}
	dout << "end pthread_create:workthread::start()" << endl;
	return nrt;
}

int workthread::stop(void)
{
	return pthread_kill(_tid,SIGKILL);
}

int workthread::detach()
{
	int rtn;
	if(_tid)
	{
		pthread_detach(_tid);
	}

	switch(_nCallBack) 
	{
	case 0:
		rtn = pthread_create(&_tid, &_attr, call_back, this);
		break;
	case 1:
		rtn = pthread_create(&_tid, &_attr, call_back_ext, this);
		break;
	case 2:
		rtn = pthread_create(&_tid, &_attr, call_back_service, this);
		break;
	default:
		rtn = -1;
		break;
	}

	if(!rtn && _tid)
	{
		pthread_detach(_tid);
		return 0;
	}
	else
		return -1;
}

void workthread::terminate(void)
{
	if(!_tid)
		return;

	if(pthread_self() != _tid)
	{
		pthread_cancel(_tid);
		_tid = 0;
	}
	pthread_attr_destroy(&_attr);
	_tid = 0;
}

int workthread::join()
{
	return pthread_join(_tid,0);
}

object* workthread::get_object()
{
	return pBaseObj;
}

void* workthread::call_back(void *pParam)
{
	workthread *pThread = (workthread*)pParam;
	if ( pThread != 0 )
	{
		object* pObject = pThread->get_object();
		if ( pObject != 0 )
			pObject->run();
	}
	return pThread;
}

void* workthread::call_back_ext(void *pParam)
{
	workthread *pThread = (workthread*)pParam;
	if ( pThread != 0 )
	{
		object* pObject = pThread->get_object();
		if ( pObject != 0 )
			pObject->run_ext();
	}
	return pThread;
}

void* workthread::call_back_service(void *pParam)
{
	workthread *pThread = (workthread*)pParam;
	if ( pThread != 0 )
	{
		object* pObject = pThread->get_object();
		if ( pObject != 0 )
			pObject->run_service();
	}
	return pThread;
}

}

