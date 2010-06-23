/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "Vector.h"

#include "HttpUtility.h"
#include "Properties.h"
#include "StringUtility.h"
#include "ISABThread.h"
#include "ExpandItemID.h"
#include "ExpandStringItem.h"
#include "RouteRequest.h"
#include "SMSFormatter.h"
#include "GfxFeature.h"
#include "GfxPolygon.h"
#include "GfxFeatureMap.h"
#include "GfxConstants.h"
#include "GfxUtility.h"
#include "ExpandRoutePacket.h"
#include "HttpHeader.h"


// Initialize static members
bool HttpUtility::m_checkAuthInitialized = false;
StringVector HttpUtility::m_authHosts;
StringVector HttpUtility::m_authUsers;
ISABMonitor initializeCheckAuthMonitor;

void
HttpUtility::initializeCheckAuth() {
   ISABSync sync( initializeCheckAuthMonitor );
   if ( !m_checkAuthInitialized ) {
      // Read property and parse
      const char* tmpauthStr = Properties::getProperty( "HTTP_AUTHENTICATE" );
      char* authStr = NULL;
      if ( tmpauthStr != NULL ) {
         // Make editable local string
         authStr = StringUtility::newStrDup( authStr );
         // Read the comma separated list of host;user
         int pos = 0;
         int length = strlen( authStr );
         char* host = NULL;
         char* user = NULL;
         char* ch = strchr( authStr + pos, ',' );
         
         while ( pos < length ) {
            char* semi = strchr( authStr + pos, ';' );
            if ( semi != NULL ) {
               *semi = '\0';
               if ( ch != NULL ) {
                  *ch = '\0';
               }
               host = authStr + pos;
               user = semi + 1;
               m_authHosts.addLast( StringUtility::newStrDup( host ) );
               m_authUsers.addLast( StringUtility::newStrDup( user ) );
            } else {
               mc2log << warn << here << " Bad auth field " 
                      << ( authStr + pos ) << endl;
            }
            if ( ch != NULL ) {
               pos = ch - authStr + 1;
            } else {
               pos = length;
            }
            ch = strchr( authStr + pos + 1, ',' );
         }
         delete [] authStr;
      }
      if ( m_authHosts.getSize() == 0 ) {
         mc2log << error << here << " No authentication loaded." << endl;
      }
      m_checkAuthInitialized = true;
   }
}


bool
HttpUtility::checkAuth( const char* auth ) {
   bool ok = false;

   if ( !m_checkAuthInitialized ) {
      initializeCheckAuth();  
   }
   // Make editable local string
   auth = StringUtility::newStrDup( auth );

   const char* hostDoc = NULL;
   const char* host = NULL;
   const char* user = NULL;
   
   char* firstSemi = NULL;
   char* secondSemi = NULL;
   
   firstSemi = StringUtility::strchr( auth, ';' );
   if ( firstSemi != NULL ) {
      secondSemi =  strchr( firstSemi + 1, ';' ); 
   }

   if ( firstSemi != NULL && secondSemi != NULL ) {
      *firstSemi = '\0';
      *secondSemi = '\0';

      hostDoc = auth;
      host = firstSemi + 1;
      user = secondSemi + 1;

      if ( strcmp( hostDoc, host ) == 0 ) {
         // Find host, user in authenticated list
         for ( uint32 i = 0 ; i < m_authHosts.getSize() ; i++ ) {
            if ( strcmp( m_authHosts[ i ], host ) == 0 &&
                 strcmp( m_authUsers[ i ], user ) == 0 )
            {
               ok = true;
               break;
            }
         }
      }
   }

   delete [] auth;

   return ok;
}


void 
HttpUtility::clear() {
   ISABSync sync( initializeCheckAuthMonitor );   
   m_authHosts.deleteAllObjs();
   m_authUsers.deleteAllObjs();
   m_checkAuthInitialized = false;
}


