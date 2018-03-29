#ifndef __FTP_H__
#define __FTP_H__

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
#include "dstream.h"
#include "user_exception.h"
#include "user_assert.h"

namespace util
{

using std::string;
using std::ostringstream;

// access() type codes
int const FTPLIB_DIR = 1;
int const FTPLIB_DIR_VERBOSE = 2;
int const FTPLIB_FILE_READ = 3;
int const FTPLIB_FILE_WRITE = 4;

// access() mode codes
int const FTPLIB_ASCII = 'A';
int const FTPLIB_IMAGE = 'I';
int const FTPLIB_TEXT = FTPLIB_ASCII;
int const FTPLIB_BINARY = FTPLIB_IMAGE;

// connection modes
int const FTPLIB_PASSIVE = 1;
int const FTPLIB_PORT = 2;

// connection option names
int const FTPLIB_CONNMODE = 1;
int const FTPLIB_CALLBACK = 2;
int const FTPLIB_IDLETIME = 3;
int const FTPLIB_CALLBACKARG = 4;
int const FTPLIB_CALLBACKBYTES = 5;

int const FTPLIB_BUFSIZ = 8192;
int const FTPLIB_RESPONSE = 256;
int const ACCEPT_TIMEOUT = 30;

int const FTPLIB_CONTROL = 0;
int const FTPLIB_READ = 1;
int const FTPLIB_WRITE = 2;

int const FTPLIB_DEFMODE = FTPLIB_PORT; //FTPLIB_PASSIVE;

int const MAX_LINE = 128;

struct netbuf;
typedef bool (*FtpCallback) (netbuf* nControl, int xfered, void* arg);
struct netbuf
{
    char* cput;
    char* cget;
    int handle;
    int cavail;
    int cleft;
    char buf[FTPLIB_BUFSIZ];
    int dir;
    netbuf* ctrl;
    int cmode;
    timeval idletime;
    FtpCallback idlecb;
    void* idlearg;
    int xfered;
    int cbbytes;
    int xfered1;
    char response[FTPLIB_RESPONSE];
};

class ftp
{
public:
    enum ftp_status
    {
        IDLE,
        CONNECTED,
        LOGINED,
        ACCESS
    };

    ftp();
    ftp(const string& host_, const string& user_, const string& passwd_);
    virtual ~ftp();
    // Connect to ftp server.
    void connect();
    void connect(const string& host_);
    // Change ftp options.
    bool options(int opt, long val);
    // Set host ip/name.
    void set_host(const string& host_);
    // Set user name.
    void set_user(const string& user_);
    // Set passwd.
    void set_passwd(const string& passwd_);
    // Login to ftp server.
    void login();
    void login(const string& user_, const string& passwd_);
    // Execute ftp commands.
    void site(const string& cmd);
    void syst(string& buf);
    void mkd(const string& path);
    void cwd(const string& path);
    void cdup();
    void rmd(const string& path);
    void pwd(string& path);
    void nlst(const string& output, const string& path);
    void list(const string& output, const string& path);
    void list(const string& output, const string& path, const string& filename);
    void size(const string& path, int& size, char mode);
    void mdtm(const string& file_name, string& dt);
    void get(const string& local_file, const string& remote_file, char mode);
    void put(const string& local_file, const string& remote_file, char mode);
    void rename(const string& src, const string& dst);
    void dele(const string& file_name);
    // return a pointer to the last response received.
    char* response();
    // Close a data connection.
    void close();
    // Close ftp connection and exit ftp.
    void quit();
    // Close control connection.
    void close_ctrl();    
    // Build data connection.
    void access(const string& path, int typ, char mode);
    // Read from socket.
    int read(void* buf, int max);
    // Write to socket.
    bool write(void* buf, int len);
    // Reset to inital status.
    void reset();

private:
    bool socket_wait(netbuf* ctl, int mode);
    int read_line(char* buf, int max, netbuf* ctl);
    int write_line(char* buf, int len);
    bool read_resp(char c);
    void send_cmd(const string& cmd, char expresp);
    void open_port(char mode, int dir);
    void accept();
    void xfer(const string& local_file, const string& path, int typ, char mode);

    netbuf net_ctrl;
    netbuf net_data;
    string host;
    string user;
    string passwd;
    ftp_status status;
    ostringstream fmt;
};

}

#endif
