#include "sharememserver.h"

#if defined(MEM_DEBUG)
#define new 		DEBUG_NEW
#define delete 	DEBUG_DELETE
#endif

namespace util
{

	sharememserver::sharememserver()
	{
		int i;
		_nIpcKey = -1;
		_nShmRecordSize = sizeof(struct super_block);
		_nSize = 0;	
		_pobjshm = 0;
		_pShmRWLock = 0;
		_nManageShmId = -1;

		for( i = 0 ; i < MAX_MEM_BLKS; i ++ )
		{
			_nDataShmId[i] = -1;
			_nHashShmId[i] = -1;
			_pDataAddr[i] = 0;
			_pHashAddr[i] = 0;
		}
	}

	sharememserver::~sharememserver()
	{
		int i;
		for(i = 0 ; i < MAX_MEM_BLKS; i ++ )
		{
			if ( _nDataShmId[i] >= 0 )
			{
				if ( ::shmdt(_pDataAddr[i]) < 0 )
					throw bad_msg(__FILE__,__LINE__,285,"shmdt failed.");
				if ( ::shmctl(_nDataShmId[i],IPC_RMID,0) < 0 )
					throw bad_msg(__FILE__,__LINE__,285,"shmctl failed.");
				_pDataAddr[i] = 0;
			}
			if ( _nHashShmId[i] >= 0 )
			{
				if ( ::shmdt(_pHashAddr[i]) < 0 )
					throw bad_msg(__FILE__,__LINE__,285,"shmdt failed.");
				if ( ::shmctl(_nHashShmId[i],IPC_RMID,0) < 0 )
					throw bad_msg(__FILE__,__LINE__,285,"shmctl failed.");
				_pHashAddr[i] = 0;
			}
		}

		if ( _nManageShmId > 0 )
		{
			if ( ::shmdt((char*)_pobjshm) < 0 )
				throw bad_msg(__FILE__,__LINE__,285,"shmdt failed.");
			if ( ::shmctl(_nManageShmId,IPC_RMID,0) < 0 )
				throw bad_msg(__FILE__,__LINE__,285,"shmctl failed.");
			_pobjshm = 0;
		}

		if ( _pShmRWLock != 0 )
		{
			delete _pShmRWLock;
			_pShmRWLock = 0;
		}
	}

	void sharememserver::dump_object_size()
	{
		for( int i = 0; i < _nSize ; i ++ )
			cout << i << "," << _nObjectSize[i] << endl;
	}

	void sharememserver::dump_super_block()
	{
		int i = 0,j = 0;
		//dump obj
		cout  << "\n\ndump object memory...\n";
		for( i = 0; i < _nSize; i ++ )
		{
			printf("tableindex:[%d]\n",i);
			printf("obj_total_blk_num\tindex_mode\tobj_blk_num\tobj_size\tobj_count\tobj_curr_count\n");
			printf("------------------------------------------------------------------------------------------------------\n");
			printf("[%-8d]\t\t[%-8d]\t[%-8d]\t[%-8d]\t[%-8d]\t[%-8d]\n",_pobjshm[i].obj_total_blk_num,
				_pobjshm[i].index_mode,_pobjshm[i].obj_blk_num, _pobjshm[i].obj_size,
				_pobjshm[i].obj_count,_pobjshm[i].obj_curr_count);
			printf("\tblk_index\tblk_offset\tobj_start\tobj_end\n");
			for ( j = 0 ; j < MAX_MEM_BLKS ; j ++ )
			{
				printf("\t[%-8d]\t[%-8d]\t[%-8d]\t[%-8d]\n",_pobjshm[i].obj_mem[j].blk_index,_pobjshm[i].obj_mem[j].blk_offset,
					_pobjshm[i].obj_mem[j].obj_start,_pobjshm[i].obj_mem[j].obj_end);
			}
		}
		cout << "\n\ndump hash memory...\n";
		//dump hash
		for( i = 0; i < _nSize; i ++ )
		{
			if ( _nObjectIndexMode[i] == HASH_TABLE )
			{
				printf("tableindex:[%d]\n",i);
				printf("hash_total_blk_num\thash_blk_num\tbucket_size\thash_size\n");
				printf("------------------------------------------------------------------\n");
				printf("[%-8d]\t\t[%-8d]\t[%-8d]\t[%-8d]\n",_pobjshm[i].hash_total_blk_num,
					_pobjshm[i].hash_blk_num,_pobjshm[i].bucket_size,_pobjshm[i].hash_size);
				printf("\tblk_index\tblk_offset\tobj_start\tobj_end\n");
				for ( j = 0 ; j < MAX_MEM_BLKS ; j ++ )
				{
					printf("\t[%-8d]\t[%-8d]\t[%-8d]\t[%-8d]\n",_pobjshm[i].hash_mem[j].blk_index,_pobjshm[i].hash_mem[j].blk_offset,
						_pobjshm[i].hash_mem[j].hash_start,_pobjshm[i].hash_mem[j].hash_end);
				}
			}
		}
	}