bool 
HttpUtility::parseUserSettings( const char* str, 
                                StringTable::languageCode& userLang )
{
   bool ok = true;
   char* endPtr = NULL;
   StringTable::languageCode tmpLang = StringTable::SMSISH_ENG;
   tmpLang = StringTable::languageCode( strtol( str, &endPtr, 10 ) );

   if ( endPtr != str && tmpLang < StringTable::SMSISH_ENG && 
        tmpLang >= StringTable::ENGLISH )
   {
      userLang = tmpLang; 
   } else {
      ok = false;
   }

   return ok;
}


char* 
HttpUtility::makeStoredRouteLink( char* target, const char* host,
                                  const char* protocol,
                                  uint32 routeID, 
                                  uint32 routeCreateTime,
                                  StringTable::languageCode lang,
                                  const char* routePage )
{
   sprintf( target, "%s://%s/%s?us=%u&r=%X_%X",
            protocol, host, routePage, lang, routeID, routeCreateTime );
   return target;
}


void 
HttpUtility::makeMapURL( 
   char* target, 
   char* widthStr,
   char* heightStr,
   const HttpHeader* inHead,
   uint32 routeID,
   uint32 routeCreateTime,
   uint32 beforeTurn, uint32 afterTurn,
   uint32 turn,
   uint16 width,
   uint16 height,
   int32 minLat,
   int32 minLon,
   int32 maxLat,
   int32 maxLon,
   ImageDrawConfig::imageFormat defaultFormat,
   MapSettingsTypes::defaultMapSetting mapSetting,
   bool showMap, bool showTopographMap, 
   bool showPOI, bool showRoute,
   bool showScale, bool showTraffic,
   struct MapSettingsTypes::ImageSettings* imageSettings,
   const GfxFeatureMap* symbolMap,
   const char* clientType )
{
   DEBUG4(cerr << "makeMapURL " << endl;);

   ImageDrawConfig::imageFormat format = getImageFormatForRequest(
      inHead, defaultFormat );
   uint32 size = width*height/8 + 4096;
   const uint32 maxURLSize = 4096 - 20; // POST ... HTTP/1.1\r\n + 4 bytes
   char routeIDStr[200];
   char routeTurnStr[30];

   routeIDStr[ 0 ] = '\0';
   routeTurnStr[ 0 ] = '\0';

   // Route handling
   if ( routeID != MAX_UINT32 && routeCreateTime != MAX_UINT32 ) {
      // Make routeIDstr      
      sprintf( routeIDStr, "%X_%X", routeID, routeCreateTime );

      if ( beforeTurn != MAX_UINT32 && afterTurn != MAX_UINT32 ) {
         // Make routeTurnStr
         sprintf( routeTurnStr, "&rt=%X_%X", 
                  beforeTurn, afterTurn );
      } else if ( turn != MAX_UINT32 ) {
         // Turn string
         sprintf( routeTurnStr, "&turn=%X", turn );
      }
   }

   // The propotionall size of the image
   GfxUtility::getDisplaySizeFromBoundingbox( minLat, minLon,
                                              maxLat, maxLon,
                                              width, height );

   // MapSetting
   const char* mapSettingStr = MapSettings::defaultMapSettingToString( 
      mapSetting );

   // ImageSettings
   char imageSettingsStr[20];
   const char* imageSettingsParam = "&is=";
   const int imageSettingsParamLength = strlen( imageSettingsParam );
   imageSettingsStr[ 0 ] = '\0';
   if ( imageSettings != NULL ) {
      strcpy( imageSettingsStr, imageSettingsParam );
      MapSettings::imageSettingsToString( 
         imageSettingsStr + imageSettingsParamLength, *imageSettings );
   }
   
   char* ext = StringUtility::newStrDup(
      StringUtility::copyLower( MC2String( 
         ImageDrawConfig::imageFormatMagick[ format ] ) ).c_str());
   
   uint32 pos = sprintf( target, 
                         "Map.%s?lla=%d&llo=%d&ula=%d&ulo=%d&w=%u&h=%u"
                         "&s=%u&r=%s%s&mt=%s%s"
                         "&map=%d&topomap=%d&poi=%d&route=%d&scale=%d"
                         "&traffic=%d",
                         ext,
                         minLat, minLon,
                         maxLat, maxLon,
                         width, height,
                         size, routeIDStr, routeTurnStr, 
                         mapSettingStr, imageSettingsStr,
                         showMap, showTopographMap, showPOI, showRoute,
                         showScale, showTraffic );
   if ( symbolMap != NULL ) {
      pos += addMapSymbols( target + pos, symbolMap, maxURLSize - pos );
   }

   if ( clientType != NULL ) {
      MC2String clientTypeParam = "&c=";
      clientTypeParam += clientType;
      strcpy( target + pos, clientTypeParam.c_str() );
      pos += clientTypeParam.size();      
   }
               
   if ( widthStr != NULL ) {
      sprintf( widthStr, "%u", width );
   }
   if ( heightStr != NULL ) {
      sprintf( heightStr, "%u", height );
   }

   delete [] ext;   
}


