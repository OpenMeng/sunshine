#if !defined(__APP_H__)
#define __APP_H__

#include <dirent.h>
#include <regex.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include "common.h"
#include "database.h"
#include "log_file.h"
#include "object.h"
#include "udpsocket.h"
#include "user_assert.h"

namespace sunshine
{
namespace ctrl
{
using namespace util;

using std::string;
using std::vector;
using util::log_file;
using util::log_udp;
using util::database;
using util::object;
using util::runtimeclass;
using util::param;
using util::common;
using util::bad_system;
using util::bad_param;
using util::bad_db;
using util::bad_ab;
using util::bad_msg;
using util::msg_packet;

const string PROC_EXIT = "Process exit with no thread running";
const string MISS_THD_IMAGE = "Thread image not exist";
const string THD_START = "Thread started";
const string THD_STOPPED = "Thread stopped";
const string THD_NORMAL_START = "Thread start normally";
const string THD_NORMAL_STOP = "Thread stop normally";

struct thread_info
{
    string thread_name;
    object* obj;
    bool status;
};

typedef object* (*class_get_fn)(const char*, const param*);
typedef void (*class_delete_fn)(const char*);
struct dll_info
{
    void* handle;
    int count;
    class_get_fn class_get;
    class_delete_fn class_delete;
};

struct app_param_t
{
    const char* class_name;
    bool create_pool;
};

static app_param_t app_param[] =
{
    { "", false }, // Not Used.
    // Start from 01 and used for control modules.
    { "sunshine::pub::billshmserver", false }, // Used for dts.
    { "sunshine::monitor::monitor", false }, // Used for monitor.
    { "sunshine::ctrl::export_data", false }, // Used for export.
    { "sunshine::ctrl::sync", false }, // Used for sync.
    { "", false }, // Not Used.    5
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.   10
    // Start from 11 and used for rate.
    { "sunshine::fts::collect", false }, // Used for collect.
    { "sunshine::pps::analyse", false }, // Used for analyse
    { "sunshine::bps::bps", false }, // Used for rate.
    { "sunshine::stat::statistic", false }, // Used for stat.
    { "sunshine::unt::unite", false }, // Used for unite.    15
    { "sunshine::mq::mq", false }, // Used for mq.
    { "sunshine::ftam::ftam", false }, // Used for ftam.
    { "sunshine::enc::encode", false }, //Used for encode
    { "", false }, // Not Used.
    { "", false }, // Not Used.   20
    { "", false }, // Not Used.  21
    { "", false }, // Not Used.  22
    { "", false }, // Not Used. 23.
    { "", false }, // Not Used. 24.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    // Start from 31 and used for billing
    { "sunshine::rate::billing", false }, // Used for billing.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    // Start from 41 and not used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    // Start from 51  .
    { "", false }, // Not Used. 
    { "", false }, // Not Used. 
    { "", false }, // Not Used. 
    { "", false }, // Not Used. 
    { "", false }, // Not Used.
    { "", false }, // Not Used.  
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    // Start from 61 and not used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    // Start from 71 and not used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    // Start from 81 and used for real_disct.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    // Start from 91 and used for account.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false }, // Not Used.
    { "", false } // Not Used.
};

struct defau_arg{
	string user_name;
	string password;
	string connect_string;
};

template<int INDEX, typename T = defau_arg>
class app
{
public:
    // Start all threads.
    app(const string& host_id, const string& proc_id)
    {
        char buf[3];
        sprintf(buf, "%02d", INDEX);
        init(host_id, buf, proc_id);
        dout << "init finished" << std::endl;
        ludp->registration(para.module_id + para.proc_id);
        map<string, thread_info>::iterator iter;
        threads_in_use.clear() ;
        for (iter = threads.begin(); iter != threads.end(); ++iter){
            threads_in_use.push_back(iter->first) ;
            start_thread(iter->first);  
        }
    }