	int sharememserver::alloc_shm()
	{
		_nManageShmId = ::shmget( _nIpcKey, 0, 0666 );

		if ( _nManageShmId >= 0 )
		{
			if ( ::shmctl(_nManageShmId,IPC_RMID,0) < 0 )
				throw bad_msg(__FILE__,__LINE__,285,"shmctl failed.");
			_nManageShmId = -1;
		}

		if( _nManageShmId < 0 ) 
		{ 
			_nManageShmId = ::shmget( _nIpcKey, _nSize* _nShmRecordSize, IPC_CREAT | 0666); 
			if( _nManageShmId < 0 ) 
			{
				throw bad_msg(__FILE__,__LINE__,283,"create super block share memory failed."); 
			}
			_pobjshm =  (super_block*) ::shmat( _nManageShmId, 0, 0 ); 
#if defined(HPUX)
			if( _pobjshm == SHM_FAILED ) 
			{
				throw bad_msg(__FILE__,__LINE__,284,"attach super block share memory failed."); 
			}
#else
			if( _pobjshm < 0 ) 
			{
				throw bad_msg(__FILE__,__LINE__,284,"attach super block share memory failed."); 
			}
#endif
		}

		::memset((char*)_pobjshm,0, _nSize* _nShmRecordSize);
		for( int i = 0 ; i < _nSize ; i ++ )
		{
			_pobjshm[i].obj_total_blk_num = 0;		
			_pobjshm[i].hash_total_blk_num = 0;		
			_pobjshm[i].index_mode = _nObjectIndexMode[i];
			_pobjshm[i].obj_curr_count = 0;
			_pobjshm[i].obj_count = _nObjectCount[i];
			_pobjshm[i].obj_size = _nObjectSize[i];
			_pobjshm[i].obj_blk_num = 0;
			_pobjshm[i].hash_size = _nHashCount[i];
			_pobjshm[i].hash_blk_num = 0;
			_pobjshm[i].bucket_size = sizeof(struct hashbucket);

			for ( int j = 0 ; j < MAX_MEM_BLKS ; j ++ )
			{
				_pobjshm[i].obj_mem[j].blk_index = 0;
				_pobjshm[i].obj_mem[j].blk_offset = 0;
				_pobjshm[i].obj_mem[j].obj_start = 0;
				_pobjshm[i].obj_mem[j].obj_end = 0;

				_pobjshm[i].hash_mem[j].blk_index = 0;
				_pobjshm[i].hash_mem[j].blk_offset = 0;
				_pobjshm[i].hash_mem[j].hash_start = 0;
				_pobjshm[i].hash_mem[j].hash_end = 0;
			}
		}
		return 0;
	}

