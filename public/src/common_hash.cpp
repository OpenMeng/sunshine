#include "common_hash.h"

namespace util {
    using namespace std;


    common_hash::common_hash(size_t hash_size, size_t max_data_count, size_t unit_size, compar_func_t in_compar, getkey_func_t in_getkey) 
    {

        if (hash_size <= 0) {
            throw bad_param(__FILE__, __LINE__, 100, "hash_size must bigger than 0.");
        }
        if (max_data_count <= 0) {
            throw bad_param(__FILE__, __LINE__, 101, "max_data_count must bigger than 0.");
        }
        if (unit_size <= 0) {
            throw bad_param(__FILE__, __LINE__, 102, "unit_size must bigger than 0.");
        }

        m_hash_type = 0;
        set_type(STATIC_TYPE);
        if (in_compar != NULL && in_getkey != NULL) {
            set_type(FUNC_PTR_TYPE);
            compar = in_compar;
            get_key = in_getkey;
        } else {
            set_type(VIRTUAL_TYPE);
            compar = NULL;
            get_key = NULL;
        }
        m_hash_size = hash_size;
        m_max_data_count = max_data_count;
        m_unit_size = unit_size;
        m_hash_mode = STRING;       //default hash mode.
        m_index = new bucket[m_hash_size];
        m_data = new char[m_unit_size * m_max_data_count];
        m_free_head = NULL;
        m_curr_data_count = 0;
        m_curr_ground = 0;
        m_save_memory = false;

    };

    common_hash::common_hash(size_t hash_size, size_t unit_size,compar_func_t in_compar, getkey_func_t in_getkey,bool save_memory) 
    {
        if (hash_size <= 0) {
            throw bad_param(__FILE__, __LINE__, 100, "hash_size must bigger than 0.");
        }
        if (unit_size <= 0) {
            throw bad_param(__FILE__, __LINE__, 102, "unit_size must bigger than 0.");
        }

        m_hash_type = 0;
        set_type(DYNAMIC_TYPE);
        if (in_compar != NULL && in_getkey != NULL) {
            set_type(FUNC_PTR_TYPE);
            compar = in_compar;
            get_key = in_getkey;
        } else {
            set_type(VIRTUAL_TYPE);
            compar = NULL;
            get_key = NULL;
        }
        m_hash_size = hash_size;
        m_unit_size = unit_size;
        m_save_memory = save_memory;
        m_hash_mode = STRING;
        m_index = new bucket[m_hash_size];
        m_data = NULL;
        m_free_head = NULL;
        m_curr_data_count = 0;
    };

    common_hash::~common_hash()
    {
        if (is_type(STATIC_TYPE)) {
            delete[] m_index;
            delete[] m_data;
        } else if (is_type(DYNAMIC_TYPE)) {
            while (m_free_head != NULL) {
                char* p = m_free_head;
                m_free_head = ((hash_unit_t*)m_free_head)->next_index;
                delete[] p;
            }
            for (int i = 0; i < m_hash_size; i ++) {
                while (m_index[i].next_index != NULL) {
                    char* p = m_index[i].next_index;
                    m_index[i].next_index = ((hash_unit_t*)(m_index[i].next_index))->next_index;
                    delete[] p;
                }
            }
            delete[] m_index;
        }

    };

    void common_hash::set_hash_mode(int hashmode)
    {
        if (m_curr_data_count > 0 && m_hash_mode != hashmode) {
            throw bad_msg(__FILE__, __LINE__, 103, "Can not change hashmode for curr_data_count is not zero.");
        }
        if (hashmode != NUMBER && hashmode != STRING) {
            throw bad_param(__FILE__, __LINE__, 104, "unknown hashmode.");
        }
        m_hash_mode = hashmode;
        return;
    }