SMSSendRequest* 
HttpUtility::makeRouteLinkSMS( const char* comment,
                               const char* host,
                               const char* protocol,
                               uint32 routeID, 
                               uint32 routeCreateTime,
                               StringTable::languageCode lang,
                               const CellularPhoneModel* cellular,
                               uint16 reqID,
                               const char* service,
                               const char* receiver,
                               const char* routePage )
{
   char link[8196];

   if ( HttpUtility::makeStoredRouteLink( link, host, protocol, routeID, 
                                          routeCreateTime, lang ) != NULL )
   {
      return makeLinkSMS( comment, link, cellular, reqID, 
                          service, receiver );
   } else {
      return NULL;
   }
}


char* 
HttpUtility::makeLocalMapLink( char* target, const char* host,
                               const char* protocol,
                               int32 ula, int32 llo,
                               int32 lla, int32 ulo,
                               GfxFeatureMap* symbolMap,
                               StringTable::languageCode lang,
                               const char* localMapString,
                               const char* signature,
                               const char* localMapPage,
                               const char* clientType )
{
   // see WAPFunctions::htmlWAPShowLocalMap and 
   // HtmlFunctions::htmlShowLocalMap
   uint32 pos = sprintf( target, 
                         "%s://%s/%s?us=%u&ula=%d&llo=%d&lla=%d&ulo=%d",
                         protocol, host, localMapPage, lang,
                         ula, (ulo-llo), (ula-lla), ulo );
   if ( symbolMap != NULL ) {
      GfxFeatureMap* mapSymbolMap = new GfxFeatureMap();
      DataBuffer mapData( symbolMap->getMapSize() );
      symbolMap->save( &mapData );
      mapSymbolMap->load( &mapData );

      // Adjust coordinates
      for ( uint32 i = 0 ; i < mapSymbolMap->getNbrFeatures() ; i++ ) {
         if ( mapSymbolMap->getFeature( i )->getType() == 
              GfxFeature::SYMBOL )
         {
            const GfxSymbolFeature* feat = static_cast< 
               const GfxSymbolFeature* > ( mapSymbolMap->getFeature( i ) );

            feat->getPolygon( 0 )->setStartLat( 
               ula - feat->getPolygon( 0 )->getLat( 0 ) );
            feat->getPolygon( 0 )->setStartLon( 
               ulo - feat->getPolygon( 0 )->getLon( 0 ) );
         }
      }
      pos += addMapSymbols( target + pos, mapSymbolMap );

      delete mapSymbolMap;
   }
   if ( localMapString != NULL ) {
      strcpy( target + pos, "&lm=" );
      pos += 4; // "&lm="
      pos += StringUtility::URLEncode( target + pos, localMapString );
   }
   if ( signature != NULL ) {
      strcpy( target + pos, "&sig=" );
      pos += 5; // "&sig="
      pos += StringUtility::URLEncode( target + pos, signature );
   }
   if ( clientType != NULL ) {
      MC2String clientTypeParam = "&c=";
      clientTypeParam += clientType;
      strcpy( target + pos, clientTypeParam.c_str() );
      pos += clientTypeParam.size();
   }

   return target;
}


