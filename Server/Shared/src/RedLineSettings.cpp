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

#include "RedLineSettings.h"

#include "Packet.h"
#include "PacketHelpers.h"

namespace {
   static RedLineSettings::speedVect_t createDefaultSpeedVect()
   {
      RedLineSettings::speedVect_t vect;
      // 50 meters from 0 km/h and up
      vect.push_back( make_pair( 0, 50 ) );
      return vect;
   }
}

// Init the default vector
RedLineSettings::speedVect_t 
RedLineSettings::c_defaultSpeedVect = createDefaultSpeedVect();

RedLineSettings::RedLineSettings( bool include_connecting_roads,
                                  const speedVect_t& speedvect )
      : m_speedVect( speedvect ),
        m_incConnecting( include_connecting_roads )
{
   // All is well that ends well
   // Good.
}

RedLineSettings::RedLineSettings() : m_speedVect( c_defaultSpeedVect ),
                                     m_incConnecting( false )
{
}

static void writeSpeedVect( Packet* packet,
                            int& pos,
                            const RedLineSettings::speedVect_t& vect )
{
   packet->incWriteLong( pos, vect.size() );
   for ( RedLineSettings::speedVect_t::const_iterator it = vect.begin();
         it != vect.end();
         ++it ) {      
      packet->incWriteLong( pos, it->first );
      packet->incWriteLong( pos, it->second );
      mc2dbg8 << "[RLH]: Writing " << it->first << "_" << it->second << endl;
   }
}

int
RedLineSettings::save( Packet* packet, int& pos ) const
{
   // Save the length
   SaveLengthHelper slh( packet, pos );
   slh.writeDummyLength( pos );

   // Save the data
   packet->incWriteLong( pos, m_incConnecting );

   writeSpeedVect( packet, pos, m_speedVect );
   writeSpeedVect( packet, pos, m_bboxSpeedVect );
   
   // Update the length
   mc2dbg8 << "[RLS]: Save Endpos before = " << pos << endl;
   uint32 tmp = slh.updateLengthUsingEndPos( pos );
   mc2dbg8 << "[RLS]: Save Endpos AFTER = " << pos << endl;
   return tmp;
}

static void readSpeedVect( const Packet* packet,
                           int& pos,
                           RedLineSettings::speedVect_t& vect )
{
   int vectSize = packet->incReadLong( pos );
   vect.resize( vectSize );
   for ( int i = 0; i < vectSize; ++i ) {
      vect[i].first  = packet->incReadLong( pos );
      vect[i].second = packet->incReadLong( pos );
   }
}

int
RedLineSettings::load( const Packet* packet, int& pos )
{
   // Clear the data
   m_speedVect.clear();
   m_bboxSpeedVect.clear();

   // Init the helper and load the length.
   LoadLengthHelper llh( packet, pos, "[RedLineS]" );
   llh.loadLength( pos );

   // Load the known data
   packet->incReadLong( pos, m_incConnecting );

   readSpeedVect( packet, pos, m_speedVect );
   if( llh.available( pos ) > 0 )
      readSpeedVect( packet, pos, m_bboxSpeedVect );
   
   // Skip unknown
   return llh.skipUnknown( pos );   
}

uint32
RedLineSettings::getSizeInPacket() const
{
   return 4 + 4 + 4 + 8 * m_speedVect.size();
}

ostream& operator<<(ostream& o, const RedLineSettings& s )
{
   o << "[RLS]: Con=" << s.m_incConnecting
     << " ";
   const char* comma = "";
   for ( RedLineSettings::speedVect_t::const_iterator it =
            s.m_speedVect.begin();
         it != s.m_speedVect.end();
         ++it ) {
      o << comma;
      o << it->first << "_" << it->second;
      comma = ",";
   }
   return o;
}