    // Start all threads. with command line inputed
    app(const string& host_id, const string& proc_id, map<string, string> *aArgs)
    {
    	para.args = aArgs;

        char buf[3];
        sprintf(buf, "%02d", INDEX);
        init(host_id, buf, proc_id);
        dout << "init finished" << std::endl;
        ludp->registration(para.module_id + para.proc_id);
        map<string, thread_info>::iterator iter;
        threads_in_use.clear() ;
        for (iter = threads.begin(); iter != threads.end(); ++iter){
            threads_in_use.push_back(iter->first) ;
            start_thread(iter->first);
        }
    }

    // Start threads that provided.
    app(const string& host_id, const string& proc_id, const vector<string>& thread_vector)
    {
        char buf[3];
        sprintf(buf, "%02d", INDEX);
        init(host_id, buf, proc_id);
        dout << "init finished" << std::endl;
        vector<string>::const_iterator iter;

        ludp->registration(para.module_id + para.proc_id);
        threads_in_use.clear() ;
        for (iter = thread_vector.begin(); iter != thread_vector.end(); ++iter)
        {
            if (buf + proc_id != iter->substr(0, 4))
                throw bad_param(__FILE__, __LINE__, 1, "thread_id");
            start_thread(*iter);
            threads_in_use.push_back(*iter) ;
        }
    }

    // Start threads that provided. with command line inputed
    app(const string& host_id, const string& proc_id, const vector<string>& thread_vector, map<string, string> *aArgs)
//    :para(aArgs)
    {
    	para.args = aArgs;

        char buf[3];
        sprintf(buf, "%02d", INDEX);
        init(host_id, buf, proc_id);
        dout << "init finished" << std::endl;
        vector<string>::const_iterator iter;

        ludp->registration(para.module_id + para.proc_id);
        threads_in_use.clear() ;
        for (iter = thread_vector.begin(); iter != thread_vector.end(); ++iter)
        {
            if (buf + proc_id != iter->substr(0, 4))
                throw bad_param(__FILE__, __LINE__, 1, "thread_id");
            start_thread(*iter);
            threads_in_use.push_back(*iter) ;
        }
    }

    virtual ~app()
    {
        delete lfile;
        delete ludp;
        if (app_param[INDEX].create_pool)
            delete para.db;

        if(para.sys_param)
          delete para.sys_param;
    }

