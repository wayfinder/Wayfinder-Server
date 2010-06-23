/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserThread.h"
#include "RouteMessageRequest.h"
#include "ExpandRouteRequest.h"
#include "SendEmailPacket.h"
#include "LocalMapMessageRequest.h"
#include "RoutePacket.h"
#include "ExpandRoutePacket.h"
#include "ExpandItemID.h"
#include "GfxFeatureMap.h"
#include "Properties.h"
#include "XMLServerUtility.h"
#include "XMLServerElements.h"
#include "ServerTileMapFormatDesc.h"
#include "CopyrightHandler.h"
#include "XMLSMSCommon.h"
#include "ParserDebitHandler.h"

#ifdef USE_XML

struct RequestMail {
   MC2String m_address;
   MC2String m_subject;
   MC2String m_sender;
};

bool xmlParseEmailRequestHeader( DOMNode* cur,
                                 RequestMail& mail,
                                 ImageDrawConfig::imageFormat& imageFormat, 
                                 RouteMessageRequestType::MessageContent& contentType,
                                 UserConstants::RouteTurnImageType& turnImageType,
                                 uint32& maxMessageSize,
                                 uint32& overviewImageWidth, uint32& overviewImageHeight,
                                 uint32& routeTurnImageWidth, uint32& routeTurnImageHeight,
                                 bool& abbreviate, bool& landmarks, bool& onlyOverview,
                                 MC2String& errorCode, MC2String& errorMessage );

PacketContainer* handleInviteEmail( ParserThread* thread,
                                    const MC2String& toMailAddr,
                                    StringTable::languageCode lang ) {

   const char* subject = 
      StringTable::getString( StringTable::VICINITY_INVITE_MAIL_SUBJECT,
                              lang );
   const MC2String fromMailAddr = 
      Properties::getProperty( "INVITE_RETURN_EMAIL_ADDRESS", 
                               "\"Wayfinder Vicinity\" "
                               "<please_dont_reply@localhost.localdomain>" );

   // setup body
   MC2String body( "<html><head></head><body>" );
   
   body += StringTable::getString( StringTable::VICINITY_INVITE_MAIL,
                                   lang );
   body += "</body></html>"; // end

   MimeMessage mime;
   mime.add( new MimePartText( reinterpret_cast< const byte* >
                               ( body.c_str() ),
                               body.size(),
                               MimePartText::CONTENT_TYPE_TEXT_HTML,
                               MimePart::CHARSET_UTF_8, "", true ) );
   mc2dbg << "[EmailRequest] Mail: " << mime.getMimeMessageBody() << endl;
   SendEmailRequestPacket* emailPack = new SendEmailRequestPacket();
   const char* optionalHeaderTypes[ 2 ] = { 
      MimeMessage::mimeVersionHeader, 
      MimeMessage::contentTypeHeader 
   };

   const char* optionalHeaderValues[ 2 ] = {
      mime.getMimeVersion(), 
      mime.getContentType() 
   };

   if ( emailPack->
        setData( toMailAddr.c_str(), fromMailAddr.c_str(), subject, mime.getMimeMessageBody(),
                 2, optionalHeaderTypes, optionalHeaderValues ) ) {
      return thread->putRequest( emailPack, MODULE_TYPE_SMTP );
   }

   return NULL;
}

