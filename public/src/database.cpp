

#include "database.h"

namespace util
{

using std::string;

/**
 * @fn util::database::database(const string& user_name_, 
 *                              const string& password_, 
 *                              const string& connect_string_, 
 *                              unsigned int min_conn, 
 *                              unsigned int max_conn, 
 *                              unsigned int incr_conn = 1)
 * @brief The constructor function.
 *
 * Creates a connection pool based on the parameters specified.
 *
 * @param[in] user_name_ The database username.
 * @param[in] password_ The database password.
 * @param[in] connect_string_ The database connect string.
 * @param[in] min_conn The minimum number of connections in the pool. The 
 * minimum number of connections are opened by this method. Additional 
 * connections are opened only when necessary. Valid values are 1 and greater.
 * @param[in] max_conn The maximum number of connections in the pool. Valid 
 * values are 1 and greater.
 * @param[in] incr_conn The increment by which to increase the number of 
 * connections to be opened if current number of connections is less than 
 * max_conn. Valid values are 1 and greater. The default value is 1.
 *
 * @see util::database::database(const string&, const string&, const string&)
 * @see util::database::database(const database&)
 */
database::database(const string& user_name_,
                   const string& password_,
                   const string& connect_string_)
    : user_name(user_name_),
      password(password_),
      connect_string(connect_string_)
{
    int ret;
    connected = false;
    try{
      otl_connect::otl_initialize(); // initialize the database API environment
      connect_info = user_name + "/" + password + "@" + connect_string;
      db.rlogon(connect_info.c_str()); // connect to the database
      connected = true;
      pthread_attr_init ( &ltAttr ) ;
      pthread_attr_setdetachstate ( &ltAttr, PTHREAD_CREATE_DETACHED ) ;
      pthread_attr_setstacksize ( &ltAttr, 1024);
      
      sched_param loSchedParam;
      loSchedParam.sched_priority = SCHED_FIFO;
      pthread_attr_setschedparam( &ltAttr, &loSchedParam );
     
      thread_info = 0;
      thread_info = new thread_info_t;
      thread_info->db = &db;
      thread_info->connected = &connected;
      ret = pthread_create(&tid,&ltAttr,heartbeat,this->thread_info);
      if(ret != 0) {
        cout << "create heartbeat thread  error, error_code is [" << ret << "]" << endl;;
        exit(1);
      }
    } catch (otl_exception& p){ // intercept OTL exceptions
      cout << p.msg << endl;
      cout << p.stm_text << endl;
      cout << p.var_info << endl;
      exit(1);
    }
//    printf ("end databse constructer!\n");
}

/**
 * @overload util::database::database(const database& the_db)
 *
 * Creates a pooled connection from the database specified.
 *
 * @param[in] the_db The database object.
 *
 * @see util::database::database(const string&, const string&, const string&, unsigned int, unsigned int, unsigned int)
 * @see util::database::database(const string&, const string&, const string&)
 */
database::database(const database& the_db)
    : user_name(the_db.user_name),
      password(the_db.password),
      connect_string(the_db.connect_string)
{
    int ret;
    connected = false;
    try{
      otl_connect::otl_initialize();                                    // initialize the database API environment
      connect_info = user_name + "/" + password + "@" + connect_string;
      db.rlogon(connect_info.c_str());                                  // connect to the database
      connected = true;
      pthread_attr_init ( &ltAttr ) ;
      pthread_attr_setdetachstate ( &ltAttr, PTHREAD_CREATE_DETACHED ) ;
      pthread_attr_setstacksize ( &ltAttr, 1024);
      
      sched_param loSchedParam;
      loSchedParam.sched_priority = SCHED_FIFO;
      pthread_attr_setschedparam( &ltAttr, &loSchedParam );      
      thread_info = 0;
      thread_info = new thread_info_t;
      thread_info->db = &db;
      thread_info->connected = &connected;
      ret = pthread_create(&tid,&ltAttr,heartbeat,this->thread_info);
      if(ret != 0) {
        cout << "create heartbeat thread  error, error_code is [" << ret << "]" << endl;;
        exit(1);
      }
    } catch (otl_exception& p){                                         // intercept OTL exceptions
      cout << p.msg << endl;
      cout << p.stm_text << endl;
      cout << p.var_info << endl;
//      std::dout<<p.msg<<std::endl;                                      // print out error message
//      std::dout<<p.stm_text<<std::endl;                                 // print out SQL that caused the error
//      std::dout<<p.var_info<<std::endl;                                 // print out the variable that caused the error
//      throw;
      exit(1);
    }
}

/**
 * @fn util::database::~database()
 * @brief The destructor function.
 *
 * Destroys a database object based on the clean flag.
 */
database::~database()
{
    connected = false;
    pthread_attr_destroy(&ltAttr);
    pthread_cancel(tid);
    disconnect();
    delete thread_info;
}

/**
 * @fn void util::database::commit()
 * @brief Commits all changes made since the previous commit or rollback, and 
 * releases any database locks currently held by the session.
 */
void database::commit()
{
#ifdef OTL_ODBC_MYSQL
  otl_cursor::direct_exec
   (
    db,
    "commit"
    );  
#else
    db.rollback();
#endif
}

/**
 * @fn void util::database::rollback()
 * @brief Drops all changes made since the previous commit or rollback, and 
 * releases any database locks currently held by the session.
 */
void database::rollback()
{
#ifdef OTL_ODBC_MYSQL
  otl_cursor::direct_exec
   (
    db,
    "rollback"
    );  
#else
    db.rollback();
#endif
}

void * heartbeat(void *thread_info)
{
  thread_info_t * info = (thread_info_t *)thread_info ;

  int tmp;
  string sql="select 1";


  while(true) {
    printf("@@@@info->connected is %d\n",*(info->connected));
    sleep (1800);
    if(*(info->connected)){
      try{
        string sql="select 1";
        otl_stream stat (1, sql.c_str(), *info->db);
        stat >> tmp;
        stat.close();
      } catch ( otl_exception &ex) {
        cout << ex.msg << endl;
        cout << ex.stm_text << endl;
        cout << ex.var_info << endl;
        exit(1);
      } 
    } else
      continue;
  }

  printf("end heart_beat\n");
  pthread_exit( NULL );
  return NULL;
}
/**
 * @fn Connection* database::get_conn() const
 * @brief Provides a pointer of the Connection object for the Oracle Call Interface (OCI).
 *
 * @return A pointer of the Connection object.
 */
otl_connect& database::get_conn()
{
    return db;
}

vector<MetaData_t> database::get_metadata(const string& table_name)
{
    MetaData_t Meta;
    
    string sql = "select table_name,columns_index,column_name,column_type,colunm_type_code,column_length from sys_table_struct \
    where table_name = '" +  table_name + "' order by table_name, columns_index";
    try
    {   
       otl_stream stat (1000, // buffer size
              sql.c_str(), // SQL statement
              db // connect object
             );
      
      while (!stat.eof()){
        stat >> Meta.table_name;
        stat >> Meta.columns_index;
        stat >> Meta.column_name;
        stat >> Meta.column_type;
        stat >> Meta.colunm_type_code;
        stat >> Meta.column_length;
        MetaData_.push_back(Meta);
      }

    }
    catch (otl_exception& p)
    {
//      std::dout<<p.msg<<std::endl;        // print out error message
//      std::dout<<p.stm_text<<std::endl;   // print out SQL that caused the error
//      std::dout<<p.var_info<<std::endl;   // print out the variable that caused the error
      throw;
    }
    return MetaData_;        
}

vector<MetaData_t>  database::get_metadata(const string& table_name ,int &column_num)
{
    MetaData_t Meta;
    otl_stream select_t;
    int desc_len = 0;

    
    string sql = "select * from " +  table_name;
    try
    {    
        select_t.open(100,sql.c_str(),db);

        // 根据查询的字段类型，用不同的数据类型获取数据

        otl_column_desc* desc; 
        desc=select_t.describe_select(desc_len);
        column_num = desc_len;

        while(!select_t.eof())
        {
            for(int i = 0;i < desc_len; i++)
            { 
                Meta.table_name = table_name;
                Meta.columns_index = i+1;
                Meta.column_name = desc[i].name;
                Meta.column_type = desc[i].dbtype;
                Meta.colunm_type_code = desc[i].otl_var_dbtype;
                Meta.column_length = desc[i].dbsize;
                MetaData_.push_back(Meta);
            }
        }
    }
    catch (otl_exception& p)
    {
//      std::dout<<p.msg<<std::endl;        // print out error message
//      std::dout<<p.stm_text<<std::endl;   // print out SQL that caused the error
//      std::dout<<p.var_info<<std::endl;   // print out the variable that caused the error
      throw;
    }
    return MetaData_;        
}

/**
 * @fn void util::database::connect()
 * @brief Establishes a connection to the database specified.
 */
void database::connect()
{
    if (!connected)
    {
      otl_connect::otl_initialize();     // initialize the database API environment
      db.rlogon(connect_info.c_str()); // connect to the database
      connected = true;
    }
}

/**
 * @fn void util::database::disconnect()
 * @brief Terminates the connection, and free all related system resources.
 */
void database::disconnect()
{
    if (connected)
    {
//        rollback();
        db.logoff(); // connect to the database
        connected = false;
    }
}

}

