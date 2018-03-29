
#if !defined(__DATABASE_H__)
#define __DATABASE_H__

#ifndef OTL_ODBC_MYSQL 
#define OTL_ODBC_MYSQL
#endif

// The following #define is required with MyODBC 3.51.11 and higher 
#ifndef OTL_ODBC_SELECT_STM_EXECUTE_BEFORE_DESCRIBE 
#define OTL_ODBC_SELECT_STM_EXECUTE_BEFORE_DESCRIBE
#endif   
#ifndef OTL_ODBC_UNIX 
#define OTL_ODBC_UNIX
#endif 

#ifndef OTL_STL 
#define OTL_STL
#endif   


#include "otlv4.h"
#include <string>
#include <vector>
#include <iostream>
#include <pthread.h>

#include "user_assert.h"



namespace util
{

using std::string;
using std::vector;
using std::endl;
using std::cout;

struct MetaData_t
{
  string table_name;
  int columns_index;
  string column_name;
  string column_type;
  int colunm_type_code;
  int column_length;
};

struct thread_info_t
{
  otl_connect *db;
  bool*  connected ;
};

//extern bool connected;    ///<The connection status.

void* heartbeat(void *db) ;

class database
{
public:
  
  
    database(const string& user_name_,
             const string& password_,
             const string& connect_string_ = "");
    database(const database& the_db);
    virtual ~database();
    void commit();
    void rollback();
    void connect();
    void disconnect();
    vector<MetaData_t> get_metadata(const string& table_name);
    vector<MetaData_t> get_metadata(const string& table_name , int& column_num);
    otl_connect &get_conn();
    otl_connect db;
private:
    string user_name;    ///<The database username.
    string password;    ///<The database password.
    string connect_string;    ///<The database connect string.
    string connect_info;

    pthread_t tid ;
    pthread_attr_t ltAttr ;
    thread_info_t *thread_info;
    //thread_info_t thread_info;

    vector<MetaData_t> MetaData_;

    bool connected;    ///<The connection status.
};

}

#endif
