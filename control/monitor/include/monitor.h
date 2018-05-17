#ifndef  __MONITOR_H__
#define __MONITOR_H__

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <netdb.h>
#include <regex.h>
#include <signal.h>

#ifdef SYSV
#include <sys/stropts.h>
#endif

#if defined(HPUX)

#include <sys/pstat.h>
#include <mntent.h>
#include <sys/fs.h>

#elif defined(SALARIS)

#include <sys/sockio.h>
#include <sys/mntent.h>
#include <sys/mnttab.h>
#include <sys/fs/ufs_fs.h>
#include <sys/procfs.h>
#include <sys/sysinfo.h>

#elif defined(AIX)

#include <fstab.h>
#include <sys/mntctl.h>
#include <sys/vmount.h>
#include <procinfo.h>
#include <libperfstat.h>

extern "C" int getprocs(struct procsinfo *ProcessBuffer, int ProcessSize,
    struct fdsinfo *FileBuffer, int FileSize, pid_t *IndexPointer, int Count);

#endif

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include "database.h"
#include "log_file.h"
#include "common.h"
#include "thread.h"
#include "object.h"

#if defined(NAMESPACE)

namespace sunshine
{
namespace ctrl
{

#endif

namespace monitor
{

using util::log_file;
using util::log_udp;
using std::string;
using std::vector;
using std::map;
using std::ostringstream;
using util::database;
using util::param;
using util::object;
using util::workthread;
using util::threadcond;
using util::runtimeclass;
using util::CLASSINIT;

struct dir_t
{
    string src_dir;
    string pattern;
};

class scan_dirs
{
public:
    scan_dirs(const string& host_id_, const string& thread_id_, database& db, log_udp& ludp_);
    virtual ~scan_dirs();
    void run();

private:
    map<string, dir_t> dirs;
    string host_id;
    string thread_id;
    log_udp& ludp;
    ostringstream fmt;
};

class monitor : public object
{
public:
    monitor(const param* para_);
    virtual ~monitor();
    int run();
    int start();
    int stop();
    int kill();
    int status();

private:
    void load_para();
    bool get_user(int id, string& user);
    void report_cpu();
    void report_mem();
    void report_fs();
    void report_proc();
    void report_net();
    void report_db();
    void report_sys();
    void sleep(int seconds);

    const param para;
    database db;
    log_file lfile;
    log_udp ludp;
    int sleep_time;
    // Judge whether or not stop thread.
    bool running;
    // Thread.
    threadcond* cond;
    workthread* thd;
    ostringstream fmt;

    scan_dirs* dir_list;

    DECLARE_DYNCREATE(monitor)
};

}

#if defined(NAMESPACE)

}
}

#endif

#endif