bool 
XMLParserThread::xmlParseEmailRequest( DOMNode* cur, 
                                       DOMNode* out,
                                       DOMDocument* reply,
                                       bool indent ) {
   bool ok = true;
   int indentLevel = 1;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );

   RequestMail reqMail;
   ImageDrawConfig::imageFormat imageFormat = ImageDrawConfig::PNG;
   uint32 routeID = 0;
   uint32 routeCreateTime = 0;
   StringTable::languageCode language = StringTable::ENGLISH;
   char* signature = NULL;
   char* originString = NULL;
   char* originLocationString = NULL;
   char* destinationString = NULL;
   char* destinationLocationString = NULL;
   bool routeEmail = false;
   bool localMapEmail = false;
   MC2String errorCode = "-1";
   MC2String errorStr = "Error handling request.";
   MC2BoundingBox lmBoundingbox;
   char* localMapString = NULL;
   GfxFeatureMap* mapSymbolMap = NULL;
   RouteMessageRequestType::MessageContent contentType = 
      RouteMessageRequestType::HTML_CONTENT;
   UserConstants::RouteTurnImageType turnImageType =
      UserConstants::ROUTE_TURN_IMAGE_TYPE_MAP_IMAGE;
   uint32 maxMessageSize = MAX_UINT32;
   uint32 overviewImageWidth = MAX_UINT32;
   uint32 overviewImageHeight = MAX_UINT32;
   uint32 routeTurnImageWidth = MAX_UINT32;
   uint32 routeTurnImageHeight = MAX_UINT32;
   bool abbreviate = true;
   bool landmarks = true;
   bool removeAheadIfDiff = true;
   bool nameChangeAsWP = false;
   bool onlyOverview = false;
   bool inviteEmail = false;

   // Create email_reply element
   DOMElement* email_reply = reply->createElement( X( "email_reply" ) );
   // Transaction ID
   email_reply->setAttribute( X( "transaction_id" ), 
                              cur->getAttributes()->getNamedItem( 
                                 X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( email_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( reply->createTextNode( X( indentStr.c_str() ) ),
                         email_reply );
   }
   
   
   // Get children
   
   for ( DOMNode* child = cur->getFirstChild(); 
         child != NULL; child = child->getNextSibling() ) {

      if ( child->getNodeType() != DOMNode::ELEMENT_NODE ) {
         continue;
      }

      // See if the element is a known type
      if ( XMLString::equals( child->getNodeName(),
                              "email_request_header" ) ) {
         ok = xmlParseEmailRequestHeader( child,
                                          reqMail,
                                          imageFormat,
                                          contentType,
                                          turnImageType,
                                          maxMessageSize,
                                          overviewImageWidth,
                                          overviewImageHeight,
                                          routeTurnImageWidth,
                                          routeTurnImageHeight,
                                          abbreviate, landmarks,
                                          onlyOverview,
                                          errorCode, errorStr );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "route_message_data" ) ) {
         routeEmail =
            XMLSMSCommon::
            xmlParseSendSMSRequestRouteMessageData( child, 
                                                    routeID, routeCreateTime,
                                                    language,
                                                    signature, 
                                                    originString, 
                                                    originLocationString,
                                                    destinationString, 
                                                    destinationLocationString);

      } else if ( XMLString::equals( child->getNodeName(),
                                     "local_map_data" ) ) {
         mapSymbolMap = new GfxFeatureMap();
         localMapEmail = xmlParseLocalMapData( child,
                                               language,
                                               signature,
                                               lmBoundingbox,
                                               localMapString,
                                               mapSymbolMap );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "invite_email" ) ) {
         inviteEmail = true;
      } else {
         mc2log << warn << "XMLParserThread::xmlParseEmailRequest "
            "odd Element in email_request element: " 
                << child->getNodeName() << endl;
      }


   }

   using XMLServerUtility::appendStatusNodes;

   MC2String emailSenderAddress;
   // Handle Request
   if ( ok ) {
      if ( ! reqMail.m_sender.empty() ) {
         emailSenderAddress = reqMail.m_sender;
      } else {
         emailSenderAddress = 
            Properties::getProperty( "DEFAULT_RETURN_EMAIL_ADDRESS", "" );
         if ( emailSenderAddress.empty() ) {
            emailSenderAddress = "mc2@localhost.localdomain";  
         }
      }
      // Check if emailaddresses are valid
      if ( ! StringUtility::validEmailAddress( reqMail.m_address.c_str() ) &&
           contentType != RouteMessageRequestType::SMIL_CONTENT ) {
         errorStr = "email_address not a valid email-address.";
         ok = false;
      }

      if ( ok && ! StringUtility::validEmailAddress( emailSenderAddress.c_str() ) &&
           contentType != RouteMessageRequestType::SMIL_CONTENT ) {
         errorStr = "return_email_address not a valid email-address.";
         ok = false;
      }

      RouteMessageRequest* routeEmailReq = NULL;
      LocalMapMessageRequest* localMapReq = NULL;
      PacketContainer* ansCont = NULL;

      if ( ok ) {
         // Not smaller than 30000
         maxMessageSize = MAX( 30000, maxMessageSize );

         // Set default values for image sizes if not set
         uint32 defaultOverviewImageWidth = 100;
         uint32 defaultOverviewImageHeight = 100;
         uint32 defaultRouteTurnImageWidth = 100;
         uint32 defaultRouteTurnImageHeight = 100;

         if ( contentType == RouteMessageRequestType::HTML_CONTENT ) {
            defaultOverviewImageWidth = defaultOverviewImageHeight = 500;
            defaultRouteTurnImageWidth = defaultRouteTurnImageHeight = 200;
         } else if ( contentType == RouteMessageRequestType::WML_CONTENT )
         {
            defaultOverviewImageWidth = defaultOverviewImageHeight = 200;
            defaultRouteTurnImageWidth = defaultRouteTurnImageHeight = 75;
         } else if ( contentType == RouteMessageRequestType::SMIL_CONTENT )
         {
            defaultOverviewImageWidth = 160;
            defaultOverviewImageHeight = 120;
            defaultRouteTurnImageWidth = 160;
            defaultRouteTurnImageHeight = 120;
         }
         if ( overviewImageWidth == MAX_UINT32 ) {
            overviewImageWidth = defaultOverviewImageWidth;
         }
         if ( overviewImageHeight == MAX_UINT32 ) {
            overviewImageHeight = defaultOverviewImageHeight;
         }
         if ( routeTurnImageWidth == MAX_UINT32 ) {
            routeTurnImageWidth = defaultRouteTurnImageWidth;
         }
         if ( routeTurnImageHeight == MAX_UINT32 ) {
            routeTurnImageHeight = defaultRouteTurnImageHeight;
         }

         if ( routeEmail ) {
            // Get and expand route
            PacketContainer* cont = NULL;
            RouteReplyPacket* routePack = NULL;
            uint32 UIN = 0;
            const char* extraUserinfo = NULL;
            uint32 validUntil = 0;
            int32 originLat = 0;
            int32 originLon = 0;
            uint32 originMapID = 0;
            uint32 originItemID = 0;
            uint16 originOffset = 0;
            int32 destinationLat = 0;
            int32 destinationLon = 0;
            uint32 destinationMapID = 0;
            uint32 destinationItemID = 0;
            uint16 destinationOffset = 0;
            byte expandType = ( ROUTE_TYPE_STRING | 
                                ROUTE_TYPE_ITEM_STRING |
                                ROUTE_TYPE_GFX | 
                                ROUTE_TYPE_GFX_COORD );
            PacketContainer* expandRouteCont = NULL;
            ExpandRouteRequest* expandReq = NULL;

            cont = getStoredRouteAndExpand( 
               routeID, routeCreateTime, expandType, language,
               abbreviate, landmarks, removeAheadIfDiff, nameChangeAsWP,
               routePack, UIN, extraUserinfo, validUntil,
               originLat, originLon, originMapID, originItemID, 
               originOffset,
               destinationLat, destinationLon, destinationMapID, 
               destinationItemID, destinationOffset,
               expandReq, expandRouteCont );
            if ( cont != NULL && expandRouteCont != NULL &&
                 static_cast< ReplyPacket* > ( 
                    expandRouteCont->getPacket() )
                 ->getStatus() == StringTable::OK )
            {
               ExpandRouteReplyPacket* expandPack = 
                  static_cast<ExpandRouteReplyPacket*> (  
                     expandRouteCont->getPacket() );
               // Get data
               ExpandItemID* exp = expandPack->getItemID();
               // The route vehicle
               uint32 mapID = 0;
               uint32 itemID = 0;
               ItemTypes::vehicle_t routeVehicle = ItemTypes::passengerCar;
               if ( routePack->getNbrItems() > 0 &&
                    GET_TRANSPORTATION_STATE( 
                       (routePack->getRouteItem(0, mapID, itemID),
                        itemID) ) != 0 )
               {
                  routeVehicle = 
                     ItemTypes::transportation2Vehicle(
                        GET_TRANSPORTATION_STATE( itemID ) );
               }

               DEBUG2(uint32 startTime = TimeUtility::getCurrentTime(););
               
               STMFDParams tileParam( LangTypes::english, false );
               CopyrightHandler 
                  copyrights( getGroup()->
                              getTileMapFormatDesc( tileParam, this )->
                              getCopyrightHolder() );
                                                                
                                                    
               // Send RouteMessageRequest
               routeEmailReq = new RouteMessageRequest( 
                  getNextRequestID(), routeID, routeCreateTime, 
                  language, routePack, expandPack, exp, 
                  overviewImageWidth, overviewImageHeight,
                  routeTurnImageWidth, routeTurnImageHeight,
                  originString, originLocationString,
                  destinationString, destinationLocationString,
                  routeVehicle, 
                  m_group->getTopRegionRequest(this),
                  true, true, false, true, false, false, 
                  onlyOverview,
                  contentType, turnImageType,
                  RouteArrowType::TURN_ARROW,
                  maxMessageSize, true,
                  reqMail.m_address.c_str(), 
                  emailSenderAddress.c_str(), 
                  reqMail.m_subject.c_str(), signature,
                  imageFormat,
                  &copyrights );


               ThreadRequestContainer* emailReqCont = 
                  new ThreadRequestContainer( routeEmailReq );
               mc2dbg2 << "About to send RouteMessageRequest" << endl;
               putRequest( emailReqCont );
               mc2dbg2 << "RouteMessageRequest done" << endl;
         
               ansCont = routeEmailReq->getAnswer();

               DEBUG2(
                  uint32 endTime = TimeUtility::getCurrentTime();
                  mc2dbg2 << "   Process time "
                  << (endTime - startTime) << "ms." << endl;
                  );
               
               delete emailReqCont;
               delete exp;
            } else {
               // Error
               errorStr = "Failed to expand stored route ";
               ok = false;
            }
            delete cont;
            delete routePack;
            delete expandReq;
            delete expandRouteCont;
         } else if ( localMapEmail ) {
            MC2String copyright = 
               getGroup()->getCopyright( this, lmBoundingbox, 
                                         LangTypes::english );
            localMapReq = new LocalMapMessageRequest(
               getNextRequestID(),
               lmBoundingbox.getMaxLat(), lmBoundingbox.getMinLon(),
               lmBoundingbox.getMinLat(), lmBoundingbox.getMaxLon(),
               language, overviewImageWidth, overviewImageHeight,
               localMapString, m_group->getTopRegionRequest(this),
               getClientSetting(),
               mapSymbolMap, 
               true, false, false, contentType, maxMessageSize,
               true, reqMail.m_address.c_str(),
               emailSenderAddress.c_str(), reqMail.m_subject.c_str(), signature,
               imageFormat, copyright );

            ThreadRequestContainer* emailReqCont = 
               new ThreadRequestContainer( localMapReq );
            mc2dbg2 << "About to send LocalMapMessageRequest" << endl;
            putRequest( emailReqCont );
            mc2dbg2 << "LocalMapMessageRequest done" << endl;
            
            ansCont = localMapReq->getAnswer();
               
            delete emailReqCont;
         } else if ( inviteEmail ) {
            ansCont = handleInviteEmail( this,
                                         reqMail.m_address,
                                         getCurrentUser()->getUser()->getLanguage() );
         } else {
            // No known type to send
            errorStr = "No supported email type to send.";
            ok = false;
         }
      } // Not ok is handled below

      if ( ok && ansCont != NULL ) {
         SendEmailReplyPacket* replyPack = 
            static_cast< SendEmailReplyPacket* >( ansCont->getPacket() );
         
         if ( replyPack->getStatus() == StringTable::OK ) {
            mc2log << info << "EmailRequest, to: " 
                   << reqMail.m_address
                   << ", from: " << emailSenderAddress
                   << ", route-email: " << ( routeEmailReq != NULL )
                   << ", contentType: " << int(contentType)
                   << endl;
            Request* request = NULL;
            if ( routeEmailReq != NULL ) {
               request = routeEmailReq;
            } else if ( localMapReq != NULL ) {
               request = localMapReq;
            }
            
            getDebitHandler()->makeMessageDebit( 
               m_user, request, ansCont, 
               reqMail.m_address.c_str(), 
               reqMail.m_subject.c_str() );
            
            // Status ok
            appendStatusNodes( email_reply, reply, indentLevel + 1, indent,
                               "0", "Email sent ok." );
         } else {
            mc2log << warn << "EmailRequest, to: " 
                   << reqMail.m_address
                   << ", from: " << emailSenderAddress
                   << ", route-email: " << ( routeEmailReq != NULL )
                   << ", contentType: " << int(contentType)
                   << ", sending not ok: " << StringTable::getString(
                      StringTable::stringCode( replyPack->getStatus() ),
                      StringTable::ENGLISH ) << endl;
            errorStr = "E-mail not sent!";
            ok = false;
         }
      } else if ( ok ) {
         mc2log << warn << "EmailRequest, to: " 
                << reqMail.m_address
                << ", from: " << emailSenderAddress
                << ", route-email: " << ( routeEmailReq != NULL )
                << ", contentType: " << int(contentType)
                << ", E-mail sender not responding!" << endl;
         errorStr = "E-mail sender not responding!";
         ok = false;
      }

      delete localMapReq;
      delete routeEmailReq;
   } // Not ok is handled below

   if ( !ok ) {
      // Error 
      appendStatusNodes( email_reply, reply, indentLevel + 1, indent,
                         errorCode.c_str(), errorStr.c_str() );
      // Error handled
      ok = true;
   }

   if ( indent ) {
      // Newline and indent before end email_reply tag   
      email_reply->appendChild( reply->createTextNode( 
                                   X( indentStr.c_str() ) ) );
   }
   

   delete [] signature;
   delete [] originString;
   delete [] originLocationString;
   delete [] destinationString;
   delete [] destinationLocationString;
   delete [] localMapString;
   delete mapSymbolMap;

   return ok;
}