SMSSendRequest* 
HttpUtility::makeLocalMapLinkSMS( 
   const char* comment,
   const char* host,
   const char* protocol,
   int32 ula, int32 llo,
   int32 lla, int32 ulo,
   GfxFeatureMap* symbolMap,
   StringTable::languageCode lang,
   CellularPhoneModel* cellular,
   uint16 reqID,
   const char* service,
   const char* receiver,
   const char* routePage )
{
   char link[8196];

   if ( HttpUtility::makeLocalMapLink( link, host, protocol, 
                                       ula, llo, lla, ulo, symbolMap,
                                       lang ) )
   {
      return makeLinkSMS( comment, link, cellular, reqID, 
                          service, receiver );
   } else {
      return NULL;
   }   
}


SMSSendRequest* 
HttpUtility::makeLinkSMS( const char* comment,
                          const char* link,
                          const CellularPhoneModel* cellular,
                          uint16 reqID, 
                          const char* service,
                          const char* receiver )
{
   char message[16384];
   int commentLength = strlen( comment );
   int linkLength = strlen( link );
   int lineBreakSize = 2*2; // Two lines
   if ( commentLength > 0 && 
        (commentLength + linkLength + lineBreakSize) > MAX_SMS_SIZE )
   {
      commentLength = MAX( 
         MIN( (MAX_SMS_SIZE - linkLength - lineBreakSize), 
              commentLength ), 0 );
   }
   strncpy( message, comment, commentLength );
   message[ commentLength ] = '\0';

   SMSSendRequest* req = SMSFormatter::makeTextSMSRequest( 
      cellular,
      reqID,
      receiver, service,
      2, true, message, link );
      
   return req;
}


uint32 
HttpUtility::addMapSymbols( char* target, 
                            const GfxFeatureMap* symbolMap,
                            uint32 maxURLSize )
{
   // Add &ms=...
   uint32 pos = 0;
   char msStr[ 4096 ];
   uint32 length = 0;
   for ( uint32 i = 0 ; i < symbolMap->getNbrFeatures() ; i++ ) {
      if ( symbolMap->getFeature( i )->getType() == GfxFeature::SYMBOL )
      { 
         length = static_cast< const GfxSymbolFeature* > ( 
            symbolMap->getFeature( i ) )->toStringBuf( msStr );
         if ( pos + length >= maxURLSize ) {
            mc2log << warn << "HttpUtility::addMapSymbols symbol string "
                   << "full @ " << pos << " can't add " << length 
                   << " bytes" << " maxURLSize is " << maxURLSize 
                   << " nbr symbols left out is " 
                   << (symbolMap->getNbrFeatures() - i) << " of total "
                   << symbolMap->getNbrFeatures() << endl;
            break;
         }
         strcpy( target + pos, "&ms=" );
         pos += 4; // "&ms="
         strcpy( target + pos, msStr );
         pos += length;
      } else {
         mc2log << warn << "HttpUtility::addMapSymbols symbolMap "
                   "contains non symbol feature! Type " 
                << int( symbolMap->getFeature( i )->getType()) << endl;
      }
   }
   
   return pos;
}