    void start()
    {
        // Register process.
        ludp->registration(para.module_id + para.proc_id);

        msg_packet packet;
        int const CHECK_TIME_INTERVAL = 60;
        time_t t = time(0);
        while (1)
        {
            if (ludp->wait(CHECK_TIME_INTERVAL)) // If data has been prepared.
            {
                // Receive messages to control threads.
                ludp->receive(packet);
                if (packet.cmd_type == 'C') // Command.
                {
                    if (packet.content.cmd.action == '0') // Request stop thread.
                    {
                        dout << "Stop command received from udp : "
                            << packet.content.cmd.self_thread_id
                            << std::endl;

                        stop_thread(packet.content.cmd.self_thread_id);
                        if (alive_threads == 0) // Only write log and not send message.
                        {
                            lfile->write_log(PROC_EXIT + " : " + para.module_id + para.proc_id);
                            return;
                        }
                    }
                    else // Request start thread.
                    {
                        dout << "Start command received from udp : "
                            << packet.content.cmd.self_thread_id
                            << std::endl;

                        start_thread(packet.content.cmd.self_thread_id);
                    }
                }
                else if (packet.cmd_type == 'M') // Message.
                {
                    dout << "Message command received from udp : "
                        << packet.content.pmsg.message
                        << std::endl;

                    switch (packet.content.pmsg.message[0])
                    {
                    case '0': // 锟节达拷刷锟斤拷
                        refresh_thread(packet.content.pmsg.message + 5);
                        break;
                    case '1': // 锟斤拷锟斤拷锟截达拷锟斤拷
                        reprocess_thread(packet.content.pmsg.message + 1);
                        break;
                    case '2': // 锟斤拷锟剿凤拷锟斤拷锟斤拷锟斤拷锟斤拷锟?
                        dump_fee(packet.content.pmsg.message + 1);
                        break;
                    default:
                        assert(0);
                    }
                }
                else
                {
                    fmt.str("");
                    fmt << "Invalid command received, packet type = [" + packet.cmd_type << "]";
                    lfile->write_log(fmt.str());
                }
            }

            if (time(0) - t >= CHECK_TIME_INTERVAL) // On time to check.
            {
                map<string, thread_info>::iterator iter;
                for (iter = threads.begin(); iter != threads.end(); ++iter)
                {
                    // Check and report thread status..
                    if (iter->second.status == true)
                    {
                        if (!iter->second.obj->status())
                        {
                            // Thread terminated abnormally. Clear image and send an error message.
                            stop_thread(iter->first);
                            ludp->proc_report(iter->first, '2'); // Stop abnormally.
                            if (alive_threads == 0) // Only write log and not send message.
                            {
                                lfile->write_log(PROC_EXIT + " : " + para.module_id + para.proc_id);
                                return;
                            }
                        }
                        else
                        {
                            ludp->proc_report(iter->first, '1'); // Thread is still alive.
                        }
                    }
                    else
                    {
                        vector<string>::iterator it = find(threads_in_use.begin(), threads_in_use.end(),iter->first) ;
                        if (it != threads_in_use.end())
                            ludp->proc_report(iter->first, '0'); // Thread stopped normally.
                    }
                }
                // Register process.
                ludp->registration(para.module_id + para.proc_id);
                t = time(0);
            }
        }
    }

private:
    // Start thread provided, if thread is not in the image or is alive, report error;
    // otherwise start the thread.
    void start_thread(const string& thread_id)
    {

        map<string, thread_info>::iterator iter = threads.find(thread_id);
        if (iter == threads.end()) // Thread is not in the image.
        {
            lfile->write_log(MISS_THD_IMAGE  + " : " + thread_id);
            ludp->thread_report("alarm", thread_id, "thread", "miss_thread_image", 2, MISS_THD_IMAGE);
        }
        else if (iter->second.obj && iter->second.obj->status()) // Thread is still alive.
        {
            lfile->write_log(THD_START + " : " + thread_id);
            ludp->thread_report("alarm", thread_id, "thread", "thread_start", 0, THD_START);
        }
        else // Thread image exists and it's not alive.
        {
            // Load thread dll, it may only add ref count.
            load_dll(iter->second.thread_name);
            // Start thread.
            para.thread_id = thread_id;
            map<string, dll_info>::iterator dll_iter = dlls.find(iter->second.thread_name);
            assert(dll_iter != dlls.end());

            iter->second.obj = dll_iter->second.class_get(app_param[INDEX].class_name, &para);

            if (!iter->second.obj)
                throw bad_system(__FILE__, __LINE__, 2, bad_system::bad_dlsym, app_param[INDEX].class_name);

            iter->second.status = true;
            iter->second.obj->start();
            alive_threads++;
            lfile->write_log(THD_NORMAL_START + " : " + thread_id);
            ludp->proc_report(thread_id, '1'); // Thread start.

            vector<string>::iterator it = find(threads_in_use.begin(), threads_in_use.end(), thread_id) ;
            if (it == threads_in_use.end()) {
                threads_in_use.push_back(thread_id) ;
            }

            dout << "In start_thread:\n";
            dout << "thread_id = [" << para.thread_id << "]\n";
            dout << ios_base::hex;
            dout << "object address = " << iter->second.obj << "\n";
            dout << ios_base::dec;
            dout << "alive_threads = " << alive_threads << "\n";
            dout << std::flush;
        }
    }

