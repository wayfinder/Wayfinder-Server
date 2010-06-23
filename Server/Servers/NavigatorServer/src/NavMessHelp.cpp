/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavMessHelp.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "UserData.h"
#include "ExpandRouteRequest.h"
#include "ExpandItemID.h"
#include "ExpandStringItem.h"
#include "ExpandRoutePacket.h"
#include "RoutePacket.h"
#include "LocalMapMessageRequest.h"
#include "MimeMessage.h"
#include "ExpandStringItemVector.h"
#include "SendEmailPacket.h"
#include "SinglePacketRequest.h"
#include "GfxFeatureMap.h"
#include "UserFavoritesRequest.h"
#include "ItemInfoRequest.h"
#include "SearchMatch.h"
#include "CoordinateTransformer.h"
#include "RouteMessageRequest.h"
#include "GfxUtility.h"
#include "CopyrightHandler.h"
#include "ServerTileMapFormatDesc.h"
#include "ParserThreadGroup.h"
#include "DeleteHelpers.h"

NavMessHelp::NavMessHelp( InterfaceParserThread* thread,
                          NavParserThreadGroup* group )
      : NavHandler( thread, group )
{
}


uint8 
NavMessHelp::handleRouteNavSend( RouteID routeID,
                                 MessageType messType,
                                 StringTable::languageCode language,
                                 bool sendMessage,
                                 ImageDrawConfig::imageFormat imageFormat,
                                 RouteMessageRequestType::MessageContent 
                                 messageType,
                                 uint32 maxMessageSize,
                                 uint32 overviewImageWidth,
                                 uint32 overviewImageHeight,
                                 uint32 routeTurnImageWidth,
                                 uint32 routeTurnImageHeight,
                                 const UserItem* userItem,
                                 const char* sender,
                                 const char* signature,
                                 const char* receiver,
                                 bool contentLocation,
                                 byte*& data, uint32& dataLen )
{
   uint8 status = NavReplyPacket::NAV_STATUS_OK;

   // Get route
   RouteReplyPacket* routePack = NULL;
   PacketContainer* routeCont = NULL;
   uint32 expandType = (ROUTE_TYPE_STRING |
                        ROUTE_TYPE_NAVIGATOR |
                        ROUTE_TYPE_ITEM_STRING |
                        ROUTE_TYPE_GFX);
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
   ExpandRouteRequest* expReq = NULL;
   PacketContainer* expandRouteCont = NULL;

   routeCont = m_thread->getStoredRouteAndExpand( 
      routeID.getRouteIDNbr(), routeID.getCreationTime(),
      expandType, language, true, false, true, false/*nameChangeAsWP*/, 
      routePack, UIN, 
      extraUserinfo, validUntil, originLat, originLon, originMapID, 
      originItemID, originOffset, destinationLat, destinationLon,
      destinationMapID, destinationItemID, destinationOffset,
      expReq, expandRouteCont );
            
   if ( routePack == NULL ||
        routePack->getStatus() != StringTable::OK ||
        expandRouteCont == NULL || static_cast< ReplyPacket* > ( 
           expandRouteCont->getPacket() )->getStatus() != 
        StringTable::OK )
   {
      if ( routePack == NULL ||
           routePack->getStatus() == StringTable::TIMEOUT_ERROR )
      {
         status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
      } else if ( expandRouteCont == NULL || 
                  static_cast< ReplyPacket* > ( 
                     expandRouteCont->getPacket() )->getStatus() == 
                  StringTable::TIMEOUT_ERROR )
      {
         status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
      } else {
         status = NavReplyPacket::NAV_STATUS_NOT_OK;
      }
   }

   if ( status == NavReplyPacket::NAV_STATUS_OK ) {
      ExpandRouteReplyPacket* expandPack = 
         static_cast<ExpandRouteReplyPacket*> (  
            expandRouteCont->getPacket() );
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
      
      // RouteMessageRequest or SMSFormatRequest
      if ( messType == SMS ) {
         // SMSFormatRequest
         ExpandStringItem** expStrItems = 
            expandPack->getStringDataItem();
         CellularPhoneModel* model = new CellularPhoneModel( 
            "UNKNOWN", "Default",
            16, UserConstants::EOLTYPE_AlwaysCRLF,
            3, true, 50, 50, 
            CellularPhoneModel::SMSCAPABLE_YES,
            CellularPhoneModel::SMSCONCATENATION_NO,
            CellularPhoneModel::SMSGRAPHICS_NO,
            CellularPhoneModel::WAPCAPABLE_NO,
            "", 2000, "" );
         vector< char* > smsVector;
               
         // ExpandStringItemVector
         ExpandStringItemVector* expander = 
            new ExpandStringItemVector( 
               expandPack->getNumStringData() );
         // Fill ExpandStringItemVector with ExpandStringItems
         for ( uint32 i = 0 ; 
               i < expandPack->getNumStringData() ; i++ )
         {
            expander->addLast( expStrItems[ i ] );
         }
         delete [] expStrItems;
         
         if ( expander->getRouteAsSMS( 
                 smsVector,
                 expandPack->getTotalDist(),
                 expandPack->getTotalTime(),
                 expandPack->getStandStillTime(),
                 StringTable::getShortLanguageCode( language ), 
                 model->getEOL(), model->getChars(),
                 !(model->getSMSConcatenation() == 
                   CellularPhoneModel::SMSCONCATENATION_NO), 
                 NULL, signature ) )
         {
            // Ok make data
            data = new byte[ (161+10) * smsVector.size() ];
            dataLen = 0;
            const char* smsBreak = "\r\n<<<<<<";
            const uint32 smsBreakLen = strlen( smsBreak );
            for ( uint32 i = 0 ; i < smsVector.size() ; i++ ) {
               if ( m_thread->clientUsesLatin1() ) {
                  // Ok, convert to iso and pry
                  MC2String text = smsVector[ i ];
                  MC2String utext = UTF8Util::utf8ToIso( text );
                  memcpy( data + dataLen, utext.c_str(), utext.length() );
                  dataLen += utext.size();
               } else {
                  uint32 len = strlen( smsVector[ i ] );
                  memcpy( data + dataLen, smsVector[ i ], 
                          len );
                  dataLen += len;
               }
               memcpy( data + dataLen, smsBreak, smsBreakLen );
               dataLen += smsBreakLen;
            }
            // This is not really needed but for debug printing
            data[ dataLen ] = '\0'; 
         } else {
            // Error
            status = NavReplyPacket::NAV_STATUS_NOT_OK;
            mc2log << warn << "handleRouteNavSend: Failed to make "
                   << "route sms." << endl;
         }

         expander->deleteAllObjs();
         STLUtility::deleteArrayValues( smsVector );
         delete expander;
         delete model;
      } else {
         // RouteMessageRequest
         // Plaintext email
         bool plainText = false;
         bool makeImages = true;
         if ( messType == non_HTML_email ) {
            sendMessage = false;
            plainText = true;
            makeImages = false;
         }

         uint32 nbrStringItems = expandPack->getNumStringData();
         ExpandStringItem** stringItems = expandPack->getStringDataItem();
         // Make origin string (Baravägen, Lund)
         const char* origin = "";
         char pOrigStr[ 512 ];
         bool originIsCoord = false;
         if ( stringItems[ 0 ]->hasName() ) {
            origin = stringItems[ 0 ]->getText();
         } else {
            // No name use gps or something
            makePositionString( 
               MC2Coordinate( originLat, originLon ), pOrigStr, language );
            origin = pOrigStr;
            originIsCoord = true;
         }

         // Make destination string (Isbergs gata, Malmö)
         const char* dest = "";
         char pDestStr[ 512 ];
         bool destIsCoord = false;
         if ( stringItems[ nbrStringItems -1 ]->hasName() ) {
            dest = stringItems[ nbrStringItems -1 ]->getText();
         } else {
            // No name use gps or something
            makePositionString( MC2Coordinate( destinationLat, 
                                               destinationLon ), 
                                pDestStr, language );
            dest = pDestStr;
            destIsCoord = true;
         }

         // Make subject (Route from Baravägen, Lund to Isbergs gata, Malmö)
         const char* preStr = "Wayfinder";
         const char* routeStr = StringTable::getString( 
            StringTable::ROUTE, language );
         char* subject = new char[ strlen(preStr) + 1 + strlen( routeStr )
                                   + 1 + 
                                   strlen( origin ) + 4 + strlen( dest ) 
                                   + 1 ];
         if ( !originIsCoord && !destIsCoord ) {
            sprintf( subject, "%s %s -> %s", routeStr, origin, dest );
         } else if ( !destIsCoord ) {
            sprintf( subject, "%s -> %s", routeStr, dest );
         } else {
            sprintf( subject, "%s %s", preStr, routeStr );
         }


         ExpandItemID* exp = expandPack->getItemID(); 
         STMFDParams tileParam( LangTypes::english, false );
         CopyrightHandler 
                  copyrights( m_thread->getGroup()->
                              getTileMapFormatDesc( tileParam, m_thread )->
                              getCopyrightHolder() );
         RouteMessageRequest* rmReq = new RouteMessageRequest( 
            m_thread->getNextRequestID(), routeID.getRouteIDNbr(), 
            routeID.getCreationTime(), 
            language, routePack, expandPack, exp, 
            overviewImageWidth, overviewImageHeight,
            routeTurnImageWidth, routeTurnImageHeight,
            origin, ""/*OriginLocation*/,
            dest, ""/*DestinationLocation*/,
            routeVehicle, 
            m_thread->getTopRegionRequest(),
            makeImages, true, false, true, false, false, 
            false/*OnlyOverview*/,
            messageType, 
            UserConstants::ROUTE_TURN_IMAGE_TYPE_MAP_IMAGE, 
            RouteArrowType::TURN_ARROW,
            maxMessageSize, sendMessage,
            receiver, sender, subject,
            signature, imageFormat, 
            &copyrights, userItem,
            contentLocation );
         
         // Send rmReq
         m_thread->putRequest( rmReq );
         
         if ( sendMessage ) {
            // Check status
            if ( rmReq->getAnswer() == NULL ||
                 static_cast< ReplyPacket* >( 
                    rmReq->getAnswer()->getPacket() )->getStatus()
                 != StringTable::OK )
            {
               if ( rmReq->getAnswer() == NULL ||
                    static_cast< ReplyPacket* >( 
                    rmReq->getAnswer()->getPacket() )->getStatus()
                    == StringTable::TIMEOUT_ERROR )
               {
                  status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
               } else {
                  status = NavReplyPacket::NAV_STATUS_NOT_OK;
               }
            }
         } else {
            // Check status
            if ( rmReq->getNbrMimeMessages() < 1 || 
                 rmReq->getMimeMessage( 0 )->getNbrMimeParts() < 1 )
            {
               // Not ok
               if ( rmReq->getNbrOutstandingPackets() > 
                    rmReq->getNbrReceivedPackets() )
               {
                  status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
               } else {
                  status = NavReplyPacket::NAV_STATUS_NOT_OK;
               }
            } else if ( plainText ) { // Ok and plainText
               // Extract plaintext and send it
               const char* messBreak = "\r\n";
               const uint32 messBreakLen = strlen( messBreak );
               uint32 totSize = 0;
               for ( uint32 i = 0 ; i < rmReq->getNbrMimeMessages() ; ++i )
               {
                  totSize += strlen( rmReq->getMimeMessage( i )->
                                     getPlaintextMessage() );
                  totSize += messBreakLen;
               }

               data = new byte[ totSize + 1 ];
               for ( uint32 i = 0 ; i < rmReq->getNbrMimeMessages() ; ++i )
               {
                  uint32 len = strlen( rmReq->getMimeMessage( i )->
                                       getPlaintextMessage() );
                  memcpy( data + dataLen, rmReq->getMimeMessage( i )
                          ->getPlaintextMessage(), len );
                  dataLen += len;
                  memcpy( data + dataLen, messBreak, messBreakLen );
                  dataLen += messBreakLen;
               }
               data[ dataLen ] = '\0'; 

               // Send the plaintext
               SendEmailRequestPacket* p = 
                  new SendEmailRequestPacket();
               const char* optionalHeaderTypes[ 1 ] =
                  { MimeMessage::contentTypeHeader };
               const char* optionalHeaderValues[ 1 ] = 
#ifdef MC2_UTF8
                  { "text/plain; charset=\"utf-8\"" }
#else
                  { "text/plain; charset=\"iso-8859-1\"" }
#endif
               ;

               if ( p->setData( 
                       receiver,
                       sender, subject,
                       (char*)data, 1, optionalHeaderTypes,
                       optionalHeaderValues ) )
               {
                  SinglePacketRequest* plainEmailRequest = 
                     new SinglePacketRequest( 
                        m_thread->getNextRequestID(), new PacketContainer( 
                           p, 0, 0, MODULE_TYPE_SMTP ) );
                  // Send it 
                  // Wait for the answer
                  m_thread->putRequest( plainEmailRequest );
                  PacketContainer* ansCont = 
                     plainEmailRequest->getAnswer();
                  // Check status
                  if ( ansCont == NULL ||
                       static_cast< ReplyPacket* >( 
                          ansCont->getPacket() )->getStatus() != 
                       StringTable::OK )
                  {
                     if ( ansCont == NULL ||
                          static_cast< ReplyPacket* >( 
                             ansCont->getPacket() )->getStatus() == 
                          StringTable::TIMEOUT_ERROR )
                     {
                        status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
                     } else {
                        status = NavReplyPacket::NAV_STATUS_NOT_OK;
                     }
                  }
                  
                  delete plainEmailRequest;
                  delete ansCont;
               } else {
                  delete p;
               }

               delete [] data;
               data = NULL;
               dataLen = 0;
            } else { // else ok and full data
               // Make data
               vector< char* > mimeVector;
               const char* mimeBreak = "\r\n<<<<<<";
               const char* NL = "\r\n";
               const uint32 mimeBreakLen = strlen( mimeBreak );
               uint32 totSize = 0;
               const char* partStr = StringTable::getString( 
                  StringTable::PART, language );
               char tmpStr[ 512 ];
               for ( uint32 i = 0 ; i < rmReq->getNbrMimeMessages() ; ++i )
               {
                  // Add mail header here
                  MC2String mail;
                  // To:
                  mail.append( "To: " );
                  mail.append( receiver );
                  mail.append( NL );
                  // From:
                  mail.append( "From: " );
                  mail.append( sender );
                  mail.append( NL );
                  // Subject
                  // Prepend Part XX if getNbrMimeMessages() != 1
                  MC2String subj( subject );
                  if ( rmReq->getNbrMimeMessages() != 1 ) {
                     sprintf( tmpStr, "%s %d ", partStr, i + 1 );
                     MC2String tmpCapitalStr = 
                        StringUtility::makeFirstCapital( MC2String(tmpStr) );
                     subj.insert( 0, tmpCapitalStr.c_str() );
                  }
                  mail.append( "Subject: " );
                  mail.append( subj );
                  mail.append( NL );
                  // MIME-Version
                  mail.append( MimeMessage::mimeVersionHeader );
                  mail.append( ": " );
                  mail.append( 
                     rmReq->getMimeMessage( i )->getMimeVersion() );
                  mail.append( NL );
                  // Content-Type
                  mail.append( MimeMessage::contentTypeHeader );
                  mail.append( ": " );
                  mail.append( 
                     rmReq->getMimeMessage( i )->getContentType() );
                  mail.append( NL );
                  // End header
                  mail.append( NL );
                  // Add body here
                  char* body = rmReq->getMimeMessage( i )
                     ->getMimeMessageBody( contentLocation );
                  mail.append( body );
                  delete [] body;
                  mimeVector.push_back( StringUtility::newStrDup(
                                           mail.c_str() ) );
                  totSize += mail.size();
                  totSize += mimeBreakLen;
               }

               data = new byte[ totSize + 1 ];
               dataLen = 0;
               for ( uint32 i = 0 ; i < mimeVector.size() ; i++ ) {
                  uint32 len = strlen( mimeVector[ i ] );
                  memcpy( data + dataLen, mimeVector[ i ], len );
                  dataLen += len;
                  memcpy( data + dataLen, mimeBreak, mimeBreakLen );
                  dataLen += mimeBreakLen;
               }
               // This is not really needed but for debug printing
               data[ dataLen ] = '\0'; 
               
               for ( uint32 i = 0 ; i < mimeVector.size() ; i++ ) {
                  delete [] mimeVector[ i ];
               }
            }
         }

         delete exp;
         delete rmReq;
         for ( uint32 i = 0 ; i < nbrStringItems ; i++ ) {
            delete stringItems[ i ];
         }
         delete [] stringItems;
         delete [] subject;
      }
   } // End if status is ok
   
   delete expandRouteCont;
   delete expReq;
   delete routePack;
   delete routeCont;

   return status;
}


