#include "log_file.h"
#include "udpsocket.h"
#include "user_exception.h"

namespace util
{

    using std::ofstream;
    using std::ios;
    using std::endl;
    using std::cerr;

// ----------------------------------------------------------------------------------------------------------
// Class for write log to local file.
// ----------------------------------------------------------------------------------------------------------
    log_file::log_file(switch_mode mode_, const string& path_, const string& id_)
    : path(path_)
    {
        mode = mode_;
        id = id_;
    }

    log_file::~log_file()
    {
    }

    void log_file::write_log(const char* filename, int linenum, const char* msg, int len)     
    {
        char date_time[32];
        time_t t = time(0);
        tm tm;
        localtime_r(&t, &tm);
        strftime(date_time, 32, "%Y-%m-%d_%H_%M_%S", &tm);
        string log_name = path;
        log_name += '/';
        log_name += id;
        log_name += '_';

        switch (mode) {
        case mode_none:
            break;
        case mode_minute:
            log_name.append(date_time, date_time + 16);
            break;
        case mode_hour:
            log_name.append(date_time, date_time + 13);
            break;
        case mode_day: // Switch daily.
            log_name.append(date_time, date_time + 10);
            break;
        case mode_month: // Switch monthly.
            log_name.append(date_time, date_time + 7);
            break;
        default:
            assert(0);
        }

        log_name += ".log";
        FILE* fp = fopen(log_name.c_str(),"ab");
        if (fp == 0) {
            cerr << "open file[" << log_name << "]	error.\n"
            << '[' << date_time << "] [" << id << "] : "
            << "message[" << msg << "]\n";
        } else {
            fprintf(fp,"[%s] [%s] : message[",date_time,id.c_str());
            fprintf(fp,"%s:%d:",filename,linenum);
            if (len == 0)
                fputs(msg,fp);
            else
                fwrite(msg,len,1,fp);
            fprintf(fp,"]\n");
            fclose(fp);
        }
    }

    void log_file::write_log(const string& msg)
    {
        char date_time[32];
        time_t t = time(0);
        tm tm;
        localtime_r(&t, &tm);
        strftime(date_time, 32, "%Y-%m-%d_%H_%M_%S", &tm);
        string log_name = path;
        log_name += '/';
        log_name += id;
        log_name += '_';

        switch (mode) {
        case mode_none:
            break;
        case mode_minute:
            log_name.append(date_time, date_time + 16);
            break;
        case mode_hour:
            log_name.append(date_time, date_time + 13);
            break;
        case mode_day: // Switch daily.
            log_name.append(date_time, date_time + 10);
            break;
        case mode_month: // Switch monthly.
            log_name.append(date_time, date_time + 7);
            break;
        default:
            assert(0);
        }

        log_name += ".log";
        ofstream ofs(log_name.c_str(), ios::out | ios::app);
        if (!ofs) {
            cerr << "open file[" << log_name << "]	error.\n"
            << '[' << date_time << "] [" << id << "] : "
            << "message[" << msg << "]" << endl;
        } else {
            ofs << '[' << date_time << "] [" << id << "] : "
            << "message[" << msg << "]" << endl;
        }
    }

// ----------------------------------------------------------------------------------------------------------
// Class for write log to local file.
// ----------------------------------------------------------------------------------------------------------
    log_udp::log_udp(int port)
    {
        client = new udpsocket();
        client->create("127.0.0.1", port);
    }

    log_udp::~log_udp()
    {
        delete client;
    }

    bool log_udp::wait(int sec, int usec)
    {
        return client->wait(util::ipsocket::READ_POLL, sec, usec);
    }

    void log_udp::receive(msg_packet& packet)
    {
        client->receive(packet);
    }

    void log_udp::registration(const string& proc_id)
    {
        msg_packet packet;
        packet.cmd_type = 'R';
        packet.content.reg.sys_proc_id = getpid();
        memcpy(packet.content.reg.self_proc_id, proc_id.c_str(), sizeof(packet.content.reg.self_proc_id) - 1);
        packet.content.reg.self_proc_id[sizeof(packet.content.reg.self_proc_id) - 1] = '\0';
        client->send(packet);
    }