void 
HttpUtility::readMapSymbols( const MC2String* str, 
                             GfxFeatureMap* symbolMap )
{
   uint32 pos = 0;
   const char* msStr = HttpHeader::getFirstStringIn( str, pos );
         
   while ( msStr != NULL ) {
      GfxSymbolFeature::symbols symbol = GfxSymbolFeature::NBR_SYMBOLS;
      char* name = NULL;
      int32 lat = 0;
      int32 lon = 0;
      char* symbolImage = NULL;
      if ( GfxSymbolFeature::readSymbolString( msStr,
                                               symbol,
                                               name, 
                                               lat, lon, 
                                               symbolImage ) )
      {
         GfxSymbolFeature* feat = new GfxSymbolFeature(
            GfxFeature::SYMBOL, name, symbol, symbolImage );
         feat->addNewPolygon( true, 0 );
         feat->addCoordinateToLast( lat, lon );
         
         symbolMap->addFeature( feat );
      } else {
         mc2log << warn << "HttpUtility::readMapSymbols bad ms parameter: "
                << msStr << endl;
      }

      delete [] name;
      delete [] symbolImage;
      msStr = HttpHeader::getNextStringIn( str, pos );
   }
}



void
HttpUtility::getRouteMC2BoundingBox( ExpandRouteReplyPacket* expand,
                                     ExpandItemID* exp,
                                     uint32 startIndex, uint32 stopIndex,
                                     int32& minLat,
                                     int32& maxLat,
                                     int32& minLon,
                                     int32& maxLon )
{
   minLat = MAX_INT32;
   maxLat = MIN_INT32;
   minLon = MAX_INT32;
   maxLon = MIN_INT32;
   bool singleTurn = (startIndex == stopIndex);

   uint32 i = 0;
   Vector& groupID = exp->getGroupID();

   // skipp until startIndex is found in groupID
   while ( i < exp->getNbrItems() && groupID[ i ] < startIndex ) {
      i++;
   }

   if ( singleTurn ) {
      // Start at last in prevoius group
      i = MAX( 0, int32( i ) - 1 );
   }

   // run until stopIndex is found in getGroupID or exp->getNbrItems()
   for ( /* skipped untill startIndex above */ ; 
         i < exp->getNbrItems() && groupID[ i ] <= stopIndex ;
         i++ ) 
   {
      int32 lat = MAX_INT32;
      int32 lon = MAX_INT32;
      for ( uint32 j = 0 ; j < exp->getNbrCoordinates( i ) ; j++ ) {
         lat = exp->getLatValue( i, j );
         lon = exp->getLonValue( i, j );  
         if ( lat != MAX_INT32 ) {
            if ( lat < minLat ) {
               minLat = lat;
            } 
            if ( lat > maxLat ) {
               maxLat = lat;
            }
         }
         if ( lon != MAX_INT32 ) {
            if ( lon < minLon ) {
               minLon = lon;
            }
            if ( lon > maxLon ) {
               maxLon = lon;
            }
         }
      }
      if ( singleTurn && groupID[ i ] == stopIndex &&
           lat != MAX_INT32 &&
           lon != MAX_INT32 )
      {
         // Stop after first in next group
         break;
      }
   }
   
   checkMinimumBoundingbox( minLat, maxLat, minLon, maxLon, 0.10 );
}


