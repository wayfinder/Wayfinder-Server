/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavCellHandler.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "UserData.h"
#include "NavUserHelp.h"
#include "MimeMessage.h"
#include "SendEmailPacket.h"
#include "Properties.h"
#include "Utility.h"

NavCellHandler::NavCellHandler( InterfaceParserThread* thread,
                              NavParserThreadGroup* group,
                              NavUserHelp* userHelp )
      : NavHandler( thread, group ),
        m_userHelp( userHelp )
{
}


bool
NavCellHandler::handleCell( UserItem* userItem, 
                            NavRequestPacket* req, 
                            NavReplyPacket* reply )
{
   if ( !checkExpectations( req->getParamBlock(), reply ) ) {
      return false;
   }

   bool ok = true;

   // The params
   const NParamBlock& params = req->getParamBlock();
   NParamBlock& rparams = reply->getParamBlock();
   // The user
   UserUser* user = userItem->getUser();

   // Start parameter printing
   mc2log << info << "handleCell:";

   if ( params.getParam( 5000 ) && 
        params.getParam( 5000 )->getLength() >= 5*2 ) 
   {
      mc2log << " Data length " << params.getParam( 5000 )->getLength()
             << endl;

      uint32 pos = 0;
      uint16 cellPackProtoVer = params.getParam( 5000 )->getUint16( pos );
      pos += 2;
      uint16 cellPackID = params.getParam( 5000 )->getUint16( pos );
      pos += 2;
      /*uint16 cellPackType =*/ params.getParam( 5000 )->getUint16( pos );
      pos += 2;
      /*uint16 cellPackPartNbr=*/params.getParam( 5000 )->getUint16( pos );
      pos += 2;
      /*uint16 cellPackTotalPartNbr =*/ params.getParam( 5000 )
         ->getUint16( pos );
      pos += 2;
      

      // Mailto logs
      MimeMessage* m = new MimeMessage( 
         MimePartApplication::getContentTypeAsString(
            MimePartApplication::CONTENT_TYPE_APPLICATION_OCTETSTREAM ) );
      m->add( new MimePartApplication( 
                 const_cast<byte*>( params.getParam( 5000 )->getBuff() ), 
                 params.getParam( 5000 )->getLength(), 
                 MimePartApplication::CONTENT_TYPE_APPLICATION_OCTETSTREAM,
                 "", true ) );
      
      bool sentOk = false;
      SendEmailRequestPacket* p = new SendEmailRequestPacket( 0 );
      char* body = m->getMimeMessageBody();
      const char* optionalHeaderTypes[ 2 ] = 
         { MimeMessage::mimeVersionHeader, 
           MimeMessage::contentTypeHeader };
      const char* optionalHeaderValues[ 2 ] = 
         { m->getMimeVersion(), m->getContentType() };
      MC2String sender( Properties::getProperty( 
         "DEFAULT_RETURN_EMAIL_ADDRESS", 
         "please_dont_reply@localhost.localdomain" ) );

      if ( user->getEmailAddress()[ 0 ] != '\0' ) {
         sender = user->getEmailAddress();
      } else {
         sender = user->getLogonID();
         // Make it a valid email-address
         sender.append( "@localhost.localdomain" );
      }
      
      if ( p->setData( "mc2logs@localhost.localdomain", 
                       sender.c_str(), "my logfiles", body,
                       2, optionalHeaderTypes, optionalHeaderValues ) )
      {
         PacketContainer* rp = new PacketContainer( p, 0, 0 , 
                                                    MODULE_TYPE_SMTP );
         PacketContainer* pc = m_thread->putRequest( rp );
         if ( pc != NULL && static_cast< ReplyPacket* >( pc->getPacket() )
              ->getStatus() == StringTable::OK )
         {
            sentOk = true; 
         }
         delete pc;
      } else {
         mc2log << error << "handleCell "
                << "SendEmailRequestPacket::setData failed." << endl;
      }
      
      delete [] body;
      delete m;
      
      if ( !sentOk ) {
         Utility::hexDump( 
            mc2log, 
            const_cast<byte*>( params.getParam( 5000 )->getBuff() ),
            params.getParam( 5000 )->getLength() );
      }

      // The reply data
      NParam& replyData = rparams.addParam( NParam( 5100 ) );

      replyData.addUint32( 5*2 );
      // Write cell packet header 
      // protocol version, 2 bytes, currently always zero.
      replyData.addUint16( cellPackProtoVer );
      // packet id, 2 bytes, a sequential number
      replyData.addUint16( cellPackID );
      // packet type, 2 bytes, 0x8000 for acknowledge.
      replyData.addUint16( 0x8000 );
      // packet part number, 2 bytes, 0-based (0 for ack)
      replyData.addUint16( 0 );
      // total number of parts, 2 bytes, 0-based (0 for ack)
      replyData.addUint16( 0 );
      
      mc2log << info << "handleCell reply: "
             << " size " << replyData.getLength() << endl;
   } else {
      mc2log << " No data!" << endl;
   }


   return ok;
}
