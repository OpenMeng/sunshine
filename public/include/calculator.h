#if !defined(__CALCULATOR_H__)
#define __CALCULATOR_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <dlfcn.h>
#include <exception>
#include "dstream.h"
#include "user_exception.h"
#include "user_assert.h"

namespace util
{

using std::string;
using std::vector;
using std::map;
using std::stack;
using std::exception;

enum token_type
{
    BEG,
    KEY, // $...
    CON, // Constant.
    OP, // Operator.
    END
};

enum priority
{
    INIT = -1,
    LB, // Left bracket.
    PM, // Plus-Minus.
    MDM, // Multiply-Divide-Mod.
    RB // Right bracket.
};

struct token
{
    token_type type;
    priority pri;
    string val;
};

class calculator
{
public:
    typedef map<string, double> value_type;
    typedef value_type* pointer;
    
    calculator();
    calculator(const string& expr_);
    virtual ~calculator();
    void parse();
    void parse(const string& expr_);
    pointer get_variables();
    double execute();

private:
    enum status_type
    {
        UNPARSED,
        PARSED,
        PREPARED
    };

    void get_token(token& tok);
    
    map<string, double> vals;
    string expr;
    string::const_iterator iter;
    status_type status;
    vector<token> suffix_code;
};

}

#endif