void
HttpUtility::getNavCrossingMC2BoundingBox( ExpandRouteReplyPacket* expand,
                                      ExpandItemID* exp,
                                      ExpandStringItem** stringItems,
                                      uint32 index,
                                      int32& minLat,
                                      int32& maxLat,
                                      int32& minLon,
                                      int32& maxLon,
                                      uint16& angle )
{
   
   uint32 i = 0;
   Vector& groupID = exp->getGroupID();

   // skip until index is found in groupID
   while ( i < exp->getNbrItems() && groupID[ i ] < index ) {
      i++;
   }

   uint32 coordIdx = 0;   

   if ( i >= exp->getNbrItems() ) {
      i = exp->getNbrItems() - 1;
      coordIdx = exp->getNbrCoordinates( i ) - 1;
   }
  
   
   int32 centerLat = minLat = maxLat = exp->getLatValue( i, coordIdx );
   int32 centerLon = minLon = maxLon = exp->getLonValue( i, coordIdx );

   // Determine how large the bbox should be depending on the 
   // kind of turn.
   int32 size;
   switch ( stringItems[index]->getCrossingKind() ) {
      case ( ItemTypes::CROSSING_2ROUNDABOUT ) :
      case ( ItemTypes::CROSSING_3ROUNDABOUT ) :
      case ( ItemTypes::CROSSING_4ROUNDABOUT ) :
      case ( ItemTypes::CROSSING_4ROUNDABOUT_ASYMMETRIC ) :
      case ( ItemTypes::CROSSING_5ROUNDABOUT ) :
      case ( ItemTypes::CROSSING_6ROUNDABOUT ) :
      case ( ItemTypes::CROSSING_7ROUNDABOUT ) :
         size = 30000;
      break;
      default :
         size = 10000;
      break;
   }
             
   checkMinimumBoundingbox( minLat, maxLat, minLon, maxLon, 
                            0.0, size, size );

   MC2BoundingBox bbox( maxLat, minLon, minLat, maxLon );
   
   float64 radAngle = 0; // Radians.
   
   // Find the angle that the map should be rotated.
   //
   // Algorithm:
   //
   // Start from the actual turn.
   // Move backwards until either we come outside the 
   // boundingbox or until we passed the previous turn. Take
   // the angle at that location.
   // 
   
   // Set start values.
   if (index == 0) {
      // Beginning of route.
      // Get intersection between first and second coordinate.
      if (exp->getNbrCoordinates( 0 ) > 1) {
         int32 prevLat = exp->getLatValue( 0, 0 );
         int32 prevLon = exp->getLonValue( 0, 0 );
         int32 curLat = exp->getLatValue( 0, 1 );
         int32 curLon = exp->getLonValue( 0, 1 );
         
         // Calculate angle.
         radAngle = GfxUtility::getAngleFromNorth( prevLat, prevLon,
                                                   curLat, curLon );
      }
      
      
   } else if (int(index) == (expand->getNumStringData() - 1)) {
      // End of route.
      // Get intersection between before last and last coordinate.   
      uint32 lastIdx = exp->getNbrItems() - 1;
      uint32 lastNbrCoords = exp->getNbrCoordinates( lastIdx );
      if ( lastNbrCoords > 1) {
         int32 prevLat = exp->getLatValue( lastIdx, lastNbrCoords - 2 );
         int32 prevLon = exp->getLonValue( lastIdx, lastNbrCoords - 2 );
         int32 curLat = exp->getLatValue( lastIdx, lastNbrCoords - 1 );
         int32 curLon = exp->getLonValue( lastIdx, lastNbrCoords - 1 );

         // Calculate angle.
         radAngle = GfxUtility::getAngleFromNorth( prevLat, prevLon,
                                                   curLat, curLon );
      }
      
   } else {
      // Somewhere in the middle of route.
      // Go backwards from the coordinate of the actual turn,
      // until we reach a coordinate that is outside the bbox
      // or if we have passed the previous turn.
      int32 curLat = exp->getLatValue( i, coordIdx );
      int32 curLon = exp->getLonValue( i, coordIdx );
      int32 prevLat = MAX_INT32;
      int32 prevLon = MAX_INT32;
      
      int32 itemIdx = i - 1;
      int32 curCoordIdx = exp->getNbrCoordinates( itemIdx ) - 1;
      if ( exp->getNbrCoordinates( itemIdx ) > 0 ) {
         bool stop = false;

         while ( ( groupID[ itemIdx ] == (index - 1) ) && ( ! stop) ) {
            
            prevLat = exp->getLatValue( itemIdx, curCoordIdx );
            prevLon = exp->getLonValue( itemIdx, curCoordIdx );
            
            if ( bbox.contains( prevLat, prevLon ) ) {
               if ( curCoordIdx > 0 ) {
                  curCoordIdx--;
                  curLat = prevLat;
                  curLon = prevLon;
               } else if ( itemIdx > 0 ) { 
                  itemIdx--;
                  curCoordIdx = exp->getNbrCoordinates( itemIdx ) - 1;
                  if ( groupID[ itemIdx ] == (index - 1) ) {
                     curLat = prevLat;
                     curLon = prevLon;
                  }
               } else {
                  stop = true;
               }
            } else {
               // prevLat/Lon are outside boundingbox, but curLat/Lon
               // isn't. Stop loop.
               stop = true;
            }
         }

         // Calculate the angle between prevLat/Lon and curLat/Lon.
         radAngle = GfxUtility::getAngleFromNorth( prevLat, prevLon,
                                                       curLat, curLon );
      }
   }
   
   // Expand bbox according to the resulting angle.
   int32 newHalfWidth = int32((bbox.getLonDiff() * fabs(cos(radAngle)) + 
         bbox.getHeight() * fabs(cos(M_PI/2.0 - radAngle))) / 2 + 0.5);
   int32 newHalfHeight = int32((bbox.getLonDiff() * fabs(sin(radAngle)) +
         bbox.getHeight()*fabs(sin(M_PI/2.0 - radAngle))) / 2 + 0.5);
   
   // Convert to degrees.
   angle = uint16(radAngle * GfxConstants::radianTodegreeFactor + 0.5);
      
   minLat = centerLat - newHalfHeight;
   maxLat = centerLat + newHalfHeight;
   minLon = centerLon - newHalfWidth;
   maxLon = centerLon + newHalfWidth;
   
}


