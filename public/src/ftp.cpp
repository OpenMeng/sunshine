////////////////////////////////////////////////////////////////////////////////////////
// modificaton histiory
//---------------------------------------------------------------------------------------
// add method 
// void ftp::list(const string& output, const string& path, const string& filename) 
//---------------------------------------------------------------------------------------
// add method    void ftp::close_ctrl()-Handle CONNECTED status,close a control connetion.
// modify method void ftp::reset()------Do close_ctrl() instead of close() when match CONNECTED.
//										Close all handles and set status to IDLE 
//										when exception occured.
// modify method void ftp::open_port()--Add ::close(data) before throw an ex.
// modify method void ftp::access()-----Set status back to LOGINED before throw an ex.
// Set handles to default value -1 after all close actions.
//----------------------------------------------------------------------------------------
#include "ftp.h"

namespace util
{

using std::ostringstream;
using util::bad_file;
using std::cout;
using std::clog;

ftp::ftp()
{
    status = IDLE;
    net_ctrl.handle = -1;
    net_data.handle = -1;
}

ftp::ftp(const string& host_, const string& user_, const string& passwd_)
{
    host = host_;
    user = user_;
    passwd = passwd_;
    status = IDLE;
    net_ctrl.handle = -1;
    net_data.handle = -1;    
}

ftp::~ftp()
{
    reset();
}

void ftp::connect()
{
    assert(status == IDLE);
    connect(host);
    status = CONNECTED;
}

void ftp::connect(const string& host_)
{
	dout << "["<<__FILE__<<"] LINE["<<__LINE__<<"]-connect() host_ = ["<<host_<<"]"<<std::endl;
    assert(status == IDLE);
    int sock_fd;
    sockaddr_in sin;
    hostent *phe;
    servent *pse;
    string tmp_host;
    
    host = host_;
    memset(&sin, 0, sizeof(sin));
    size_t pos = host.find_first_of(':');
    if (pos == string::npos) // Not found.
    {
#if defined(VMS)
    	sin.sin_port = htons(21);
#else
        pse = getservbyname("ftp", "tcp");
        if (pse == 0)
            throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
        sin.sin_port = pse->s_port;
#endif

        tmp_host = host;
    }
    else
    {
        sin.sin_port = atoi(host.c_str() + pos + 1);
        if (sin.sin_port < 1)
        {
            pse = getservbyname(host.c_str() + pos + 1, "tcp");
            if (pse == 0)
                throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
            sin.sin_port = pse->s_port;
        }
        else
        {
            sin.sin_port = htons(sin.sin_port);
        }
        tmp_host = string(host.begin(), host.begin() + pos);
    }
    
    if (::inet_aton(tmp_host.c_str(), &sin.sin_addr))
    {
        sin.sin_family = AF_INET;
    }
    else
    {
        if ((phe = gethostbyname(tmp_host.c_str())) == 0)
            throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
        sin.sin_family = phe->h_addrtype;
        memcpy((char*)&sin.sin_addr, phe->h_addr_list[0], phe->h_length);
    }

    sock_fd = socket(sin.sin_family, SOCK_STREAM, 0);
    dout<< "["<<__FILE__<<"] LINE["<<__LINE__<<"]-"<<"In connect(),sockaddr_in: sin_family = ["<<sin.sin_family<<"] sin_port = ["
    		<<sin.sin_port<<"] sin_addr = ["<<	::inet_ntoa(sin.sin_addr)<<"]"<<std::endl;
    dout<< "["<<__FILE__<<"] LINE["<<__LINE__<<"]-"<<"In connect(),sock_fd = ["<<sock_fd<<"]"<<std::endl;

    if (sock_fd == -1)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
    
    if (::connect(sock_fd, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) == -1)
    {
        ::close(sock_fd);
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
    }

    memset(&net_ctrl, '\0', sizeof(netbuf));

    net_ctrl.handle = sock_fd;
    net_ctrl.dir = FTPLIB_CONTROL;
    net_ctrl.ctrl = 0;
    net_ctrl.cmode = FTPLIB_DEFMODE;
    net_ctrl.idlecb = 0;
    net_ctrl.idletime.tv_sec = 240;
    net_ctrl.idletime.tv_usec = 0;
    net_ctrl.idlearg = 0;
    net_ctrl.xfered = 0;
    net_ctrl.xfered1 = 0;
    net_ctrl.cbbytes = 0;
    if (!read_resp('2'))
    {
        ::close(sock_fd);
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
    }
    status = CONNECTED;
}

bool ftp::options(int opt, long val)
{
	dout << "["<<__FILE__<<"] LINE["<<__LINE__<<"]-options() opt = ["<<opt<<"] val = ["<<val<<"]"<<std::endl;
    int v;
    bool ret = false;
    switch (opt)
    {
    case FTPLIB_CONNMODE:
        v = static_cast<int>(val);
        if ((v == FTPLIB_PASSIVE) || (v == FTPLIB_PORT))
        {
            ret = true;
            net_ctrl.cmode = v;
        }
        break;
    case FTPLIB_CALLBACK:
        ret = true;
        net_ctrl.idlecb = reinterpret_cast<FtpCallback>(val);
        break;
    case FTPLIB_IDLETIME:
        v = static_cast<int>(val);
        ret = true;
        net_ctrl.idletime.tv_sec = 600;
        net_ctrl.idletime.tv_usec = 0;
        break;
    case FTPLIB_CALLBACKARG:
        ret = true;
        net_ctrl.idlearg = reinterpret_cast<void*>(val);
        break;
    case FTPLIB_CALLBACKBYTES:
        ret = true;
        net_ctrl.cbbytes = static_cast<int>(val);
        break;
    }
    return ret;
}

void ftp::set_host(const string& host_)
{
    host = host_;
}

void ftp::set_user(const string& user_)
{
    user = user_;
}

void ftp::set_passwd(const string& passwd_)
{
    passwd = passwd_;
}

void ftp::login()
{
    assert(status == CONNECTED);
    login(user, passwd);
    status = LOGINED;
}

void ftp::login(const string& user_, const string& passwd_)
{
    assert(status == CONNECTED);
    string cmd;
    user = user_;
    passwd = passwd_;
    cmd = "USER ";
    cmd += user;
    try
    {
        send_cmd(cmd, '3');
    }
    catch (...)
    {
        if (net_ctrl.response[0] == '2')
            throw;
    }
    cmd = "PASS ";
    cmd += passwd;
    send_cmd(cmd, '2');
    status = LOGINED;
}

void ftp::site(const string& cmd)
{
    string cmd_tmp = "SITE ";
    cmd_tmp += cmd;
    send_cmd(cmd_tmp, '2');
}

void ftp::syst(string& buf)
{
    char* s;
    send_cmd("SYST", '2');
    s = net_ctrl.response + 4;
    buf = "";
    while (*s != ' ')
        buf += *s++;
}

void ftp::mkd(const string& path)
{
    string cmd = "MKD ";
    cmd += path;
    send_cmd(cmd, '2');
}

void ftp::cwd(const string& path)
{
    string cmd = "CWD ";
    cmd += path;
    send_cmd(cmd, '2');
}

void ftp::cdup()
{
    send_cmd("CDUP", '2');
}

void ftp::rmd(const string& path)
{
    string cmd = "RMD ";
    cmd += path;
    send_cmd(cmd, '2');
}

void ftp::pwd(string& path)
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

void ftp::nlst(const string& outputfile, const string& path)
{
    xfer(outputfile, path, FTPLIB_DIR, FTPLIB_ASCII);
}

void ftp::list(const string& outputfile, const string& path)
{
    cwd(path);
    xfer(outputfile, path, FTPLIB_DIR_VERBOSE, FTPLIB_ASCII);
}

void ftp::list(const string& outputfile, const string& path, const string& filename)
{
    if (path.length() != 0)
    	cwd(path);
    xfer(outputfile, filename, FTPLIB_DIR_VERBOSE, FTPLIB_ASCII);
}

void ftp::size(const string& path, int& size, char mode)
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

void ftp::mdtm(const string& file_name, string& dt)
{
    string cmd = "MDTM ";
    cmd += file_name;
    send_cmd(cmd, '2');
    dt.assign(net_ctrl.response + 4, net_ctrl.response + 18);
}

void ftp::get(const string& local_file, const string& remote_file, char mode)
{
    xfer(local_file, remote_file, FTPLIB_FILE_READ, mode);
}

void ftp::put(const string& local_file, const string& remote_file, char mode)
{
    xfer(local_file, remote_file, FTPLIB_FILE_WRITE, mode);
}

void ftp::rename(const string& src, const string& dst)
{
    int ntry = 3;
    while (ntry--)
    {
        try
        {
            string cmd;
            cmd = "RNFR ";
            cmd += src;
            send_cmd(cmd, '3');
            cmd = "RNTO ";
            cmd += dst;
            send_cmd(cmd, '2');
            return;
        }
        catch (...)
        {
            if (ntry == 0)
                throw;
        }
    }
}

void ftp::dele(const string& file_name)
{
    string cmd = "DELE ";
    cmd += file_name;
    send_cmd(cmd, '2');
}

char* ftp::response()
{
    if (net_ctrl.dir == FTPLIB_CONTROL)
        return net_ctrl.response;
    return 0;
}

void ftp::close()
{
    assert(status == ACCESS);
    if (net_data.dir != FTPLIB_WRITE && net_data.dir != FTPLIB_READ)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "close");
    shutdown(net_data.handle, 2);
    ::close(net_data.handle);
    net_data.handle = -1;
	if (net_data.ctrl && !read_resp('2')){
		status = LOGINED;     
		throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "close");
	}
    status = LOGINED;
}

