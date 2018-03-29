#if !defined(__SIMPLE_REG_H__)
#define __SIMPLE_REG_H__

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
#include "user_exception.h"
#include "user_assert.h"

namespace util
{

using std::string;
using std::vector;

class simple_reg
{
public:
    typedef vector<string> token_vector;
    
    simple_reg();
    simple_reg(const string& expr_);
    virtual ~simple_reg();
    void parse(token_vector& result);
    void parse(const string& expr_, token_vector& result);

private:
    // Combine strings from any of first plus any of second.
    static void concat(token_vector& result, const token_vector& first, const token_vector& second);
    // Merge strings of first and second to first.
    static void merge(token_vector& first, const token_vector& second);
    void get_token(string& token);
    void do_op(char op);

    string expr;
    string::iterator iter;
    vector<token_vector> tokens_vector;
};

}

#endif