uint8 
NavMessHelp::handleItemNavSend( ObjectType objectType,
                                uint32 favoriteID,
                                MC2Coordinate position,
                                const char* positionName,
                                const SearchMatch* searchMatch,
                                MessageType messType,
                                StringTable::languageCode language,
                                bool sendMessage,
                                ImageDrawConfig::imageFormat imageFormat,
                                RouteMessageRequestType::MessageContent 
                                messageType,
                                uint32 maxMessageSize,
                                uint32 overviewImageWidth,
                                uint32 overviewImageHeight,
                                const UserUser* user,
                                const UserItem* userItem,
                                const char* sender,
                                const char* signature,
                                const char* receiver,
                                bool contentLocation,
                                byte*& data, uint32& dataLen )
{
   uint8 status = NavReplyPacket::NAV_STATUS_OK;
   LangTypes::language_t languageType = 
      ItemTypes::getLanguageCodeAsLanguageType( language );
   MC2Coordinate coord = MC2Coordinate::invalidCoordinate;
   char* itemName = NULL;

   if ( objectType == Destination ) {
      // Get favorite
      UserFavoritesRequest* userFavReq = new UserFavoritesRequest( 
         m_thread->getNextRequestID(), user->getUIN() );
      
      // Wait for the answer
      m_thread->putRequest( userFavReq );
      
      if ( userFavReq->getStatus() == StringTable::OK ) {
         const UserFavorite* favorite = userFavReq->getAddFav();
         while ( favorite != NULL ) {
            if ( favorite->getID() == favoriteID ) {
               coord.lat = favorite->getLat();
               coord.lon = favorite->getLon();
               itemName = StringUtility::newStrDup( 
                  favorite->getName() );
               break;
            }
            
            favorite = userFavReq->getAddFav();
         }
         if ( favorite == NULL ) {
            // Not found TODO: Not found errorcode
            status = NavReplyPacket::NAV_STATUS_NOT_OK; 
            mc2log << warn << "handleItemNavSend: Favorite " << favoriteID
                   << " not found." << endl;
         }
      } else {
         mc2log << warn << "handleItemNavSend: SendMessage: userFavReq "
                << "failed "
                << StringTable::getString( 
                   StringTable::stringCode( userFavReq->getStatus() ),
                   StringTable::ENGLISH ) << endl;
         if ( userFavReq->getStatus() == StringTable::UNKNOWN ) {
            status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
         } else {
            status = NavReplyPacket::NAV_STATUS_NOT_OK;
         }
      }
      
      delete userFavReq;
   } else if ( objectType == Position ) {
      coord = position;
      itemName = StringUtility::newStrDup( positionName );
      if ( !position.isValid() ) {
         mc2log << warn << "handleItemNavSend: position coordinate is "
                << "invalid." << endl;
      }
   } else {
      // SearchMatch, get coordinate for it
      const SearchMatch* match = searchMatch;
      if ( match != NULL ) {
         ItemInfoRequest* ir = new ItemInfoRequest( 
            m_thread->getNextRequestID(),
            m_thread->getTopRegionRequest());
         ir->setItem( match->getMapID(), match->getItemID(),
                      languageType, match->getItemType() );
         // Wait for the answer
         m_thread->putRequest( ir );
         
         if ( ir->replyDataOk() ) {
            coord.lat = ir->getReplyLatitude( 0 );
            coord.lon = ir->getReplyLongitude( 0 );
            itemName = StringUtility::newStrDup( 
               ir->getReplyName( 0 ) );
         } else {
            mc2log << warn << "handleItemNavSend: ItemInfoRequest failed." 
                   << endl;
            // TODO: Status of request (ItemInfoRequest doesn't have any)
            status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
         }
         delete ir;
      } else {
         // Perhaps other error code?
         status = NavReplyPacket::NAV_STATUS_NOT_OK; 
      }
   }
   
   if ( coord.isValid() ) {
      // Make bbox around coord
      uint32 posSize = 60000; // TODO: Setting from client?
      MC2BoundingBox bbox( coord.lat + posSize,
                           coord.lon - posSize,
                           coord.lat - posSize,
                           coord.lon + posSize );
      
      bool makeImages = true;
      
      maxMessageSize = 100*1024*1024;
      GfxFeatureMap* mapSymbolMap = new GfxFeatureMap();
      GfxSymbolFeature* feat = new GfxSymbolFeature( 
         GfxFeature::SYMBOL, /*itemName*/"", GfxSymbolFeature::PIN, "" );
      feat->addNewPolygon( true, 1 );
      feat->addCoordinateToLast( coord.lat, coord.lon );
      mapSymbolMap->addFeature( feat );
      MC2String subject( StringTable::getString( 
                            StringTable::YOUR_LOCAL_MAP, language ) );
      MC2String itemNameStr( itemName );
      char posStr[256];
      subject.append( " " ); 
      if ( itemName[ 0 ] == '\0' ) {
         // Use coordiante
         makePositionString( coord, posStr, language );
         subject.append( posStr );
         itemNameStr = posStr;
      } else {
         subject.append( itemName ); 
      }


      if ( messType == SMS ) {
         // Return SMS with text
         MC2String text( subject );
         const char* smsBreak = "\r\n<<<<<<";
         text.append( smsBreak );
         dataLen = text.length();
         data = new byte[ dataLen + 1 ];
         if ( m_thread->clientUsesLatin1() ) {
            // Ok, convert to iso and pry (Decreases size)
            MC2String utext = UTF8Util::utf8ToIso( text );
            text = utext;
         }
         dataLen = text.size();
         memcpy( data, text.c_str(), dataLen );

         // This is not really needed but for debug printing
         data[ dataLen ] = '\0'; 
      } else { // Not SMS use localmaprequest
         MC2String copyright = 
            m_thread->getGroup()->getCopyright( m_thread, bbox, 
                                                LangTypes::english );
         // LocalMapMessageRequest
         LocalMapMessageRequest* lmReq = new LocalMapMessageRequest(
            m_thread->getNextRequestID(),
            bbox.getMaxLat(), bbox.getMinLon(),
            bbox.getMinLat(), bbox.getMaxLon(),
            language, overviewImageWidth, overviewImageHeight,
            itemNameStr.c_str(),
            m_thread->getTopRegionRequest(),
            m_thread->getClientSetting(),
            mapSymbolMap, 
            makeImages, false, false, messageType, maxMessageSize,
            sendMessage, receiver, sender, 
            subject.c_str(), signature, imageFormat,
            copyright, userItem,
            contentLocation );
      
         // Send lmReq
         // Wait for the answer
         m_thread->putRequest( lmReq );
      
         if ( sendMessage ) {
            // Check status
            if ( lmReq->getAnswer() == NULL || static_cast< ReplyPacket* >( 
                    lmReq->getAnswer()->getPacket() )->getStatus() != 
                 StringTable::OK )
            {
               mc2log << warn << "handleItemNavSend: "
                      << "LocalMapMessageRequest failed. " 
                      << (lmReq->getAnswer() == NULL ? 
                          "NULL" : StringTable::getString(
                             StringTable::stringCode( 
                                static_cast< ReplyPacket* >( 
                                   lmReq->getAnswer()->getPacket() )
                                ->getStatus() ), StringTable::ENGLISH ))
                      << endl;
               if ( lmReq->getAnswer() == NULL || static_cast< ReplyPacket* >(
                       lmReq->getAnswer()->getPacket() )->getStatus() != 
                    StringTable::TIMEOUT_ERROR )
               {
                  status = NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
               } else {
                  status = NavReplyPacket::NAV_STATUS_NOT_OK;
               }
            }
         } else {
            // Make data
            MC2String mail;
            const char* NL = "\r\n";
            // To:
            mail.append( "To: " );
            mail.append( receiver );
            mail.append( NL );
            // From:
            mail.append( "From: " );
            mail.append( sender );
            mail.append( NL );
            // Subject
            mail.append( "Subject: " );
            mail.append( subject.c_str() );
            mail.append( NL );
            // MIME-Version
            mail.append( MimeMessage::mimeVersionHeader );
            mail.append( ": " );
            mail.append( lmReq->getMimeMessage()->getMimeVersion() );
            mail.append( NL );
            // Content-Type
            mail.append( MimeMessage::contentTypeHeader );
            mail.append( ": " );
            mail.append( lmReq->getMimeMessage()->getContentType() );
            mail.append( NL );
            // End header
            mail.append( NL );
            // Add body here
            char* mimeText = lmReq->getMimeMessage()->getMimeMessageBody( 
               contentLocation );
            mail.append( mimeText );
            uint32 mimeLen = mail.size();
            data = new byte[ mimeLen + 1 ];
            dataLen = mimeLen;
            memcpy( data, mail.c_str(), mimeLen );

            // This is not really needed but for debug printing
            data[ dataLen ] = '\0'; 
         
            delete [] mimeText;
         } // End else ( part of if sendMessage)
      
         delete lmReq;
         delete mapSymbolMap;
      } // End else not SMS use localmaprequest
   } else {
      mc2log << warn << "handleItemNavSend No coordinate for localmap, "
             << "nothing more to do."
             << " lat " << coord.lat << " lon " << coord.lon 
             << " name " << itemName << endl;
      if ( status == NavReplyPacket::NAV_STATUS_OK ) {
         status = NavReplyPacket::NAV_STATUS_NOT_OK;
      }
   }
   
   delete [] itemName;
   
   return status;
}