void ftp::close_ctrl()
{
	assert(status == CONNECTED);
	shutdown(net_ctrl.handle, 2);
	::close(net_ctrl.handle);
	net_ctrl.handle = -1;
	status = IDLE;
}
void ftp::quit()
{
    assert(status == LOGINED);
    if (net_ctrl.dir != FTPLIB_CONTROL)
        return;
    
    send_cmd("QUIT", '2');
    dout<<"In quit, after send_cmd!"<<std::endl;
    ::close(net_ctrl.handle);
    net_ctrl.handle = -1;
    status = IDLE;
}

// Wait for socket to receive or flush data
bool ftp::socket_wait(netbuf* ctrl, int mode)
{
    fd_set fd;
    fd_set* rfd = 0;
    fd_set* wfd = 0;
    timeval tv;
    int rv = 0;

    switch(ctrl->dir)
    {
    case FTPLIB_WRITE:
        wfd = &fd;
    	break;
    case FTPLIB_READ:
        rfd = &fd;
    	break;
    case FTPLIB_CONTROL:
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
int ftp::read_line(char* buf, int max, netbuf* ctl)
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
    
    if (ctl->dir != FTPLIB_CONTROL && ctl->dir != FTPLIB_READ)
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
            ctl->cleft = FTPLIB_BUFSIZ;
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
int ftp::write_line(char* buf, int len)
{
    int x;
    int w;
    int count;

    if(net_data.dir != FTPLIB_WRITE)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "Invalid direction");
    for (x = 0; x < len; x += FTPLIB_BUFSIZ)
    {
        if (!socket_wait(&net_data, 1))
            return x;
        count = (x + FTPLIB_BUFSIZ > len) ? len - x : FTPLIB_BUFSIZ;
        w = ::write(net_data.handle, buf + x, count);
        if (w != count)
            throw bad_file(__FILE__, __LINE__, 124, bad_file::bad_write);
    }
    return len;
}

// Read a response from the server
bool ftp::read_resp(char c)
{
    char match[5];
    read_line(net_ctrl.response, FTPLIB_RESPONSE, &net_ctrl);
    dout << net_ctrl.response << std::endl;
    if (net_ctrl.response[3] == '-')
    {
        strncpy(match, net_ctrl.response, 3);
        match[3] = ' ';
        match[4] = '\0';
        do
        {
            read_line(net_ctrl.response, FTPLIB_RESPONSE, &net_ctrl);
            dout << net_ctrl.response << std::endl;
        } while (strncmp(net_ctrl.response, match, 4));
    }
    if (net_ctrl.response[0] == c)
        return true;
    else
        return false;
}

// Send a command and wait for expected response
void ftp::send_cmd(const string& cmd, char expresp)
{
    string cmd_tmp;
    if (net_ctrl.dir != FTPLIB_CONTROL)
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
void ftp::open_port(char mode, int dir)
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

    if (net_ctrl.dir != FTPLIB_CONTROL)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "open_port");
    if ((dir != FTPLIB_READ) && (dir != FTPLIB_WRITE))
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "Invalid direction");
    if ((mode != FTPLIB_ASCII) && (mode != FTPLIB_IMAGE))
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "Invalid mode");
    l = sizeof(sin);
    if (net_ctrl.cmode == FTPLIB_PASSIVE)
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
    if (net_ctrl.cmode == FTPLIB_PASSIVE)
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
    net_data.ctrl = (net_ctrl.cmode == FTPLIB_PASSIVE) ? &net_ctrl : 0;
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
void ftp::accept()
{
	dout << "["<<__FILE__<<"] LINE["<<__LINE__<<"]-accept()"<<std::endl;
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
    dout << "["<<__FILE__<<"] LINE["<<__LINE__<<"]- i = ["<<i<<"]"<<std::endl;

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
        	dout << "["<<__FILE__<<"] LINE["<<__LINE__<<"]- net_data.handle = ["<<net_data.handle<<"]"<<std::endl;
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
        	dout << "["<<__FILE__<<"] LINE["<<__LINE__<<"]- net_ctrl.handle = ["<<net_ctrl.handle<<"]"<<std::endl;
            ::close(net_data.handle);
            net_data.handle = -1;
            read_resp('2');
            throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "FD_ISSET");
        }
    }
}

