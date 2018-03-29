#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <string.h>
#include <stdlib.h>

#include "comparer.h"
#include "object.h"
#include "user_exception.h"

namespace util
{
	class hashtable;
  
  class array
  {
  public:	
	array(char** pDataAddr,super_block* pObjShm,int nObjectIndex);
	virtual ~array();

	int get_size() const;
	int get_max_size() const;
	void set_size(int nSize);
	void clear();

	virtual int get_at(const int nIndex,void*& p);
	int swap(const int index_a, const int index_b);

	int quick_sort(const int left, const int right, icomparer* compar);
	int sort(icomparer* compar);

	int get_object_index() const;
	int alloc_record(void*& obj);

	int calc_hash(enHashKeyMethod nMode,const char* key,int nHashSize);

	//数据必须降序排列，比较函数按照返回。
	//输入的key值为比较函数的第一个参数。
	int binary_search(const void * key, icomparer* compar,void*& p);	
	int linear_search(const void * key, icomparer* compar,void*& p);

	virtual void dump_all_objects(int nIndex) {};
	virtual void dump_single_object(void* p,int nIndex) {};

	virtual int get_next_index(const char* key,int &index) { index = -1;return 0; };
	virtual int get_hash(const char* key,int nHashSize) { return 0; };
	virtual int get_key(const void* p,char* key,int len) { return 0; };
	virtual int get_key(const int index,char* key,int len) { return 0; };
	virtual int add_link(const int newindex,const int  oldindex) { return -1; };	
	virtual int search(const void* key,icomparer* comparer,void*& presult,int nResult){ return -1; };
	virtual int search(const void *key,icomparer*  comparer,void*& presult) { return -1; };
	//new added
	//for billshmarray
	virtual int  search(const void* key,void*& presult,int nResult){return 0;};
	virtual int  search(const void *key,void*& presult){return 0;};
	virtual int load_data(int action){return 0;};	//加载数据;如果hash_seq不是0, 说明不是第一索引, 则此处不加载数据, 只是创建索引(细节待明确)
	virtual int deal_q_data(){return 0;};	//处理q表数据
	virtual int pre_refresh_data() {return 0;};

	virtual void add_observer(array *observer){};	//添加索引表对应的array，用于deal_q_data
	virtual void set_hash_table(hashtable *hash){}; //添加对应的hashtable
	virtual void add_hash_index(void *key, int index){};				



protected:
	int get_block_info(int index,int& memblkindex,int& blkindex);

	char* m_pData[MAX_MEM_BLKS];
	int 	m_nSize;
	int 	m_nMaxSize;
	int 	m_nRecordSize;
	int 	m_nObjectIndex;
	int		m_nObjBlockNum;
	int		m_nTotalObjBlockNum;
	memblock	m_memblk[MAX_MEM_BLKS];
  };

}
#endif

