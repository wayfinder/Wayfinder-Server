/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TCPSocket.h"
#include "TimeUtility.h"
#include "NetUtility.h"
#include "SysUtility.h"
#include "DebugClock.h"
#include "IPnPort.h"

#ifndef _MSC_VER
   #include <sys/select.h>
   #include <netinet/tcp.h>
   #include <stdlib.h>
#endif

#ifdef __CYGWIN__
#define getdtablesize() (howmany(FD_SETSIZE, NFDBITS))
#endif

#include <string.h>

#ifndef _MSC_VER
// Will be inited before the threads start in linux.
// Will crash if getprotobyname returns NULL.
int TCPSocket::c_tcp_proto_nbr = getprotobyname("tcp")->p_proto;
#else
int TCPSocket::c_tcp_proto_nbr = 0;
#endif

namespace SockHelp {

#ifndef _MSC_VER
inline void close( int& sock ) {
   if ( sock != 0 ) {
      ::close(sock); 
      sock = 0;
   }
}
#else
inline void close( SOCKET& sock ) {
   if ( sock != 0 ) {
      ::closesocket(sock);
      sock = 0;
   }
}
#endif

/// unblocks the socket during its life time, if needed.
struct UnBlock {
   UnBlock( TCPSocket& sock, bool& currentBlock ):
      m_sock( sock ), 
      m_currBlock( currentBlock ),
      m_oldBlock( currentBlock ) { 
      // if socket already blocking, unblock it
      if ( m_oldBlock ) {
         if ( ! m_sock.setBlocking( false ) ) {
            mc2log << warn << "[TCPSocket] non blocking failed." << endl;
         }
      }
   }

   ~UnBlock() {
      // if old socket was blocking, block it.
      if ( m_oldBlock != m_currBlock ) {
         if ( ! m_sock.setBlocking( true ) ) {
            mc2log << warn << "[TCPSocket] blocking failed." << endl;
         }
      }
   }

   TCPSocket& m_sock; //< socket to unblock during this objects life time
   bool& m_currBlock; //< actual block value
   bool m_oldBlock; //< old block value
};

}

TCPSocket::TCPSocket()
{
   init(0);
}

TCPSocket::TCPSocket(int backlog)
{
   init(0, backlog);
}

TCPSocket::TCPSocket(SOCKET sock, int backlog)
{
   init(sock, backlog);
   // Set state to Connected as that is most likely the case here
   m_currentState = CONNECTED;
   setupSocket();
}

bool
TCPSocket::initialHandshake( long micros ) {
   // Nothing needed for standard socket.
   return true;
}


int 
TCPSocket::initialHandshaking() {
   // Nothing needed for standard socket.
   return true;
}


void
TCPSocket::init(SOCKET sock, int backlog)
{
   if ( c_tcp_proto_nbr == 0 ) {
      // Not initialized, probably windows.
      c_tcp_proto_nbr = getprotobyname("tcp")->p_proto;
      if ( c_tcp_proto_nbr == 0 ) {
         PANIC("Couldn't get protocol entry for protocol 'tcp'", "");
      }
   }

   m_blocking = true;
   m_backlog = backlog;
   m_sock = sock;
   m_currentState = UNKNOWN;
}

int
TCPSocket::setNoDelay(bool noDelay)
{
   // The socket should send data as soon as it is available or not.
   unsigned int onoff = noDelay ? 1 : 0;
#ifdef _MSC_VER
   int level = SOL_SOCKET;
#else
   int level = IPPROTO_TCP;
#endif
   return setsockopt(m_sock, level, TCP_NODELAY,
                     (char*)&onoff, sizeof(onoff));
}

void
TCPSocket::setupSocket()
{
   // Make sure we have a valid fd
   if (m_sock != 0) {
      return;
   }

   m_blocking = true;
 
   // The socket should send data as soon as it is available
   if ( setNoDelay(true) ) {
      mc2log << warn << "TCPSocket::setupSocket, failed to set TCP_NODELAY; "
              << strerror(errno) << endl;
   }
   // Set the keepalive option
   static const unsigned int on = 1;
   if (setsockopt(m_sock, SOL_SOCKET,
                  SO_KEEPALIVE, (char*)&on, sizeof(on)) < 0 ) {
      mc2log << warn << "TCPSocket::setupSocket, failed to set SO_KEEPALIVE; "
              << strerror(errno) << endl;
   }
}

TCPSocket::~TCPSocket()
{
   if ( m_sock != 0 ) 
      this->close();
}

bool
TCPSocket::open()
{
   DebugClock openClock;
   if ( m_sock == 0 ) {
      m_sock = socket( PF_INET, SOCK_STREAM, c_tcp_proto_nbr );

      if ( m_sock < 0 ) {
         PANIC("TCPSocket::open() failure: ", strerror(errno));
      }
   }       
   m_currentState = OPEN;
#define ADD_STEPS
#ifdef ADD_STEPS
   addStepTime( "Open", openClock );
#endif
   unsigned int on = 1;
   if (setsockopt(m_sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&on, sizeof(on)) < 0 ) {
      mc2log << warn << "TCPSocket::setupSocket, failed to set SO_KEEPALIVE; "
             << strerror(errno) << endl;
   }
   return true;
}

