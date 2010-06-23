/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "SearchReplyData.h"
#include "SearchMatch.h"
#include "DeleteHelpers.h" // STLUtility
#include "Packet.h"
#include "PacketContainer.h"

SearchReplyData::SearchReplyData()
{
   m_startIdx = -1;
   m_endIdx   = -1;
   m_totalNbrHits = -1;
}

void
SearchReplyData::clear()
{
   STLUtility::deleteValues( m_matches );
   STLUtility::deleteValues( m_overviewMatches );
   STLUtility::deleteValues( m_packetContainers );
   m_matches.clear();
   m_overviewMatches.clear();
}

SearchReplyData::~SearchReplyData()
{
   clear();
}

void
SearchReplyData::setNbrHits( int startIdx,
                             int endIdx,
                             int total )
{
   m_startIdx     = startIdx;
   m_endIdx       = endIdx;
   m_totalNbrHits = total;
   mc2dbg << "[SearchReplyData]: setNbrHits(" << startIdx << ", "
          << endIdx << ", " << total << ")" << endl;
}

int
SearchReplyData::save( Packet* p, int& pos ) const
{
   int orig_pos = pos;
   int orig_pos2 = pos;
   int nbr_written = 0;   
   p->incWriteLong( pos, m_startIdx );
   p->incWriteLong( pos, m_endIdx );
   p->incWriteLong( pos, m_totalNbrHits );
   // Unused right now - but always zero
   p->incWriteLong( pos, 0 );
   p->incWriteLong( pos, 0 );
   p->incWriteLong( pos, 0 );

   p->incWriteLong( pos, m_matches.size() );
   for( vector<VanillaMatch*>::const_iterator it = m_matches.begin();
        it != m_matches.end();
        ++it ) {
      (*it)->save( p, pos );
      ++nbr_written;
      // Check if the packet is full.
      if ( pos >= int(p->getBufSize()-VP_MIN_FREE) ) {
         p->incWriteLong( orig_pos2, nbr_written );
         orig_pos2 = orig_pos;
         break;
      }
   }
   p->incWriteLong( pos, m_overviewMatches.size() );
   for ( uint32 i = 0; i < m_overviewMatches.size(); ++i ) {
      m_overviewMatches[ i ]->save( p, pos );
   }

   return pos - orig_pos;
}

int
SearchReplyData::load( const Packet* p, int& pos )
{
   int orig_pos = pos;
   clear();
   p->incReadLong( pos, m_startIdx );
   p->incReadLong( pos, m_endIdx );
   p->incReadLong( pos, m_totalNbrHits );
   // Unused right now
   p->incReadLong( pos );
   p->incReadLong( pos );
   p->incReadLong( pos );
   int nbr = p->incReadLong( pos );
   m_matches.reserve( nbr );
   while ( nbr -- ) {
      m_matches.push_back( static_cast<VanillaMatch*>
                           (SearchMatch::createMatch(p, pos ) ) );
   }
   nbr = p->incReadLong( pos );
   m_overviewMatches.reserve( nbr );
   while ( nbr-- ) {
      OverviewMatch* match = 
         dynamic_cast<OverviewMatch*>( SearchMatch::createMatch( p, pos ) );

      MC2_ASSERT( match );

      m_overviewMatches.push_back( match );

   }

   return pos - orig_pos;
}

uint32
SearchReplyData::getTotalNbrMatches() const
{
   if ( m_totalNbrHits >= 0 ) {
      return m_totalNbrHits;
   } else {
      return m_matches.size() + m_overviewMatches.size();
   }
}

#define IN_RANGE( x,y,z ) (MIN(MAX((x),(y)),(z)))

int
SearchReplyData::translateMatchIdx( int wantedIdx ) const
{
   int size = m_matches.empty() ? m_overviewMatches.size() : m_matches.size();
   mc2dbg << "[SearchReplyData]::translateMatchIdx("
          << wantedIdx << ")" << endl;
   mc2dbg << "[SearchReplyData]::m_startIdx = "
          << m_startIdx << ", m_endIdx = " << m_endIdx
          << ", m_matches | m_overviewMatches.size() = " << size << endl;

   if ( m_startIdx >= 0 ) {
      return IN_RANGE( 0, wantedIdx - m_startIdx, size );
   } else {
      return IN_RANGE( 0, wantedIdx, size );
   }
   
}
