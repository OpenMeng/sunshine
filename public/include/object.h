
#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <pthread.h>
#include <exception>
#include <iostream>
#include <stdio.h>
#include <limits.h>

#include "dstream.h"
#include "macro.h"
#include "common.h"
#include "memleak.h"

using namespace std;

namespace util
{

#define RUNTIME_CLASS(class_name) ((runtimeclass*)(&class_name::class##class_name))

#define DECLARE_DYNCREATE(class_name) \
public: \
    static runtimeclass class##class_name; \
    virtual runtimeclass* get_runtime_class() const; \
    static object* create_object(const param* para = 0);

#define IMPLEMENT_DYNCREATE(name_space,class_name) \
    object* class_name::create_object(const param* para) \
    { return new class_name(para); } \
    runtimeclass class_name::class##class_name = { \
     #name_space"::"#class_name,sizeof(class class_name),class_name::create_object,0 }; \
    runtimeclass* class_name::get_runtime_class() const \
    { return RUNTIME_CLASS(class_name); } \
    CLASSINIT _init_##class_name(RUNTIME_CLASS(class_name));

class object;

struct runtimeclass
{
    const char* m_lpszClassName;
    int m_nObjectSize;
    object* (* m_pfnCreateObject)(const param *para);	
    object* create_object(const param* para);
    runtimeclass* m_pNextClass;
};

class object
{
public:
    object() ;
    virtual ~object() ;
    virtual int run();
    virtual int run_ext();
    virtual int run_service();
    virtual int start();
    virtual int stop();
    virtual int kill();
    virtual int status();
    virtual int reprocess();
    virtual int dump_fee(const string& taskid);
    virtual int refresh_data(const vector<int>& tableid,int nAction);
    virtual int rollback(const string& taskid);
};

extern "C"
{
    void class_init(runtimeclass* pNewClass);
    //classname 为 带命名空间的类名
    object* class_get(const char* classname,const param* para);
    //classname 为 带命名空间的类名
    void class_delete(const char* classname);
}

struct CLASSINIT
{ 
    CLASSINIT(runtimeclass* pNewClass) 
    { 
        class_init(pNewClass); 
    } 
};

#if defined(LINUX)
#define BLOCK_SIZE			1024 * 1024 * 400
#else
#define BLOCK_SIZE			INT_MAX
#endif

#define MAX_OBJ_MEM_BLKS		4
#define MAX_MEM_BLKS			8
#define TABLE_INFO_SIZE			8*1024

struct memblock
{
    char	blk_index;
    int  	blk_offset;
    int  	obj_start;
    int  	obj_end;
};

struct hashblock
{
    char	blk_index;
    int  	blk_offset;
    int  	hash_start;
    int  	hash_end;
};

struct hashbucket 
{
    int 	index;
    int		count;
};

struct super_block
{
    char	obj_total_blk_num;
    char	hash_total_blk_num;
    char 	index_mode;
    //数据使用内存块总数
    char	obj_blk_num; 
    short obj_size;
    int  	obj_count;
    int  	obj_curr_count;

    //索引使用内存块总数
    char   	hash_blk_num;
    short  	bucket_size;
    int    	hash_size;

    struct memblock obj_mem[MAX_MEM_BLKS];
    struct hashblock hash_mem[MAX_MEM_BLKS];
    
    char  table_info[TABLE_INFO_SIZE];
};

enum enIndexMode
{
    ARRAY = 0,
    HASH_TABLE
};

enum enHashKeyMethod
{
    NUMBER = 0,
    STRING
};

}
#endif

