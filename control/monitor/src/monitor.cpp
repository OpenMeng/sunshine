#include "monitor.h"

#if defined(NAMESPACE)

namespace sunshine
{
namespace ctrl
{

#endif

namespace monitor
{

using oracle::occi::Statement;
using oracle::occi::ResultSet;
using oracle::occi::SQLException;
using util::bad_file;
using util::bad_system;
using util::bad_db;
using util::bad_oci;
using util::bad_param;
using std::ostringstream;
using util::common;
using util::datetime;
using util::workthread;
using util::threadcond;

int const MAX_CPUS = 32;

#if defined(NAMESPACE)
IMPLEMENT_DYNCREATE(sunshine::ctrl::monitor, monitor)
#else
IMPLEMENT_DYNCREATE(monitor, monitor)
#endif

extern int getprocs();

scan_dirs::scan_dirs(const string& host_id_, const string& thread_id_, database& db, log_udp& ludp_)
    : host_id(host_id_),
      thread_id(thread_id_),
      ludp(ludp_)
{
    string sql_stmt;
    Statement* stmt = 0;
    ResultSet* rset = 0;
    string para_name;
    dir_t tmp;
    map<string, dir_t> defaults;
    map<string, dir_t>::iterator iter;
    string key;

    try
    {
        // Get default parameter para_values.
        sql_stmt  = "select module_id, para_name, default_value from info_thread_para "
            "where para_name in ('src_dir', 'pattern') order by module_id, para_name";
        stmt = db.create_statement();
        rset = stmt->executeQuery(sql_stmt);
        while (rset->next())
        {
            if (key.empty())
            {
                key = rset->getString(1);
            }
            else if (key != rset->getString(1))
            {
                if (tmp.src_dir.empty() || tmp.pattern.empty())
                {
                    tmp.src_dir = "";
                    tmp.pattern = "";
                    continue;
                }
                else
                {
                    defaults[key] = tmp;
                    tmp.src_dir = "";
                    tmp.pattern = "";
                    key = rset->getString(1);
                }
            }
            para_name = rset->getString(2);
            if (para_name == "src_dir")
                tmp.src_dir = rset->getString(3);
            else
                tmp.pattern = rset->getString(3);
        }
        // Last one.
        if (!key.empty())
        {
            defaults[key] = tmp;
            tmp.src_dir = "";
            tmp.pattern = "";
            key = "";
        }

        stmt->closeResultSet(rset);
        rset = 0;
        // Get thread parameter para_values.
        sql_stmt = "select thread_id, para_name, para_contents from conf_thread_para "
            "where substr(thread_id, 1, 4) in (select module_id || process_id from "
            "conf_host_proc where host_id = '" + host_id +
            "') and para_name in ('src_dir', 'pattern') order by thread_id, para_name";
        rset = stmt->executeQuery(sql_stmt);
        while (rset->next())
        {
            if (key.empty())
            {
                key = rset->getString(1);
            }
            else if (key != rset->getString(1))
            {
                if (tmp.src_dir.empty() || tmp.pattern.empty())
                {
                    iter = defaults.find(key.substr(0, 2));
                    if (iter == defaults.end())
                    {
                        if (tmp.src_dir.empty())
                            tmp.src_dir = iter->second.src_dir;
                        if (tmp.pattern.empty())
                            tmp.pattern = iter->second.src_dir;
                    }
                }
                dirs[key] = tmp;
                tmp.src_dir = "";
                tmp.pattern = "";
                key = rset->getString(1);
            }
            para_name = rset->getString(2);
            if (para_name == "src_dir")
                tmp.src_dir = rset->getString(3);
            else if (para_name == "pattern")
                tmp.pattern = rset->getString(3);
            else
                continue;
        }
        if (!key.empty())
        {
            if (tmp.src_dir.empty() || tmp.pattern.empty())
            {
                iter = defaults.find(key.substr(0, 2));
                if (iter != defaults.end())
                {
                    if (tmp.src_dir.empty())
                        tmp.src_dir = iter->second.src_dir;
                    if (tmp.pattern.empty())
                        tmp.pattern = iter->second.src_dir;
                }
            }
            dirs[key] = tmp;
        }
    }
    catch (SQLException& ex)
    {
        if (rset)
            stmt->closeResultSet(rset);
        db.terminate_statement(stmt);
        throw bad_db(__FILE__, __LINE__, ex);
    }
}

scan_dirs::~scan_dirs()
{
}

void scan_dirs::run()
{
    DIR* dirp;
    dirent ent;
    dirent* result;
    string full_name;
    struct stat stat_buf;
    regex_t reg;
    map<string, dir_t>::iterator iter;
    for (iter = dirs.begin(); iter != dirs.end(); ++iter)
    {
        int count = 0;
        if (regcomp(&reg, iter->second.pattern.c_str(), REG_NOSUB | REG_EXTENDED))
            throw bad_system(__FILE__, __LINE__, bad_system::bad_reg, iter->second.pattern);
        dirp = opendir(iter->second.src_dir.c_str());
        if(!dirp)
            continue;
        while (readdir_r(dirp, &ent, &result) == 0 && result != 0)
        {
            if (strcmp(ent.d_name, ".") == 0
                || strcmp(ent.d_name, "..") == 0)
                continue;
            if (regexec(&reg, ent.d_name, (size_t)0, 0, 0) != 0)
                continue;
            full_name = iter->second.src_dir + '/' + ent.d_name;
            if (::lstat(full_name.c_str(), &stat_buf) < 0)
                throw bad_file(__FILE__, __LINE__, bad_file::bad_lstat, full_name);
            if (S_ISDIR(stat_buf.st_mode) == 0)
                count++;
        }
        closedir(dirp);
        regfree(&reg);
        // Send message to web server.
        fmt.str("");
        fmt << count;
#if defined(DEBUG)
        cout << "Dir message = [" << fmt.str() << "]" << std::endl;
#endif
        ludp.host_report(iter->first, "dir", fmt.str());
    }
}

monitor::monitor(const param* para_)
    : para(*para_),
      db(*para.db),
      lfile(log_file::mode_day, para.log_path, para.thread_id),
      ludp(para.port),
      running(true),
      thd(0)
{
    load_para();
    cond = new threadcond();
    dir_list = new scan_dirs(para.host_id, para.thread_id, db, ludp);
}

monitor::~monitor()
{
    delete cond;
    delete thd;
}

int monitor::run()
{
    try
    {
        // At the first time, it only record statistics and not really report.
        report_cpu();
        while (running)
        {
            report_sys();
            sleep(sleep_time);
        }
    }
    catch (bad_file& ex) // filesystem error.
    {
        ludp.thread_report("alarm", para.thread_id, "file", ex.error_msg(), 2, ex.what());
        cout << ex.what();
    }
    catch (bad_system& ex) // system error.
    {
        ludp.thread_report("alarm", para.thread_id, "system", ex.error_msg(), 2, ex.what());
        cout << ex.what();
    }
    catch (bad_param& ex) // param error.
    {
        ludp.thread_report("alarm", para.thread_id, "param", "bad_param", 2, ex.what());
        cout << ex.what();
    }
    catch (bad_db& ex) // database error.
    {
        fmt.str("");
        fmt << ex.getErrorCode();
        ludp.thread_report("alarm", para.thread_id, "oracle", fmt.str(), 2, ex.what());
        cout << ex.what();
    }
    catch (bad_oci& ex) // database error.
    {
        ludp.thread_report("alarm", para.thread_id, "oracle", ex.error_msg(), 2, ex.what());
        cout << ex.what();
    }
    catch (exception& ex) // Other error.
    {
        ludp.thread_report("alarm", para.thread_id, "other", "unknown", 2, ex.what());
        cout << ex.what();
    }

    return 1;
}

int monitor::start()
{
    thd = new workthread(this);
    thd->start();
    return 0;
}

int monitor::stop()
{
    running = false;
    cond->signal();
    return 0;
}

int monitor::kill()
{
    thd->stop();
    cond->signal();
    return 0;
}

int monitor::status()
{
    return thd->is_running();
}

bool monitor::get_user(int id, string& user)
{
    passwd* pwd;
    pwd = getpwuid(id);
    if (pwd)
    {
        user = pwd->pw_name;
        return true;
    }
    else
    {
        return false;
    }
}

// -------------------------------------------HPUX specification---------------------------------------------
#if defined(HPUX)

// Old cpu time.
struct cpu_time
{
    _T_LONG_T time[PST_MAX_CPUSTATES];
    _T_LONG_T count;
};

// Cpu info is combined of :
// cpu_name|user_percent|sys_percent|idle_percent|other_percent.
void monitor::report_cpu()
{
    static cpu_time ctime[MAX_CPUS];
    static int cpu_count = 0;
    static bool first = true;
    pst_processor processor[MAX_CPUS];
    if (first) // On the first time, we can't get cpu statistics.
    {
        first = false;
        cpu_count = pstat_getprocessor(processor, sizeof(pst_processor), MAX_CPUS, 0);
        for (int i = 0; i < cpu_count; i++)
        {
            ctime[i].count = 0;
            for (int j = 0; j < PST_MAX_CPUSTATES; j++)
            {
                ctime[i].time[j] = processor[i].psp_cpu_time[j];
                ctime[i].count += processor[i].psp_cpu_time[j];
            }
        }
    }
    else // Now we can get cpu statistics.
    {
        pstat_getprocessor(processor, sizeof(pst_processor), MAX_CPUS, 0);
        for (int i = 0; i < cpu_count; i++)
        {
            int j;
            // On the run of process, cpu count can't modify. So using index is alwayse valid.
            _T_LONG_T interval = -ctime[i].count;
            for (j = 0; j < PST_MAX_CPUSTATES; j++)
                interval += processor[i].psp_cpu_time[j];
            if (interval == 0) // Interval is too small.
                continue;

            double user_percent = static_cast<double>((processor[i].psp_cpu_time[0] - ctime[i].time[0])) / interval * 100.0; 
            double sys_percent = static_cast<double>((processor[i].psp_cpu_time[2] - ctime[i].time[2])) / interval * 100.0;
            double idle_percent = static_cast<double>((processor[i].psp_cpu_time[3] - ctime[i].time[3])) / interval * 100.0;
            double other_percent = 100.0 - user_percent - sys_percent - idle_percent;

            // Set new value.
            ctime[i].count = 0;
            for (j = 0; j < PST_MAX_CPUSTATES; j++)
            {
                ctime[i].time[j] = processor[i].psp_cpu_time[j];
                ctime[i].count += processor[i].psp_cpu_time[j];
            }

            // Send message to web server.
            fmt.str("");
            fmt << "CPU" << std::setfill('0') << std::setw(2) << i << "|"
                << user_percent << "|"
                << sys_percent << "|"
                << idle_percent << "|"
                << other_percent;
#if defined(DEBUG)
            cout << "Cpu message = [" << fmt.str() << "]" << std::endl;
#endif
            ludp.host_report(para.thread_id, "cpu", fmt.str());
        }
    }
}

// Mem info is combined of :
// mem_name|user_percent|sys_percent|idle_percent|other_percent.
void monitor::report_mem()
{
    pst_static sta;
    pst_dynamic dyn;

    if (pstat_getstatic(&sta, sizeof(pst_static), 1, 0) == -1)
        throw bad_system(__FILE__, __LINE__, bad_system::bad_other, "pstat_getstatic");

    if (pstat_getdynamic(&dyn, sizeof(pst_dynamic), 1, 0) == -1)
        throw bad_system(__FILE__, __LINE__, bad_system::bad_other, "pstat_getdynamic");

    int total = sta.physical_memory * (sta.page_size / 1024.0 / 1024.0);
    int used = (sta.physical_memory - dyn.psd_free) * (sta.page_size / 1024.0 / 1024.0);
    int avail = dyn.psd_free * (sta.page_size / 1024.0 / 1024.0);
    double percent = static_cast<double>(used) / total * 100.0;
    // Send message to web server.
    fmt.str("");
    fmt << "mem|"
        << total << "|"
        << used << "|"
        << avail << "|"
        << percent;
#if defined(DEBUG)
    cout << "Mem message = [" << fmt.str() << "]" << std::endl;
#endif
    ludp.host_report(para.thread_id, "mem", fmt.str());
}

// Fs info is combined of :
// fs_name|total|used|avail|percent.
void monitor::report_fs()
{
    mntent* mnt;
    FILE* fp;
    struct statfs fs;

    sync();
    if ((fp = fopen(MNT_MNTTAB, "r")) == 0)
        throw bad_file(__FILE__, __LINE__, bad_file::bad_open, "Read mount table");
    
    while ((mnt = getmntent(fp)) != 0)
    {
        if (!strcmp(mnt->mnt_type, MNTTYPE_IGNORE)
            || !strcmp(mnt->mnt_type, MNTTYPE_SWAP))
            continue;

        if (statfs(mnt->mnt_dir, &fs) < 0)
        {
            fclose(fp);
            throw bad_file(__FILE__, __LINE__, bad_file::bad_statfs, mnt->mnt_dir);
        }

        int total = fs.f_blocks * (fs.f_bsize / 1024.0 / 1024.0);
        int used = (fs.f_blocks - fs.f_bfree) * (fs.f_bsize / 1024.0 / 1024.0);
        double percent = static_cast<double>(used) / total * 100.0;
        // Send message to web server.
        fmt.str("");
        fmt << mnt->mnt_dir << "|"
            << total << "|"
            << used << "|"
            << static_cast<int>(fs.f_bfree * (fs.f_bsize / 1024.0 / 1024.0)) << "|"
            << percent;
#if defined(DEBUG)
        cout << "Fs message = [" << fmt.str() << "]" << std::endl;
#endif
        ludp.host_report(para.thread_id, "fs", fmt.str());
    }

    fclose(fp);
}

// Proc info is combined of :
// process_id|proc_start|command_line|user_name|process_name|mem_used|stat|utime|stime.
void monitor::report_proc()
{
    pst_status status;
    for (int idx = 0; pstat_getproc(&status, sizeof(pst_status), 1, idx) == 1; idx = status.pst_idx+1)
    {
        datetime dt(status.pst_start);
        string proc_start;
        dt.iso_string(proc_start);
        string user_name;
        if (!get_user(status.pst_uid, user_name))
            user_name = "unknown";
        // Send message to web server.
        fmt.str("");
        fmt << status.pst_pid << "|"
            << proc_start << "|"
            << status.pst_cmd << "|"
            << user_name << "|"
            << status.pst_ucomm << "|"
            << (status.pst_ssize + status.pst_tsize + status.pst_dsize) * (getpagesize() / 1024.0) << "|"
            << status.pst_stat << "|"
            << status.pst_utime << "|"
            << status.pst_stime;
#if defined(DEBUG)
        cout << "Proc message = [" << fmt.str() << "]" << std::endl;
#endif
        ludp.host_report(para.thread_id, "proc", fmt.str());
    }
}

// --------------------------------------------AIX specification---------------------------------------------
#elif defined(AIX)

// Old cpu time.
struct cpu_time
{
    u_longlong_t time[3];
    u_longlong_t count;
};

void monitor::report_cpu()
{
    perfstat_id_t name;
    strcpy(name.name, "");

    static cpu_time ctime[MAX_CPUS];
    static int cpu_count = perfstat_cpu(0, 0, sizeof(perfstat_cpu_t), 0);
    static bool first = true;
    perfstat_cpu_t perf_cpu[MAX_CPUS];
    if (first) // On the first time, we can't get cpu statistics.
    {
        first = false;
        if (perfstat_cpu(&name, perf_cpu, sizeof(perfstat_cpu_t), cpu_count) >= 0)
        {
            for (int i = 0; i < cpu_count; i++)
            {
                ctime[i].time[0] = perf_cpu[i].user;
                ctime[i].time[1] = perf_cpu[i].sys;
                ctime[i].time[2] = perf_cpu[i].idle;
                ctime[i].count = perf_cpu[i].user + perf_cpu[i].sys
                    + perf_cpu[i].idle + perf_cpu[i].wait;
            }
        }
    }
    else // Now we can get cpu statistics.
    {
        if (perfstat_cpu(&name, perf_cpu, sizeof(perfstat_cpu_t), cpu_count) >= 0)
        {
            for (int i = 0; i < cpu_count; i++)
            {
                // On the run of process, cpu count can't modify. So using index is alwayse valid.
                u_longlong_t interval = perf_cpu[i].user + perf_cpu[i].sys
                    + perf_cpu[i].idle + perf_cpu[i].wait - ctime[i].count;
                if (interval == 0) // Interval is too small.
                    continue;

                double user_percent = static_cast<double>((perf_cpu[i].user - ctime[i].time[0])) / interval * 100.0;
                double sys_percent = static_cast<double>((perf_cpu[i].sys - ctime[i].time[1])) / interval * 100.0;
                double idle_percent = static_cast<double>((perf_cpu[i].idle - ctime[i].time[2])) / interval * 100.0;
                double other_percent = 100.0 - user_percent - sys_percent - idle_percent;

                // Set new value
                ctime[i].time[0] = perf_cpu[i].user;
                ctime[i].time[1] = perf_cpu[i].sys;
                ctime[i].time[2] = perf_cpu[i].idle;
                ctime[i].count = perf_cpu[i].user + perf_cpu[i].sys
                    + perf_cpu[i].idle + perf_cpu[i].wait;

                // Send message to web server.
                fmt.str("");
                fmt << perf_cpu[i].name << "|"
                    << user_percent << "|"
                    << sys_percent << "|"
                    << idle_percent << "|"
                    << other_percent;
#if defined(DEBUG)
                cout << "Cpu message = [" << fmt.str() << "]" << std::endl;
#endif
                ludp.host_report(para.thread_id, "cpu", fmt.str());
            }
        }
    }
}

void monitor::report_mem()
{
    perfstat_memory_total_t perfstat_memory;
    if (perfstat_memory_total(0, &perfstat_memory, sizeof(perfstat_memory_total_t), 1) >= 0)
    {
        // The unit is 4K, so change to M.
        int total = perfstat_memory.real_total / 256;
        int used = perfstat_memory.real_inuse / 256;
        int avail = perfstat_memory.real_free / 256;
        double percent = static_cast<double>(used) / total * 100.0;
        // Send message to web server.
        fmt.str("");
        fmt << "mem|"
            << total << "|"
            << used << "|"
            << avail << "|"
            << percent;
#if defined(DEBUG)
        cout << "Mem message = [" << fmt.str() << "]" << std::endl;
#endif
        ludp.host_report(para.thread_id, "mem", fmt.str());
    }
}

// Fs info is combined of :
// fs_name|total|used|avail|percent.
void monitor::report_fs()
{
    struct fstab* mnt;
    struct statfs fs;
    
    sync();
    setfsent();
    while (mnt = getfsent())
    {
        if (!strcmp(mnt->fs_file, "/proc"))
            continue;

        if (statfs(mnt->fs_file, &fs) < 0)
        {
            endfsent();
            throw bad_file(__FILE__, __LINE__, bad_file::bad_statfs, mnt->fs_file);
        }

        int total = fs.f_blocks * (fs.f_bsize / 1024.0 / 1024.0);
        int used = (fs.f_blocks - fs.f_bfree) * (fs.f_bsize / 1024.0 / 1024.0);
        double percent = static_cast<double>(used) / total * 100.0;
        // Send message to web server.
        fmt.str("");
        fmt << mnt->fs_file << "|"
            << total << "|"
            << used << "|"
            << static_cast<int>(fs.f_bfree * (fs.f_bsize / 1024.0 / 1024.0)) << "|"
            << percent;
#if defined(DEBUG)
        cout << "Fs message = [" << fmt.str() << "]" << std::endl;
#endif
        ludp.host_report(para.thread_id, "fs", fmt.str());
    }
    endfsent();
}

// Proc info is combined of :
// process_id|proc_start|command_line|user_name|process_name|mem_used|stat|utime|stime.
void monitor::report_proc()
{
    procsinfo procs_info;
    pid_t pid = 0;
    while (getprocs(&procs_info, sizeof(procsinfo), 0, 0, &pid, 1) == 1)
    {
        datetime dt(procs_info.pi_start);
        string proc_start;
        dt.iso_string(proc_start);
        string user_name;
        if (!get_user(procs_info.pi_uid, user_name))
            user_name = "unknown";
        string proc_name;
        char* ptr = strchr(procs_info.pi_comm, ' ');
        if (ptr == 0)
            proc_name = procs_info.pi_comm;
        else
            proc_name.assign(procs_info.pi_comm, ptr - 1);

        int mem_used = procs_info.pi_size * (getpagesize() / 1024.0);
        // Send message to web server.
        fmt.str("");
        fmt << procs_info.pi_pid << "|"
            << proc_start << "|"
            << procs_info.pi_comm << "|"
            << user_name << "|"
            << proc_name << "|"
            << mem_used << "|";
        switch (procs_info.pi_state)
        {
        case SNONE:
            fmt << "E|";
            break;
        case SIDL:
            fmt << "C|";
            break;
        case SZOMB:
            fmt << "Z|";
            break;
        case SSTOP:
            fmt << "S|";
            break;
        case SACTIVE:
            fmt << "A|";
            break;
        case SSWAP:
            fmt << "P|";
            break;
        default:
            fmt << "U|";
            break;
        }
        fmt << procs_info.pi_state << "|"
            << procs_info.pi_utime << "|"
            << procs_info.pi_stime;
#if defined(DEBUG)
        cout << "Proc message = [" << fmt.str() << "]" << std::endl;
#endif
        ludp.host_report(para.thread_id, "proc", fmt.str());
    }
}

#endif

void monitor::report_net()
{
}

/*
void monitor::report_net()
{
    string cmd;
    char line_buf[1024];
    FILE* fp;
    char* ptr;
    for (iter = pos.first; iter != pos.second; ++iter)
    {
#if defined(SALARIS)
        cmd = "ping -s " + iter->host_ip + " -n 1";
#elif defined(LINUX)
        cmd = "ping " + iter->host_ip + " -w 1";
#else
        cmd = "ping " + iter->host_ip + " -n 1";
#endif

        if ((fp = popen(cmd.c_str(), "r")) == 0)
            throw bad_file(__FILE__, __LINE__, bad_file::bad_popen, cmd);

        while (fgets(line_buf, 1024, fp))
        {
            if ((ptr = strchr(line_buf, '%')) == 0)
                continue;
            ptr--;
            while (isdigit(*ptr))
                ptr--;
            info.object_type = "NET";
            info.object_name = iter->host_ip;
            info.total_size = 1;
            info.percent = atoi(ptr);
            mons.push_back(info);
            break;
        }
        pclose(fp);
    }
}

void monitor::get_index()
{
    string sql_stmt;
    Statement* stmt = 0;
    ResultSet* rset = 0;
    index_info info;

    try
    {
        // Get default parameter para_values.
        sql_stmt  = "select owner, index_name, table_owner, table_name, table_type "
            "from dba_indexes where status <> 'VALID'";
        stmt = db.create_statement();
        rset = stmt->executeQuery(sql_stmt);
        while (rset->next())
        {
            info.owner = stmt->getString(1);
            info.index_name = stmt->getString(2);
            info.table_owner = stmt->getString(3);
            info.table_name = stmt->getString(4);
            info.table_type = stmt->getString(5);
            // Send abnormal infomation.
        }
    }
    catch (SQLException& ex)
    {
        if (rset)
            stmt->closeResultSet(rset);
        db.terminate_statement(stmt);
        throw bad_db(__FILE__, __LINE__, ex);
    }

    stmt->closeResultSet(rset);
    db.terminate_statement(stmt);
}

void monitor::get_table()
{
    string sql_stmt;
    Statement* stmt = 0;
    ResultSet* rset = 0;
    index_info info;

    try
    {
        // Get default parameter para_values.
        sql_stmt  = "select owner, index_name, table_owner, table_name, table_type "
            "from dba_indexes where status <> 'VALID'";
        stmt = db.create_statement();
        rset = stmt->executeQuery(sql_stmt);
        while (rset->next())
        {
            info.owner = stmt->getString(1);
            info.index_name = stmt->getString(2);
            info.table_owner = stmt->getString(3);
            info.table_name = stmt->getString(4);
            info.table_type = stmt->getString(5);
            // Send abnormal infomation.
        }
    }
    catch (SQLException& ex)
    {
        if (rset)
            stmt->closeResultSet(rset);
        db.terminate_statement(stmt);
        throw bad_db(__FILE__, __LINE__, ex);
    }

    stmt->closeResultSet(rset);
    db.terminate_statement(stmt);
}
*/
void monitor::report_sys()
{
    report_cpu();
    report_mem();
    report_fs();
    report_proc();
    report_net();
    dir_list->run();
}

void monitor::load_para()
{
    string sql_stmt;
    Statement* stmt = 0;
    ResultSet* rset = 0;
    string para_name;

    try
    {
        // Get default parameter para_values.
        sql_stmt  = "select para_name, default_value from "
            "info_thread_para where module_id = '" + para.module_id + '\'';
        stmt = db.create_statement();
        rset = stmt->executeQuery(sql_stmt);
        while (rset->next())
        {
            para_name = rset->getString(1);
            if (para_name == "sleep_time")
                sleep_time = atoi(rset->getString(2).c_str());
            else
                continue;
        }
        
        stmt->closeResultSet(rset);
        rset = 0;
        // Get thread parameter para_values.
        sql_stmt = "select para_name, para_contents from "
            "conf_thread_para where thread_id = '" + para.thread_id + '\'';
        rset = stmt->executeQuery(sql_stmt);
        while (rset->next())
        {
            para_name = rset->getString(1);
            if (para_name == "sleep_time")
                sleep_time = atoi(rset->getString(2).c_str());
            else
                continue;
        }
    }
    catch (SQLException& ex)
    {
        if (rset)
            stmt->closeResultSet(rset);
        db.terminate_statement(stmt);
        throw bad_db(__FILE__, __LINE__, ex);
    }

    stmt->closeResultSet(rset);
    db.terminate_statement(stmt);
}

void monitor::sleep(int seconds)
{
    fmt.str("");
    fmt << "sleep " << seconds << " seconds.";
    ludp.thread_report("message", para.thread_id, fmt.str());
    cond->wait(seconds);
}

}

#if defined(NAMESPACE)

}
}

#endif

