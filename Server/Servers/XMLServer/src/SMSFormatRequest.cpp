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
#include "ExpandStringItemVector.h"
#include "ExpandRouteRequest.h"
#include "HttpUtility.h"
#include "UserData.h"
#include "SMSPacket.h"
#include "SMSSendRequest.h"
#include "WayfinderSMSRequest.h"
#include "Properties.h"
#include "RoutePacket.h"
#include "ExpandRoutePacket.h"
#include "XMLServerElements.h"
#include "XMLTool.h"
#include "XMLSMSCommon.h"
#include "FixedSizeString.h"
#include "CoordinateTransformer.h"
#include "GfxUtility.h"
#include "STLStringUtility.h"
#include "XMLSearchUtility.h"
#include "SearchMatch.h"

namespace {

bool parsePlaceSMS( XMLSMSCommon::PlaceSMSData& placeData,
                    const DOMNode* child,
                    MC2String& errorCode, MC2String& errorMessage ) {

   XMLTool::getAttribValue( placeData.m_type, "type", child );
   const DOMNode* posNode = XMLTool::findNodeConst( child, "position_item" );
   uint16 angle = 0; // not used
   MC2Coordinate placeCoord;
   if ( ! posNode ) {
      // see if we have a search_item node
      const DOMNode* searchItem = XMLTool::findNodeConst( child, "search_item" );
      if ( searchItem != NULL ) {
         placeData.m_match.
            reset( XMLSearchUtility::
                   getVanillaMatchFromSearchItem( searchItem,
                                                  errorCode, errorMessage
                                                  ) );
         XMLTool::getNodeValue( placeData.m_language, "language", child,
                                LangTypes::english );
      } else {
         errorMessage.insert( 0, "problem with position_item or search_item" );
         return false;
      }
   } else if ( ! XMLCommonElements::
               getPositionItemData( posNode,
                                    placeCoord, angle,
                                    errorCode, errorMessage ) ) {
      errorMessage.
         insert( 0, "problem with position_item in place_sms" );
      return false;
   }
   // success
   return true;
}

bool handleRouteMessage( ParserThread* thread,
                         uint32 routeID,
                         uint32 routeCreateTime,
                         StringTable::languageCode language,
                         StringVector* smsVector,
                         const CellularPhoneModel* model,
                         const char* signature,
                         MC2String& errorMessage );

bool handleWAPLinkSMS( ParserThread* thread,
                       uint32 routeID, uint32 routeCreateTime,
                       StringTable::languageCode language,
                       StringVector* smsVector,
                       const CellularPhoneModel* model,
                       const char* signature,
                       MC2String& errorMessage );

} // anonymous

bool
XMLParserThread::xmlParseSMSFormatRequest( DOMNode* cur, 
                                           DOMNode* out,
                                           DOMDocument* reply,
                                           bool indent )
