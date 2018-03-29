
#include "sftp.h"
#include <regex.h>
#include <sys/types.h>
#include "common.h"
namespace util
{

using std::ostringstream;
using util::bad_file;
using std::cout;
using std::clog;
using namespace util;

const unsigned int sftp::READ(0x00000001);
const unsigned int sftp::WRITE(0x00000002);
const unsigned int sftp::CREATE(0x00000008);

const unsigned int sftp::FILTER_FILE(0x00000001);
const unsigned int sftp::FILTER_PATH(0x00000002);
const unsigned int sftp::FILTER_LONG(0x00000004);
sftp::sftp()
{
    sock_fd = -1;
session = NULL;
sftp_session = NULL;
connected = false;  
}
sftp::sftp(const string& host_, const string& user_, const string& passwd_)
{
    host = host_;
    user = user_;
    passwd = passwd_;
    sock_fd = -1;
session = NULL;
sftp_session = NULL;
connected = false;  
}

sftp::~sftp()
{
 	this->closeSession();
}
void sftp::disconnect()
{
  if(session){
    libssh2_session_disconnect(session,"Normal Shutdown, Thank you for playing");
    libssh2_session_free(session);
  	}
	::close(sock_fd);
	connected = false;
	libssh2_exit();
}

bool sftp::openSession(const string& host_,const string& userName, const string& password, const int& port)
{
    if (this->connected)
        return true;
    if (!this->connectTo(host_,userName, password, port))
        return false;
    this->sftp_session = libssh2_sftp_init(this->session);
    if (!this->sftp_session) {
        this->disconnect();
        return false;
    }
    return true;
}

void sftp::closeSession(void) {
    if (this->connected) {
        libssh2_sftp_shutdown(this->sftp_session);
		this->sftp_session = NULL;
        this->disconnect();
    }
}

bool sftp::connectTo(const string& host_,const string& uname,const string& password, const int& port)
{
    dout<<"["<<__FILE__<<"]["<<__LINE__<<"] connectTo() host_ = ["<<host_<<"] uname = ["<<uname<<"] password = ["<<password<<"] port = ["<<port<<"]"<<std::endl;

    int rc;
    sockaddr_in sin;
    host = host_;
	user = uname;
	passwd = password;
    memset(&sin, 0, sizeof(sin));
     rc = libssh2_init (0);
    if (rc != 0) {
        bad_file(__FILE__, __LINE__, 172, bad_file::bad_sock, strerror(errno));
	 return false;
    }
 
    unsigned long hostaddr;	
    hostaddr = inet_addr(host_.c_str());
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = hostaddr;
    sock_fd = socket(sin.sin_family, SOCK_STREAM, 0);
    dout<<"In connect,sock_fd = ["<<sock_fd<<"]"<<std::endl;
    if (sock_fd == -1){
         bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
		  return false;
    	}
    
    if (::connect(sock_fd, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0) {
         bad_file(__FILE__, __LINE__, 174, bad_file::bad_sock, strerror(errno));
		  return false;
    	}
	
	
	session = libssh2_session_init();
	if (session == NULL)
	 {
        ::close(sock_fd);
         bad_file(__FILE__, __LINE__, 300, bad_file::bad_sock, strerror(errno));
		  return false;
    }
	rc = libssh2_session_startup(session, sock_fd);   
	if(rc) {      
		::close(sock_fd);
	     bad_file(__FILE__, __LINE__, 301, bad_file::bad_sock, strerror(errno));
		 return false;
		}
	libssh2_session_set_blocking(session, 1);	//阻塞
	
	if (libssh2_userauth_password(session, user.c_str(), passwd.c_str())) {
			dout<<"Authentication by password failed"<<endl;   
			::close(sock_fd);
		 bad_file(__FILE__, __LINE__, 302, bad_file::bad_sock, strerror(errno));  
		  return false;
			}    

	sftp_session = libssh2_sftp_init(session);
	if (sftp_session == NULL){
		::close(sock_fd);
		 bad_file(__FILE__, __LINE__, 306, bad_file::bad_sock, strerror(errno));
		  return false;
		}
	connected = true;
	cout<<"CONNECTED:"<<sock_fd<<endl;
	 return true;
}

bool sftp::options(int opt, long val)
{
    int v;
    bool ret = false;
    switch (opt)
    {
    case sftpLIB_CONNMODE:
        v = static_cast<int>(val);
        if ((v == sftpLIB_PASSIVE) || (v == sftpLIB_PORT))
        {
            ret = true;
            net_ctrl.cmode = v;
        }
        break;
    case sftpLIB_CALLBACK:
        ret = true;
        net_ctrl.idlecb = reinterpret_cast<sftpCallback>(val);
        break;
    case sftpLIB_IDLETIME:
        v = static_cast<int>(val);
        ret = true;
        net_ctrl.idletime.tv_sec = 600;
        net_ctrl.idletime.tv_usec = 0;
        break;
    case sftpLIB_CALLBACKARG:
        ret = true;
        net_ctrl.idlearg = reinterpret_cast<void*>(val);
        break;
    case sftpLIB_CALLBACKBYTES:
        ret = true;
        net_ctrl.cbbytes = static_cast<int>(val);
        break;
    }
    return ret;
}

void sftp::set_host(const string& host_)
{
    host = host_;
}

void sftp::set_user(const string& user_)
{
    user = user_;
}

void sftp::set_passwd(const string& passwd_)
{
    passwd = passwd_;
}

/*
std::streampos sftp::seek(int off, ios_base::seekdir way, ios_base::openmode which)
	{
		libssh2_uint64_t offset = off;

		switch (way) {
		case ios_base::beg:
			break;
		case ios_base::cur:
			offset += libssh2_sftp_tell64(sftp_handle);
			break;
		case ios_base::end:
			{
				LIBSSH2_SFTP_ATTRIBUTES attrs;
				int retval = libssh2_sftp_stat(sftp_session, path_.c_str(), &attrs);
				if (retval)
					throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));

				offset += attrs.filesize;
			}
			break;
		default:
			break;
		}

		libssh2_sftp_seek64(sftp_handle, offset);
		return offset;
	}
	*/
void sftp::site(const string& cmd)
{
    string cmd_tmp = "SITE ";
    cmd_tmp += cmd;
    send_cmd(cmd_tmp, '2');
}

void sftp::syst(string& buf)
{
    char* s;
    send_cmd("SYST", '2');
    s = net_ctrl.response + 4;
    buf = "";
    while (*s != ' ')
        buf += *s++;
}

void sftp::mkd(const string& path)
{
    string cmd = "MKD ";
    cmd += path;
    send_cmd(cmd, '2');
}
time_t sftp::getmtime(const string& path, const string& filename) const {
    if (this->connected) {
        LIBSSH2_SFTP_HANDLE* sftpHandle = libssh2_sftp_opendir(this->sftp_session, path.c_str());
        if (sftpHandle) {
            char buff[512];
            char longentry[512];
            LIBSSH2_SFTP_ATTRIBUTES attrs;
           while(libssh2_sftp_readdir_ex(sftpHandle, buff, sizeof(buff), longentry, sizeof(longentry), &attrs) > 0){
	              if (longentry[0] != '\0' && longentry[0] != 'd' && (strncmp(filename.c_str(),buff,filename.size()) == 0)){
				libssh2_sftp_closedir(sftpHandle);
				  return attrs.mtime;
	              }
		}
            libssh2_sftp_closedir(sftpHandle);
        }
    }
    return  time(NULL);
}

void sftp::cwd(const string& path)
{
    string cmd = "CWD ";
    cmd += path;
    send_cmd(cmd, '2');
}

void sftp::cdup()
{
    send_cmd("CDUP", '2');
}

void sftp::rmd(const string& path)
{
    string cmd = "RMD ";
    cmd += path;
    send_cmd(cmd, '2');
}

void sftp::pwd(string& path)
{
    char* s;
    send_cmd("PWD", '2');
    s = strchr(net_ctrl.response, '\"');
    if (s == 0)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "PWD");
    s++;
    path = "";
    while (*s && (*s != '"'))
        path += *s++;
}

/*
void sftp::nlst(const string& outputfile, const string& path)
{
    xfer(outputfile, path, sftpLIB_DIR, sftpLIB_ASCII);
}
*/
void* sftp::opendir(const string& path, const unsigned int mode) const 
{

    //return libssh2_sftp_open((LIBSSH2_SFTP*)this->sftpSession, fullName.c_str(), flag, ~mask);
	return libssh2_sftp_open_ex(this->sftp_session,path.c_str(), path.size(),LIBSSH2_FXF_READ,
						LIBSSH2_SFTP_S_IRWXU|LIBSSH2_SFTP_S_IRWXG|LIBSSH2_SFTP_S_IRWXO
						, mode );
}
void* sftp::open(const string& fullName, const unsigned int flag) const 
{
    //return libssh2_sftp_open((LIBSSH2_SFTP*)this->sftpSession, fullName.c_str(), flag, ~mask);
	return libssh2_sftp_open_ex(this->sftp_session,fullName.c_str(), fullName.size(),flag,
								LIBSSH2_SFTP_S_IRWXU|LIBSSH2_SFTP_S_IRWXG|LIBSSH2_SFTP_S_IRWXO
								, LIBSSH2_SFTP_OPENFILE );
}
void sftp::list( const string& path,const string& pattern,map<string,long>& ret,int ls_num)
{
    //cwd(path);
    //vector<string> result;
   // map<string,time_t> result;
    LIBSSH2_SFTP_HANDLE* sftp_handle = NULL;
    int tmp_num = 0;
    if(connected)
    sftp_handle  = libssh2_sftp_opendir(sftp_session,path.c_str());
if(!sftp_handle){
	throw bad_file(__FILE__, __LINE__, 40, bad_file::bad_open, path);
}
 //   LIBSSH2_SFTP_HANDLE* sftp_whandle = (LIBSSH2_SFTP_HANDLE*)this->open(path+outputfile,LIBSSH2_FXF_APPEND);
       
		char mem[1024];     
		char longentry[1024];  
		LIBSSH2_SFTP_ATTRIBUTES attrs;  
		 // Get files.
	    regex_t reg;
	    if (regcomp(&reg, pattern.c_str(), REG_NOSUB | REG_EXTENDED))
	        throw bad_system(__FILE__, __LINE__, 179, bad_system::bad_reg, pattern);
	
		/* loop until we fail */     
		int rc;
		while( (rc = libssh2_sftp_readdir_ex(sftp_handle, mem, sizeof(mem),   
			longentry, sizeof(longentry), &attrs)) > 0) {          
			/* rc is the length of the file name in the mem buffer */        
			if (longentry[0] != '\0' && longentry[0] != 'd') //排除目录
			{       
				
				vector<string> tokens;
           		      common::string_to_array((const string&)longentry, ' ', tokens);
			   // dout<<tokens.rbegin()->c_str()<<"--"<<pattern<<endl;
				if (regexec(&reg, tokens.rbegin()->c_str(), (size_t)0, 0, 0) == 0) {
					
					//result.push_back(longentry);
					ret[longentry]= attrs.mtime;
					if(++tmp_num == ls_num)
						break;
				}
				//this->write(sftp_whandle,longentry,rc);
			} 
		}
	
	libssh2_sftp_closedir(sftp_handle);
	//return result;
    //xfer(outputfile, path, sftpLIB_DIR_VERBOSE, sftpLIB_ASCII);
}

void sftp::close(const void* handle) const {
     libssh2_sftp_close((LIBSSH2_SFTP_HANDLE*)handle);
	handle = NULL;
}


void sftp::size(const string& path, int& size, char mode)
{
    ostringstream fmt;
    string cmd;
    int resp;

    fmt << "TYPE " << mode;
    send_cmd(fmt.str(), '2');
    cmd = "SIZE " + path;
    send_cmd(cmd, '2');
    if (sscanf(net_ctrl.response, "%d %d", &resp, &size) != 2)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "SIZE");
}