	int sharememserver::alloc_object_mem()
	{
		ostringstream err_msg;
		int i,j;	
		int offset = 0;
		int blk_index = 0;
		int blk_num = 0;
		int obj_blk_num = 0;
		int blk_obj_cnt = 0;
		int blk_obj_cnt_lft = 0;
		int obj_cnt_lft = 0;
		int obj_total_cnt = 0;
		int blk_lft = 0;
		int  obj_blk_index = -1;

		for( i = 0 ; i < _nSize; i ++ )
		{
			obj_blk_index = -1;
			obj_blk_num = 0;

			//要申请的对象记录数量
			obj_total_cnt = _pobjshm[i].obj_count;
			if ( obj_total_cnt == 0 )
				continue;

			//每块所能存储的对象的个数
			blk_obj_cnt = (int)( BLOCK_SIZE / _pobjshm[i].obj_size );

			//地址空间对齐
			offset = (( offset + 7 ) >> 3 ) << 3;

			//每块剩余空间所能存储的对象的个数
			assert( _pobjshm[i].obj_size > 0 );
			blk_obj_cnt_lft = (int)(( BLOCK_SIZE - offset ) / _pobjshm[i].obj_size);

			if ( blk_obj_cnt_lft > 0 )
			{
				blk_obj_cnt_lft = obj_total_cnt > blk_obj_cnt_lft  ? blk_obj_cnt_lft : obj_total_cnt;
				obj_blk_num = 1;
				obj_blk_index += 1;

				_pobjshm[i].obj_blk_num = 1;
				_pobjshm[i].obj_mem[0].blk_index = blk_index;
				_pobjshm[i].obj_mem[0].blk_offset = offset;
				_pobjshm[i].obj_mem[0].obj_start = 0;
				_pobjshm[i].obj_mem[0].obj_end = blk_obj_cnt_lft - 1;
				offset += blk_obj_cnt_lft * _pobjshm[i].obj_size;
			}

			//计算该对象剩余要存储的记录数
			//若剩余块数为0，则只需一块内存就可以存储
			obj_cnt_lft = obj_total_cnt - blk_obj_cnt_lft;
			if ( obj_cnt_lft > 0 )
			{
				offset = 0;
				//计算除去第一块，还需要的块数
				blk_num = (int)(obj_cnt_lft / blk_obj_cnt); 
				blk_lft = obj_cnt_lft % blk_obj_cnt;
				obj_blk_num += blk_num +  (blk_lft > 0  ? 1 : 0);

				if ( obj_blk_num > MAX_MEM_BLKS )
				{
					err_msg.str("");
					err_msg << "MAX_MEM_BLKS too small,[" << obj_blk_num <<"] > [" << MAX_MEM_BLKS << "]";
					throw bad_msg(__FILE__,__LINE__,286,err_msg.str());
				}
				_pobjshm[i].obj_blk_num = obj_blk_num;		

				for ( j = 0 ; j < blk_num ; j ++ )
				{
					//开始新的块
					blk_index += 1;
					obj_blk_index += 1;
					_pobjshm[i].obj_mem[obj_blk_index].blk_index = blk_index;
					_pobjshm[i].obj_mem[obj_blk_index].blk_offset = 0;
					_pobjshm[i].obj_mem[obj_blk_index].obj_start = blk_obj_cnt_lft + j * blk_obj_cnt;
					_pobjshm[i].obj_mem[obj_blk_index].obj_end = blk_obj_cnt_lft + (j + 1) * blk_obj_cnt - 1;
				}

				if ( blk_lft > 0 )
				{
					//开始新的块
					blk_index += 1;
					obj_blk_index += 1;
					_pobjshm[i].obj_mem[obj_blk_index].blk_index = blk_index;
					_pobjshm[i].obj_mem[obj_blk_index].blk_offset = 0;
					_pobjshm[i].obj_mem[obj_blk_index].obj_start = blk_obj_cnt_lft + blk_num * blk_obj_cnt;
					_pobjshm[i].obj_mem[obj_blk_index].obj_end = obj_total_cnt - 1;
					offset = blk_lft * _pobjshm[i].obj_size;
				}
			}
		}

		if (  (blk_index + 1 ) > MAX_MEM_BLKS )
		{
			err_msg.str("");
			err_msg << "MAX_MEM_BLKS too small,[" << (blk_index + 1) << "] > [" << MAX_MEM_BLKS << "]";
			throw bad_msg(__FILE__,__LINE__,286,err_msg.str());
		}

		for( i = 0 ; i < _nSize ; i ++ )
			_pobjshm[i].obj_total_blk_num = blk_index + 1;

		for( i = 0 ; i < blk_index ; i ++ )
		{
			_nDataShmId[i] = ::shmget( _nIpcKey + 2 * MAX_MEM_BLKS + i, 0, 0666 ); 	
			if ( _nDataShmId[i] >= 0 )
			{
				if (::shmctl(_nDataShmId[i],IPC_RMID,0) < 0)
					throw bad_msg(__FILE__,__LINE__,285,"shmctl failed.");
				_nDataShmId[i] = -1;
			}

			_nDataShmId[i] = ::shmget( _nIpcKey + 2 * MAX_MEM_BLKS + i, BLOCK_SIZE, IPC_CREAT | 0666); 
			if( _nDataShmId[i] < 0 ) 
				throw bad_msg(__FILE__,__LINE__,283,"create data share memory failed.");
			
			_pDataAddr[i] =  (char*)::shmat( _nDataShmId[i], 0, 0 ); 
#if defined(HPUX)
			if( _pDataAddr[i] == SHM_FAILED )
				throw bad_msg(__FILE__,__LINE__,284,"attach data share memory failed.");
#else
			if( _pDataAddr[i] < 0 )
				throw bad_msg(__FILE__,__LINE__,284,"attach data share memory failed.");
#endif
		}

		//地址空间对齐
		offset = (( offset + 7 ) >> 3 ) << 3;
		if ( offset > 0 )
		{
			_nDataShmId[blk_index] = ::shmget( _nIpcKey + 2 * MAX_MEM_BLKS + blk_index, 0, 0666 ); 	
			if ( _nDataShmId[blk_index] >= 0 )
			{
				if ( ::shmctl(_nDataShmId[blk_index],IPC_RMID,0) < 0 )
					throw bad_msg(__FILE__,__LINE__,285,"shmctl failed.");
				_nDataShmId[blk_index] = -1;
			}

			_nDataShmId[blk_index] = ::shmget( _nIpcKey + 2 * MAX_MEM_BLKS + blk_index, offset, IPC_CREAT | 0666); 
			if( _nDataShmId[blk_index] < 0 ) 
				throw bad_msg(__FILE__,__LINE__,283,"create data share memory failed.");

			_pDataAddr[blk_index] =  (char*)::shmat( _nDataShmId[blk_index], 0, 0 ); 
#if defined(HPUX)
			if( _pDataAddr[i] == SHM_FAILED )
				throw bad_msg(__FILE__,__LINE__,284,"attach data share memory failed.");
#else
			if( _pDataAddr[i] < 0 )
				throw bad_msg(__FILE__,__LINE__,284,"attach data share memory failed.");
#endif
		}

		for( i = 0 ; i < _nSize ; i ++ )
		{
			_pArrayData[i] = construct_object(i + 1);
		}

		return 0;
	}