    // Stop thread provided, if thread is not in the image or has already stopped, report error;
    // otherwise stop the thread, free resources the thread occupied.
    void stop_thread(const string& thread_id)
    {
        map<string, thread_info>::iterator iter = threads.find(thread_id);
        if (iter == threads.end()) // Thread is not in the image.
        {
            lfile->write_log(MISS_THD_IMAGE + " : " + thread_id);
            ludp->thread_report("alarm", thread_id, "thread", "miss_thread_image", 2, MISS_THD_IMAGE);
        }
        else if (!iter->second.obj) // Thread has already stopped.
        {
            lfile->write_log(THD_STOPPED + "  : " + thread_id);
            ludp->thread_report("alarm", thread_id, "thread", "thread_stopped", 2, THD_STOPPED);

        }
        else
        {
            // If thread is still alive, stop it first.
            if (iter->second.obj->status())
            {
                // Stop immediately
                iter->second.obj->stop();
                int i;
                int const WAIT_TIME = 6000;
                for (i = 0; i < WAIT_TIME; i++)
                {
                    usleep(10000);
                    if (!iter->second.obj->status())
                        break;
                }
                // If still alive in some seconds, kill it.
                if (i == WAIT_TIME)
                    iter->second.obj->kill();
            }
            // Unload dll if the dll belonging to this thread is still in memory.
            // It may just decrease ref count if the dll is used by some threads.
            dout << "before delete obj [" << __LINE__ << "]" << endl ;
            delete iter->second.obj;
            dout << "before unload_dll [" << __LINE__ << "]" << endl ;
            unload_dll(iter->second.thread_name);
            dout << "after unload_dll  [" << __LINE__ << "]" << endl ;
            
            iter->second.obj = 0;
            iter->second.status = false;
            alive_threads--;
            lfile->write_log(THD_NORMAL_STOP + " : " + thread_id);
            ludp->proc_report(thread_id, '0'); // Thread stop normally.
        }
    }

    // If dll exists according to thread name, just increase ref count.
    // Otherwise load dll and initialize ref count to 1.
    void load_dll(const string& thread_name)
    {
        map<string, dll_info>::iterator iter = dlls.find(thread_name);
        if (iter != dlls.end()) // Found it, just add ref count.
        {
            iter->second.count++;
            return;
        }

        string file_name = "../lib/lib" + thread_name + ".so";
        void* handle = dlopen(file_name.c_str(), RTLD_LAZY);
        if (!handle)
            throw bad_system(__FILE__, __LINE__, 3, bad_system::bad_dlopen, dlerror());
        dll_info info;
        info.handle = handle;
        info.count = 1;
        info.class_get = (class_get_fn)dlsym(handle, "class_get");
        if (!info.class_get)
            throw bad_system(__FILE__, __LINE__, 4, bad_system::bad_dlsym, dlerror());
        info.class_delete = (class_delete_fn)dlsym(handle, "class_delete");
        if (!info.class_delete)
            throw bad_system(__FILE__, __LINE__, 5, bad_system::bad_dlopen, dlerror());
        dlls[thread_name] = info;
        dout << "thread_name = [" << thread_name << "] loaded." << std::endl;
    }

    // If ref count > 1, just decrease ref count.
    // Otherwise unload dll.
    void unload_dll(const string& thread_name)
    {
        map<string, dll_info>::iterator iter = dlls.find(thread_name);
        assert(iter != dlls.end());
        if (--iter->second.count == 0) // Remove image.
        {
            iter->second.class_delete(app_param[INDEX].class_name);
            dlclose(iter->second.handle);
            dlls.erase(iter);
            dout << "thread_name = [" << thread_name << "] unloaded." << std::endl;
        }
    }

