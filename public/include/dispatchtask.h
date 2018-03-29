#ifndef  __DISPACTHTASK_H__
#define  __DISPACTHTASK_H__

#define MAX_THREAD_NUM	256
#define MAX_FILENAME_LEN	512
#define MAX_THREADID_LEN	32

class dispatchtask
{
public:
	struct threadinfo
	{
		char _libname[MAX_FILENAME_LEN];
		char _threadid[MAX_THREADID_LEN];
		void* (*pfn_CreateObject)(void*);
	};

public:
	void* _pHandle[MAX_THREAD_NUM];

	dispatchtask();
	~dispatchtask();
	
	int load_libary();
	int release_libary();
	int create_object();
};

#endif

