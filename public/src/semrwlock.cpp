#include "semrwlock.h"

#if defined(MEM_DEBUG)
#define new 		DEBUG_NEW
#define delete 	DEBUG_DELETE
#endif


namespace util
{
  semrwlock::semrwlock(int nIpcKey)
    : _nIpcKey(nIpcKey)
  {
    _bServer = false;
    for(int i = 0 ; i < LOCK_NUM ; i ++ )
      _nSemId[i] = -1;
  }

  semrwlock::~semrwlock()
  {
    for( int i = 0 ; i < LOCK_NUM ; i ++ )
    {
      if ( _nSemId[i] >= 0 && _bServer )
      {
        if ( ::semctl(_nSemId[i],0,IPC_RMID) < 0 )
          throw bad_msg(__FILE__,__LINE__,277,"semctl failed.");
      }
    }
  }

  int semrwlock::create_rwlock()
  {
    _bServer = true;
    int ntry = 0;
    ostringstream err_msg;

    for ( int i = 0 ; i < LOCK_NUM ; i ++ )
    {
      _nSemId[i] = ::semget(_nIpcKey + i, 0, 0600);

      if ( _nSemId[i] >= 0 )
      {
        if ( ::semctl(_nSemId[i],0,IPC_RMID) < 0 )
        {
          throw bad_msg(__FILE__,__LINE__,277,"remove semphore failed.");
        }
        _nSemId[i] = -1;
      }

      if ( _nSemId[i] < 0 )
      {
        while ( (_nSemId[i] = ::semget(_nIpcKey + i, 1, IPC_CREAT|0600)) < 0 )
        {
          usleep(10);
          ntry ++;
          if ( ntry > 1000 )
          {
            err_msg.str("");
            err_msg << __FILE__ << ": "<<__LINE__ <<  ":semget failed. key = " << _nIpcKey + i;
            throw (__FILE__,__LINE__,276,err_msg.str());
          }
        }

        union semun {
          int val;
          struct semid_ds *buf;
          ushort *array;
        }arg;

        if ( i == WRITER_COUNT )
            arg.val = 0;//writer count
        else if ( i == READER_COUNT )
          arg.val = 0;//writer count
        else  if ( i == MUTEX_LOCK )
          arg.val = 1;//writer count
        while ( semctl(_nSemId[i],0,SETVAL,arg) < 0 )
        {
          usleep(10);
          ntry ++;
          if ( ntry > 1000 )
          {
            err_msg.str("");
            err_msg << __FILE__ << ": "<<__LINE__ <<  ":semctl failed. key = " << _nIpcKey + i;
            throw (__FILE__,__LINE__,277,err_msg.str());
          }
        }
      }
    }

    return 0;
  }

  int semrwlock::get_rwlock()
  {
    _bServer = FALSE;
    int ntry = 0;
    ostringstream err_msg;

    for( int i = 0 ; i < LOCK_NUM ; i ++ )
    {
      while ( (_nSemId[i] = semget(_nIpcKey + i, 0, 0600) ) < 0 )
      {
        dout << i << "get_rwlock failed.\n";
        usleep(10);
        ntry ++;
        if ( ntry > 1000 )
        {
          err_msg.str("");
          err_msg << __FILE__ << ": "<<__LINE__ << ":semget failed. key = " << _nIpcKey + i;
          throw (__FILE__,__LINE__,276,err_msg.str());
        }
      }
    }

    return 0;
  }

  int semrwlock::lock(int nSemNo)
  {
    int ntry = 0;
    ostringstream err_msg;

    struct sembuf ops[] = {{0,-1, SEM_UNDO}};

    if ( _nSemId[nSemNo] < 0 )
      throw bad_msg(__FILE__,__LINE__,280,"invalid semid.");

    if ( nSemNo < 0 || nSemNo > 2 )
      throw bad_msg(__FILE__,__LINE__,279,"invalid sem no.");

    int value = -1;
    while( (value = semop(_nSemId[nSemNo],ops,1)) < 0 )
    {
      dout << nSemNo << "," << _nSemId[nSemNo] << "lock failed.\n";
      usleep(10);
      ntry ++;
      if ( ntry > 1000 )
      {
        err_msg.str("");
        err_msg << __FILE__ << ": "<<__LINE__ << ":semop failed. nSemNo = " << nSemNo;
        throw (__FILE__,__LINE__,278,err_msg.str());
      }
    }
    return value;
  }