ImageDrawConfig::imageFormat 
HttpUtility::getImageFormatForRequest( 
   const HttpHeader* inHead,
   ImageDrawConfig::imageFormat defaultFormat )
{
   // Check accept header for "image/png", "image/gif" 
   // and "image/vnd.wap.wbmp"
   MC2String acceptStr = "Accept";
   bool png = false;
   bool gif = false;
   bool wbmp = false;

   const MC2String* accept = NULL;
   if ( inHead != NULL ) {
      accept = inHead->getHeaderValue( &acceptStr );
   }
   if ( accept != NULL ) {
      // Check all accept lines
      uint32 pos = 0;

      const char* matchStr = HttpHeader::getFirstStringIn( accept, 
                                                           pos );
      while ( matchStr != NULL ) {
         if ( strstr( matchStr, "image/png" ) != NULL ) {
            png = true;
         } else if ( strstr( matchStr, "image/gif" ) != NULL ) {
            gif = true;
         } else if ( strstr( matchStr, "image/vnd.wap.wbmp" ) != NULL ) {
            wbmp = true;
         }
         matchStr = HttpHeader::getNextStringIn( accept, pos );
      }
   }

   // Set result
   ImageDrawConfig::imageFormat format = defaultFormat;
   if ( png ) {
      format = ImageDrawConfig::PNG;
   } else if ( gif ) {
      format = ImageDrawConfig::GIF;
   } else if ( wbmp ) {
      format = ImageDrawConfig::WBMP;
   }

   return format;
}


