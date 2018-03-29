#if !defined(__COMPILER_H__)
#define __COMPILER_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <exception>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include "common.h"
#include "user_exception.h"
#include "user_assert.h"
#include "scan_file.h"
#include "dstream.h"

namespace util
{

using std::string;
using std::vector;
using std::ofstream;
using std::map;
using std::exception;
using std::ostringstream;

struct debug_info_t
{
    int global_size;
    int in_size;
    int out_size;
};

typedef map<string, int> field_map_t;

class compiler
{
public:
    typedef int (*fun_type)(void* client, char** global, const char** in, char** out);
    typedef void (*lock_handle)(void* client);
    typedef void (*check_handle)(void* client);

    compiler(const param* para_ = 0);
    compiler(const string& threadid);
    compiler(const map<string,int>& rplc_,const string& threadid);
    // Set replace map and initialize.
    compiler(const map<string, int>& rplc_, const param* para_ = 0);
    compiler(const map<string, int>& rplc_, const field_map_t& in_rplc_, const map<string, int>& out_rplc_, const param* para_ = 0);
    compiler(const map<string, int>& rplc_, const vector<field_map_t>& in_rplc_, const map<string, int>& out_rplc_, const param* para_ = 0);
    virtual ~compiler();
    // Set replace map and pass ipc_key etc. if neccessary.
    void set(const map<string, int>& rplc_);
    // Set replace map, include global varibles and in varibles and out varibles
    void set(const map<string, int>& rplc_, const field_map_t& in_rplc_, const map<string, int>& out_rplc_);
    void set(const map<string, int>& rplc_, const vector<field_map_t>& in_rplc_, const map<string, int>& out_rplc_);
    // Get global size.
    int get_global_size() const;
    void set_global_code(const string& global_code);
    // Create new function according to code string and maybe delay.
    // Return new function index.
    // This function doesn't affect other function indexes.
    int create_function(const string& src_code, bool immediate = false, int record_serial = 0);
    // Build and load.
    void compile();
    // Execute function according to function index.
    int execute(int index, char** global, const char** in, char** out) const;
    // Lock share memory for reading.
    void lock();
    // Unlock share mamory for reading.
    void unlock();
    // Check dts server
    void check_dts();

private:
    // Initializing, called by constructor.
    void init();
    // Precomp source code to dest code, replace some variables.
    void precomp(string& dst_code, const string& src_code, int record_serial);
    // Call cc to generate library.
    void build();
    // Load library and functions. After that, execute can be called.
    void load();
    // Unload library, free resouces first.
    void unload();
    // Write to temporily C file.
    void write(const char* buf);

    // Replacement from variable name to global variable index.
    map<string, int> rplc;
    // Replacement from variable name like $... to in variable index.
    vector<field_map_t> in_rplc;
    // Replacement from variable name like #... to out variable index.
    map<string, int> out_rplc;
    // Whether libsearch.so is needed, if needed, para must be provided,
    // otherwise ignore it.
    const param* para;
    // Source file name generated automatically.
    string file_name;
    // Object file name.
    string obj_name;
    // library file name.
    string lib_name;
    string log_name;
    // Record all function-names.
    vector<string> name_vector;
    // Record all function-address.
    vector<fun_type> fun_vector;
    // Pointer used by dlopen.
    string thread_id;
    void* handle;
    lock_handle read_lock;
    lock_handle read_unlock;
    check_handle check_dts_server;
    int fd;
    void* shm_client;
    ostringstream fmt;
    scan_file<>* fscan;
#if defined(DEBUG)
    // Record every function's global/in/out array size.
    vector<debug_info_t> debug_info;
#endif
};

}

#endif

