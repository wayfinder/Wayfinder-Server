/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandRouteRequest.h"
#include "RouteExpander.h"
#include "RoutePacket.h"
#include "DisturbanceDescription.h"

ExpandRouteRequest::ExpandRouteRequest( uint32 requestID,
                                        RouteReplyPacket* routeReply,
                                        uint32 expandRouteType,
                                        StringTable::languageCode language,
                                        bool abbreviate,
                                        bool landmarks,
                                        bool removeAhead,
                                        bool nameChangeWP )
      : Request( requestID )
{
   m_answer = NULL;
   m_expander = new  RouteExpander( this, routeReply, expandRouteType,
                                    routeReply->getUTurn(),
                                    false, /* concatinate*/
                                    0, /* no passed roads */
                                    language, abbreviate, landmarks,
                                    removeAhead, nameChangeWP );
   // Disturbances!
   vector<DisturbanceDescription> v =
      routeReply->getDisturbanceDescription();
   if ( v.size() != 0){
      typedef const vector<DisturbanceDescription> cddv;
      for ( cddv::const_iterator it = 
               v.begin() ; it != v.end() ; ++it )
      {
         mc2dbg4 << "split " << it->m_splitPoint << " distPoint " << 
                    it->m_distPoint << " joinPoint " << it->m_joinPoint 
                << " type " << uint32(it->m_type) << " id " 
                << it->m_distID << " comment " << it->m_comment << endl;
      }
      m_expander->addDisturbanceInfo( v );
      // Two as in routing using time and disturbances.
      if ( routeReply->getNbrRouteObjectsUsed() == 2 ) {
         m_expander->setUseAddCost( true );
      }
   }
}


ExpandRouteRequest::~ExpandRouteRequest() {
   delete m_expander;
}


PacketContainer*
ExpandRouteRequest::getNextPacket() {
   return m_expander->getNextRequest();
}


void
ExpandRouteRequest::processPacket( PacketContainer* cont ) {
   m_expander->packetReceived( cont );
   if (m_expander->getDone()) {
      // We're done, create answer. Answer can be null,
      // then something went wrong.
      m_answer = m_expander->createAnswer();
      setDone( true );
   }
   delete cont;
}


PacketContainer*
ExpandRouteRequest::getAnswer() {
   return m_answer;
}