	array* sharememserver::construct_object(int nObjectIndex)
	{
		return new array(_pDataAddr,&_pobjshm[nObjectIndex - 1],nObjectIndex);
	}

	int sharememserver::alloc_index_mem()
	{
		ostringstream err_msg;
		int i,j;	
		int offset = 0;
		int blk_index = 0;
		int blk_num = 0;
		int hash_blk_num = 0;
		int blk_hash_cnt = 0;
		int blk_hash_cnt_lft = 0;
		int hash_cnt_lft = 0;
		int hash_total_cnt = 0;
		int  hash_blk_index = -1;
		int blk_lft = 0;
		bool is_allocated = false;

		for( i = 0 ; i < _nSize ; i ++ )
		{
			if (  _pobjshm[i].index_mode  != HASH_TABLE ) 
				continue;

			//如果运行到这儿,说明分配过内存了.当然配置的hash_count得大于0才行.
			if(_nHashCount[i] > 0)
			{
				is_allocated = true;
			}

			hash_blk_index = -1;
			hash_blk_num = 0;

			//每块所能存储的对象的个数
			blk_hash_cnt = (int)( BLOCK_SIZE / _pobjshm[i].bucket_size);

			//索引节点的总数
			hash_total_cnt = _pobjshm[i].hash_size;

			//地址空间对齐
			offset = (( offset + 7 ) >> 3 ) << 3;

			//每块剩余空间所能存储的对象的个数
			blk_hash_cnt_lft = (int)(( BLOCK_SIZE - offset ) / _pobjshm[i].bucket_size);

			if ( blk_hash_cnt_lft > 0 )
			{
				blk_hash_cnt_lft = hash_total_cnt > blk_hash_cnt_lft ? blk_hash_cnt_lft : hash_total_cnt;

				_pobjshm[i].hash_blk_num = 1;
				hash_blk_num += 1;
				hash_blk_index += 1;

				_pobjshm[i].hash_mem[0].blk_index = blk_index;
				_pobjshm[i].hash_mem[0].blk_offset = offset;
				_pobjshm[i].hash_mem[0].hash_start = 0;
				_pobjshm[i].hash_mem[0].hash_end = blk_hash_cnt_lft - 1;
				offset += blk_hash_cnt_lft * _pobjshm[i].bucket_size;
			}

			//计算该对象剩余要存储的记录数
			//若剩余块数为0，则只需一块内存就可以存储
			hash_cnt_lft = hash_total_cnt - blk_hash_cnt_lft;
			if ( hash_cnt_lft > 0 )
			{
				offset = 0;
				//计算除去第一块，还需要的块数
				blk_num = (int)(hash_cnt_lft / blk_hash_cnt); 
				blk_lft = hash_cnt_lft % blk_hash_cnt;
				hash_blk_num += blk_num + ( blk_lft > 0  ? 1 : 0);

				if ( hash_blk_num > MAX_MEM_BLKS )
				{
					err_msg.str("");
					err_msg << "MAX_MEM_BLKS too small,[" << hash_blk_num << "] > [" << MAX_MEM_BLKS << "]";
					throw bad_msg(__FILE__,__LINE__,286,err_msg.str());
				}
				_pobjshm[i].hash_blk_num = hash_blk_num;		

				for ( j = 0 ; j < blk_num ; j ++ )
				{
					//开始新的块
					blk_index += 1;
					hash_blk_index += 1;
					_pobjshm[i].hash_mem[hash_blk_index].blk_index = blk_index;
					_pobjshm[i].hash_mem[hash_blk_index].blk_offset = 0;
					_pobjshm[i].hash_mem[hash_blk_index].hash_start = blk_hash_cnt_lft + j * blk_hash_cnt;
					_pobjshm[i].hash_mem[hash_blk_index].hash_end = blk_hash_cnt_lft + (j + 1) * blk_hash_cnt - 1;
				}

				if ( blk_lft > 0 )
				{
					//开始新的块
					blk_index += 1;
					hash_blk_index += 1;
					_pobjshm[i].hash_mem[hash_blk_index].blk_index = blk_index;
					_pobjshm[i].hash_mem[hash_blk_index].blk_offset = 0;
					_pobjshm[i].hash_mem[hash_blk_index].hash_start = blk_hash_cnt_lft + blk_num * blk_hash_cnt;
					_pobjshm[i].hash_mem[hash_blk_index].hash_end =hash_total_cnt - 1;
					offset = blk_lft * _pobjshm[i].bucket_size;
				}
			}
		}

		if(is_allocated)
		{
			if (  (blk_index + 1 ) > MAX_MEM_BLKS )
			{
				err_msg.str("");
				err_msg << "MAX_MEM_BLKS too small,[" << (blk_index + 1) << "] > [" << MAX_MEM_BLKS << "]";
				throw bad_msg(__FILE__,__LINE__,286,err_msg.str());
			}

			for( i = 0 ; i < _nSize ; i ++ )
				_pobjshm[i].hash_total_blk_num = blk_index + 1;

			for ( i = 0 ; i < blk_index ; i ++ )
			{
				_nHashShmId[i] = ::shmget( _nIpcKey + 4  * MAX_MEM_BLKS + i, 0, 0666 ); 

				if ( _nHashShmId[i] >= 0 )
				{
					if ( ::shmctl(_nHashShmId[i],IPC_RMID,0) < 0 )
						throw bad_msg(__FILE__,__LINE__,285,"shmctl failed.");
					_nHashShmId[i] = -1;
				}

				_nHashShmId[i] = ::shmget( _nIpcKey + 4  * MAX_MEM_BLKS + i, BLOCK_SIZE, IPC_CREAT | 0666); 
				if( _nHashShmId[i] < 0 ) 
				{
					throw bad_msg(__FILE__,__LINE__,283,"create index share memory failed.");
				}
				_pHashAddr[i] = (char*)::shmat( _nHashShmId[i], 0, 0 ); 
#if defined(HPUX)
				if( _pHashAddr[blk_index] == SHM_FAILED ) 
				{
					throw bad_msg(__FILE__,__LINE__,284,"attach index share memory failed.");
				}
#else
				if( _pHashAddr[blk_index] < 0 ) 
				{	
					throw bad_msg(__FILE__,__LINE__,284,"attach index share memory failed.");
				}
#endif			
			}
			//地址空间对齐
			offset = (( offset + 7 ) >> 3 ) << 3;
			if ( offset > 0 )
			{
				_nHashShmId[blk_index] = ::shmget( _nIpcKey + 4  * MAX_MEM_BLKS + blk_index, 0, 0666 ); 

				if ( _nHashShmId[blk_index] >= 0 )
				{
					if ( ::shmctl(_nHashShmId[blk_index],IPC_RMID,0) < 0 )
						throw bad_msg(__FILE__,__LINE__,285,"shmctl failed.");
					_nHashShmId[blk_index] = -1;
				}

				_nHashShmId[blk_index] = ::shmget( _nIpcKey + 4  * MAX_MEM_BLKS + blk_index, offset, IPC_CREAT | 0666); 
				if( _nHashShmId[blk_index] < 0 ) 
				{
					throw bad_msg(__FILE__,__LINE__,283,"create index share memory failed.");
				}
				_pHashAddr[blk_index] = (char*)::shmat( _nHashShmId[blk_index], 0, 0 ); 
#if defined(HPUX)
				if( _pHashAddr[blk_index] == SHM_FAILED ) 
				{
					throw bad_msg(__FILE__,__LINE__,284,"attach index share memory failed.");
				}
#else
				if( _pHashAddr[blk_index] < 0 ) 
				{
					throw bad_msg(__FILE__,__LINE__,284,"attach index share memory failed.");
				}
#endif			
			}
		}

		for ( i = 0 ; i < _nSize; i ++ )
		{
			if ( _pobjshm[i].index_mode== HASH_TABLE )
			{
				_pHashIndex[i] = construct_index( i + 1, _pArrayData[i]);
			}
		}
		return 0;
	}

