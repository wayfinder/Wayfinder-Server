/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SOCKETRECEIVER_H
#define SOCKETRECEIVER_H

#include "config.h"
#include "NotCopyable.h"

class DatagramReceiver;
class Head;
class Socket;
class TCPSocket;

namespace SocketReceiverPrivate {
   class Notice;
   class NotifyNotice;
}

/** 
  *   Class to select from multiple Datagram- or TCPsockets.
  *
  */
class SocketReceiver: private NotCopyable {
   public:
      /**
        *   Create a new SocketReceiver.
        */
      SocketReceiver();


      /**
        *   Destruct this socket reciever.
        */
      virtual ~SocketReceiver();


      /**
        *   Free possible selections.
        */
      void terminate();


      /** 
        *   Select the .
        *   @param  micros   Time in microseconds before timeout. 
        *                     If < 0 then wait forever.
        *   @param  tcpSock The selected TCPSocket if a TCPSocket
        *           was selected.
        *   @param  udpSocket The selected DatagramSocket if a 
        *           datagramSocket was selected.
        *   @return True if socket has data, false if timeout.
        */
      bool select( int32 micros, 
                   TCPSocket*& tcpSock, DatagramReceiver*& datagramSock );


      /** 
        *   Add a DatagramReceiver to list of active receivers.
        *   @param   sock  The datagram receiver that should be added
        *                  to the receiverlist.
        */
      void addDatagramSocket( DatagramReceiver *sock );


      /** 
        *   Remove a DatagramReceiver from list of active receivers.
        *   @param   The datagram-socket to remove.
        */
      void removeDatagramSocket( DatagramReceiver *sock );


      /** 
        *   Add a TCPSocket to list of active receivers.
        *   @param   sock  The TCPSocket that should be added
        *                  to the receiverlist.
        *   @param   norestart Do not restart the select.
        */
      void addTCPSocket( TCPSocket *sock, bool norestart = false );


      /** 
        *   Remove a TCPSocket from list of active receivers.
        *   @param   The TCP-socket to remove.
        */
      void removeTCPSocket( TCPSocket *sock );

      /**
       *    Adds a socket that should be checked for write-ready.
       *    @param sock The TCP-socket to use for write selection.
       */
      void addWriting( TCPSocket* sock );

      /**
       *    Removes one of the writing sockets.
       *    @param sock The TCP-socket that should be removed from
       *                the list of writing sockets.
       */
      void removeWriting( TCPSocket* sock );
      
      /**
        *   The socket that received last.
        *   @return pointer to the last socket that recived. May be
        *           DatagramReceiver or TCPSocket pointer.
        */
      Socket* getSocketThatReceived();


      /**
        *   Force the receive to restart (reselect).
        */
      void forceRestart();


      /**
       *   Force the receive to time out.
       */
      void forceTimeout();

   void notify() { forceTimeout(); }
   private:
      /** 
        *   List of active notify pipes.
        */
      Head *m_notifyList;


      /** 
        *   List of active Datagram receivers.
        */
      Head *m_datagramList;


      /**
        *   List of active TCP receivers.
        */
      Head *m_tcpList;

      /**
       *    List of active writing TCP sockets.
       */
      Head* m_tcpWriteList;

      /** 
        *   True if select is performed.
        */
      bool m_selected;


      /** 
        *   True if this server is to be terminated!
        */
      bool m_terminated;

      
      /**
        *   The filedescriptor to use to make the select
        *   to wake up.
        */
      int m_notifyFD;
      
      /// The set of lowlevel sockets (reading).
      fd_set m_readfds;
      
   
      /// The set of writing sockets.    
      fd_set m_writefds;

      /**
        *   The last selected notice (reading)
        */
      SocketReceiverPrivate::Notice* m_lastNotice;

      /**
       *    The last selected notice (writing)
       */
      SocketReceiverPrivate::Notice* m_lastWriteNotice;
};


#endif // SOCKETRECEIVER

