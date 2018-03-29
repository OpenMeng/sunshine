#ifndef __LOG_FILE_H__ 
#define __LOG_FILE_H__

#include <cerrno>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "udpsocket.h"
#include "user_assert.h"

namespace util
{

    using std::string;
    using std::vector;

    class log_file {
    public:
        enum switch_mode {
            mode_none, // no switch mode
            mode_minute, // swtich on minute
            mode_hour, // switch on hour
            mode_day, // switch once a day
            mode_month // switch once a month
        };

        // If id's length is 4, this means it's a process log,
        // and if id's length is 8, this means it's a thread log.
        // and if it's length is 0, this means it's a daemon log.
        log_file(switch_mode mode_, const string& path_, const string& id_ = "daemon");
        
        virtual ~log_file();
        void write_log(const char* filename, int linenum, const char* msg, int len = 0) ;    
        void write_log(const string& msg);

    private:
        string id; // It can be thread_id(8) or proc_id(4).
        switch_mode mode;
        const string path;
    };

    class log_udp {
    public:
        log_udp(int port);
        virtual ~log_udp();
        // Wait udp up to seconds provided, if data is prepared, return immediately.
        bool wait(int sec = 0, int usec = 0);        
        // Receive message from daemon, it's used by process.
        void receive(msg_packet& packet);
        // Used by process to send to daemon to tell process is alive.
        void registration(const string& proc_id);
        // Used by process to report thread status.
        void proc_report(const string& thread_id, char status);
        // Used by thread to report thread message.
        void thread_report(const string& topic, const string& thread_id, const string& msg);
        // Used by thread to report thread progress.
        void thread_report(const string& thread_id, const string& msg);
        // Refresh share memory.
        void refresh(const string& proc_id, const vector<int>& table_ids);
        // Reprocess thread.
        void reprocess(const string& thread_id);
        // Dump fee realtime.
        void dump_fee(const string& thread_id);
        // Rollback thread.
        void rollback(const string& thread_id);
        // Used by thread to report thread alarm.
        // Alarm level: 0 slight, 1 serious, 2 fatal.
        void thread_report(const string& topic, const string& thread_id, const string& object_type,
                           const string& object_name, int level, const string& msg);
        // Used to report host infomation.
        void host_report(const string& thread_id, const string& fmt_name, const string& msg);

        // Used for process start by command.
        void logon(const string& acct_month, int area_group_id, const string& task_code, pid_t pid);
        void logout(const string& acct_month, int area_group_id, const string& task_code);
        void acct_progress(const string& acct_month,
                           int area_group_id,
                           const string& task_code,
                           const string& thread_code,
                           int total,
                           int dealed);
        void acct_report(const string& acct_month,
                         int area_group_id,
                         const string& task_code,
                         const string& thread_code,
                         const string& msg);
        void acct_alarm(const string& acct_month,
                        int area_group_id,
                        const string& task_code,
                        const string& thread_code,
                        const string& msg);
        void start_cmd(const string& acct_month,
                       int area_group_id,
                       const string& task_code,
                       const string& cmd);
        void stop_cmd(const string& acct_month, int area_group_id, const string& task_code);

    private:
        udpsocket* client;
    };

}

#endif

