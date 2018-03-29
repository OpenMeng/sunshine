#if !defined(__THRESHOLD_H__)
#define __THRESHOLD_H__

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <set>
#include <string>
#include <algorithm>
#include <functional>
#include "database.h"
#include "log_file.h"
#include "user_exception.h"
#include "compiler.h"
#include "tcpsocket.h"
#include "compiler.h"
#include "user_assert.h"

namespace util
{

using std::string;
using std::set;
using std::vector;
using util::tcpsocket;
using util::compiler;

int const MAX_THRESH_IN = 100;
int const MAX_IN_LEN = 256;
int const MAX_OUT_LEN = 256;

struct threshold_t
{
    string object_name;
    double lower_limit;
    double upper_limit;
    int alarm_level;

    bool operator<(const threshold_t& rhs) const
    {
        int ret = object_name.compare(rhs.object_name);
        if (ret < 0)
        {
            return true;
        }
        else if (ret == 0)
        {
            if (upper_limit < rhs.lower_limit)
                return true;
            else
                return false;
        }
        else
        {
            return false;
        }
    }
};

struct fun_t
{
    string object_type;
    string object_name;
    int idx;
};

class threshold
{
public:
    threshold(database& db, log_file& lfile_, tcpsocket& tcp_);
    virtual ~threshold();
    // Check to see whether the record exceed threshold. If so, report an alarm message.
    void alarm(const string& thread_id, const string& msg);

private:
    typedef set<threshold_t> thresh_set;

    bool to_array(const string& msg);
    string get_msg_prefix() const;

    compiler cmpl;
    log_file& lfile;
    tcpsocket& tcp;
    map<string, vector<fun_t> > funs;
    map<string, thresh_set> threshs;
    char* thresh_in[MAX_THRESH_IN];
    ostringstream fmt;
};

}

#endif

