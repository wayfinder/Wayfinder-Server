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
#include "TCPSocket.h"
#include "DataBuffer.h"

#include "MapCacheElement.h"

MapCacheElement::MapCacheElement( uint32 mapID,
                                  CACHE_ELEMENT_TYPE element_type,
                                  byte zoomlevel ) 
   : CacheElement( mapID, element_type, SERVER_TYPE /*Obsolete*/)
{
   m_zoomlevel = zoomlevel;
   m_version = 0;
   m_data = NULL;
   m_size = 0;
}

MapCacheElement::MapCacheElement( uint32 mapID,
                                  CACHE_ELEMENT_TYPE element_type,
                                  uint32 hostIP, 
                                  uint16 hostPort,
                                  byte zoomlevel ) 
   : CacheElement( mapID, element_type, SERVER_TYPE /*Obsolete*/ ) 
{
   m_zoomlevel = zoomlevel;
   m_version = 0;
   m_data = NULL;
   m_size = 0;
   ReadMap( hostIP, hostPort );
}

MapCacheElement::MapCacheElement( uint32 mapID,
                                  CACHE_ELEMENT_TYPE element_type,
                                  byte zoomlevel,
                                  uint32 version,
                                  byte* data, 
                                  int length ) 
   : CacheElement( mapID, element_type, SERVER_TYPE /*Obsolete*/ )
{
   m_zoomlevel = zoomlevel;
   m_version = version;
   m_size = length;
   m_data = new byte[m_size];
   memcpy( m_data, data, m_size );
}

void MapCacheElement::ReadMap( uint32 mapIP, uint16 mapPort ) 
{
   // Create socket
   TCPSocket sock;
   sock.open(); 

   // Connect to mapmodule
   if ( sock.connect(mapIP, mapPort) ) {
      if ( getCacheType() == SERVER_TYPE ) {
         mc2dbg4 << "MapCEl::ReadMap(): ReadMap = SERVER_TYPE, Sending "
                    "handshaking" << endl;
         sock.writeAll( (byte*)"\n\tHey You! Give me some!\n\0", 
                        27 ); //Dummy send
      } 

      DataBuffer dataBuffer( 8 );
      if( sock.readExactly( dataBuffer.getBufferAddress(), 8 ) != 8 ) {
         mc2dbg << "MapCEl::ReadMap(): SocketError, version and size. " 
                << endl;
         m_isValid = false;
         return;
      }
      dataBuffer.reset();
      m_version = dataBuffer.readNextLong();
      m_size = dataBuffer.readNextLong();
      mc2dbg2 << "MapCEl::ReadMap(): Version " << m_version << " size " 
              << m_size << endl; 
      if( m_size > 0 ){
         m_data = new byte[m_size];
         int nbrRead; 
         if( ( nbrRead = sock.readExactly( m_data, m_size )) != (int)m_size ){
            mc2dbg << "MapCEl::ReadMap(): SocketError, no map read read " 
                   << nbrRead << " bytes " << endl;
            m_isValid = false;
            return;
         }
      }
   } 
   else { // Unable to connect
      mc2dbg << "MapCEl::ReadMap(): Unable to connect to MapModule! No data "
                "read!" << endl;
      m_isValid = false;
   }
   sock.close();
}

bool MapCacheElement::operator == (const MapCacheElement& el) const
{
   return m_ID == el.m_ID && m_zoomlevel == el.m_zoomlevel;
}

bool MapCacheElement::operator != (const MapCacheElement& el) const
{
   return m_ID != el.m_ID || m_zoomlevel != el.m_zoomlevel;
}

