#ifndef __MACRO_H__
#define __MACRO_H__

#define DOUBLE_EQUAL(a,b) (fabs(a - b) < 0.1E-6)
#define TRUE		1    // A bool object can be assigned the literal value TRUE.
#define FALSE		0    // A bool object can be assigned the literal value FLASE.

#define	SUCCESS	        0x00    // handle successfully
#define BILL_IPC_KEY	  0x10000
typedef unsigned char		U8 ;
typedef unsigned short	U16 ;
typedef unsigned int		UINT ;
typedef unsigned long		timeout_t;

/**
 * ALARM TYPE
 */
#define HOST_ALARM      "1"         //主机警告
#define NETWORK_ALARM   "2"         //网络警告
#define DATABASE_ALARM  "3"         //数据库警告
#define APP_ALARM       "4"         //应用警告

/**
 * EXCEPTION TYPE
 */
#define MEM_INSUFF_EXCEPTION       "11000"    //内存空间不足
#define DISK_INSUFF_EXCEPTION      "11001"    //文件系统空间不足

#define NETWORK_CONNECT_EXCEPTION  "12000"    //网络连接异常

#define OCCI_EXCEPTION             "13001"    //OCCI调用异常
#define ORAL_EXCEPTION             "13002"    //ORACLE异常 
#define FILE_HANDLE_EXCEPTION      "14001"    //文件读写异常
#define PARAMETER_EXCEPTION        "14002"    //参数配置异常
#define MEM_ALLOC_EXCEPTION        "14003"    //内存分配异常
#define SHARE_MEM_EXCEPTION        "14004"    //共享内存异常
#define DATA_EXCEPTION             "14005"    //数据错误（包括数据库和内存中少资料或资料错误）
#define PROC_STATUS_EXCEPTION      "14006"    //进程状态异常
#define UNEXPECT_EXCEPTION         "14007"    //运行结果异常(失败)
#define SYS_EXEC_EXCEPTION         "14008"    //系统命令执行异常
#define FILE_DUP_EXCEPTION         "14009"    //文件重复处理异常                                                                          
#define UNKNOWN_EXCEPTION          "19999"    //其它未知异常

#endif