bool 
xmlParseEmailRequestHeader( DOMNode* cur,
                            RequestMail& reqMail,
                            ImageDrawConfig::imageFormat& imageFormat, 
                            RouteMessageRequestType::MessageContent& contentType,
                            UserConstants::RouteTurnImageType& turnImageType,
                            uint32& maxMessageSize,
                            uint32& overviewImageWidth, uint32& overviewImageHeight,
                            uint32& routeTurnImageWidth, uint32& routeTurnImageHeight,
                            bool& abbreviate, bool& landmarks, bool& onlyOverview,
                            MC2String& errorCode, MC2String& errorMessage ) {
   bool ok = true;

   // Go throu attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;

   using XMLServerUtility::readSizeValue;

   for ( uint32 i = 0 ; i < attributes->getLength() && ok ; i++ ) {
      attribute = attributes->item( i );
      
      MC2String tmpStr = XMLUtility::
         transcodefrom( attribute->getNodeValue() );

      if ( XMLString::equals( attribute->getNodeName(), "image_format" ) ) {
         imageFormat = ImageDrawConfig::imageFormatFromString( tmpStr.c_str() );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "message_type" ) ) {
         contentType = 
            RouteMessageRequest::MessageContentFromString( tmpStr.c_str() );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_turn_image_type" ) ) {
         turnImageType = XMLServerUtility::
            routeTurnImageTypeFromString( tmpStr.c_str() );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "max_message_size" ) ) {
         if ( !readSizeValue( tmpStr, maxMessageSize ) ) {
            mc2log << warn 
                   << "XMLParserThread::xmlParseEmailRequestHeader "
                   << "max_message_size not a number " << tmpStr << endl;
            errorCode = "-1";
            errorMessage = "max_message_size not a number.";
            ok = false;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "overview_image_width" ) ) {
         if ( !readSizeValue( tmpStr, overviewImageWidth ) ) {
            mc2log << warn 
                   << "XMLParserThread::xmlParseEmailRequestHeader "
                   << "overview_image_width not a number " << tmpStr 
                   << endl;
            errorCode = "-1";
            errorMessage = "overview_image_width not a number.";
            ok = false;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "overview_image_height" ) ) {
         if ( !readSizeValue( tmpStr, overviewImageHeight ) ) {
            mc2log << warn 
                   << "XMLParserThread::xmlParseEmailRequestHeader "
                   << "overview_image_height not a number " << tmpStr 
                   << endl;
            errorCode = "-1";
            errorMessage = "overview_image_height not a number.";
            ok = false;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_turn_image_width" ) ) {
         if ( !readSizeValue( tmpStr, routeTurnImageWidth ) ) {
            mc2log << warn 
                   << "XMLParserThread::xmlParseEmailRequestHeader "
                   << "route_turn_image_width not a number " << tmpStr 
                   << endl;
            errorCode = "-1";
            errorMessage = "route_turn_image_width not a number.";
            ok = false;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_turn_image_height" ) ) {
         if ( !readSizeValue( tmpStr, routeTurnImageHeight ) ) {
            mc2log << warn 
                   << "XMLParserThread::xmlParseEmailRequestHeader "
                   << "route_turn_image_height not a number " << tmpStr 
                   << endl;
            errorCode = "-1";
            errorMessage = "route_turn_image_height not a number.";
            ok = false;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "abbreviate_route_names" ) ) {
         abbreviate = StringUtility::checkBoolean( tmpStr.c_str() );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_landmarks" ) ) {
         landmarks = StringUtility::checkBoolean( tmpStr.c_str() );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_only_overview" ) ) {
         onlyOverview = StringUtility::checkBoolean( tmpStr.c_str() );
      } else {
         mc2log << warn << "XMLParserThread::xmlParseEmailRequestHeader "
                   "unknown attribute "
                << "Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() 
                << " Value " << tmpStr << endl;
      }
   }

   
   // Get children
   for ( DOMNode* child = cur->getFirstChild();
         child != NULL; child = child->getNextSibling() ) {
      if ( child->getNodeType() != DOMNode::ELEMENT_NODE ) {
         continue;
      }
      // See if the element is a known type
      if ( XMLString::equals( child->getNodeName(),
                              "email_address" ) ) {
         reqMail.m_address = XMLUtility::getChildTextStr( *child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "subject" ) ) {
         reqMail.m_subject = XMLUtility::getChildTextStr( *child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "return_email_address" ) ) {
         reqMail.m_sender = XMLUtility::getChildTextStr( *child );
      } else {
         mc2log << warn << "XMLParserThread::"
            "xmlParseEmailRequestHeader "
            "odd Element in email_request_header element: " 
                << child->getNodeName() << endl;
      }

   }
   
   return ok;
}


