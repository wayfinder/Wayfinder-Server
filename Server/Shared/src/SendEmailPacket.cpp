/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SendEmailPacket.h"
#include "StringUtility.h"

#define SENDEMAIL_REQUEST_PRIO   1
#define SENDEMAIL_REPLY_PRIO     1

const uint32 
SendEmailRequestPacket::m_addressLengthPos = REQUEST_HEADER_SIZE;

const uint32 
SendEmailRequestPacket::m_fromAddressLengthPos = REQUEST_HEADER_SIZE + 2;

const uint32 
SendEmailRequestPacket::m_subjectLengthPos = REQUEST_HEADER_SIZE + 4;

const uint32 
SendEmailRequestPacket::m_dataLengthPos = REQUEST_HEADER_SIZE + 8;

const uint32 
SendEmailRequestPacket::m_nbrHeaderLinesPos = REQUEST_HEADER_SIZE + 6;

const uint32 
SendEmailRequestPacket::m_addressStartPos = REQUEST_HEADER_SIZE + 12;



SendEmailRequestPacket::SendEmailRequestPacket()
   : RequestPacket( MAX_PACKET_SIZE,
                    SENDEMAIL_REQUEST_PRIO,
                    PACKETTYPE_SENDEMAILREQUEST,
                    0, 
                    0,
                    MAX_UINT32 )
{
   setAddressLength(0);
   setFromAddressLength(0);
   setSubjectLength(0);
   setDataLength(0);
   setNbrOptionalHeaderLines(0);
   setLength(m_addressStartPos);
}
   
SendEmailRequestPacket::SendEmailRequestPacket(uint16 requestID)
   : RequestPacket( MAX_PACKET_SIZE,
                    SENDEMAIL_REQUEST_PRIO,
                    PACKETTYPE_SENDEMAILREQUEST,
                    0, 
                    requestID,
                    MAX_UINT32 )
{
   setAddressLength(0);
   setFromAddressLength(0);
   setSubjectLength(0);
   setDataLength(0);
   setNbrOptionalHeaderLines(0);
   setLength(m_addressStartPos);
}
   
bool
SendEmailRequestPacket::setData(const char* adr, const char* fromAdr,
                                const char* subject, const char* data,
                                uint32 nbrOptionalHeaders , 
                                const char * const * const 
                                optionalHeaderTypes,
                                const char * const * const 
                                optionalHeaderValues)
{
   // Get the length of the strings
   uint32 adrLength = strlen(adr)+1;
   uint32 fromAdrLength = strlen(fromAdr)+1;
   uint32 subjectLength = strlen(subject)+1;
   uint32 dataLength = strlen(data)+1;
   uint32 optionalHeaderLengths = 0;
   
   for ( uint32 i = 0 ; i < nbrOptionalHeaders ; i++ ) {
      optionalHeaderLengths += strlen( optionalHeaderTypes[ i ] ) + 1;
      optionalHeaderLengths += strlen( optionalHeaderValues[ i ] ) + 1;
   }

   // Make sure it all fits
   if ( adrLength + fromAdrLength + subjectLength + 
       dataLength + m_addressStartPos + optionalHeaderLengths 
        >= getBufSize() )
   {
      resize( adrLength + fromAdrLength + subjectLength + 
              dataLength + m_addressStartPos + optionalHeaderLengths + 10);
   }

   if (adrLength + fromAdrLength + subjectLength + 
       dataLength + m_addressStartPos + optionalHeaderLengths 
       < getBufSize() ) 
   {
      // Write the strings
      int pos = m_addressStartPos;
      incWriteString(pos, adr);
      incWriteString(pos, fromAdr);
      incWriteString(pos, subject);
      incWriteString(pos, data);
      for ( uint32 i = 0 ; i < nbrOptionalHeaders ; i++ ) {
         incWriteString( pos, optionalHeaderTypes[ i ] );
         incWriteString( pos, optionalHeaderValues[ i ] );
      }
      setLength(pos);

      // Set the length of the strings
      setAddressLength(adrLength);
      setFromAddressLength(fromAdrLength);
      setSubjectLength(subjectLength);
      setDataLength(dataLength);
      setNbrOptionalHeaderLines( nbrOptionalHeaders );

      // Return
      return (true);
   } else {
      // Strings too long!
      return (false);
   }
}

bool 
SendEmailRequestPacket::
getData( MC2String& adr, MC2String& fromAdr, MC2String& subject,
         MC2String& data,
         uint32& nbrOptionalHeaders, char**& optionalHeaderTypes,
         char**& optionalHeaderValues ) const {
   // Create and initiate variables 
   int pos = m_addressStartPos;


   // Read address
   incReadString(pos, adr);

   // Read from address
   incReadString(pos, fromAdr);

   
   // Read subject
   incReadString(pos, subject);

   // Read data field (body)
   incReadString(pos, data);

   // Read optional header lines
   uint32 nbr = getNbrOptionalHeaderLines();
   nbrOptionalHeaders = nbr;

   optionalHeaderTypes = new char*[ nbr ];
   optionalHeaderValues = new char*[ nbr ];
   for ( uint32 i = 0 ; i < nbr ; i++ ) {
      MC2String tmpchar;
      incReadString( pos, tmpchar );
      optionalHeaderTypes[ i ] = StringUtility::newStrDup( tmpchar.c_str() );
      incReadString( pos, tmpchar );
      optionalHeaderValues[ i ] = StringUtility::newStrDup( tmpchar.c_str() );
   }

   // Return
   return (true);
}
   

// ====================================================================
//                                                        ReplyPacket =

SendEmailReplyPacket::SendEmailReplyPacket(const SendEmailRequestPacket* p,
                                           uint32 status)
   : ReplyPacket(REPLY_HEADER_SIZE,
                 PACKETTYPE_SENDEMAILREPLY,
                 p,
                 status)
{
   setLength(REPLY_HEADER_SIZE);
}

SendEmailReplyPacket::SendEmailReplyPacket()
   : ReplyPacket(REPLY_HEADER_SIZE,
                 PACKETTYPE_SENDEMAILREPLY)
{
   setLength(REPLY_HEADER_SIZE);
}

