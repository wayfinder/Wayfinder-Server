/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavMessHandler.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "NavUtil.h"
#include "UserData.h"
#include "NavUserHelp.h"
#include "NavMessHelp.h"
#include "SearchMatch.h"
#include "Properties.h"


NavMessHandler::NavMessHandler( InterfaceParserThread* thread,
                              NavParserThreadGroup* group,
                              NavUserHelp* userHelp )
      : NavHandler( thread, group ),
        m_userHelp( userHelp ),
        m_messHelp( new NavMessHelp( thread, group ) )
{
   m_expectations.push_back( ParamExpectation( 2000, NParam::Byte ) );
   m_expectations.push_back( ParamExpectation( 2001, NParam::Byte ) );
   m_expectations.push_back( ParamExpectation( 2002, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 2003, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 2004, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 2005, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 2006, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 2007, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 2008, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 2009, NParam::String ) );
   m_expectations.push_back(
      ParamExpectation( 2010, NParam::Int32_array, 8, 8 ) );
   m_expectations.push_back( ParamExpectation( 2011, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 2012, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 2013, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 2014, NParam::String ) );
}


NavMessHandler::~NavMessHandler() {
   delete m_messHelp;
}


bool
NavMessHandler::handleMess( UserItem* userItem, 
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
   mc2log << info << "handleMess:";

   StringTable::languageCode language = user->getLanguage();
   if ( params.getParam( 6 ) ) {
      language = NavUtil::mc2LanguageCode( 
         params.getParam( 6 )->getUint16() );
      mc2log << " Lang " << StringTable::getString( 
         StringTable::getLanguageAsStringCode( language ), 
         StringTable::ENGLISH );
   }
   
   // 2000 Type byte MessageType
   NavMessHelp::MessageType messType = NavMessHelp::invalid;
   if ( params.getParam( 2000 ) ) {
      messType = NavMessHelp::MessageType( 
         params.getParam( 2000 )->getByte() );
      mc2log << " MessageType " << NavMessHelp::getMessageTypeAsString(
         messType );
   }

   // 2001 Type byte ObjectType
   NavMessHelp::ObjectType objectType = NavMessHelp::invalidObjectType;
   if ( params.getParam( 2001 ) ) {
      objectType = NavMessHelp::ObjectType( 
         params.getParam( 2001 )->getByte() );
      mc2log << " ObjectType " << NavMessHelp::getObjectTypeAsString(
         objectType );
   }

   // 2002 Type string Signature
   MC2String signature = "";
   if ( params.getParam( 2002 ) ) {
      signature = params.getParam( 2002 )->getString(
         m_thread->clientUsesLatin1());
      mc2log << " signature " << signature;
   }

   // 2003 Type string Subject
   MC2String subject = "";
   if ( params.getParam( 2003 ) ) {
      subject = params.getParam( 2003 )->getString(
         m_thread->clientUsesLatin1());
      mc2log << " subject " << subject;
   }

   // 2004 Type string EmailReceiver
   MC2String emailReceiver = "";
   if ( params.getParam( 2004 ) ) {
      emailReceiver = params.getParam( 2004 )->getString(
         m_thread->clientUsesLatin1());
      mc2log << " emailReceiver " << emailReceiver;
   }

   // 2005 Type string EmailSender
   MC2String emailSender = "";
   if ( params.getParam( 2005 ) ) {
      emailSender = params.getParam( 2005 )->getString(
         m_thread->clientUsesLatin1());
      mc2log << " emailSender " << emailSender;
   }

   // 2006 Type string PhoneReceiver
   MC2String phoneReceiver = "";
   if ( params.getParam( 2006 ) ) {
      phoneReceiver = params.getParam( 2006 )->getString(
         m_thread->clientUsesLatin1());
      mc2log << " phoneReceiver " << phoneReceiver;
   }

   // 2007 Type string PhoneSender
   MC2String phoneSender = "";
   if ( params.getParam( 2007 ) ) {
      phoneSender = params.getParam( 2007 )->getString(
         m_thread->clientUsesLatin1());
      mc2log << " phoneSender " << phoneSender;
   }

   // 2008 Type uint32 FavoriteID
   uint32 favoriteID = 0;
   if ( params.getParam( 2008 ) ) {
      favoriteID = params.getParam( 2008 )->getUint32();
      mc2log << " favoriteID " << favoriteID;
   }

   // 2009 Type string SearchItemID
   MC2String searchItemID = "";
   SearchMatch* searchMatch = NULL;
   if ( params.getParam( 2009 ) ) {
      searchItemID = params.getParam( 2009 )->getString(
         m_thread->clientUsesLatin1());
      searchMatch = SearchMatch::createMatch( searchItemID.c_str() );
      mc2log << " searchItemID " << searchItemID;
      if ( searchMatch == NULL ) {
         mc2log << "INVALID";
      }
   }

   // 2010 Type int32,int32 Position
   MC2Coordinate position;
   if ( params.getParam( 2010 ) ) {
      position = MC2Coordinate( 
         Nav2Coordinate( params.getParam( 2010 )->getUint32( 0 ),
                         params.getParam( 2010 )->getUint32( 4 ) ) );
      mc2log << " position " << position;
   }

   // 2011 Type string LocationName
   MC2String locationName = "";
   if ( params.getParam( 2011 ) ) {
      locationName = params.getParam( 2011 )->getString(
         m_thread->clientUsesLatin1());
      mc2log << " locationName " << locationName;
   }

   // 2012 Type string RouteID
   RouteID routeID( 0, 0 );
   if ( params.getParam( 2012 ) ) {
      routeID = RouteID( params.getParam( 2012 )->getString(
                            m_thread->clientUsesLatin1()).c_str() );
      mc2log << " routeID " << routeID;
   }

   // 2013 Type string OriginName
   MC2String originName = "";
   if ( params.getParam( 2013 ) ) {
      originName = params.getParam( 2013 )->getString(
         m_thread->clientUsesLatin1());
      mc2log << " originName " << originName;
   }

   // 2014 Type string DestinationName
   MC2String destinationName = "";
   if ( params.getParam( 2014 ) ) {
      destinationName = params.getParam( 2014 )->getString(
         m_thread->clientUsesLatin1());
      mc2log << " destinationName " << destinationName;
   }

   mc2log << endl;

   uint8 status = NavReplyPacket::NAV_STATUS_OK;
   byte* data = NULL;
   uint32 dataLen = 0;
   RouteMessageRequestType::MessageContent messageType = 
      RouteMessageRequestType::HTML_CONTENT;
   MC2String receiver = "";
   MC2String sender = "";
   bool contentLocation = false; // When we send it it works.

   switch ( messType ) {
      case NavMessHelp::invalid :
         status = NavReplyPacket::NAV_STATUS_NOT_OK; // XXX: UNSUPPRTED
         break;
      case NavMessHelp::HTML_email :
      case NavMessHelp::non_HTML_email : {
         messageType = RouteMessageRequestType::HTML_CONTENT;
         receiver = emailReceiver;
         sender = emailSender;
         const char* defaultEmailSender = Properties::getProperty( 
            "DEFAULT_RETURN_EMAIL_ADDRESS", 
            "\"Wayfinder\" <please_dont_reply@localhost.localdomain>" );
         if ( sender.empty() || 
              sender == "please_dont_reply@localhost.localdomain" ||
              sender == defaultEmailSender ) 
         {
            if ( StringUtility::validEmailAddress( 
                    user->getEmailAddress() ) )
            {
               sender = user->getEmailAddress();
            } else {
               sender = defaultEmailSender;
            }
         }
      }  break;
      case NavMessHelp::SMS :
         messageType = RouteMessageRequestType::WML_CONTENT;//Just anything
         receiver = phoneReceiver;
         sender = phoneSender;
         break;
      case NavMessHelp::MMS :
         messageType = RouteMessageRequestType::SMIL_CONTENT;
         receiver = phoneReceiver;
         sender = phoneSender;
         break;
      case NavMessHelp::FAX :
         status = NavReplyPacket::NAV_STATUS_NOT_OK; // XXX: UNSUPPRTED
         break;
      case NavMessHelp::InstantMessage :
         status = NavReplyPacket::NAV_STATUS_NOT_OK; // XXX: UNSUPPRTED
         break;
   };


   uint32 maxMessageSize = MAX_UINT32;
   uint32 overviewImageWidth = 100;
   uint32 overviewImageHeight = 100;
   uint32 routeTurnImageWidth = 100;
   uint32 routeTurnImageHeight = 100;
   bool sendMessage = true;
   ImageDrawConfig::imageFormat imageFormat = ImageDrawConfig::PNG;
   if ( messageType == RouteMessageRequestType::HTML_CONTENT ) {
      overviewImageWidth = overviewImageHeight = 500;
      routeTurnImageWidth = routeTurnImageHeight = 200;
   } else if ( messageType == RouteMessageRequestType::WML_CONTENT ) {
      overviewImageWidth = overviewImageHeight = 200;
      routeTurnImageWidth = routeTurnImageHeight = 75;
   } else if ( messageType == RouteMessageRequestType::SMIL_CONTENT ) {
      overviewImageWidth = 160;
      overviewImageHeight = 120;
      routeTurnImageWidth = 160;
      routeTurnImageHeight = 120;
      maxMessageSize = 50000; // XXX: 30000 Spec minimum
      sendMessage = false;
      // Client uses content location as file name
      contentLocation = true; // Set from reqVer in future
      imageFormat = ImageDrawConfig::GIF;
   }

   if ( status == NavReplyPacket::NAV_STATUS_OK ) {
      if ( objectType == NavMessHelp::Route ||
           objectType == NavMessHelp::Itinerary )
      {
         status = m_messHelp->handleRouteNavSend( 
            routeID, messType, language, sendMessage, imageFormat,
            messageType, maxMessageSize, overviewImageWidth,
            overviewImageHeight, routeTurnImageWidth, 
            routeTurnImageHeight, userItem, sender.c_str(), signature.c_str(),
            receiver.c_str(), contentLocation, data, dataLen );
      } else if ( (objectType == NavMessHelp::Destination ||
                   objectType == NavMessHelp::SearchItem ||
                   objectType == NavMessHelp::FullSearchItem||
                   objectType == NavMessHelp::Position) )
      {
         MC2String positionName = "";
         if ( locationName.size() > 0 ) {
            positionName = locationName;
         }
         status = m_messHelp->handleItemNavSend( 
            objectType, favoriteID, position, positionName.c_str(),
            searchMatch, messType, language, sendMessage, imageFormat,
            messageType, maxMessageSize, overviewImageWidth,
            overviewImageHeight, user, 
            userItem, sender.c_str(), signature.c_str(),
            receiver.c_str(), contentLocation, data, dataLen );
      } else {
         mc2log << warn << "handleMess: "
                << " unsupported message type " 
                << NavMessHelp::getMessageTypeAsString( messType )
                << " object type " << NavMessHelp::getObjectTypeAsString(
                   objectType ) << endl;
         status = NavReplyPacket::NAV_STATUS_NOT_OK; // XXX: Unsupported
      }
   } // End if status is ok

   mc2log << "handleMess: Status "
          << int(status) << " DataLen " << dataLen << endl;

   if ( data != NULL ) {
      // Add in chunks if needed
      uint32 pos = 0;
      while ( pos < dataLen ) {
         uint32 addSize = MIN( dataLen - pos, MAX_UINT16 );
         rparams.addParam( NParam( 2100, data + pos, addSize ) );
         pos += addSize;
      }
   }

   // Set status
   if ( status == NavReplyPacket::NAV_STATUS_OK ) {
      // OK!
   } else if ( status == NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT ) {
      reply->setStatusCode( NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
   } else { // NOT_OK
      reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
   }

   delete [] data;

   return ok;
}
