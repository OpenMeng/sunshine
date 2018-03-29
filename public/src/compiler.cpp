#include "compiler.h"

namespace util
{

using std::ofstream;
using std::cout;
using std::ios;
using std::exception;

compiler::compiler(const param* para_)
    : para(para_)
{
    in_rplc.clear();
    out_rplc.clear();
    thread_id = (para == NULL) ? "00000000" : para->thread_id;
    init();
}

compiler::compiler(const string& threadid)
    : para(0)
{
    in_rplc.clear();
    out_rplc.clear();
    thread_id = threadid;
    init();
}

compiler::compiler(const map<string, int>& rplc_, const param* para_)
    : rplc(rplc_),
      para(para_)
{
    in_rplc.clear();
    out_rplc.clear();
    thread_id = (para == NULL) ? "00000000" : para->thread_id;
    init();
}

compiler::compiler(const map<string, int>& rplc_, const string& threadid)
    : rplc(rplc_),
      para(0)
{
    in_rplc.clear();
    out_rplc.clear();
    thread_id = threadid;
    init();
}

compiler::compiler(const map<string, int>& rplc_, const field_map_t& in_rplc_, const map<string, int>& out_rplc_, const param* para_)
    : rplc(rplc_),
    out_rplc(out_rplc),
    para(para_)
{
    in_rplc.push_back(in_rplc_);
    thread_id = (para == NULL) ? "00000000" : para->thread_id;
    init();
}

compiler::compiler(const map<string, int>& rplc_, const vector<field_map_t>& in_rplc_, const map<string, int>& out_rplc_, const param* para_)
    : rplc(rplc_),
    in_rplc(in_rplc_),
    out_rplc(out_rplc),
    para(para_)
{
    thread_id = (para == NULL) ? "00000000" : para->thread_id;
    init();
}

compiler::~compiler()
{
    try
    {
        if (name_vector.size() > 0)
            unload();
    }
    catch (exception& ex)
    {
        dout << ex.what() << std::endl;
        exit(1);
    }

    if (unlink(file_name.c_str()) == -1)
        dout << "Can't remove file " << file_name << '\n';

    unlink(obj_name.c_str()); // It may not succeed, just skip it.
    if (unlink(lib_name.c_str()) == -1)
        dout << "Can't remove file " << lib_name << '\n';
}

void compiler::set(const map<string, int>& rplc_)
{
    rplc = rplc_;
}

void compiler::set(const map<string, int>& rplc_, const field_map_t& in_rplc_, const map<string, int>& out_rplc_)
{
    rplc = rplc_;
    in_rplc.push_back(in_rplc_);
    out_rplc = out_rplc_;
}

void compiler::set(const map<string, int>& rplc_, const vector<field_map_t>& in_rplc_, const map<string, int>& out_rplc_)
{
    rplc = rplc_;
    in_rplc = in_rplc_;
    out_rplc = out_rplc_;
}

int compiler::get_global_size() const
{
    return rplc.size();
}

void compiler::set_global_code(const string& global_code)
{
    write(global_code.c_str());
}

int compiler::create_function(const string& src_code, bool immediate, int record_serial)
{
    string dst_code;
    string tmp;

    fmt.str("");
    fmt << "fun_" << time(0) << name_vector.size();
    tmp = fmt.str();
    name_vector.push_back(tmp);

#if defined(USEACC)
	write("extern \"C\"{\n");
	write("int ");
	write("(void* client, char** global, const char** in, char** out);\n");
	write("}\n\n");
#endif

    precomp(dst_code, src_code, record_serial);
    write("int ");
    write(tmp.c_str());
    write("(void* client, char** global, const char** in, char** out)\n");
    write("{\n");
    write(dst_code.c_str());
    write("return 0;\n");
    write("}\n\n");
    if (immediate)
        compile();

    return name_vector.size() - 1;
}

void compiler::compile()
{
    if (name_vector.size() > 0)
    {
        build();
        load();
    }
}

int compiler::execute(int index, char** global, const char** in, char** out) const
{
#if defined(DEBUG)
    if (global == 0 && debug_info[index].global_size > 0 || debug_info[index].global_size > rplc.size())
        throw bad_param(__FILE__, __LINE__, 112, "Can't use global variables.");
    if (in == 0 && debug_info[index].in_size > 0)
        throw bad_param(__FILE__, __LINE__, 113, "Can't use in variables.");
    if (out == 0 && debug_info[index].out_size > 0)
        throw bad_param(__FILE__, __LINE__, 114, "Can't use out variables.");
#endif

    assert(fun_vector.size() > index);
    return(fun_vector[index])(shm_client, global, in, out);
}

void compiler::lock()
{
    assert(para);
    read_lock(shm_client);
}

void compiler::unlock()
{
    assert(para);
    read_unlock(shm_client);
}

void compiler::check_dts()
{
    assert(para);
    check_dts_server(shm_client);
}

void compiler::init()
{
    while (1)
    {
        fmt.str("");
        fmt << "./." << thread_id << "." << time(0);
        //fmt << "./." << time(0);
        file_name = fmt.str() + ".c";
        obj_name = fmt.str() + ".o";
        lib_name = fmt.str() + ".so";
        log_name = fmt.str() + ".log";

        if (thread_id.length() == 8 && thread_id != "00000000") {
            ostringstream cmdstr;
            cmdstr << "rm ./." << thread_id << ".*";
            ::system(cmdstr.str().c_str());
        }
        if ((fd = open(file_name.c_str(), O_EXCL | O_CREAT | O_WRONLY, 0666)) == -1)
        {
            sleep(1);
            continue;
        }
        // Include head files.
        write("#include <stdio.h>\n");
        write("#include <stdlib.h>\n");
        write("#include <math.h>\n");
        write("#include <errno.h>\n");
        write("#include <assert.h>\n");
        write("#include <string.h>\n");
        write("#include <strings.h>\n");
        write("#include <stdarg.h>\n");
        write("#include <time.h>\n");
        write("#include <unistd.h>\n");
        write("#include <ctype.h>\n");
        write("#include <fcntl.h>\n");
        if (para)
            write("#include \"search.h\"\n");
        write("\n");
        
        handle = 0;

        return;
    }
}

void compiler::precomp(string& dst_code, const string& src_code, int record_serial)
{
    string::const_iterator iter;
    dst_code = "";
    string token;
    ostringstream fmt;

#if defined(DEBUG)
    // Check upper bound of global, in and out arrays.
    int tmp;
    debug_info_t info;
    info.global_size = 0;
    info.in_size = 0;
    info.out_size = 0;
#endif

    for (iter = src_code.begin(); iter != src_code.end();)
    {
        if (*iter == '\\')
        {
            // This charactor and next are a whole.
            dst_code += *iter;
            ++iter;
            dst_code += *iter;
            ++iter;
        }
        else if (*iter == '\"')
        {
            // Const string.
            dst_code += *iter;
            ++iter;
            while (iter != src_code.end())
            {
                // Skip const string.
                dst_code += *iter;
                ++iter;
                if (*(iter-1) == '\"' && *(iter-2) != '\\')
                    break;
            }
        }
        else if (*iter == '/' && (iter  + 1) != src_code.end() && *(iter + 1) == '*')
        {
            // Multi Line Comments
            dst_code += *iter;
            ++iter;
            dst_code += *iter;
            ++iter;
            while (iter != src_code.end())
            {
                // Skip comment string.
                dst_code += *iter;
                ++iter;
                if (*(iter - 1) == '/' && *(iter - 2) == '*')
                    break;
            }
        }
        else if (*iter == '$')
        {
            // Input variables
            dst_code += "in[";
            iter++;
            if (*iter >= '0' && *iter <= '9')
            {
                for (iter; iter != src_code.end(); ++iter)
                {
                    if (*iter >= '0' && *iter <= '9')
                        dst_code += *iter;
                    else
                    {
                        break;
                    }
                }
            }
            else if (isalnum(*iter) || *iter == '_')
            {
                token = "";
                for (iter; iter != src_code.end(); ++iter)
                {
                    if (isalnum(*iter) || *iter == '_')
                        token += *iter;
                    else
                        break;
                }
                //dout<<"token: "<<token<<endl;
                field_map_t::const_iterator map_iter = in_rplc[record_serial].find(token);
                if (map_iter == in_rplc[record_serial].end())
                {
                    // Not found.
                    fmt.str("");
                    fmt << "In varible " << token << " not found in map.";
                    throw bad_msg(__FILE__, __LINE__, 115, fmt.str());
                }
                else
                {
                    fmt.str("");
                    fmt << map_iter->second;
                    dst_code += fmt.str();
                }
            }
            dst_code += ']';
        }
        else if (*iter == '#')
        {
            // Output variables
            dst_code += "out[";
            iter++;
            if (*iter >= '0' && *iter <= '9')
            {
                for (iter; iter != src_code.end(); ++iter)
                {
                    if (*iter >= '0' && *iter <= '9')
                        dst_code += *iter;
                    else
                    {
                        break;
                    }
                }
            }
            else if (isalnum(*iter) || *iter == '_')
            {
                token = "";
                for (iter; iter != src_code.end(); ++iter)
                {
                    if (isalnum(*iter) || *iter == '_')
                        token += *iter;
                    else
                        break;
                }
                //dout<<"token: "<<token<<endl;
                map<string, int>::const_iterator map_iter = out_rplc.find(token);
                if (map_iter == out_rplc.end())
                {
                    // Not found.
                    fmt.str("");
                    fmt << "out varible " << token << " not found in map.";
                    throw bad_msg(__FILE__, __LINE__, 116, fmt.str());
                }
                else
                {
                    fmt.str("");
                    fmt << map_iter->second;
                    dst_code += fmt.str();
                }
            }
            dst_code += ']';
        }
        else if (isalpha(*iter) || *iter == '_')
        {
            // Key word.
            token = "";
            token += *iter;
            for (++iter; iter != src_code.end(); ++iter)
            {
                if (isalnum(*iter) || *iter == '_')
                    token += *iter;
                else
                    break;
            }
            map<string, int>::const_iterator map_iter = rplc.find(token);
            if (map_iter == rplc.end())
            {
                // Not found, no need to replace.
                dst_code += token;
            }
            else
            {
                fmt.str("");
                fmt << map_iter->second;
                dst_code += "global[";
                dst_code += fmt.str();
                dst_code += ']';
            }
        }
        else
        {
            // Other case
            dst_code += *iter;
            ++iter;
        }

#if defined(DEBUG)
        debug_info.push_back(info);
#endif
    }

}

void compiler::build()
{
#if defined(USEACC)
    string cmd = "aCC +DD64 -lm -lc -lnsl -lrt -ldl -b -dynamic";
#elif defined(HPUX)
    string cmd = "cc +DD64 -lm -lc -lnsl -lrt -ldl -b -dynamic";
#elif defined(TRUE64)
    string cmd = "cc -lm -lc -lrt -shared";
#elif defined(AIX)
    string cmd = "cc -q64 -lm -lc -lnsl -lrt -ldl -G -qlanglvl=stdc99";
#elif defined(LINUX)
    string cmd = "cc -lm -lc -lnsl -lrt -ldl -shared -fPIC ";
#elif defined(SOLARIS)
    string cmd = "cc -xarch=generic64 -lm -lc -lnsl -lrt -ldl -G -Kpic";
#else
    #error This platform is not supported.
#endif

#if defined(DEBUG)
    cmd += " -DDEBUG -g -o ";
#elif defined(USEACC)
    cmd += " +O2 -o ";
#elif defined(HPUX)
    cmd += " +O2 -o ";
#elif defined(TRUE64)
    cmd += " -O2 -o ";
#elif defined(AIX)
    cmd += " -O2 -o ";
#elif defined(LINUX)
    cmd += " -O2 -o ";
#elif defined(SOLARIS)
    cmd += " -O -o ";
#else
    #error This platform is not supported.
#endif

    cmd += lib_name;
    cmd += " ";
    cmd += file_name;

    if (para)
#if defined(AIX)
        cmd += " -L../lib -lsearch";
#else
        cmd += " ../lib/libsearch.so";
#endif

    unload();

    dout << "command: "<< cmd << std::endl;

#ifdef DEBUG
    dout << "command: "<< cmd << std::endl;
#endif

    cmd += " 2> " + log_name;

    if (system(cmd.c_str()))
        throw bad_system(__FILE__, __LINE__, 117, bad_system::bad_cmd, cmd + " : " + strerror(errno));

#if !defined(DEBUG) // Strip libary in normal mode.
#if defined(HPUX) || defined(AIX)
    cmd = "strip -X64 " + lib_name;
#elif defined(LINUX) || defined(TRUE64) ||defined(SOLARIS)
    cmd = "strip " + lib_name;
#else
#error This platform is not supported.
#endif
#endif
}

void compiler::load()
{
    dout << "begin of load, lib_name = " << lib_name << endl;
    handle = dlopen(lib_name.c_str(), RTLD_LAZY);
    dout << lib_name.c_str() << std::endl;
    if (!handle)
        throw bad_system(__FILE__, __LINE__, 3, bad_system::bad_dlopen, dlerror());

    if (para)
    {
        dout << "before get create_fun";

        typedef void* (*create_type)(const void* para);
        create_type create_fun = (create_type)dlsym(handle, "create");
        if (create_fun == 0){
            dout << "create() is null" << endl;
            throw bad_system(__FILE__, __LINE__, 118, bad_system::bad_dlsym, dlerror());
        }
        dout << "before get sm_client" << endl;
        shm_client = create_fun(reinterpret_cast<const void*>(para));
        dout << "before get read_lock" << endl;
        read_lock = (lock_handle)dlsym(handle, "read_lock");
        if (read_lock == 0)
            throw bad_system(__FILE__, __LINE__, 119, bad_system::bad_dlsym, dlerror());
        read_unlock = (lock_handle)dlsym(handle, "read_unlock");
        if (read_unlock == 0)
            throw bad_system(__FILE__, __LINE__, 120, bad_system::bad_dlsym, dlerror());
        check_dts_server = (check_handle)dlsym(handle, "check_dts_server");
        if (check_dts_server == 0)
            throw bad_system(__FILE__, __LINE__, 121, bad_system::bad_dlsym, dlerror());
    }

    dout << "before get functions by name" << endl;
    // Load user defined functions.
    vector<string>::iterator iter;
    fun_type fn;
    fun_vector.resize(0);
    for (iter = name_vector.begin(); iter != name_vector.end(); ++iter)
    {
        fn = (fun_type)dlsym(handle, iter->c_str());
        if (!fn)
            throw bad_system(__FILE__, __LINE__, 122, bad_system::bad_dlsym, *iter);
        fun_vector.push_back(fn);
    }
    dout << "before return from load" << endl;
}

void compiler::unload()
{
    if (handle)
    {
        if (para)
        {
            typedef void (*destroy_type)(void* client);
            destroy_type destroy_fun = (destroy_type)dlsym(handle, "destroy");
            if (destroy_fun == 0)
                throw bad_system(__FILE__, __LINE__, 123, bad_system::bad_dlsym, dlerror());
            destroy_fun(shm_client);
        }
        dlclose(handle);
        handle = 0;
    }
}

void compiler::write(const char* buf)
{
    size_t len = strlen(buf);
    if (::write(fd, buf, len) != len)
        throw bad_file(__FILE__, __LINE__, 124, bad_file::bad_write, file_name);
}

}