void sftp::mdtm(const string& file_name, string& dt)
{
    string cmd = "MDTM ";
    cmd += file_name;
    send_cmd(cmd, '2');
    dt.assign(net_ctrl.response + 4, net_ctrl.response + 18);
}

void sftp::get(const string& local_file, const string& remote_file, char mode)
{
    //xfer(local_file, remote_file, sftpLIB_FILE_READ, mode);
}

void sftp::put(const string& local_file, const string& remote_file, char mode)
{
   // xfer(local_file, remote_file, sftpLIB_FILE_WRITE, mode);
}
void sftp::rm(const string& fullName) const {
    if (this->connected)
        libssh2_sftp_unlink(this->sftp_session, fullName.c_str());
}
bool sftp::rename(const string& src, const string& obj) const {

    if (this->connected){
	rm(obj);	//先rm掉目标文件，如果存在
	return libssh2_sftp_rename_ex(this->sftp_session, src.c_str(),src.size(),obj.c_str(),obj.size(),
									LIBSSH2_SFTP_RENAME_OVERWRITE |
									LIBSSH2_SFTP_RENAME_ATOMIC |
									LIBSSH2_SFTP_RENAME_NATIVE)==0;
        //return libssh2_sftp_rename((LIBSSH2_SFTP*)this->sftpSession, src.c_str(), obj.c_str()) == 0;
		}
    return false;
}