    int common_hash::calc_hash(const char* key)
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
    }

    bool common_hash::insert(const hash_unit_t* unit) 
    {
        char* occup;
        long hash_key;

        if ((occup = alloc_record()) == NULL)
            throw bad_msg(__FILE__, __LINE__, 105, "m_max_data_count is too small.");

        hash_key = m_get_key(unit);

        if (hash_key < 0) {
            add_free(occup);
            throw bad_msg(__FILE__, __LINE__, 106, "hash_key can not be a negative number.");
        }

        bucket* p = &m_index[hash_key % m_hash_size];
        memcpy(occup, unit, m_unit_size);
        ((hash_unit_t*)occup)->next_index = p->next_index;
        p->next_index = occup;
        m_curr_data_count ++;

        return true;
    };

    void common_hash::clear() 
    {
        int i;
        if (is_type(STATIC_TYPE)) {
            m_curr_ground = 0;
            m_curr_data_count = 0;
            m_free_head = NULL;
            for (i = 0; i < m_hash_size; i ++) {
                m_index[i].next_index = NULL;
            }
        } else if (is_type(DYNAMIC_TYPE)) {
            for (; m_free_head != NULL;) {
                char* p = m_free_head;
                m_free_head = ((hash_unit_t*)m_free_head)->next_index;
                delete[] p;
            }
            for (i = 0; i < m_hash_size; i ++) {
                for (; m_index[i].next_index != NULL;) {
                    char* p = m_index[i].next_index;
                    m_index[i].next_index = ((hash_unit_t*)(m_index[i].next_index))->next_index;
                    delete[] p;
                }
            }
        }
    };

    bool common_hash::m_compar(const hash_unit_t* left, const hash_unit_t* right)
    {
        if (is_type(VIRTUAL_TYPE)) {
            return left->equal(right);
        } else {
            return compar(reinterpret_cast<const char*>(left),
                          reinterpret_cast<const char*>(right));
        }
    };

    long common_hash::m_get_key(const hash_unit_t* unit)
    {
        m_hash_key[0] = 0;
        if (is_type(VIRTUAL_TYPE)) {            
            unit->get_hash_key(m_hash_key);            
        } else {
            get_key(reinterpret_cast<const char*>(unit), m_hash_key);
        }
        m_hash_key[HASH_KEY_SIZE - 1] = 0;
        if (m_hash_key[0] == 0) {
            throw bad_msg(__FILE__, __LINE__, 16, "hash_key can not be null.");
        }
        return calc_hash(m_hash_key);
    };

    void common_hash::add_free(char* index) 
    {
        if (index == NULL) {
            throw bad_msg(__FILE__, __LINE__, 107, "illegal index.");
        }
        if ((is_type(DYNAMIC_TYPE)) && m_save_memory) {
            delete[] index;
        } else {
            ((hash_unit_t*)index)->next_index = m_free_head;
            m_free_head = index;
        }
    };

    char* common_hash::alloc_record()
    {
        char* ret = NULL;

        if (m_free_head != NULL) {
            ret = m_free_head;
            m_free_head = ((hash_unit_t*)m_free_head)->next_index;
        } else if (is_type(STATIC_TYPE)) {
            if (m_curr_ground < m_max_data_count) {
                ret = &m_data[m_curr_ground * m_unit_size];
                m_curr_ground ++;
            }
        } else if (is_type(DYNAMIC_TYPE)) {
            ret = new char[m_unit_size];
        }

        return ret;
    };
    char* common_hash::get_head(const hash_unit_t * condition)
    {
        long hash_key = m_get_key(condition);
        return m_index[hash_key % m_hash_size].next_index;
    }

    hash_unit_t* common_hash::get_next(const hash_unit_t* preunit) 
    {
        int i;
        long hash_key;
        if (preunit == NULL) {
            for (i = 0; i < m_hash_size; i ++) {
                if (m_index[i].next_index != NULL) {
                    return (hash_unit_t*)(m_index[i].next_index);
                }
            }
        }else{
            if (preunit->next_index != NULL) {
                return (hash_unit_t*)(preunit->next_index);
            }else{
                hash_key = m_get_key(preunit);                
                for (i = (hash_key % m_hash_size) + 1; i < m_hash_size; i ++) {
                    if (m_index[i].next_index != NULL) {
                        return (hash_unit_t*)(m_index[i].next_index);
                    }
                }
            }
        }
        return NULL;
    }
    hash_unit_t* common_hash::get_at(char* index)
    {
        if (index == NULL) {
            return NULL;
        }
        return((hash_unit_t*)index);
    }

    bucket* common_hash::get_bucket(const hash_unit_t* condition)
    {
        long hash_key = m_get_key(condition);
        return &m_index[hash_key % m_hash_size];
    }

    bool common_hash::del(const hash_unit_t* condition)
    {
        bucket* pbucket = get_bucket(condition);
        bool ret = false;
        if (pbucket == NULL) {
            return ret;
        }

        char* index = pbucket->next_index;        

        char** pre = &(pbucket->next_index);
        while (index != NULL) {
            if (m_compar(condition, (hash_unit_t*)index)) {
                index = ((hash_unit_t*)index)->next_index;
                add_free(*pre);
                *pre = index;
                m_curr_data_count --;
                ret = true;
            } else {
                pre = &(((hash_unit_t*)index)->next_index);
                index = ((hash_unit_t*)index)->next_index;                
            }
        }
        return ret;
    }

    const hash_unit_t* common_hash::lookup(const hash_unit_t* condition) 
    {
        char* index = get_head(condition);        

        hash_unit_t* p;
        while (index != NULL) {
            p = (hash_unit_t*)index;
            if (m_compar(condition, p)) {
                return p;
            }
            index = p->next_index;
        }
        return NULL;
    }

    hash_unit_t* common_hash::search(const hash_unit_t* condition) 
    {
        char* index = get_head(condition);   

        hash_unit_t* p ;
        while (index != NULL) {
            p = (hash_unit_t*)index;
            if (m_compar(condition, p)) {
                return p;
            }
            index = p->next_index;
        }
        return NULL;
    }

    int common_hash::search(const hash_unit_t* condition, char* result, int max_result_count) 
    {
        int ret = 0;
        if (result == NULL) {
            throw bad_param(__FILE__, __LINE__, 108, "resultbuf can not be null.");
        }
        char* index = get_head(condition);   

        hash_unit_t* p;
        while (index != NULL) {
            p = (hash_unit_t*)index;
            if (m_compar(condition,p)) {
                if (ret >= max_result_count)
                    throw bad_msg(__FILE__, __LINE__, 109, "max_result_count is too small.");
                memcpy(result + ret * m_unit_size, index, m_unit_size); 
                ret ++;
            }
            index = p->next_index;
        }
        return ret;
    }
}

