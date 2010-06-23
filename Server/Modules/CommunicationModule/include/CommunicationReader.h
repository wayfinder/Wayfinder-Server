/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMMUNICATIONREADER_H
#define COMMUNICATIONREADER_H


#include "config.h"
#include "StandardReader.h"


/** 
 * Reader for CommunicationModule.
 * 
 */
class CommunicationReader : public StandardReader {
   public:
      /**
       * Creates an Reader according to the specifications.
       *   
       * @param q           The queue where to insert the request 
       *                    to be  processed by the processor.
       * @param loadedMaps  Where to look for the currently loaded
       *                    maps.
       * @param definedRank Value used when voting for leader. High
       *                    value indicated that the system 
       *                    administrator want this Module to be
       *                    leader.
       */
      CommunicationReader( Queue *q,
                           NetPacketQueue& sendQueue,
                           MapSafeVector* loadedMaps,
                           PacketReaderThread* packetReader, uint16 port,
                           uint32 definedRank );

      /**
       * Destructor.
       */
      ~CommunicationReader();


   protected:
      /**
       * Called by reader periodically, subclasses may override this to
       * make things.
       */
      virtual void periodicMethod();

   private:
};

#endif // COMMUNICATIONREADER_H

