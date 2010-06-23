/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERREADER_H
#define USERREADER_H


#include "config.h"
#include "Types.h"
#include "StandardReader.h"
#include "Packet.h"


/** 
 *    Handles user controlpackets such as writes to the database. 
 * 
 */
class UserReader : public StandardReader {
public:
      /**
       *   Creates an MapReader according to the specifications.
       *   
       *   @param   jobQueue       The queue where to insert the request 
       *                           to be  processed by the processor.
       *   @param   sendQueue      The queue for sending packets
       *   @param   loadedMaps     Where to look for the currently loaded
       *                           maps.
       *   @param   packetReaderThread
       *   @param   port
       *   @param   definedRank    Value used when voting for leader. High
       *                           value indicated that the system 
       *                           administrator want this Module to be
       *                           leader.
       */
      UserReader(Queue *jobQueue,
                 NetPacketQueue& sendQueue,
                 MapSafeVector* loadedMaps,
                 PacketReaderThread* packetReader,
                 uint16 port,
                 uint32 definedRank );

   
      /**
       *    Destructor.
       */
      ~UserReader();


protected:
      /**
       *   Take care of some of the special controllpackets that might
       *   be received by the UserModule-leader. Calls the 
       *   leaderProcessCtrlPacket
       *   in the super class to take care of the ``ordinary'' control
       *   packet.
       *
       *   @param   p  The incomming control packet.
       *   @return  True if the packet was handled by this method or
       *            by the method in the superclass, false otherwise.
       */
      bool leaderProcessCtrlPacket(Packet *p);

      /**
       *   Send a request packet to the best module. This might
       *   be this module. The packet is either deleted or put in
       *   a queue.
       *   @param   request  The packet to send.
       **/
      virtual void sendRequestPacket(RequestPacket* request);
      
      /**
       *    Reimplemented to do initial database check when 
       *    first becoming leader.
       */
      virtual void becomeLeader();
      
      /**
       *    UserModule has no maps so this is empty!
       */
      virtual void dumpMapData() {}

      /**
       * Called by reader periodically, overriden to make checks from
       * time to time.
       */
      virtual void periodicMethod();

private:   
   /**
    * Sends a periodic request to this module.
    */
   void sendPeriodicToMyself();
};

#endif

