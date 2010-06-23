/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PUSHRESOURCEDATA_H
#define PUSHRESOURCEDATA_H

#include "config.h"


class TCPSocket;

/**
 * Class for holding information about a PushResource for a specific
 * push service.
 *
 */
class PushResourceData {
   public:
      /**
       * Create a new PushResourceData.
       */
      PushResourceData();

      
      /**
       * Decontruct this PushResourceData.
       */
      virtual ~PushResourceData();


      /**
       * Get the time for the last update for this resource.
       * @return The time for the last update for this resource.
       */
      inline uint32 getTimeForLastUpdate() const;


      /**
       * Get the time for the last heartbeat for this resource.
       * @return The time for the last heartbeat for this resource.
       */
      inline uint32 getTimeForLastHeartbeat() const;


      /**
       * Get the time for the last sent subscription for this resource.
       * @return The time for the last sent subscription for this resource.
       */
      inline uint32 getTimeForLastSentSubscription() const;


      /**
       * Get the last time this resource received something from push
       * provider.
       * @return The last time this resource received something from push
       *         provider.
       */
      inline uint32 getTimeForLastProviderContact() const;


      /**
       * Prints all data on stream.
       *
       * @param out The stream to print onto.
       */
      void dump( ostream& out ) const;


   private:
      /**
       * Set the time for the last update for this resource.
       * @param newTime The time for the last update for this resource.
       */
      inline void setTimeForLastUpdate( uint32 newTime );


      /**
       * Set the time for the last heartbeat for this resource.
       * @param newTime The time for the last heartbeat for this resource.
       */
      inline void setTimeForLastHeartbeat( uint32 newTime );


      /**
       * Set the time for the last sent subscription for this resource.
       * @param newTime The time for the last sent subscription for this 
       *                resource.
       */
      inline void setTimeForLastSentSubscription( uint32 newTime );


      /**
       * Get the socket that provides this resource with push.
       * @return The socket that provides this resource with push.
       */
      inline TCPSocket* getPushSocket() const;

      
      /**
       * Set the socket that provides this resource with push.
       * @param sock The socket that provides this resource with push.
       */
      inline void setPushSocket( TCPSocket* sock );

      
      /**
       * The time of last update for this resource.
       */
      uint32 m_lastUpdateTime;


      /**
       * The time of last heartbeat for this resource.
       */
      uint32 m_lastHeartbeatTime;


      /**
       * The time of last sent subscription for this resource.
       */
      uint32 m_lastSubscriptionTime;


      /**
       * The socket that provides this resource with push.
       */
      TCPSocket* m_pushSocket;


      /**
       * PushService may use private functions.
       */
      friend class PushService;
};


// =======================================================================
//                                 Implementation of the inlined methods =


inline uint32 
PushResourceData::getTimeForLastUpdate() const {
   return m_lastUpdateTime;
}


inline uint32 
PushResourceData::getTimeForLastHeartbeat() const {
   return m_lastHeartbeatTime;
}


inline uint32 
PushResourceData::getTimeForLastSentSubscription() const {
   return m_lastSubscriptionTime;
}


inline uint32 
PushResourceData::getTimeForLastProviderContact() const {
   return MAX( m_lastHeartbeatTime, m_lastUpdateTime );
}


inline void 
PushResourceData::setTimeForLastUpdate( uint32 newTime ) {
   m_lastUpdateTime = newTime;
}


inline void 
PushResourceData::setTimeForLastHeartbeat( uint32 newTime ) {
   m_lastHeartbeatTime = newTime;
}


inline void 
PushResourceData::setTimeForLastSentSubscription( uint32 newTime ) {
   m_lastSubscriptionTime = newTime;
}


inline TCPSocket* 
PushResourceData::getPushSocket() const {
   return m_pushSocket;
}


inline void 
PushResourceData::setPushSocket( TCPSocket* sock ) {
   m_pushSocket = sock;
}


#endif // PUSHRESOURCEDATA_H

