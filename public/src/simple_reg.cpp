#include "simple_reg.h"

namespace util
{

using std::string;
using std::vector;
using std::map;
using std::stack;
using std::exception;
using util::bad_file;
using util::bad_system;

simple_reg::simple_reg()
{
}

simple_reg::simple_reg(const string& expr_)
    : expr(expr_)
{
}

simple_reg::~simple_reg()
{
}

void simple_reg::parse(token_vector& result)
{
    string token;
    char op;
    stack<char> op_stack;
    token_vector tokens;
    
    iter = expr.begin();
    while (iter != expr.end())
    {
        if (*iter == '(')
        {
            op_stack.push('(');
            ++iter;
        }
        else if (*iter == ')')
        {
            while (1)
            {
                if (op_stack.size() == 0)
                    throw bad_param(__FILE__, __LINE__, 287, expr);
                op = op_stack.top();
                op_stack.pop();
                if (op == '(') // End of brackets.
                    break;
                else
                    do_op(op);
            }
            ++iter;
            if (iter != expr.end() && *iter != ')' && *iter != '|')
                op_stack.push('+');
        }
        else if (*iter == '|')
        {
            while (op_stack.size() > 0)
            {
                op = op_stack.top();
                if (op == '(')
                {
                    break;
                }
                else
                {
                    op_stack.pop();
                    do_op(op);
                }
            }
            op_stack.push('|');
            ++iter;
        }
        else
        {
            get_token(token);
            tokens.resize(0);
            tokens.push_back(token);
            tokens_vector.push_back(tokens);
            // Add operation if needed.
            if (iter != expr.end() && *iter == '(')
                op_stack.push('+');
        }
    }

    while (op_stack.size() > 0)
    {
        op = op_stack.top();
        op_stack.pop();
        do_op(op);
    }
    
    if (tokens_vector.size() != 1)
        throw bad_param(__FILE__, __LINE__, 287, expr);
    result = *tokens_vector.begin();
}

void simple_reg::parse(const string& expr_, token_vector& result)
{
    expr = expr_;
    parse(result);
}

void simple_reg::concat(token_vector& result, const token_vector& first, const token_vector& second)
{
    token_vector::const_iterator iter1;
    token_vector::const_iterator iter2;
    result.resize(0);
    for (iter1 = first.begin(); iter1 != first.end(); ++iter1)
    {
        for (iter2 = second.begin(); iter2 != second.end(); ++iter2)
            result.push_back(*iter1 + *iter2);
    }
}

void simple_reg::get_token(string& token)
{
    token = "";
    for (; iter != expr.end() && *iter != '(' && *iter != ')' && *iter != '|'; ++iter)
        token += *iter;
}

void simple_reg::do_op(char op)
{
    token_vector tokens;
    vector<token_vector>::reverse_iterator iter1;
    vector<token_vector>::reverse_iterator iter2;
    if (op == '+')
    {
        if (tokens_vector.size() < 2)
            throw bad_param(__FILE__, __LINE__, 287, expr);
        concat(tokens, *(tokens_vector.rbegin() + 1), *tokens_vector.rbegin());
        tokens_vector.pop_back();
        tokens_vector.pop_back();
        tokens_vector.push_back(tokens);
    }
    else if (op == '|')
    {
        if (tokens_vector.size() < 2)
            throw bad_param(__FILE__, __LINE__, 287, expr);
        iter1 = tokens_vector.rbegin() + 1;
        iter2 = tokens_vector.rbegin();
        iter1->insert(iter1->end(), iter2->begin(), iter2->end());
        tokens_vector.pop_back();
    }
}

}

