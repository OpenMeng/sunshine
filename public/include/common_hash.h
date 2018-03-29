
#ifndef _COMMON_HASH_H_
#define _COMMON_HASH_H_

#include "common.h"
#include "user_exception.h"
#include "object.h"

#define HASH_KEY_SIZE 512
namespace util{
    using namespace std;

    //data units in common_hash must inherit from struct hash_unit_t
    struct hash_unit_t {
        char* next_index;

        hash_unit_t()
        {
            next_index = NULL;
        }
        virtual void get_hash_key(char* hash_key) const
        {    
            throw bad_msg(__FILE__, __LINE__, 110, "Can't find overrided function void get_hash_key(char* hash_key) const");
            //VIRTUAL_TYPE下,自定义结构体必须重载函数long get_hash_key() const
            return ;
        }   
    
        virtual bool equal (const hash_unit_t* right) const
        {
            throw bad_msg(__FILE__, __LINE__, 111, "Can't find overrided function bool equal(const hash_unit_t* right) const");
            //VIRTUAL_TYPE下,自定义结构体必须重载函数bool equal (const hash_unit_t* src) const
            return false;
        }

        hash_unit_t* next()
        {
            return (hash_unit_t*)next_index;
        }
    };

    struct bucket {
        char* next_index;
        bucket()
        {
            next_index = NULL;
        }
    };

    enum HASH_TYPE {
        STATIC_TYPE = 0x01 ,    //空间在HASH对象建立时做一次分配.以后不再分配空间
        DYNAMIC_TYPE = 0x02 ,   //空间在插入数据时动态扩展 
        VIRTUAL_TYPE = 0x04,   //采用结构体的多态实现自定义数据单元,
                                //自定义数据结构体必须继承自hash_unit_t且必须对虚函数
                                //long get_hash_key() const 和 bool equal(const hash_unit_t* src) const 
                                //进行重载
                                //**注意**此种方式下数据结构体和类实例相似
                                //不可以对数据结构体进行memset操作
        FUNC_PTR_TYPE = 0x08    //采用外部传入函数指针的方式,自定义结构体必须继承自hash_unit_t
                                //比较函数和GETHASHKEY函数必须在构造时传入
    };

    typedef bool (*compar_func_t)(const char*,const char*);
    typedef void (*getkey_func_t)(const char*, char*);
    class common_hash {

    public:
        common_hash(size_t hash_size, size_t max_data_count, size_t unit_size, compar_func_t in_compar = NULL, getkey_func_t in_getkey = NULL);

        common_hash(size_t hash_size, size_t unit_size,compar_func_t in_compar = NULL, getkey_func_t in_getkey = NULL,bool save_memory = true);

        ~common_hash();

        const hash_unit_t* lookup(const hash_unit_t* condition);
        hash_unit_t* search(const hash_unit_t* condition);
        int search(const hash_unit_t* condition, char* result, int max_result_count);

        char* get_head(const hash_unit_t* condition);

        bucket* get_bucket(const hash_unit_t* condition);
        hash_unit_t* get_at(char* index);
        hash_unit_t* get_next(const hash_unit_t* preunit);

        bool insert(const hash_unit_t* unit);
        bool del(const hash_unit_t* condition);

        void clear();
        void set_hash_mode(int hashmode);
        size_t get_max_data_count(){return m_max_data_count;};
        size_t get_hash_size(){return m_hash_size;};
        size_t get_curr_data_count(){return m_curr_data_count;};
        size_t get_curr_ground(){return m_curr_ground;};

    private:
        size_t m_unit_size;                //单个数据结构体大小
        int m_hash_type;                   //hash类型,取决与调用哪个构造函数以及传入多少个参数
        size_t m_curr_data_count;          //目前HASH表中存在的数据个数
        size_t m_curr_ground;              //当前水平线,STATIC_TYPE有效
        size_t m_max_data_count;           //最大可以存储多少个数据单元,STATIC_TYPE有效
        size_t m_hash_size;                //buckets的数量
        int m_hash_mode;
        char* m_free_head;                  //空闲空间链表头
        bool m_save_memory;                //是否节约内存模式,默认为true,DYNAMIC_TYPE有效
        compar_func_t compar;       //外部传入比较函数指针,FUNC_PTR_TYPE有效
        getkey_func_t get_key;                   //外部传入得到HASHKEY的函数指针,FUNC_PTR_TYPE有效


        char m_hash_key[HASH_KEY_SIZE];

        bucket* m_index;                   //buckets指针
        char* m_data;                      //数据存放空间指针,STATIC_TYPE有效


        inline bool m_compar(const hash_unit_t* left, const hash_unit_t* right);

        inline long m_get_key(const hash_unit_t* unit);
        
        inline void add_free(char* free_index);

        inline char* alloc_record();

        inline int calc_hash(const char* key);

        void set_type(int type){m_hash_type |= type;};
        bool is_type(int type){ return (m_hash_type & type != 0);};

    };


}


#endif
