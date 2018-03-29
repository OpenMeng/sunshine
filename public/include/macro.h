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
#define HOST_ALARM      "1"         //��������
#define NETWORK_ALARM   "2"         //���羯��
#define DATABASE_ALARM  "3"         //���ݿ⾯��
#define APP_ALARM       "4"         //Ӧ�þ���

/**
 * EXCEPTION TYPE
 */
#define MEM_INSUFF_EXCEPTION       "11000"    //�ڴ�ռ䲻��
#define DISK_INSUFF_EXCEPTION      "11001"    //�ļ�ϵͳ�ռ䲻��

#define NETWORK_CONNECT_EXCEPTION  "12000"    //���������쳣

#define OCCI_EXCEPTION             "13001"    //OCCI�����쳣
#define ORAL_EXCEPTION             "13002"    //ORACLE�쳣 
#define FILE_HANDLE_EXCEPTION      "14001"    //�ļ���д�쳣
#define PARAMETER_EXCEPTION        "14002"    //���������쳣
#define MEM_ALLOC_EXCEPTION        "14003"    //�ڴ�����쳣
#define SHARE_MEM_EXCEPTION        "14004"    //�����ڴ��쳣
#define DATA_EXCEPTION             "14005"    //���ݴ��󣨰������ݿ���ڴ��������ϻ����ϴ���
#define PROC_STATUS_EXCEPTION      "14006"    //����״̬�쳣
#define UNEXPECT_EXCEPTION         "14007"    //���н���쳣(ʧ��)
#define SYS_EXEC_EXCEPTION         "14008"    //ϵͳ����ִ���쳣
#define FILE_DUP_EXCEPTION         "14009"    //�ļ��ظ������쳣                                                                          
#define UNKNOWN_EXCEPTION          "19999"    //����δ֪�쳣

#endif
