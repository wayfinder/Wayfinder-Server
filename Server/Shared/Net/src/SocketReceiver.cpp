/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "TCPSocket.h"
#include "DatagramSocket.h"
#include "SocketReceiver.h"
#include "CCSet.h"

namespace SocketReceiverPrivate {
   /**
    *   Notice class used to store Datagram/TCP sockets in lists.
    */
   class Notice : public Link {
   public:
      /** 
       *   Create a notice object for a Socket.
       *   @param   sock  Socket to associate with notice object.
       */
      Notice( Socket* sock ) {
         m_sock = sock;
      }
      
      
      /**
       * Destructor. Doesn't delete socket.
       */
      virtual ~Notice() { }
      
      
      /** 
       *   @return The socket associated with notice object.
       */
      Socket* getSocket() {
         return m_sock;
      }
      
      
   private:
      /**
       *   The socket associated with this notice object.
       */
      Socket* m_sock;
   };
   
   
   /**
    *   Describes one way of notify the receive-method.
    */
   class NotifyNotice : public Link {
   public:
      /**
       *   Definition of the possible notify-pipes to force the select
       *   to "wake up".
       */
      enum notify_t {
         /** 
          *   Used when to force a reselection in the select-method.
          *   If the attribute terminate is set, the select-method will
          *   return false.
          */
         RESELECT,
         
         /**
          *    Used to make the select-method exit with false return value.
          */
         TIMEOUT
      };

      /** 
       *   Create a notice object for a filedescriptor
       *   @param   listenFD    The listen filedescriptor.
       *   @param   writeFD     The write filedescriptor.
       *   @param   type        The type of notification that 
       *                        is done when using this fd.
       */
      NotifyNotice(int listenFD, int writeFD, notify_t type) {
         m_fd = listenFD;
         m_writefd = writeFD;
         m_type = type;
      }
      
      
      /**
       *   Delete this NotifyNotice.
       */
      virtual ~NotifyNotice() {;}
      
      /** 
       *   @return The listen-fd associated with notify 
       *           notice object.
       */
      int getfd() {
         return (m_fd);
      }
      
      
      /**
       *   @return  The fd where to write something when to notify.
       */
      int getWritefd() {
         return (m_writefd);
      }
      
      
      /**
       *   @return  The type of notification when using this fd.
       */
      notify_t getNotifyType() {
         return (m_type);
      }
      
      
   private:
      /**
       *   The fd associated with this notify notice object.
       */
      int m_fd;
      
      
      /**
       *   The fd where to write when notify.
       */
      int m_writefd;
      
      
      /**
       *   The type of notification.
       */
      notify_t m_type;
   };
}

using namespace SocketReceiverPrivate;

SocketReceiver::SocketReceiver() {
   m_notifyList = new Head;
   m_datagramList = new Head;
   m_tcpList = new Head;
   m_tcpWriteList = new Head;
   m_selected = false;
   m_terminated = false;
   m_lastNotice = NULL;
   // Create the RESELECT-notify
#ifndef _WIN32
   // RESELECT PIPE
   int reselectfds[2];
   int err = pipe( reselectfds );
   if ( err != 0 ) {
      mc2log << fatal << "SocketReceiver::SocketReceiver failed to create "
             << "RESELECT pipe " << strerror(errno) << endl;
      abort();
   }

   // Set the pipes to non-blocking. If it's full we're already notified.
   for(int i=0; i < 2; ++i ) {
      int res = fcntl( reselectfds[i], F_SETFL, O_NONBLOCK );
      if ( res < 0 ) {
         mc2log << fatal << "SocketReceiver::SocketReceiver failed to set "
                << "RESELECT pipe to nonblock" << strerror(errno) << endl;
         abort();
      }
   }
   // Put notice in list.
   NotifyNotice *n = new NotifyNotice(reselectfds[0],
                                      reselectfds[1],
                                      NotifyNotice::RESELECT);
   n->into( m_notifyList );

   // TIMEOUT PIPE
   int timeoutfds[2];
   err = pipe( timeoutfds );
   if ( err != 0 ) {
      mc2log << fatal << "SocketReceiver::SocketReceiver failed to create "
             << "TIMEOUT pipe " << strerror(errno) << endl;
      abort();
   }
   // Set to nonblocking.
   for(int i=0; i < 2; ++i ) {
      int res = fcntl( timeoutfds[i], F_SETFL, O_NONBLOCK );
      if ( res < 0 ) {
         mc2log << fatal << "SocketReceiver::SocketReceiver failed to set "
                << "RESELECT pipe to nonblock" << strerror(errno) << endl;
         abort();
      }
   }   
   // Put notice in list
   NotifyNotice* t = new NotifyNotice( timeoutfds[0],
                                       timeoutfds[1],
                                       NotifyNotice::TIMEOUT );
   t->into( m_notifyList );
#else 
#error IMPLEMENT RESELECT-notify! in SocketReceiver.
#endif
}


