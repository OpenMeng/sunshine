#include "sharememclient.h" 

#if defined(MEM_DEBUG)
#define new 		DEBUG_NEW
#define delete 	DEBUG_DELETE
#endif

namespace util
{

#if defined(HPUX)
    pthread_mutex_t sharememclient::_mutex = PTHREAD_MUTEX_INITIALIZER;
    char* sharememclient::_pDataAddr[MAX_MEM_BLKS] = { 0,0,0,0, 0,0,0,0 };
    char* sharememclient::_pHashAddr[MAX_MEM_BLKS] = { 0,0,0,0, 0,0,0,0 };
    super_block* sharememclient::_pobjshm = 0;
    int sharememclient::_nLinkNum = 0;
#endif

    sharememclient::sharememclient()
    {
        _nIpcKey = -1;
        _nShmRecordSize = sizeof(struct super_block);
        _nSize = 0;	
        _pShmRWLock = 0;
        _nTotalHashBlkNum = 0;
        _nTotalObjBlkNum = 0;
        _pHashIndex = 0;
        _pArrayData = 0;
#if !defined(HPUX)
		for(int i=0; i<MAX_MEM_BLKS; i++)
		{
			_pDataAddr[i] = 0;
			_pHashAddr[i] = 0;
		}
		_pobjshm = 0;
#endif
    }

    sharememclient::~sharememclient()
    {
        int i = 0;

#if defined(HPUX)
        pthread_mutex_lock(&_mutex);
        if ( _nLinkNum == 0 )
        {
#endif
            for( i = 0 ; i < MAX_MEM_BLKS ; i ++ )
            {
                if ( _pDataAddr[i] != 0 )
                {
                    if ( ::shmdt(_pDataAddr[i]) < 0 )
                      throw bad_msg(__FILE__,__LINE__,282,"shmdt failed.");
                    _pDataAddr[i] = 0;
                }
                if ( _pHashAddr[i] != 0 )
                {
                    if ( ::shmdt(_pHashAddr[i]) < 0 )
                      throw bad_msg(__FILE__,__LINE__,282,"shmdt failed.");
                    _pHashAddr[i] = 0;
                }
            }
            if ( _pobjshm != 0 )
            {
                if ( ::shmdt((char*)_pobjshm) < 0 )
                  throw bad_msg(__FILE__,__LINE__,282,"shmdt failed.");
                _pobjshm = 0;
            }

#if defined(HPUX)
        }
        else
        {
            _nLinkNum --;
        }

        pthread_mutex_unlock(&_mutex);
#endif

        if ( _pShmRWLock != 0)
        {
            delete _pShmRWLock;
            _pShmRWLock = 0;
        }
    }

    int sharememclient::alloc_shm()
    {
        int shmid = 0; 
        int ntry = 0;
        ostringstream err_msg;

        while ( (shmid = ::shmget( _nIpcKey, 0, 0444 )) < 0 )
        {
            usleep(10);
            ntry ++;
            if ( ntry > 1000 )
            {
              err_msg.str("");
              err_msg << __FILE__ << ": "<<__LINE__ << ":shmget failed. key = " << _nIpcKey;
              throw (__FILE__,__LINE__,283,err_msg.str());
            }
        }

#if defined(HPUX)
        if ( _nLinkNum == 0 )
#endif
            _pobjshm =  (super_block*) ::shmat( shmid, 0, SHM_RDONLY); 

#if defined(HPUX)
        if( _pobjshm == SHM_FAILED || _pobjshm == 0) 
          throw bad_msg(__FILE__,__LINE__,284,"attach super block share memory failed."); 
#else
        if( _pobjshm <= 0 ) 
          throw bad_msg(__FILE__,__LINE__,284,"attach super block share memory failed."); 
#endif

        _nTotalObjBlkNum = _pobjshm[0].obj_total_blk_num;
        _nTotalHashBlkNum = _pobjshm[0].hash_total_blk_num;

        return 0;
    }

    int sharememclient::alloc_object_mem()
    {
        int shmid = 0; 
        int i;
        char* p = 0;
        int ntry = 0;
        ostringstream err_msg;

        for ( i = 0 ; i < _nTotalObjBlkNum ; i ++ )
        {
            while ( (shmid = ::shmget( _nIpcKey + 2 * MAX_MEM_BLKS + i, 0, 0444 )) < 0 )
            {
              usleep(10);
              ntry ++;
              if ( ntry > 1000 )
              {
                err_msg.str("");
                err_msg << "shmget failed. key = " << _nIpcKey + 2 * MAX_MEM_BLKS + i;
                throw (__FILE__,__LINE__,283,err_msg.str());
              }
            }

#if defined(HPUX)
            if ( _nLinkNum == 0 )
#endif
                _pDataAddr[i] = (char*)::shmat(shmid,0,SHM_RDONLY);

#if defined(HPUX)
            if ( _pDataAddr[i] == SHM_FAILED || _pDataAddr[i] == 0 )
              throw bad_msg(__FILE__,__LINE__,284,"attach data share memory failed.");
#else
            if ( _pDataAddr[i] < 0 )
              throw bad_msg(__FILE__,__LINE__,284,"attach data share memory failed.");
#endif
        }

        for( i = 0 ; i < _nSize ; i ++ )
        {
            array* p = construct_object(i + 1);
            if ( p == 0 )
                return -1;
            _pArrayData[i] = p;
            _pArrayData[i]->set_size(_pobjshm[i].obj_curr_count);
        }

        return 0;
    }

