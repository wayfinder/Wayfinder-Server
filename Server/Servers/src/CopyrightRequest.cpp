/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CopyrightRequest.h"
#include "Properties.h"
#include "CopyrightBoxPacket.h"

CopyrightRequest::CopyrightRequest( uint32 requestID,
                                    const LangType& language ):
   RequestWithStatus( requestID ),
   m_packetsLeft( 0 ) {

   uint32 mapSetCount = Properties::getUint32Property( "MAP_SET_COUNT",
                                                       MAX_UINT32 );

   m_packetsLeft = 0;

   if ( mapSetCount != MAX_UINT32 ) {
      // Create a copyrightbox request packet for each map set
      for ( uint32 mapSet = 0; mapSet < mapSetCount; ++mapSet ) {
         setupPacketToSend( language, mapSet );
      }
   } else {
      // The single map set
      setupPacketToSend( language, mapSetCount );
   }
}

void CopyrightRequest::setupPacketToSend( const LangType& language, 
                                          uint32 mapSet ) {
   CopyrightBoxRequestPacket* pack =
      new CopyrightBoxRequestPacket( getID(), getNextPacketID(),
                                     language );

   m_packetsReadyToSend.
      add( new PacketContainer( pack, 0, 0,
                                MODULE_TYPE_MAP,
                                PacketContainer::defaultResendTimeoutTime,
                                PacketContainer::defaultResends,
                                mapSet ) );
   m_packetsLeft++;
}

void CopyrightRequest::processPacket( PacketContainer* pack ) {
   if ( m_packetsLeft > 0 ) {
      // merge the copyrights from the module
      m_packetsLeft--;
      CopyrightBoxReplyPacket* reply = 
         static_cast< CopyrightBoxReplyPacket* >( pack->getPacket() );

      CopyrightHolder other;
      reply->getCopyrightHolder( other );
      m_copyrights.merge( other );
      
   } 

   delete pack;

   // no packets left, lets finalize the copyrights
   if ( m_packetsLeft == 0 ) {
      // now we are done; Lets sort the tree 
      m_copyrights.sort();
      setDone( true );
   }


}


StringTable::stringCode CopyrightRequest::getStatus() const {
   if ( m_packetsLeft == 0 ) {
      return StringTable::OK;
   }
   return StringTable::NOTOK;
}