SocketReceiver::~SocketReceiver() {
   mc2dbg4 << "~SocketReceiver" << endl;
   terminate();

   // Close all pipes
   NotifyNotice* nnotice = static_cast<NotifyNotice *>( 
      m_notifyList->first() );
   while ( nnotice != NULL ) {
      close( nnotice->getfd() );
      close( nnotice->getWritefd() );
      nnotice = static_cast<NotifyNotice*>( nnotice->suc() );
   }

   delete m_notifyList;
   delete m_datagramList;
   delete m_tcpList;
   delete m_tcpWriteList;
}

void
SocketReceiver::terminate()
{
   mc2dbg4 << "SocketReceiver::terminate()" << endl;
   m_terminated = true;
   forceRestart();
}


void
SocketReceiver::addDatagramSocket( DatagramReceiver *sock ) {
   mc2dbg4 << "AddDatagramsocket, start" << endl;
   Notice* notice = new Notice( reinterpret_cast<Socket*> ( sock ) );
   notice->into( m_datagramList );
   MC2_ASSERT( notice->getSocket()->getSOCKET() != -1 );
   forceRestart();
}


void
SocketReceiver::removeDatagramSocket( DatagramReceiver *sock ) {
   Notice *notice = static_cast<Notice *>( m_datagramList->first() );
   while(notice != NULL) {
      if ( notice->getSocket() == reinterpret_cast<Socket*> ( sock ) ) {
         notice->out();
         if ( m_lastNotice == notice ) {
            m_lastNotice = NULL;
         }
         delete notice;
         notice = NULL;
      }
      else {
         notice = static_cast<Notice *>( notice->suc() );
      }
   }
   forceRestart();
}


void
SocketReceiver::addTCPSocket(TCPSocket *sock, bool norestart ) {
   DEBUG2(uint32 ip; uint16 port);
   DEBUG2(sock->getPeerName(ip, port));
   DEBUG2(mc2dbg << "addTCPSocket, port = " << port << ", ip = " << ip 
          << endl);
   Notice* notice = new Notice( sock );
   MC2_ASSERT( notice->getSocket()->getSOCKET() != -1 );
   notice->into( m_tcpList );
   if ( ! norestart ) {
      forceRestart();
   }
}


void
SocketReceiver::removeTCPSocket(TCPSocket *sock) {
   Notice* notice = static_cast<Notice *>( m_tcpList->first() );
   while(notice != NULL) {
      if( notice->getSocket() == sock ) {
         notice->out();
         if ( m_lastNotice == notice ) {
            m_lastNotice = NULL;
         }
         delete notice;
         notice = NULL;
      }
      else {
         notice = static_cast<Notice *>( notice->suc() );
      }
   }
   forceRestart();
}

void
SocketReceiver::addWriting( TCPSocket* sock )
{
   removeWriting( sock );
   Notice* notice = new Notice( sock );
   notice->into( m_tcpWriteList );
   MC2_ASSERT( notice->getSocket()->getSOCKET() != -1 );
   forceRestart();
}

void
SocketReceiver::removeWriting(TCPSocket* sock )
{
   Notice* notice = static_cast<Notice*>(m_tcpWriteList->first() );
   while ( notice != NULL ) {
      if ( notice->getSocket() == sock ) {
         mc2dbg8 << "[SockRecv]: Found socket" << endl;
         notice->out();
         if ( m_lastWriteNotice == notice ) {
            m_lastWriteNotice = NULL;
         }
         delete notice;
         notice = NULL;
      } else {
         notice = static_cast<Notice *>( notice->suc() );
      }      
   }
   forceRestart();
}

