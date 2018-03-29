#ifndef __sftp_H__
#define __sftp_H__
#include <ios>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cctype>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <libssh2.h>
#include <libssh2_sftp.h>
#include "dstream.h"
#include "user_exception.h"
#include "user_assert.h"

namespace util
{

using std::string;
using std::ostringstream;
using std::ios_base;
// access() type codes
int const sftpLIB_DIR = 1;
int const sftpLIB_DIR_VERBOSE = 2;
int const sftpLIB_FILE_READ = 3;
int const sftpLIB_FILE_WRITE = 4;

// access() mode codes
int const sftpLIB_ASCII = 'A';
int const sftpLIB_IMAGE = 'I';
int const sftpLIB_TEXT = sftpLIB_ASCII;
int const sftpLIB_BINARY = sftpLIB_IMAGE;

// connection modes
int const sftpLIB_PASSIVE = 1;
int const sftpLIB_PORT = 2;

// connection option names
int const sftpLIB_CONNMODE = 1;
int const sftpLIB_CALLBACK = 2;
int const sftpLIB_IDLETIME = 3;
int const sftpLIB_CALLBACKARG = 4;
int const sftpLIB_CALLBACKBYTES = 5;

int const sftpLIB_BUFSIZ = 8192;
int const sftpLIB_RESPONSE = 256;
int const ACCEPT_TIMEOUT = 30;

int const sftpLIB_CONTROL = 0;
int const sftpLIB_READ = 1;
int const sftpLIB_WRITE = 2;

int const sftpLIB_DEFMODE = sftpLIB_PORT; //sftpLIB_PASSIVE;

int const MAX_LINE = 128;

struct netbuf;
typedef bool (*sftpCallback) (netbuf* nControl, int xfered, void* arg);
struct netbuf
{
    char* cput;
    char* cget;
    int handle;
    int cavail;
    int cleft;
    char buf[sftpLIB_BUFSIZ];
    int dir;
    netbuf* ctrl;
    int cmode;
    timeval idletime;
    sftpCallback idlecb;
    void* idlearg;
    int xfered;
    int cbbytes;
    int xfered1;
    char response[sftpLIB_RESPONSE];
};

class sftp
{
public:
    enum sftp_status
    {
        IDLE,
        CONNECTED,
        LOGINED,
        ACCESS
    };
sftp();	
sftp(const string& host_, const string& user_, const string& passwd_);
virtual bool openSession(const string&,const string&, const string&, const int& port);
virtual void closeSession(void);
    void disconnect();
    virtual ~sftp();
    // Connect to sftp server.
    bool connectTo(const string& ,const string& ,const string& , const int& port);
    // Change sftp options.
    bool options(int opt, long val);
    // Set host ip/name.
    void set_host(const string& host_);
    // Set user name.
    void set_user(const string& user_);
    // Set passwd.
    void set_passwd(const string& passwd_);
    // Execute sftp commands.
    void site(const string& cmd);
    void syst(string& buf);
    void mkd(const string& path);
    void cwd(const string& path);
    void cdup();
    void rmd(const string& path);
    void pwd(string& path);
  //  void nlst(const string& output, const string& path);
 void  list(const string& path,const string& pattern, map<string,time_t>& ret,int ls_num);
   // void list(const string& output, const string& path, const string& filename);
    void size(const string& path, int& size, char mode);
    void mdtm(const string& file_name, string& dt);
    void get(const string& local_file, const string& remote_file, char mode);
    void put(const string& local_file, const string& remote_file, char mode);
    time_t getmtime(const string& path, const string& filename) const;
    void* open(const string& fullName, const unsigned int flag = LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT|LIBSSH2_FXF_TRUNC) const;
	void* opendir(const string& path, const unsigned int mode) const ;
	 void rm(const string&) const;
    void dele(const string& file_name);
    // return a pointer to the last response received.
    char* response();
    // Close a data connection.
    void close();
     void close(const void* handle) const ;
    // Close sftp connection and exit sftp.
    void quit();
    // Close control connection.
    void close_ctrl();    
    // Build data connection.
    void access(const string& path, int typ, char mode);
    // Read from socket.
    //int read(void* buf, int max);
	long read(const void* handle, char* buf, const unsigned int max) ;
    // Write to socket.
     bool write(const void* handle, const char* buff, const unsigned int size)  ;
    // Reset to inital status.
    void reset();
bool rename(const string&, const string&) const;
private:
   
    bool socket_wait(netbuf* ctl, int mode);
    int read_line(char* buf, int max, netbuf* ctl);
    int write_line(char* buf, int len);
    bool read_resp(char c);
    void send_cmd(const string& cmd, char expresp);
    void open_port(char mode, int dir);
    void accept();
   // void xfer(const string& local_file, const string& path, int typ, char mode);
//std::streampos seek(int off, ios_base::seekdir way, ios_base::openmode which = ios_base::in | ios_base::out);

/*void kbd_callback(const char *name, int name_len, \
	const char *instruction, int instruction_len, int num_prompts, \
	const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts, \
	LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses,  \
	void **abstract);*/
    netbuf net_ctrl;
    netbuf net_data;
    string host;
    string user;
    string passwd;
    sftp_status status;
    ostringstream fmt;

	int sock_fd;
	string path_;
	//libssh2_socket_t sock;
	LIBSSH2_SESSION *session;
	LIBSSH2_SFTP *sftp_session;
	bool connected;
	 static const unsigned int READ;
        static const unsigned int WRITE;		
        static const unsigned int CREATE;

        static const unsigned int FILTER_FILE;
        static const unsigned int FILTER_PATH;
        static const unsigned int FILTER_LONG;
};

}

#endif
