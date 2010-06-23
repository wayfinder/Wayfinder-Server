/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavTunnelHandler.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "UserData.h"
#include "NavUserHelp.h"
#include "isabBoxSession.h"
#include "TCPSocket.h"
#include "NavRequestData.h"
#include "HttpHeader.h"
#include "STLStringUtility.h"
#include "Properties.h"
#include "ClientSettings.h"
#include "ParserCWHandler.h"
#include "URLAddToer.h"
#include "NamedServerLists.h"
#include "HttpInterfaceRequest.h"


using namespace URLAddToer;

NavTunnelHandler::NavTunnelHandler( InterfaceParserThread* thread,
                                    NavParserThreadGroup* group,
                                    NavUserHelp* userHelp )
      : NavHandler( thread, group ),
        m_userHelp( userHelp )
{
   m_expectations.push_back( ParamExpectation( 
                                5400, NParam::Byte_array ) );
   m_expectations.push_back( ParamExpectation( 
                                5401, NParam::Byte_array ) );
}


bool
NavTunnelHandler::handleTunnel( NavRequestData& rd ) {
   if ( !checkExpectations( rd.params, rd.reply ) ) {
      return false;
   }

   bool ok = true;
   int type = 0;

   // Start parameter printing
   //mc2log << info << "handleTunnel:";

   const NParam* adparam = NULL;
   if ( rd.params.getParam( 5400 ) != NULL && 
        rd.params.getParam( 5400 )->getLength() >= 4 ) {
      adparam = rd.params.getParam( 5400 );
      int32 data_type = adparam->getInt32();
      switch (data_type) {
      case 0:
         /* Content win data. */
         //mc2log << info << " with ad_data";
         type = data_type;
         break;
      case 1 :
         //mc2log << info << " Data type 1";
         type = data_type;
         break;
            
      default:
         /* Not handled. */
         mc2log << info << "handleTunnel: unknown type" << data_type << endl;
         rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         return false;
         break;
      }
   } else {
      /* Type set to 0 by default. */
      //mc2log << info << " default type 0";
   }

   //mc2log << endl;

   switch (type) {
      case 0 : {
         // Type 0 was never released.
         ok = false;
         mc2log << info << " Old tunnel type not supported." << endl;
         rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
      }  break;
      case 1:
         ok = tunnelRequest( rd );
      default:
         break;
   }

   return ok;
}


