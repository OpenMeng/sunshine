
#include <sstream>
#include "user_exception.h"
#include <string.h>


namespace util
{
using namespace std;
using std::string;
using std::ostringstream;
using std::exception;

const char* bad_file::messages[] =
{
    "bad_stat",
    "bad_lstat",
    "bad_statfs",
    "bad_sock",
    "bad_open",
    "bad_seek",
    "bad_read",
    "bad_write",
    "bad_flush",
    "bad_close",
    "bad_remove",
    "bad_rename",
    "bad_opendir",
    "bad_ioctl",
    "bad_popen"
};

/**
 * @fn util::bad_file::bad_file(const string& file, 
 *                              int line, 
 *                              int error_code_, 
 *                              const string& obj = "") throw()
 * @brief The constructor function.
 *
 * Creates a bad_file object. To avoid the nested exception, it cannot throw a 
 * new exception.
 *
 * @param[in] file The filename which throws an exception.
 * @param[in] line The line number of the file which throws an exception.
 * @param[in] error_code_ The error code of the exception. And the error code is
 * one of the util::bad_file::bad_type which is an enumeration type.
 * @param[in] obj The object of the exception, and the default value is null.
 */
bad_file::bad_file(const string& file, int line, int sys_code_, int error_code_, const string& obj) throw()
{
    sys_code = sys_code_;
    error_code = error_code_;
    ostringstream fmt;
    fmt << "bad_file : In file(" << file
        << ") at line(" << line
        << ") error code(" << error_code
        << ") object(" << obj << ").";
    fmt << "\nsys_code (" << sys_code << "), use aerr for more detail.";
    what_ = fmt.str();
}

/**
 * @fn util::bad_file::~bad_file() throw()
 * @brief The destructor function.
 *
 * Destroys a bad_file object. To avoid the nested exception, it cannot throw a 
 * new exception.
 */
bad_file::~bad_file() throw()
{
}

/**
 * @fn const char* util::bad_file::error_msg() const throw()
 * @brief Returns the error content associated with the error code.
 *
 * To avoid the nested exception, it cannot throw a new exception.
 */
const char* bad_file::error_msg() const throw()
{
    return messages[error_code];
}

/**
 * @fn const char* util::bad_file::what() const throw()
 * @brief Returns the error message associated with the exception.
 *
 * To avoid the nested exception, it cannot throw a new exception.
 */
const char* bad_file::what () const throw()
{
    return what_.c_str();
}

const char* bad_system::messages[] =
{
    "bad_cmd",
    "bad_dlopen",
    "bad_dlsym",
    "bad_dlclose",
    "bad_reg",
    "bad_sem",
    "bad_shm",
    "bad_msg",
    "bad_pstat",
    "bad_alloc",
    "bad_select",
    "bad_fork",
    "bad_exec",
    "bad_other"
};

/**
 * @fn util::bad_system::bad_system(const string& file, 
 *                                  int line, 
 *                                  int error_code_, 
 *                                  const string& cause = "") throw()
 * @brief The constructor function.
 *
 * Creates a bad_system object. To avoid the nested exception, it cannot throw a 
 * new exception.
 *
 * @param[in] file The filename which throws an exception.
 * @param[in] line The line number of the file which throws an exception.
 * @param[in] error_code_ The error code of the exception. And the error code is
 * one of the util::bad_system::bad_type which is an enumeration type.
 * @param[in] cause The cause of the exception, and the default value is null.
 */
bad_system::bad_system(const string& file, int line, int sys_code_, int error_code_, const string& cause) throw()
{
    sys_code = sys_code_;
    error_code = error_code_;
    ostringstream fmt;
    fmt << "bad_system : In file(" << file
        << ") at line(" << line
        << ") error code(" << error_code;
    if (cause.empty())
        fmt << ").";
    else
        fmt << ") cause(" << cause << ").";
    fmt << "\nsys_code (" << sys_code << "), use aerr for more detail.";
    what_ = fmt.str();
}

/**
 * @fn util::bad_system::~bad_system() throw()
 * @brief The destructor function.
 *
 * Destroys a bad_system object. To avoid the nested exception, it cannot throw 
 * a new exception.
 */
bad_system::~bad_system() throw()
{
}

/**
 * @fn const char* util::bad_system::error_msg() const throw()
 * @brief Returns the error content associated with the error code.
 *
 * To avoid the nested exception, it cannot throw a new exception.
 */
const char* bad_system::error_msg() const throw()
{
    return messages[error_code];
}

/**
 * @fn const char* util::bad_system::what() const throw()
 * @brief Returns the error message associated with the exception.
 *
 * To avoid the nested exception, it cannot throw a new exception.
 */
const char* bad_system::what () const throw()
{
    return what_.c_str();
}


/**
 * @fn util::bad_db::bad_db(const string& file, 
 *                          int line, 
 *                          const SQLException& ex, 
 *                          const string& message = "") throw()
 * @brief The constructor function.
 *
 * Creates a bad_db object. To avoid the nested exception, it cannot throw a new 
 * exception.
 *
 * @param[in] file The filename which throws an exception.
 * @param[in] line The line number of the file which throws an exception.
 * @param[in] ex The SQLException object.
 * @param[in] message The error message of the exception, and the default value 
 * is null.
 */
bad_db::bad_db(const string& file, int line, int sys_code_,  otl_exception& p, const string& message) throw()
    : otl_ex(p)
{
    sys_code = sys_code_;
    ostringstream fmt;
    fmt << "bad_db : In file(" << file
        << ") at line(" << line
        << ") msg:[" << otl_ex.msg 
        << "] var_info:["<< otl_ex.var_info 
        << "] stm_text [" << otl_ex.stm_text
        << "] " << message;
    fmt << "\nsys_code (" << sys_code << "), use aerr for more detail.";
    what_ = fmt.str();
}

/**
 * @fn util::bad_db::~bad_db() throw()
 * @brief The destructor function.
 *
 * Destroys a bad_db object. To avoid the nested exception, it cannot throw a 
 * new exception.
 */
bad_db::~bad_db() throw()
{
}

/**
 * @fn util::bad_db::what() const throw()
 * @brief Returns the error message associated with the SQLException.
 *
 * To avoid the nested exception, it cannot throw a new exception.
 */
const char* bad_db::what () const throw()
{
    return what_.c_str();
}


bad_ab::bad_ab(const string& file, int line, int error_code, const string& message) throw()
{
    ostringstream fmt;
    fmt << "bad_ab : In file(" << file
        << ") at line(" << line
        << ") error code : " << error_code
        << ") error message : " << message;
    error_code_ = error_code;
    what_ = fmt.str();
}

/**
 * @fn util::bad_ab::~bad_ab() throw()
 * @brief The destructor function.
 *
 * Destroys a bad_ab object. To avoid the nested exception, it cannot throw a 
 * new exception.
 */
bad_ab::~bad_ab() throw()
{
}

/**
 * @fn int util::bad_ab::get_error_code() const throw()
 * @brief Returns the error code.
 *
 * To avoid the nested exception, it cannot throw a new exception.
 */
int bad_ab::get_error_code() const throw()
{
    return error_code_;
}

/**
 * @fn const char* util::bad_ab::error_msg() const throw()
 * @brief Returns the error content associated with the error code.
 *
 * To avoid the nested exception, it cannot throw a new exception.
 */
const char* bad_ab::error_msg() const throw()
{
    return what_.c_str();
}

/**
 * @fn const char* util::bad_ab::what() const throw()
 * @brief Returns the error message associated with the exception.
 *
 * To avoid the nested exception, it cannot throw a new exception.
 */
const char* bad_ab::what () const throw()
{
    return what_.c_str();
}

/**
 * @fn util::bad_auth::bad_auth(const string& file, int line) throw()
 * @brief The constructor function.
 *
 * Creates a bad_auth object. To avoid the nested exception, it cannot throw a 
 * new exception.
 *
 * @param[in] file The filename which throws an exception.
 * @param[in] line The line number of the file which throws an exception.
 */
bad_auth::bad_auth(const string& file, int line, int sys_code_) throw()
{
    sys_code = sys_code_;
    ostringstream fmt;
    fmt << "bad_auth : In file(" << file
        << ") at line(" << line
        << ").";
    fmt << "\nsys_code (" << sys_code << "), use aerr for more detail.";
    what_ = fmt.str();
}

/**
 * @fn util::bad_auth::~bad_auth() throw()
 * @brief The destructor function.
 *
 * Destroys a bad_auth object. To avoid the nested exception, it cannot throw a 
 * new exception.
 */
bad_auth::~bad_auth() throw()
{
}

/**
 * @fn const char* util::bad_auth::what() const throw()
 * @brief Returns the error message associated with the exception.
 *
 * To avoid the nested exception, it cannot throw a new exception.
 */
const char* bad_auth::what () const throw()
{
    return what_.c_str();
}

/**
 * @fn util::bad_param::bad_param(const string& file, 
 *                                int line, 
 *                                const string& cause) throw()
 * @brief The constructor function.
 *
 * Creates a bad_param object. To avoid the nested exception, it cannot throw a 
 * new exception.
 *
 * @param[in] file The filename which throws an exception.
 * @param[in] line The line number of the file which throws an exception.
 * @param[in] cause The cause of the exception.
 */
bad_param::bad_param(const string& file, int line, int sys_code_, const string& cause) throw()
{
    sys_code = sys_code_;
    ostringstream fmt;
    fmt << "bad_param : In file(" << file
        << ") at line(" << line
        << ")." << cause << ".";
    fmt << "\nsys_code (" << sys_code << "), use aerr for more detail.";
    what_ = fmt.str();
}

/**
 * @fn util::bad_param::~bad_param() throw()
 * @brief The destructor function.
 *
 * Destroys a bad_param object. To avoid the nested exception, it cannot throw a 
 * new exception.
 */
bad_param::~bad_param() throw()
{
}

/**
 * @fn const char* util::bad_param::what() const throw()
 * @brief Returns the error message associated with the exception.
 *
 * To avoid the nested exception, it cannot throw a new exception.
 */
const char* bad_param::what () const throw()
{
    return what_.c_str();
}

const char* bad_datetime::messages[] =
{
    "bad_year",
    "bad_month",
    "bad_day",
    "bad_hour",
    "bad_minute",
    "bad_second",
    "bad_other"
};

/**
 * @fn util::bad_datetime::bad_datetime(const string& file, 
 *                                      int line, 
 *                                      int error_code_, 
 *                                      const string& obj = "") throw()
 * @brief The constructor function.
 *
 * Creates a bad_datetime object. To avoid the nested exception, it cannot throw 
 * a new exception.
 *
 * @param[in] file The filename which throws an exception.
 * @param[in] line The line number of the file which throws an exception.
 * @param[in] error_code_ The error code of the exception. And the error code is
 * one of the util::bad_datetime::bad_type which is an enumeration type.
 * @param[in] obj The object of the exception, and the default value is null.
 */
bad_datetime::bad_datetime(const string& file, int line, int sys_code_, int error_code_, const string& obj) throw()
{
    sys_code = sys_code_;
    error_code = error_code_;
    ostringstream fmt;
    fmt << "bad_datetime : In file(" << file
        << ") at line(" << line
        << ") error code(" << error_code
        << ") object(" << obj << ").";
    fmt << "\nsys_code (" << sys_code << "), use aerr for more detail.";
    what_ = fmt.str();
}

/**
 * @fn util::bad_datetime::~bad_datetime() throw()
 * @brief The destructor function.
 *
 * Destroys a bad_datetime object. To avoid the nested exception, it cannot 
 * throw a new exception.
 */
bad_datetime::~bad_datetime() throw()
{
}

/**
 * @fn const char* util::bad_datetime::error_msg() const throw()
 * @brief Returns the error content associated with the error code.
 *
 * To avoid the nested exception, it cannot throw a new exception.
 */
const char* bad_datetime::error_msg() const throw()
{
    return messages[error_code];
}

/**
 * @fn const char* util::bad_datetime::what() const throw()
 * @brief Returns the error message associated with the exception.
 *
 * To avoid the nested exception, it cannot throw a new exception.
 */
const char* bad_datetime::what () const throw()
{
    return what_.c_str();
}

/**
 * @fn util::bad_msg::bad_msg(const string& file, 
 *                            int line, 
 *                            const string& msg) throw()
 * @brief The constructor function.
 *
 * Creates a bad_msg object. To avoid the nested exception, it cannot throw a 
 * new exception.
 *
 * @param[in] file The filename which throws an exception.
 * @param[in] line The line number of the file which throws an exception.
 * @param[in] msg The error message of the exception.
 */
bad_msg::bad_msg(const string& file, int line, int sys_code_, const string& msg) throw()
{
    sys_code = sys_code_;
    ostringstream fmt;
    fmt << "bad_msg : In file(" << file
        << ") at line(" << line
        << ") message(" << msg << ").";
    fmt << "\nsys_code (" << sys_code << "), use aerr for more detail.";
    what_ = fmt.str();
}

/**
 * @fn util::bad_msg::~bad_msg() throw()
 * @brief The destructor function.
 *
 * Destroys a bad_msg object. To avoid the nested exception, it cannot throw a
 * new exception.
 */
bad_msg::~bad_msg() throw()
{
}

/**
 * @fn const char* util::bad_msg::what() const throw()
 * @brief Returns the error message associated with the exception.
 *
 * To avoid the nested exception, it cannot throw a new exception.
 */
const char* bad_msg::what () const throw()
{
    return what_.c_str();
}

}

