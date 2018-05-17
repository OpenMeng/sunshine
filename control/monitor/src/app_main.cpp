#include <cstdio>
#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <strings.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <exception>
#include <iostream>
#include <vector>
#include <libgen.h>
#include "common.h"
#include "app.h"

#if defined(MEM_DEBUG)
#define new DEBUG_NEW
#define delete DEBUG_DELETE
#endif

using util::database;
#if defined(NAMESPACE)
using sunshine::ctrl::app;
#endif
using std::exception;
using std::cout;
using util::common;
using std::vector;

void do_signal(int sig_no)
{
    exit(0);
}

int main(int argc, char** argv)
{
    try
    {

        signal(SIGINT, do_signal);
        app<2>* instance;
        string host_id;
        string module_id;
        string proc_id;
        vector<string> threads;
        int ch;
        while ((ch = getopt(argc, argv, ":h:m:p:t:")) != -1)
        {
            switch (ch)
            {
            case 'h':
                host_id = optarg;
                break;
            case 'm':
                module_id = optarg;
                break;
            case 'p':
                proc_id = optarg;
                break;
            case 't':
                if (strlen(optarg) != 8)
                {
                    cout << "Option -" << static_cast<char>(optopt) << " requires an arguement with length 8." << std::endl;
                    exit(1);
                }
                threads.push_back(optarg);
                break;
            case ':':
                cout << "Option -" << static_cast<char>(optopt) << " requires an argument." << std::endl;
                exit(1);
            case '?':
                cout << "Usage : " << argv[0] << " -h host_id -m module_id -p proc_id [-t thread_id] ..." << std::endl;
                exit(1);
            }
        }

        // Check arg list.
        if (host_id.empty())
        {
            cout << "Option -h must be provided." << std::endl;
            exit(1);
        }
        else if (module_id.length() != 2)
        {
            cout << "Option -m requires an arguement with length 2." << std::endl;
            exit(1);
        }
        else if (proc_id.length() != 2)
        {  
            cout << "Option -p requires an arguement with length 2." << std::endl;
            exit(1);
        }

        // Set application's path to the current path.
        chdir(dirname(argv[0]));

        // Create application.
        if (threads.size() > 0)
            instance = new app<2>(host_id, module_id, proc_id, threads);
        else
            instance = new app<2>(host_id, module_id, proc_id);
        instance->start();
        delete instance;
        return 0;
    }
    catch (exception& ex)
    {
        cout << "In file(" << __FILE__
            << ") at line(" << __LINE__
            << ") " << ex.what() << '\n';
        return 1;
    }
}