    void log_udp::proc_report(const string& thread_id, char status)
    {
        msg_packet packet;
        packet.cmd_type = 'P';
        memcpy(packet.content.proc.self_thread_id, thread_id.c_str(), sizeof(packet.content.proc.self_thread_id) - 1);
        packet.content.proc.self_thread_id[sizeof(packet.content.proc.self_thread_id) - 1] = '\0';
        packet.content.proc.status = status;
        client->send(packet);
    }

    void log_udp::thread_report(const string& topic, const string& thread_id, const string& msg)
    {
        msg_packet packet;
        int len(0);
        int len_topic(0);
        int len_thread(0);
        int len_msg(0);
        packet.cmd_type = 'T';
        memcpy(packet.content.thr.self_thread_id, thread_id.c_str(), sizeof(packet.content.thr.self_thread_id) - 1);
        packet.content.thr.self_thread_id[sizeof(packet.content.thr.self_thread_id) - 1] = '\0';

        len_topic  =  topic.length();
        len_thread = thread_id.length();
        len_msg = msg.length();

        if ((MAX_PACKET - 3) >= (len_topic + len_thread + len_msg))
            sprintf(packet.content.thr.cause, "%s|%s|%s", topic.c_str(), thread_id.c_str(), msg.c_str());
        else {
            len = sprintf(packet.content.thr.cause, "%s|%s|",topic.c_str(),thread_id.c_str()) ;  
            memcpy(&(packet.content.thr.cause[len]), msg.c_str(), MAX_PACKET - len - 1) ;
            packet.content.thr.cause[MAX_PACKET - 1] = 0 ;
        }
        client->send(packet);
    }

    void log_udp::refresh(const string& proc_id, const vector<int>& table_ids)
    {
        msg_packet packet;
        packet.cmd_type = 'E';
        memcpy(packet.content.pmsg.self_thread_id, proc_id.c_str(), sizeof(packet.content.pmsg.self_thread_id) - 1);
        packet.content.pmsg.self_thread_id[sizeof(packet.content.pmsg.self_thread_id) - 1] = '\0';
        sprintf(packet.content.pmsg.message, "0%4.4s", proc_id.c_str());
        int i = 5;
        for (vector<int>::const_iterator iter = table_ids.begin(); iter != table_ids.end(); ++iter) {
            i += sprintf(packet.content.pmsg.message + i, "%d", *iter);
            if (iter != table_ids.end() - 1)
                packet.content.pmsg.message[i++] = ',';
            else
                packet.content.pmsg.message[i] = '\0';
        }
        client->send(packet);
    }

    void log_udp::reprocess(const string& thread_id)
    {
        msg_packet packet;
        packet.cmd_type = 'E';
        memcpy(packet.content.pmsg.self_thread_id, thread_id.c_str(), sizeof(packet.content.pmsg.self_thread_id) - 1);
        packet.content.pmsg.self_thread_id[sizeof(packet.content.pmsg.self_thread_id) - 1] = '\0';
        packet.content.pmsg.message[0] = '1';
        memcpy(packet.content.pmsg.message + 1, thread_id.c_str(), 8);
        packet.content.pmsg.message[9] = '\0';
        client->send(packet);
    }

    void log_udp::dump_fee(const string& thread_id)
    {
        msg_packet packet;
        packet.cmd_type = 'E';
        memcpy(packet.content.pmsg.self_thread_id, thread_id.c_str(), sizeof(packet.content.pmsg.self_thread_id) - 1);
        packet.content.pmsg.self_thread_id[sizeof(packet.content.pmsg.self_thread_id) - 1] = '\0';
        packet.content.pmsg.message[0] = '2';
        memcpy(packet.content.pmsg.message + 1, thread_id.c_str(), 8);
        packet.content.pmsg.message[9] = '\0';
        client->send(packet);
    }

    void log_udp::rollback(const string& thread_id)
    {
        msg_packet packet;
        packet.cmd_type = 'E';
        memcpy(packet.content.pmsg.self_thread_id, thread_id.c_str(), sizeof(packet.content.pmsg.self_thread_id) - 1);
        packet.content.pmsg.self_thread_id[sizeof(packet.content.pmsg.self_thread_id) - 1] = '\0';
        packet.content.pmsg.message[0] = '3';
        memcpy(packet.content.pmsg.message + 1, thread_id.c_str(), 8);
        packet.content.pmsg.message[9] = '\0';
        client->send(packet);
    }