try {
   bool ok = true;
   int indentLevel = 1;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   bool smsMessage = false;
   bool routeMessage = false;
   bool wayfinderRouteSMS = false;
   bool wayfinderDestinationSMS = false;
   bool wayfinderFavouriteSMS = false;
   char* smsMessageText = NULL;
   char* phoneModelName = NULL;
   char* phoneManufacturerName = NULL;
   uint32 routeID = 0;
   uint32 routeCreateTime = 0;
   StringTable::languageCode language = StringTable::ENGLISH;
   char* signature = NULL;
   char* originString = NULL;
   char* originLocationString = NULL;
   char* destinationString = NULL;
   char* destinationLocationString = NULL;
   CellularPhoneModel* model = NULL; // For routeMessage
   bool wapLink = false; // For routeMessage
   int32 wayfinderSMSVersion = MAX_INT32;
   int32 wayfinderOriginLat = MAX_INT32;
   int32 wayfinderOriginLon = MAX_INT32;
   int32 wayfinderDestinationLat = MAX_INT32;
   int32 wayfinderDestinationLon = MAX_INT32;
   MC2String wayfinderOriginDescription;
   MC2String wayfinderDestinationDescription;
   MC2String errorCode = "-1";
   MC2String errorMessage = "Failed to handle request.";
   int32 wayfinderFavouriteLat;
   int32 wayfinderFavouriteLon;
   MC2String wayfinderFavouriteName;
   MC2String wayfinderFavouriteShortName;
   MC2String wayfinderFavouriteDescription;
   MC2String wayfinderFavouriteCategory;
   MC2String wayfinderFavouriteMapIconName;
   // Create sms_format_reply element
   DOMElement* sms_format_reply = 
      reply->createElement( X( "sms_format_reply" ) );
   // Transaction ID
   sms_format_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( sms_format_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( reply->createTextNode( XindentStr.XMLStr() ), 
                         sms_format_reply );
   }
   
   // Look for Wayfinder sms version
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      if ( XMLString::equals( attribute->getNodeName(),
                              "wayfinder_sms_version" ) ) {
         MC2String tmpStr = 
            XMLUtility::transcodefrom( attribute->getNodeValue() );
         wayfinderSMSVersion = atoi( tmpStr.c_str() );
      }
   }

   XMLSMSCommon::InviteData inviteData; // for invite_sms

   XMLSMSCommon::PlaceSMSData placeData; // for place_sms

   for ( DOMNode* child = cur->getFirstChild();
         child != NULL && ok; 
         child = child->getNextSibling() ) {
      if ( child->getNodeType() != DOMNode::ELEMENT_NODE ) {
         continue;
      }
      // See if the element is a known type
      if ( XMLString::equals( child->getNodeName(), "smsmessage" ) ) {
         smsMessageText = XMLUtility::getChildTextValue( child );
         smsMessage = true;
      } else if ( XMLString::equals( child->getNodeName(),
                                     "phone_manufacturer" ) ) {
         phoneManufacturerName = 
            XMLUtility::getChildTextValue( child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "phone_model" ) ) {
         phoneModelName = XMLUtility::getChildTextValue( child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "route_sms_message" ) ) {
         routeMessage = 
            xmlParseSendSMSRequestRouteSMSMessage( child, model, wapLink );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "route_message_data" ) ) {
         routeMessage = 
            XMLSMSCommon::
            xmlParseSendSMSRequestRouteMessageData( child, 
                                                    routeID,
                                                    routeCreateTime,
                                                    language,
                                                    signature,
                                                    originString,
                                                    originLocationString,
                                                    destinationString,
                                                    destinationLocationString );

      } else if( XMLString::equals( child->getNodeName(),
                                    "wayfinder_route_sms" ) ) {
         wayfinderRouteSMS = 
            XMLSMSCommon::
            xmlParseWayfinderSMS( child, signature,
                                  wayfinderOriginLat,
                                  wayfinderOriginLon,
                                  wayfinderOriginDescription,
                                  wayfinderDestinationLat,
                                  wayfinderDestinationLon,
                                  wayfinderDestinationDescription,
                                  errorCode, errorMessage );
         if ( ! wayfinderRouteSMS ) {
            ok = false;
            // errorCode and errorMessage is set by 
            // xmlParseWayfinderSMS
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseSendSMSRequest failed to parse "
                   << "wayfinder_route_sms." << endl;
         }              
      } else if ( XMLString::equals( child->getNodeName(),
                                     "wayfinder_destination_sms" ) ) {
         wayfinderDestinationSMS = 
            XMLSMSCommon::
            xmlParseWayfinderSMS( child, signature,
                                  wayfinderOriginLat, wayfinderOriginLon,
                                  wayfinderOriginDescription,
                                  wayfinderDestinationLat, 
                                  wayfinderDestinationLon,
                                  wayfinderDestinationDescription, 
                                  errorCode, errorMessage );
         if ( ! wayfinderDestinationSMS ) {
            ok = false;
            // errorCode and errorMessage is set by 
            // xmlParseWayfinderSMS
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
         if( ! wayfinderFavouriteSMS ) {
            // errorCode and errorMessage is set by 
            // xmlParseWayfinderFavouriteSMS
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseSMSFormatRequest failed to parse "
                   << "wayfinder_favourite_sms." << endl;
         }
      } else if ( XMLString::equals( child->getNodeName(),
                                     "invite_sms" ) ) {
         XMLTool::getAttribValue( inviteData.m_type, "type", child );
         XMLTool::getNodeValue( inviteData.m_name, "name", child );
      } else if ( XMLString::equals( child->getNodeName(),
                                     "place_sms" ) ) {
         ok = parsePlaceSMS( placeData, child, errorCode, errorMessage );
      } else {
         mc2log << warn << "XMLParserThread::"
            "xmlParseSMSFormatRequest "
            "odd Element in sms_format_request element: "
                << child->getNodeName() << endl;
      }
   }

   
   // Handle request
   if ( ok ) {
      // The smses
      StringVector* smsVector = new StringVector();

      if ( smsMessage ) {
         // ExpandStringItemVector?
         ok = false;
         errorMessage = "Formating of smsmessage is not yet supported.";
      } else if ( routeMessage ) {
         if ( !wapLink ) {
            ok = handleRouteMessage( this, 
                                     routeID, routeCreateTime, 
                                     language,
                                     smsVector, model, signature, 
                                     errorMessage );
         } else {
            ok = handleWAPLinkSMS( this,
                                   routeID, routeCreateTime,
                                   language, smsVector,
                                   model, signature,
                                   errorMessage );
         }
         
         if ( ok ) {
            mc2log << info << "XMLParserThread::xmlParseSMSFormatRequest ";
            if ( !wapLink ) {
               mc2log << "routeID " << routeID << " routeCreateTime "
                      << routeCreateTime;
            } else {
               mc2log << "wapLink true";
            }
            mc2log << " signature " << signature << " model " 
                   << model->getName() 
                   << " nbrSMSes " << smsVector->getSize() << endl;

            // Add smses
            XMLSMSCommon::appendSMSList( sms_format_reply, reply, smsVector,
                                         indentLevel + 1, indent );
         } // Not ok handled below

      } else if ( wayfinderDestinationSMS || wayfinderRouteSMS ) {
         // Make sms data
         WayfinderSMSRequest* wayReq = new WayfinderSMSRequest( 
            getNextRequestID() );
         MC2Coordinate origin( wayfinderOriginLat, wayfinderOriginLon );
         MC2Coordinate dest( wayfinderDestinationLat,
                             wayfinderDestinationLon );
         wayReq->addMessage( 
            "DUMMY", "NO_WAY",
            origin,
            wayfinderOriginDescription.c_str(),
            dest,
            wayfinderDestinationDescription.c_str(),
            signature, wayfinderSMSVersion );
         smsVector->addLast( StringUtility::newStrDup( const_cast<char*>( 
            wayReq->getLastDestinationMessage() ) ) );
         // Add sms
         XMLSMSCommon::appendSMSList( sms_format_reply, reply, smsVector,
                                      indentLevel + 1, indent );

         delete wayReq;
      } else if( wayfinderFavouriteSMS ) {
         // Make sms data
         WayfinderSMSRequest* wayReq = new WayfinderSMSRequest(
            getNextRequestID() );
         
         MC2Coordinate coord( wayfinderFavouriteLat, wayfinderFavouriteLon );

         wayReq->addFavouriteMessage( "DUMMY", "NO_WAY",
                                      coord,
                                      wayfinderFavouriteName,
                                      wayfinderFavouriteShortName,
                                      wayfinderFavouriteDescription,
                                      wayfinderFavouriteCategory,
                                      wayfinderFavouriteMapIconName,
                                      signature,
                                      wayfinderSMSVersion );
         smsVector->addLast( StringUtility::newStrDup( const_cast<char*>( 
            wayReq->getLastDestinationMessage() ) ) );
         // Add sms
         XMLSMSCommon::appendSMSList( sms_format_reply, reply, smsVector,
                                      indentLevel + 1, indent );
         delete wayReq;
      } else if ( !inviteData.m_type.empty() ) { 
         XMLSMSCommon::
            composeInviteSMS( sms_format_reply, reply, indentLevel + 1, indent,
                              getCurrentUser(), inviteData );
      } else if ( ! placeData.m_type.empty() ) {
         XMLSMSCommon::
            composePlaceSMS( *this,
                             sms_format_reply, reply, indentLevel + 1, indent,
                             getCurrentUser(), placeData );
      } else {
         // Couldn't get all needed indata
         ok = false;
         errorMessage = "Couldn't get all needed indata.";
      }

      if ( ok ) {
         mc2log << info << "SMSFormat: OK " << smsVector->getSize()
                << " SMSes ";
         if ( smsVector->getSize() > 0 ) {
            mc2log << "Last " << smsVector->getElementAt( 0 );
         }
         mc2log << endl;
      } // Error printed below

      smsVector->deleteAllObjs();
      delete smsVector;

   } // Not ok handled below

   if ( ! ok ) {
      mc2log << info << "SMSFormat: Error " << errorCode << ","
             << errorMessage << endl;
      // Error 
      XMLServerUtility::
         appendStatusNodes( sms_format_reply, reply, indentLevel + 1, indent,
                            errorCode.c_str(), errorMessage.c_str() );
      // Error handled
      ok = true;
   }

   if ( indent ) {
      // Newline and indent before end sms_format_reply tag   
      sms_format_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }
 
   delete [] smsMessageText;
   delete [] phoneModelName;
   delete [] phoneManufacturerName;
   delete [] signature;
   delete [] originString;
   delete [] originLocationString;
   delete [] destinationString;
   delete [] destinationLocationString;
   delete model;

   return ok;
} catch ( const XMLTool::Exception& e ) {
   
   return false;
}