void sftp::dele(const string& file_name)
{
    string cmd = "DELE ";
    cmd += file_name;
    send_cmd(cmd, '2');
}

char* sftp::response()
{
    if (net_ctrl.dir == sftpLIB_CONTROL)
        return net_ctrl.response;
    return 0;
}

void sftp::close()
{
    assert(status == ACCESS);
    if (net_data.dir != sftpLIB_WRITE && net_data.dir != sftpLIB_READ)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "close");
    shutdown(net_data.handle, 2);
    ::close(net_data.handle);
   //	libssh2_channel_close(channel);
//	libssh2_channel_free(channel);
   if(sftp_session){
   	libssh2_sftp_shutdown(sftp_session);
	sftp_session = NULL;
   	}
	if (session) {
		libssh2_session_disconnect(session, "INFO: ssh session disconnect normally");
		libssh2_session_free(session);
		session = NULL;
	}

	libssh2_exit();
    net_data.handle = -1;
//	if (net_data.ctrl && !read_resp('2')){
//		status = LOGINED;     
//		throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "close");
//	}
    status = LOGINED;
}

void sftp::close_ctrl()
{
	assert(status == CONNECTED);
	shutdown(net_ctrl.handle, 2);
	::close(net_ctrl.handle);
	net_ctrl.handle = -1;
	status = IDLE;
}
void sftp::quit()
{
    assert(status == LOGINED);
    if (net_ctrl.dir != sftpLIB_CONTROL)
        return;
    
    send_cmd("QUIT", '2');
    dout<<"In quit, after send_cmd!"<<std::endl;
    ::close(net_ctrl.handle);
    net_ctrl.handle = -1;
    status = IDLE;
}