  int semrwlock::unlock(int nSemNo)
  {
    int ntry = 0;
    ostringstream err_msg;

    struct sembuf ops[] = {{0, 1, SEM_UNDO}};

    if ( _nSemId[nSemNo] < 0 )
      throw bad_msg(__FILE__,__LINE__,280,"invalid semid.");

    if ( nSemNo < 0 || nSemNo > 2 )
      throw bad_msg(__FILE__,__LINE__,279,"invalid sem no.");

    int value = -1;
    while( (value = semop(_nSemId[nSemNo],ops,1)) < 0 )
    {
      dout << nSemNo << "," << _nSemId[nSemNo] << " unlock failed.\n";
      usleep(10);
      ntry ++;
      if ( ntry > 1000 )
      {
        err_msg.str("");
        err_msg << __FILE__ << ": "<<__LINE__ << ":semop failed.nSemNo = " << nSemNo;
        throw bad_msg(__FILE__,__LINE__,278,err_msg.str());
      }
    }

    return 0;
  }

  int semrwlock::get_value(int nSemNo)
  {
    int nReaders = 0;
    int ntry = 0;
    ostringstream err_msg;

    if ( _nSemId[nSemNo] < 0 )
      return -1;

    if ( nSemNo < 0 || nSemNo > 2 )
      throw bad_msg(__FILE__,__LINE__,279,"invalid sem no.");

    while ( (nReaders = semctl(_nSemId[nSemNo],0,GETVAL)) < 0 )
    {
      dout << nSemNo << "," << _nSemId[nSemNo] << " get_value failed.\n";
      usleep(10);
      ntry ++;
      if ( ntry > 1000 )
      {
        err_msg.str("");
        err_msg << __FILE__ << ": "<<__LINE__ << ":semctl failed.nSemNo = " << nSemNo;
        throw bad_msg(__FILE__,__LINE__,277,err_msg.str());
      }
    }
    return nReaders;
  }

  int semrwlock::read_lock()
  {
    int nwriter;
#if defined(DEBUG)
    int nreader;
#endif
    for( ; ; )
    {
      if ( lock(MUTEX_LOCK) < 0 )
        throw bad_msg(__FILE__,__LINE__,281,"invalid semid.");
      nwriter = get_value(WRITER_COUNT);
#if defined(DEBUG)
      nreader = get_value(READER_COUNT);
#endif
      if ( nwriter == 0 )
        break;
      if ( unlock(MUTEX_LOCK) < 0 )
        return -1;
    }
    if ( unlock(READER_COUNT) < 0 )
      return -1;
    if ( unlock(MUTEX_LOCK) < 0 )
      return -1;

#if defined(DEBUG)
    dout << "read_lock: writers = " << nwriter  << ": readers = " << nreader<< endl;
    print_time("read_lock");
#endif
    return 0;
  }

  int semrwlock::read_unlock()
  {
    if ( lock(READER_COUNT) < 0 )
      return -1;

#if defined(DEBUG)
    print_time("read_unlock");
#endif
    return 0;
  }

  int semrwlock::write_lock()
  {
    int nreader;
#if defined(DEBUG)
    int nwriter;
#endif
    if ( unlock(WRITER_COUNT) < 0 )
      return -1;

    for( ; ; )
    {
      if ( lock(MUTEX_LOCK) < 0 )
        return -1;
      nreader = get_value(READER_COUNT);
#if defined(DEBUG)
      nwriter = get_value(WRITER_COUNT);
#endif
      if ( nreader == 0 )
        break;
      if ( unlock(MUTEX_LOCK) < 0 )
        return -1;
    }

#if defined(DEBUG)
    dout << "write_lock: reades = " << nreader <<": writer = " << nwriter << endl;
    print_time("write_lock");
#endif
    return 0;
  }

  int semrwlock::write_unlock()
  {
    if ( lock(WRITER_COUNT) < 0 )
      return -1;
    if ( unlock(MUTEX_LOCK) < 0 )
      return -1;

#if defined(DEBUG)
    print_time("write_unlock");
#endif
    return 0;
  }

  void semrwlock::print_time(const char* func_name)
  {
    time_t now;
    struct tm now_tm;
    time(&now);
    now_tm = *localtime(&now);
    printf("[%s] %04d-%02d-%02d %02d:%02d:%02d\n",func_name,now_tm.tm_year + 1900,now_tm.tm_mon + 1,now_tm.tm_mday,now_tm.tm_hour,now_tm.tm_min,now_tm.tm_sec);
  }

}

