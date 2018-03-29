#include "hashtable.h"

#if defined(MEM_DEBUG)
#define new 	DEBUG_NEW
#define delete 	DEBUG_DELETE
#endif


namespace util
{

  hashtable::hashtable(int nInit, char** pData, super_block* pObjShm,array*& pArray)
  {
    int i = 0;
    for( i = 0 ; i < MAX_MEM_BLKS ; i ++ )
    {
      pHashAddr[i] = pData[i];
    }

    memcpy((char*)&m_hashblk,(char*)pObjShm->hash_mem,MAX_MEM_BLKS*sizeof(hashblock));
    hashsize = pObjShm->hash_size;
    bucketsize = pObjShm->bucket_size;
    hashblocknum = pObjShm->hash_blk_num;
    totalhashblocknum = pObjShm->hash_total_blk_num;
    _pArray = pArray;

    // 0: sharememclient 使用
    // 1: sharememserver 使用
    if ( nInit == 1 ) 
    {
      clear();
    }
  }

  int hashtable::get_block_info(int index,int& memblkindex,int& blkindex)
  {
    if ( index <  0 || index >= hashsize)
    {
      ostringstream err_msg;
      err_msg.str("");
      err_msg << "get_block_info: index = " << index <<  " exceed hash size = " << hashsize << endl;
      throw bad_msg(__FILE__,__LINE__,216,err_msg.str());
    }

    if ( hashblocknum < 1 )
    {
      throw bad_msg(__FILE__,__LINE__,217,"get_block_info: block number < 1.");
    }

    if ( hashblocknum == 1 )
    {
      if ( index > m_hashblk[0].hash_end || index < m_hashblk[0].hash_start  )
      {
        blkindex = -1;
        memblkindex = -1;
        throw bad_msg(__FILE__,__LINE__,216,"get_block_info: index exceed block range.");
      }
      blkindex = m_hashblk[0].blk_index;
      memblkindex = 0;
      return 0;
    }

    int nBegin= 0;
    int nLast  = hashblocknum - 1;
    register int nMid = 0;

    //升序排列
    while ( nLast >= nBegin )
    {
      nMid = nBegin + (( nLast - nBegin ) >> 1);

      if ( index < m_hashblk[nMid].hash_start )
      {
        nLast = nMid  - 1;
      }
      else if ( index > m_hashblk[nMid].hash_end )
      {
        nBegin = nMid  + 1;
      }
      else
      {
        memblkindex = nMid;
        blkindex = m_hashblk[nMid].blk_index;
        if ( blkindex < 0 || blkindex >= totalhashblocknum )
          throw bad_msg(__FILE__,__LINE__,218,"get_block_info: index exceed block range.");
        return 0;
      }
    }
    memblkindex = -1;
    blkindex = -1;
    return -1;
  }

  hashtable::~hashtable()
  {
  }

  void hashtable::clear() 
  {
    int memblkindex,blkindex;
    for (register int i = 0; i < hashsize; i ++)
    {
      if ( get_block_info(i, memblkindex, blkindex) < 0 )
        return;
      pbucket =(hashbucket*)(pHashAddr[blkindex] + m_hashblk[memblkindex].blk_offset + (i - m_hashblk[memblkindex].hash_start) * bucketsize);
      pbucket->count  = 0;
      pbucket->index = -1;
    }
  }

  int hashtable::add_index(const char* key,const int index)
  {
    char skey1[512];
    int memblkindex,blkindex;
    if ( key == 0 )
      throw bad_msg(__FILE__,__LINE__,16,"add_index: key is null.");

    memset(skey1,0,512);
    _pArray->get_key(key,skey1,512);
    int bn = _pArray->get_hash(skey1,hashsize);
    get_block_info(bn, memblkindex, blkindex);
    pbucket =(hashbucket*)(pHashAddr[blkindex] + m_hashblk[memblkindex].blk_offset + (bn - m_hashblk[memblkindex].hash_start) * bucketsize);
    if ( pbucket->index == -1 || pbucket->count == 0 )
    {
      pbucket->index = index;
      pbucket->count = 1;
    }
    else
    {
      _pArray->add_link(index,pbucket->index);
      pbucket->index = index;
      pbucket->count += 1;
    }
    return 0;
  }

  //只能够返回节点的第一索引
  int hashtable::get_value(const char* key,int& index,int& ncount)
  {
    char skey1[512];
    int memblkindex,blkindex;
    if ( key == 0 )
      throw bad_msg(__FILE__,__LINE__,16,"get_value: key is null.");

    if ( _pArray->get_size() == 0 )
    {
      index = -1;
      ncount = 0;
      return 0;
    }
    index = -1;
    ncount = 0;	
    memset(skey1,0,512);
    _pArray->get_key(key,skey1,512); 
    int bn = _pArray->get_hash(skey1,hashsize);
    get_block_info(bn, memblkindex, blkindex);
    pbucket =(hashbucket*)(pHashAddr[blkindex] + m_hashblk[memblkindex].blk_offset + (bn - m_hashblk[memblkindex].hash_start) * bucketsize);
    ncount = pbucket->count;
    index = pbucket->index;
    return ncount;
  }

