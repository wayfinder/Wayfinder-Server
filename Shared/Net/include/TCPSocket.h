/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include "config.h"
#include "Socket.h"
#include "SysUtility.h"
#include "StepLogger.h"

class IPnPort;

#define DEFAULT_BACKLOG 5

/** 
 *    System independant socket interface. 
 *
 *    The Windows type SOCKET is an u_int, while linux represents sockets 
 *    as signed integers. This is of importance in error reporting, where
 *    linux' socket systemcalls returns -1 upon failure. The <winsock2.h>
 *    header file defines the macro SOCKET_ERROR, which is equal to
 *    #(u_int) ~(0) = 0xffffffff = MAX_UINT32 = (u_int) (-1)#.
 *
 */ 
class TCPSocket : public Socket, public Writeable, public Readable,
                  public StepLogger
{   
   public:
      /**
        *   The possible choices when listen on a port.
        *      - @b GENERIC ``Ordinary bind'' to the port.
        *      - @b FINDFREEPORT  Tries to find a free port.
        * \end{description}
        */
      enum TCPPortChoice {GENERIC, FINDFREEPORT};

      /**
       *    The possible states of a TCP-socket.
       */
      enum TCPSocketState {
         /// State unknown (probably just created).
         UNKNOWN, 
         
         /// Socket open.
         OPEN, 
         
         /// Socket open for listening.
         LISTEN, 
         
         /// The socket is connected to another socket.
         CONNECTED, 
         
         /// This socket is closed.
         CLOSED };

      /**
       *    Create a new socket.
       */
      TCPSocket();

      /**
       *    Create a new socket with given number of connections accepted
       *    at once.
       *    @param   backlog  The number of connections accepted to this
       *                      socket at the same time.
       */
      TCPSocket(int backlog);

      /**
       *    Delete this socket. If the socket is open, it is also closed.
       */
      virtual ~TCPSocket();

      /**
       *    Open the socket.
       *    @return  True upon success. For now, this method does
       *             a PANIC exit upon failure.
       */
      virtual bool open();

      /**
       *    Bind the socket. Used to set the source address when  
       *    you later do a connect().
       *    @param   hostname  The name of the local host to bind to.
       *    @return  True upon success.
       */
      virtual bool bind(const char *hostname);

      /**
       *    Bind the socket. Used in when you later do a listen().
       *    @param   hostname  The name of the local host to bind to.
       *    @param   portNumber  The local port number to bind to.
       *    @return  True upon success.
       */
      virtual bool bind(const char *hostname, uint16 portNumber);

      /**
       *    Close the socket.
       */
      virtual void close();

      /**
        *   Listen for connections on this socket.
        *  
        *   @param   portnumber.
        *   @param   pc In which way the port is selected. The value
        *            of this parameter is defaulted to "GENERIC".
        *   @param   bindSocket If socket should be bound, default true.
        *   @return  The portnumber used. 0 is returned if no free port 
        *            was found.
        */
      virtual uint16 listen(uint16 portnumber, TCPPortChoice pc = GENERIC,
                            bool bindSocket = true );

      /**
       * Open and listen on a port with timed retries, try every 100
       * micro second for duration seconds.
       *
       * @param port The port to listen too.
       * @param duration Number of seconds to (re)try to listen.
       * @param bindSocket If socket should be bound, default true.
       */
      virtual bool listenDuration( uint16 port, uint32 duration = 1,
                                   bool bindSocket = true );

      /**
       *    Initiate a connection on a tcp socket. Since this method looks
       *    up the ip-address from the hostname (gethostbyname(char*)), the
       *    other connect-method should be used when possible.
       * 
       *    @param   hostname    The name of the host to connect to.
       *    @param   portnumber  The port number to connect to at 
       *                         hostname.
       *    @return  True is returned on success, false upon error.
       */
      virtual bool connect(const char *hostname, int portnumber);

      /**
       *    Initiate a connection on a tcp socket with a timeout . Since this method looks
       *    up the ip-address from the hostname (gethostbyname(char*)), the
       *    other connect-method should be used when possible.
       * 
       *    @param   hostname    The name of the host to connect to.
       *    @param   portnumber  The port number to connect to at 
       *                         hostname.
       *    @param   timeout_us  The timeout in micro seconds
       *
       *    @return  True is returned on success, false upon error.
       */
      virtual bool connect(const char *hostname, int portnumber, uint32 timeout_us);

      /**
       *    Initiate a connection on a tcp socket.
       *
       *    @param   addr      The ip-address and port of the host to connect to.
       *    @param   timeout_us  The timeout in micro seconds
       *
       *    @return  True is returned on success, false upon error.
       */   
      virtual bool connect(const IPnPort& addr, uint32 timeout_us);

      /**
       *    Initiate a connection on a tcp socket.
       *
       *    @param   hostip      The ip-address of the host to connect to.
       *    @param   portnumber  The port number to connect to at 
       *                         hostname.
       *
       *    @return  True is returned on success, false upon error.
       */
      virtual bool connect(uint32 hostip, uint16 hostport);


      /**
       *    If the function is not done in 10 minutes, something is 
       *    wrong.
       */
      #define TCPSOCKET_DEFAULT_TIMEOUT (10 * 60 * 1000000)
      
      /**
       *    Also define the infinite timeout-time.
       */
      #define TCPSOCKET_INFINITE_TIMEOUT (-1)

      /**
       *    Disable or enable the nagle algo.
       */
      int setNoDelay(bool noDelay);

      /**
       *    Read from the socket.
       *
       *    @param   buffer  The buffer where the result will be stored.
       *                     This buffer must be at least length bytes big.
       *    @param   size    The requested size.
       *    @return  The number of successfully read bytes or -1
       *             upon failure, 0 on closed socket and -3 on EAGAIN.
       *             EAGAIN can only happen on non blocking sockets.
       */
      virtual ssize_t read( byte* buffer, size_t size );


      /**
       *    Read from the socket with timeout. Does a select before
       *    calling read.
       *
       *    @param   buffer  The buffer where the result will be stored.
       *                     This buffer must be at least length bytes big.
       *    @param   size    The requested size.
       *    @return  The number of successfully read bytes or -1
       *             upon failure,  -2 on timeout, 0 on closed socket and
       *             -3 on EAGAIN. EAGAIN can only happen on non blocking
       *             sockets.
       */
      virtual ssize_t read( byte* buffer, size_t size, uint32 timeout );


      /**
       *    Read the specified number of bytes from the socket.
       *    This is
       *    a hanging method that will not return until the specified
       *    number of bytes are read, or an error has occured.
       *    unless <code>micros</code> != TCPSOCKET_INFINITE_TIMEOUT or
       *    <code>totalMicros</code> != <code>TCPSOCKET_INFINITE_TIMEOUT</code>
       *    when this method will hang until the specified number of bytes
       *    are read, an error has occured, or the timeout has expired.
       *    There are two different timeouts here. If <code>micros</code>
       *    is given and <code>totalMicros</code> is
       *    <code>TCPSOCKET_INFINITE_TIMEOUT</code> the socket will
       *    time out if nothing is received in <code>micros</code>
       *    micro seconds. If both <code>micros</code> and <code>totalMicros
       *    </code> are given the socket will timeout if no
       *    data is received in <code>micros</code> micro seconds and
       *    if all of the data is not received in <code>totalMicros</code>
       *    micro seconds. 
       * 
       *    @param   buffer      The buffer where the result will be stored.
       *                         This buffer must be at least length bytes
       *                         big.
       *    @param   length      The requested length.
       *    @param   micros      The timeout value of this function. If
       *                         no data is received for micros microseconds
       *                         the function will timeout.
       *                         Default value is TCPSOCKET_INFINITE_TIMEOUT.
       *    @param   totalMicros If all data isn't received in totalMicros
       *                         then the function will timeout.
       *                         Default value is TCPSOCKET_INFINITE_TIMEOUT.
       *    @return  The number of successully read bytes or -1
       *             upon failure.
       */
      virtual ssize_t readExactly(byte *buffer,
                           size_t length,
                           long micros = TCPSOCKET_INFINITE_TIMEOUT,
                           long totalMicros = TCPSOCKET_INFINITE_TIMEOUT);

      /**
       *    Read from the socket with timeout.
       * 
       *    @param   buffer      The buffer where the result will be stored.
       *                         This buffer must be at least length bytes
       *                         big.
       *    @param   length      The requested length.
       *    @param   micros      The max number of microseconds to wait.
       *    @return  The number of successully read bytes or a negative value
       *             uponi failure. -1 is returned upon "general" failure,
       *             and -2 on timeout.
       */
      ssize_t readMaxBytes( byte *buffer, 
                            size_t length, 
                            uint32 micros );
      
      /**
       *    Read from the socket. This method will hang until anything is
       *    available on the socket. When something becomes available it
       *    will read as much as possible from the socket (up to length
       *    bytes) and return. The number of bytes read might be anything
       *    from 1 to length, so there is no guarantee that length bytes 
       *    will be read.
       * 
       *    @param   buffer   The buffer where the result will be stored.
       *                      This buffer must be at least length bytes
       *                      big.
       *    @param   length   The requested length.
       * 
       *    @return  The number of successully read bytes or -1
       *             upon failure
       */
      inline virtual ssize_t readMaxBytes(byte *buffer, size_t length);

      /**
       *    Read from the socket. This method will hang until anything is
       *    available on the socket. When something becomes available it
       *    will read as much as possible from the socket (up to length
       *    bytes) until the socket is closed. The number of bytes read 
       *    might be anything from 1 to length, so there is no guarantee 
       *    that length bytes will be read.
       * 
       *    @param   buffer   The buffer where the result will be stored.
       *                      This buffer must be at least length bytes
       *                      big.
       *    @param   length   The requested length.
       * 
       *    @return  The number of successully read bytes.
       */
      virtual ssize_t readUntilClosed(byte *buffer, size_t length);
      

      /**
       *    Write to the socke.
       * 
       *    @param   buffer   A buffer containing the data that will be 
       *                      written to the socket.
       *    @param   length   The number of bytes that will be written.
       *    @return  The number of successfully written bytes or -1
       *             upon failure, 0 on closed socket and -3 on EAGAIN.
       *             EAGAIN can only happen on non blocking sockets.
       */
       virtual ssize_t write( const byte *buffer, size_t length );


      /**
       *    Write to the socket with timeout. Does a select before
       *    calling write.
       * 
       *    @param   buffer   A buffer containing the data that will be 
       *                      written to the socket.
       *    @param   length   The number of bytes that will be written.
       *    @param   timeout  The timeout in microseconds.
       *    @return  The number of successfully written bytes or -1
       *             upon failure, -2 on timeout, 0 on closed socket and
       *             -3 on EAGAIN. EAGAIN can only happen on non blocking
       *             sockets.
       */
       virtual ssize_t write( const byte *buffer, size_t length,
                              uint32 timeout );


      /**
       *    Write to the socket. Writes until all is written or
       *    socket is closed/broken.
       * 
       *    @param   buffer   A buffer containing the data that will be 
       *                      written to the socket.
       *    @param   length   The number of bytes that will be written.
       *    @return  The number of successfully written bytes or -1
       *             upon failure.
       */
      virtual ssize_t writeAll(const byte *buffer, size_t length);

      /**
       *    Accept a connection. 
       *    @return  A TCPSocket that is connected to a socket at the 
       *             caller. This socket must be deleted by the caller
       *             to accept.
       */
      virtual TCPSocket* accept();

      /**
       *    Accept a connection, wait maximum a specified time.
       *    @param micros  Timeout value in micro seconds.
       *    @return  A TCPSocket that is connected to a socket at the 
       *             caller. This socket must be deleted by the caller
       *             to accept. NULL is returned if timeout.
       */
      TCPSocket* accept(uint32 micros);
      
      /**
       *    Get the IP-address and port of the peer socket.
       *    @param IP   Outparameter that is set to the IP-address of 
       *                the peer.
       *    @param port Outparameter that is set to the port number at 
       *                the peer.
       *    @return  True if the outparameter IP and port contains valid 
       *             data, false otherwise.
       */
      bool getPeerName(uint32& IP, uint16& port) const;

      /**
       *    @return ip number and port for the peer of the socket
       *            or 0,0 if error.
       */
      IPnPort getPeer() const;

      /**
       *    Get the IP-address and port of the socket (the local end).
       *    @param IP   Outparameter that is set to the IP-address of 
       *                the socker.
       *    @param port Outparameter that is set to the port number at 
       *                the local host.
       *    @return  True if the outparameter IP and port contains valid 
       *             data, false otherwise.
       */
      bool getSockName(uint32& IP, uint16& port);

      /**
       *    Read a string from this socket.
       *    Times out if nothing is received within DEFAULT_TIMEOUT.
       *    @param target  A preallocated string with a length of at 
       *                   least maxstringlength. The read string is
       *                   copied into this string.
       *    @return The number of bytes read, -1 if socket error.
       */
      virtual int32 readString(char* target, uint32 maxstringlength);

      /**
       *    Read a string ending with linebreak from this socket.
       *    Times out if nothing is received within DEFAULT_TIMEOUT.
       *    @param target  A preallocated string with a length of at 
       *                   least maxlinelength. The read string is
       *                   copied into this string.
       *    @return The number of bytes read, -1 if socket error.
       */
      virtual int32 readLine(char* target, uint32 maxlinelength);

      /**
       *    Get the state of this socket.
       *    @return  The state of this socket.
       */
      TCPSocketState getState() const;

      /**
       *    Get the state of this socket.
       *    @return  A string describing the state of this socket. This
       *             string must not be deleted.
       */
      const char* getStateAsString() const;

      /**
       *    Sets the socket to blocking or nonblocking depending on
       *    the value blocking. The nonblocking socket is not fully
       *    supported in all functions, e.g. the state of the socket
       *    is undefined after a connect and some functions will
       *    misbehave when the socket is nonblocking.
       *    @param blocking True if the socket should be set to blocking.
       *                    False if the socket should be set to nonblocking.
       *    @return true if state change was successfull
       */
      bool setBlocking(bool blocking);

   
      /**
       *    Constructor that takes an fd as parameter.
       *    @param   sock     The filedescriptor of the socket.
       *    @param   backlog  The value of the backlog variable.
       */
      TCPSocket(SOCKET sock, int backlog);


      /**
       *    Initializes a socket using reads/writes after an accept.
       *    The default is to to return true.
       *    @param micros The timeout value of this function. If
       *                  no data is received for micros microseconds
       *                  the function will timeout.
       *                  Default value is TCPSOCKET_INFINITE_TIMEOUT.
       *    @return True.
       */
      virtual bool initialHandshake( 
         long micros = TCPSOCKET_INFINITE_TIMEOUT );


      /**
       *    Initializes a socket using reads/writes after an accept.
       *    The default is to to return true.
       *
       *    @return True.
       */
      virtual int initialHandshaking();

   protected:
      /**
       *    True if the socket is blocking.
       */
      bool m_blocking;
   
      /**
       *    The current state of this socket.
       */
      TCPSocketState m_currentState;


      /**
       *    The maximum number of connections allowed to this socket.
       */
      int m_backlog;
      
      
      /**
       *    The identificationnumber of the tcp protocol. Initialized
       *    in the constructors ba a call to 
       *    #getprotobyname("tcp")->p_proto;#
       *
       *    Since this call can't be
       *    made in windows before the call to initWinSock, this must
       *    be a member here.
       */
      static int c_tcp_proto_nbr;

      /**
       *  Read from the socket with timeout. This method reads from the
       *  socket until minLength bytes are read or until it times out.
       *  No more than maxLength bytes will be written to the buffer.
       *  @param minLength The minimum number of bytes to wait for
       *                   while timeout has not occured.
       *  @see readMaxBytes with timeout for the rest of the parameters.
       *  @param micros The unit of micros is microseconds,
       *                but the accuracy is in milliseconds.
       */
      virtual ssize_t readMinMaxBytesWithTimeout(byte* buffer,
                                                 size_t minLength,
                                                 size_t maxLength,
                                                 long micros,
                                                 long totalMicros);
      
      /**
       *    Reads from the socket taking care of EINTR.
       *    Hope this works, the man page for errno says it's
       *    thread local.
       *    @see privateWrite.
       *    @param buffer The buffer to put the result into.
       *    @param size   The maximum size to read.
       *    @return The number of bytes read or -1 if errno != EINTR.
       */
      virtual ssize_t protectedRead( byte* buffer,
                                     size_t size );
      
      /**
       *    Writes to the socket taking care of EINTR.
       *    @see privateRead.
       *    @param buffer The buffer to write.
       *    @param size   The maximum size to write.
       *    @return The number of bytes written or -1 if errno != EINTR.
       */
      virtual ssize_t protectedWrite( const byte* buffer,
                                      size_t size );

   
     

      /**
       *    Checks if there is enough cached data on the socket to
       *    fill the buffer.
       *    The default implementation is to return false.
       *
       *    @param    buffer The buffer to write cached data into.
       *    @param    size   The amount of data to get.
       *    @return True if size bytes was written into buffer from cache,
       *            false if not.
       */
      virtual bool checkForCachedData( byte* buffer, size_t size );

      /**
       * Internal accept that does the system accept call and returns
       * 0 if accept failed. 
       *
       * @return The socket from accept call.
       */
      virtual SOCKET internalAccept();

   private:
      /**
       *    Initiate this socket. This method should be called from the
       *    constructors.
       *    @param   sock     The filedesciptor of the socket.
       *    @param   backlog  The value of the backlog variable.
       */
      void init(SOCKET sock, int backlog = DEFAULT_BACKLOG);

      /**
       *    Currently only sets the socket to not use the Nagle algorithm.
       *    This means that the socket does not wait before sending the
       *    data on the network. Must be called after e.g. connect.
       */
      void setupSocket();
      /**
       * Connect to socket using sockaddr_in
       * @param sin  The sockaddr
       * @return true on success else false
       */
       bool connect(struct sockaddr_in &sin);
       /**
        * Waits for connect() to finish and reads error
        * @return true if connect() was done successfully or not
        */
       bool waitForConnect(uint32 timeout_us);
};


