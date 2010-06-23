/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __READER_H__
#define __READER_H__

#include "config.h"

#include "ISABThread.h"
#include "MapBits.h"
#include <set>

class LeaderStatus;
class MapSafeVector;

/**
 * Abstract base class for readers, see StandardReader (and its subclasses)
 * for implementation.
 * 
 * A reader is used for handling incoming messages to a module. Different
 * behaviour depending on whether the module is leader or not.
 * The voting procedure and all control-messages (the CTRL_PACKET_BIT
 * is set) are taken care of here. 
 */
class Reader : public ISABThread {
public:
   virtual ~Reader() {}
   
   /**
    *    Return true if leader
    *   @return true if this module is a leader
    */
   virtual bool isLeader() const = 0;
   
   /**
    * Returns an object which can be queried about 
    * the leader/available-status.
    * @see LeaderStatus
    */
   virtual const LeaderStatus* getLeaderStatus() const = 0;
   
   /**
    *    Return the number of leader/available votes since startup
    *    @return number of votes
    */
   virtual uint32 getNbrVotes() = 0;

   /**
    *    Return the number of processed requests since startup
    *    @return number of requests
    */
   virtual uint32 getNbrRequests() = 0;

   /**
    *    Return the rank of this module
    *    @return the module's rank
    */
   virtual int32 getRank() = 0;
   
   /**
    *    Set the rank of this module
    *    @param newRank the module's new rank
    */
   virtual void setRank(int32 newRank) = 0;

   /**
    *    Initiate a vote
    */
   virtual void vote() = 0;
   
   /**
    * A last vote (not send via PacketSenderReceiver) used when shutting 
    * down reader
    */
   virtual void lastVote() = 0;

   /**
    *   Initializes the leader (retreives all available maps 
    *   from the MapModule).
    */
   virtual void initLeader(set<MapID>& allMaps) = 0;
   
   /**
    *    Get the name of this module.
    *    @return  The name of this module taken from the module type.
    */
   virtual const char* getName() = 0;

   /**
    *    Returns the vector of loaded maps. 
    */
   virtual MapSafeVector* getLoadedMaps() = 0;

   /**
    * Set "start as leader" state.
    * @param state The new state.
    */
   virtual void setStartAsLeader( bool state ) = 0;
   
   /**
    *    Returns the length of the queue to the Processor.
    */
   virtual int getQueueLength() = 0;   
};


#endif // __READER_H__
