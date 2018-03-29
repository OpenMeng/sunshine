

#include "udpsocket.h"

#if defined (MEM_DEBUG)
#define new DEBUG_NEW
#define delete DEBUG_DELETE
#endif

namespace util
{

using std::string;
using std::map;

udpsocket::udpsocket()
    : ipsocket()
{
    _runmode = -1;
}

udpsocket::~udpsocket()
{
}

//client use
int udpsocket::create(const char* ipaddr,int port)
{
    ::memset(&inetAddress, 0,sizeof(inetAddress));
    if (ipaddr == 0)
    {
        inetAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {

//        int nret = ::inet_pton(AF_INET,ipaddr,&inetAddress.sin_addr);
  //      if (nret <= 0)
    //        printf("convert error\n");
        inetAddress.sin_addr.s_addr = inet_addr(ipaddr) ;
    }
    inetAddress.sin_family      = AF_INET;
    inetAddress.sin_port        = htons(port);

    _runmode = 0;
    open(UDP);
    return 0;
}

//server use
int udpsocket::create(int port)
{
    open(UDP);

    ::memset(&inetAddress, 0,sizeof(inetAddress));
    inetAddress.sin_family      = AF_INET;
    inetAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    inetAddress.sin_port        =  htons(port);

    bind(&inetAddress);

    _runmode = 1;
    return 0;
}

int udpsocket::receive(msg_packet& packet)
{
    int nlen = 0,nleft = 0,nread = 0;
    unsigned char _strrecv[MAX_PACKET*2],*pread;

    memset(_strrecv,0,MAX_PACKET*2);
    pread = _strrecv;
    nleft = MAX_PACKET*2;
    while (nleft > 0)
    {
        nread = recvfrom(&remoteAddress,pread+ nread,nleft);
        if (nread < 0 && errno == EINTR)
            continue;

        memcpy((unsigned char*)&packet.len,pread,sizeof(int));
        memcpy((unsigned char*)&packet.cmd_type,pread + sizeof(int),sizeof(char));
        if (packet.len == nread)
        {
            break;
        }
#ifdef LINUX
        nread = nleft;
#endif        
        nleft -= nread;
        pread += nread;
    }
    switch(packet.cmd_type)
    {
    case 'C':
        memcpy((unsigned char*)&packet.content.cmd,pread + sizeof(int) + sizeof(char),sizeof(cmd_packet));
        break;
    case 'M':
        memcpy((unsigned char*)&packet.content.pmsg,pread + sizeof(int) + sizeof(char),sizeof(proc_msg_packet));
        break;
    case 'P':
        memcpy((unsigned char*)&packet.content.proc,pread + sizeof(int) + sizeof(char),sizeof(proc_packet));
        break;
    case 'R':
        memcpy((unsigned char*)&packet.content.reg,pread + sizeof(int) + sizeof(char),sizeof(reg_packet));
        break;
    case 'T':
    case 'J':
        memcpy((unsigned char*)&packet.content.thr,pread + sizeof(int) + sizeof(char),sizeof(thread_packet));
        break;
    case 'I':
        memcpy((unsigned char*)&packet.content.acct_msg,pread + sizeof(int) + sizeof(char),sizeof(acct_msg_packet));
        break;
    case 'L':
        memcpy((unsigned char*)&packet.content.acct_reg,pread + sizeof(int) + sizeof(char),sizeof(acct_reg_packet));
        break;
    case 'O':
        memcpy((unsigned char*)&packet.content.acct_cmd,pread + sizeof(int) + sizeof(char),sizeof(acct_cmd_packet));
        break;
    }
    if (_runmode == 1)
    {
        if (packet.cmd_type == 'R')
        {
            portmap record;
            record.sin_addr = remoteAddress.sin_addr;
            record.sin_port = remoteAddress.sin_port;
            record.sys_proc_id = packet.content.reg.sys_proc_id;
            _portmap[packet.content.reg.self_proc_id] = record;
        }
    }

#if defined(DEBUG)
    switch(packet.cmd_type)
    {
    case 'C':
        printf("%d,%d,%c,%s,%c\n",nread,packet.len,packet.cmd_type,packet.content.cmd.self_thread_id,packet.content.cmd.action);
        break;
    case 'M':
        printf("%d,%d,%c,%s,%s\n",nread,packet.len,packet.cmd_type,packet.content.pmsg.self_thread_id,packet.content.pmsg.message);
        break;
    case 'P':
        printf("%d,%d,%c,%s,%c\n",nread,packet.len,packet.cmd_type,packet.content.proc.self_thread_id,packet.content.proc.status);
        break;
    case 'R':
        printf("%d,%d,%c,%d,%s\n",nread,packet.len,packet.cmd_type,packet.content.reg.sys_proc_id,packet.content.reg.self_proc_id);
        break;
    case 'T':
    case 'J':
        printf("%d,%d,%c,%s,%s\n",nread,packet.len,packet.cmd_type,packet.content.thr.self_thread_id,packet.content.thr.cause);
        break;
    case 'I':
        printf("%d,%d,%c,%s\n",nread,packet.len,packet.cmd_type,packet.content.acct_msg.msg);
        break;
    case 'L':
        printf("%d,%d,%c,%s,%s,%s,%d,%c\n",nread,packet.len,packet.cmd_type,packet.content.acct_reg.acct_month,packet.content.acct_reg.area_group_id,packet.content.acct_reg.task_code,packet.content.acct_reg.pid,packet.content.acct_reg.status);
        break;
    case 'O':
        printf("%d,%d,%c,%s,%s,%s,%c,%s\n",nread,packet.len,packet.cmd_type,packet.content.acct_cmd.acct_month,packet.content.acct_cmd.area_group_id,packet.content.acct_cmd.task_code,packet.content.acct_cmd.action,packet.content.acct_cmd.cmd);
        break;
    }
#endif

    return packet.len;
}

int udpsocket::send(msg_packet& packet)
{
    int nleft = 0,nwrite = 0,nlen = 0;
    int port;
    unsigned char strsnd[ MAX_PACKET*2];
    map<string, portmap>::iterator iter;

    memset(strsnd,0,MAX_PACKET*2);

    switch(packet.cmd_type)
    {
    case 'C':
        if (_runmode == 0)
            return -1;		
        nleft = sizeof(int) + sizeof(char) + sizeof(cmd_packet) ;
        packet.len = nleft;
        memcpy(strsnd,(unsigned char*)&packet.len,sizeof(int));
        memcpy(strsnd + sizeof(int),(char*)&packet.cmd_type,sizeof(char));
        memcpy(strsnd + sizeof(int) + sizeof(char),(char*)&packet.content.cmd,sizeof(cmd_packet));
        iter = _portmap.find(string(packet.content.cmd.self_thread_id,packet.content.cmd.self_thread_id + 4));
        if (iter == _portmap.end())
            return -1;
        inetAddress.sin_addr = iter->second.sin_addr;
        inetAddress.sin_port = iter->second.sin_port;
        break;
    case 'M':
        if (_runmode == 0)
            return -1;
        nleft = sizeof(int) + sizeof(char) + sizeof(proc_msg_packet) ;
        packet.len = nleft;
        memcpy(strsnd,(unsigned char*)&packet.len,sizeof(int));
        memcpy(strsnd + sizeof(int),(char*)&packet.cmd_type,sizeof(char));
        memcpy(strsnd + sizeof(int) + sizeof(char),(char*)&packet.content.pmsg,sizeof(proc_msg_packet));
        iter = _portmap.find(string(packet.content.cmd.self_thread_id,packet.content.cmd.self_thread_id + 4));
        if (iter == _portmap.end())
            return -1;
        inetAddress.sin_addr = iter->second.sin_addr;
        inetAddress.sin_port = iter->second.sin_port;
        break;
    case 'P':
        if (_runmode == 1)
            return -1;
        nleft = sizeof(int) + sizeof(char) + sizeof(proc_packet);
        packet.len = nleft;
        memcpy(strsnd,(unsigned char*)&packet.len,sizeof(int));
        memcpy(strsnd + sizeof(int),(char*)&packet.cmd_type,sizeof(char));
        memcpy(strsnd + sizeof(int) + sizeof(char),(char*)&packet.content.proc,sizeof(proc_packet));
        break;
    case 'R':
        if (_runmode == 1)
            return -1;
        nleft = sizeof(int)  + sizeof(char)  + sizeof(reg_packet);
        packet.len = nleft;
        memcpy(strsnd,(unsigned char*)&packet.len,sizeof(int));
        memcpy(strsnd + sizeof(int),(char*)&packet.cmd_type,sizeof(char));
        memcpy(strsnd + sizeof(int) + sizeof(char),(char*)&packet.content.reg,sizeof(reg_packet));
        break;
    case 'T':
    case 'J':
        if (_runmode == 1)
            return -1;
        nleft = sizeof(char) + sizeof(int) + sizeof(thread_packet);
        packet.len = nleft;
        memcpy(strsnd,(unsigned char*)&packet.len,sizeof(int));
        memcpy(strsnd + sizeof(int),(char*)&packet.cmd_type,sizeof(char));
        memcpy(strsnd + sizeof(int) + sizeof(char),(char*)&packet.content.thr,sizeof(thread_packet));
        break;
    case 'I':
        if (_runmode == 1)
            return -1;
        nleft = sizeof(int) + sizeof(char) + sizeof(acct_msg_packet);
        packet.len = nleft;
        memcpy(strsnd,(unsigned char*)&packet.len,sizeof(int));
        memcpy(strsnd + sizeof(int),(char*)&packet.cmd_type,sizeof(char));
        memcpy(strsnd + sizeof(int) + sizeof(char),(char*)&packet.content.acct_msg,sizeof(acct_msg_packet));
        break;
    case 'L':
        if (_runmode == 1)
            return -1;
        nleft = sizeof(int) + sizeof(char) + sizeof(acct_reg_packet);
        packet.len = nleft;
        memcpy(strsnd,(unsigned char*)&packet.len,sizeof(int));
        memcpy(strsnd + sizeof(int),(char*)&packet.cmd_type,sizeof(char));
        memcpy(strsnd + sizeof(int) + sizeof(char),(char*)&packet.content.acct_reg,sizeof(acct_reg_packet));
        break;
    case 'O':
        if (_runmode == 1)
            return -1;
        nleft = sizeof(int) + sizeof(char) + sizeof(acct_cmd_packet);
        packet.len = nleft;
        memcpy(strsnd,(unsigned char*)&packet.len,sizeof(int));
        memcpy(strsnd + sizeof(int),(char*)&packet.cmd_type,sizeof(char));
        memcpy(strsnd + sizeof(int) + sizeof(char),(char*)&packet.content.acct_cmd,sizeof(acct_cmd_packet));
        break;
    default:
        return -1;
    }

    nlen = nleft;
    while (nleft > 0)
    {
        nwrite = sendto(&inetAddress,strsnd + nwrite,nleft);
        if (nwrite < 0 && errno == EINTR)
            continue;
#ifdef LINUX
        nwrite = nleft;
#endif
        nleft -= nwrite;
    }

#if	defined(DEBUG)
    switch(packet.cmd_type)
    {
    case 'C':
        printf("%d,%d,%c,%s,%c\n",nlen-nleft,packet.len,packet.cmd_type,packet.content.cmd.self_thread_id,packet.content.cmd.action);
        break;
    case 'M':
        printf("%d,%d,%c,%s,%s\n",nlen-nleft,packet.len,packet.cmd_type,packet.content.pmsg.self_thread_id,packet.content.pmsg.message);
        break;
    case 'P':
        printf("%d,%d,%c,%s,%c\n",nlen-nleft,packet.len,packet.cmd_type,packet.content.proc.self_thread_id,packet.content.proc.status);
        break;
    case 'R':
        printf("%d,%d,%c,%d,%s\n",nlen-nleft,packet.len,packet.cmd_type,packet.content.reg.sys_proc_id,packet.content.reg.self_proc_id);
        break;
    case 'T':
    case 'J':
        printf("%d,%d,%c,%s,%s\n",nlen-nleft,packet.len,packet.cmd_type,packet.content.thr.self_thread_id,packet.content.thr.cause);
        break;
    case 'I':
        printf("%d,%d,%c,%s\n",nlen-nleft,packet.len,packet.cmd_type,packet.content.acct_msg.msg);
        break;
    case 'L':
        printf("%d,%d,%c,%s,%s,%s,%d,%c\n",nlen-nleft,packet.len,packet.cmd_type,packet.content.acct_reg.acct_month,packet.content.acct_reg.area_group_id,packet.content.acct_reg.task_code,packet.content.acct_reg.pid,packet.content.acct_reg.status);
        break;
    case 'O':
        printf("%d,%d,%c,%s,%s,%s,%c,%s\n",nlen-nleft,packet.len,packet.cmd_type,packet.content.acct_cmd.acct_month,packet.content.acct_cmd.area_group_id,packet.content.acct_cmd.task_code,packet.content.acct_cmd.action,packet.content.acct_cmd.cmd);
        break;
    }
#endif

    return (nlen - nleft);
}

}

