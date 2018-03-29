
#ifndef __UDPSOCKET_H__
#define __UDPSOCKET_H__

#include <map>
#include <string>
#include "ipsocket.h"

namespace util
{

const int MAX_PACKET  = 1024;

/******************************************************************************
// ���涨��Ľṹ��ʵʱ����/�߳���UDP SERVERͨѶʹ��
******************************************************************************/
// daemon => app
// ��Ӧ�÷��������̻߳���̵�����,(���ʹ��)
struct cmd_packet
{
    char self_thread_id[8 + 1];
    char action; // 0 ֹͣ,1 ����
};

// daemon => app
// ��Ӧ�÷����ɽ��̽���������,(���ʹ��)
struct proc_msg_packet
{
    char self_thread_id[8 + 1];
    char message[256]; // ��Ҫ���͸������ɽ��̽�������Ϣ
};

// app=>daemon
// ���ر����̵߳��������,(Ӧ��ʹ��)
struct proc_packet
{
    char self_thread_id[8 + 1];
    char status; // 0 ����ֹͣ,1 ��������,2�쳣ֹͣ
};

// app => daemon
// ����ע��������,(Ӧ��ʹ��)
struct reg_packet
{
    int	sys_proc_id;
    char self_proc_id[4 + 1];
};

// thread => daemon
// ���ر����̵߳�����/�澯���,(�߳�ʹ��)
struct thread_packet
{
    char self_thread_id[8 + 1];
    char cause[MAX_PACKET];
};

/******************************************************************************
// ���涨��Ľṹ�ɷ�ʵʱ����/�߳���UDP SERVERͨѶʹ��
******************************************************************************/
struct acct_msg_packet
{
    char msg[MAX_PACKET];
};

struct acct_reg_packet
{
    char acct_month[6 + 1];
    char area_group_id[2 + 1];
    char task_code[6 + 1];
    pid_t pid;
    char status;
};

struct acct_cmd_packet
{
    char acct_month[6 + 1];
    char area_group_id[2 + 1];
    char task_code[6 + 1];
    char action; // 0 ֹͣ,1 ����
    char cmd[200]; // �����������������ֹͣ�������Ϊ��
};

struct msg_packet
{
    int len;
    // C - cmd_packet, M - proc_msg_packet, R - reg_packet, P - proc_packet, T - thread_packet
    // J - thread_packet, �̱߳���Ľ�����Ϣ����Ҫ�ٽ�ֵ�ж�
    // I - acct_msg_packet, L - acct_reg_packet, O - acct_cmd_packet.
    char cmd_type;
    union
    {
        cmd_packet cmd;
        proc_msg_packet pmsg;
        reg_packet reg;
        proc_packet proc;
        thread_packet thr;
        acct_msg_packet acct_msg;
        acct_reg_packet acct_reg;
        acct_cmd_packet acct_cmd;
    }content;
};

//app 
struct threadinfo
{
    //	util::workthread* pthread;
    char threadid[8 + 1];
};

class udpsocket : public ipsocket
{
public:
    udpsocket();
    ~udpsocket();
    int create(const char* ip,int nport);
    int create(int nport);

    int receive(msg_packet& packet);
    int send(msg_packet& packet);

private:
    //deamon
    struct portmap
    {
        struct in_addr sin_addr;
        unsigned int sin_port;
        int sys_proc_id;
    };

    int find_port(const std::string& selfprocid);

    sockaddr_in inetAddress;
    sockaddr_in remoteAddress;
    std::map<std::string, portmap> _portmap;
    int _runmode; //0: client 1: server
};

}

#endif