void 
HttpUtility::turnItemIndexFromStringItemIndex( ExpandItemID* exp,
                                               uint32 startIndex,
                                               uint32 stopIndex,
                                               uint32& beforeTurn,
                                               uint32& afterTurn )
{
   beforeTurn = MAX_UINT32;
   afterTurn = MAX_UINT32;

   if ( exp->getGroupID().getSize() > 0 ) {
//        for ( uint32 i = 0 ; i < exp->getNbrItems() ; i++ ) {
//           cerr << "Item " << i << endl;
//           cerr << "   groupID " << exp->getGroupID()[ i ] << endl;
//           cerr << "   itemID  " << exp->getItemID()[ i ] << endl;
//           cerr << "   mapID   " << exp->getMapID()[ i ] << endl;
//           cerr << "   nbrCoords " << exp->getNbrCoordinates( i ) << endl;
//           for ( uint32 j = 0 ; j < exp->getNbrCoordinates( i ) ; j++ ) {
//              cerr << "      lat     " << exp->getLat()[ 
//                 exp->getCoordinateOffset()[i] ] << endl;
//              cerr << "      lon     " << exp->getLon()[ 
//                 exp->getCoordinateOffset()[i] ] << endl;
//           }
//        }

      uint32 maxTurnIndex = 0;
      if ( exp->getNbrItems() > 0 ) {
         maxTurnIndex = exp->getGroupID()[ exp->getNbrItems() -1 ];
      }

      if ( startIndex > maxTurnIndex ) {
         // Destination
         beforeTurn = exp->getNbrItems() - 1;
         afterTurn = exp->getNbrItems() - 1;
      } else {
         if ( stopIndex < startIndex ) {
            stopIndex = startIndex;
         }

         if ( startIndex == stopIndex ) {
            Vector& groupID = exp->getGroupID();

            // skipp until startIndex is found in getGroupID()
            uint32 i = 0;
            while ( i < exp->getNbrItems() && groupID[ i ] < startIndex ) {
               i++;
            }
      
            // Start at last in prevoius group
            beforeTurn = MAX( 0, int32( i ) - 1 );

            afterTurn = MIN( i, exp->getNbrItems() );
         }
      }
   }
}


char* 
HttpUtility::makeRouteLink( char* target, const char* host,
                            const char* protocol,
                            uint32 routeID, 
                            uint32 routeCreateTime,
                            StringTable::languageCode lang,
                            const char* originString,
                            const char* originLocationString,
                            const char* destinationString,
                            const char* destinationLocationString,
                            const char* signature,
                            const char* routePage )
{
   char safeOStr[ 4096 ];
   char safeOlStr[ 4096 ];
   char safeDStr[ 4096 ];
   char safeDlStr[ 4096 ];
   char safeSStr[ 4096 ];
   
   StringUtility::URLEncode( safeOStr, originString );
   StringUtility::URLEncode( safeOlStr, originLocationString );
   StringUtility::URLEncode( safeDStr, destinationString );
   StringUtility::URLEncode( safeDlStr, destinationLocationString );
   StringUtility::URLEncode( safeSStr, signature );

   sprintf( target, "%s://%s/%s?us=%u&r=%X_%X&"
            "o_s=%s&ol_s=%s&d_s=%s&dl_s=%s&sig=%s",
            protocol, host, routePage, lang, routeID, routeCreateTime,
            safeOStr, safeOlStr, safeDStr, safeDlStr, safeSStr );
   return target;
}


void 
HttpUtility::checkMinimumBoundingbox( int32& minLat, int32& maxLat,
                                      int32& minLon, int32& maxLon,
                                      float64 frame,
                                      int32 minWidth,
                                      int32 minHeight )
{
   // Check for minimum lon- lat-size
   int32 width = maxLat - minLat;
   int32 height = maxLon - minLon;
   float64 coslat = GfxUtility::getCoslat( minLat, maxLat);

   // Take coslat into consideration for minHeight.
   minHeight = int32(minHeight / coslat );
   
   if ( height < minHeight ) {
      int32 diff = (minHeight - height)/2;
      maxLon += diff;
      minLon -= diff;
   }
   if ( width < minWidth ) {
      int32 diff = (minWidth - width)/2;
      maxLat += diff;
      minLat -= diff;
   }

   if ( frame != 0.0 ) {
      // Add frame around the bbox
      width = maxLat - minLat;
      height = maxLon - minLon;
      int32 latOffset = int32( frame * width);
      int32 lonOffset = int32( frame * height / coslat );
      
      if ( latOffset > lonOffset ) {
         lonOffset = int32( latOffset / coslat );
      } else {
         latOffset = int32( lonOffset * coslat );
      }
      maxLat += latOffset;
      minLat -= latOffset;
      maxLon += lonOffset;
      minLon -= lonOffset;
   }
}
