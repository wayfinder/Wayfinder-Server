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


#ifdef USE_XML

#include "SMSPacket.h"
#include "SMSFormatter.h"
#include "ExpandRouteRequest.h"
#include "HttpUtility.h"
#include "CellularPhoneModelsElement.h"
#include "WayfinderSMSRequest.h"
#include "GfxFeatureMap.h"
#include "Properties.h"
#include "RoutePacket.h"
#include "ExpandRoutePacket.h"
#include "SearchMatch.h"
#include "XMLServerElements.h"
#include "XMLSMSCommon.h"
#include "ParserDebitHandler.h"

bool 
XMLParserThread::xmlParseSendSMSRequest( DOMNode* cur, 
                                         DOMNode* out,
                                         DOMDocument* reply,
                                         bool indent )
{
   bool ok = true;
   int indentLevel = 1;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   
   char* phoneNumber = NULL;
   char* message = NULL;
   char* signature = NULL;
   char* originString = NULL;
   char* originLocationString = NULL;
   char* destinationString = NULL;
   char* destinationLocationString = NULL;
   StringTable::languageCode language = StringTable::ENGLISH;
   bool messageSMS = false;
   bool routeSMS = false;
   bool localMapSMSLink = false;
   bool wayfinderRouteSMS = false;
   bool wayfinderDestinationSMS = false;
   bool wayfinderFavouriteSMS = false;
   bool wapLink = false; // for routeSMS
   bool wapPushServiceIndication = false;
   CellularPhoneModel* model = NULL; // for routeSMS
   uint32 routeID = 0;
   uint32 routeCreateTime = 0;
   const char* senderService = "WEBB"; // INFO: Server send nbr
   MC2BoundingBox lmBoundingbox;
   GfxFeatureMap* symbolMap = new GfxFeatureMap();
   char* localMapString = NULL;
   int32 wayfinderSMSVersion = MAX_INT32;
   int32 wayfinderOriginLat = MAX_INT32;
   int32 wayfinderOriginLon = MAX_INT32;
   int32 wayfinderDestinationLat = MAX_INT32;
   int32 wayfinderDestinationLon = MAX_INT32;
   MC2String wapPushSIMessage;
   MC2String wapPushSIHref;
   MC2String wayfinderOriginDescription;
   MC2String wayfinderDestinationDescription;
   MC2String errorCode;
   MC2String errorMessage;
   int32 wayfinderFavouriteLat;
   int32 wayfinderFavouriteLon;
   MC2String wayfinderFavouriteName;
   MC2String wayfinderFavouriteShortName;
   MC2String wayfinderFavouriteDescription;
   MC2String wayfinderFavouriteCategory;
   MC2String wayfinderFavouriteMapIconName;
   MC2String smsType;

   // check for alternate senderService in prop file
   if (Properties::getProperty("SMS_SEND_SERVICE") != NULL)
      senderService = Properties::getProperty("SMS_SEND_SERVICE");

   // Create send_sms_reply element
   DOMElement* send_sms_reply = 
      reply->createElement( X( "send_sms_reply" ) );
   // Transaction ID
   send_sms_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id"  ) )->getNodeValue() );
   out->appendChild( send_sms_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), send_sms_reply );
   }

   // Look for Wayfinder sms version
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      if ( XMLString::equals( attribute->getNodeName(),
                              "wayfinder_sms_version" ) ) {
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         wayfinderSMSVersion = atoi( tmpStr );

         delete[] tmpStr;
      }
   }
      
   // Go through the elements and handle them.
   for ( DOMNode* child = cur->getFirstChild();
         child != NULL && ok;
         child = child->getNextSibling() ) {
      if ( child->getNodeType() != DOMNode::ELEMENT_NODE ) {
         continue;
      }

      // See if the element is a known type
      if ( XMLString::equals( child->getNodeName(),
                              "phone_number" ) ) {
         phoneNumber = XMLUtility::getChildTextValue( child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "smsmessage" ) ) {
         message = XMLUtility::getChildTextValue( child );
         messageSMS = true;
         smsType = "messageSMS";
      } else if ( XMLString::equals( child->getNodeName(),
                                     "route_sms_message" ) ) {
         routeSMS = xmlParseSendSMSRequestRouteSMSMessage( 
                                                          child, model, wapLink );
         if ( !wapLink ) {
            smsType = "routeSMSes";
         } else {
            smsType = "WAP link routeSMS";
         }
         if ( !routeSMS ) {
            mc2log << info << "SendSMS: Error failed parsing "
                   << "route sms message" << endl;
         }
      } else if ( XMLString::equals( child->getNodeName(),
                                     "route_message_data" ) ) {
         routeSMS =
            XMLSMSCommon::
            xmlParseSendSMSRequestRouteMessageData( child, routeID,
                                                    routeCreateTime,
                                                    language,
                                                    signature,
                                                    originString,
                                                    originLocationString,
                                                    destinationString,
                                                    destinationLocationString );

      } else if ( XMLString::equals( child->getNodeName(),
                                     "local_map_sms_settings" ) ) {
         localMapSMSLink = xmlParseLocalMapSMSSettings( child,
                                                        model );
         if ( !localMapSMSLink ) {
            mc2log << info << "SendSMS: Error failed parsing "
                   << "local map sms settings" << endl;
         }
      } else if ( XMLString::equals( child->getNodeName(),
                                     "local_map_data" ) ) {
         localMapSMSLink = 
            xmlParseLocalMapData( child, language, signature,
                                  lmBoundingbox,
                                  localMapString, symbolMap );
         smsType = "WAP link local map SMS";
      } else if( XMLString::equals( child->getNodeName(),
                                    "wayfinder_route_sms" ) ) {
         wayfinderRouteSMS = 
            XMLSMSCommon::
            xmlParseWayfinderSMS( child, signature,
                                  wayfinderOriginLat, wayfinderOriginLon,
                                  wayfinderOriginDescription,
                                  wayfinderDestinationLat, 
                                  wayfinderDestinationLon,
                                  wayfinderDestinationDescription,
                                  errorCode, errorMessage );
         smsType = "WF Route SMS";
         if( ! wayfinderRouteSMS ) {
            ok = false;
            // errorCode and errorMessage is set by
            // xmlParsWayfinderSMS
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseSendSMSRequest failed to parse "
                   << "wayfinder_route_sms." << endl;
         }
      } else if ( XMLString::equals( child->getNodeName(),
                                     "wayfinder_destination_sms"
                                     ) ) {
         wayfinderDestinationSMS = 
            XMLSMSCommon::
            xmlParseWayfinderSMS( child, signature,
                                  wayfinderOriginLat, wayfinderOriginLon,
                                  wayfinderOriginDescription,
                                  wayfinderDestinationLat, wayfinderDestinationLon,
                                  wayfinderDestinationDescription, 
                                  errorCode, errorMessage );
         smsType = "WF Destination SMS";
         if ( ! wayfinderDestinationSMS ) {
            ok = false;
            // errorCode and errorMessage is set by 
            // xmlParseWayfinderDestinationSMS
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseSendSMSRequest failed to parse "
                   << "wayfinder_destination_sms." << endl;
         }
      } else if( XMLString::equals( child->getNodeName(),
                                    "wayfinder_favourite_sms" ) ) {
         wayfinderFavouriteSMS = 
            XMLSMSCommon::
            xmlParseWayfinderFavouriteSMS( child, signature,
                                           wayfinderFavouriteLat,
                                           wayfinderFavouriteLon,
                                           wayfinderFavouriteName,
                                           wayfinderFavouriteShortName,
                                           wayfinderFavouriteDescription,
                                           wayfinderFavouriteCategory,
                                           wayfinderFavouriteMapIconName,
                                           errorCode, errorMessage );
         smsType = "WF Favourite SMS";
         if( ! wayfinderFavouriteSMS ) {
            // errorCode and errorMessage is set by 
            // xmlParseWayfinderFavouriteSMS
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseSendSMSRequest failed to parse "
                   << "wayfinder_favourite_sms." << endl;
         }
      } else if (XMLString::equals( child->getNodeName(),
                                    "wap_push_service_indication") ) {
         wapPushServiceIndication = 
            xmlParseWapPushServiceIndication( child,
                                              wapPushSIMessage,
                                              wapPushSIHref,
                                              errorCode,
                                              errorMessage);
         smsType = "WF Push SMS";
         if (! wapPushServiceIndication ) {
            ok = false;
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseSendSMSRequest failed to parse "
                   << "wap_push_service_indication." << endl;
         }
      } else {
         mc2log << warn << "XMLParserThread::xmlParseSendSMSRequest "
                << "odd Element in send_sms_request "
                << "element: "
                << child->getNodeName() << endl;
      }

   }

   using XMLServerUtility::appendStatusNodes;

   if ( ok && phoneNumber != NULL ) {
      // Add countrycode if it isn't there already
      phoneNumber = StringTableUtility::setDefaultCountryCode( phoneNumber, StringTable::SWEDEN_CC );

      // The answer and request
      PacketContainer* ansCont = NULL;
      SMSSendRequest* smsReq = NULL;
      bool allOK = true;
      const char* errorMsg = "Error sending SMS.";

      if ( messageSMS ) {
         if ( message != NULL ) {
            // Send SMS
            smsReq = new SMSSendRequest( getNextRequestID() );
            SMSSendRequestPacket* p = new SMSSendRequestPacket();

            p->fillPacket( CONVERSION_TEXT, senderService,
                           phoneNumber, MIN( strlen( message ), 
                                             MAX_SMS_SIZE ),
                           (byte*)message );
            smsReq->addNewSMS( p );
         } else {
            // Nothing to send
            allOK = false;
            errorMsg = "Nothing to send.";
         }
      } else if ( routeSMS ) {
         if ( !wapLink ) {
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
                                ROUTE_TYPE_ITEM_STRING );
            PacketContainer* expandRouteCont = NULL;
            ExpandRouteRequest* expandReq = NULL;

            cont = getStoredRouteAndExpand( 
               routeID, routeCreateTime, expandType, language,
               true, false, true, false,
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

               VanillaMatch* oVM =
                  new VanillaCompanyMatch(IDPair_t(0,0),
                                          originString,
                                          originLocationString,
                                          MAX_UINT16, 0);
               VanillaMatch* dVM =
                  new VanillaCompanyMatch(IDPair_t(0,0),
                                          destinationString,
                                          destinationLocationString,
                                          MAX_UINT16, 0);

               // Make route SMS(s)
               SMSUserSettings* smsUserFormat = new SMSUserSettings();
               SMSFormatter::setSMSUserSettingsFromCellularPhoneModel(
                  smsUserFormat, model, language );
               bool concatenatesSMS = 
                  (model->getSMSConcatenation() == 
                   CellularPhoneModel::SMSCONCATENATION_YES || 
                   model->getSMSConcatenation() == 
                   CellularPhoneModel::SMSCONCATENATION_RECEIVE_ONLY);
               
               smsReq = SMSFormatter::getRouteSMSes( 
                  expandPack, senderService, phoneNumber, concatenatesSMS,
                  getNextRequestID(), oVM, NULL, dVM, NULL, 
                  smsUserFormat );

               // Add signature, add new sms with signature
               if ( signature != NULL && signature[ 0 ] != '\0' ) {
                  if ( strlen( signature ) > MAX_SMS_SIZE ) {
                     signature[ MAX_SMS_SIZE ] = '\0';
                  }
                  char data[ MAX_SMS_SIZE + 1 ];
                  SMSFormatter::printSMSString( 
                     data, signature,
                     smsUserFormat->getLanguage(),
                     smsUserFormat->getEOLType(),
                     smsUserFormat->getSMSLineLength(),
                     0, MAX_UINT32, false );
                  
                  SMSSendRequestPacket* sigPack = 
                     new SMSSendRequestPacket();
                  sigPack->fillPacket( CONVERSION_TEXT, senderService, 
                                       phoneNumber, strlen( data ), 
                                       (byte*) data );
                  smsReq->addNewSMS( sigPack );
               }

               delete smsUserFormat;
               delete oVM;
               delete dVM;
            } else {
               // Error
               allOK = false;
               errorMsg = 
                  "Error while obtaining stored roure and expanding it.";
            }

            delete cont;
            delete routePack;
            delete expandReq;
            delete expandRouteCont;
         } else {
            // WAP link SMS
            // The hostname of this server.
            const char* host = "localhost.localdomain";
            if ( Properties::getProperty( "DEFAULT_WEB_HOST" ) != NULL ) {
               host = Properties::getProperty( "DEFAULT_WEB_HOST" ); 
            }
            smsReq = HttpUtility::makeRouteLinkSMS( 
               signature,
               host, "http",
               routeID, routeCreateTime,
               language, model, 
               getNextRequestID(),
               senderService, phoneNumber );
         }
      } else if ( localMapSMSLink ) {
         // Local map WAP link SMS
         // The hostname of this server.
         const char* host = "localhost.localdomain";
         if ( Properties::getProperty( "DEFAULT_WEB_HOST" ) != NULL ) {
            host = Properties::getProperty( "DEFAULT_WEB_HOST" ); 
         }
         smsReq = HttpUtility::makeLocalMapLinkSMS( 
            signature,
            host, "http",
            lmBoundingbox.getMaxLat(), lmBoundingbox.getMinLon(),
            lmBoundingbox.getMinLat(), lmBoundingbox.getMaxLon(),
            symbolMap, language, model, 
            getNextRequestID(),
            senderService, phoneNumber );
      } else if ( wayfinderDestinationSMS || wayfinderRouteSMS ) {
         WayfinderSMSRequest* wayReq = new WayfinderSMSRequest( 
            getNextRequestID() );
         smsReq = wayReq; // smsReq is sent below
         MC2Coordinate origin( wayfinderOriginLat, wayfinderOriginLon );
         MC2Coordinate dest( wayfinderDestinationLat, 
                             wayfinderDestinationLon );
         wayReq->addMessage( 
            senderService, phoneNumber,
            origin,
            wayfinderOriginDescription.c_str(),
            dest,
            wayfinderDestinationDescription.c_str(),
            signature, wayfinderSMSVersion );
      } else if( wayfinderFavouriteSMS ) {
         WayfinderSMSRequest* wayReq = new WayfinderSMSRequest(
            getNextRequestID() );
         smsReq = wayReq;
         MC2Coordinate coord( wayfinderFavouriteLat, wayfinderFavouriteLon );

         wayReq->addFavouriteMessage( senderService, phoneNumber,
                                      coord,
                                      wayfinderFavouriteName,
                                      wayfinderFavouriteShortName,
                                      wayfinderFavouriteDescription,
                                      wayfinderFavouriteCategory,
                                      wayfinderFavouriteMapIconName,
                                      signature,
                                      wayfinderSMSVersion );
      } else if ( wapPushServiceIndication ) {
         smsReq = new SMSSendRequest( getNextRequestID() );
         SMSSendRequestPacket* p = new SMSSendRequestPacket();

         p->fillPacket( senderService, phoneNumber, wapPushSIHref.c_str(),
                        wapPushSIMessage.c_str());
         smsReq->addNewSMS( p );
      } else {
         allOK = false;
         errorMsg = 
            "Couldn't make out what to send.";
      }

      // Send SMS(s)
      if ( allOK ) {
         char messages[smsReq->getNbrSMSs()][255];
         char* sendPhone = NULL;
         char* receivePhone = NULL;
         int dataLength = 0;
         for ( uint32 i = 0 ; i < smsReq->getNbrSMSs() ; i++ ) {
            messages[ i ][ 0 ] = '\0';

            SMSSendRequestPacket* sendPack = 
               smsReq->getSMSSendRequestPacket( i );
            if ( sendPack != NULL ) {
               byte* data = NULL;
               sendPack->getPacketContents( CONVERSION_NO,
                                            sendPhone, receivePhone,
                                            dataLength, data );
               memcpy( messages[ i ], data, dataLength );
               messages[ i ][ dataLength ] = '\0';
               mc2log << info << "messages " << messages[ i ] << endl;
               delete [] data;
            } 
         }

         ThreadRequestContainer* reqCont = 
            new ThreadRequestContainer( smsReq );
         mc2dbg8 << "About to send SMSSendRequest" << endl;
         putRequest( reqCont );
         mc2dbg8 << "SMSSendRequest returned" << endl;

         for ( uint32 i = 0 ; i < smsReq->getNbrSMSs() ; i++ ) {
            ansCont = smsReq->getAnswer();
            if ( ansCont != NULL &&
                 static_cast<ReplyPacket*>( ansCont->getPacket() )
                 ->getStatus() == StringTable::OK ) 
            {
               // OK
            } else {
               allOK = false;
            }
            // Debit
            getDebitHandler()->makeMessageDebit( 
               m_user, smsReq, ansCont, phoneNumber, messages[ i ], 1 );
         }

         if ( allOK ) {
            
            // All OK
            appendStatusNodes( send_sms_reply, reply, 
                               indentLevel + 1, indent,
                               "0", "OK" );
            mc2log << "XMLParserThread::xmlParseSendSMSRequest ";
            mc2log << smsType;
            mc2log << " phoneNumber " << phoneNumber 
                   << " nbrSMSes " << smsReq->getNbrSMSs() << endl;
                  
         } else {
            // Set statusnodes with errormessage
            const char* statusMessage = NULL;
            if ( ansCont != NULL ) {
               statusMessage = StringTable::getString( 
                  StringTable::stringCode( static_cast<ReplyPacket*>( 
                     ansCont->getPacket() )->getStatus() ),
                  StringTable::ENGLISH );
            } else {
               statusMessage = "Request failed.";
            }
            mc2log << warn << "XMLParserThread::xmlParseSendSMSRequest "
                   << smsType <<  "sending falied falied " 
                   << statusMessage << endl;
            appendStatusNodes( send_sms_reply, reply, indentLevel + 1, 
                               indent,
                               "-1", statusMessage );
         }

         delete smsReq;
         delete ansCont;
         delete reqCont;
      } else {
         mc2log << warn << "XMLParserThread::xmlParseSendSMSRequest "
                << smsType << "request falied " << errorMsg << endl;
         appendStatusNodes( send_sms_reply, reply, indentLevel + 1, 
                            indent,
                            "-1", errorMsg );
      }
   } else {
      if ( ok ) {
         appendStatusNodes( send_sms_reply, reply, indentLevel + 1, indent,
                            "-1", "Error in inparameters." );
         mc2log << info << "SendSMS " << smsType
                << "Error in inparameters." << endl;
      } else {
         // An error in errorCode, errorMessage
         appendStatusNodes( send_sms_reply, reply, 
                            indentLevel + 1, indent, 
                            errorCode.c_str(), errorMessage.c_str() );
         // Error handled 
         ok = true;
         mc2log << info << "SendSMS " << smsType 
                << errorCode << "," << errorMessage << endl;
      }
   }

   if ( indent ) {
      // Newline and indent before end send_sms_reply tag   
      send_sms_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   delete [] phoneNumber;
   delete [] message;
   delete [] signature;
   delete [] originString;
   delete [] originLocationString;
   delete [] destinationString;
   delete [] destinationLocationString;
   delete model;
   delete symbolMap;
   delete [] localMapString;
   
   return ok;
}