bool
NavTunnelHandler::tunnelRequest( NavRequestData& rd ) {
   bool ok = true;

   // The user
//   UserUser* user = rd.session->getUser()->getUser();

   if ( rd.params.getParam( 5400 ) == NULL || 
        rd.params.getParam( 5400 )->getLength() < 17 ) {
      ok = false;
      mc2log << warn << "handleTunnel: No 5400 param. Or too short." << endl;
      rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
   }

   uint32 type = 0;
   const NParam* p5400 = rd.params.getParam( 5400 );
   const NParam* p5401 = rd.params.getParam( 5401 );
   if ( ok ) {
      type = p5400->getUint32();
   } // End if ok

      
   if ( ok && type != 1 ) {
      ok = false;
      mc2log << warn << "handleTunnel: No supported type " << type 
             << endl;
      rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
   }

   if ( ok ) {
      uint32 pos = 4; // Type
      MC2String urlStr = p5400->incGetString( 
         m_thread->clientUsesLatin1(), pos );
      uint32 tunnelID = p5400->getUint32( pos ); pos += 4;
      uint32 fromByte = p5400->getUint32( pos ); pos += 4;
      uint32 toByte   = p5400->getUint32( pos ); pos += 4;

      // WFID
      if ( rd.clientSetting->getWFID() || 
           rd.clientSetting->getNotWFIDButUsesServices() ) {
         // Add some to url
         bool first = urlStr.find( "?" ) == MC2String::npos;
         // su (server uin)
         if ( rd.session->getUser() != NULL ) {
            addToURL( first, urlStr, "su", rd.session->getUser()->getUIN() );
         } else {
            addToURL( first, urlStr, "su", "" );
         }
         // hw keys
         // ADD key(s) hwd and hwdt for each key 
         for ( UserLicenceKeyVect::const_iterator key = rd.hwKeys.begin();
               key != rd.hwKeys.end() ; ++key ) {
            addToURL( first, urlStr, "hwd", (*key).getLicenceKeyStr() );
            addToURL( first, urlStr, "hwdt", (*key).getKeyType() );
         }

         // External auth
         if ( !rd.externalAuth.empty()  ) {
            // External auth failed
            if ( rd.session->getUser() == NULL ) {
               const char* extAuthErr = "error";
               if ( rd.externalAuthStatusCode == 
                    NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER ) {
                  extAuthErr = "unauthorized";
               } else if ( rd.externalAuthStatusCode ==
                           NavReplyPacket::NAV_STATUS_EXPIRED_USER ) {
                  extAuthErr = "expired";
               }
               addToURL( first, urlStr, "extautherr", extAuthErr );
            } else if ( rd.clientSetting->usesRights() && 
                        ! m_thread->checkAccessToService( 
                           rd.session->getUser()->getUser(), 
                           UserEnums::UR_WF, 
                           m_userHelp->getUrLevel( rd.clientSetting ) ) ) {
               addToURL( first, urlStr, "extautherr", "expired" );
            }
            addToURL( first, urlStr, "extauth", rd.externalAuth );
         }

         // Add server type srvt
         const char* srvt = NamedServerLists::NavServerType.c_str();
         if ( rd.ireq->getHttpRequest() != NULL ) {
            srvt = NamedServerLists::HttpServerType.c_str();
         }
         addToURL( first, urlStr, "srvt", srvt );
      }

      mc2log << info << "handleTunnel: " << "url " << urlStr
             << " tunnelID " << tunnelID << " fromByte " << fromByte
             << " toByte " << toByte << endl;

      MC2String postData;
      if ( p5401 != NULL ) {
         postData.insert( 
            0, reinterpret_cast< const char*> ( p5401->getBuff() ), 
            p5401->getLength() );
      }

      const MC2String eol( "\r\n" );
      HttpHeader outHeaders;
      MC2String reply;
      uint32 startByte;
      uint32 endByte;
      const HttpHeader* inHeaders = NULL;
      if ( rd.ireq->getHttpRequest() != NULL ) {
         inHeaders = rd.ireq->getHttpRequest()->getRequestHeader();
      }
      int ures = m_thread->getCWHandler()->getURL( 
         urlStr, postData, m_thread->getPeerIP(), fromByte, toByte,
         rd.clientLang, inHeaders, outHeaders, reply, startByte, endByte );
      mc2dbg8 << "ures " << ures << endl;

      NParam& replyHeaderParam = rd.rparams.addParam( 5500 );
      replyHeaderParam.addUint32( 1 ); // Ver
      replyHeaderParam.addString( urlStr, m_thread->clientUsesLatin1() );
      replyHeaderParam.addUint32( tunnelID );
      replyHeaderParam.addUint32( startByte );
      replyHeaderParam.addUint32( endByte );
      replyHeaderParam.addUint32( reply.size() );

      MC2String replyData;
      outHeaders.putHeader( &replyData );
      replyData.append( eol ); // End header
      uint32 headerSize = replyData.size();
      mc2dbg2 << "Reply " << endl << replyData << endl;
      replyData.insert( replyData.size(), reply.c_str() + startByte, 
                        endByte - startByte + 1 );
      mc2dbg4 << "Body " 
              << reply.substr( startByte, endByte - startByte + 1 ) 
              << endl;

      // Add in chunks if needed
      pos = 0;
      while ( pos < replyData.size() ) {
         uint32 addSize = MIN( replyData.size() - pos, MAX_UINT16 );
         rd.rparams.addParam( NParam( 
                                 5501, reinterpret_cast<const byte*> (
                                    replyData.c_str() ) + pos, addSize ) );
         pos += addSize;
      }

      mc2log << info << "handleTunnel:" << " startByte " << startByte 
             << " endByte " << endByte << " of body " << reply.size()
             << " and then header " << headerSize << endl;


      
   }

   return ok;
}
