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

#include "Packet.h"

/**
 *    Class that helps keeping track of saving the length of the
 *    data in a packet.
 */
class SaveLengthHelper {
public:
   /// Creates a new SaveLengthInPacketHelper and sets the start pos.
   SaveLengthHelper( Packet* packet, int startPos ) :
         m_packet( packet ),
         m_startPos( startPos ),
         m_lengthPos( MAX_UINT32 ) {
   }

   /// Makes room for length and saves the position. Updates length_pos
   void writeDummyLength( int& length_pos ) {
      m_lengthPos = length_pos;
      m_packet->incWriteLong( length_pos, 0 );
   }
   
   /// Updates the length at the saved position. Returns the length
   uint32 updateLengthUsingEndPos( int end_pos ) {
      MC2_ASSERT( m_lengthPos != -1 );
      int tmpPos = m_lengthPos;
      uint32 length = end_pos - m_startPos;
      m_packet->incWriteLong( tmpPos, length );
      return length;
   }
private:
   /// Current packet
   Packet* m_packet;
   /// Start position
   int m_startPos;
   /// Position where the length should be written.
   int m_lengthPos;
   
};

/**
 *    Class that helps keeping track of load the length of the
 *    data from a packet and then skipping the unknown data.
 */
class LoadLengthHelper {
public:
   /// Constructs the loadlength helper.
   LoadLengthHelper( const Packet* packet, int start_pos,
                     const char* dbg = NULL ) :
         m_packet( packet ),
         m_startPos( start_pos ),
         m_length( MAX_UINT32 ),
         m_dbg( dbg ) {
   }

   /// Load the length - update pos
   uint32 loadLength( int& pos ) {
      m_length = m_packet->incReadLong( pos );
      return m_length;
   }

   /// Skips unread data. Returns the length.
   uint32 skipUnknown( int& pos ) const {
      MC2_ASSERT( m_length != MAX_UINT32 );
      if ( m_dbg ) {
         if ( uint32(pos) != m_startPos + m_length ) {
            mc2dbg << m_dbg << ": skipping unknown data - ok if mixed versions"
                   << endl;
         }
      }
      pos = m_startPos + m_length;
      return m_length;
   }
   
   /// Returns the number of remaining bytes of data.
   uint32 available( int& pos ) const {
      if( (pos - m_startPos) < m_length ) {
         return pos - m_startPos;
      } else {
         return 0;
      }
   }
   
private:
   /// The packet to read from
   const Packet* m_packet;
   /// The start position
   uint32 m_startPos;
   /// The length
   uint32 m_length;
   /// Debug string
   const char* m_dbg;
};