inline ssize_t
TCPSocket::readMaxBytes(  byte *buffer,
                          size_t size  )
{
#ifndef _MSC_VER
   DEBUG4(cerr << "TCPSocket::readMaxBytes(buffer = " << buffer
          << ", size = " << size << ")" << endl; );

   if (m_sock == 0 ) {
      MC2ERROR("TCPSocket::readMaxBytes(byte*, size_t) - m_sock == 0");
      return -1;
   }   
   SysUtility::setPipeDefault();
#else
   almostAssert(m_sock != 0);
#endif

   ssize_t result = protectedRead(buffer, size);

#ifndef _MSC_VER
   SysUtility::setPipeDefault();
#endif
   return result;
}

inline ssize_t
TCPSocket::protectedRead( byte* buffer,
                        size_t size )
{
   size_t readLength;
#ifndef _MSC_VER
#  ifdef EINTR
   do { // Read until the error isn't EINTR
      readLength = ::read(m_sock, buffer, size);
      if ( readLength < 0 && errno == EINTR ) {
         MC2WARNING("TCPSocket::protectedRead - EINTR - trying again");
      }
   } while ( readLength < 0 && errno == EINTR );
#  else
   readLength = ::read(m_sock, buffer, size);
#  endif

#else // IT IS _MSC_VER
   readLength = ::recv(m_sock, (char *)buffer, size, 0);
#endif
      return readLength;
}

#endif // TCPSOCKET_H