namespace {

bool handleRouteMessage( ParserThread* thread,
                         uint32 routeID,
                         uint32 routeCreateTime,
                         StringTable::languageCode language,
                         StringVector* smsVector,
                         const CellularPhoneModel* model,
                         const char* signature,
                         MC2String& errorMessage ) {

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
   bool ok = true;
   cont = thread->
      getStoredRouteAndExpand( routeID, routeCreateTime, expandType, 
                               language,
                               true, false, true, false,
                               routePack, UIN, extraUserinfo, validUntil,
                               originLat, originLon, originMapID, 
                               originItemID, 
                               originOffset,
                               destinationLat, destinationLon, 
                               destinationMapID, 
                               destinationItemID, destinationOffset,
                               expandReq, expandRouteCont );

   if ( cont != NULL && expandRouteCont != NULL &&
        static_cast< ReplyPacket* > ( expandRouteCont->getPacket() )
        ->getStatus() == StringTable::OK ) {
      ExpandRouteReplyPacket* expandPack = 
         static_cast<ExpandRouteReplyPacket*>( expandRouteCont->getPacket() );
      ExpandStringItem** expStrItems = 
         expandPack->getStringDataItem();
         

      // ExpandStringItemVector
      ExpandStringItemVector* expander = 
         new ExpandStringItemVector( expandPack->getNumStringData() );
      // Fill ExpandStringItemVector with ExpandStringItems
      for ( uint32 i = 0 ; i < expandPack->getNumStringData() ; i++ ) {
         expander->addLast( expStrItems[ i ] );
      }
      delete [] expStrItems;
      vector<char*> tmpSMSVector;
      if ( expander->getRouteAsSMS( tmpSMSVector,
                                    expandPack->getTotalDist(),
                                    expandPack->getTotalTime(),
                                    expandPack->getStandStillTime(),
                                    StringTable::
                                    getShortLanguageCode( language ), 
                                    model->getEOL(), model->getChars(),
                                    !(model->getSMSConcatenation() == 
                                      CellularPhoneModel::SMSCONCATENATION_NO), 
                                    NULL, signature ) ) {
         for ( size_t i = 0; i < tmpSMSVector.size(); ++i ) {
            smsVector->addLast( tmpSMSVector[ i ] );
         }
         // Ok
      } else {
         // Error
         ok = false;
         errorMessage = "Failed to make route sms.";
      }
      expander->deleteAllObjs();
      delete expander;
   } else {
      // Error
      ok = false;
      errorMessage = 
         "Error while obtaining stored roure and expanding it.";
   }

   delete cont;
   delete routePack;
   delete expandReq;
   delete expandRouteCont;

   return ok;
}

bool 
handleWAPLinkSMS( ParserThread* thread,
                  uint32 routeID, 
                  uint32 routeCreateTime,
                  StringTable::languageCode language,
                  StringVector* smsVector,
                  const CellularPhoneModel* model,
                  const char* signature,
                  MC2String& errorMessage ) {
   bool ok = true;
   // WAP link SMS
   // The hostname of this server.
   const char* host = "localhost.localdomain";
   if ( Properties::getProperty( "DEFAULT_WEB_HOST" ) != NULL ) {
      host = Properties::getProperty( "DEFAULT_WEB_HOST" ); 
   }
   SMSSendRequest* smsReq = 
      HttpUtility::makeRouteLinkSMS( signature,
                                     host, "http",
                                     routeID, routeCreateTime,
                                     language, model, 
                                     thread->getNextRequestID(),
                                     "DUMMY", "46708000000" );

   if ( smsReq != NULL && smsReq->getNbrSMSs() == 1 ) {
      // Get text
      SMSSendRequestPacket* smsPack = 
         smsReq->getSMSSendRequestPacket( 0 );
      char* message = NULL;
      char* sendPhone = NULL;
      char* receivePhone = NULL;
      int dataLength = 0;
      byte* data = NULL;
      smsPack->getPacketContents( CONVERSION_NO,
                                  sendPhone, receivePhone,
                                  dataLength, data );
      message = new char[ dataLength + 1 ];
      memcpy( message, data, dataLength );
      message[ dataLength ] = '\0';
      smsVector->addLast( message );
      delete [] data;
   } else {
      // Error
      ok = false;
      errorMessage = "Failed to make wap link sms.";
   }
   delete smsReq;
   return ok;
}
}

#endif // USE_XML

