
#ifndef _EASYHASH_H_
#define _EASYHASH_H_

#include "common.h"
#include "user_exception.h"
#include "object.h"

#define HASH_KEY_SIZE 512
namespace util{
using namespace std;

template <typename UNIT>
    class easyhash;

template <typename UNIT>
struct _easy_hash_iterator;  //��ͨ������

template <typename UNIT>
struct _easy_hash_const_iterator;  //const������

template <typename UNIT>
struct _easy_hash_lazy_iterator;  //��ѯר�õ����������begin(const UNIT& unit)ʹ��

template <typename UNIT>
struct _easy_hash_iterator
{
    typedef easyhash<UNIT> _Hashtable;
    typedef _easy_hash_iterator<UNIT> iterator;
    typedef _easy_hash_const_iterator<UNIT> const_iterator;
    typedef UNIT value_type;
    typedef ptrdiff_t difference_type;
    typedef UNIT& reference;
    typedef UNIT* pointer;

    pointer p_curr_node;
    _Hashtable* p_hashtable;

    _easy_hash_iterator(pointer node,_Hashtable* table)
        : p_curr_node(node),p_hashtable(table){}

    _easy_hash_iterator(){}

    reference operator*() const
    {
        return *p_curr_node;
    }

    pointer operator->() const
    {
        return p_curr_node;
    }
    bool operator==(const iterator& __it)
    {
        return (p_curr_node == __it.p_curr_node);
    }
    bool operator!=(const iterator& __it)
    {
        return (p_curr_node != __it.p_curr_node);
    }

    iterator& operator++()
    {
        p_curr_node = p_hashtable->get_next(p_curr_node);
        return *this;
    }
    inline iterator operator++(int)
    {
        iterator __tmp = *this;
        ++*this;
        return __tmp;
    }   

};
template <typename UNIT>
struct _easy_hash_lazy_iterator
{
    typedef easyhash<UNIT> _Hashtable;
    typedef _easy_hash_iterator<UNIT> iterator;
    typedef _easy_hash_const_iterator<UNIT> const_iterator;
    typedef _easy_hash_lazy_iterator<UNIT> lazy_iterator;
    typedef UNIT value_type;
    typedef ptrdiff_t difference_type;
    typedef UNIT& reference;
    typedef UNIT* pointer;

    pointer p_curr_node;
    _Hashtable* p_hashtable;

    _easy_hash_lazy_iterator(pointer node,_Hashtable* table)
        : p_curr_node(node),p_hashtable(table){}

    _easy_hash_lazy_iterator(const iterator& __it)
        :p_curr_node(__it.p_curr_node),p_hashtable(__it.p_hashtable){}
    _easy_hash_lazy_iterator(const lazy_iterator& __it)
        :p_curr_node(__it.p_curr_node),p_hashtable(__it.p_hashtable){}

    _easy_hash_lazy_iterator(){}

    reference operator*() const
    {
        return *p_curr_node;
    }

    pointer operator->() const
    {
        return p_curr_node;
    }
    bool operator==(const iterator& __it)
    {
        return (p_curr_node == __it.p_curr_node);
    }
    bool operator!=(const iterator& __it)
    {
        return (p_curr_node != __it.p_curr_node);
    }
    bool operator==(const lazy_iterator& __it)
    {
        return (p_curr_node == __it.p_curr_node);
    }
    bool operator!=(const lazy_iterator& __it)
    {
        return (p_curr_node != __it.p_curr_node);
    }

    lazy_iterator& operator = (const iterator& __it)
    {
        p_curr_node = __it.p_curr_node;
        p_hashtable = __it.p_hashtable;
        return *this;
    }
    lazy_iterator& operator = (const lazy_iterator& __it)
    {
        p_curr_node = __it.p_curr_node;
        p_hashtable = __it.p_hashtable;
        return *this;
    }

    lazy_iterator& operator ++() 
    {
        if(p_curr_node == NULL)
            return *this;
        pointer org = p_curr_node;

        while((p_curr_node = p_curr_node->next) != NULL){                
            if(org->hash_equal(*p_curr_node))
                break;
        }

        return *this;
    }
    inline lazy_iterator operator++(int)
    {
        lazy_iterator __tmp = *this;
        ++*this;
        return __tmp;

    }   

};

template <typename UNIT>
struct _easy_hash_const_iterator
{
    typedef easyhash<UNIT> _Hashtable;
    typedef _easy_hash_iterator<UNIT> iterator;
    typedef _easy_hash_const_iterator<UNIT> const_iterator;
    typedef UNIT value_type;
    typedef ptrdiff_t difference_type;
    typedef const UNIT& reference;
    typedef const UNIT* pointer;

