#include "semmutex.h"

#if defined(MEM_DEBUG)
#define new 		DEBUG_NEW
#define delete 	DEBUG_DELETE
#endif

namespace util
{

semmutex::semmutex()
{
  union semun 
  { 
    int val; 
    struct semid_ds *buf; 
    ushort *array; 
  } arg; 

  if((_nsemid = ::semget(IPC_PRIVATE, 1, 0600 | IPC_CREAT)) < 0 )
  {
    throw bad_msg(__FILE__,__LINE__,276,"semget failed.");
  }

  arg.val = 0;
  if(::semctl(_nsemid, 0, SETVAL, arg) < 0)
  {
    throw bad_msg(__FILE__,__LINE__,277,"semctl failed.");
  }
}

semmutex::~semmutex()
{
  if ( ::semctl(_nsemid, 0, IPC_RMID) < 0 )
    throw bad_msg(__FILE__,__LINE__,277,"semctl failed.");
}

void semmutex::lock(void)
{
  struct sembuf ops[] = {{0, -1, 0}};

  if ( ::semop(_nsemid, ops, 1) < 0 )
  {
    throw bad_msg(__FILE__,__LINE__,278,"semop failed.");
  }
}

bool semmutex::try_lock(void)
{
  struct sembuf ops[] = {{0, -1, IPC_NOWAIT}};

  return (::semop(_nsemid, ops, 1) == EAGAIN) ? TRUE : FALSE;
}

void semmutex::unlock(void)
{
  struct sembuf ops[] = {{0, 1, 0}};

  if ( ::semop(_nsemid, ops, 1) < 0 )
  {
    throw bad_msg(__FILE__,__LINE__,278,"semop failed.");
  }
}

int semmutex::get_value(void)
{
  return ::semctl(_nsemid,0,GETVAL);
}

}