// Wait for socket to receive or flush data
bool sftp::socket_wait(netbuf* ctrl, int mode)
{
    fd_set fd;
    fd_set* rfd = 0;
    fd_set* wfd = 0;
    timeval tv;
    int rv = 0;

    switch(ctrl->dir)
    {
    case sftpLIB_WRITE:
        wfd = &fd;
    	break;
    case sftpLIB_READ:
        rfd = &fd;
    	break;
    case sftpLIB_CONTROL:
        if (mode == 1)
            wfd = &fd;
        else
            rfd = &fd;
        break;
    }
    
    FD_ZERO(&fd);
    do
    {
 
        FD_SET(ctrl->handle, &fd);
        tv = ctrl->idletime;
        

        rv = select(ctrl->handle + 1, rfd, wfd, 0, &tv);
        if (rv == -1)
        {
            rv = 0;
            dout<<"before strncpy in socket_wait!"<<std::endl;
            strncpy(ctrl->ctrl->response, strerror(errno), sizeof(ctrl->ctrl->response));
            dout<<"after strncpy in socket_wait!"<<std::endl;
            break;
        }
        else if (rv > 0)
        {
            rv = 1;
            break;
        }
        assert(ctrl->idlecb == 0);
        if (ctrl->idlecb == 0)
            break;
    } while (rv = ctrl->idlecb(ctrl, ctrl->xfered, ctrl->idlearg));
    return (rv == 1);
}

