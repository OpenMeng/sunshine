#include <cstdio>
#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <exception>
#include <iostream>
#include <libgen.h>
#include "common.h"
#include "monitor.h"

#if defined(MEM_DEBUG)
#define new DEBUG_NEW
#define delete DEBUG_DELETE
#endif

using util::database;
using util::param;
using util::log_file;
using std::exception;
using std::cout;
using util::common;

void do_signal(int sig_no)
{
    exit(0);
}

int main(int argc,char** argv)
{
    try
    {
        signal(SIGINT, do_signal);
        param para;

        // Set default value if not provided.
        para.user_name = "router";
        para.password = "router";
        para.connect_string = "test";
        para.port = 8000;
        para.ipc_key = 88888;
        para.log_path = "../log/monitor";
        para.mode = log_file::mode_day;
        para.host_id = "openMeng";
        para.module_id = "02";
        para.proc_id = "01";
        para.thread_id = "02010101";

        int ch;
        while ((ch = getopt(argc, argv, ":u:d:c:o:k:h:p:t:")) != -1)
        {
            switch (ch)
            {
            case 'u':
                para.user_name = optarg;
                break;
            case 'd':
                para.password = optarg;
                break;
            case 'c':
                para.connect_string = optarg;
                break;
            case 'o':
                para.port = atoi(optarg);
                break;
            case 'k':
                para.ipc_key = atoi(optarg);
                break;
            case 'h':
                para.host_id = optarg;
                break;
            case 'p':
                para.proc_id = optarg;
                break;
            case 't':
                para.thread_id = optarg;
                break;
            case ':':
                cout << "Option -" << static_cast<char>(optopt) << " requires an argument." << std::endl;
                exit(1);
            case '?':
                cout << "Usage : " << argv[0] << " -u user_name -d password -c connect_string "
                    "-o udp_port -k ipc_key -h host_id -p proc_id -t thread_id" << std::endl;
                exit(1);
            }
        }

        // Set application's path to the current path.
        chdir(dirname(argv[0]));
        para.db = new database(para.user_name, para.password, para.connect_string, 0, 1, 1);
#if defined(NAMESPACE)
        sunshine::ctrl::monitor::monitor* instance = new sunshine::ctrl::monitor::monitor(&para);
#else
        monitor::monitor* instance = new monitor::monitor(&para);
#endif
        instance->run();
        delete instance;
        return 0;
    }
    catch (exception& ex)
    {
        cout << "In file(" << __FILE__
            << ") at line(" << __LINE__
            << ") " << ex.what() << std::endl;
        exit(1);
    }
}

