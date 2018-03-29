
#include "user_exception.h"
#include "ipsocket.h"

namespace util
{

using std::cout;
using std::endl;
using util::bad_file;

#if defined(HPUX)
#undef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 20
#endif

/**
 * @fn util::ipsocket::ipsocket()
 * @brief The default constructor function.
 *
 * Creates an ipsocket object, and the socket descriptor is initialized with a 
 * first vaule of -1.
 *
 * @see util::ipsocket::ipsocket(int sfd)
 * @see util::ipsocket::ipsocket(Protocol protocol)
 */
ipsocket::ipsocket()
    : _socket(-1)
{
}

/**
 * @overload util::ipsocket::ipsocket(int sfd)
 *
 * Creates an ipsocket object, and the socket descriptor is initialized with a 
 * sockfd specified.
 *
 * param[in] sfd The sockfd.
 *
 * @see util::ipsocket::ipsocket()
 * @see util::ipsocket::ipsocket(Protocol protocol)
 */
ipsocket::ipsocket(int sfd)
    : _socket(sfd)
{
}

/**
 * @overload util::ipsocket::ipsocket(Protocol protocol)
 *
 * Creates an ipsocket object, and opens a new socket initialized with the 
 * protocol specified.
 *
 * param[in] protocol The protocol.
 *
 * @see util::ipsocket::ipsocket()
 * @see util::ipsocket::ipsocket(int sfd)
 */
ipsocket::ipsocket(Protocol protocol)
{
    open(protocol);
}

/**
 * @fn util::ipsocket::~ipsocket()
 * @brief The destructor function.
 *
 * To close a socket and destroy an ipsocket object.
 */
ipsocket::~ipsocket()
{
    if (is_opened())
    {
        ::close(_socket);
    }
}

/**
 * @fn void util::ipsocket::open(Protocol protocol)
 * @brief Creates a new socket initialized with the protocol specified.
 *
 * param[in] protocol The protocol.
 */
void ipsocket::open(Protocol protocol)
{
    if (!is_opened())
    {
        _socket = ::socket(AF_INET, protocol, 0);
        
        if (_socket < 0)
            throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
        int btmp = 1;
        int ret = ::setsockopt(_socket,SOL_SOCKET,SO_REUSEADDR,&btmp,sizeof(int)) ;
        if (ret != 0) {
            throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
        }
    }
}

/**
 * @fn void util::ipsocket::close()
 * @brief Closes the socket and terminates the connection.
 */
void ipsocket::close()
{
    if (is_opened())
    {
        ::close(_socket);
        _socket = -1;
    }
}

/**
 * @fn int util::ipsocket::get_socket_fd()
 * @brief Returns the socket descriptor.
 *
 * @return The socket descriptor.
 */
int ipsocket::get_socket_fd()
{
    return _socket;
}

/**
 * @fn bool util::ipsocket::is_opened()
 * @brief Returns the socket state.
 *
 * @return Returns ture if the socket descriptor is valid, false if it is invalid.
 */
bool ipsocket::is_opened()
{
    return _socket > 0;
}

/**
 * @fn void util::ipsocket::bind(const sockaddr_in* addr)
 * @brief Assigns a local protocol address to an unbound socket.
 *
 * @param[in] addr The pointer of the socket address structure.
 */
void ipsocket::bind(const sockaddr_in* addr)
{
#if defined(HPUX)
    if (::bind(_socket, addr, sizeof(sockaddr_in)) == -1)
#else
    if (::bind(_socket, reinterpret_cast<const sockaddr*>(addr), sizeof(sockaddr_in)) == -1)
#endif
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
}

/**
 * @fn void util::ipsocket::connect(const sockaddr_in* addr)
 * @brief The function is used by a client to establish a connection with a server.
 *
 * @param[in] addr The pointer of a socket address structure containing the 
 * address of a remote socket to which a connection is to be established.
 */
void ipsocket::connect(const sockaddr_in* addr)
{
#if defined(HPUX)
    if (::connect(_socket, addr, sizeof(sockaddr_in)) == -1)
#else
    if (::connect(_socket, reinterpret_cast<const sockaddr*>(addr), sizeof(sockaddr_in)) == -1)
#endif
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
}

/**
 * @fn int util::ipsocket::send(const char* data, size_t size, int flags = 0)
 * @brief Sends a messasge from a socket.
 *
 * The function may be used only when the socket is in a connected state (so 
 * that the intended recipient is known).
 *
 * @param[in] data Points to the buffer containing the message.
 * @param[in] size The length of the message.
 * @param[in] flags It is the bitwise OR of zero or more of the following flags: 
 * MSG_DONTROUTE, MSG_DONTWAIT, MSG_OOB. The default value is 0.
 *
 * @return Returns the number of characters sent, or -1 if an error occurred.
 */
int ipsocket::send(const char* data, size_t size, int flags)
{
    int bytes = ::send(_socket, data, size, flags);
    if (bytes < 0)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    return bytes;
}

/**
 * @fn int util::ipsocket::sendto(const sockaddr_in* addr, 
 *                                 const unsigned char* data, 
 *                                 size_t size, 
 *                                 int flags = 0)
 * @brief Sends a messasge from a socket.
 *
 * It can be used at any time.
 *
 * @param[in] addr The pointer of the socket address structure.
 * @param[in] data Points to the buffer containing the message.
 * @param[in] size The length of the message.
 * @param[in] flags It is the bitwise OR of zero or more of the following flags: 
 * MSG_DONTROUTE, MSG_DONTWAIT, MSG_OOB. The default value is 0.
 *
 * @return Returns the number of characters sent, or -1 if an error occurred.
 */
int ipsocket::sendto(const sockaddr_in* addr, const unsigned char* data, size_t size, int flags)
{
#if defined(HPUX)
    int bytes = ::sendto(_socket, data, size, flags, addr, sizeof(sockaddr_in));
#else
    int bytes = ::sendto(_socket, data, size, flags, reinterpret_cast<const sockaddr*>(addr), sizeof(sockaddr_in));
#endif
    if (bytes < 0)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    return bytes;
}

/**
 * @fn int util::ipsocket::recv(char* data, size_t size, int flags = 0)
 * @brief Receives a messasge from a socket.
 *
 * The function may be used only when the socket is in a connected state.
 *
 * @param[in] data Points to the buffer into which the messages are placed.
 * @param[in] size The maximum number of bytes that can fit in the buffer.
 * @param[in] flags It is the bitwise OR of zero or more of the following flags: 
 * MSG_DONTWAIT, MSG_OOB, MSG_PEEK, MSG_WAITALL. The default value is 0.
 *
 * @return Returns the number of bytes received, or -1 if an error occurred. The
 * return value will be 0 when the peer has performed an orderly shutdown.
 */
int ipsocket::recv(char* data, size_t size, int flags)
{
    int bytes = ::recv(_socket, data, size, flags);
    if (bytes < 0)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    return bytes;
}


/**
 * @fn int util::ipsocket::recvfrom(sockaddr_in* addr, 
 *                                  unsigned char* data, 
 *                                  size_t size, 
 *                                  int flags = 0)
 * @brief Receives a messasge from a socket.
 *
 * It may be used to receive data on a socket whether or not it is 
 * connection-oriented.
 *
 * @param[in] addr The pointer of the socket address structure.
 * @param[in] data Points to the buffer into which the messages are placed.
 * @param[in] size The maximum number of bytes that can fit in the buffer.
 * @param[in] flags It is the bitwise OR of zero or more of the following flags: 
 * MSG_DONTWAIT, MSG_OOB, MSG_PEEK, MSG_WAITALL. The default value is 0.
 *
 * @return Returns the number of bytes received, or -1 if an error occurred. The
 * return value will be 0 when the peer has performed an orderly shutdown and 
 * there is no more data to read (the socket has reached the end of its data 
 * stream.
 */
int ipsocket::recvfrom(sockaddr_in* addr, unsigned char* data, size_t size, int flags)
{
#if defined(HPUX)
    int len = sizeof(sockaddr_in);
    int bytes = ::recvfrom(_socket, data, size, flags, addr, &len);
#elif defined(TRUE64)
    int len = sizeof(sockaddr_in);
    int bytes = ::recvfrom(_socket, data, size, flags, reinterpret_cast<sockaddr*>(addr), &len);
#else
    socklen_t len = sizeof(sockaddr_in);
    int bytes = ::recvfrom(_socket, data, size, flags, reinterpret_cast<sockaddr*>(addr), &len);
#endif
    if (bytes < 0)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    return bytes;
}

/**
 * @fn void util::ipsocket::listen(int queuelen = 5)
 * @brief Listens for connections on a socket.
 *
 * To accept connections, a socket is first created using socket(), a queue for
 * incoming connections is activated using listen(), and then connections are 
 * accepted using accept(). listen() applies only to unconnected sockets of type
 * SOCK_STREAM.
 *
 * @param[in] queuelen It defines the maximum length the queue of pending 
 * connections may grow to. The default value is 5.
 */
void ipsocket::listen(int queuelen)
{
    if (::listen(_socket, queuelen) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
}

/**
 * @fn void util::ipsocket::accept(sockaddr_in* addr)
 * @brief Accepts a connection on a socket.
 *
 * It is used with connection-based socket types, such as SOCK_STREAM.
 *
 * @param[in] addr The pointer of the socket address structure.
 */
int ipsocket::accept(sockaddr_in* addr)
{
#if defined(HPUX)
    int len = sizeof(sockaddr_in);
    int fd = ::accept(_socket, addr, &len);
#elif defined(TRUE64)
    int len = sizeof(sockaddr_in);
    int fd = ::accept(_socket, reinterpret_cast<sockaddr*>(addr), &len);
#else
    socklen_t len = sizeof(sockaddr_in);
    int fd = ::accept(_socket, reinterpret_cast<sockaddr*>(addr), &len);
#endif
    if (fd < 0)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    return fd;
}

/**
 * @fn void util::ipsocket::set_linger(int on = 0, int l = 0)
 * @brief Sets the SO_LINGER socket option for the socket.
 *
 * It specifies how the close function operates for a connection-oriented 
 * protocol (e.g., for TCP and STCP, but not for UDP). By default, close returns
 * immediately, but if there is any data still remaining in the socket send 
 * buffer, the system will try to deliver the data to the peer.
 *
 * @param[in] on 0=off, nonzero=on. The default value is 0.
 * @param[in] l Linger time, POSIX specifies units as seconds. The default value 
 * is 0.
 */
void ipsocket::set_linger(int on, int l)
{
    struct linger opt;
    opt.l_onoff = on;
    opt.l_linger = l;

    if (::setsockopt(_socket, SOL_SOCKET, SO_LINGER, &opt, sizeof(opt)) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
}

/**
 * @fn int util::ipsocket::get_linger()
 * @brief Gets the current value of the SO_LINGER socket option for the socket.
 *
 * @return Returns 1 if the SO_LINGER is disabled, 0 if it is enabled.
 */
int ipsocket::get_linger()
{
    struct linger opt;
#if defined(HPUX)||defined(TRUE64)
    int opt_size = sizeof(opt);
#else
    socklen_t opt_size = sizeof(opt);
#endif
    if (::getsockopt(_socket, SOL_SOCKET, SO_LINGER, &opt, &opt_size) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    return (opt.l_onoff == 0);
}

/**
 * @fn void util::ipsocket::set_oobline(int on = 1)
 * @brief Sets the SO_OOBINLINE socket option for the socket.
 *
 * If enabled, specifies that out-of-band data should be placed in the normal 
 * input queue (i.e., inline). When this occurs, the MSG_OOB flag to the receive
 * functions cannot be used to read the out-of-band data. Default: disabled.
 *
 * @param[in] on 0=off, nonzero=on. The default value is 1.
 */
void ipsocket::set_oobline(int on)
{
    if (::setsockopt(_socket, SOL_SOCKET, SO_OOBINLINE, &on, sizeof(on)) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
}

/**
 * @fn int util::ipsocket::get_oobline()
 * @brief Gets the current value of the SO_OOBINLINE socket option for the 
 * socket.
 *
 * @return Returns 1 if the SO_OOBINLINE is enabled, 0 if it is disabled.
 */
int ipsocket::get_oobline()
{
    int opt;
#if defined(HPUX)||defined(TRUE64)
    int opt_size = sizeof(opt);
#else
    socklen_t opt_size = sizeof(opt);
#endif
    if (::getsockopt(_socket, SOL_SOCKET, SO_OOBINLINE, &opt, &opt_size) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    return opt;
}

/**
 * @fn void util::ipsocket::set_send_time_out(int timeout)
 * @brief Sets the SO_SNDTIMEO socket option for the socket.
 *
 * Specifies a timeout in seconds and microseconds on socket sends. The send 
 * timeout affects the five output functions: write, writev, send, sendto, and
 * sendmsg.
 *
 * @param[in] timeout The timeout interval.
 */
void ipsocket::set_send_time_out(int timeout)
{
    if (::setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
}

/**
 * @fn int util::ipsocket::get_send_time_out()
 * @brief Gets the current value of the SO_SNDTIMEO socket option for the socket.
 *
 * @return Returns the timeout interval.
 */
int ipsocket::get_send_time_out()
{
    int opt;
#if defined(HPUX)||defined(TRUE64)
    int opt_size = sizeof(opt);
#else
    socklen_t opt_size = sizeof(opt);
#endif
    if (::getsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, &opt, &opt_size) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    return opt;
}

/**
 * @fn void util::ipsocket::set_recv_time_out(int timeout)
 * @brief Sets the SO_RCVTIMEO socket option for the socket.
 *
 * Specifies a timeout in seconds and microseconds on socket receives. The 
 * receive timeout affects the five input functions: read, readv, recv, recvfrom, 
 * and recvmsg.
 *
 * @param[in] timeout The timeout interval.
 */
void ipsocket::set_recv_time_out(int timeout)
{
    if (::setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
}

/**
 * @fn int util::ipsocket::get_recv_time_out()
 * @brief Gets the current value of the SO_RCVTIMEO socket option for the socket.
 *
 * @return Returns the timeout interval.
 */
int ipsocket::get_recv_time_out()
{
    int opt;
#if defined(HPUX)||defined(TRUE64)
    int opt_size = sizeof(opt);
#else
    socklen_t opt_size = sizeof(opt);
#endif
    if (::getsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &opt, &opt_size) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    return opt;
}

/**
 * @fn void util::ipsocket::set_send_buffer_size(int size)
 * @brief Sets the SO_SNDBUF socket option for the socket.
 *
 * Specifies the maximum size of the send socket buffer. For SOCK_STREAM sockets,
 * the send buffer size limits how much data can be queued for transmission 
 * before the application is blocked. FOR SOCK_DGRAM sockets, the send buffer
 * size may limit the maximum size of messages that the application can send
 * through the socket.
 *
 * @param[in] size The value of the buffer size, in bytes.
 */
void ipsocket::set_send_buffer_size(int size)
{
    if (::setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
}

/**
 * @fn int util::ipsocket::get_send_buffer_size()
 * @brief Gets the current value of the SO_SNDBUF socket option for the socket.
 *
 * @return Returns the value of the send buffer size.
 */
int ipsocket::get_send_buffer_size()
{
    int opt;
#if defined(HPUX)||defined(TRUE64)
    int opt_size = sizeof(opt);
#else
    socklen_t opt_size = sizeof(opt);
#endif
    if (::getsockopt(_socket, SOL_SOCKET, SO_SNDBUF, &opt, &opt_size) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    return opt;
}

/**
 * @fn void util::ipsocket::set_send_buffer_size(int size)
 * @brief Sets the SO_RCVBUF socket option for the socket.
 *
 * Specifies the maximum size of the receive socket buffer. FOR SOCK_DGRAM 
 * sockets, the receive buffer size may limit the maximum size of messages that 
 * the socket can receive.
 *
 * @param[in] size The value of the buffer size, in bytes.
 */
void ipsocket::set_recv_buffer_size(int size)
{
    if (::setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
}

/**
 * @fn int util::ipsocket::get_recv_buffer_size()
 * @brief Gets the current value of the SO_RCVBUF socket option for the socket.
 *
 * @return Returns the value of the receive buffer size.
 */
int ipsocket::get_recv_buffer_size()
{
    int opt;
#if defined(HPUX)||defined(TRUE64)
    int opt_size = sizeof(opt);
#else
    socklen_t opt_size = sizeof(opt);
#endif
    if (::getsockopt(_socket, SOL_SOCKET, SO_RCVBUF, &opt, &opt_size) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    return opt;
}

/**
 * @fn void util::ipsocket::set_keep_alive(int on = 1)
 * @brief Sets the SO_KEEPALIVE socket option for the socket (SOCK_STREAM only).
 *
 * If enabled and no data has been exchanged across the socket in either 
 * direction for two hours, TCP automatically sends a keep-alive probe to the 
 * peer. This probe is a TCP segment to which the peer must respond.
 *
 * @param[in] on 0=off, nonzero=on. The default value is 1.
 */
void ipsocket::set_keep_alive(int on)
{
    if (::setsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
}

/**
 * @fn int util::ipsocket::get_keep_alive()
 * @brief Gets the current value of the SO_KEEPALIVE socket option for the 
 * socket.
 *
 * @return Returns 1 if the SO_KEEPALIVE is enabled, 0 if it is disabled.
 */
int ipsocket::get_keep_alive()
{
    int opt;
#if defined(HPUX)||defined(TRUE64)
    int opt_size = sizeof(opt);
#else
    socklen_t opt_size = sizeof(opt);
#endif
    if (::getsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, &opt, &opt_size) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    return opt;
}

/**
 * @fn void util::ipsocket::set_reuse_addr(int on = 1)
 * @brief Sets the SO_REUSEADDR socket option for the socket.
 *
 * If enabled, allows a local address to be reused in subsequent calls to bind().
 * Default: disallowed.
 *
 * @param[in] on 0=off, nonzero=on. The default value is 1.
 */
void ipsocket::set_reuse_addr(int on)
{
    if (::setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
}

/**
 * @fn int util::ipsocket::get_reuse_addr()
 * @brief Gets the current value of the SO_REUSEADDR socket option for the 
 * socket.
 *
 * @return Returns 1 if the SO_REUSEADDR is enabled, 0 if it is disabled.
 */
int ipsocket::get_reuse_addr()
{
    int opt;
#if defined(HPUX)||defined(TRUE64)
    int opt_size = sizeof(opt);
#else
    socklen_t opt_size = sizeof(opt);
#endif
    if (::getsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, &opt_size) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    return opt;
}

/**
 * @fn void util::ipsocket::get_local_addr(char* localaddr)
 * @brief Returns a character string in the dotted-quad format of the local 
 * address for the specificed socket.
 *
 * @param[in,out] localaddr The character string of the local address.
 */
void ipsocket::get_local_addr(char* localaddr)
{
    char *ip_addr;
    struct sockaddr_in local_addr;
    if ( localaddr == 0 || strlen(localaddr) < INET_ADDRSTRLEN + 1 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, "localaddr is null or localaddr length must be more than 16");
    memset(localaddr, 0, strlen(localaddr));
#if defined(HPUX)
    int local_addr_size;
    local_addr_size = sizeof(struct sockaddr_in);
    if ( ::getsockname(_socket, &local_addr, &local_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#elif defined(TRUE64)
    int local_addr_size;
    local_addr_size = sizeof(struct sockaddr_in);
    if ( ::getsockname(_socket, reinterpret_cast<sockaddr*>(&local_addr), &local_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#else
    socklen_t local_addr_size;
    local_addr_size = sizeof(struct sockaddr_in);
    if ( ::getsockname(_socket, reinterpret_cast<sockaddr*>(&local_addr), &local_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#endif
#if defined(HPUX)
    if ( (ip_addr = ::inet_ntoa(local_addr.sin_addr)) == 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    strcpy(localaddr,ip_addr);
#else
    if ( ::inet_ntop(AF_INET, &local_addr.sin_addr, localaddr, strlen(localaddr) - 1) == 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#endif
}

/**
 * @overload void util::ipsocket::get_local_addr(string& localaddr)
 *
 * @param[in,out] localaddr The string of the local address.
 */
void ipsocket::get_local_addr(string& localaddr)
{
    struct sockaddr_in local_addr;
    char *ip_addr;

#if defined(HPUX)
    int local_addr_size;
    local_addr_size = sizeof(struct sockaddr_in);
    if ( ::getsockname(_socket, &local_addr, &local_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#elif defined(TRUE64)
    int local_addr_size;
    local_addr_size = sizeof(struct sockaddr_in);
    if ( ::getsockname(_socket, reinterpret_cast<sockaddr*>(&local_addr), &local_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#else
    socklen_t local_addr_size;
    local_addr_size = sizeof(struct sockaddr_in);
    if ( ::getsockname(_socket, reinterpret_cast<sockaddr*>(&local_addr), &local_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#endif
#if defined(HPUX)
    if ( (ip_addr = ::inet_ntoa(local_addr.sin_addr)) == 0 )
#else
    if ( ::inet_ntop(AF_INET, &local_addr.sin_addr, ip_addr, INET_ADDRSTRLEN) == 0 )
#endif
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    localaddr = ip_addr;
}

/**
 * @fn int util::ipsocket::get_local_port()
 * @brief Returns a port number of the local address for the specificed socket.
 *
 * @return Returns a number of the port.
 */
int ipsocket::get_local_port()
{
    struct sockaddr_in local_addr;
#if defined(HPUX)
    int local_addr_size;
    local_addr_size = sizeof(struct sockaddr_in);
    if ( ::getsockname(_socket, &local_addr, &local_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#elif defined(TRUE64)
    int local_addr_size;
    local_addr_size = sizeof(struct sockaddr_in);
    if ( ::getsockname(_socket, reinterpret_cast<sockaddr*>(&local_addr), &local_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#else
    socklen_t local_addr_size;
    local_addr_size = sizeof(struct sockaddr_in);
    if ( ::getsockname(_socket, reinterpret_cast<sockaddr*>(&local_addr), &local_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#endif
    return ntohs(local_addr.sin_port);
}

/**
 * @fn void util::ipsocket::get_remote_addr(char* remoteaddr)
 * @brief Returns a character string in the dotted-quad format of the remote 
 * address for the specificed socket.
 *
 * @param[in,out] localaddr The character string of the remoate address.
 */
void ipsocket::get_remote_addr(char* remoteaddr)
{
    char *ip_addr;
    struct sockaddr_in remote_addr;

    if ( remoteaddr == 0 || strlen(remoteaddr) < INET_ADDRSTRLEN )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, "remoteaddr is null or remoteaddr length must be more than 16");
    memset(remoteaddr, 0, strlen(remoteaddr));
#if defined(HPUX)
    int remote_addr_size;
    remote_addr_size = sizeof(struct sockaddr_in);
    if ( ::getpeername(_socket, &remote_addr, &remote_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#elif defined(TRUE64)
    int remote_addr_size;
    remote_addr_size = sizeof(struct sockaddr_in);
    if ( ::getpeername(_socket, reinterpret_cast<sockaddr*>(&remote_addr), &remote_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#else
    socklen_t remote_addr_size;
    remote_addr_size = sizeof(struct sockaddr_in);
    if ( ::getpeername(_socket, reinterpret_cast<sockaddr*>(&remote_addr), &remote_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#endif
#if defined(HPUX)
    if ( (ip_addr = ::inet_ntoa(remote_addr.sin_addr)) == 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    strcpy(remoteaddr, ip_addr);
#else
    if ( ::inet_ntop(AF_INET, &remote_addr.sin_addr, remoteaddr, strlen(remoteaddr) - 1) == 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#endif
}

/**
 * @overload void util::ipsocket::get_remote_addr(string& remoteaddr)
 *
 * @param[in,out] localaddr The string of the remote address.
 */
void ipsocket::get_remote_addr(string& remoteaddr)
{
    struct sockaddr_in remote_addr;
    char *ip_addr;

#if defined(HPUX)
    int remote_addr_size;
    remote_addr_size = sizeof(struct sockaddr_in);
    if ( ::getpeername(_socket, &remote_addr, &remote_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#elif defined(TRUE64)
    int remote_addr_size;
    remote_addr_size = sizeof(struct sockaddr_in);
    if ( ::getpeername(_socket, reinterpret_cast<sockaddr*>(&remote_addr), &remote_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#else
    socklen_t remote_addr_size;
    remote_addr_size = sizeof(struct sockaddr_in);
    if ( ::getpeername(_socket, reinterpret_cast<sockaddr*>(&remote_addr), &remote_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#endif
#if defined(HPUX)
    if ( (ip_addr = ::inet_ntoa(remote_addr.sin_addr)) == 0 )
#else
    if ( ::inet_ntop(AF_INET, &remote_addr.sin_addr, ip_addr, INET_ADDRSTRLEN) == 0 )
#endif
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    remoteaddr = ip_addr;
}

/**
 * @fn int util::ipsocket::get_remote_port()
 * @brief Returns a port number of the remote address for the specificed socket.
 *
 * @return Returns a number of the port.
 */
int ipsocket::get_remote_port()
{
    struct sockaddr_in remote_addr;
#if defined(HPUX)
    int remote_addr_size;
    remote_addr_size = sizeof(struct sockaddr_in);
    if ( ::getpeername(_socket, &remote_addr, &remote_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#elif defined(TRUE64)
    int remote_addr_size;
    remote_addr_size = sizeof(struct sockaddr_in);
    if ( ::getpeername(_socket, reinterpret_cast<sockaddr*>(&remote_addr), &remote_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#else
    socklen_t remote_addr_size;
    remote_addr_size = sizeof(struct sockaddr_in);
    if ( ::getpeername(_socket, reinterpret_cast<sockaddr*>(&remote_addr), &remote_addr_size) < 0 )
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
#endif
    return ntohs(remote_addr.sin_port);
}

/**
 * @fn void util::ipsocket::shutdown(Shutdown how)
 * @brief It is used to shut down a socket. 
 *
 * In the case of a full-duplex connection, shutdown() can be used to either 
 * partially or fully shut down the socket, depending upon the value of how.
 *
 * @param[in] how The type of shutdown.
 */
void ipsocket::shutdown(Shutdown how)
{
    if (::shutdown(_socket, how) == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
}

/**
 * @fn bool util::ipsocket::wait(int pollmode, int sec = -1, int usec = -1)
 * @brief Synchronous I/O multiplexing.
 *
 * It indicates which of the specified socket descriptors is ready for reading, 
 * ready for writing, or has an error condition pending. If the specified 
 * condition is false for all of the specified socket descriptors, it blocks, up
 * to the specified timeout interval, until the specified condition is true for
 * at least one of the specified socket descriptors.
 *
 * @param[in] pollmode The type of the conditions for waiting.
 * @param[in] sec The timeout interval, seconds. The default value is -1.
 * @param[in] usec The timeout interval, microseconds. The default value is -1.
 *
 * @return Returns ture if the condition is ture, false if timeout.
 */
bool ipsocket::wait(int pollmode, int sec, int usec)
{
    int ret = -1;
    struct timeval waittime, *pwaittime = NULL;
    fd_set fds;

    if (sec >= 0 && usec >= 0)
    {
        waittime.tv_sec  = sec;
        waittime.tv_usec = usec;
        pwaittime = &waittime;
    }

    FD_ZERO(&fds);
    FD_SET(_socket, &fds);

    switch((enPollMode)pollmode)
    {
    case READ_POLL:
        ret = ::select(_socket + 1, &fds, NULL, NULL, pwaittime);
	      break;
    case WRITE_POLL:
        ret = ::select(_socket + 1, NULL, &fds, NULL, pwaittime);
	      break;
    case EXCEPTION_POLL:
        ret = ::select(_socket + 1, NULL, NULL, &fds, pwaittime);
        break;
    }

    if (ret == -1)
        throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
    ret = FD_ISSET(_socket, &fds);

    return (ret != 0);
}

/**
 * @fn int util::ipsocket::read(void* buf, int maxsize, int readmode = 1)
 * @brief Reads from a socket descriptor.
 *
 * @param[in] buf Points to the buffer into which the messages are placed.
 * @param[in] maxsize The maximum number of bytes that can fit in the buffer.
 * @param[in] readmode Mode 0: read until the size of receive bytes is maxsize, 
 * 1: only read once. The default value is 1.
 *
 * @return The number of bytes read is returned (zero indicates end of file).
 */
int ipsocket::read(void* buf, int maxsize, int readmode)
{
    int nread, nleft, nret;

    nleft = maxsize;
    nread = 0;
    while ( nleft > 0 )
    {
        nret = ::read(_socket, (char*)buf + nread, nleft);
        if ( nret == 0 )
            break;
        else if ( nret > 0)
        {
            if (nread > maxsize)
                throw bad_msg(__FILE__, __LINE__, 219, "recieve buffer is too small.");
            nread += nret;
            nleft -= nret;
            if (readmode) 
                break;
        }
        else
        {
            if ( errno == EINTR )
                continue;
            else
                throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
        }
    }

    return nread;
}

/**
 * @fn bool util::ipsocket::write(void* buf, int size)
 * @brief Writes to a socket descriptor.
 *
 * @param[in] buf Points to the buffer containing the message.
 * @param[in] size The length of the message.
 *
 * @return Returns true if the number of size written successfully, otherwise 
 * reutrns false.
 */
bool ipsocket::write(void* buf, int size)
{
    int nwrite, nleft, nret;

    nleft = size;
    nwrite = 0;
    while ( nleft > 0 )
    {
        nret = ::write(_socket, (char*)buf + nwrite, nleft);
        if ( nret == 0 )
            break;
        else if ( nret > 0)
        {
            nwrite += nret;
            nleft -= nret;
        }
        else
        {
            if ( errno == EINTR )
                continue;
            else
                throw bad_file(__FILE__, __LINE__, 219, bad_file::bad_sock, strerror(errno));
        }
    }

    return (nwrite == size);
}

}

