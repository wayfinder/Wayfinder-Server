/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DebitPacket.h"
#include "Properties.h"

#include <fstream>


// /////////////////////////////////////////////////////////////////////
// DebitRequestPacket
// /////////////////////////////////////////////////////////////////////

DebitRequestPacket::DebitRequestPacket( uint16 packetID, 
                                        uint16 requestID,
                                        uint32 UIN,
                                        uint32 messageID,
                                        uint32 debInfo,
                                        uint32 date,
                                        uint32 operationType,
                                        uint32 sentSize,
                                        const char* userOrigin,
                                        const char* serverID,
                                        const char* operationDescription,
                                        uint32 nbrTransactions,
                                        uint32 originIP,
                                        uint16 originPort )
      : RequestPacket( MAX_PACKET_SIZE,
                       DEBIT_REQUEST_PRIO,
                       Packet::PACKETTYPE_DEBITREQUEST,
                       packetID,
                       requestID,
                       MAX_UINT32 )
{
   //Fill packet
   int pos = REQUEST_HEADER_SIZE;
   setOriginIP( originIP );
   setOriginPort( originPort );
   incWriteLong( pos, UIN );
   incWriteLong( pos, messageID );
   incWriteLong( pos, debInfo );
   incWriteLong( pos, date );
   incWriteLong( pos, operationType );
   incWriteLong( pos, sentSize );
   incWriteLong( pos, nbrTransactions );
   incWriteString( pos, userOrigin );
   incWriteString( pos, serverID );   
   incWriteString( pos, operationDescription);
   setLength( pos );
   
   MC2String debitFilename = 
      Properties::getProperty( "DEBIT_FILENAME", "" );

   if ( ! debitFilename.empty() ) {
      //Append to file
      char buffer[1024];
      uint32 size = 0;
   
      //Log time
      size += sprintf( buffer + size, "%d\t", date );
      
      time_t time = date;
      struct tm result;
      size += strftime( buffer + size, 1023 - size, 
                        "%a, %d %b %Y %H:%M:%S GMT\t", gmtime_r( &time, &result ) );
   
      //Log UIN
      size += sprintf( buffer + size, "%u\t", UIN );

      //Log server and user
      size += sprintf( buffer + size, "%s\t", userOrigin );
      size += sprintf( buffer + size, "%s\t", serverID );
   
      //Log nbr of sent sms/bytes
      size += sprintf( buffer + size, "%u\t", sentSize );

      //Log messageID
      size += sprintf( buffer + size, "%u\t", messageID );
   
      //Log operationType
      size += sprintf( buffer + size, "%u\t", operationType );
   
      //Log operationDescription
      size += sprintf( buffer + size, "%s\t", operationDescription );
   
      //Add a linefeed
      size += sprintf( buffer + size, "\n" );
   
      //Write to file
      ofstream outf;
   
      outf.open( debitFilename.c_str(), ios::app );
   
      if( !outf )
         cout << buffer;
      else
         outf << buffer;
   }
}


const char* 
DebitRequestPacket::getUserOrigin() const {
   char* temp;
   int pos = REQUEST_HEADER_SIZE+28;
   
   incReadString( pos, temp );
   
   return temp;
}


const char* 
DebitRequestPacket::getServerID() const {
   char* temp;
   int pos = REQUEST_HEADER_SIZE+28;
   
   //First read the UserOrigin thus updating pos
   incReadString( pos, temp );
   
   //Finally read the ServerID
   incReadString( pos, temp );
   
   return temp;
}


const char* 
DebitRequestPacket::getDescription() const {
   char* temp;
   int pos = REQUEST_HEADER_SIZE+28;
   
   //First read the UserOrigin and ServerID thus updating pos
   incReadString( pos, temp );
   incReadString( pos, temp );
   
   //Finally read the operationdescription
   incReadString( pos, temp );
   
   return temp;
}


////////////////////////////////////////////////////////////////////////
// DebitReplyPacket
////////////////////////////////////////////////////////////////////////


DebitReplyPacket::DebitReplyPacket( const DebitRequestPacket* p,
                                    uint32 status )
      : ReplyPacket( REPLY_HEADER_SIZE,
                     Packet::PACKETTYPE_DEBITREPLY,
                     p,
                     status)
{
   setLength( REPLY_HEADER_SIZE );
}