    // Do all things before start thread.
    void init(const string& host_id, const string& module_id, const string& proc_id)
    {
        char host_name[32];
        if (gethostname(host_name, sizeof(host_name)) == -1)
            throw bad_system(__FILE__, __LINE__, 6, bad_system::bad_other, "gethostname");
        if (host_id != host_name)
            throw bad_param(__FILE__, __LINE__, 7, "host_id");
        if (atoi(module_id.c_str()) != INDEX)
            throw bad_param(__FILE__, __LINE__, 8, "module_id");
        if (common::is_prog_run())
            throw bad_msg(__FILE__, __LINE__, 9, "Program is already started!");
        para.host_id = host_id;
        para.module_id = module_id;
        para.proc_id = proc_id;
        alive_threads = 0;
        // Load process parameters.
        load_param();
        // Load thread info(thread_id/thread_name).
        load_thread_info();
        // Create log file for process writing error messages.
        lfile = new log_file(para.mode, para.log_path, para.module_id + para.proc_id);
        // Create udp client for process sending error messages.
        ludp = new log_udp(para.port);
        // Create db pool, if every thread uses its own connection, just assign to 0.
        if (app_param[INDEX].create_pool)
            //para.db = new database(para.user_name, para.password, para.connect_string, 0, 10, 1);
            para.db = new database(para.user_name, para.password, para.connect_string);
        else
            para.db = 0;

        para.sys_param= reinterpret_cast<char*> (new T) ; //信控需要的一个内存入口指针 2006.2.17 added
        dout << "para.sys_param already allocated." << std::endl;
    }

    // Get process parameters from table.
    void load_param()
    {
        string user_name;
        string password;
        string connect_string;

        common::read_db_info(user_name, password, connect_string);
        // Database will disconnect when exit its scope.
        database db(user_name, password, connect_string);
        string sql_stmt = "select user_name, passwd, connect_string, ipc_key, udp_port from v_conf_proc_para "
            "where module_id = '" + para.module_id + "' and process_id = '" + para.proc_id + '\'';

        dout <<"["<<__FILE__<<"] LINE["<<__LINE__<<"]sql_stmt = [" << sql_stmt << "] user_name = ["<<user_name<<"] password = ["<<password<<"] connect_string = ["<<connect_string<<"]" << std::endl;

        try
        {
            otl_stream select_t (100,sql_stmt.c_str(),db.get_conn());
            
            if (select_t.eof() ) 
               throw bad_param(__FILE__, __LINE__, 10, "v_conf_proc_para");
            else {
              string tmp_pass;
              select_t >> para.user_name;
              select_t >> tmp_pass;
              select_t >> para.connect_string;
              select_t >> para.ipc_key;
              select_t >> para.port;
              char text[256];
              common::decrypt(tmp_pass.c_str(), text);
              para.password = text;
            }

            select_t.close();
        }
        catch (otl_exception& ex)
        {
            db.disconnect();
            throw  bad_db(__FILE__, __LINE__, 11, ex, sql_stmt);
        }
        db.disconnect();

        dout<< "["<<__FILE__<<"] LINE["<<__LINE__<<"]user_name = [" << para.user_name << "]\n";
        dout<< "["<<__FILE__<<"] LINE["<<__LINE__<<"]connect_string = [" << para.connect_string << "]\n";
        dout<< "["<<__FILE__<<"] LINE["<<__LINE__<<"]ipc_key = " << para.ipc_key << "\n";
        dout<< "["<<__FILE__<<"] LINE["<<__LINE__<<"]port = " << para.port << "\n";
        dout << std::flush;
    }

