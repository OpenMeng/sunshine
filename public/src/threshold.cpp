#include "threshold.h"

namespace util
{

using std::string;
using std::set;
using std::vector;
using std::cout;
using util::tcpsocket;
using util::compiler;

threshold::threshold(database& db, log_file& lfile_, tcpsocket& tcp_)
    : cmpl(),
      lfile(lfile_),
      tcp(tcp_)
{
    // Load parameters and create functions.
    string sql_stmt = "select thread_id, object_type, object_name, rule_threshold, lower_limit,"
        "upper_limit, alarm_level from v_conf_thread_threshold order by thread_id, object_type, object_name";

    threshold_t record;
    thresh_set record_set;
    string thread_id;
    string tmp_thread_id;
    string tmp_object_type;
    string tmp_object_name;
    string tmp_rule_threshold;
    fun_t fun;
    vector<fun_t> fun_set;
    try
    {
      otl_stream stat (1000,sql_stmt.c_str(),db.get_conn());
      while (!stat.eof ()){
            stat >> tmp_thread_id;
            stat >> tmp_object_type;
            stat >> tmp_object_name;
            stat >> tmp_rule_threshold;

            if (thread_id.empty()) // First time.
            {
                thread_id = tmp_thread_id;
                dout << "thread_id = [" << thread_id << "]" << std::endl;
            }
            if (thread_id != tmp_thread_id) // Switch to another thread.
            {
                threshs.insert(std::make_pair(thread_id, record_set));
                record_set.clear();
                funs.insert(std::make_pair(thread_id, fun_set));
                fun_set.clear();
                thread_id = tmp_thread_id;
                record.object_name = "";
                dout << "thread_id = [" << thread_id << "]" << std::endl;
            }
            if (record.object_name.empty() || record.object_name != tmp_object_name)
            { // Switch to another object_name, so create another function.
                record.object_name = tmp_object_name;

                dout << "object_name = [" << record.object_name << "]\n";
                dout << "rule_threshold = [" << tmp_rule_threshold << "]\n";
                dout << std::flush;

                // Create function
                fun.object_type = tmp_object_type;
                fun.object_name = record.object_name;
                try {
                    fun.idx = cmpl.create_function(tmp_rule_threshold);
                } catch (exception& ex) {
                    throw bad_msg(__FILE__, __LINE__, 379, ex.what());
                }
                fun_set.push_back(fun);
            }
            stat >> record.lower_limit ;
            stat >> record.upper_limit ;
            stat >> record.alarm_level ;
            
            dout << "lower_limit = [" << record.lower_limit << "]\n";
            dout << "upper_limit = [" << record.upper_limit << "]\n";
            dout << "alarm_level = [" << record.alarm_level << "]\n";

            record_set.insert(record);
      }
      
        if (record_set.size() > 0)
            threshs.insert(std::make_pair(thread_id, record_set));
        if (fun_set.size() > 0)
            funs.insert(std::make_pair(thread_id, fun_set));
    }
     catch (otl_exception& p) {
        throw bad_db(__FILE__, __LINE__, 92, p, sql_stmt);
     }
    // Allocate memory for calculate.
    for (int i = 0; i < MAX_THRESH_IN; i++)
        thresh_in[i] = new char[MAX_IN_LEN];
    
    // Generate functions and load.
    cmpl.compile();
}

threshold::~threshold()
{
    for (int i = 0; i < MAX_THRESH_IN; i++)
        delete[] thresh_in[i];
}

bool threshold::to_array(const string& msg)
{
    string::size_type pos_prev = 0;
    string::size_type pos;
    int i = 0;
    int len;
    // If topic is progress, skip first 2 fields: topic, thread_id;
    // otherwise host, skip first 3 fields: topic, thread_id, fmt_name.
    pos = msg.find_first_of('|', 0);
    if (pos == string::npos)
        return false;
    pos = msg.find_first_of('|', pos + 1);
    if (pos == string::npos)
        return false;
    if (!memcmp(msg.c_str(), "host", 4))
    {
        pos = msg.find_first_of('|', pos + 1);
        if (pos == string::npos)
            return false;
    }
    pos_prev = pos + 1;
    while ((pos = msg.find_first_of('|', pos_prev)) != string::npos)
    {
        len = pos - pos_prev;
        if (len >= MAX_IN_LEN)
            return false;
        if (i >= MAX_THRESH_IN - 1)
            return false;
        memcpy(thresh_in[i], msg.c_str() + pos_prev, len);
        thresh_in[i][len] = '\0';
        i++;
        pos_prev = pos + 1;
    }
    // Last field.
    len = msg.length() - pos_prev - 1;
    if (len >= MAX_IN_LEN)
        return false;
    strcpy(thresh_in[i], msg.c_str() + pos_prev);
    return true;
}

void threshold::alarm(const string& thread_id, const string& msg)
{
    fmt.str("");
    if (!to_array(msg))
    {
        fmt << "thread_id = [" << thread_id
            << "] message = [" << msg
            << "] can't pass format conversion.";
        lfile.write_log(fmt.str());
        return;
    }
    map<string, vector<fun_t> >::iterator fun_iter = funs.find(thread_id);
    if (fun_iter == funs.end()) // Not found.
        return;
    map<string, thresh_set>::iterator thresh_iter = threshs.find(thread_id);
    if (thresh_iter == threshs.end()) // Not found.
        return;

    char out_value[MAX_OUT_LEN];
    char* out[1] = { out_value };
    threshold_t key;
    vector<fun_t>::const_iterator iter;
    set<threshold_t>::const_iterator set_iter;
    for (iter = fun_iter->second.begin(); iter != fun_iter->second.end(); ++iter)
    {
        key.object_name = iter->object_name;
        cmpl.execute(iter->idx, 0, const_cast<const char**>(thresh_in), out);

        dout << "result = [" << out_value << "]" << std::endl;

        key.lower_limit = atof(out_value);
        key.upper_limit = key.lower_limit;
        set_iter = thresh_iter->second.find(key);
        if (set_iter != thresh_iter->second.end()) // Send message.
        {
            // Write to log file.
            fmt.str("");
            fmt << "thread_id = [" << thread_id
                << "] object_type = [" << iter->object_type
                << "] object_name = [" << iter->object_name
                << "] alarm_level = [" << set_iter->alarm_level
                << "] content = [" << out_value << "]";
            lfile.write_log(fmt.str());
            // Return message to send to web server.
            fmt.str("");
            fmt << get_msg_prefix()
                << "alarm|" << thread_id
                << "|" << iter->object_type
                << "|" << iter->object_name 
                << "|" << set_iter->alarm_level
                << "|" << out_value;
            tcp.send(fmt.str());
        }
    }
}

string threshold::get_msg_prefix() const
{
    datetime dt(time(0));
    string dts;
    dt.iso_string(dts);
    return dts + "|";
}

}

