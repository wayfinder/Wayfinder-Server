/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LEADERIPREQUESTPACKET_H
#define LEADERIPREQUESTPACKET_H

// This packet is a little more important than others
#define LEADER_IP_REQUEST_PRIO (DEFAULT_PACKET_PRIO - 1)

#include "config.h"
#include "Packet.h"
#include "IPnPort.h"

/**   Packet used for requesting the ip-number and the port
 *    number for a Leader-module, to be able to send TCP to
 *    the Leader module.
 *      
 */
class LeaderIPRequestPacket : public RequestPacket {
public:

   /** 
    *    Creates a LeaderIPRequestPacket.
    *    @param destAddr The address the packet was sent to, e.g. 
    *                    the leader multicast address.
    *
    */
   LeaderIPRequestPacket( uint16 modType, 
                          const RequestPacket& packet,
                          const IPnPort& destAddr );

   /**
    *    Returns the module type.
    */
   uint16 getModuleType() const;
   /// @return true if the request is a ctrl packet
   bool regardingCtrlPacket() const;
   /// @return The address to which the packet was originally sent. Copied into reply
   IPnPort getOriginalDestAddr() const;
};



/** 
 *    Packet that is sent as a reply to a LeaderIPRequestPacket
 *
 *   
 *
 */
class LeaderIPReplyPacket : public ReplyPacket {
public:
   /**
    *    @param  ip      The IP-number of the module suitable to handle the
    *                    requestpacket sent into the LeaderIPRequestPacket.
    *    @param  port    The port-number of said module.
    *    @param  modType The module type.
    *    @param  leader  Address of real leader.
    */
   LeaderIPReplyPacket( const LeaderIPRequestPacket& p,
                        uint32 ip,
                        uint16 port,
                        uint16 modType,
                        const IPnPort& leader );
  
   /**
    *    Returns the IP-number.
    */
   uint32 getIP() const;

   /**
    *   @return the port number.
    */
   uint16 getPort() const;

   /**
    * @return the module type.
    */   
   uint16 getModuleType() const;

   /**
    * @return map id.
    */
   uint32 getMapID() const;

   /// @return leader address
   IPnPort getLeaderAddr() const;
   /// @return The address to which the packet was originally sent. Copied into reply
   IPnPort getOriginalDestAddr() const;
};

#endif 