// Read a line of text, return bytecount.
int sftp::read_line(char* buf, int max, netbuf* ctl)
{
    int x;
    int retval = 0;
    char* end = 0;
    char* bp = buf;

    max--; // Used for end string null.
    if (max == 0)
    {
        buf[0] = '\0';
        return 0;
    }
    else if (max < 0)
    {
        throw bad_param(__FILE__, __LINE__, 174, "read_line");
    }
    
    if (ctl->dir != sftpLIB_CONTROL && ctl->dir != sftpLIB_READ)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "Invalid direction");

    while (1)
    {
        if (ctl->cavail > 0)
        {
            x = (max >= ctl->cavail) ? ctl->cavail : max;
            end = (char*)memccpy(bp, ctl->cget, '\n', x);
            if (end != 0) // Found return. Adjust read size.
        	x = end - bp;
            retval += x;
            bp += x;
            max -= x;
            ctl->cget += x;
            ctl->cavail -= x;
        }

        if (end != 0 || max == 0) // Read to end-of-line or max size.
            break;

        // Set control variables.
        if (ctl->cput == ctl->cget)
        {
            ctl->cput = ctl->cget = ctl->buf;
            ctl->cavail = 0;
            ctl->cleft = sftpLIB_BUFSIZ;
        }

        // Read from socket.
        if (!socket_wait(ctl, 0))
            return retval;
        x = ::read(ctl->handle, ctl->cput, ctl->cleft);
        if (x == -1) // Error occured.
            throw bad_file(__FILE__, __LINE__, 17, bad_file::bad_read, strerror(errno));
        else if (x == 0) // Read end-of-file.
            break;
        ctl->cleft -= x;
        ctl->cavail += x;
        ctl->cput += x;
    }

    buf[retval] = '\0';
    return retval;
}

// Write lines of text, return bytecount.
int sftp::write_line(char* buf, int len)
{
    int x;
    int w;
    int count;

    if(net_data.dir != sftpLIB_WRITE)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "Invalid direction");
    for (x = 0; x < len; x += sftpLIB_BUFSIZ)
    {
        if (!socket_wait(&net_data, 1))
            return x;
        count = (x + sftpLIB_BUFSIZ > len) ? len - x : sftpLIB_BUFSIZ;
        w = ::write(net_data.handle, buf + x, count);
        if (w != count)
            throw bad_file(__FILE__, __LINE__, 124, bad_file::bad_write);
    }
    return len;
}

// Read a response from the server
bool sftp::read_resp(char c)
{
    char match[5];
    read_line(net_ctrl.response, sftpLIB_RESPONSE, &net_ctrl);
    dout << net_ctrl.response << std::endl;
    if (net_ctrl.response[3] == '-')
    {
        strncpy(match, net_ctrl.response, 3);
        match[3] = ' ';
        match[4] = '\0';
        do
        {
            read_line(net_ctrl.response, sftpLIB_RESPONSE, &net_ctrl);
            dout << net_ctrl.response << std::endl;
        } while (strncmp(net_ctrl.response, match, 4));
    }
    if (net_ctrl.response[0] == c)
        return true;
    else
        return false;
}

// Send a command and wait for expected response
void sftp::send_cmd(const string& cmd, char expresp)
{
    string cmd_tmp;
    if (net_ctrl.dir != sftpLIB_CONTROL)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "send_cmd");
    dout << cmd << std::endl;
    cmd_tmp = cmd + "\r\n";
    socket_wait(&net_ctrl, 1);
    if (::write(net_ctrl.handle, cmd_tmp.c_str(), cmd_tmp.length()) <= 0)
        throw bad_file(__FILE__, __LINE__, 124, bad_file::bad_write, cmd);
    if (!read_resp(expresp))
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "read_resp:" + cmd);
}