bool
TCPSocket::bind(const char *hostname)
{
	return bind(hostname, 0);
}

bool
TCPSocket::bind(const char *hostname, uint16 portName)
{
   struct sockaddr_in sin;

   memset(&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   if ( ! NetUtility::gethostbyname( hostname, sin.sin_addr ) ) {
      #ifdef __SVR4
      sin.sin_addr.s_addr = inet_addr(hostname);
      if (inet_addr(hostname) < 0 ) {
      #else
      if ((sin.sin_addr.s_addr = inet_addr(hostname)) == INADDR_NONE) {
      #endif
         mc2log << warn << "TCPSocket::bind():sys_inet_addr failure: "
                << strerror(errno) << endl;
         mc2log << warn << "Could not lookup hostname " << hostname
                << "\nSocket will now be closed."
                << endl;
         SockHelp::close( m_sock );
         m_currentState = CLOSED;
         return (false);
      }
   }

   if (::bind( m_sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      DEBUG1(cerr << "TCPSocket::bindSocket() failure: "
         << strerror(errno) << endl);
      return(false);
   }

   return(true);
}

void
TCPSocket::close()
{
   mc2dbg4 << "TCPSocket::close()" << endl;
   
   // Don't close stdin or whatever.
   if ( m_sock == 0 ) {
      DEBUG1(cerr << "TCPSocket::close - m_sock == 0."
             << endl);
      return;
   }
      
   // finish with graceful close
   ::shutdown(m_sock, 1); // Close input

   char c;

#ifndef _MSC_VER
   fcntl(m_sock, F_SETFL, O_NONBLOCK);
   int i = ::read(m_sock, &c, 1);
#else
   unsigned long arg = 1;
   ioctlsocket(m_sock, FIONBIO, &arg);
   int i = ::recv(m_sock, &c, 1, 0);
#endif
   
   while ( !( (i == 0) || (i == SOCKET_ERROR) ) ) {

#ifndef _MSC_VER
      i = ::read(m_sock, &c, 1);
#else
      i = ::recv(m_sock, &c, 1, 0);
#endif
   }

   ::shutdown(m_sock, 2); // Close both input and output

   SockHelp::close( m_sock );

   m_currentState = CLOSED;
}

bool
TCPSocket::listenDuration( uint16 port, uint32 duration, bool bindSocket ) {
   bool opened = false;
   uint32 startTime = TimeUtility::getCurrentTime();
   while ( !opened && 
           startTime + duration*1000/*ms*/ > TimeUtility::getCurrentTime() ) {
      opened = true;
      if ( !open() ) {
         opened = false;
      }
      if ( opened ) {
         uint16 foundPort = listen( port, GENERIC, bindSocket );
         if ( foundPort != port ) {
            opened = false;
         }
      }
      if ( !opened ) {
         close();
         usleep( 100 ); // 0.1ms
      }
   }

   return opened;
}

uint16
TCPSocket::listen(uint16 portnumber, TCPPortChoice pc,
                  bool bindSocket)
{
   mc2dbg4 << "TCPSocket::listen(xxx)" << endl;
   
   struct sockaddr_in sin;

   int nbrTries;

   if (pc == FINDFREEPORT) {
      nbrTries = NetUtility::MAX_NBR_TRIES_PORTFIND;
   } else {
      nbrTries = 1;
   }

   while (nbrTries > 0 && bindSocket) {
      mc2dbg4 << "   TCPSocket: Trying port = " << portnumber << endl;

      // Reuse the port as quickly as possible and not wait for timeouts
      // but not when FINDFREEPORTing
      unsigned int on = 1; // true
      if ( pc != FINDFREEPORT ) {
         if ( setsockopt( m_sock, SOL_SOCKET, SO_REUSEADDR,
                          (char*)&on, sizeof(on) ) < 0 )
         {
            mc2log << warn 
                   << "TCPSocket::listen -> setsockopt(REUSEADDR): "
                   << strerror(errno) << endl;
         }
      }

      // The socket should send data as soon as it arrives
      /*if (setsockopt(m_sock, SOL_TCP, TCP_NODELAY,
                     (char*)&on, sizeof(on)) < 0 ) {
         DEBUG1(cerr << "TCPSocket::listen -> setsockopt(NODELAY): "
                << strerror(errno) << endl;);
      }*/

      memset(&sin, 0, sizeof(sin));
      sin.sin_family = AF_INET;
      sin.sin_port = htons((u_short)portnumber);

      if (::bind( m_sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
#ifndef _MSC_VER
         if ( (errno == EADDRINUSE) && (FINDFREEPORT) ) {
#else
         if(true) {
#endif
            // increase the portnumber and try again!
            portnumber++;
            nbrTries--;
            mc2dbg4 << "TCPSocket::listen: Port already in use "
                    << " trying the next one (" << portnumber 
                    << ")" << endl;
         } else {
            PANIC("TCPSocket::listen() (bind) failure: ", strerror(errno));
         }
      } else {
         // Done -- port OK!
         nbrTries = -1;
      }
   }

   if (nbrTries == 0) {
      mc2dbg2 << "TCPSocket, __bind: No free port found!" << endl;
      portnumber = 0; // return 0 if no free port
   }

   if (portnumber != 0 && ::listen(m_sock, m_backlog) == SOCKET_ERROR) {
      PANIC("TCPSocket::listen() failure: ", strerror(errno));
   }

   if (portnumber > 0)
      m_currentState = LISTEN;

   return portnumber;
}

TCPSocket*
TCPSocket::accept()
{
   mc2dbg4 << "TCPSocket::accept()" << endl;

   SOCKET tmpsock = internalAccept();
   if ( tmpsock != 0 ) {
      return new TCPSocket( tmpsock, DEFAULT_BACKLOG );
   } else {
      return NULL;
   }
}

TCPSocket*
TCPSocket::accept(uint32 micros)
{
   mc2dbg4 << "TCPSocket::accept(micros = " << micros << ")" << endl;
   
   fd_set rfds;
   struct timeval tv;
   FD_ZERO(&rfds);
   FD_SET(m_sock, &rfds);
   tv.tv_sec = micros / 1000000;
   tv.tv_usec = micros % 1000000;
#ifndef _MSC_VER
   select(getdtablesize(), &rfds, NULL, NULL, &tv);
#else
   select(0, &rfds, NULL, NULL, &tv);
#endif

   if(!FD_ISSET(m_sock, &rfds)) {
      return NULL;   // Timeout before connection attempt
   }
   
   return (accept()); // WARNING: This will block if the connection is removed
   // by an error between select and ::accept.
   
   /*
   SOCKET tmpsock;

   if ((tmpsock = ::accept(m_sock, (struct sockaddr *)0, 0)) == 
       SOCKET_ERROR) 
   {
      DEBUG1(cerr << "TCPSocket::accept() failure: " <<  strerror(errno)
             << endl;);
      return NULL;  
   }

   return new TCPSocket(tmpsock);
   */
}

bool
TCPSocket::connect(const char *hostname, int portnumber)
{
   mc2dbg4 << "TCPSocket::connect(hostname = " << hostname
           << ", portnumber = " << portnumber << ")" << endl; 

   if ( m_sock == 0 ) {
      DEBUG1(cerr << "TCPSocket::connect - m_sock == 0." << endl);
      return false;
   }
      
   struct sockaddr_in sin;

   memset(&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_port = htons((u_short)portnumber);

   if ( ! NetUtility::gethostbyname( hostname, sin.sin_addr ) ) {
#ifdef __SVR4
      sin.sin_addr.s_addr = inet_addr(hostname);
      if (inet_addr(hostname) < 0 ) {
#else
      if ((sin.sin_addr.s_addr = inet_addr(hostname)) == INADDR_NONE) {
#endif
         DEBUG1(cerr << "TCPSocket::connect():sys_inet_addr failure: "
                << strerror(errno) << endl;
                cerr << "Could not connect to " << hostname
                << ":" << portnumber
                << "\nSocket will now be closed."
                << endl; );
         SockHelp::close( m_sock );
         m_currentState = CLOSED;
         return (false);
      }
   }

   return connect(sin);
}

bool TCPSocket::connect(const char *addr, int portnumber, uint32 timeout_us)
{
   SockHelp::UnBlock unblock( *this, m_blocking );
   
   // both connect and connect-test must be true
   return TCPSocket::connect(addr, portnumber) ? 
      waitForConnect(timeout_us) : false;
}

bool TCPSocket::connect(const IPnPort& addr, uint32 timeout_us) {
   SockHelp::UnBlock unblock( *this, m_blocking ); 
                  
   // both connect and connect-test must be true
   return TCPSocket::connect( addr.getIP(), addr.getPort() ) ? 
      waitForConnect( timeout_us ) : false;
}

bool
TCPSocket::connect(uint32 hostip, uint16 hostport)
{
   mc2dbg4 << "TCPSocket::connect(hostip = " << hostip
           << ", hostport = " << hostport << ")" << endl;
   
   if ( m_sock == 0 ) {
      mc2dbg << "TCPSocket::connect - m_sock == 0." << endl;
      return false;
   }
   
   struct sockaddr_in sin;

   memset(&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_port = htons((u_short)hostport);
   sin.sin_addr.s_addr = htonl(hostip);

   mc2dbg8 << "Trying to connect to ("
           << hostip << ", " << hostport << ")" << endl;

   return connect(sin);
  
}

bool TCPSocket::connect(struct sockaddr_in &sin) {

   if (::connect(m_sock, (struct sockaddr *)&sin, sizeof(sin)) == 0) {
      m_currentState = CONNECTED;
      setupSocket();
      return (true);
   } else {
#ifndef _MSC_VER
      if ( m_blocking || ((errno != EINPROGRESS ) && (errno != EALREADY ) ) ) {
#else
      if ( m_blocking || ((errno != WSAEINPROGRESS ) && (errno != WSAEALREADY ) ) ) {
#endif
         SockHelp::close( m_sock );
         m_currentState = CLOSED;
         return (false);
      } else {
         mc2dbg2 << "[TCPSocket]: Nonblocking connect not finished "
            " - returning true " << endl;
         return true;
      }
   }
}

ssize_t 
TCPSocket::read( byte* buffer, size_t size ) {
   mc2dbg4 << "TCPSocket::read(buffer = " << buffer 
           << " size = " << size << endl;
   if ( m_sock == 0 ) {
      mc2log << warn << "TCPSocket::read - socket == 0" << endl;
      return -1;
   }

   // Check if socket has cached data to fullfill read.
   if ( checkForCachedData( buffer, size ) ) {
      return size;
   }

   // If the socket is closed by the other side during transmission, we
   // receve the SIGPIPE-signal that normaly leads to program exit.
   // But now this signal is ignord during reading --> read() returns
   // -1 if this happens.      
   SysUtility::ignorePipe();

   ssize_t readLength = protectedRead( buffer, size );

   // Let the default handler take care of the SIGPIPE-signal now when
   // data read.
   SysUtility::setPipeDefault();

   if ( readLength < 0 ) { 
      // Some error check errno
      if ( errno == EPIPE ) { // Closed, this should not happen on read
         readLength = 0;
      } else if ( errno == EAGAIN) { // try again later, select first
         readLength = -3;
      } // Else leave it as -1
   }

   return readLength;
}


ssize_t 
TCPSocket::read( byte* buffer, size_t size, uint32 timeout ) {
   mc2dbg4 << "TCPSocket::read(buffer = " << buffer 
           << " size = " << size << " timeout" << timeout << endl;
   if ( m_sock == 0 ) {
      mc2log << warn << "TCPSocket::read w.timeout - socket == 0" << endl;
      return -1;
   }
   
   // Check if socket has cached data to fullfill read.
   if ( checkForCachedData( buffer, size ) ) {
      return size;
   }

   fd_set rfds;
   struct timeval tv;
   int retval;

   /* Add our fd to the set */
   FD_ZERO( &rfds );
   FD_SET( m_sock, &rfds );

   /* Put the microseconds into tv */
   tv.tv_sec = timeout / 1000000;
   tv.tv_usec = timeout % 1000000;
   
   /* Do the selection */
#ifndef _MSC_VER
   retval = select( getdtablesize(), &rfds, NULL, NULL, &tv );
#else
   retval = select( 0, &rfds, NULL, NULL, &tv );
#endif
   if ( retval < 0 ) {
      mc2log << error << "TCPSocket::read(byte*, size_t, uint32) failed"
             << endl;
      perror( "Last system error" );
   }
   if ( FD_ISSET( m_sock, &rfds ) ) {
      // Hope this works.
      retval = read( buffer, size );
   } else {
      /* Timeout */
      retval = -2;
   }

   return retval;
}


ssize_t
TCPSocket::readExactly(byte *buffer,
                       size_t length,
                       long micros,
                       long totalMicros)
{
   mc2dbg4 << "TCPSocket::readExactly(buffer = " << buffer
           << "length = " << length << endl;
   
   if ( m_sock == 0 ) {
      mc2log << warn << "TCPSocket::readExactly - socket == 0" << endl;
      return -1;
   }

   int totalRead = 0;
   if (micros != TCPSOCKET_INFINITE_TIMEOUT ||
       totalMicros != TCPSOCKET_INFINITE_TIMEOUT) {
      totalRead = readMinMaxBytesWithTimeout
         (buffer, length, length, micros, totalMicros);
   } else {

      // If the socket is closed by the other side during transmission, we
      // receve the SIGPIPE-signal that normaly leads to program exit.
      // But now this signal is ignord during reading --> read() returns
      // -1 if this happens.      
      SysUtility::ignorePipe();
      
      size_t readLength = protectedRead(buffer, length);
      
      totalRead = readLength;
      int offset = 0;
      mc2dbg8 << "readlength = " << readLength << ", length = " 
              << length << endl;
         while (readLength > 0 && (uint32) totalRead < length) {
            mc2dbg8 << "readlength = " << readLength << ", length = " 
                    << length << endl;
            offset = totalRead;
            
            readLength = protectedRead(buffer + offset, (length-totalRead));

            if (readLength > 0)
               totalRead += readLength;
            else
               totalRead = readLength;
         }
      
      // Let the default handler take care of the SIGPIPE-signal now when
      // data read.
      SysUtility::setPipeDefault();
      
      if (totalRead <= 0)
         return (-1);
   } // if INFINITE_TIMEOUT
   return totalRead;
}

ssize_t
TCPSocket::readMinMaxBytesWithTimeout(byte* buffer,
                                      size_t minLength,
                                      size_t maxLength,
                                      long micros,
                                      long totalMicros)
{
   mc2dbg4 << "TCPSocket::readMinMaxBytes(buffer = " << buffer
           << ", minLength = " << minLength
           << ", maxLength = " << maxLength
           << ", micros = " << micros
           << ", totalMicros = " << totalMicros << ")"
           << endl;

   // granularity is milliseconds.
   long stopTime =
      TimeUtility::getCurrentTime() +
      totalMicros / 1000; // in milliseconds
   
   long timeLeftMicros = micros;

   // Check if we have a total timeout.
   if ( totalMicros != TCPSOCKET_INFINITE_TIMEOUT ) {
      if ( micros == TCPSOCKET_INFINITE_TIMEOUT ) {
         // Only count down.
         timeLeftMicros = totalMicros;
      } else {
         // Start by waiting micros if it is smaller.
         // This way we will wait micros for the first byte
         // and then count down the time until everything is received.
         timeLeftMicros = MIN(micros, totalMicros);
      }
   }

   ssize_t totalBytesRead = 0;
   ssize_t currentBytesRead = 0;

   // Do until timeout here or in readMaxBytes or we have read everything.
   do {
      currentBytesRead = readMaxBytes(buffer + totalBytesRead,
                                      maxLength - totalBytesRead,
                                      timeLeftMicros);
      if (currentBytesRead > 0) {
         totalBytesRead += currentBytesRead;
      }
      // Check if we have a total timeout.
      if ( totalMicros != TCPSOCKET_INFINITE_TIMEOUT ) {
         // Now we have received some bytes or nothing.
         // Next time we will use the total timeout instead.
         // If we haven't received any we will exit the loop.
         timeLeftMicros = (stopTime - TimeUtility::getCurrentTime()) * 1000;
      } else {
         // Total time left is the same as micros all the time
         // if we don't have a total-timeout.
         timeLeftMicros = micros;
      }
   } while ((timeLeftMicros > 0) &&
            (totalBytesRead < (int) minLength) &&
            (currentBytesRead > 0));
   
   ssize_t result = totalBytesRead;
   if ((currentBytesRead == 0) ||
       (currentBytesRead == -2))
   {
      // last read timed out
      result = -2;
   } else if ((currentBytesRead < 0) ||
              (totalBytesRead < (int) minLength))
   {
      // some other error occured
      result = -1;
   }

   DEBUG2(
      if (result <= 0) {
         cerr << "\n readMinMaxBytes was insuccessful:"
              << "\n currentBytesRead = " << currentBytesRead
              << "\n totalBytesRead = " << totalBytesRead
              << "\n minLength = " << minLength
              << "\n maxLenght = " << maxLength
              << "\n micros = " << micros
              << "\n totalMicros = " << totalMicros
              << "\n timeLeftMicros = " << timeLeftMicros
              << "\n result = " << result
              << endl;
         perror("Last system error is");
      } );
   return result;
}

ssize_t
TCPSocket::readMaxBytes(byte* buffer,
                        size_t size,
                        uint32 micros)
{
   mc2dbg4 << "TCPSocket::readMaxBytes(buffer = " << buffer
           << ", size = " << size << ", micros = " << micros << ")"
           << endl;
   
   
   // Check th fd
   if ( m_sock == 0 ) {
      MC2ERROR("TCPSocket::readMaxBytes(byte*, size_t, uint32) m_sock ==0");
      return -1;
   }
   
   // Check if socket has cached data to fullfill read.
   if ( checkForCachedData( buffer, size ) ) {
      return size;
   }

   fd_set rfds;
   struct timeval tv;
   int retval;

   /* Add our fd to the set */
   FD_ZERO(&rfds);
   FD_SET(m_sock, &rfds);

   /* Put the microseconds into tv */
   tv.tv_sec = micros / 1000000;
   tv.tv_usec = micros % 1000000;
   
   /* Do the selection */
#ifndef _MSC_VER
   retval = select(getdtablesize(), &rfds, NULL, NULL, &tv);
#else
   retval = select(0, &rfds, NULL, NULL, &tv);
#endif
   if ( retval < 0 ) {
      DEBUG1(cerr << "TCPSocket::readMaxBytes(byte*, size_t, uint32) failed\n");
      perror("Last system error");
   }
   if ( FD_ISSET(m_sock, &rfds) ) {
      // Hope this works.
      // If readMaxBytes gets interrupted and there is nothing
      // to read the next time we try it will not. Then we should
      // select again maybe? It shouldn't happen though.
      size_t len = readMaxBytes(buffer, size);
      if ( len == 0 ) // Disconnected (EOF)
         return -1;
      else
         return len;
   } else {
      /* Timeout */
      return -2;
   }
}

ssize_t
TCPSocket::protectedWrite( const byte* buffer,
                         size_t size )
{
   size_t writtenLength;
#ifndef _MSC_VER
#  ifdef EINTR
   do { // Write until the error isn't EINTR
      writtenLength = ::write(m_sock, buffer, size);
      if ( writtenLength < 0 && errno == EINTR ) {
         MC2WARNING("TCPSocket::protectedWrite - EINTR - trying again");
      }
   } while ( writtenLength < 0 && errno == EINTR );
#  else
   writtenLength = ::write(m_sock, buffer, size);
#  endif

#else // IT IS _MSC_VER
   writtenLength = ::send(m_sock, (char *)buffer, size, 0);
#endif
   return writtenLength;
}


bool 
TCPSocket::checkForCachedData( byte* buffer, size_t size ) {
   return false;
}

SOCKET
TCPSocket::internalAccept() {
   SOCKET tmpsock;

   if ( (tmpsock = ::accept(m_sock, (struct sockaddr *)0, 0)) == 
        SOCKET_ERROR ) 
   {
      if ( errno == EAGAIN ) {
         // Not any sockets in queue, try again after a select
         mc2dbg2 << "TCPSocket::accept() EAGAIN: No socket right now."
                 << endl;
      } else {
         mc2log << error << "TCPSocket::accept() failure: " 
                << strerror( errno ) << endl;
      }
      return 0;
   }

   return tmpsock;
}

ssize_t
TCPSocket::readUntilClosed(byte *buffer,
                           size_t size  )
{

   ssize_t totalBytesRead = 0;
   ssize_t nbrRead;
   while ((nbrRead = readMaxBytes(buffer, size)) > 0) {
      totalBytesRead += nbrRead;
      buffer += nbrRead;
      size -= nbrRead;
   }
  
   return (totalBytesRead);
}

ssize_t 
TCPSocket::write( const byte *buffer, size_t length ) {
   mc2dbg4 << "TCPSocket::write(buffer = " << buffer
           << ", length = " << length << ")" << endl;
   if ( m_sock == 0 ) {
      mc2log << error << "TCPSocket::write - m_sock == 0" << endl;
      return -1;
   }

   // If the socket is closed by the other side during transmission, we
   // receve the SIGPIPE-signal that normaly leads to program exit.
   // But now this signal is ignord during writing --> write() returns
   // -1 if this happens.
   // No effect in MSC_VER.
   SysUtility::ignorePipe();

   int writeLength = protectedWrite( buffer, length );

   // Let the default handler take care of the SIGPIPE-signal now when
   // data written.
   // No effect in MSC.
   SysUtility::setPipeDefault();

   if ( writeLength < 0 ) { 
      // Some error check errno
      if ( errno == EPIPE ) { // Closed
         writeLength = 0;
      } else if ( errno == EAGAIN) { // try again later, select first
         writeLength = -3;
      } // Else leave it as -1
   }

   return writeLength;
}


ssize_t 
TCPSocket::write( const byte *buffer, size_t length, 
                  uint32 timeout )
{
   mc2dbg4 << "TCPSocket::write(buffer = " << buffer
           << ", length = " << length << ", timeout = " << timeout << ")"
           << endl;
   if ( m_sock == 0 ) {
      mc2log << error << "TCPSocket::write - m_sock == 0" << endl;
      return -1;
   }

   fd_set wfds;
   struct timeval tv;
   int retval;
   int writeLength = 0;

   /* Add our fd to the set */
   FD_ZERO( &wfds );
   FD_SET( m_sock, &wfds );

   /* Put the microseconds into tv */
   tv.tv_sec = timeout / 1000000;
   tv.tv_usec = timeout % 1000000;
   
   /* Do the selection */
#ifndef _MSC_VER
   retval = select( getdtablesize(), NULL, &wfds, NULL, &tv );
#else
   retval = select( 0, NULL, &wfds, NULL, &tv );
#endif
   if ( retval < 0 ) {
      mc2log << warn << "TCPSocket::write(byte*, size_t, uint32) "
             << "select failed" << endl;
      perror( "Last system error" );
   }
   if ( FD_ISSET( m_sock, &wfds ) ) {
      // Hope this works.
      writeLength = write( buffer, length );
   } else {
      /* Timeout */
      writeLength = -2;
   }

   return writeLength;
}


ssize_t
TCPSocket::writeAll( const byte *buffer,
                     size_t length  )
{
   mc2dbg4 << "TCPSocket::writeAll(buffer = " << buffer
           << ", length = " << length << ")" << endl;

   if ( m_sock == 0 ) {
      mc2log << error << "TCPSocket::writeAll - m_sock == 0" << endl;
      return -1;
   }
   

   // If the socket is closed by the other side during transmission, we
   // receve the SIGPIPE-signal that normaly leads to program exit.
   // But now this signal is ignord during writing --> write() returns
   // -1 if this happens.
   // No effect in MSC_VER.
   SysUtility::ignorePipe();

   int writeLength = protectedWrite(buffer, length );


   int totalWrite = writeLength;

   mc2dbg4 << "totalWrite=" << totalWrite << ", writeLength=" 
           << writeLength << ", length=" << length << endl;

   while ( ( totalWrite > -1 ) && ( (uint32) totalWrite < length) ) {

      mc2dbg4 << "totalWrite=" << totalWrite << ", writeLength=" 
              << writeLength << endl;

      mc2dbg4 << "Before write, (length-totalWrite)=" 
              << (length-totalWrite) << endl;

      writeLength = protectedWrite( (buffer + totalWrite), 
                                    length-totalWrite );

      // FIXME: Is this correct? What happens if we have written
      //        40 bytes and we want to write 80. Then we don't
      //        know how many we wrote.
      if (writeLength > -1)
         totalWrite += writeLength;
      else
         totalWrite = writeLength;
   }
   mc2dbg4 << "END totalWrite=" << totalWrite << ", writeLength=" 
           << writeLength << ", length=" << length << endl;

   // Let the default handler take care of the SIGPIPE-signal now when
   // data written.
   // No effect in MSC.
   SysUtility::setPipeDefault();

   if (totalWrite < 0) {
      mc2log << warn << "TCPSocket::write RETURN -1"
             << strerror(errno) << endl;
      return (-1);
   }
   return totalWrite;
}


bool
TCPSocket::getSockName(uint32& IP, uint16& port) {
   mc2dbg4 << "TCPSocket::getSockName(IP = " << IP
           << ", port = " << port << ")" << endl;
   
   if ( m_sock == 0 ) {
      mc2dbg4 << "TCPSocket::getSockName failure: NO SOCKET!" 
              << endl;
      return false;
   }

   struct sockaddr_in addr; 
#if defined(_WIN32) || defined(__CYGWIN32__)
   int size = sizeof(struct sockaddr_in);
#else
   uint32 size = sizeof(struct sockaddr_in);
#endif

   if ( getsockname( m_sock, 
                     (struct sockaddr *)&addr, 
                     (&size) ) == 0 ) {
      port = ntohs(addr.sin_port);
      IP = ntohl(addr.sin_addr.s_addr);
      return true;
   } else {
      mc2dbg4 << "TCPSocket::getSockName failure: " << strerror(errno)
              << endl;
      return false;
   }
}

bool
TCPSocket::getPeerName(uint32& IP, uint16& port) const {
   mc2dbg4 << "TCPSocket::getPeerName(IP = " << IP
           << ", port = " << port << ")" << endl;
   
   if ( m_sock == 0 ) {
      mc2dbg4 << "TCPSocket::getPeerName failure: NO SOCKET!" 
              << endl;
      return false;
   }

   struct sockaddr_in addr; 
#if defined(_WIN32) || defined(__CYGWIN32__)
   int size = sizeof(struct sockaddr_in);
#else
   uint32 size = sizeof(struct sockaddr_in);
#endif

   if ( getpeername( m_sock, 
                     (struct sockaddr *)&addr, 
                     (&size) ) == 0 ) {
      port = ntohs(addr.sin_port);
      IP = ntohl(addr.sin_addr.s_addr);
      return true;
   } else {
      mc2dbg4 << "TCPSocket::getPeerName failure: " << strerror(errno)
              << endl;
      return false;
   }
}

IPnPort
TCPSocket::getPeer() const
{
   IPnPort addr;
   if ( getPeerName( addr.first, addr.second ) ) {
      return addr;
   } else {
      // errore
      return IPnPort(0,0);
   }
}

int32
TCPSocket::readString(char* target, uint32 maxstringlength)
{
   mc2dbg4 << "TCPSocket::readString(target = " << target
           << ", maxstringlength = " << maxstringlength << ")" << endl;
   
   int nbr;
   uint32 nbrRead = 0;
   char c = '\0';
   
   nbr = readExactly((byte*)&c, 1, TCPSOCKET_DEFAULT_TIMEOUT);
   DEBUG8(cerr << "read #" << c << "#" << endl;);
   while ( (nbr == 1) && (c != '\0') && (c != '\n') && // \n debug
           (nbrRead < maxstringlength) ) { 
      // More to read and not end of string
      target[nbrRead++] = c;
      nbr = readExactly((byte*)&c, 1, TCPSOCKET_DEFAULT_TIMEOUT);
      DEBUG8(cerr << "read #" << c << "#" << endl;);
   }
   
   if ( (nbr == 1) && ( (c == '\0') || (c == '\n') ) ) { 
      // Has read a string ; \n is for debug
      target[nbrRead] = '\0';
      return nbrRead;
   } else {
      target[nbrRead] = '\0';
      return -1;
   }
}


bool
TCPSocket::setBlocking(bool blocking)
{
   if ( blocking ) {
#ifndef _MSC_VER
      // This might be right: fcntl( m_sock, F_SETFL, 0 );
      int res = fcntl( m_sock, F_SETFL, O_SYNC );
#else
      unsigned long arg = 0;
      int res = ioctlsocket(m_sock, FIONBIO, &arg);      
#endif
      if ( res >= 0 ) {
         m_blocking = true;
         return true;
      } else {
         return false;
      }
   } else {
#ifndef _MSC_VER
      int res = fcntl(m_sock, F_SETFL, O_NONBLOCK);      
#else
      unsigned long arg = 1;
      int res = ioctlsocket(m_sock, FIONBIO, &arg);      
#endif
      if ( res >= 0 ) {
         m_blocking = false;
         return true;
      } else {
         return false;
      }
   }
}

int
TCPSocket::readLine(char* target, uint32 maxlinelength)
{
   mc2dbg4 << "TCPSocket::readLine(target = " << target
           << ", maxlinelength = " << maxlinelength << ")" << endl;
   
   int nbr;
   uint32 nbrRead = 0;
   char c;

   nbr = readExactly((byte*)&c, 1, TCPSOCKET_DEFAULT_TIMEOUT);
   mc2dbg8 << "read #" << c << "#" << endl;
   while ( (nbr == 1) && (c != '\r') && (c != '\n') &&
           (nbrRead < maxlinelength) ) { 
      // More to read and not end of line
      target[nbrRead++] = c;
      nbr = readExactly((byte*)&c, 1, TCPSOCKET_DEFAULT_TIMEOUT);
      DEBUG8(cerr << "read #" << c << "#" << endl;);
   }

   // Skip linebreak
   if ( (nbr == 1) && (c == '\r') ) { // Has read CR
      nbr = readExactly((byte*)&c, 1, TCPSOCKET_DEFAULT_TIMEOUT); // Then read the LF
      mc2dbg8 << "read #" << c << "#" << endl;
   }
   target[nbrRead] = '\0';
   mc2dbg8 << "Line: " << target << endl;
   if ( nbr != 1 ) {
      if ( nbr != 0 ) { // EOF is ok, return this last line
         nbrRead = nbr; // return error
      }
   }
   return nbrRead;
}

TCPSocket::TCPSocketState
TCPSocket::getState() const
{
   mc2dbg4 << "TCPSocket::getState()" << endl;
   
   return (m_currentState);
}


const char*
TCPSocket::getStateAsString() const
{
   switch (m_currentState) {
   case (UNKNOWN) :
      return ("UNKNOWN");
   case (OPEN) :
      return ("OPEN");
   case (LISTEN) :
      return ("LISTEN");
   case (CONNECTED) :
      return ("CONNECTED");
   case (CLOSED) :
      return ("CLOSED");
   }
   return ("UNHANDLED STATE");
}


bool TCPSocket::waitForConnect(uint32 timeout_us) {
   DebugClock clock;

   // no need to check connect() if we did not get any error
   if ( errno == 0 ) {
      return true;
   }

   bool success = true;

   // if there was an error and not EINPROGRESS, we did not succeed
   // mark success false so we shutdown the socket later
   if ( errno != EINPROGRESS ) {
      success = false;
   }

   //
   // else we are still connecting, use select to determine 
   // whether connect() completed successfully or not with
   // a timeout
   //

   // loop until we get an error or 
   // the sock SO_ERROR returns successful
   while ( success ) {
      //
      // Setup select with timeout
      //
      struct timeval tv; 
      tv.tv_sec = timeout_us / 1000000;
      tv.tv_usec = timeout_us % 1000000;
      fd_set theSet;
      FD_ZERO(&theSet);
      FD_SET(m_sock, &theSet);

      int result = select(m_sock + 1, NULL, &theSet, NULL, &tv);

      if (result < 0 && result != EINTR) {
         mc2dbg << "TCPSocket::connect(): Error " << strerror( result ) << endl;
         success = false;
      } else if (result > 0 && FD_ISSET( m_sock, &theSet ) ) {

         int socket_error;
         socklen_t valSize = sizeof ( int );
         // using getsockopt to read SO_ERROR option 
         // so we can determine if whether connect() completed successfully or not
         if (getsockopt(m_sock, SOL_SOCKET, SO_ERROR, 
                        &socket_error, &valSize ) < 0) {
            mc2dbg << "TCPSocket::connect(): Error in getsockopt. " 
                   << strerror( errno ) << endl;
            success = false;
         } 

         // still successfull...?
         if (success) {
            if ( socket_error != 0 ) {
               mc2dbg << "TCPSocket::connect(): Error "
                      << strerror( socket_error ) << endl;
               success = false;
            } else {
               m_currentState = CONNECTED;
               // hurray! We have a connection, 
               // end loop please. :)
               success = true;
               break;
            }
         }

      } else {
         // else timeout...
         mc2dbg<<"TCPSocket::connect(): timed out!"<<endl;
         success = false;         
      }
   }

#ifdef ADD_STEPS
   addStepTime( "Connect", clock );
#endif

   if (!success) {

      // close socket if we failed to make a connection
      SockHelp::close( m_sock );
      m_currentState = CLOSED;
   }

   return success;
}
