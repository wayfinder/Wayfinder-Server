/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKETRECEIVER_H
#define PACKETRECEIVER_H

#include "config.h"
#include "SocketReceiver.h"

#include <set>

class DatagramReceiver;
class Packet;
class TCPSocket;

/** 
  *   Class to receive a packet from Datagram- or TCPsockets.
  *   The functions are thread unsafe.
  *
  */
class PacketReceiver {
   public:
      /**
        *   Create a new packet receiver.
        */
      PacketReceiver();

      /**
        *   Delete this packet reciever.
        */
      virtual ~PacketReceiver();

      /**
        *   Free possible selections.
        */
      void terminate();

      /** 
        *   Receive next incoming packet.
        *   @param   micros   Time in microseconds before timeout. 
        *                     If < 0 then wait forever.
        *   @param   tcpReceiverSock
        *                     Pointer to pointer to receiversocket
        *                     if it still exists. Will not be set if null.
        *   @return A new Packet or NULL if read error.
        */
      Packet* receiveAndCreatePacket(int32 micros,
                                     TCPSocket** tcpReceiverSock = NULL);

      /** 
        *   Add a DatagramReceiver to list of active receivers.
        *   @param   sock  The datagram receiver that should be added
        *                  to the receiverlist.
        */
      void addDatagramSocket(DatagramReceiver *sock);

      /** 
        *   Remove a DatagramReceiver from list of active receivers.
        *   @param   The datagram-socket to remove.
        */
      void removeDatagramSocket(DatagramReceiver *sock);

      /** 
        *   Add a TCPSocket to list of active receivers.
        *   @param   sock  The TCPSocket that should be added
        *                  to the receiverlist.
        */
      void addTCPSocket(TCPSocket *sock);

      /** 
        *   Remove a TCPSocket from list of active receivers.
        *   @param   The TCP-socket to remove.
        */
      void removeTCPSocket(TCPSocket *sock);

      /**
       *    Adds a socket to the list of active receivers
       *    that should not be disconnected after a packet
       *    has been received.
       */
      void addPermanentTCPSocket(TCPSocket* sock);
      
      /**
        *   The socket that received last.
        *   @return pointer to the last socket that recived. May be
        *           DatagramReceiver or TCPSocket pointer.
        */
      void *getSocketThatReceived();

      /**
        *   Force the receive to restart (reselect).
        */
      void forceRestart();

      /**
       * Read and create a packet from readSocket.
       * 
       * @param readSocket The socket to read packet from.
       * @param timeout    Timeout in microseconds.
       * @return The Packet or NULL if fatal read error.
       */
      static Packet* readAndCreatePacket( TCPSocket* readSocket,
                                          uint32 timeout = MAX_UINT32 );


      /**
       * Read a packet data from readSocket.
       * 
       * @param readSocket The socket to read packet from.
       * @param bufferLength The length of the byte buffer.
       * @param packetLength The number of bytes used by the packet in 
       *                     buffer. Is less than or equal to bufferLength.
       * @param timeOut      Timeout to use. Socket will be closed if it is
       *                     exceeded.
       * @return The buffer with the packet or NULL if fatal read error.
       */
      static byte* readPacket( TCPSocket* readSocket, 
                               uint32& bufferLength, 
                               int& packetLength,
                               uint32 timeout = MAX_UINT32);


   private:
      /**
       *    Returns true if the TCPSocket is a permanent one.
       */
      bool isPermanent(TCPSocket* sock);
      
      /**
       * The socketreceiver.
       */
      SocketReceiver m_receiver;

      /**
       *   Set of permanent sockets.
       */
      set<TCPSocket*> m_permanentSockets;

};

#endif