    array* sharememclient::construct_object(int nObjectIndex)
    {
        return new array(_pDataAddr,&_pobjshm[nObjectIndex - 1],nObjectIndex);
    }

    int sharememclient::alloc_index_mem()
    {
        int shmid = 0; 
        int i = 0;
        int ntry = 0;
        ostringstream err_msg;

        if ( _nTotalHashBlkNum < 1 )
            return 0;

        for ( i = 0 ; i < _nTotalHashBlkNum ; i ++ )
        {
            while ( (shmid = ::shmget( _nIpcKey + 4 * MAX_MEM_BLKS + i, 0, 0444 )) < 0 )
            {
              usleep(10);
              ntry ++;
              if ( ntry > 1000 )
              {
                err_msg.str("");
                err_msg << __FILE__ << ": "<<__LINE__ << ":shmget failed. key = " << _nIpcKey + 4 * MAX_MEM_BLKS + i;
                throw (__FILE__,__LINE__,283,err_msg.str());
              }
            }

#if defined(HPUX)
            if ( _nLinkNum == 0 )
#endif
                _pHashAddr[i] = (char*)::shmat(shmid,0,SHM_RDONLY);

#if defined(HPUX)
            if ( _pHashAddr[i] == SHM_FAILED || _pHashAddr[i] == 0 )
              throw bad_msg(__FILE__,__LINE__,284,"attach index share memory failed.");
#else
            if ( _pHashAddr[i] <= 0 )
              throw bad_msg(__FILE__,__LINE__,284,"attach index share memory failed.");
#endif
        }

        for ( i = 0 ; i < _nSize ; i ++ )
        {
            if ( _pobjshm[i].index_mode== HASH_TABLE )
            {
                hashtable* p = construct_index( i + 1, _pArrayData[i]); 
                if ( p == 0 )
                    return -1;
                _pHashIndex[i] = p;
            }
        }

        return 0;
    }

    hashtable* sharememclient::construct_index(int nObjectIndex,array* pArray)
    {
        return new hashtable(0,_pHashAddr,&_pobjshm[nObjectIndex - 1],pArray);
    }

    int sharememclient::alloc_mutex()
    {
        _pShmRWLock = new semrwlock(_nIpcKey);
        return _pShmRWLock->get_rwlock();
    }

    int sharememclient::initialize()
    {	
        if ( alloc_mutex() < 0 )
        {
            return -1;
        }

#if defined(HPUX)
        pthread_mutex_lock(&_mutex);
#endif

        if ( alloc_shm() < 0 )
        {
#if defined(HPUX)
            pthread_mutex_unlock(&_mutex);
#endif
            return -1;
        }

        if ( alloc_object_mem() < 0 )
        {
#if defined(HPUX)
          pthread_mutex_unlock(&_mutex);
#endif
            return -1;
        }

        if ( alloc_index_mem() < 0 )
        {
#if defined(HPUX)
          pthread_mutex_unlock(&_mutex);
#endif
            return -1;
        }

#if defined(HPUX)
        _nLinkNum ++;
        pthread_mutex_unlock(&_mutex);
#endif

        return 0;
    }

    semrwlock *sharememclient::get_rwlock()
    {
        return _pShmRWLock;
    }

    super_block* sharememclient::get_manageshm()
    {
        return _pobjshm;
    }

    int sharememclient::get_object_addr(int nObjectIndex,array*& pArray)
    {
        if ( nObjectIndex < 1 || nObjectIndex > _nSize)
        {
            return -1;
        }
        if( _pobjshm == 0 ) 
            return -1; 	
        pArray = _pArrayData[nObjectIndex - 1];		

        return 0;
    }

    int sharememclient::get_object_index(int nObjectIndex,hashtable*& pHash)
    {
        if ( nObjectIndex < 1 || nObjectIndex > _nSize )
        {
            return -1;
        }
        if( _pobjshm == 0 ) 
            return -1; 
        pHash = _pHashIndex[nObjectIndex - 1];			

        return 0;
    }

    int sharememclient::get_object_index_mode(int nObjectIndex)
    {
        if ( nObjectIndex < 1 || nObjectIndex > _nSize )
        {
            dout << "[get_object_index_mode] nObjectIndex = " << nObjectIndex
                << ", _nSize = " << _nSize << endl ;
            return -1;
        }

        return _pobjshm[nObjectIndex - 1].index_mode;
    }

    int sharememclient::update_object_addr(int nObjectIndex,int nCurrDataCount)
    {
        if ( nObjectIndex < 1 || nObjectIndex > _nSize )
        {
            return -1;
        }
        _pArrayData[nObjectIndex - 1]->set_size(nCurrDataCount);
        return -1;
    }

    int sharememclient::get_object_record_size(int nObjectIndex)
    {
        if ( nObjectIndex < 1 || nObjectIndex > _nSize )
        {
            return -1;
        }
        return _pobjshm[nObjectIndex - 1].obj_size;
    }

    int sharememclient::get_object_index_size(int nObjectIndex)
    {
        if ( nObjectIndex < 1 || nObjectIndex > _nSize )
        {
            return -1;
        }
        return _pobjshm[nObjectIndex - 1].bucket_size;
    }

    int sharememclient::search(int nObjectIndex,const void *key,void* presult,int nresult) 
    { 
        return -1;
    }

    int sharememclient::search(int nObjectIndex,const void *key,void* presult) 
    {
        return -1;
    }

}