bool 
XMLParserThread::xmlParseSendSMSRequestRouteSMSMessage( 
   DOMNode* cur,
   CellularPhoneModel*& model,
   bool& wapLink )
{
   bool ok = true;
   
   if ( XMLString::equals( cur->getNodeName(), "route_sms_message" ) ) {
      // Attributes
      DOMNamedNodeMap* attributes = cur->getAttributes();
      DOMNode* attribute;
   
      for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
         attribute = attributes->item( i );
         
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         if ( XMLString::equals( attribute->getNodeName(), "wap_link" ) ) {
            wapLink = StringUtility::checkBoolean( tmpStr );
         } else {
            mc2log << warn << "XMLParserThread::"
                      "xmlParseSendSMSRequestRouteSMSMessage "
                      "unknown attribute for route_sms_message element "
                   << "Name " << attribute->getNodeName()
                   << " Type " << attribute->getNodeType() 
                   << " Value " << tmpStr << endl;
         }
         delete [] tmpStr;
      }

      // Children
      char* phoneManufacturerName = NULL;
      char* phoneModelName = NULL;

      DOMNode* child = cur->getFirstChild();
   
      while ( child != NULL && ok ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(),
                                       "phone_manufacturer" ) ) 
               {
                  phoneManufacturerName = 
                     XMLUtility::getChildTextValue( child );
               } else if ( XMLString::equals( child->getNodeName(),
                                              "phone_model" ) ) 
               {
                  phoneModelName = XMLUtility::getChildTextValue( child );
               } else { // Odd element in route_data element
                  mc2log << warn << "XMLParserThread::"
                     "xmlParseSendSMSRequestRouteSMSMessage "
                     "odd Element in route_sms_message"
                     " element: "
                         << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            default:
               mc2log << warn << "XMLParserThread::"
                  "xmlParseSendSMSRequestRouteSMSMessage "
                  "odd node type in route_sms_message"
                  " element: "
                      << child->getNodeName() 
                      << " type " << child->getNodeType() << endl;
               break;
         }
         child = child->getNextSibling();
      }

      
      if ( ok && phoneManufacturerName != NULL && phoneModelName != NULL )
      {
         CellularPhoneModelsElement* models = 
            m_group->getCurrentCellularPhoneModels( this );
         model = models->findModel( phoneModelName );
         if ( model == NULL ) {
            model = new CellularPhoneModel( 
               "UNKNOWN", "Default",
               16, UserConstants::EOLTYPE_AlwaysCRLF,
               3, true, 50, 50, 
               CellularPhoneModel::SMSCAPABLE_YES,
               CellularPhoneModel::SMSCONCATENATION_NO,
               CellularPhoneModel::SMSGRAPHICS_NO,
               CellularPhoneModel::WAPCAPABLE_NO,
               "", 2000, "" );;
         } else {
            // Copy
            model = new CellularPhoneModel( *model );
         }
         m_group->releaseCacheElement( models );
      }

      delete [] phoneManufacturerName;
      delete [] phoneModelName;
   } else {
      ok = false;
   }

   return ok;
}

