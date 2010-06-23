/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SearchHeadingManager.h"

#include "Requester.h"
#include "DebugClock.h"
#include "ISABThread.h"
#include "ExtServicePacket.h"
#include "Properties.h"
#include "PacketContainer.h"
#include "RequestData.h"

struct SearchHeadingManager::Impl {
   Impl():
      m_headingCRC( 0 ) {
   }

   uint32 getCRC() const {
      ISABSync sync( m_dataLock );
      return m_headingCRC;
   }

   void getHeadings( CompactSearchHitTypeVector& headings,
                     uint32& crc ) const {
      ISABSync sync( m_dataLock );
      headings = m_headings;
      crc = m_headingCRC;
   }

   void setHeadings( CompactSearchHitTypeVector& headings,
                     uint32 crc ) {
      ISABSync sync( m_dataLock );
      m_headings.swap( headings );
      m_headingCRC = crc;
      m_updateClock = DebugClockSeconds();
   }

   /// @return true if update time is reached.
   bool updateTimeReached() const {
      ISABSync sync( m_dataLock );
      return m_updateClock.getTime() >=
         Properties::getUint32Property( "SEARCH_HEADING_TIMEOUT", 180 );
   }
private:
   /// Data lock for setting and getting variables in this class.
   mutable ISABMutex m_dataLock;

   /// Checksum for headings.
   uint32 m_headingCRC;
   /// Headings fetch from module.
   CompactSearchHitTypeVector m_headings;

   /// Count number of seconds since last update
   DebugClockSeconds m_updateClock;
};

SearchHeadingManager::SearchHeadingManager():
   m_impl( new Impl() ) {
}

SearchHeadingManager::~SearchHeadingManager() {
}


bool SearchHeadingManager::
shouldUpdateSearchHeadings( uint32 headingCRC ) const {
   // if crc mismatch or search heading update time is reached.
   return
      m_impl->getCRC() != headingCRC ||
      m_impl->updateTimeReached();
}

void SearchHeadingManager::
getSearchHeadings( Requester& requester,
                   CompactSearchHitTypeVector& headings,
                   uint32& headingCRC ) {

   //
   // If we haven't fetch any headings yet ( crc = 0 ) or
   // if we have a timeout on headings, then fetch new headings from
   // the ExtServiceModule.
   //
   if ( m_impl->getCRC() == 0 ||
        m_impl->updateTimeReached() ) {

      // Fetch the headings so we can setup the search parser handler
      ExtServiceRequestPacket* extServiceListRequest =
         new ExtServiceRequestPacket( 0,
                                      requester.getNextRequestID(),
                                      m_impl->getCRC() );

      auto_ptr< PacketContainer >
         cont( requester.putRequest( extServiceListRequest,
                                     MODULE_TYPE_EXTSERVICE ) );

      if ( cont.get() ) {
         ExtServiceReplyPacket*
            pack = static_cast< ExtServiceReplyPacket* >( cont->getPacket() );

         if ( pack ) {
            CompactSearchHitTypeVector headings;
            pack->getHitTypes( headings );
            m_impl->setHeadings( headings, pack->getCRC() );
         }
      }
   }

   m_impl->getHeadings( headings, headingCRC );
}

