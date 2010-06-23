/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DATAGRAMSOCKET_H
#define DATAGRAMSOCKET_H

#include "config.h"
#include "Socket.h"
#include "Packet.h"

class IPnPort;

/*
#ifdef _WIN32
   #include <winsock.h>
   #include "MC2String.h"
   #define socklen_t int 
#else
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <netdb.h>
   #include "MC2String.h"
   #include <errno.h>
   #include <sys/time.h>
   #include <unistd.h>
#endif
*/

// No multicast, make sure all IPs are 127.0.0.1
// #define SINGLE_VERSION

/**
 *    DatagramSocket.
 *
 */
class DatagramSocket : public Socket {
public:
   virtual selectable getSelectable() const;

   protected:
      /**
        *   Create a new DatagramSocket. Declaired protected to avoid
        *   any instances of this class!
        */
      DatagramSocket();

      /**
        *   Delete this datagramsender and release the allocated memory.
        */
      virtual ~DatagramSocket();


   protected:
      /**
       *  The filedescriptor of this socket.
       */
      int fd;

      /**
       *   The protocol number for UDP.  Statically  initialized
       *   if _MFC_VER is not  defined. Visual C++ needs winsock
       *   to be initialized before getprotobyname can be called
       *   but in linux we have a problem with getprotobyname
       *   when called in different threads at the same time
       */
      static int c_udp_proto_nbr;
};

/**
 *    Objects of this class is to be used when sending mc2-packets or
 *    other data via UDP.
 *
 */
class DatagramSender : public DatagramSocket {
   public:
      /**
        *   Create a new DatagramSender.
        */
      DatagramSender();

      /**
        *   Delete this datagramsender and release the allocated memory.
        */
      virtual ~DatagramSender();

      /**
        *   Send one packet in one UDP to the given address.
        *   @param   packet   The packet to send.
        *   @param   ip       The ip-address to send the packet to.
        *   @param   port     The port number at ip to send the packet to.
        */
      bool send(const Packet *packet, uint32 ip, uint16 port);

      /**
       *    Send one packet in one UDP to the supplied address.
       *    @param packet   The packet to send.
       *    @param destAddr The destination address of the packet.
       */
      bool send(const Packet* packet, const IPnPort& destAddr);
      
      /**
       * Sends a buffer of max max_udp_size.
       * @param buffer Is the byte buffer to send.
       * @param buffSize Is the number of bytes in buffer to send,
       *        maximun Packet::MAX_UDP_PACKET_SIZE long.
       * @param ip The IP adress to send to.
       * @param port The port number to send to.
       */
      bool send( byte* buffer, uint32 buffSize, uint32 ip, uint16 port );

   private:
      /**
       * Sends a header and data part.
       * @param header Pointer to header of Packet, HEADER_SIZE long.
       * @param data Pointer to data of Packet.
       * @param ip The IP adress to send to.
       * @param port The port number to send to.
       */
      bool send( byte* header, byte* data, uint32 dataSize, 
                 uint32 ip, uint16 port );
};

/**
 *    Socket to be used for receiving messages.
 *
 */
class DatagramReceiver : public DatagramSocket {
   public:
      /**
       * The possible choices when creating a DatagramReceiver:
       * \begin{description}
       *    \item[GENERIC]       ``Ordinary bind'' to the port
       *    \item[REUSEADDR]     Set the {\tt SO_REUSEADDR}-flag before
       *                         binding the socket to allow several
       *                         processes to use the same port, useful
       *                         when running several modules on the
       *                         same machine that should listen to the
       *                         same multicast address
       *    \item[FINDFREEPORT]  Tries to find a free port
       * \end{description}
       */
      enum PortChoice {GENERIC, REUSEADDR, FINDFREEPORT};

      /** 
        *   
        * @param  port  The portnumber the socket uses
        * @param  pc    Optional parameter that sets the 
        *               behavior when creating (and specially
        *               binding) the socket. Possible values and
        *               their meaning are listed by {\tt PortChoice}
        * @see    PortChoice
        */
      DatagramReceiver( uint16 port,
                        PortChoice pc = GENERIC);

      /**
        *   Delete this Datagram receiver and release the allocated memory.
        */
      virtual ~DatagramReceiver();

      /**
        *   Fill a Packet with data from the network. This method hangs
        *   until any UDP is received.
        *   @param   packet   The packet where the data will be inserted.
        *   @return  True if data is inserted into the packet, false 
        *            otherwise.
        */
      bool receive(Packet* packet);

      /**
       *    Receives an UDP packet into buff with maxSize bbuffSize.
       *
       *    @param buff     The buffer to write packet into.
       *    @param buffSize The size of buff.
       *    @return Number bytes received. < 0 means error
       */
      int32 receive( byte* buff, uint32 buffSize );

      /**
        *   Fill a Packet with data from the network. This method hangs
        *   until any UDP is received or the time-out is reached.
        *   @param   packet   The packet where the data will be inserted.
        *   @param   micros   The maximum waittime for the UDP.
        *   @return  True if data is inserted into the packet, false 
        *            otherwise (probably time-out).
        */
      bool receive(Packet* packet, uint32 micros);

      /**
       *    Receives an UDP packet into buff with maxSize bbuffSize.
       *
       *    @param buff     The buffer to write packet into.
       *    @param buffSize The size of buff.
       *    @param micros   The max time to wait for a packet, in 
       *                    microseconds.
       *    @return Number bytes received. < 0 means error
       */
      int32 receive( byte* buff, uint32 buffSize, uint32 micros );

      /**
        *   Used to make this receiver join a multicast group.
        *   @param   ip The ip of the multicast group to join.
        *   @return  True if joined group with ip, false otherwise.
        */
      bool joinGroup(uint32 ip);

      /**
        *   Get the filedescriptor of this socket.
        *   @return  The filedescriptor of this receiver.
        */
      int getSOCKET();
      
      /**
        *   Get the portnumber that this receiver listens on.
        *   @return  The portnumber that this socket use.
        */
      uint16 getPort();

      /**
       *    Returns the IP-address and port of the last received packet.
       *    @param IP   Outparameter that is set to the IP-address of 
       *                the last received packet.
       *    @param port Outparameter that is set to the port of the last
       *                received packet.
       *    @return     True if the IP and port contains valid data,
       *                false otherwise.
       */
      bool getPeerName(uint32& IP, uint16& port);

      /**
       *    Removes all current data on the socket. 
       */
      void clear();
      
   private:
      /**
        *   The portnumber used by this socket.
        */
      uint16 port;


      /**
       * The address of the last received packet.
       * Is set to contain only zeros in constructor and before each receive.
       */
      struct sockaddr_in m_peerAddr;
};



inline DatagramSocket::selectable
DatagramSocket::getSelectable() const {
   return fd;
}


#endif //DATAGRAMSOCKET_H

