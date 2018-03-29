#include "msgqueue.h"
#include <string.h>
#if defined (MEM_DEBUG)
    #define new 		DEBUG_NEW
    #define delete	DEBUG_DELETE
#endif

namespace util
{

    msgqueue::msgqueue(int nIpcKey)
    :_nIpcKey(nIpcKey)
    {
        _nmsgid = -1;
        _nrunmode = -1;
    }

    msgqueue::~msgqueue()
    {
        if ( _nrunmode == 1 && _nmsgid >= 0 ) {
            ::msgctl(_nmsgid, IPC_RMID, 0);
            _nmsgid = -1;
            _nrunmode = -1;
        }
    }

    int msgqueue::create_msgqueue()
    {
        int ntry = 0;
        _nmsgid= ::msgget(_nIpcKey, 0600 );
        if ( _nmsgid >= 0 ) {
            if ( ::msgctl(_nmsgid, IPC_RMID, 0) < 0 )
                throw bad_msg(__FILE__,__LINE__,220,"mgsctl: IPC_RMID failed.");
            _nmsgid = -1;
        }

        while ( (_nmsgid= ::msgget(_nIpcKey, 0600 |IPC_CREAT)) < 0 ) {
            sleep(1);
            ntry ++;
            if ( ntry > 60 )
                throw bad_system(__FILE__,__LINE__,221,0,"create message queue failed.");
        }
        _nrunmode = 1;
        return _nmsgid > 0 ? 0 : -1;
    }

    int msgqueue::get_msgqueue()
    {
        int ntry = 0;
        while ( (_nmsgid= ::msgget(_nIpcKey, 0600 )) < 0 ) {
            sleep(1);
            ntry ++;
            if ( ntry > 60 )
                throw bad_system(__FILE__,__LINE__,221,0,"get message queue failed.");
        }
        _nrunmode = 0;
        return _nmsgid > 0 ? 0 : -1;
    }

    int msgqueue::sendmsg(int nmsgtype, char cmd)
    {
        message msg;
        int nret = -1;
        int ntry = 0;
        msg.mtype = nmsgtype;
        msg.mprocid = ::getpid();
        msg.mtext[0] = cmd;
        while ( ( nret = ::msgsnd(_nmsgid,(char*)&msg,sizeof(message) - sizeof(long),0)) < 0 ) {
            ntry ++;
            if ( ntry > 60 )
                throw bad_system(__FILE__,__LINE__,222,0,"send message queue failed.");
            sleep(1);
        }
        return nret;
    }

    int msgqueue::recvmsg(int nmsgtype,long& pid,char& cmd)
    {
        message msg;
        int nret = -1;
        int ntry = 0;
        ::memset(&msg,0,sizeof(msg));
        while ( ( nret = ::msgrcv(_nmsgid,(char*)&msg,sizeof(message),nmsgtype,0)) < 0 ) {
            ntry ++;
            if ( ntry > 60 )
                throw bad_system(__FILE__,__LINE__,223,0,"recieve message queue failed.");
            sleep(1);
        }
        pid = msg.mprocid;
        cmd = msg.mtext[0];

        return nret;
    }

    int msgqueue::recvmsg(long nmsgtype, char* msgbuf, size_t buf_size, int msg_flag)
    {
        msgbuf[0] = 0;

        char tmp[2048];
        int length = sizeof(commandmsg) - sizeof(long);
        int nret = ::msgrcv(_nmsgid, tmp, length, nmsgtype, msg_flag);
        if (nret > 0)
            memcpy(msgbuf, ((commandmsg*)tmp)->mtext, buf_size);
        
        if (nret == -1 && errno == ENOMSG)
            return 0;

        return nret;
    }

}


