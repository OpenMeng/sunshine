#include "calculator.h"

namespace util
{

using namespace std;

calculator::calculator()
{
    iter = expr.begin();
    status = UNPARSED;
}

calculator::calculator(const string& expr_)
    : expr(expr_)
{
    iter = expr.begin();
    status = UNPARSED;
}

calculator::~calculator()
{
}

void calculator::parse()
{
    stack<token> op_stack;
    token tok;
    
    // Insert a minimum priority operator.
    tok.type = BEG;
    tok.pri = INIT;
    tok.val = "";
    op_stack.push(tok);
    
    while (1)
    {
        get_token(tok);
        if (tok.type == OP)
        {
            if (op_stack.top().pri == LB) // Push LB.
            {
                op_stack.push(tok);
            }
            else if (op_stack.top().pri == RB)
            {
                while (op_stack.top().pri != LB) // Pop up operators until LB.
                {
                    suffix_code.push_back(op_stack.top());
                    op_stack.pop();
                }
                op_stack.pop(); // Pop up LB.
            }
            else
            {
                while (op_stack.top().pri >= tok.pri) // Pop up operators.
                {
                    suffix_code.push_back(op_stack.top());
                    op_stack.pop();
                }
                op_stack.push(tok);
            }
            
        }
        else if (tok.type == CON)
        {
            suffix_code.push_back(tok);
        }
        else if (tok.type == KEY)
        {
            vals[tok.val] = 0.0;
            suffix_code.push_back(tok);
        }
        else if (tok.type == END)
        {
            while (op_stack.top().pri != INIT) // Pop up operators until INIT.
            {
                suffix_code.push_back(op_stack.top());
                op_stack.pop();
            }
            break;
        }
    }
    
    status = PARSED;
}

void calculator::parse(const string& expr_)
{
    expr = expr_;
    vals.clear();
    iter = expr.begin();
    suffix_code.clear();
    parse();
}

calculator::pointer calculator::get_variables()
{
    status = PREPARED;
    return &vals;
}

double calculator::execute()
{
    if (status != PREPARED)
        throw bad_param(__FILE__, __LINE__, 86, expr);

    value_type::iterator map_iter;
    for (map_iter = vals.begin(); map_iter != vals.end(); ++map_iter)
        dout << map_iter->first << " = [" << map_iter->second << "]\n";
    dout << std::flush;

    stack<double> val_stack;
    token tok;
    vector<token>::const_iterator viter;
    for (viter = suffix_code.begin(); viter != suffix_code.end(); ++viter)
    {
        if (viter->type == KEY)
        {
            val_stack.push(vals[viter->val]);
        }
        else if (viter->type == CON)
        {
            val_stack.push(atof(viter->val.c_str()));
        }
        else // Operator, Calculate.
        {
            double lhs;
            double rhs;
            // Get result.
            if (val_stack.size() < 2)
                throw bad_param(__FILE__, __LINE__, 87, expr);
            lhs = val_stack.top();
            val_stack.pop();
            rhs = val_stack.top();
            val_stack.pop();
            // Calculate.
            if (viter->val == "+")
                val_stack.push(lhs + rhs);
            else if (viter->val == "-")
                val_stack.push(lhs - rhs);
            else if (viter->val == "*")
                val_stack.push(lhs * rhs);
            else if (viter->val == "/")
                val_stack.push(lhs / rhs);
            else if (viter->val == "%")
                val_stack.push(static_cast<int>(lhs) % static_cast<int>(rhs));
            else
                throw bad_param(__FILE__, __LINE__, 88, expr);
        }
    }
    if (val_stack.size() != 1)
        throw bad_param(__FILE__, __LINE__, 87, expr);
    return val_stack.top();
}

void calculator::get_token(token& tok)
{
    while (iter != expr.end() && *iter == ' ')
        ++iter;
    if (iter == expr.end())
    {
        tok.type = END;
        tok.pri = INIT;
        tok.val = "";
    }
    else if (*iter == '$')
    {
        tok.type = KEY;
        tok.pri = INIT;
        ++iter;
        tok.val = "";
        for (; iter != expr.end() && isalnum(*iter); ++iter)
            tok.val += *iter;
    }
    else if (*iter == '+' || *iter == '-')
    {
        tok.type = OP;
        tok.pri = PM;
        tok.val = *iter;
        ++iter;
    }
    else if (*iter == '*' || *iter == '/' || *iter == '%')
    {
        tok.type = OP;
        tok.pri = MDM;
        tok.val = *iter;
        ++iter;
    }
    else if (*iter == '(')
    {
        tok.type = OP;
        tok.pri = LB;
        tok.val = *iter;
        ++iter;
    }
    else if (*iter == ')')
    {
        tok.type = OP;
        tok.pri = RB;
        tok.val = *iter;
        ++iter;
    }
    else if (isdigit(*iter))
    {
        tok.type = CON;
        tok.pri = INIT;
        tok.val = "";
        for (; isdigit(*iter) || *iter == '.'; ++iter)
            tok.val += *iter;
    }
    else
    {
        throw bad_param(__FILE__, __LINE__, 88, expr);
    }
}

}