	hashtable* sharememserver::construct_index(int nObjectIndex,array* pArray)
	{
		return new hashtable(1,_pHashAddr,&_pobjshm[nObjectIndex - 1],pArray);
	}

	int sharememserver::alloc_mutex()
	{
		_pShmRWLock = new semrwlock(_nIpcKey);
		_pShmRWLock->create_rwlock();
		return 0;
	}

	int sharememserver::initialize()
	{	
		if ( alloc_mutex() < 0 )
		{
			return -1;
		}

		if ( alloc_shm() < 0 )
		{
			return -1;
		}

		if ( alloc_object_mem() < 0 )
		{
			return -1;
		}

		if ( alloc_index_mem() < 0 )
		{
			return -1;
		}

		return 0;
	}

	semrwlock *sharememserver::get_rwlock()
	{
		return _pShmRWLock;
	}

	int sharememserver::get_object_addr(int nObjectIndex,array*& pArray)
	{
		if ( nObjectIndex < 1 || nObjectIndex > _nSize )
		{
			return -1;
		}
		pArray = _pArrayData[nObjectIndex - 1];		
		return 0;
	}

  int sharememserver::get_object_record_size(int nObjectIndex)
  {
    if ( nObjectIndex < 1 || nObjectIndex > _nSize )
    {
      return -1;
    }
    return _pobjshm[nObjectIndex - 1].obj_size;
  }

	int sharememserver::get_object_index(int nObjectIndex,hashtable*& pHash)
	{
		if ( nObjectIndex < 1 || nObjectIndex > _nSize )
		{
			return -1;
		}
		pHash = _pHashIndex[nObjectIndex - 1];		
		return 0;
	}

	int sharememserver::get_object_index_mode(int nObjectIndex)
	{
		if ( nObjectIndex < 1 || nObjectIndex > _nSize )
		{
			return -1;
		}

		return _nObjectIndexMode[nObjectIndex - 1];
	}

	int sharememserver::update_object_addr(int nObjectIndex,int nCurrDataCount)
	{
		if ( nObjectIndex < 1 || nObjectIndex > _nSize )
		{
			return -1;
		}
		_pobjshm[nObjectIndex - 1].obj_curr_count= nCurrDataCount;
		return -1;
	}

	int sharememserver::load_data(int nAction)
	{
		return 0;
	}

	int sharememserver::refresh_data(const vector<int>& tableids,int nAction)
	{
		return 0;
	}

}