void 
NavMessHelp::makePositionString( MC2Coordinate coord, char* str,
                                 StringTable::languageCode language ) const
{
   float64 latRad = 0.0;
   float64 lonRad = 0.0;
   float64 h      = 0.0;
   char latStr[ 256 ];
   char lonStr[ 256 ];
   CoordinateTransformer::transformFromMC2( 
      coord.lat, coord.lon, 
      CoordinateTransformer::sweref93_LLA, latRad, lonRad, h );
   GfxUtility::printLatLon( latStr,
                            latRad,
                            true,
                            language,
                            0,
                            GfxUtility::wgs84Style );
   GfxUtility::printLatLon( lonStr,
                            lonRad,
                            false,
                            language,
                            0,
                            GfxUtility::wgs84Style );
   sprintf( str, "%s %s", latStr, lonStr );

   
}

const char*
NavMessHelp::getMessageTypeAsString( MessageType type ) {
   switch ( type ) {
      case invalid :
         return "invalid";
      case HTML_email :
         return "HTML_email";
      case non_HTML_email :
         return "non_HTML_email";
      case SMS :
         return "SMS";
      case MMS :
         return "MMS";
      case FAX :
         return "FAX";
      case InstantMessage :
         return "InstantMessage";
   };
   return "unknown";
}


const char*
NavMessHelp::getObjectTypeAsString( ObjectType type ) {
   switch ( type ) {
      case Route:
      case Itinerary:
         return "Route";
      case Destination:
         return "Favorite";
      case SearchItem:
      case FullSearchItem:
         return "SearchItem";
      case Position:
         return "Position";
      case Map:
         return "Map";
      case invalidObjectType:
         return "invalidObjectType";
   };
   return "unknown";
}