Socket*
SocketReceiver::getSocketThatReceived() {
   if ( m_lastNotice != NULL ) {
      return m_lastNotice->getSocket();
   } else {
      return NULL;
   }
}


bool
SocketReceiver::select( int32 micros, 
                        TCPSocket*& tcpSock, 
                        DatagramReceiver*& datagramSock )
{
   const int maxPipeMessageSize = 512;
#ifndef _WIN32
   int nfds = getdtablesize();
#else
   int nfds = 0; // Ignored in WIN32
#endif
   
   tcpSock = NULL;
   datagramSock = NULL;

   // Loop until returned (from inside the loop) or until the
   // SocketReceiver is terminated
   while ( !m_terminated ) {
      if( m_selected ) {
         // Something has happend on any of the sockets/pipes
         m_lastWriteNotice = NULL;
         m_lastNotice      = NULL;
         
         // Check all the datagram sockets for messages
         Notice* notice = NULL;
         notice = static_cast<Notice *>( m_datagramList->first() );
         while (notice) {
            if ( FD_ISSET(reinterpret_cast<DatagramReceiver *>(notice->
                           getSocket())->getSOCKET(), &m_readfds) ) {
               FD_CLR(reinterpret_cast<DatagramReceiver *>(notice->
                           getSocket())->getSOCKET(), &m_readfds);
               m_lastNotice = notice;
               datagramSock = 
                  reinterpret_cast<DatagramReceiver *>( notice->getSocket() );
               return true;
            }
            notice = static_cast<Notice *>( notice->suc() );
         }

         // Check all the reading TCP-sockets for messages
         notice = static_cast<Notice *>( m_tcpList->first() );
         while (notice) {
            if (FD_ISSET(static_cast<TCPSocket *>(notice->getSocket())
                         ->getSOCKET(), &m_readfds) ) 
            {
               FD_CLR(static_cast<TCPSocket *>(notice->getSocket())
                      ->getSOCKET(), &m_readfds);
               m_lastNotice = notice;
               tcpSock = static_cast<TCPSocket *>( notice->getSocket() );
               return true;
            }
            notice = static_cast<Notice *>( notice->suc() );
         }

         // Check the writing TCP-sockets for messages.
         notice = static_cast<Notice *>( m_tcpWriteList->first() );
         while (notice) {
            if (FD_ISSET(static_cast<TCPSocket *>(notice->getSocket())
                         ->getSOCKET(), &m_writefds) ) 
            {
               FD_CLR(static_cast<TCPSocket *>(notice->getSocket())
                      ->getSOCKET(), &m_writefds);               
               m_lastWriteNotice = notice;
               tcpSock = static_cast<TCPSocket *>( notice->getSocket() );
               return true;
            }
            notice = static_cast<Notice *>( notice->suc() );
         }
         
         
         // Check the notify-pipes for messages.
         NotifyNotice* nnotice = static_cast<NotifyNotice *>( 
            m_notifyList->first() );
         while (nnotice) {
            if ( FD_ISSET(nnotice->getfd(), &m_readfds)) {
               FD_CLR(nnotice->getfd(), &m_readfds);
               switch (nnotice->getNotifyType()) {
                  case NotifyNotice::RESELECT :
                  case NotifyNotice::TIMEOUT :
                  {
                     // Clear pipe
                     byte tmpBuf[maxPipeMessageSize];
                     // Save the number of read bytes if debugging...
                     int nbrRead = 1;
                     int totRead = 0; // For debugging
                     // Read until empty. Remember that the fd is nonblock.
                     while ( nbrRead > 0 ) {
                        nbrRead = read(nnotice->getfd(), 
                                       tmpBuf, 
                                       maxPipeMessageSize);
                        totRead += ((nbrRead > 0) ? nbrRead : 0);
                     }
                     mc2dbg4 << "SocketReceiver: RESELECT/TO, read " 
                             << totRead << " bytes" << endl;
                     
                     if ( nnotice->getNotifyType() ==
                          NotifyNotice::RESELECT ) {
                        // We do not have to do anything!
                     } else {
                        // Exit as if timeout
                        return false;
                     }
                  };
                  break;
               }
            }
            nnotice = static_cast<NotifyNotice*>( nnotice->suc() );
         }
         m_selected = false;
      } else {
         // No socket/pipe was selected.
         FD_ZERO( &m_readfds );
         FD_ZERO( &m_writefds );
         Notice* notice = NULL;

         // Register all the datagram receivers for select
         notice = static_cast<Notice*>( m_datagramList->first() );
         while ( notice ) {
            mc2dbg8 << "Adding UDP-socket" << endl;
            FD_SET(reinterpret_cast<DatagramReceiver *>( notice->
               getSocket())->getSOCKET(), &m_readfds);
            notice = static_cast<Notice *>( notice->suc() );
         }

         // Register the reading TCP sockets for select
         notice = static_cast<Notice *>( m_tcpList->first() );
         while(notice) {
            mc2dbg8 << "Adding TCP-socket" << endl;
            FD_SET(static_cast<TCPSocket *>( notice->getSocket())
                   ->getSOCKET(), &m_readfds);
            notice = static_cast<Notice *>( notice->suc() );
         }

         // Register the writing TCP sockets for select
         notice = static_cast<Notice *>( m_tcpWriteList->first() );
         while(notice) {
            mc2dbg2 << "[SockRec]: Adding writing TCP-socket" << endl;
            FD_SET(static_cast<TCPSocket *>( notice->getSocket())
                   ->getSOCKET(), &m_writefds);
            notice = static_cast<Notice *>( notice->suc() );
         }

         // Register all the notify pipes for select
         NotifyNotice* nnotice = static_cast<NotifyNotice *>( 
            m_notifyList->first() );
         while (nnotice != NULL) {
            FD_SET(nnotice->getfd(), &m_readfds);
            nnotice = static_cast<NotifyNotice *>( nnotice->suc() );
         }

         // Perform the select-call
         int status;
         if (micros >= 0) {
            // With timeout
            struct timeval tv;
            tv.tv_sec = micros / 1000000;
            tv.tv_usec = micros % 1000000;
            status = ::select(nfds, &m_readfds, 
                              &m_writefds, (fd_set *)0, &tv);
         } else {
            // Without any timeout (wait forever).
            status = ::select(nfds,
                              &m_readfds,
                              &m_writefds,
                              (fd_set *)0, 
                              NULL);
         }

         if (status < 0) {
            // Failure
            mc2log << error
                   << "SocketReceiver::receive() select failure: "
                   << strerror(errno) << endl;
            // It must be examined why this can happen.
            MC2_ASSERT( errno != EBADF );
            return false;
         } else if( status == 0 ) {
            // Nothing happend on the selected sockets (timeout !?)
            return false;
         }
         m_selected = true;
      }
   }

   // Terminated!
   mc2dbg1 << "SocketReceiver::receive terminated!!!" << endl;
   return false;
}