    const pointer p_curr_node;
    const _Hashtable* p_hashtable;

    _easy_hash_const_iterator(pointer node,_Hashtable* table)
        : p_curr_node(node),p_hashtable(table){}

    _easy_hash_const_iterator(){}

    _easy_hash_const_iterator(const iterator& __it)
        :p_curr_node(__it.node),p_hashtable(__it.table){}

    reference operator*() const
    {
        return *p_curr_node;
    }

    pointer operator->() const
    {
        return p_curr_node;
    }
    bool operator==(const iterator& __it)
    {
        return (p_curr_node == __it.p_curr_node);
    }
    bool operator!=(const iterator& __it)
    {
        return (p_curr_node != __it.p_curr_node);
    }

    const_iterator& operator++()
    {
        p_curr_node = p_hashtable->get_next(p_curr_node);
        return *this;
    }
    
    inline const_iterator operator++(int)
    {
    	const_iterator __tmp = *this;
    	++*this;
    	return __tmp;
    }
};

template <typename UNIT>
class easyhash {

public:
typedef int (*calc_hash_func_t)(const char* key,const int hashmode);
typedef easyhash<UNIT> _Hashtable;
typedef _easy_hash_iterator<UNIT> iterator;
typedef _easy_hash_const_iterator<UNIT> const_iterator;
typedef _easy_hash_lazy_iterator<UNIT> lazy_iterator;
typedef UNIT value_type;
typedef ptrdiff_t difference_type;
typedef const UNIT& const_reference;
typedef const UNIT* const_pointer;
typedef UNIT& reference;
typedef UNIT* pointer;

	friend struct  _easy_hash_iterator<value_type>;
	friend struct  _easy_hash_const_iterator<value_type>;
    friend struct  _easy_hash_lazy_iterator<value_type>;

    struct bucket {
        pointer next;
        bucket()
        {
            next = NULL;
        }
    };      

    // hash_size : HASH������С
    // max_data_count : �����������
    // block_size : ��block_sizeΪ���������ڴ�,��ʼ�ڴ����Ϊ0,ÿ�η���block_size*unit_size��С��һ��.
    //������(max_data_count + block_size - 1) / block_size ���ڴ�
    //calchashfunc hashɢ�к���,Ĭ��ΪNULL,ʹ�������㷨
    easyhash(size_t hash_size, size_t max_data_count, size_t block_size = 10000, calc_hash_func_t calchashfunc = NULL)
    {
        if (hash_size <= 0) {
            throw bad_param(__FILE__, __LINE__, 145, "hash_size must bigger than 0.");
        }
        if (max_data_count <= 0) {
            throw bad_param(__FILE__, __LINE__, 146, "max_data_count must bigger than 0.");
        }

        m_incr = block_size;
        m_hash_size = hash_size;
        m_max_data_count = max_data_count;
        m_unit_size = sizeof(value_type);
        m_hash_mode = STRING;       //default hash mode.
        m_index = new bucket[m_hash_size];
        m_total_block = (m_max_data_count + m_incr - 1) / m_incr;
        m_max_data_count = m_total_block * m_incr;
        m_curr_block = 0;
        m_data = new pointer[m_total_block];

        for (int i = 0; i < m_total_block; i ++) {
            m_data[i] = NULL;
        }
        for (int i = 0; i < m_hash_size; i ++) {
            m_index[i].next = NULL;
        }

        m_free_head = NULL;
        m_curr_data_count = 0;
        m_curr_ground = m_incr;
        m_calc_hash_func = calchashfunc;

    };

    ~easyhash()
    {
        delete[] m_index;
        for (int i = 0; i < m_total_block; i ++) {
            if (m_data[i] != NULL)
                delete[] m_data[i];
        }
        delete[] m_data;
    };