/*
    Example Code

STATIC_TYPE && VIRTUAL_TYPE :
    struct work_unit : hash_unit{
        int data1;
        int data2;
        bool equal(const hash_unit_t* right) const
        {
            return (data1 == ((work_unit*)right)->data1) && (data2 == ((work_unit*)right)->data2);
        }
        void get_hash_key(char* hash_key) const
        {
            if(hash_key != NULL)
                sprintf(hash_key, "%d", data1);
        }
        void clear()
        {
            data1 = 0; data2 = 0;
        }
    }

    common_hash work_hash(100,100,sizeof(work_unit));
    work_unit tmp;
    tmp.clear();
    tmp.data1 = 1;
    tmp.data2 = 2;
    work_hash.insert(&tmp);
    work_unit* p = (work_unit*)work_hash.search(&tmp);

DYNAMIC_TYPE and FUNC_PTR_TYPE :
    struct work_unit : hash_unit{
        int data1;
        int data2;
    }

    bool compar(const char* left, const char* right) 
    {
        return (((const work_unit*)left)->data1 == ((const work_unit*)right)->data1) &&
                (((const work_unit*)left)->data2 == ((const work_unit*)right)->data2)
    }

    void get_hash_key(const char* src, char* hashkey) 
    {
        sprintf(hashkey, "%d", ((const work_unit*)src)->data1);
    }
    
    common_hash work_hash(100,sizeof(work_unit),compar,get_hash_key);
    work_unit tmp;
    memset(&tmp, 0, sizeof(tmp));
    tmp.data1 = 1;
    tmp.data2 = 2;
    work_hash.insert(&tmp);
    work_unit* p = (work_unit*)(work_hash.search(&tmp));
    
*/