  //返回节点，用户可以根据不同情况取不同的索引
  int hashtable::get_value(const char* key,void*& p,int& ncount)
  {
    char skey1[512];
    int memblkindex,blkindex;

    if ( key == 0 )
      throw bad_msg(__FILE__,__LINE__,16,"get_value: key is null.");
    if ( _pArray->get_size() == 0 )
    {
      p = 0;
      ncount = 0;
      return 0;
    }
    p = 0;
    ncount = 0;
    memset(skey1,0,512);
    _pArray->get_key(key,skey1,512); 
    int bn = _pArray->get_hash(skey1,hashsize);
    get_block_info(bn, memblkindex, blkindex);
    pbucket =(hashbucket*)(pHashAddr[blkindex] + m_hashblk[memblkindex].blk_offset + (bn - m_hashblk[memblkindex].hash_start) * bucketsize);
    if ( pbucket->count == 0 ) 
      return 0;
    ncount = pbucket->count;
    _pArray->get_at(pbucket->index,p);
    return ncount;
  }	

  int hashtable::remove_index(const char* key,icomparer* compar)
  {
    char skey1[512];
    void *p = 0;
    int curr_index,prev_index,ncount;
    int memblkindex,blkindex;

    try
    {
      if ( key == 0 )
        throw bad_msg(__FILE__,__LINE__,16,"get_value: key is null.");

      if ( _pArray->get_size() == 0 )
      {
        ncount = 0;
        return 0;
      }

      ncount = 0;
      memset(skey1,0,512);
      _pArray->get_key(key,skey1,512); 
      int bn = _pArray->get_hash(skey1,hashsize);
      get_block_info(bn, memblkindex, blkindex);
      pbucket =(hashbucket*)(pHashAddr[blkindex] + m_hashblk[memblkindex].blk_offset + (bn - m_hashblk[memblkindex].hash_start) * bucketsize);
      ncount = pbucket->count;

      prev_index = -1;
      curr_index = pbucket->index;
      pbucket->index = -1;
      for( int i = 0 ; i < ncount ; i ++ )
      {
        if ( _pArray->get_at(curr_index,p) < 0 )
        {
          return -1;
        }
        //满足条件要删除的节点
        if ( compar->compare_hash(_pArray->get_object_index(),key,p) == 0 )
        {
          //节点数调整
          pbucket->count --;
          //修改未删除节点的next_index
          if ( prev_index != -1 )
            _pArray->add_link(-1,prev_index);
        }
        else
        {
          //调整hash bucket指向的数据索引
          if ( pbucket->index == -1 )
          {
            pbucket->index = curr_index;
          }
          //调整数据的next_index指向
          else
          {
            _pArray->add_link(curr_index,prev_index);
          }
          prev_index = curr_index;
        }
        //取下一个节点的索引
        if ( _pArray->get_next_index((char*)p,curr_index) < 0 )
        {
          return -1;
        }
      }
    }
    catch(std::exception& ex)
    {
      dout << ex.what() << endl;
      return -1;
    }

    return 0;
  }

  int hashtable::get_hash_size() const
  {
    return hashsize;
  }

  void hashtable::dump_key(void)
  {
#if defined(DEBUG_DUMPKEY)
    int data_count = _pArray->get_size();
    if ( data_count <= 0 )
      return;

    int bs_count = (int) ::log( (double)data_count / 2 ) + 1;
    int *stat_counts = new int[bs_count + 1];
    int i;

    for ( i = 0 ; i <= bs_count ; i ++ )
      stat_counts[i] = 0;
    for( int i = 0 ; i < hashsize ; i ++ )
    {
      int memblkindex,blkindex;
      if ( get_block_info(i , memblkindex, blkindex) < 0 )
        return;
      pbucket =(hashbucket*)(pHashAddr[blkindex] + m_hashblk[memblkindex].blk_offset + (i - m_hashblk[memblkindex].hash_start) * bucketsize);
      if ( pbucket->index != -1 )
      {
        if (  pbucket->count <= bs_count ) 
          stat_counts[pbucket->count - 1] ++;
        else
          printf( "index = %010d,node_count =%010d,[%d]\n",pbucket->index,pbucket->count,bs_count);
      }
    }

    for ( i = 0 ; i < bs_count ; i ++ )
    {
      printf( "node_count =%010d,[%d],[%d]\n",i + 1,stat_counts[i],bs_count);
    }
    delete [] stat_counts;
    stat_counts = 0;
#endif
  }

}

