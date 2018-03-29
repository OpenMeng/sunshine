
#ifndef _SOCKET_H
#define _SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cerrno>
#include <string>
#include <iostream>
#include "user_assert.h"

using std::string;

namespace util
{

/**
 * @class ipsocket
 * @brief The ipsocket class.
 */
class ipsocket
{
public:
/**
 * @enum Protocol
 * @brief The type of the protocols.
 */
    enum Protocol
    {
        TCP = SOCK_STREAM,    ///<Connection-oriented.
        UDP = SOCK_DGRAM    ///<Connectionless.
    };

/**
 * @enum Shutdown
 * @brief The type of shutdown.
 */
    enum Shutdown
    {
        READ = SHUT_RD,    ///<Disables further receive operations.
        WRITE = SHUT_WR,    ///<Disables further send operations.
        READ_WRITE = SHUT_RDWR    ///<Disables further send operations and receive operations.
    };

/**
 * @enum enPollMode
 * @brief The type of the conditions for waiting.
 */
    enum enPollMode
    {
        READ_POLL,    ///<Ready for reading.
        WRITE_POLL,    ///<Ready for writing.
        EXCEPTION_POLL    ///<Has an error condition pending.
    };

    ipsocket();
    ipsocket(int sfd);
    explicit ipsocket(Protocol protocol);
    ~ipsocket();

    void open(Protocol protocol);
    void close();
    int  get_socket_fd();
    bool is_opened();
    void bind(const sockaddr_in* addr);
    void connect(const sockaddr_in* addr);
    int  send(const char* data, size_t size, int flags = 0);
    int  sendto(const sockaddr_in* addr, const unsigned char* data, size_t size, int flags = 0);
    int  recv(char* data, size_t size, int flags = 0);
    int  recvfrom(sockaddr_in* addr, unsigned char* data, size_t size, int flags = 0);

    void listen(int queuelen = 5);
    int  accept(sockaddr_in* addr);
    void shutdown(Shutdown how);
    // if sec = -1 or usec = -1 then the last parameter timeout of select function is null.
    bool wait(int pollmode, int sec = -1, int usec = -1);
    // mode 0: read until the size of recieve bytes is maxsize, 1: only read once.
    int  read(void* buff, int maxsize, int readmode = 1);
    bool write(void* buff, int size);
    void get_local_addr(char* localaddr);
    void get_local_addr(string& localaddr);
    int  get_local_port();
    void get_remote_addr(char* remoteaddr);
    void get_remote_addr(string& remoteaddr);
    int  get_remote_port();

    void set_linger(int on = 0,int l = 0);
    int  get_linger();
    void set_oobline(int on = 1);
    int  get_oobline();
    void set_send_time_out(int timeout);
    int  get_send_time_out();
    void set_recv_time_out(int timeout);
    int  get_recv_time_out();
    void set_send_buffer_size(int size);
    int  get_send_buffer_size();
    void set_recv_buffer_size(int size);
    int  get_recv_buffer_size();
    void set_keep_alive(int on = 1);
    int  get_keep_alive();
    void set_reuse_addr(int on = 1);
    int  get_reuse_addr();
private:
    int _socket;    ///<The socket descriptor.
};

}

#endif