    void log_udp::thread_report(const string& thread_id, const string& msg)
    {
        msg_packet packet;
        packet.cmd_type = 'J';
        memcpy(packet.content.thr.self_thread_id, thread_id.c_str(), sizeof(packet.content.thr.self_thread_id) - 1);
        packet.content.thr.self_thread_id[sizeof(packet.content.thr.self_thread_id) - 1] = '\0';

        int len = sprintf(packet.content.thr.cause, "progress|%s|", thread_id.c_str());
        memcpy(&(packet.content.thr.cause[len]), msg.c_str(), MAX_PACKET - len - 1) ;
        packet.content.thr.cause[MAX_PACKET - 1] = 0 ;
        client->send(packet);
    }

    void log_udp::thread_report(const string& topic, const string& thread_id, const string& object_type,
                                const string& object_name, int level, const string& msg)
    {
        msg_packet packet;
        int len(0);
        int len_packet(0);
        int len_level(0);
        char level_str[MAX_PACKET];
        
        packet.cmd_type = 'T';
        memcpy(packet.content.thr.self_thread_id, thread_id.c_str(), sizeof(packet.content.thr.self_thread_id) - 1);
        packet.content.thr.self_thread_id[sizeof(packet.content.thr.self_thread_id) - 1] = '\0';

	 len_level = sprintf(level_str,"%d",level);
	 len_packet = topic.length() + thread_id.length() + object_type.length() + object_name.length() 
	 			+ len_level +msg.length();

	 if(MAX_PACKET - 6 >= len_packet)
	 {
	 	   sprintf(packet.content.thr.cause, "%s|%s|%s|%s|%d|%s",
            topic.c_str(), thread_id.c_str(), object_type.c_str(), object_name.c_str(), level, msg.c_str());

	 }
	 else
	 {
         	len = sprintf(packet.content.thr.cause, "%s|%s|%s|%s|%d|",
                          topic.c_str(), thread_id.c_str(), object_type.c_str(), object_name.c_str(), level);
        	memcpy(&(packet.content.thr.cause[len]), msg.c_str(), MAX_PACKET - len - 1) ;
       	 packet.content.thr.cause[MAX_PACKET - 1] = 0 ;
	 }

        client->send(packet);
    }

    void log_udp::host_report(const string& thread_id, const string& fmt_name, const string& msg)
    {
        msg_packet packet;
        packet.cmd_type = 'J';
        memcpy(packet.content.thr.self_thread_id, thread_id.c_str(), sizeof(packet.content.thr.self_thread_id) - 1);
        packet.content.thr.self_thread_id[sizeof(packet.content.thr.self_thread_id) - 1] = '\0';
        int len = sprintf(packet.content.thr.cause, "host|%s|%s|", thread_id.c_str(), fmt_name.c_str());
        memcpy(&(packet.content.thr.cause[len]), msg.c_str(), MAX_PACKET - len - 1) ;
        packet.content.thr.cause[MAX_PACKET - 1] = 0 ;

        client->send(packet);
    }

    void log_udp::logon(const string& acct_month, int area_group_id, const string& task_code, pid_t pid)
    {
        msg_packet packet;
        packet.cmd_type = 'L';
        memcpy(packet.content.acct_reg.acct_month, acct_month.c_str(), sizeof(packet.content.acct_reg.acct_month) - 1);
        packet.content.acct_reg.acct_month[sizeof(packet.content.acct_reg.acct_month) - 1] = '\0';
        sprintf(packet.content.acct_reg.area_group_id, "%.2d", area_group_id);
        memcpy(packet.content.acct_reg.task_code, task_code.c_str(), sizeof(packet.content.acct_reg.task_code) - 1);
        packet.content.acct_reg.task_code[sizeof(packet.content.acct_reg.task_code) - 1] = '\0';
        packet.content.acct_reg.pid = pid;
        packet.content.acct_reg.status = '1';
        client->send(packet);
    }

    void log_udp::logout(const string& acct_month, int area_group_id, const string& task_code)
    {
        msg_packet packet;
        packet.cmd_type = 'L';
        memcpy(packet.content.acct_reg.acct_month, acct_month.c_str(), sizeof(packet.content.acct_reg.acct_month) - 1);
        packet.content.acct_reg.acct_month[sizeof(packet.content.acct_reg.acct_month) - 1] = '\0';
        sprintf(packet.content.acct_reg.area_group_id, "%.2d", area_group_id);
        memcpy(packet.content.acct_reg.task_code, task_code.c_str(), sizeof(packet.content.acct_reg.task_code) - 1);
        packet.content.acct_reg.task_code[sizeof(packet.content.acct_reg.task_code) - 1] = '\0';
        packet.content.acct_reg.pid = -1;
        packet.content.acct_reg.status = '0';
        client->send(packet);
    }

