/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PUSHSERVICES_H
#define PUSHSERVICES_H

#include "config.h"
#include "PushService.h"
#include <map>


// A map of serviceID and PushServices
typedef map<uint32,PushService*> PushServiceMap;


/**
 * Class as interface to PushService handling.
 *
 */
class PushServices {
   public:
      /**
       * Create a new PushServices.
       */
      PushServices();


      /**
       * Decontruct this PushServices.
       */
      virtual ~PushServices();


      /**
       * Add a PushService.
       * 
       * @param service The PushService to add. The object is owned by 
       *                this class after the call to this method.
       * @param packetList The list of packets to add packets that needs
       *                   to be sent to.
       * @param lastUpdateTime For all resources the lastUpdateTime is 
       *                       added to this vector.
       */
      void addPushService( PushService* service,
                           PacketContainerList& packetList,
                           vector<uint32>& lastUpdateTime );


      /**
       * Removes a PushService entierly.
       *
       * @param serviceID The ID of the PushService to remove.
       * @param packetList The list of packets to add packets that needs
       *                   to be sent to.
       * @return True if serviceID did exist, false if it didn't.
       */
      bool removePushService( uint32 serviceID,
                              PacketContainerList& packetList );


      /**
       * Removes a certain resource from a PushService.
       * The PushService is removed entierly if there remains no 
       * resourceIDs in it.
       *
       * @param serviceID The ID of the PushService to remove.
       * @param resource The resource to stop having push for.
       * @param packetList The list of packets to add packets that needs
       *                   to be sent to.
       * @return True if resource did exist, false if it didn't.
       */
      bool removePushServiceResource( uint32 serviceID, 
                                      SubscriptionResource& resource,
                                      PacketContainerList& packetList  );


      /**
       * Handle a received push packet.
       *
       * @param pushPacket The push packet to handle.
       * @param pushSocket The socket that the pushPacket was received on.
       * @param packetList The list of packets to add packets that needs
       *                   to be sent to.
       * @param isDataPacket Set to true if the pushPacket is a data 
       *                     packet.
       * @param serviceID  Set to the serviceID of the pushPacket if it is
       *                   a data packet.
       * @param resource Set to the resource of the pushPacket if it is a
       *                 data packet or NULL if not.
       */
      void handlePushPacket( PushPacket* pushPacket, TCPSocket* pushSocket,
                             PacketContainerList& packetList,
                             bool& isDataPacket,
                             uint32& serviceID, 
                             SubscriptionResource*& resource );


      /**
       * Handle a broken socket.
       *
       * @param pushSocket The socket that is broken.
       * @param packetList The list of packets to add packets that needs
       *                   to be sent to.
       */
      void handleBrokenSocket( TCPSocket* pushSocket, 
                               PacketContainerList& packetList );


      /**
       * Make all packets that has to be sent and produce a new timeout.
       *
       * @param packetList The list of packets to add packets that needs
       *                   to be sent to.
       * @return Time for next timeout.
       */
      uint32 checkAndCalculateTimeout( PacketContainerList& packetList );


      /**
       * Prints all data on stream.
       *
       * @param out The stream to print onto.
       */
      void dump( ostream& out ) const;


   private:
      /**
       * The services
       */
      PushServiceMap m_services;
};


#endif // PUSHSERVICES_H


