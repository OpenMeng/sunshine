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

	//���ݱ��뽵�����У��ȽϺ������շ��ء�
	//�����keyֵΪ�ȽϺ����ĵ�һ��������
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
	virtual int load_data(int action){return 0;};	//��������;���hash_seq����0, ˵�����ǵ�һ����, ��˴�����������, ֻ�Ǵ�������(ϸ�ڴ���ȷ)
	virtual int deal_q_data(){return 0;};	//����q������
	virtual int pre_refresh_data() {return 0;};

	virtual void add_observer(array *observer){};	//����������Ӧ��array������deal_q_data
	virtual void set_hash_table(hashtable *hash){}; //��Ӷ�Ӧ��hashtable
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

