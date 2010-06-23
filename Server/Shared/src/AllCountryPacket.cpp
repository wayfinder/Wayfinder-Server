/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "StringTable.h"
#include "MC2BoundingBox.h"

#include "AllCountryPacket.h"

// ============================================================= Request =
AllCountryRequestPacket::AllCountryRequestPacket( uint16 reqID, uint16 packetID )
   : RequestPacket( REQUEST_HEADER_SIZE,      // Size
                    ALL_COUNTRY_REQUEST_PRIO, // prio
                    Packet::PACKETTYPE_ALLCOUNTRYREQUEST,
                    packetID,
                    reqID,
                    MAX_UINT32 ) // mapID
{
}

// ============================================================= Reply =

AllCountryReplyPacket::AllCountryReplyPacket()
   : ReplyPacket( MAX_PACKET_SIZE, 
                  Packet::PACKETTYPE_ALLCOUNTRYREPLY )
{
   setStatus( StringTable::OK );
   setLength( ALL_COUNTRY_REPLY_HEADER_SIZE );
   setNbrCountries( 0 );
}

AllCountryReplyPacket::AllCountryReplyPacket( AllCountryRequestPacket* reqPacket )
   : ReplyPacket( MAX_PACKET_SIZE,
                  Packet::PACKETTYPE_ALLCOUNTRYREPLY,
                  reqPacket,
                  StringTable::OK )
{
   setLength( ALL_COUNTRY_REPLY_HEADER_SIZE );
   setNbrCountries( 0 );
}

void AllCountryReplyPacket::addCountry( uint32 mapID, uint32 version, 
  									             const MC2BoundingBox& boundingBox,
									             Vector& stringIndex )
{
   int pos = getLength();
   incWriteLong( pos, mapID );
   incWriteLong( pos, version );
   incWriteLong( pos, boundingBox.getMinLat() );
   incWriteLong( pos, boundingBox.getMinLon() );
   incWriteLong( pos, boundingBox.getMaxLat() );
   incWriteLong( pos, boundingBox.getMaxLon() );
   incWriteLong( pos, stringIndex.getSize() );
   for( uint32 i=0; i<stringIndex.getSize();i++)
      incWriteLong( pos, stringIndex.getElementAt(i) );      

   setNbrCountries( getNbrCountries() + 1 );
   setLength( pos );
}

uint32 AllCountryReplyPacket::getNbrCountries()
{
   return readLong( REPLY_HEADER_SIZE );
}

void AllCountryReplyPacket::setNbrCountries( uint32 number )
{
   writeLong( REPLY_HEADER_SIZE, number );
}

uint32 AllCountryReplyPacket::getStartPos( uint32 index )
{
   int pos = ALL_COUNTRY_REPLY_HEADER_SIZE;
   for( uint32 i=0; i < index ; i++ ) {
      // Read number of strings at position + after all the static
      // data. 6 longs => 6 * 4.
      int numberStrings = readLong( pos + 6 * 4 );
      // Skip the number of strings too. That makes 7.
      pos +=  4 * ( 7 + numberStrings );
   }
   return pos;
}

bool AllCountryReplyPacket::getCountryData( uint32 index,
                                            uint32& mapID, 
										              uint32& version,
                                            MC2BoundingBox& boundingBox,
											           Vector& stringIndex )
{
   if( index < getNbrCountries() ) {
      // Empty the vector.
      stringIndex.reset();
      int pos = getStartPos( index );
      mapID   = incReadLong( pos );
      version = incReadLong( pos );
      boundingBox.setMinLat( incReadLong( pos ) );
      boundingBox.setMinLon( incReadLong( pos ) );
      boundingBox.setMaxLat( incReadLong( pos ) );
      boundingBox.setMaxLon( incReadLong( pos ) );
      int nbrStrings = incReadLong( pos );
      for( int i=0; i < nbrStrings; i++ ) {
         int string = incReadLong( pos );      
         stringIndex.addLast( string );
      }
      return true;
   }
   return false;
}






