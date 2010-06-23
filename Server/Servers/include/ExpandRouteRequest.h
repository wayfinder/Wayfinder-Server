/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPANDROUTEREQUEST_H
#define EXPANDROUTEREQUEST_H

#include "config.h"
#include "Request.h"

#include "StringTable.h"

class RouteReplyPacket;
class RouteExpander;

/**
 * Request for expanding a RouteReply containing lists of IDs to 
 * string and/or coordinates.
 *
 */
class ExpandRouteRequest : public Request {
   public:
      /**
       * Create new expand route request.
       * @param requestID The ID of the request.
       * @param routeReply The RouteReply to expand.
       * @param expandRouteType What types of data to be included in the 
       *        answer.
       * @param language The prefered language, default ENGLISH.
       * @param abbreviate If roadnames should be abbreviated.
       * @param landmarks Should landmarks be added to the result.
       * @param removeAhead Should Aheads be removed if roadname differs?
       * @param nameChangeWP Should road name change force a waypoint?
       */
      ExpandRouteRequest( uint32 requestID,
                          RouteReplyPacket* routeReply,
                          uint32 expandRouteType,
                          StringTable::languageCode language = 
                          StringTable::ENGLISH,
                          bool abbreviate = true,
                          bool landmarks = false,
                          bool removeAhead = false,
                          bool nameChangeWP = false );
      

      /**
       * Delates all allocated data.
       */
      ~ExpandRouteRequest();


      /**
       * Return the next Packet to send.
       *
       * @return Packet if available, NULL otherwise
       */
      virtual PacketContainer* getNextPacket();

      
      /**
       * Process the data returned by a module.
       * @param pack Packet container with the packet that was
       *             processed by the modules.
       */
      virtual void processPacket( PacketContainer* cont );
      
      
      /**
       * Get the answer from this request.
       * @return A packet containing the answer to the request.
       */
      virtual PacketContainer* getAnswer();

      
   private:
      /**
       * The actuall RouteExpander.
       */
      RouteExpander* m_expander;

      
      /**
       * The answer.
       */
      PacketContainer* m_answer;
};


#endif // EXPANDROUTEREQUEST_H