// Set up data connection
void sftp::open_port(char mode, int dir)
{
    int data;
    union
    {
        sockaddr sa;
        sockaddr_in in;
    } sin;
    linger lng = { 0, 0 };
#if defined(HPUX)||defined(TRUE64)
    int l;
#else
    socklen_t l;
#endif
    int on = 1;
    char* cp;
    unsigned int v[6];
    char buf[256];

    if (net_ctrl.dir != sftpLIB_CONTROL)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "open_port");
    if ((dir != sftpLIB_READ) && (dir != sftpLIB_WRITE))
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "Invalid direction");
    if ((mode != sftpLIB_ASCII) && (mode != sftpLIB_IMAGE))
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "Invalid mode");
    l = sizeof(sin);
    if (net_ctrl.cmode == sftpLIB_PASSIVE)
    {
        memset(&sin, 0, l);
        sin.in.sin_family = AF_INET;
        send_cmd("PASV", '2');
        cp = strchr(net_ctrl.response, '(');
        if (cp == 0)
            throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "PASV");
        cp++;
        sscanf (cp, "%u,%u,%u,%u,%u,%u", &v[2], &v[3], &v[4], &v[5], &v[0], &v[1]);
        sin.sa.sa_data[2] = v[2];
        sin.sa.sa_data[3] = v[3];
        sin.sa.sa_data[4] = v[4];
        sin.sa.sa_data[5] = v[5];
        sin.sa.sa_data[0] = v[0];
        sin.sa.sa_data[1] = v[1];
    }
    else
    {
        if (getsockname(net_ctrl.handle, &sin.sa, &l) < 0)
            throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
    }
    data = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (data == -1)
	    throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "socket");
    if (setsockopt(data, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) == -1)
    {
        ::close(data);
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
    }
    if (setsockopt (data, SOL_SOCKET, SO_LINGER, (void*)&lng, sizeof(lng)) == -1)
    {
        ::close(data);
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
    }
    if (net_ctrl.cmode == sftpLIB_PASSIVE)
    {
        if (::connect(data, &sin.sa, sizeof (sin.sa)) == -1)
        {
            ::close(data);
            throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
        }
    }
    else
    {
        sin.in.sin_port = 0;
        if (bind(data, &sin.sa, sizeof (sin)) == -1)
        {
            ::close(data);
            throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
        }
        if (listen(data, 1) < 0)
        {
            ::close(data);
            throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
        }

        if (getsockname(data, &sin.sa, &l) < 0){
            ::close(data);
            throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
        }
        sprintf(buf, "PORT %d,%d,%d,%d,%d,%d",
            (unsigned char)sin.sa.sa_data[2],
            (unsigned char)sin.sa.sa_data[3],
            (unsigned char)sin.sa.sa_data[4],
            (unsigned char)sin.sa.sa_data[5],
            (unsigned char)sin.sa.sa_data[0],
            (unsigned char)sin.sa.sa_data[1]);
        try
        {
            send_cmd(buf, '2');
        }
        catch (...)
        {
            ::close(data);
            throw;
        }
    }

    memset(&net_data, '\0', sizeof(netbuf));
    net_data.handle = data;
    net_data.dir = dir;
    net_data.ctrl = (net_ctrl.cmode == sftpLIB_PASSIVE) ? &net_ctrl : 0;
    net_data.idletime = net_ctrl.idletime;
    net_data.idlearg = net_ctrl.idlearg;
    net_data.xfered = 0;
    net_data.xfered1 = 0;
    net_data.cbbytes = net_ctrl.cbbytes;
    if (net_data.idletime.tv_sec | net_data.idletime.tv_usec)
        net_data.idlecb = net_ctrl.idlecb;
    else
        net_data.idlecb = 0;
}