    const pointer lookup(const reference condition)
    {
        pointer p = m_get_head(condition);
        while (p != NULL) {
            if (p->hash_eqaul(condition)) {
                return p;
            }
            p = p->next;
        }
        return NULL;
    };
    pointer search(const reference condition) 
    {
        pointer p = m_get_head(condition);
        while (p != NULL) {
            if (p->hash_equal(condition)) {
                return p;
            }
            p = p->next;
        }
        return NULL;
    };
    int search(const reference condition, pointer result, int max_result_count) 
    {
        int ncount = 0;
        if (result == NULL) {
            throw bad_param(__FILE__, __LINE__, 108, "resultbuf can not be null.");
        }

        pointer p = m_get_head(condition);
        while (p != NULL) {
            if (p->hash_equal(condition)) {
                if (ncount >= max_result_count)
                    throw bad_msg(__FILE__, __LINE__, 109, "max_result_count is too small.");
                memcpy(&result[ncount], p, m_unit_size); 
                ncount ++;
            }
            p = p->next;
        }
        return ncount;

    };

    int search(const reference condition, vector<value_type>& result) 
    {
        int ncount = 0;

        pointer p = m_get_head(condition);
        while (p != NULL)
        {
            if (p->hash_equal(condition))
            {
                result.push_back(*p);
                ncount++;
            }
            p = p->next;
        }
        return ncount;
    };
    // End add

    pointer get_next(const pointer preunit)
    {
        int i;
        long hash_key;
        if (preunit == NULL) {
            for (i = 0; i < m_hash_size; i ++) {
                if (m_index[i].next != NULL) {
                    return m_index[i].next;
                }
            }
        } else {
            if (preunit->next != NULL) {
                return preunit->next;
            } else {
                for (i = (m_get_key(*preunit) % m_hash_size) + 1; i < m_hash_size; i ++) {
                    if (m_index[i].next != NULL) {
                        return m_index[i].next;
                    }
                }
            }
        }
        return NULL;
    };
    


    pointer insert(const reference unit)
    {
        pointer occup;
        long hash_key;

        if ((occup = alloc_record()) == NULL)
            throw bad_msg(__FILE__, __LINE__, 147, "m_max_data_count is too small.");

        bucket* p = m_get_bucket(unit);
        memcpy(occup, &unit, m_unit_size);
        occup->next = p->next;
        p->next = occup;
        m_curr_data_count ++;

        return occup;

    };

	inline void erase(const iterator& __it)
	{
		this->del(*(__it.p_curr_node));
		return;
	}
	
    bool del(const reference condition)
    {
        bucket* pbucket = m_get_bucket(condition);
        bool ret = false;
        if (pbucket == NULL) {
            return ret;
        }

        pointer index = pbucket->next;        

        pointer* pre = &(pbucket->next);
        while (index != NULL) {
            if (index->hash_equal(condition)) {
                index = index->next;
                add_free(*pre);
                *pre = index;
                m_curr_data_count --;
                ret = true;
            } else {
                pre = &(index->next);
                index = index->next;                
            }
        }
        return ret;

    };

    void clear()
    {
        for (int i = 0; i < m_total_block; i ++) {
            if (m_data[i] != NULL) {
                delete[] m_data[i];
                m_data[i] = NULL;
            }
        }

        for (int i = 0; i < m_hash_size; i ++) {
            m_index[i].next = NULL;
        }

        m_free_head = NULL;
        m_curr_data_count = 0;
        m_curr_block = 0;
        m_curr_ground = m_incr;
    };

    pointer get_head(const reference condition)
    {
        return m_get_head(condition);
    };

    void set_hash_mode(int hashmode)
    {
        if (m_hash_mode == hashmode) {
            return;
        }
        if (m_curr_data_count > 0) {
            throw bad_msg(__FILE__, __LINE__, 103, "Can not change hashmode for curr_data_count is not zero.");
        }
        if (hashmode != NUMBER && hashmode != STRING) {
            throw bad_param(__FILE__, __LINE__, 104, "unknown hashmode.");
        }
        m_hash_mode = hashmode;
        return;        
    };

    void set_calc_hash_func(calc_hash_func_t calchashfunc)
    {
        if (calchashfunc == NULL) {
            return; 
        }
        if (m_curr_data_count > 0) {
            throw bad_msg(__FILE__, __LINE__, "Can not change calc_hash_func for curr_data_count is not zero.");
        }
        m_calc_hash_func = calchashfunc;
        return;

    };
    size_t get_max_data_count(){return m_max_data_count;};
    size_t get_hash_size(){return m_hash_size;};
    size_t get_curr_data_count(){return m_curr_data_count;};
    size_t get_curr_ground(){return m_curr_ground;};

