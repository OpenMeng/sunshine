#ifndef __SHAREMEMSERVER_H___
#define __SHAREMEMSERVER_H__

#include <stdlib.h> 
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <sys/shm.h>
#include <iostream>

#include "object.h"
#include "hashtable.h"
#include "comparer.h"
#include "array.h"
#include "semrwlock.h"
#include "memleak.h"
#include "user_exception.h"

using namespace std;

namespace util 
{

	///////////////////////////////////////////////////////////////////////
	//// class sharememserver 共享内存数据维护程序
	class sharememserver : public object
	{
	protected:
		hashtable**	_pHashIndex;
		array**		_pArrayData;
		char*		_pHashAddr[MAX_MEM_BLKS];
		char*		_pDataAddr[MAX_MEM_BLKS];

		super_block*	_pobjshm;
		semrwlock*	_pShmRWLock;
		
		int		_nSize;
		int		_nCurrSize;
		int		_nShmRecordSize;
		key_t	_nIpcKey;
		int		_nManageShmId;
		int		_nHashShmId[MAX_MEM_BLKS];
		int		_nDataShmId[MAX_MEM_BLKS];

		int*	_nObjectCount;
		int*	_nObjectSize;
		int*	_nObjectIndexMode;
		int*	_nHashCount;
	private:
		int alloc_index_mem();
		int alloc_object_mem();
		int alloc_mutex();
		int alloc_shm();
	public:
		sharememserver();
		virtual ~sharememserver();

		void dump_object_size();
		void dump_super_block();

		semrwlock* get_rwlock();

		virtual int load_data(int nAction);
		virtual int refresh_data(const vector<int>& tableids,int nAction);

		int initialize();
		int get_object_addr(int nObjectIndex,array*& pArray);
		int get_object_index(int nObjectIndex,hashtable*& pHash);
		int get_object_index_mode(int nObjectIndex);
		int update_object_addr(int nObjectIndex,int nCurrDataCount);
		int get_object_record_size(int nObjectIndex);
	protected:
		hashtable* construct_index(int nObjectIndex,array* pArray);
		virtual array* construct_object(int nObjectIndex);
	};

}
#endif

