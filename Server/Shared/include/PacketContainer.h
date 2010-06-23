/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKETCONTAINER_H
#define PACKETCONTAINER_H

#include "config.h"

#include "ModuleTypes.h"
#include "ScopedArray.h"
#include "TimeoutSequence.h"

class Packet;

/**
 * Contains a packet and information about it.
 *
 */
class PacketContainer {
   public:
      /// The default resend timeout time in ms.
      static const uint32 defaultResendTimeoutTime = 5000;
   

      /// The minimun resend timeout 
      static const uint32 minResendTimeout = 1000;


      /// The default number of resends
      static const uint32 defaultResends = 3;

      /**
       * Creates a new container for the packet.
       * @param thePacket The Packet to hold.
       * @param serverTimestamp The initial timestamp.
       * @param serverResend The initial resend count.
       *        Sets resendNbr in packet
       * @param type The type of the destination module.
       * @param resendTimeoutSequence Gives the time between resends 
       *                              of the packet.
       * @param nbrResends The number of times thePacket is resent.
       * @param mapSet Force use of a certain map/module set, ignored if
       *               (default) value MAX_UINT32 is passed
       */
      PacketContainer( Packet* thePacket, 
                       uint32 serverTimestamp,
                       byte serverResend,
                       moduletype_t type,
                       TimeoutSequence resendTimeoutSequence = defaultResendTimeoutTime,
                       uint32 nbrResends = defaultResends,
                       uint32 mapSet = MAX_UINT32);


      /**
       * Simple constructor, creates a new container for the packet.
       * @param thePacket is the Packet to hold.
       */
      PacketContainer( Packet* thePacket );
      

      /** 
       * Destructor, deletes the packet.
       */
      virtual ~PacketContainer();
      
      /// Inspect the packet and set mapSet accordingly if enabled
      void checkMapSet();
      
      /**
       * Function for retreiving the packet contained in this 
       * PacketContainer.
       * @return The packet of this container.
       */
      inline Packet* getPacket() const;
      
      
      /**
       * Sets the last sent time.
       * @param ts timestamp in micros
       */
      inline void setServerTimestamp( uint32 ts );
      
      
      /** 
       * The last sent time.
       * @return timestamp in micros
       */
      inline uint32 getServerTimestamp() const;

      /**
       *   Gets the absolute time when the packet times out.
       */
      inline uint32 getTimeoutAbsTime() const;
      
      /** 
       * Set the packet resend counter.
       * Sets resendNbr in packet.
       * @param count number of retries
       */
      void setServerResend( uint16 count );
   

      /**
       * Return the packet resend counter.
       * @return number of times packet is resend
       */
      byte getServerResend() const;
   

      /** 
       * Set the packet resend timeout. Is never less than 
       * minResendTimeout. Always a fix value, never a sequence.
       * @param the new resend timeout.
       */
      inline void setResendTimeout( uint32 newResend );
      

      /**
       * Return the timeout before the next resend.
       * Note that the timeout can vary between each resend.
       *
       * @return the resend timeout in ms.
       */
      inline uint32 getNextResendTimeout() const;
   

      /**
       * Return the packets nbrResend.
       * @return the number of times the packet may be resent.
       */
      inline uint32 getNbrResend() const;


      /** 
       * Set the packets nbrResend.
       * @param newNbrResend is the new number of resends.
       */
      inline void setNbrResend( uint32 newNbrResend );


      /** 
       * Set the moduletype to process this packet 
       */
      inline void setModuleType( moduletype_t type );
   

      /** 
       * Get the moduletype to process this packet 
       */
      inline moduletype_t getModuleType() const;


      /**
       * Adds a answer packet. May be a partiall packet (Split).
       * @param pack The answer packet.
       */
      void addAnswer( Packet* pack );

   
      /**
       * Returns true if all parts of the answer has been received.
       */
      bool answerComplete() const;


      /**
       * Returns the answer.
       */
      PacketContainer* getAnswer();


      /**
       * Resets the partial answer vectors and sets the resendNbr in packet
       * to current server_resend.
       */
      void resetAnswerData();


      /**
       * Returns the sendTo-IP.
       * SendTo-IP is only used if ModuleType is INVALID.
       * @return The IP to send to.
       */
      inline uint32 getSendIP() const;