void
SocketReceiver::forceRestart() 
{
   mc2dbg4 << "SocketReceiver:: forceRestart" << endl;
   NotifyNotice* nnotice = static_cast<NotifyNotice *> (
      m_notifyList->first() );
   while ((nnotice != NULL) &&
          (nnotice->getNotifyType() != NotifyNotice::RESELECT)) {
      mc2dbg8 << "      LOOP" << endl;
      nnotice = static_cast<NotifyNotice*>( nnotice->suc() );
   }
   if (nnotice != NULL) {
      const char* str = "Reselect";
      mc2dbg4 << "Writing to pipe!!!" << endl;
      write(nnotice->getWritefd(), str, strlen(str)); 
   }
}


void 
SocketReceiver::forceTimeout() {
   mc2dbg4 << "SocketReceiver:: forceTimeout" << endl;
   NotifyNotice* nnotice = static_cast<NotifyNotice *> (
      m_notifyList->first() );
   while ((nnotice != NULL) &&
          (nnotice->getNotifyType() != NotifyNotice::TIMEOUT)) {
      mc2dbg8 << "      LOOP" << endl;
      nnotice = static_cast<NotifyNotice*>( nnotice->suc() );
   }
   if (nnotice != NULL) {
      const char* str = "Timeout";
      mc2dbg4 << "Writing to pipe!!!" << endl;
      write(nnotice->getWritefd(), str, strlen(str)); 
   }
}