void ftp::xfer(const string& local_file, const string& path, int typ, char mode)
{
	dout << "["<<__FILE__<<"] LINE["<<__LINE__<<"]-xfer() local_file = ["<<local_file<<"] path = ["<<path<<"] typ = ["<<typ<<"] mode = ["<<mode<<"]"<<std::endl;

	int l;
    int c;
    char dbuf[FTPLIB_BUFSIZ];
    FILE* local;

    if (!local_file.empty())
    {
        local = fopen(local_file.c_str(), (typ == FTPLIB_FILE_WRITE) ? "r" : "w");
        if (local == 0)
            throw bad_file(__FILE__, __LINE__, 40, bad_file::bad_open, local_file);
    }
    else
    {
        local = (typ == FTPLIB_FILE_WRITE) ? stdin : stdout;
    }

    access(path, typ, mode);
    if (typ == FTPLIB_FILE_WRITE)
    {
        while ((l = fread(dbuf, 1, FTPLIB_BUFSIZ, local)) > 0)
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
        while ((l = read(dbuf, FTPLIB_BUFSIZ)) > 0)
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

// Return a handle for a data stream
void ftp::access(const string& path, int typ, char mode)
{
	dout << "["<<__FILE__<<"] LINE["<<__LINE__<<"]-access() path = ["<<path<<"] typ = ["<<typ<<"] mode = ["<<mode<<"]"<<std::endl;

	assert(status == LOGINED);
    status = ACCESS;
    ostringstream fmt;
    string cmd;
    int dir;

    if (path.empty() && ((typ == FTPLIB_FILE_WRITE) || (typ == FTPLIB_FILE_READ))){
        status = LOGINED;
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "Missing path argument");
    }
    fmt << "TYPE " << mode;
    try{
    	send_cmd(fmt.str(), '2');
    }
    catch(...){
    	dout << "["<<__FILE__<<"] LINE["<<__LINE__<<"]- exception when send TYPE command. "<<std::endl;
    	status = LOGINED;
    	throw;
    }

    switch (typ)
    {
    case FTPLIB_DIR:
        cmd = "NLST";
        dir = FTPLIB_READ;
        break;
    case FTPLIB_DIR_VERBOSE:
        cmd = "LIST";
        dir = FTPLIB_READ;
        break;
    case FTPLIB_FILE_READ:
        cmd = "RETR";
        dir = FTPLIB_READ;
        break;
    case FTPLIB_FILE_WRITE:
        cmd = "STOR";
        dir = FTPLIB_WRITE;
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
    	dout << "["<<__FILE__<<"] LINE["<<__LINE__<<"]- exception when open_port(mode = ["<<mode<<"] dir = ["<<dir<<"]) "<<std::endl;
    	status = LOGINED;
    	throw;
    }

    try
    {
        send_cmd(cmd, '1');
    	dout << "["<<__FILE__<<"] LINE["<<__LINE__<<"]- net_ctrl.cmode = ["<<net_ctrl.cmode<<"]"<<std::endl;
        if (net_ctrl.cmode == FTPLIB_PORT){
            accept();
        }
    }
    catch (...)
    {    
    	dout << "["<<__FILE__<<"] LINE["<<__LINE__<<"]- exception when send_cmd(cmd = ["<<cmd<<"])"<<std::endl;
		close();
        throw;
    }
}

// Read from a data connection
int ftp::read(void* buf, int max)
{
    int i;

    if (net_data.dir != FTPLIB_READ)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "Invalid direction");
    socket_wait(&net_data, 0);
    i = ::read(net_data.handle, buf, max);
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
bool ftp::write(void* buf, int len)
{
	dout << "["<<__FILE__<<"] LINE["<<__LINE__<<"]-write() buf = ["<<buf<<"] len = ["<<len<<"]"<<std::endl;
    int i;

    if (net_data.dir != FTPLIB_WRITE)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, "Invalid direction");
    socket_wait(&net_data, 1);
    i = ::write(net_data.handle, buf, len);
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
    return (i == len);
}

void ftp::reset()
{
    dout<<"net_ctrl.handle = ["<<net_ctrl.handle<<"]"<<std::endl;
    dout<<"net_data.handle = ["<<net_data.handle<<"]"<<std::endl;
    try
    {
        switch (status)
        {
        case CONNECTED:
            close_ctrl();
            break;
        case LOGINED:  
            quit();
            break;
        case ACCESS:  
            close();
            quit();
            break;
        default:
            break;
        }
    }
    catch (...) 
    {
    	::close(net_ctrl.handle);
    	::close(net_data.handle);
    	net_ctrl.handle = -1;
    	net_data.handle = -1;
    	status = IDLE;
    }
}

}

