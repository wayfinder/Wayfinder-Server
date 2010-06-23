/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "EmailProcessor.h"
#include "SendEmailPacket.h"
#include "EmailSender.h"

#include "StringTable.h"

EmailProcessor::EmailProcessor( MapSafeVector* loadedMaps,
                                EmailSender* emailSender ):
   Processor( loadedMaps ),
   m_emailSender( emailSender )
{

}


EmailProcessor::~EmailProcessor() {

}

Packet*
EmailProcessor::handleRequestPacket( const RequestPacket& p,
                                     char* packetInfo )
{
   Packet* answerPacket = NULL;

   switch ( p.getSubType() ) {
   case Packet::PACKETTYPE_SENDEMAILREQUEST: {
      answerPacket = 
         handleEmailRequest( static_cast< const SendEmailRequestPacket& >
                             ( p ) );

   } break;

   default:
      mc2log << warn << "EmailProcessor: Packet with subtype = " 
             << (uint32) p.getSubType() << " not supported." << endl;
      break;
   }

   return answerPacket;
}

int
EmailProcessor::getCurrentStatus()
{
   return 1;
}

Packet* 
EmailProcessor::
handleEmailRequest( const SendEmailRequestPacket& reqPacket ) {
   mc2log << "[EmailProcessor] processing SendEmailRequestPacket" << endl;

   Packet* answerPacket = NULL;

   MC2String address;
   MC2String fromAddress;
   MC2String subject;
   MC2String data;
   uint32 nbrOptionalHeaders = 0;
   char** optionalHeaderTypes = NULL;
   char** optionalHeaderValues = NULL;

   if ( reqPacket.getData( address, fromAddress, subject, data,
                           nbrOptionalHeaders, 
                           optionalHeaderTypes, 
                           optionalHeaderValues ) ) {

      mc2dbg << "Address = " << address << endl 
             << "FromAddress = " << fromAddress << endl 
             << "Subject = " << subject << endl;
      for ( uint32 i = 0 ; i < nbrOptionalHeaders ; i++ ) {
         cerr << optionalHeaderTypes[ i ] << ": " 
              << optionalHeaderValues[ i ] << endl;
      }
      mc2dbg << "Data = " << data << endl;;

      if ( m_emailSender->sendMail( fromAddress.c_str(),
                                    address.c_str(), 
                                    subject.c_str(),
                                    nbrOptionalHeaders,
                                    optionalHeaderTypes,
                                    optionalHeaderValues,
                                    data.c_str() ) ) {
         answerPacket = 
            new SendEmailReplyPacket( &reqPacket, StringTable::OK );
      } else {
         answerPacket = 
            new SendEmailReplyPacket( &reqPacket, StringTable::NOTOK );

         mc2log << warn << "EmailProcessor::handleRequest "
                << "falied to send email to " << address
                << endl;
      }
   } else {
      mc2dbg << "Error reading address and data from packet"
             << endl;
      answerPacket = new SendEmailReplyPacket( &reqPacket, 
                                               StringTable::NOTOK );
   }
   for ( uint32 i = 0 ; i < nbrOptionalHeaders ; i++ ) {
      delete optionalHeaderTypes[ i ];
      delete optionalHeaderValues[ i ];
   }
   delete optionalHeaderTypes;
   delete optionalHeaderValues;

   return answerPacket;
}
