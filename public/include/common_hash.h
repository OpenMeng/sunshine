
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
            //VIRTUAL_TYPE��,�Զ���ṹ��������غ���long get_hash_key() const
            return ;
        }   
    
        virtual bool equal (const hash_unit_t* right) const
        {
            throw bad_msg(__FILE__, __LINE__, 111, "Can't find overrided function bool equal(const hash_unit_t* right) const");
            //VIRTUAL_TYPE��,�Զ���ṹ��������غ���bool equal (const hash_unit_t* src) const
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
        STATIC_TYPE = 0x01 ,    //�ռ���HASH������ʱ��һ�η���.�Ժ��ٷ���ռ�
        DYNAMIC_TYPE = 0x02 ,   //�ռ��ڲ�������ʱ��̬��չ 
        VIRTUAL_TYPE = 0x04,   //���ýṹ��Ķ�̬ʵ���Զ������ݵ�Ԫ,
                                //�Զ������ݽṹ�����̳���hash_unit_t�ұ�����麯��
                                //long get_hash_key() const �� bool equal(const hash_unit_t* src) const 
                                //��������
                                //**ע��**���ַ�ʽ�����ݽṹ�����ʵ������
                                //�����Զ����ݽṹ�����memset����
        FUNC_PTR_TYPE = 0x08    //�����ⲿ���뺯��ָ��ķ�ʽ,�Զ���ṹ�����̳���hash_unit_t
                                //�ȽϺ�����GETHASHKEY���������ڹ���ʱ����
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
        size_t m_unit_size;                //�������ݽṹ���С
        int m_hash_type;                   //hash����,ȡ��������ĸ����캯���Լ�������ٸ�����
        size_t m_curr_data_count;          //ĿǰHASH���д��ڵ����ݸ���
        size_t m_curr_ground;              //��ǰˮƽ��,STATIC_TYPE��Ч
        size_t m_max_data_count;           //�����Դ洢���ٸ����ݵ�Ԫ,STATIC_TYPE��Ч
        size_t m_hash_size;                //buckets������
        int m_hash_mode;
        char* m_free_head;                  //���пռ�����ͷ
        bool m_save_memory;                //�Ƿ��Լ�ڴ�ģʽ,Ĭ��Ϊtrue,DYNAMIC_TYPE��Ч
        compar_func_t compar;       //�ⲿ����ȽϺ���ָ��,FUNC_PTR_TYPE��Ч
        getkey_func_t get_key;                   //�ⲿ����õ�HASHKEY�ĺ���ָ��,FUNC_PTR_TYPE��Ч


        char m_hash_key[HASH_KEY_SIZE];

        bucket* m_index;                   //bucketsָ��
        char* m_data;                      //���ݴ�ſռ�ָ��,STATIC_TYPE��Ч


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
