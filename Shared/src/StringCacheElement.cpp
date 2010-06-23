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

#include "StringCacheElement.h"
#include "TCPSocket.h"
#include "DataBuffer.h"


StringTableCacheElement::StringTableCacheElement( uint32 mapID,
                                                  CACHE_TYPE cache_type )
   : CacheElement( mapID,
                   STRINGTABLE_ELEMENT_TYPE,
                   cache_type ) 
{
   m_strVector  = NULL;
   m_nbrStrings = 0;
   m_stringData = NULL;
   m_stringSize = 0;
}


StringTableCacheElement::StringTableCacheElement( uint32 mapID,
                                                  char* hostName,
                                                  uint16 hostPort,
                                                  CACHE_TYPE cache_type)
   : CacheElement( mapID,
                   STRINGTABLE_ELEMENT_TYPE,
                   cache_type ) 
{
   m_strVector  = NULL;
   m_nbrStrings = 0;
   m_stringData = NULL;
   readStringTable( hostName, hostPort );
} 

StringTableCacheElement:: StringTableCacheElement( uint32 mapID,
                                                   uint32 mapIP,
                                                   uint16 mapPort,
                                                   CACHE_TYPE cache_type)
   : CacheElement( mapID,
                   STRINGTABLE_ELEMENT_TYPE,
                   cache_type ) 
{
   m_strVector  = NULL;
   m_nbrStrings = 0;
   m_stringData = NULL;
   readStringTable( mapIP, mapPort );
}

StringTableCacheElement:: StringTableCacheElement( uint32 mapID,
                                                   byte* data,
                                                   int length,
                                                   int nbrStrings,
                                                   CACHE_TYPE cache_type)
   : CacheElement( mapID,
                   STRINGTABLE_ELEMENT_TYPE,
                   cache_type ) 
{
   m_strVector  = NULL;
   m_nbrStrings = nbrStrings;
   m_stringData = NULL;
   readStringTable( data, length );
}

StringTableCacheElement::~StringTableCacheElement()
{
   delete m_stringData;
   delete m_strVector;
}

void 
StringTableCacheElement::readStringTable( uint32 mapIP, uint16 mapPort )
{
   TCPSocket sock;
   sock.open(); 

   if ( sock.connect( mapIP, mapPort ) ) {
      readStringTable( sock );
   } 
   else { 
      DEBUG1(cerr << "Unable to connect to MapModule! No data read" 
                  << endl;);
      m_isValid = false;
   }
   sock.close();
}

void 
StringTableCacheElement::readStringTable( char* hostName, uint16 hostPort )
{
   TCPSocket sock;
   sock.open(); 
   if ( sock.connect( hostName, hostPort ) ) {
      readStringTable( sock );
   }
   else { // Unable to connect
      DEBUG1(cerr << "Unable to connect to MapModule! No data read" 
            << endl;);
      m_isValid = false;
   }
   sock.close();   
}

void 
StringTableCacheElement::readStringTable( byte* data, int length )
{
   DataBuffer dataBuffer(data, length );

   //m_stringSize = length - 4; 
   //m_nbrStrings = dataBuffer.readNextLong();
   m_stringSize = length; 
   
   // Read the strings in the stringtable
   m_stringData = new byte[m_stringSize];
   //memcpy( m_stringData, data+4, m_stringSize );
   memcpy( m_stringData, data, m_stringSize );

   // Convert to a vector with strings
   m_strVector = new char*[m_nbrStrings];
   uint32 i=0;
   char* temp = (char*)m_stringData; 
   while( i<m_nbrStrings ){
     m_strVector[i++] = temp;
     while( *temp != '\0' )
       temp++;
     temp++;
   }
}

void 
StringTableCacheElement::readStringTable( TCPSocket& sock )
{
   DataBuffer dataBuffer(4*4);

   if ( getCacheType() == SERVER_TYPE ) {
      DEBUG8(cout << "readStringTable: SERVER_TYPE" << endl);
      sock.writeAll( (byte*)"\n\tHey You! Give me some!\n", 
                     25 ); // Dummy send 
   } 
   else if ( getCacheType() == CLIENT_TYPE ) {
/*
      MapRequestPacket* p = new MapRequestPacket( 0, MapRequestPacket::MAPREQUEST_STRINGTABLE, m_ID );   

      dataBuffer.writeNextLong(MAP_REQUEST_STRING);
      dataBuffer.writeNextLong( p->getLength() );
      sock.writeAll( dataBuffer.getBufferAddress(), 8 );

      sock.writeAll( p->getDataAddress(), p->getLength() - HEADER_SIZE );
      delete p;

      if ( !(sock.readExactly(dataBuffer.getBufferAddress(), 1*4) == 1*4) ) {
         DEBUG1(cerr << "Socket error, no map read" << endl;);
         return;
      } 
      dataBuffer.reset();
      uint32 status = dataBuffer.readNextLong();
      if ( status != 0 ) {
         DEBUG1(cerr << "Server not ok." << endl;);
         return;
      }
      */
   }

   if ( !(sock.readExactly(dataBuffer.getBufferAddress(), 2*4) == 2*4) ) {
      DEBUG1(cerr << "Socket error, no map read" << endl;);
      return;
   }
   dataBuffer.reset();
   m_stringSize = dataBuffer.readNextLong();
   m_nbrStrings = dataBuffer.readNextLong();
   DEBUG8(
   cerr << "Size = " << m_stringSize
        << " ,number of strings = "
        << m_nbrStrings << endl;
   );
   
   // Read the strings in the stringtable
   m_stringData = new byte[m_stringSize];
   if (sock.readExactly( m_stringData, m_stringSize) != (int32)m_stringSize){
      DEBUG1(cerr << "SocketError, no strings read" << endl;);
      delete [] m_stringData;
      m_stringData = NULL;
      m_stringSize = 0;
      m_nbrStrings = 0;
      return;
   }

   // Convert to a vector with strings
   m_strVector = new char*[m_nbrStrings];
   uint32 i=0;
   char* temp = (char*)m_stringData; 
   while( i<m_nbrStrings ){
     m_strVector[i++] = temp;
     while( *temp != '\0' )
       temp++;
     temp++;
   }
}

void 
StringTableCacheElement::addLast( char* newString )
{
   char** tempVector = new char*[ m_nbrStrings + 1 ];

   uint32 i;
   for( i = 0; i < m_nbrStrings; i++ )
      tempVector[i] = m_strVector[i];

   tempVector[m_nbrStrings] = new char[ strlen(newString) + 1 ];
   strcpy( tempVector[m_nbrStrings], newString );

   delete m_strVector;
   m_strVector = tempVector;

   m_nbrStrings++;
}