    void log_udp::acct_progress(const string& acct_month,
                                int area_group_id,
                                const string& task_code,
                                const string& thread_code,
                                int total,
                                int dealed)
    {
        msg_packet packet;
        packet.cmd_type = 'I';
        sprintf(packet.content.acct_msg.msg, "acctProgress|%s|%d|%s|%s|%d|%d",
                acct_month.c_str(), area_group_id, task_code.c_str(), thread_code.c_str(), total, dealed);
        client->send(packet);
    }

    void log_udp::acct_report(const string& acct_month,
                              int area_group_id,
                              const string& task_code,
                              const string& thread_code,
                              const string& msg)
    {
        msg_packet packet;
        packet.cmd_type = 'I';
        int len = sprintf(packet.content.acct_msg.msg, "acctMsg|%s|%d|%s|%s|",
                          acct_month.c_str(), area_group_id, task_code.c_str(), thread_code.c_str());
        memcpy(&(packet.content.acct_msg.msg[len]), msg.c_str(), MAX_PACKET - len - 1) ;
        packet.content.acct_msg.msg[MAX_PACKET - 1] = 0 ;
        client->send(packet);
    }

    void log_udp::acct_alarm(const string& acct_month,
                             int area_group_id,
                             const string& task_code,
                             const string& thread_code,
                             const string& msg)
    {
        msg_packet packet;
        packet.cmd_type = 'I';
        int len = sprintf(packet.content.acct_msg.msg, "alarm|%s|%d|%s|%s|",
                          acct_month.c_str(), area_group_id, task_code.c_str(), thread_code.c_str());
        memcpy(&(packet.content.acct_msg.msg[len]), msg.c_str(), MAX_PACKET - len - 1) ;
        packet.content.acct_msg.msg[MAX_PACKET - 1] = 0 ;

        client->send(packet);
    }

    void log_udp::start_cmd(const string& acct_month,
                            int area_group_id,
                            const string& task_code,
                            const string& cmd)
    {
        msg_packet packet;
        packet.cmd_type = 'O';
        memcpy(packet.content.acct_cmd.acct_month, acct_month.c_str(), sizeof(packet.content.acct_cmd.acct_month) - 1);
        packet.content.acct_cmd.acct_month[sizeof(packet.content.acct_cmd.acct_month) - 1] = '\0';
        sprintf(packet.content.acct_cmd.area_group_id, "%.2d", area_group_id);
        memcpy(packet.content.acct_cmd.task_code, task_code.c_str(), sizeof(packet.content.acct_cmd.task_code) - 1);
        packet.content.acct_cmd.task_code[sizeof(packet.content.acct_cmd.task_code) - 1] = '\0';
        packet.content.acct_cmd.action = '1';
        memcpy(packet.content.acct_cmd.cmd, cmd.c_str(), sizeof(packet.content.acct_cmd.cmd) - 1);
        packet.content.acct_cmd.cmd[sizeof(packet.content.acct_cmd.cmd) - 1] = '\0';
        client->send(packet);
    }

    void log_udp::stop_cmd(const string& acct_month, int area_group_id, const string& task_code)
    {
        msg_packet packet;
        packet.cmd_type = 'O';
        memcpy(packet.content.acct_cmd.acct_month, acct_month.c_str(), sizeof(packet.content.acct_cmd.acct_month) - 1);
        packet.content.acct_cmd.acct_month[sizeof(packet.content.acct_cmd.acct_month) - 1] = '\0';
        sprintf(packet.content.acct_cmd.area_group_id, "%.2d", area_group_id);
        memcpy(packet.content.acct_cmd.task_code, task_code.c_str(), sizeof(packet.content.acct_cmd.task_code) - 1);
        packet.content.acct_cmd.task_code[sizeof(packet.content.acct_cmd.task_code) - 1] = '\0';
        packet.content.acct_cmd.action = '0';
        client->send(packet);
    }

}