// Accept connection from server
void sftp::accept()
{
    int data;
    struct sockaddr addr;
#if defined(HPUX)||defined(TRUE64)
    int l;
#else
    socklen_t l;
#endif
    int i;
    struct timeval tv;
    fd_set mask;

    FD_ZERO(&mask);
    FD_SET(net_ctrl.handle, &mask);
    FD_SET(net_data.handle, &mask);
    tv.tv_usec = 0;
    tv.tv_sec = ACCEPT_TIMEOUT;
    i = net_ctrl.handle;
    if (i < net_data.handle)
        i = net_data.handle;
    i = select(i + 1, &mask, 0, 0, &tv);
    if (i == -1)
    {
        ::close(net_data.handle);
        net_data.handle = -1;
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
    }
    else if (i == 0)
    {
        ::close(net_data.handle);
        net_data.handle = -1;
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "time out");
    }
    else
    {
        if (FD_ISSET(net_data.handle, &mask))
        {
            l = sizeof(addr);
            data = ::accept(net_data.handle, &addr, &l);
            ::close(net_data.handle);
            if (data > 0)
            {
        	net_data.handle = data;
        	net_data.ctrl = &net_ctrl;
            }
            else
            {
        	net_data.handle = -1;
        	throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "FD_ISSET");
            }
        }
        else if (FD_ISSET(net_ctrl.handle, &mask))
        {
            ::close(net_data.handle);
            net_data.handle = -1;
            read_resp('2');
            throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "FD_ISSET");
        }
    }
}
/*
void sftp::xfer(const string& local_file, const string& path, int typ, char mode)
{
    int l;
    int c;
    char dbuf[sftpLIB_BUFSIZ];
    FILE* local;

    if (!local_file.empty())
    {
        local = fopen(local_file.c_str(), (typ == sftpLIB_FILE_WRITE) ? "r" : "w");
        if (local == 0)
            throw bad_file(__FILE__, __LINE__, 40, bad_file::bad_open, local_file);
    }
    else
    {
        local = (typ == sftpLIB_FILE_WRITE) ? stdin : stdout;
    }

    access(path, typ, mode);
    if (typ == sftpLIB_FILE_WRITE)
    {
        while ((l = fread(dbuf, 1, sftpLIB_BUFSIZ, local)) > 0)
        {
            if (!write(dbuf, l))
            {
                if (!local_file.empty())
                    fclose(local);
                close();
                throw bad_file(__FILE__, __LINE__, 124, bad_file::bad_write, "write");
            }
        }
    }
    else
    {
        while ((l = read(dbuf, sftpLIB_BUFSIZ)) > 0)
        {
            if (fwrite(dbuf, 1, l, local) <= 0)
            {
                if (!local_file.empty())
                    fclose(local);
                close();
                throw bad_file(__FILE__, __LINE__, 124, bad_file::bad_write, local_file);
            }
        }
    }
    if (!local_file.empty())
        fclose(local);
    close();
}
*/
// Return a handle for a data stream
void sftp::access(const string& path, int typ, char mode)
{
    assert(status == LOGINED);
    status = ACCESS;
    ostringstream fmt;
    string cmd;
    int dir;

    if (path.empty() && ((typ == sftpLIB_FILE_WRITE) || (typ == sftpLIB_FILE_READ))){
        status = LOGINED;
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "Missing path argument");
    }
    fmt << "TYPE " << mode;
    try{
    	send_cmd(fmt.str(), '2');
    }
    catch(...){
    	status = LOGINED;
    	throw;
    }
    switch (typ)
    {
    case sftpLIB_DIR:
        cmd = "NLST";
        dir = sftpLIB_READ;
        break;
    case sftpLIB_DIR_VERBOSE:
        cmd = "LIST";
        dir = sftpLIB_READ;
        break;
    case sftpLIB_FILE_READ:
        cmd = "RETR";
        dir = sftpLIB_READ;
        break;
    case sftpLIB_FILE_WRITE:
        cmd = "STOR";
        dir = sftpLIB_WRITE;
        break;
    default:
    	status = LOGINED;
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "Invalid open type");
    }
    if (!path.empty())
    {
        cmd += ' ';
        cmd += path;
    }
    try
    {
    	open_port(mode, dir);
    }
    catch(...)
    {
    	status = LOGINED;
    	throw;
    }
    try
    {
        send_cmd(cmd, '1');
        if (net_ctrl.cmode == sftpLIB_PORT)
            accept();
    }
    catch (...)
    {    
		close();
        throw;
    }
}

// Read from a data connection

long sftp::read(const void* handle, char* buf, const unsigned int max) 
{
    int i;

    i = libssh2_sftp_read((LIBSSH2_SFTP_HANDLE*)handle, (char*)buf, max);
    net_data.xfered += i;
    if (net_data.idlecb && net_data.cbbytes)
    {
        net_data.xfered1 += i;
        if (net_data.xfered1 > net_data.cbbytes)
        {
            net_data.idlecb(&net_data, net_data.xfered, net_data.idlearg);
            net_data.xfered1 = 0;
        }
    }
    return i;
}

// Write to a data connection
bool sftp::write(const void* handle, const char* buff, const unsigned int size)  
{
	int i;

	i =  libssh2_sftp_write((LIBSSH2_SFTP_HANDLE*)handle, buff, size);
	net_data.xfered += i;
    if (net_data.idlecb && net_data.cbbytes)
    {
        net_data.xfered1 += i;
        if (net_data.xfered1 > net_data.cbbytes)
        {
            net_data.idlecb(&net_data, net_data.xfered, net_data.idlearg);
            net_data.xfered1 = 0;
        }
    }
    return (i == size);
}

void sftp::reset()
{
	this->disconnect();
   
}

}