bool 
XMLParserThread::xmlParseLocalMapSMSSettings( DOMNode* cur,
                                              CellularPhoneModel*& model )
{
   bool ok = true;
   
   if ( XMLString::equals( cur->getNodeName(),
                           "local_map_sms_settings" ) ) 
   {
      // Children
      char* phoneManufacturerName = NULL;
      char* phoneModelName = NULL;

      DOMNode* child = cur->getFirstChild();
   
      while ( child != NULL && ok ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(),
                                       "phone_manufacturer" ) ) 
               {
                  phoneManufacturerName = XMLUtility::getChildTextValue( 
                     child );
               } else if ( XMLString::equals( child->getNodeName(),
                                              "phone_model" ) ) 
               {
                  phoneModelName = XMLUtility::getChildTextValue( child );
               } else { // Odd element in route_data element
                  mc2log << warn << "XMLParserThread::"
                     "xmlParseLocalMapSettings "
                     "odd Element in route_sms_message"
                     " element: "
                         << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            default:
               mc2log << warn << "XMLParserThread::"
                  "xmlParseSendSMSRequestRouteSMSMessage "
                  "odd node type in local_map_settings"
                  " element: "
                      << child->getNodeName() 
                      << " type " << child->getNodeType() << endl;
               break;
         }
         child = child->getNextSibling();
      }

      
      if ( ok && phoneManufacturerName != NULL && phoneModelName != NULL )
      {
         CellularPhoneModelsElement* models = 
            m_group->getCurrentCellularPhoneModels( this );
         model = models->findModel( phoneModelName );
         if ( model == NULL ) {
            model = new CellularPhoneModel( 
               "UNKNOWN", "Default",
               16, UserConstants::EOLTYPE_AlwaysCRLF,
               3, true, 50, 50, 
               CellularPhoneModel::SMSCAPABLE_YES,
               CellularPhoneModel::SMSCONCATENATION_NO,
               CellularPhoneModel::SMSGRAPHICS_NO,
               CellularPhoneModel::WAPCAPABLE_NO,
               "", 2000, "" );;
         } else {
            // Copy
            model = new CellularPhoneModel( *model );
         }
         m_group->releaseCacheElement( models );
      }

      delete [] phoneManufacturerName;
      delete [] phoneModelName;
   } else {
      ok = false;
   }

   return ok;
}