      /**
       * Sets the sendTo-IP.
       * SendTo-IP is only used if ModuleType is INVALID.
       * @param IP The IP to send to.
       */
      inline void setSendIP( uint32 IP );
 

      /**
       * Returns the sendTo-Port.
       * SendTo-Port is only used if ModuleType is INVALID.
       * @return The port to send to.
       */
      inline uint16 getSendPort() const;


      /**
       * Sets the sendTo-Port.
       * SendTo-Port is only used if ModuleType is INVALID.
       * @param port The port to send to.
       */
      inline void setSendPort( uint16 port );
      

      /**
       * Get the IP and Port to send packet to.
       * Uses ModuleType unless it is INVALID then uses 
       * sendIP and sendPort.
       *
       * @param IP Set to the IP to send packet to.
       * @param port Set to the port to send packet to.
       */
      void getIPAndPort( uint32& IP, uint16& port ) const;

      /**
       *    Returns a copy of the container to return
       *    when the packet times out.
       */
      PacketContainer* newTimeoutContainer() const;

      /**
       *    Gives a packet to the container. To be used
       *    when a timout occurds.
       */
      void putTimeoutPacket( Packet* p );

private:
      /**
       * Initializes the answer vectors acording to pack.
       */
      void initializeAnswerData( Packet* pack );

      
      /**
       * Adds an answer to answer data.
       * @param pack Is the partial answer to add.
       * @return true if all answers are received, false otherwise.
       */
      bool addPartialAnswer( Packet* pack );


      /**
       * Makes a complete answer from partial answers.
       */
      void makeAnswer();


      /** Packet of this container */
      Packet* m_packet;


      /** Timestamp used by the server. */
      uint32 m_server_timestamp;
   

      /// The resend timeout sequence (in ms).
      TimeoutSequence m_resendTimeoutSequence;

   
      /// The nbr of resends for this packetcontainer.
      uint32 m_nbrResend;
   

      /** Type of module to process this packet */
      moduletype_t m_moduleType;

   
      /** The AnswerPacketContainer */ 
      PacketContainer* m_answer;
   

      /** The answers in a vector */
      Packet** m_answers;

      /** The answer to return if the packet times out */
      Packet* m_timeoutPacket;

      /** The number of answers to expect */
      uint32 m_nbrPackets;


      /** The current number of received packets */
      uint32 m_packetNbr;


      /** The received answer packets */
      ScopedArray<bool> m_received;


      /** The Specific IP-address to send to, usually not used. */
      uint32 m_sendIP;

      
      /** The Specific port to send to, usually not used. */
      uint16 m_sendPort;

      /// The number of the map set
      uint32 m_mapSet;

};


// inlines


Packet* 
PacketContainer::getPacket() const {
   return m_packet;
}


void
PacketContainer::setServerTimestamp( uint32 ts ) {
   m_server_timestamp = ts; 
}


uint32
PacketContainer::getServerTimestamp() const {
   return m_server_timestamp;
}

uint32 
PacketContainer::getNextResendTimeout() const {
   return m_resendTimeoutSequence.getTimeout(getServerResend());
}

uint32
PacketContainer::getTimeoutAbsTime() const
{
   return getServerTimestamp() + getNextResendTimeout();
}


void
PacketContainer::setModuleType( moduletype_t type ) {
   m_moduleType = type;
}


moduletype_t
PacketContainer::getModuleType() const {
   return m_moduleType;
}

uint32
PacketContainer::getSendIP() const {
   return m_sendIP;
}


void 
PacketContainer::setSendIP( uint32 IP ) {
   m_sendIP = IP;
}


uint16
PacketContainer::getSendPort() const {
   return m_sendPort;
}


void 
PacketContainer::setSendPort( uint16 port ) {
   m_sendPort = port;
}


void 
PacketContainer::setResendTimeout(uint32 newResend) {
   if ( newResend > minResendTimeout )
      m_resendTimeoutSequence = newResend;
   else 
      m_resendTimeoutSequence = minResendTimeout;
}

uint32 
PacketContainer::getNbrResend() const {
   return m_nbrResend;
}


void 
PacketContainer::setNbrResend( uint32 newNbrResend ) {
   m_nbrResend = newNbrResend;
}


#endif // PACKETCONTAINER_H 