    const_iterator begin() const
    {
        return const_iterator(this->get_next(NULL),this);
    };
    const_iterator end() const
    {
        return const_iterator(NULL,this);
    };
    iterator begin()
    {
        return iterator(this->get_next(NULL),this);
    };
    iterator begin(const reference unit)
    {
        return iterator(search(unit),this);
    };
    iterator end()
    {
        return iterator(NULL,this);
    };
    iterator end(const reference unit)
    {
        return iterator(NULL,this);
    };


private:
    size_t m_unit_size;                //�������ݽṹ���С
    size_t m_curr_data_count;          //ĿǰHASH���д��ڵ����ݸ���
    size_t m_max_data_count;           //�����Դ洢���ٸ����ݵ�Ԫ
    size_t m_hash_size;                //buckets������
    size_t m_incr;                     //ÿ�η���һ��ռ���������¼��
    int m_hash_mode;                   //NUMBER ����STRING, ȡ������������, NUMBERЧ������STRING
    pointer m_free_head;                  //���м�¼�ռ�����ͷ
    int m_total_block;                 //��������ٿ�ռ�
    int m_curr_block;                  //��һ��δ����Ŀ����
    int m_curr_ground;                 //��ǰ�����ѷ���ļ�¼�ռ��ˮƽ��
    calc_hash_func_t m_calc_hash_func;    //HASHɢ�к���

    bucket* m_index;                   //bucketsָ��
    value_type** m_data;                     //��ָ������

    bucket* m_get_bucket(const reference condition)
    {
        long hash_key = m_get_key(condition);
        return &(m_index[hash_key % m_hash_size]);
    };

    pointer m_get_head(const reference condition)
    {
        long hash_key = m_get_key(condition);
        return m_index[hash_key % m_hash_size].next;
    };

    //ʹ�ú����ھֲ�����hash_key����ԭ��Ա����m_hash_key��������̲߳�ѯʱ�ĳ�ͻ����
    long m_get_key(const reference unit)
    {
        char hash_key[HASH_KEY_SIZE];
        unit.get_hash_key(hash_key);
        hash_key[HASH_KEY_SIZE - 1] = 0;
        if (hash_key[0] == 0) {
            throw bad_msg(__FILE__, __LINE__, 16, "hash_key can not be null.");
        }
        long ret;
        if (m_calc_hash_func == NULL) {
            ret = calc_hash(hash_key);
        }else{
            ret = m_calc_hash_func(hash_key,m_hash_mode);
        }
        
        if (ret < 0) {
            throw bad_msg(__FILE__, __LINE__, 106, "hash_key can not be a negative number.");
        }
        return ret;
    };

    void add_free(pointer freeunit)
    {
        freeunit->next = m_free_head;
        m_free_head = freeunit;        
    };

    pointer alloc_record()
    {
        pointer ret = NULL;

        if (m_free_head != NULL) {
            ret = m_free_head;
            m_free_head = m_free_head->next;   
        } else if (m_curr_ground < m_incr){
            ret = &(m_data[m_curr_block - 1][m_curr_ground]);
            m_curr_ground ++;
        }else if (m_curr_block < m_total_block) {
            m_data[m_curr_block] = new value_type[m_incr];
            m_curr_block ++;
            m_curr_ground = 0;
            ret = &(m_data[m_curr_block - 1][m_curr_ground]);
            m_curr_ground ++;
        }
        return ret;
    };

    
    int calc_hash(const char* key) 
    {
        int nLen  = 0;
        int  i = 0;
        int  ha = 0;
        unsigned long hb = 0;

        switch (m_hash_mode) {
        case STRING:
            nLen = ::strlen(key);
            for ( i = 0; i < nLen; i ++) {
                if ( key[i] >= '0' && key[i] <= '9' ) {
                    hb = (hb * 64 + key[i] - '0') % m_hash_size;
                } else if ( key[i] >= 'A' && key[i] <= 'Z' ) {
                    hb = (hb * 64 + key[i] - 'A' + 10 ) % m_hash_size;
                } else if ( key[i] >= 'a' && key[i] <= 'z' ) {
                    hb = (hb * 64 + key[i] - 'a' + 36 ) % m_hash_size;
                } else {
                    hb = (hb * 64 + key[i] - ' ' + 32) % m_hash_size;
                }
            }
            break;
        case NUMBER:
            hb = ::atol(key) % m_hash_size;
            break;
        }

        ha = (unsigned int)(hb > INT_MAX ? 0 : hb);

        return ha;
    };
};

}

#endif