bool 
XMLParserThread::xmlParseWapPushServiceIndication( DOMNode* cur,
                                                   MC2String& message,
                                                   MC2String& href,
                                                   MC2String& errorCode, 
                                                   MC2String& errorMessage )
{
   bool ok = true;

   if ( XMLString::equals( cur->getNodeName(),
                           "wap_push_service_indication" ) ) 
   {
      message = XMLUtility::getChildTextStr( *cur );

      // Attributes
      DOMNamedNodeMap* attributes = cur->getAttributes();
      DOMNode* attribute;
   
      for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
         attribute = attributes->item( i );
      
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         if ( XMLString::equals( attribute->getNodeName(),
                                 "href" ) ) 
         {
            href = tmpStr;
         } else {
            mc2log << warn << "XMLParserThread::"
                      "xmlParseWapPushServiceIndication "
                      "unknown attribute for wap_push_service_indication "
                      "element "
                   << " Name " << attribute->getNodeName()
                   << " Type " << attribute->getNodeType() << endl;
         }
         delete [] tmpStr;
      }

      // Check children
      DOMNode* child = cur->getFirstChild();
   
      while ( child != NULL && ok ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               { // Odd element in 
                  mc2log << warn << "XMLParserThread::"
                            "xmlParseWapPushServiceIndication "
                            "odd Element in wap_push_service_indication"
                            " element: "
                         << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            case DOMNode::TEXT_NODE :
               // Ignore stray texts
               break;
            default:
               mc2log << warn << "XMLParserThread::"
                         "xmlParseWapPushServiceIndication "
                         "odd node type in wap_push_service_indication"
                         " element: "
                      << child->getNodeName() 
                      << " type " << child->getNodeType() << endl;
               break;
         }
         child = child->getNextSibling();
      }
      
   } else {
      errorCode = "-1";
      errorMessage = "wap_push_service_indication element is not named "
         "wap_push_service_indication.";
      ok = false;
   }

   return ok;
}


#endif // USE_XML