bool
XMLParserThread::xmlParseLocalMapData( DOMNode* cur,
                                       StringTable::languageCode& language,
                                       char*& signature,
                                       MC2BoundingBox& lmBoundingbox,
                                       char*& localMapString,
                                       GfxFeatureMap* mapSymbolMap )
{
   bool ok = true;
   MC2String errorCode;
   MC2String errorMessage;
      
   // Get children

   
   for ( DOMNode* child = cur->getFirstChild();
         child != NULL;
         child = child->getNextSibling() ) {
      if ( child->getNodeType() != DOMNode::ELEMENT_NODE ) {
         continue;
      }
      // See if the element is a known type
      if ( XMLString::equals( child->getNodeName(), "language" ) ) {
         language = getLanguageCode( XMLUtility::getChildTextStr( *child ).c_str() );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "signature" ) ) {
         signature = XMLUtility::getChildTextValue( child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "boundingbox" ) ) {
         ok = XMLCommonElements::
            readBoundingbox( child, lmBoundingbox, errorCode, errorMessage );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "map_symbol_list" ) ) {
         ok = readMapSymbolList( child, mapSymbolMap );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "local_map_string" ) ) {
         localMapString = XMLUtility::getChildTextValue( child );
      } else {
         mc2log << warn << "XMLParserThread::"
            "xmlParseLocalMapData "
            "odd Element in local_map_data element: " 
                << child->getNodeName() << endl;
      }
   }
   
   return ok;
}


#endif // USE_XML

