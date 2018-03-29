#ifndef __MEMLEAK_H__
#define __MEMLEAK_H__

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>


#ifdef MEM_DEBUG
void* operator new( size_t nSize, char* pszFileName, int nLineNum );
void* operator new[]( size_t nSize, char* pszFileName, int nLineNum );

#ifdef HPUX
void operator delete( void *ptr ) __THROWSPEC_NULL;
void operator delete[]( void *ptr ) __THROWSPEC_NULL;
#else
void operator delete( void *ptr, char* pszFileName, int nLineNum ) throw();
void operator delete[]( void *ptr, char* pszFileName, int nLineNum ) throw();
#endif

#define DEBUG_NEW     new( __FILE__, __LINE__ )
#define DEBUG_DELETE  printf("%s:%d:",__FILE__,__LINE__);delete

#endif			

#endif
