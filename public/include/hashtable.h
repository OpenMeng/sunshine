#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <limits.h>

#include "object.h"
#include "array.h"
#include "comparer.h"

namespace util
{

  class hashtable
  {
  private: 
    hashbucket* pbucket;
    char*  pHashAddr[MAX_MEM_BLKS];
    hashblock m_hashblk[MAX_MEM_BLKS];
    int hashblocknum;
    int totalhashblocknum;
    int hashsize;
    int bucketsize; 
    array* _pArray;

  private:
    int get_block_info(int index,int& memblkindex,int& blkindex);

  public:
    hashtable(int nInit,char** pAddr,super_block* pObjShm,array*& pArray);
    ~hashtable();

    void clear();
    int add_index(const char* key,const int index);
    int remove_index(const char* key,icomparer* compar);
    int get_value(const char* key,void*& p,int& ncount);
    int get_value(const char* key,int& index,int& ncount);
    int get_hash_size() const;
  public:
    void dump_key(void);
  };

}
#endif