    // Load all threads' info about this process.
    void load_thread_info()
    {
        // Database will disconnect when exit its scope.
        database db(para.user_name, para.password, para.connect_string);
        string sql_stmt = "select thread_id, thread_name, log_path, switch_mode "
            " from v_conf_thread "
            " where module_id = '" + para.module_id
            + "' and process_id = '" + para.proc_id + "' order by thread_name";

        dout <<"["<<__FILE__<<"] LINE["<<__LINE__<<"] sql_stmt = [" << sql_stmt << "]" << std::endl;

        thread_info info;
        try
        {
            otl_stream select_t (100,sql_stmt.c_str(),db.get_conn());
            
            while (!select_t.eof()) {
                string index;
                int tmp_mode;
                string path_tmp;
                select_t >> index;
                select_t >> info.thread_name;
                select_t >> path_tmp;
                select_t >> tmp_mode;

                if (para.log_path.empty())
                {
                  para.log_path = path_tmp; 
                  para.mode = static_cast<log_file::switch_mode>(tmp_mode);
                }

                info.obj = 0;
                // At begin, every thread is not alive.
                info.status = false;
                threads[index] = info;
            }
            select_t.close();
        }
        catch (otl_exception& ex)
        {
            db.disconnect();
            throw  bad_db(__FILE__, __LINE__, 12, ex, sql_stmt);
        }
        db.disconnect();

        if(threads.size() == 0)
        	throw bad_param(__FILE__, __LINE__, 10, "v_conf_thread");

        map<string, thread_info>::iterator iter;
        dout << "In load_thread_info:\n";
        for (iter = threads.begin(); iter != threads.end(); ++iter)
        {
            dout << "thread_id = [" << iter->first << "]\n";
            dout << "thread_name = [" << iter->second.thread_name << "]\n";
            dout << "log_path = [" << para.log_path << "]\n";
            dout << "mode = [" << para.mode << "]\n";
        }
        dout << std::flush;
    }

    void refresh_thread(const string& msg)
    {
        // Message is devided by ',' and every field is an integer indicating a table.
        vector<string> str_vec;
        common::string_to_array(msg, ',', str_vec);
        vector<int> int_vec;
        for (int i = 0; i < str_vec.size(); i++)
            int_vec.push_back(atoi(str_vec[i].c_str()));

        map<string, thread_info>::iterator iter;
        for (iter = threads.begin(); iter != threads.end(); ++iter)
        {
            if (iter->second.status == true && iter->second.obj->status())
                iter->second.obj->refresh_data(int_vec, 1); // Part refresh.
        }
    }

    void reprocess_thread(const string& thread_id)
    {
        if (thread_id.length() != 8) // Not correct, skip it.
            return;
        map<string, thread_info>::iterator iter = threads.find(thread_id);
        if (iter == threads.end()) // Thread is not in the image.
        {
            lfile->write_log(MISS_THD_IMAGE  + " : " + thread_id);
            ludp->thread_report("alarm", thread_id, "thread", "miss_thread_image", 2, MISS_THD_IMAGE);
        }
        else if (iter->second.status == true && iter->second.obj->status()) // Thread is still alive.
        {
            iter->second.obj->reprocess();
        }
        else // Thread image exists and it's not alive.
        {
            lfile->write_log(THD_STOPPED + " : " + thread_id);
            ludp->thread_report("alarm", thread_id, "thread", "thread_start", 0, THD_STOPPED);
        }
    }

    void dump_fee(const string& thread_id)
    {
        if (thread_id.length() != 8) // Not correct, skip it.
            return;
        map<string, thread_info>::iterator iter = threads.find(thread_id);
        if (iter == threads.end()) // Thread is not in the image.
        {
            lfile->write_log(MISS_THD_IMAGE  + " : " + thread_id);
            ludp->thread_report("alarm", thread_id, "thread", "miss_thread_image", 2, MISS_THD_IMAGE);
        }
        else if (iter->second.status == true && iter->second.obj->status()) // Thread is still alive.
        {
            iter->second.obj->dump_fee("");
        }
        else // Thread image exists and it's not alive.
        {
            lfile->write_log(THD_STOPPED + " : " + thread_id);
            ludp->thread_report("alarm", thread_id, "thread", "thread_start", 0, THD_STOPPED);
        }
    }

    param para;
    log_file* lfile;
    log_udp* ludp;
    map<string, thread_info> threads;
    map<string, dll_info> dlls;
    vector<string> threads_in_use ;
    int alive_threads;
    ostringstream fmt;
};

}
}

#endif

