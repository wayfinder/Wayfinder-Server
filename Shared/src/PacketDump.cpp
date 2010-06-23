/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PacketDump.h"

#include "Packet.h"
#include "FilePtr.h"
#include "NetUtility.h"
#include "DataBuffer.h"

#include <iostream>
#include <ctime>
#include <cstring>
#include <netinet/in.h>

using namespace std;

namespace PacketUtils {

void dumpToFile( const MC2String& filename, const Packet& packet ) {
   FileUtils::FilePtr file( fopen( filename.c_str(), "w") );
   if ( file.get() == NULL ) {
      mc2dbg << "[dumpToFile] Failed to dump packet to file: " 
             << filename << ", error =  " 
             << strerror( errno ) << endl;
      return;
   }
   dumpToFile( file.get(), packet );
}

void printPacket( const Packet& packet ) {
   dumpToFile( stdout, packet );
}

void dumpHeaderToFile( FILE* file, const Packet& packet, 
                       bool lookupIP ) {
#ifndef _MSC_VER
   MC2_ASSERT( file ); 
   time_t tt;   
   struct tm* tm;

   ::time(&tt);
   struct tm result;
   tm = localtime_r(&tt, &result);

   fprintf( file, "%04d-%02d-%02d %02d:%02d:%02d\n ",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);
          
   fprintf( file, "HEADER:\n");
   fprintf( file, "Subtype = %s\n", packet.getSubTypeAsString() );
   fprintf( file, "Protocolversion = %x\n", packet.getProtocolVersion() );
   fprintf( file, "Priority = %x\n", packet.getPriority() );
   fprintf( file, "Length = %d\n", packet.getLength() );
   fprintf( file, "ReqID = %d\n", packet.getRequestID() );
   fprintf( file, "PacketID = %d\n", packet.getPacketID() );
   fprintf( file, "Deb info = %u\n", packet.getDebInfo() );
   fprintf( file, "PacketNbr = %u\n", packet.getPacketNbr());
   fprintf( file, "NbrPackets = %u\n", packet.getNbrPackets() );
   fprintf( file, "ResendNbr = %u\n", packet.getResendNbr() );
   fprintf( file, "Timeout = %u\n", packet.getTimeout() );
   fprintf( file, "ArrivalTime = %u\n", packet.getArrivalTime() );
   fprintf( file, "IP = %s ", NetUtility::getHostName( packet.getOriginIP(),
                                                       lookupIP ).c_str() );
   fprintf( file, "Port = %d\n", packet.getOriginPort() );
#endif
}

void dumpDataToFile( FILE* file, const Packet& packet ) {
   dumpDataToFile( file, packet, HEADER_SIZE );
}

void dumpDataToFile( FILE* file, const Packet& packet, uint32 startPos ) {
#ifndef _MSC_VER
   MC2_ASSERT( file );
   const byte* buffer = packet.getBuf();
   fprintf( file, "DATA:\n%0#8x  ", startPos );
    for ( uint32 i=startPos; i < packet.getLength(); ++i ) {
      if ( i != startPos ) {
         if (! ( ( i - startPos ) % 2 ) ) {
            fprintf( file, " " );
         }
         if ( ! ( ( i - startPos ) % 8 ) ) {
            fprintf( file, "\n%0#8x  ", i );
         }
      }
      fprintf( file, "%x%x", (buffer[i] >> 4) & 0x0f, buffer[i] & 0x0f );
    }
    fprintf( file, "\n" );
#endif 

}

void dumpToFile( FILE* file, const Packet& packet ) {
   dumpHeaderToFile( file, packet );
   dumpDataToFile( file, packet );
}

void dumpBinaryToFile( const MC2String& filename, const Packet& packet ) {
   if ( filename.empty() ) {
      return;
   }

   FileUtils::FilePtr packetFile( fopen( filename.c_str(), "ab" ) );

   if ( packetFile.get() == NULL ) {
      mc2dbg << "[PacketDump] Failed to open file for packet file: "
             << filename << "." << endl;
      mc2dbg << "[PacketDump] Error: " << strerror( errno ) << endl;

      return;
   }

   dumpBinaryToFile( packetFile.get(), packet );
}

void dumpBinaryToFile( FILE* file, const Packet& packet ) {
   MC2_ASSERT( file );

   uint32 packLen = ntohl( packet.getLength() );

   if ( fwrite( &packLen, 4, 1, file ) != 1 ) {
      mc2dbg << warn << "[PacketDump]: Could not write packet length."
             << " The packet file might be corrupted. " << endl;
      return;
   }

   if ( fwrite( packet.getBuf(),
                packet.getLength(), 1, file ) != 1 ) {
      mc2dbg << warn << "[PacketDump]: Could not write buffer."
             << " The packet file might be corrupted. " << endl; 
      return;
   }
}
Packet* loadPacketFromFile( const MC2String& filename ) {
   FileUtils::FilePtr file( fopen( filename.c_str(), "r" ) );
   if ( ! file.get() ) {
      return NULL;
   }

   return loadPacketFromFile( file.get() );
}

Packet* loadPacketFromFile( FILE* file ) {
   // read packet size
   DataBuffer buf( 4 );
   if ( fread( buf.getBufferAddress(), buf.getBufferSize(), 1, file ) != 1 ) {
      return NULL;
   }

   uint32 packSize = buf.readNextLong();
   RequestPacket* packet = new RequestPacket( packSize + buf.getBufferSize() );
   if ( fread ( packet->getBuf(), packSize, 1, file ) != 1 ) {
      delete packet;
      return NULL;
   }

   return packet;
}

}

