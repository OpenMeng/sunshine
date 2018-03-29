
#ifndef __UDPSOCKET_H__
#define __UDPSOCKET_H__

#include <map>
#include <string>
#include "ipsocket.h"

namespace util
{

const int MAX_PACKET  = 1024;

/******************************************************************************
// 下面定义的结构由实时进程/线程与UDP SERVER通讯使用
******************************************************************************/
// daemon => app
// 向应用发出控制线程或进程的命令,(监控使用)
struct cmd_packet
{
    char self_thread_id[8 + 1];
    char action; // 0 停止,1 运行
};

// daemon => app
// 向应用发出由进程解析的命令,(监控使用)
struct proc_msg_packet
{
    char self_thread_id[8 + 1];
    char message[256]; // 需要发送给进程由进程解析的消息
};

// app=>daemon
// 向监控报告线程的运行情况,(应用使用)
struct proc_packet
{
    char self_thread_id[8 + 1];
    char status; // 0 正常停止,1 正在运行,2异常停止
};

// app => daemon
// 向监控注册的命令包,(应用使用)
struct reg_packet
{
    int	sys_proc_id;
    char self_proc_id[4 + 1];
};

// thread => daemon
// 向监控报告线程的运行/告警情况,(线程使用)
struct thread_packet
{
    char self_thread_id[8 + 1];
    char cause[MAX_PACKET];
};

/******************************************************************************
// 下面定义的结构由非实时进程/线程与UDP SERVER通讯使用
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
    char action; // 0 停止,1 运行
    char cmd[200]; // 启动程序的命令，如果是停止命令，可以为空
};

struct msg_packet
{
    int len;
    // C - cmd_packet, M - proc_msg_packet, R - reg_packet, P - proc_packet, T - thread_packet
    // J - thread_packet, 线程报告的进度消息，需要临界值判断
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

