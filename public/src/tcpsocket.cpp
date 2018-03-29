#include "user_exception.h"
#include "tcpsocket.h"

namespace util
{

using namespace std;

/**
 * @fn util::tcpsocket::tcpsocket()
 * @brief The default constructor function.
 *
 * Creates a tcpsocket object, and the runmode is initialized with a first vaule 
 * of -1.
 */
tcpsocket::tcpsocket()
    : ipsocket()
{
    _runmode = -1;
}

/**
 * @fn util::tcpsocket::~tcpsocket()
 * @brief The destructor function.
 *
 * To close a socket and destroy a tcpsocket object.
 */
tcpsocket::~tcpsocket()
{
    close();
}

/**
 * @fn int util::tcpsocket::create(const char* ipaddr, int port)
 * @brief Creates a TCP socket, 
 *
 * To close a socket and destroy a tcpsocket object.
 */
//client use
int tcpsocket::create(const char* ipaddr, int port)
{
    ::memset(&inet_address, 0, sizeof(inet_address));
    if (ipaddr == 0)
    {
        inet_address.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        inet_address.sin_addr.s_addr = inet_addr(ipaddr) ;
    }
    inet_address.sin_family = AF_INET;
    inet_address.sin_port = htons(port);

    _runmode = 0;
    open(TCP);
    connect(&inet_address);
    return 0;
}

//server use
int tcpsocket::create(int port)
{
    open(TCP);
    ::memset(&inet_address, 0, sizeof(inet_address));
    inet_address.sin_family = AF_INET;
    inet_address.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_address.sin_port = htons(port);
    bind(&inet_address);
    accept(&inet_address);
    _runmode = 1;
    return 0;
}

void tcpsocket::receive(string& msg)
{
    int read_count;
    int total;
    int len;
    char buff[4];

    if (this == 0)
        return;

    // Read length.
    total = 0;
    do
    {
        read_count = recv(buff + total, 4 - total);
        if (read_count < 0)
        {
            if (errno == EINTR)
                continue;
            else
                throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
        }
        total += read_count;
        if (total == 0) // No data available.
            throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
    } while (total < 4);

    // Get message length.
    len = ((buff[0] & 0xff) << 24)  + ((buff[1] & 0xff) << 16) + ((buff[2] & 0xff) << 8) + (buff[3] & 0xff);
    if (len <= 0)
        throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));

    // Read message.
    char* data = new char[len + 1];
    total = 0;
    do
    {
        read_count = ipsocket::recv(data + total, len - total);
        if (read_count < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                delete[] data;
                throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
            }
        }
        total += read_count;
    } while (total < len);
    
    data[len] = '\0';
    msg = data;
    delete[] data;

    dout << "length = " << msg.length() << " [" << msg << "]" << std::endl;
}

void tcpsocket::send(const string& msg)
{
    dout << "length = " << msg.length() << " [" << msg << "]" << std::endl;

    if (this == 0)
        return;

    int write_count;
    int len = msg.length();
    char* data = new char[len + 4];

    // This is length of packet.
    data[3] = len & 0xff;
    data[2] = (len >> 8) & 0xff;
    data[1] = (len >> 16) & 0xff;
    data[0] = (len >> 24) & 0xff;
    memcpy(data + 4, msg.c_str(), len);
    
    int total = 0;
    len += 4;
    do
    {
        write_count = ipsocket::send(data + total, len - total);
        if (write_count < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                delete[] data;
                throw bad_file(__FILE__, __LINE__, 173, bad_file::bad_sock, strerror(errno));
            }
        }
        total += write_count;
    } while (total < len);

    delete[] data ;
}

}

