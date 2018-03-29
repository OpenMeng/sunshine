#ifndef __SHAREMEMCLIENT_H___
#define __SHAREMEMCLIENT_H__

#include <stdlib.h> 
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <sys/shm.h>

#include "object.h"
#include "hashtable.h"
#include "comparer.h"
#include "array.h"
#include "semrwlock.h"
#include "memleak.h"

namespace util
{

    /////////////////////////////////////////////////////////////////////////
    //// class sharememclient 内存数据查询

    class sharememclient 
    {
    protected:
        hashtable**	_pHashIndex;
        array**		_pArrayData;

#if defined(HPUX)
        static  pthread_mutex_t  _mutex;
        static  char*		_pDataAddr[MAX_MEM_BLKS];
        static  char*		_pHashAddr[MAX_MEM_BLKS];
        static  super_block*	_pobjshm;
        static  int 	_nLinkNum;
#else
        char*		_pDataAddr[MAX_MEM_BLKS];
        char*		_pHashAddr[MAX_MEM_BLKS];
        super_block*	_pobjshm;
#endif
        semrwlock*	_pShmRWLock;

        int			_nSize;
        int			_nCurrSize;
        int			_nTotalObjBlkNum;
        int			_nTotalHashBlkNum;
        int			_nShmRecordSize;
        key_t		_nIpcKey;

    private:
        int alloc_index_mem();
        int alloc_object_mem();
        int alloc_mutex();
        int alloc_shm();
    public:
        sharememclient();
        ~sharememclient();

        int initialize();

        semrwlock* get_rwlock();
        super_block* get_manageshm();

        int get_object_addr(int nObjectIndex,array*& pArray);
        int get_object_index(int nObjectIndex,hashtable*& pHash);
        int get_object_index_mode(int nObjectIndex);
        int get_object_record_size(int nObjectIndex);
        int get_object_index_size(int nObjectIndex);

        int update_object_addr(int nObjectIndex,int nCurrDataCount);

        virtual int search(int nObjectIndex,const void *key,void* presult,int nresult);
        virtual int search(int nObjectIndex,const void *key,void* presult);

    protected:
        hashtable* construct_index(int nObjectIndex,array* pArray);
        virtual array* construct_object(int nObjectIndex);
    };

}
#endif


