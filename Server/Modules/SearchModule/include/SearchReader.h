/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHREADER_H
#define SEARCHREADER_H

#include "StandardReader.h"

#include "StringTable.h"
#include <sys/types.h>
#include <dirent.h>

/**
  *   Class that specifies the name of the search module
  *
  */
class SearchReader : public StandardReader {
   public :
      /**
       *   Creates a SearchReader according to the specifications.
       *   
       *   @param   q              The queue where to insert the request 
       *                           to be  processed by the processor.
       *   @param   loadedMaps     Where to look for the currently loaded
       *                           maps.
       *   @param   preferredPort  The port to use if possible
       *   @param   leaderIP       IP-address of the leader multicast address.
       *   @param   leaderPort     Port for packets to the leader.
       *   @param   availableIP    IP-address of the available multicast 
       *                           address.
       *   @param   availablePort  Port for packets to the available's.
       *   @param   definedRank    Value used when voting for leader. High
       *                           value indicated that the system 
       *                           administrator want this Module to be
       *                           leader.
       */
      SearchReader(  Queue *q,
                     NetPacketQueue& sendQueue,
                     MapSafeVector* loadedMaps,
                     PacketReaderThread* packetReader,
                     uint16 port,
                     uint32 definedRank );

      /**
       *   Deletes the SearchReader and releases the allocated memory.
       */
      virtual ~SearchReader();


  protected:
      /**
       * Implementation of virtual method in the superclass.
       */
      virtual void printDebugPacketInfo(Packet *p);
      
   /**
    * This module doesn't use country maps.
    */
   virtual bool usesCountryMaps() const { return false; }
};

#endif

