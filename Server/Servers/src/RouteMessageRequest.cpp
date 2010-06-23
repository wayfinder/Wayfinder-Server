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

#include <algorithm>

#include "RouteMessageRequest.h"

#include "MC2String.h"
#include <list>
#include <vector>

#include "HttpUtility.h"

#include "Properties.h"
#include <sys/stat.h>
#include "HttpParserThreadUtility.h"

#include "GfxFeatureMapPacket.h"
#include "GfxFeatureMapImagePacket.h"
#include "GfxPolygon.h"
#include "SendEmailPacket.h"
#include "ExpandRoutePacket.h"
#include "ExpandStringItem.h"
#include "ExpandItemID.h"
#include "RoutePacket.h"
#include "StringTableUtility.h"

#include "MimeMessage.h"

#include "GfxFeatureMapImageRequest.h"
#include "RouteStoragePacket.h"

#include "LandmarkLink.h"
#include "LandmarkHead.h"

#include "CopyrightHandler.h"
#include "Utility.h"

//#define VECTOR_MAPS

// Helper functions
typedef vector< pair< const char*, const char* > >  strStrVector;
typedef pair< const char*, const char* > strStrPair;

void repStrs( strStrVector strs, 
              MC2String& str, bool wapStr )
{
   for ( uint32 i = 0 ; i < strs.size() ; i++ ) {
      RouteMessageRequest::replaceString( 
         str, strs[ i ].first, strs[ i ].second, wapStr );
   }
}

struct mimePartEq : public binary_function<const MimePart*, char*, bool> {
   bool operator()( const MimePart* a, char* uri) const {
      return strcmp( a->getContentLocation(), uri ) == 0;
   }
};

const uint32 RouteMessageRequest::EMAILREQUESTPACKET_TIMEOUT = 30000; // 30 sec

RouteMessageRequest::RouteMessageRequest( 
   const RequestData& reqData,
   uint32 routeID, uint32 routeCreateTime,
   StringTable::languageCode lang,
   const RouteReplyPacket* p,
   ExpandRouteReplyPacket* expand,
   ExpandItemID* exp,
   uint16 routeOverviewWidth,
   uint16 routeOverviewHeight,
   uint16 routeTurnWidth,
   uint16 routeTurnHeight,
   const char* originString,
   const char* originLocationString,
   const char* destinationString,
   const char* destinationLocationString,
   ItemTypes::vehicle_t routeVehicle,
   const TopRegionRequest* topReq,
   bool makeImages,
   bool makeLink,
   bool realMapURLs,
   bool includeLandmarks,
   bool useNavTurnBoundingBox,
   bool exportMap,
   bool onlyOverview,
   RouteMessageRequestType::MessageContent contentType,
   UserConstants::RouteTurnImageType turnImageType,
   RouteArrowType::arrowType arrowType,
   uint32 maxMessageSize,
   bool sendEmail,
   const char* toMailAddr,
   const char* fromMailAddr,
   const char* subject,
   const char* signature,
   ImageDrawConfig::imageFormat defaultFormat,
   const CopyrightHandler* copyrights,
   const UserItem* user,
   bool contentLocation,
   MapSettingsTypes::defaultMapSetting mapSetting,
   struct MapSettingsTypes::ImageSettings* imageSettings,
   const char* messageStartFmt,
   const char* messageRestartFmt,
   const char* messageStartTextFmt,
   const char* messageRouteTurnFmt,
   const char* messageRouteTurnTextFmt,
   const char* messageRoutePreTurnLandmarkFmt,
   const char* messageRoutePostTurnLandmarkFmt,
   const char* messageEndFmt,
   const char* messageEndTextFmt,
   uint32 nbrMessageResources,
   char** messageResourcesURI,
   uint32* messageResourcesBuffLength,
   byte** messageResourcesBuff,
   MapSettings* mapSettings ):
   Request( reqData ),
   m_copyrights( copyrights )
{
   m_state = INITIAL;
   m_nbrImageRequests = 0;
   m_currentImageRequest = 0;
   m_imageRequests = NULL;
   m_imageURIs = NULL;
   m_mapSettings = NULL;
   m_answer = NULL;
   m_format = defaultFormat;
   m_contentLocation = contentLocation;
   m_sendEmailRequest = NULL;
   m_sendEmail = sendEmail;
   m_toMailAddr = toMailAddr;
   m_fromMailAddr = fromMailAddr;
   m_subject = subject;
   m_signature = signature;
   m_sendMMS = false;
   m_maxNbrMMSObjects = 10;
   m_nbrMMSObjects = 0;
   m_topReq = topReq;
   m_user = user;

   m_messageStartFmt = const_cast<char*>( messageStartFmt );
   m_messageRestartFmt = const_cast< char* > ( messageRestartFmt );
   m_messageStartTextFmt = const_cast<char*>( messageStartTextFmt );
   m_messageRouteTurnFmt = const_cast<char*>( messageRouteTurnFmt );
   m_messageRouteTurnTextFmt = const_cast<char*>( messageRouteTurnTextFmt);
   m_messageRoutePreTurnLandmarkFmt = const_cast<char*>( 
      messageRoutePreTurnLandmarkFmt );
   m_messageRoutePostTurnLandmarkFmt = const_cast<char*>( 
      messageRoutePostTurnLandmarkFmt );
   m_messageEndFmt = const_cast<char*>( messageEndFmt );
   m_messageEndTextFmt = const_cast<char*>( messageEndTextFmt );
   m_nbrMessageResources = nbrMessageResources;
   m_messageResourcesURI = messageResourcesURI;
   m_messageResourcesBuffLength = messageResourcesBuffLength;
   m_messageResourcesBuff = messageResourcesBuff;
   m_deleteMessageFmt = false;

   m_originString = originString;
   m_originLocationString = originLocationString;
   m_destinationString = destinationString;
   m_destinationLocationString = destinationLocationString;

   m_routeID = routeID;
   m_routeCreateTime = routeCreateTime;
   m_language = lang;
   m_routeReplyPacket = p;
   m_expand = expand;
   m_exp = exp;
   m_deleteExp = false;
   m_stringItems = NULL;
   m_nbrStringItems = 0;
   m_routeOverviewWidth = routeOverviewWidth;
   m_routeOverviewHeight = routeOverviewHeight;
   m_routeTurnWidth = routeTurnWidth;
   m_routeTurnHeight = routeTurnHeight;
   m_routeVehicle = routeVehicle;
   m_makeImages = makeImages;
   m_makeLink = makeLink;
   m_realMapURLs = realMapURLs;
   m_includeLandmarks = includeLandmarks;
   m_useNavTurnBoundingBox = useNavTurnBoundingBox;
   m_exportMap = exportMap;
   m_onlyOverview = onlyOverview;
   m_contentType = contentType;
   m_turnImageType = turnImageType;
   m_arrowType = arrowType;
   m_maxMessageSize = maxMessageSize;
   m_mapSetting = mapSetting;
   m_imageSettings = imageSettings;


   m_mapSettings = mapSettings;

   m_deleteMapSettings = (mapSettings == NULL);

   m_message.push_back( new MimeMessage() );

   if ( m_messageStartFmt != NULL && m_messageRestartFmt != NULL &&
        m_messageRouteTurnFmt != NULL && 
        m_messageRoutePreTurnLandmarkFmt != NULL &&
        m_messageRoutePostTurnLandmarkFmt != NULL &&
        m_messageEndFmt != NULL && m_nbrMessageResources != MAX_UINT32 &&
        m_messageResourcesURI != NULL && 
        m_messageResourcesBuffLength != NULL && 
        m_messageResourcesBuff != NULL )
   {
      // Use user supplied message
      m_deleteMessageFmt = false;
   } else {
      mc2dbg2 << "Some parameters to RouteMessageRequest " 
              << " construtor are NULL. Gets default message" << endl;

      // Get default message
      char* msf = NULL;
      char* mrsf = NULL;
      char* mstf = NULL;
      char* mrtf = NULL;
      char* mrttf = NULL;
      char* mrtlpref = NULL;
      char* mrtlpostf = NULL;
      char* mef = NULL;
      char* metf = NULL;
      char** mruf = NULL;
      uint32* mrbl = NULL;
      byte** mrbf = NULL;

      getDefaultMessage( contentType, 
                         msf, mrsf, mstf, mrtf, mrttf, mrtlpref, mrtlpostf,
                         mef, metf,
                         m_nbrMessageResources,
                         mruf, mrbl, mrbf );
      m_messageStartFmt = msf;
      m_messageRestartFmt = mrsf;
      m_messageStartTextFmt = mstf;
      m_messageRouteTurnFmt = mrtf;
      m_messageRouteTurnTextFmt = mrttf;
      m_messageRoutePreTurnLandmarkFmt = mrtlpref;
      m_messageRoutePostTurnLandmarkFmt = mrtlpostf;
      m_messageEndFmt = mef;
      m_messageEndTextFmt = metf;
      m_messageResourcesURI = mruf;
      m_messageResourcesBuffLength = mrbl;
      m_messageResourcesBuff = mrbf;

      m_deleteMessageFmt = true;
   }

   // Check if message data is ok
   if ( m_messageStartFmt != NULL && m_messageRestartFmt != NULL &&
        m_messageRouteTurnFmt != NULL && 
        m_messageRoutePreTurnLandmarkFmt != NULL &&
        m_messageRoutePostTurnLandmarkFmt != NULL &&
        m_messageEndFmt != NULL && m_nbrMessageResources != MAX_UINT32 &&
        m_messageResourcesURI != NULL && 
        m_messageResourcesBuffLength != NULL && 
        m_messageResourcesBuff != NULL )
   {
      if ( m_signature == NULL ) {
         m_signature = "";
      }
      // All is ok, make image requests
      m_state = makeInitialSetup();
   } else {
      // Error nothing to do about it
      mc2log << warn << "RouteMessageRequest::RouteMessageRequest "
             << "some indata in NULL." << endl;
      setDone( true );
      m_state = ERROR;
   }
}


RouteMessageRequest::~RouteMessageRequest() {

   for ( uint32 i = 0 ; i < m_nbrImageRequests ; i++ ) {
      delete m_imageRequests[ i ]->getAnswer();
      delete m_imageRequests[ i ];
   }

   delete [] m_imageRequests;
   if ( m_deleteMapSettings ) {
      delete m_mapSettings;
   }

   delete m_answer;

   if ( m_deleteMessageFmt ) {
      // Delete message fmt
      delete [] m_messageStartFmt;
      delete [] m_messageRestartFmt;
      delete [] m_messageStartTextFmt;
      delete [] m_messageRouteTurnFmt;
      delete [] m_messageRouteTurnTextFmt;
      delete [] m_messageRoutePreTurnLandmarkFmt;
      delete [] m_messageRoutePostTurnLandmarkFmt;
      delete [] m_messageEndFmt;
      delete [] m_messageEndTextFmt;

      for ( uint32 i = 0 ; i < m_nbrMessageResources ; i++ ) {
         delete [] m_messageResourcesURI[ i ];
         delete [] m_messageResourcesBuff[ i ];
      }

      delete [] m_messageResourcesURI;
      delete [] m_messageResourcesBuff;
      delete [] m_messageResourcesBuffLength;
   }

   for ( uint32 i = 0 ; i < m_nbrStringItems ; i++ ) {
      delete m_stringItems[ i ];
      delete [] m_imageURIs[ i ];
   }

   delete [] m_stringItems;
   if ( m_imageURIs != NULL ) {
      delete [] m_imageURIs[ m_nbrStringItems ];
   }
   delete [] m_imageURIs;

   for ( uint32 i = 0 ; i < m_message.size() ; i++ ) {
      delete m_message[ i ];
   }

   list< PacketContainer* >::iterator it = m_emailRequest.begin();
   while ( it != m_emailRequest.end() ) {
      delete *it;
      it++;
   }

   delete m_sendEmailRequest;

   if ( m_deleteExp ) {
      delete m_exp;

   }

   m_message.clear();
   m_mmsMessage.clear();
   m_nbrSlides.clear();

}


PacketContainer*
RouteMessageRequest::getNextPacket() {
   PacketContainer* res = NULL; 

   switch ( m_state ) {
      case IMAGE_REQUESTS : 
         if ( m_currentImageRequest < m_nbrImageRequests ) {
            res = m_imageRequests[ m_currentImageRequest ]->
               getNextPacket();
         }
         break;
      case EMAIL_REQUEST :
         res = m_sendEmailRequest;
         m_sendEmailRequest = NULL;
         break;
      case UPDATE_ROUTE: 
         res = Request::getNextPacket();
         break;
      case DONE :
         // Done, nothing to send
         break;
      case INITIAL :
      case ERROR :
         mc2log << error << "RouteMessageRequest::getNextPacket called in "
            "wrong state(INITIAL|ERROR) something is very wrong" << endl;
         break;

   }

   return res;
}


void
RouteMessageRequest::processPacket( PacketContainer* ansCont ) {
   switch ( m_state ) {
      case IMAGE_REQUESTS : 
         if ( m_currentImageRequest < m_nbrImageRequests ) {
            m_imageRequests[ m_currentImageRequest ]->processPacket(
               ansCont );
            if ( m_imageRequests[ m_currentImageRequest ]->requestDone() )
            {
               mc2dbg2 << "RouteMessageRequest::processPacket(): "
                          "ImageRequest " << (m_currentImageRequest+1)
                      << " done of " << m_nbrImageRequests << endl;
               m_currentImageRequest++;
               if ( m_currentImageRequest >= m_nbrImageRequests ) {
                  // All image requests processed
                  m_state = makeEmailRequest();
               }
            } // Else continue with this image request
         } else {
            delete ansCont;
            mc2log << error << "RouteMessageRequest::processPacket called"
                      " in state(IMAGE_REQUESTS) but all images received! "
                      "Something is very wrong!" 
                   << endl;
         }
         break;
      case EMAIL_REQUEST :
         if ( !m_emailRequest.empty() && ansCont != NULL && 
              static_cast< ReplyPacket* >( 
                 ansCont->getPacket() )->getStatus() == StringTable::OK )
         {
            m_sendEmailRequest = m_emailRequest.front();
            m_emailRequest.pop_front();
            delete ansCont;
         } else {
            m_answer = ansCont;
            bool done = true;
            if ( m_answer != NULL ) {
               if ( m_routeID != 0 && m_routeCreateTime != 0 ) {
                  // Update validUntil for route
                  m_state = UPDATE_ROUTE;
                  done = false;
                  uint32 validUntil = TimeUtility::getRealTime() + 
                     30*(24*3600); // 30 days ahead
                  RouteStorageChangeRouteRequestPacket* p = 
                     new RouteStorageChangeRouteRequestPacket( 
                        m_routeID, m_routeCreateTime, validUntil );
                  p->setRequestID( getID() );
                  p->setPacketID( getNextPacketID() );
                  enqueuePacket( p, MODULE_TYPE_USER );
               } else {
                  m_state = DONE;
               }
            } else {
               m_state = ERROR;
            }
            if ( done ) {
               setDone( true );
            }
         }
         break;
      case UPDATE_ROUTE: 
         // Ok done with that, disscard result
         delete ansCont;
         // All done!
         m_state = DONE;
         setDone( true );
         break;
      case DONE :
         // Done, nothing to do
         delete ansCont;
         break;
      case INITIAL :
      case ERROR :
         delete ansCont;
         mc2log << error << "RouteMessageRequest::processPacket called in "
            "wrong state(INITIAL|ERROR) something is very wrong" << endl;
         break;

   }   
   
}


PacketContainer*
RouteMessageRequest::getAnswer() {
   return m_answer;
}


void 
RouteMessageRequest::getDefaultMessage( 
   RouteMessageRequestType::MessageContent contentType,
   char*& messageStartFmt,
   char*& messageRestartFmt,
   char*& messageStartTextFmt,
   char*& messageRouteTurnFmt,
   char*& messageRouteTurnTextFmt,
   char*& messageRoutePreTurnLandmarkFmt,
   char*& messageRoutePostTurnLandmarkFmt,
   char*& messageEndFmt,
   char*& messageEndTextFmt,
   uint32& nbrMessageResources,
   char**& messageResourcesURI,
   uint32*& messageResourcesBuffLength,
   byte**& messageResourcesBuff )
{
   byte* buff = NULL;
   int length = 0;

   const char* fileName = m_onlyOverview ?
      m_messageOverviewOnlyStartFmtFile[ contentType ] :
      m_messageStartFmtFile[ contentType ];
      length = loadFile( buff, fileName );
   if ( length >= 0 ) {
      buff[ length ] = '\0';
      messageStartFmt = (char*)buff;
   } else {
      mc2log << warn << "RouteMessageRequest::getDefaultMessage "
             << "problem with file \"" << fileName << "\"" << endl;
   }

   fileName = m_onlyOverview ?
      m_messageOverviewOnlyRestartFmtFile[ contentType ] :
      m_messageRestartFmtFile[ contentType ];
   length = loadFile( buff, fileName );
   if ( length >= 0 ) {
      buff[ length ] = '\0';
      messageRestartFmt = (char*)buff;
   } else {
      mc2log << warn << "RouteMessageRequest::getDefaultMessage "
             << "problem with file \"" << fileName << "\"" << endl;
   }

   fileName = m_onlyOverview ?
      m_messageOverviewOnlyStartTextFmtFile[ contentType ]:
      m_messageStartTextFmtFile[ contentType ];
   length = loadFile( buff, fileName );
   if ( length >= 0 ) {
      buff[ length ] = '\0';
      messageStartTextFmt = (char*)buff;
   } else {
//      mc2log << warn << "RouteMessageRequest::getDefaultMessage "
//             << "problem with file \"" << fileName << "\"" << endl;
   }

   fileName = m_onlyOverview ?
      m_messageOverviewOnlyRouteTurnFmtFile[ contentType ] : 
      m_messageRouteTurnFmtFile[ contentType ];
   length = loadFile( buff, fileName );
   if ( length >= 0 ) {
      buff[ length ] = '\0';
      messageRouteTurnFmt = (char*)buff;
   } else {
      mc2log << warn << "RouteMessageRequest::getDefaultMessage "
             << "problem with file \"" << fileName << "\"" << endl;
   }

   fileName = m_onlyOverview ? 
      m_messageOverviewOnlyRouteTurnTextFmtFile[ contentType ] : 
      m_messageRouteTurnTextFmtFile[ contentType ];
   length = loadFile( buff, fileName );
   if ( length >= 0 ) {
      buff[ length ] = '\0';
      messageRouteTurnTextFmt = (char*)buff;
   } else {
//      mc2log << warn << "RouteMessageRequest::getDefaultMessage "
//             << "problem with file \"" << fileName << "\"" << endl;
   }
   
   fileName = m_onlyOverview ? 
      m_messageOverviewOnlyRoutePreTurnLandmarkFmtFile[ contentType ] :
      m_messageRoutePreTurnLandmarkFmtFile[ contentType ];
   length = loadFile( buff, fileName );
   if ( length >= 0 ) {
      buff[ length ] = '\0';
      messageRoutePreTurnLandmarkFmt = (char*)buff;
   } else {
      mc2log << warn << "RouteMessageRequest::getDefaultMessage "
             << "problem with file \"" << fileName << "\"" << endl;
   }

   fileName = m_onlyOverview ? 
      m_messageOverviewOnlyRoutePostTurnLandmarkFmtFile[ contentType ] :
      m_messageRoutePostTurnLandmarkFmtFile [ contentType ];
   length = loadFile( buff, fileName );
   if ( length >= 0 ) {
      buff[ length ] = '\0';
      messageRoutePostTurnLandmarkFmt = (char*)buff;
   } else {
      mc2log << warn << "RouteMessageRequest::getDefaultMessage "
             << "problem with file \"" << fileName << "\"" << endl;
   }

   fileName = m_onlyOverview ? 
      m_messageOverviewOnlyEndFmtFile[ contentType ] :
      m_messageEndFmtFile[ contentType ];
   length = loadFile( buff, fileName );
   if ( length >= 0 ) {
      buff[ length ] = '\0';
      messageEndFmt = (char*)buff;
   } else {
      mc2log << warn << "RouteMessageRequest::getDefaultMessage "
             << "problem with file \"" << fileName << "\"" << endl;
   }

   fileName = m_onlyOverview ? 
      m_messageOverviewOnlyEndTextFmtFile[ contentType ] :
      m_messageEndTextFmtFile[ contentType ];
   length = loadFile( buff, fileName );
   if ( length >= 0 ) {
      buff[ length ] = '\0';
      messageEndTextFmt = (char*)buff;
   } else {
//      mc2log << warn << "RouteMessageRequest::getDefaultMessage "
//             << "problem with file \"" << fileName << "\"" << endl;
   }

   fileName = m_messageResourcesFile[ contentType ];
   length = loadFile( buff, fileName );
   if ( length >= 0 ) {
      buff[ length ] = '\0';
      length++; // Add null byte
      const char* pos = (char*)buff;
      char line[ length ];
      uint32 nbrLines = 0;

      // Count lines
      while ( pos != NULL ) {
         pos = Utility::getString( pos, '\0', '\n', line, (length - 1) );
         if ( line[ 0 ] != '#' &&
              StringUtility::trimStart( line )[ 0 ] != '\0' ) 
         {
            nbrLines++;
         }
         line[ 0 ] = '\0';
      }

      nbrMessageResources = nbrLines;
      messageResourcesURI = new char*[ nbrMessageResources ];
      messageResourcesBuffLength = new uint32[ nbrMessageResources ];
      messageResourcesBuff = new byte*[ nbrMessageResources ];
      bool ok = true;
      uint32 lineNbr = 0;
      pos = (char*)buff;
      line[ 0 ] = '\0';

      // Get resources
      while ( pos != NULL && ok ) {
         pos = Utility::getString( pos, '\0', '\n', line, length - 1 );
         if ( line[ 0 ] != '#' &&
              StringUtility::trimStart( line )[ 0 ] != '\0' ) 
         {
            // Remove eol
            uint32 linePos = strlen( line );
            while ( linePos > 0 && 
                    (line[ linePos ] == '\r' || line[ linePos ] == '\n') ) 
            {
               line[ linePos ] = '\0';
               linePos--;
            }

            // Parse line
            // [URI] [sp] [file]
            char uri[ length ];
            char file[ length ];
            if ( sscanf( line, "%s%s", uri, file ) == 2 ) {
               messageResourcesURI[ lineNbr ] = StringUtility::newStrDup( 
                  uri );
               
               byte* buffer = NULL;
               uint32 fileLength = loadFile( buffer, file, true );
               if ( fileLength >= 0 ) {
                  messageResourcesBuff[ lineNbr ] = buffer;
                  messageResourcesBuffLength[ lineNbr ]  = fileLength;
               } else {
                  mc2log << info 
                         << "RouteMessageRequest::getDefaultMessage " 
                         << "resource " << file << " failed to load."
                         << endl;
                  ok = false;
               }
            } else {
               mc2log << error
                      << "RouteMessageRequest::getDefaultMessage " 
                      << "resource line " << line << " parse error."
                      << endl;
               ok = false;
            }
            
            if ( !ok ) {
               for ( uint32 i = 0 ; i < lineNbr ; i++ ) {
                  delete messageResourcesURI[ i ];
                  delete messageResourcesBuff[ i ];
               }
               delete [] messageResourcesURI;
               messageResourcesURI = NULL;
               delete [] messageResourcesBuffLength;
               messageResourcesBuffLength = NULL;
               delete [] messageResourcesBuff;
               messageResourcesBuff = NULL;
               nbrMessageResources = 0;
            }

            lineNbr++;
         }
         line[ 0 ] = '\0';
      }
   } else {
      mc2log << warn << "RouteMessageRequest::getDefaultMessage "
             << "problem with file \"" << fileName << "\"" << endl;
      nbrMessageResources = 0;
      messageResourcesURI = NULL;
      messageResourcesBuffLength = NULL;
      messageResourcesBuff = NULL;
   }
   delete [] buff;
}

RouteMessageRequest::state
RouteMessageRequest::makeInitialSetup()
{
   state newState = IMAGE_REQUESTS;
   if ( m_makeImages ) {
      newState = IMAGE_REQUESTS;
   } else if ( m_sendEmail ) {
      newState = EMAIL_REQUEST;
   } else {
      newState = DONE;
      setDone( true );
   }
   if ( m_exp == NULL ) {
      m_exp = m_expand->getItemID();
      m_deleteExp = true;
   }
   m_stringItems = m_expand->getStringDataItem();
   m_nbrStringItems = m_expand->getNumStringData();

   uint32 mapImageNbr = 0;
   char mapImageLocalFmt[ 512 ];
   char routeOverviewURI[4096];
   char routeTurnURI[4096];
   memset( routeTurnURI, 0, 4096 );

   if ( m_contentType == RouteMessageRequestType::HTML_CONTENT ) {
   } else if ( m_contentType == RouteMessageRequestType::WML_CONTENT ) {
      m_format = ImageDrawConfig::WBMP;
      m_mapSetting = MapSettingsTypes::MAP_SETTING_WAP;
   } else if ( m_contentType == RouteMessageRequestType::SMIL_CONTENT ) {
      // Nokia Ericsson conformance says max 160x120
      m_routeOverviewWidth = MIN( 160, m_routeOverviewWidth );
      m_routeOverviewHeight = MIN( 120, m_routeOverviewHeight );
      m_routeTurnWidth = MIN( 160, m_routeTurnWidth );
      m_routeTurnHeight = MIN( 120, m_routeTurnHeight );
      if ( m_maxMessageSize > 30720 ) {
         m_maxNbrMMSObjects = 14;
      } else {
         m_maxNbrMMSObjects = 10;
      }
      m_nbrMMSObjects = 0;
   }

   // MAP_SETTING_WAP and WBMP work together STD and WBMP is too blank
   if ( m_format == ImageDrawConfig::WBMP ) {
      m_mapSetting = MapSettingsTypes::MAP_SETTING_WAP;
   }
   
   if ( m_mapSettings == NULL ) {
      // No MapSettings was submitted. 
      // Create one, based on m_mapSetting and m_imageSettings.
      
      m_mapSettings = MapSettings::createDefaultMapSettings( m_mapSetting );
      bool showPoi = true;
      if ( m_contentType == RouteMessageRequestType::SMIL_CONTENT ) {
         // No poi on images
         showPoi = false;
      }
      // Set what to show on maps
      m_mapSettings->setMapContent( true,   // Map
                                    true,   // Topography
                                    showPoi,   // POI
                                    true ); // Route
      if ( m_imageSettings != NULL ) {
         m_mapSettings->mergeImageSettings( *m_imageSettings );
      }
      if ( ! (m_contentType == RouteMessageRequestType::WML_CONTENT) ) {
         // Draw scale on images
         m_mapSettings->setDrawScale( true );
      }
   }

   // Image type
   char* ext = StringUtility::newStrDup(
      StringUtility::copyLower( MC2String( 
         ImageDrawConfig::imageFormatMagick[ m_format ] ) ).c_str());
   
   sprintf( mapImageLocalFmt, "map%%d.%s", ext );


   // Route overview 
   int32 rOminLat = 0;
   int32 rOminLon = 0;
   int32 rOmaxLat = 0;
   int32 rOmaxLon = 0;
   HttpUtility::getRouteMC2BoundingBox( m_expand, m_exp, 0, 
                                        m_nbrStringItems,
                                        rOminLat, rOmaxLat,
                                        rOminLon, rOmaxLon );
   GfxUtility::getDisplaySizeFromBoundingbox( rOminLat, rOminLon,
                                              rOmaxLat, rOmaxLon,
                                              m_routeOverviewWidth, 
                                              m_routeOverviewHeight );
   if ( m_realMapURLs ) {
      HttpUtility::makeMapURL( routeOverviewURI, NULL, NULL, NULL,
                               m_routeID, m_routeCreateTime, MAX_UINT32,
                               MAX_UINT32, MAX_UINT32, 
                               m_routeOverviewWidth, m_routeOverviewHeight,
                               rOminLat, rOminLon,
                               rOmaxLat, rOmaxLon, m_format, 
                               m_mapSetting, 
                               m_mapSettings->getShowMap(),
                               m_mapSettings->getShowTopographMap(),
                               m_mapSettings->getShowPOI(),
                               m_mapSettings->getShowRoute(), 
                               m_mapSettings->getDrawScale(),
                               m_mapSettings->getShowTraffic(),
                               m_imageSettings );
   } else {
      sprintf( routeOverviewURI, mapImageLocalFmt, mapImageNbr++ );
   }


   if ( m_makeImages ) {
      m_imageRequests = 
         new GfxFeatureMapImageRequest*[ m_nbrStringItems + 1 ];
   } else {
      m_imageRequests = NULL;
   }
   m_nbrImageRequests = 0;
   m_imageURIs = new char*[ m_nbrStringItems + 1  ];

   bool exportMap = false;
   bool image = true;
   uint32 maxScale = INVALID_SCALE_LEVEL;
   uint32 minScale = CONTINENT_LEVEL;
   if ( m_exportMap ) {
      exportMap = true;
      image = false;
      maxScale = DETAILED_STREET_LEVEL;
      minScale = CONTINENT_LEVEL;
   }

  
   for ( int32 i = 0 ; i < m_nbrStringItems ; i++ ) {
      // Route turn 
      if ( m_turnImageType == 
           UserConstants::ROUTE_TURN_IMAGE_TYPE_MAP_IMAGE ) // Not pictogr.
      {
         int32 minLat = 0;
         int32 minLon = 0;
         int32 maxLat = 0;
         int32 maxLon = 0;
         uint32 beforeTurn = MAX_UINT32;
         uint32 afterTurn = MAX_UINT32;
         uint16 mapRotationAngle = 0;
         if ( !m_useNavTurnBoundingBox ) {
            HttpUtility::getRouteMC2BoundingBox( m_expand, m_exp, i, i,
                                                 minLat, maxLat,
                                                 minLon, maxLon );
         } else {
            HttpUtility::getNavCrossingMC2BoundingBox( m_expand, m_exp, 
                                                  m_stringItems, i, 
                                                  minLat, maxLat,
                                                  minLon, maxLon,
                                                  mapRotationAngle );
            m_mapRotationAngles.push_back( mapRotationAngle );
         }

         GfxUtility::getDisplaySizeFromBoundingbox( minLat, minLon,
                                                    maxLat, maxLon,
                                                    m_routeTurnWidth, 
                                                    m_routeTurnHeight );
         HttpUtility::turnItemIndexFromStringItemIndex( m_exp, i, i,
                                                        beforeTurn,
                                                        afterTurn );
         if ( m_realMapURLs ) {
            HttpUtility::makeMapURL( routeTurnURI, NULL, NULL, NULL,
                                     m_routeID, m_routeCreateTime, 
                                     beforeTurn, afterTurn, MAX_UINT32,
                                     m_routeTurnWidth, m_routeTurnHeight,
                                     minLat, minLon, maxLat, maxLon, 
                                     m_format, m_mapSetting, 
                                     m_mapSettings->getShowMap(),
                                     m_mapSettings->getShowTopographMap(),
                                     m_mapSettings->getShowPOI(),
                                     m_mapSettings->getShowRoute(),
                                     m_mapSettings->getDrawScale(),
                                     m_mapSettings->getShowTraffic(),
                                     m_imageSettings );
         } else {
            sprintf( routeTurnURI, mapImageLocalFmt, mapImageNbr++ );
         }

         if ( m_makeImages && !m_onlyOverview ) {
            // Make image request and store uri for it
            MC2BoundingBox bbox( maxLat, minLon, minLat, maxLon );

            MC2BoundingBox ccBBox;
            
            m_imageRequests[ m_nbrImageRequests ] = 
               new GfxFeatureMapImageRequest( 
                  this, &bbox, m_routeTurnWidth, m_routeTurnHeight, 
                  ItemTypes::getLanguageCodeAsLanguageType( m_language ), 
                  m_routeReplyPacket, image, exportMap, 
                  true/*drawCopyRight*/, m_format, 
                  (m_routeTurnWidth*m_routeTurnHeight / 8), // size
                  new MapSettings(*m_mapSettings), m_topReq, ccBBox,
                  maxScale,
                  minScale );
            m_imageRequests[ m_nbrImageRequests ]->ownMapSettings();

            mc2dbg8 << here<< "Made GfxFeatrueMapImageRequest" << endl;
            m_imageRequests[ m_nbrImageRequests ]
               ->setParentRequest( this );

            m_imageRequests[ m_nbrImageRequests ]
               ->setRouteTurn( beforeTurn, afterTurn, m_arrowType );
            if ( m_copyrights ) {
               m_imageRequests[ m_nbrImageRequests ]->
                  setCopyright( m_copyrights->getCopyrightString( bbox ) );
            }
            m_imageRequests[ m_nbrImageRequests ]->setUser( m_user );
            m_nbrImageRequests++;
         }
      } else if ( m_turnImageType == 
                  UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM||
                  m_turnImageType == 
                  UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_1 ||
                  m_turnImageType == 
                  UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_2 ||
                  m_turnImageType == 
                  UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_3 ||
                  m_turnImageType == 
                  UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_4 ||
                  m_turnImageType == 
                  UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_5 )
      {
         const char* turnImageName = getPictogramForTurn( 
            m_stringItems[ i ]->getStringCode() );
         sprintf( routeTurnURI, "%s%s", turnImageName, ext );
      }
      m_imageURIs[ i ] = StringUtility::newStrDup( routeTurnURI );

      
   } // for all stringItems

   m_imageURIs[ m_nbrStringItems ] = 
      StringUtility::newStrDup( routeOverviewURI );
   if ( m_makeImages ) {
      // Add RouteOverview image request
      
      if ( !m_useNavTurnBoundingBox ){
         // Draw scale on image. If enough space 
         m_mapSettings->setDrawScale( true );
      }
      else{
         // Do nothing. Let the mapSettings supplied to the method 
         // determine whether to draw a scale bar or not.
      }
      MC2BoundingBox bbox( rOmaxLat, rOminLon, rOminLat, rOmaxLon );
      MC2BoundingBox ccBBox;
      m_imageRequests[ m_nbrImageRequests ] = 
         new GfxFeatureMapImageRequest( 
            this, &bbox, m_routeOverviewWidth, m_routeOverviewHeight, 
            ItemTypes::getLanguageCodeAsLanguageType( m_language ), 
            m_routeReplyPacket, image, exportMap, 
            true,/*drawCopyRight*/ m_format, 
            (m_routeOverviewWidth*m_routeOverviewHeight / 8), // size
            new MapSettings( *m_mapSettings ), m_topReq, ccBBox,
            maxScale,
            minScale );
      m_imageRequests[ m_nbrImageRequests ]->setParentRequest( this );
      m_imageRequests[ m_nbrImageRequests ]->ownMapSettings();
      m_nbrImageRequests++;
   }

   delete [] ext;

   // If no image requests then make message
   if ( newState == DONE ) {
      newState = makeEmailRequest();
   }
   return newState;
}


RouteMessageRequest::state 
RouteMessageRequest::makeEmailRequest() {
   //**** First make the message(s)

   StringTableUtility::distanceFormat distFormat = StringTableUtility::NORMAL;
   StringTableUtility::distanceUnit distUnit = StringTableUtility::METERS;
   DescProps descProps = ExpandStringItem::createRouteDescProps(
      m_language, false, 9, true, distFormat, distUnit, false,
      m_expand->getStartDirectionOddEven(), 
      m_expand->getStartDirectionHousenumber() );

   
   //*** Make message 
   MC2String startFmtStr( m_messageStartFmt );
   MC2String startTextStr( m_messageStartTextFmt ? 
                        m_messageStartTextFmt : "" );
   const char* startTextFileName = "start";
   const MC2String routeTurnStr( m_messageRouteTurnFmt );
   MC2String turnTextStr;
   const char* routeTurnTextFileName = "turn";
   const MC2String routeTurnLandmarkPreFmt( 
      m_messageRoutePreTurnLandmarkFmt );
   MC2String routeTurnLandmarkPreStr = routeTurnLandmarkPreFmt;
   const MC2String routeTurnLandmarkPostFmt( 
      m_messageRoutePostTurnLandmarkFmt );
   MC2String routeTurnLandmarkPostStr = routeTurnLandmarkPostFmt;
   bool addNewLineToLandmarkFormat = false;
   MC2String endFmtStr( m_messageEndFmt );
   MC2String endTextStr( m_messageEndTextFmt ? m_messageEndTextFmt : "" );
   const char* endTextFileName = "end";
   MC2String message;
   MC2String plainTextMessage;
   const MC2String eolStr( "\r\n" );
   const MC2String shortEolStr( "\n" );
   MC2String messageEOLStr( "<br>\r\n" );
   const char* pageName = "route.html";
   m_textFileType = MimePartText::CONTENT_TYPE_TEXT_HTML;
   const char* textFileNameExtension = ".html";
   MC2String textFileName;
   MimePart::characterSet internalCharacterSet = 
#ifdef MC2_UTF8
      MimePart::CHARSET_UTF_8;
#else
      MimePart::CHARSET_ISO_8859_1;
#endif
   m_defaultCharacterSet = internalCharacterSet;
   MimePart::mainContentType messageMainContentType = 
      MimePart::MAIN_CONTENT_TYPE_TEXT;
   MimePartText::contentType messageTextMarkupType = 
      MimePartText::CONTENT_TYPE_TEXT_HTML;
   MimePartApplication::contentType messageApplicationMarkupType =
      MimePartApplication::CONTENT_TYPE_APPLICATION_OCTETSTREAM;
   uint32 nbrReplaces = 0;
   vector<MimePartText*> textPartVector;
   uint32 messageSize = 0;
   uint32 headerSize = 0;
   vector< MC2String > usedPictogramImages;
   vector< MimePartImage* > usedPictogramImageParts;
   vector< MimePartImage* > usedMapImageParts;
   vector< MimePart* > resourcesParts;
   uint32 resourcesSize = 0;
   uint32 lineLength = 78;
   // Restart 
   MC2String restartFmtStr( m_messageRestartFmt );
   MC2String restartTextStr( m_messageStartTextFmt ? 
                          m_messageStartTextFmt : "" );
   const char* restartTextFileName = "start";
   vector< MimePart* > restartResourcesParts;
   vector< MimePartText* > restartTextParts;
   uint32 restartSize = 0;
   uint32 restartNbrMMSObjects = 0;
   // Continue end
   MC2String contEndFmtStr( m_messageEndFmt );
   MC2String contEndTextStr( m_messageEndTextFmt ? m_messageEndTextFmt : "" );
   const char* contEndTextMessageStr = StringTable::getString( 
      StringTable::ROUTE_CONTINUES, m_language );
   vector< MimePart* > contEndResourcesParts;
   uint32 contEndSize = 0;
   uint32 contEndNbrMMSObjects = 0;

   if ( m_contentType == RouteMessageRequestType::HTML_CONTENT ) {
      pageName = "route.html";
      messageEOLStr = "<br>\r\n";
      textFileNameExtension = ".html";
      m_textFileType = MimePartText::CONTENT_TYPE_TEXT_HTML;
      messageTextMarkupType = MimePartText::CONTENT_TYPE_TEXT_HTML;
      m_message.back()->setMainContentType( 
         MimePartText::getContentTypeAsString(
         MimePartText::CONTENT_TYPE_TEXT_HTML ) );
      addNewLineToLandmarkFormat = false;
   } else if ( m_contentType == RouteMessageRequestType::WML_CONTENT ) {
      pageName = "route.wml";
      messageEOLStr = "<br/>\r\n";
      textFileNameExtension = ".wml";
      m_textFileType = MimePartText::CONTENT_TYPE_TEXT_WML;
      messageTextMarkupType = MimePartText::CONTENT_TYPE_TEXT_WML;
      m_message.back()->setMainContentType( 
         MimePartText::getContentTypeAsString(
            MimePartText::CONTENT_TYPE_TEXT_WML ) );
      addNewLineToLandmarkFormat = true;
   } else if ( m_contentType == RouteMessageRequestType::SMIL_CONTENT ) {
      pageName = "route.smil";
      messageEOLStr = "\r\n";
      textFileNameExtension = ".txt";
      m_textFileType = MimePartText::CONTENT_TYPE_TEXT_PLAIN;
      messageMainContentType = MimePart::MAIN_CONTENT_TYPE_APPLICATION;
      messageApplicationMarkupType = 
         MimePartApplication::CONTENT_TYPE_APPLICATION_SMIL;
      m_message.back()->setMainContentType( 
         MimePartApplication::getContentTypeAsString(
            MimePartApplication::CONTENT_TYPE_APPLICATION_SMIL ) );
      m_defaultCharacterSet = MimePart::CHARSET_UTF_8;
      addNewLineToLandmarkFormat = true;
   }
   if ( m_turnImageType == UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM||
        m_turnImageType == 
        UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_1 ||
        m_turnImageType == 
        UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_2 ||
        m_turnImageType == 
        UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_3 ||
        m_turnImageType == 
        UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_4 ||
        m_turnImageType == 
        UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_5 ) 
   {
      m_routeTurnWidth = 35;
      m_routeTurnHeight = 35;
   }


   // Headers size + safety buffer
   headerSize = strlen( m_toMailAddr ) + 6 + 
      strlen( m_fromMailAddr ) + 8 + strlen( m_subject ) + 11 +
      strlen( m_message.back()->getBoundaryText() ) + 2000;

   messageSize = headerSize;



   plainTextMessage.reserve( 10000 );

   //** Make replace strings
   const char* directions = StringTable::getString( 
      StringTable::DIRECTIONS, m_language );
   const char* from = StringTable::getString( 
      StringTable::ORIGIN, m_language );
   const char* to = StringTable::getString( 
      StringTable::DESTINATION, m_language );
   char routeOverviewWidthValue[20];
   char routeOverviewHeightValue[20];
   char routeOverviewAlt[512];
   const char* routeInfo = StringTable::getString( 
      StringTable::ROUTE_INFO, m_language );
   const char* vehicle = StringTable::getString( 
      StringTable::VEHICLE, m_language );
   char routeVehicleValue[512];
   const char* totalTime = StringTable::getString( 
      StringTable::TOTAL_TIME, m_language );
   char routeTotalTime[512];
   const char* totalDistance = StringTable::getString( 
      StringTable::TOTAL_DISTANCE, m_language );
   char routeTotalDistance[512];
   char tmp[ 4096 ];
   const char* routeOverviewURI = NULL;

   routeOverviewURI = m_imageURIs[ m_nbrStringItems ];

   sprintf( routeOverviewWidthValue, "%d", m_routeOverviewWidth );
   sprintf( routeOverviewHeightValue, "%d", m_routeOverviewHeight );


   strcpy( routeOverviewAlt, StringTable::getString( 
      StringTable::OVERVIEW_OF_ROUTE, m_language ) );

   // RouteVehicle
   strcpy( routeVehicleValue, StringTable::getString(
      ItemTypes::getVehicleSC( m_routeVehicle ), m_language ) );
   
   // Total time
   StringUtility::splitSeconds( m_expand->getTotalTime(), routeTotalTime);

   // Total distance
   StringTableUtility::printDistance(routeTotalDistance, 
                                m_expand->getTotalDist(), 
                                m_language, distFormat, distUnit);




   //** Replace in startStr
   MC2String* startStr = &startFmtStr;

   uint32 nbrRouteOverview = replaceString( 
      *startStr, routeOverviewURIStr,
      routeOverviewURI, 
      m_contentType == RouteMessageRequestType::WML_CONTENT );
   replaceString( *startStr, routeOverviewWidthStr, 
                  routeOverviewWidthValue, 
                  m_contentType == RouteMessageRequestType::WML_CONTENT );
   replaceString( *startStr, routeOverviewHeightStr, 
                  routeOverviewHeightValue, 
                  m_contentType == RouteMessageRequestType::WML_CONTENT );
   replaceString( *startStr, routeOverviewAltStr, routeOverviewAlt, 
                  m_contentType == RouteMessageRequestType::WML_CONTENT );

   textFileName = startTextFileName;
   textFileName.append( textFileNameExtension );
   nbrReplaces = replaceString( 
      *startStr, startFileStr, textFileName.c_str(), 
      m_contentType == RouteMessageRequestType::WML_CONTENT );
   bool useStartTextFile = (nbrReplaces > 0 && 
                            m_messageStartTextFmt != NULL);
   if ( useStartTextFile ) {
      // A special text file for the text
      startStr = &startTextStr;
   }

   strStrVector startReplaceStrings;

   startReplaceStrings.push_back( strStrPair( directionsStr, 
                                              directions ) );
   startReplaceStrings.push_back( strStrPair( fromStr, from ) );
   startReplaceStrings.push_back( strStrPair( toStr, to ) );
   startReplaceStrings.push_back( strStrPair( originStr, 
                                              m_originString ) );
   startReplaceStrings.push_back( strStrPair( originLocationStr, 
                                  m_originLocationString ) );
   startReplaceStrings.push_back( strStrPair( destinationStr, 
                                              m_destinationString ) );
   startReplaceStrings.push_back( strStrPair( destinationLocationStr, 
                                  m_destinationLocationString ) );
   startReplaceStrings.push_back( strStrPair( routeInfoStr, routeInfo ) );
   startReplaceStrings.push_back( strStrPair( vehicleStr, vehicle ) );
   startReplaceStrings.push_back( strStrPair( routeVehicleStr, 
                                              routeVehicleValue ) );
   startReplaceStrings.push_back( strStrPair( totalDistanceStr, 
                                              totalDistance ) );
   startReplaceStrings.push_back( strStrPair( routeTotalDistanceStr, 
                                  routeTotalDistance ) );
   startReplaceStrings.push_back( strStrPair( totalTimeStr, totalTime ) );
   startReplaceStrings.push_back( strStrPair( routeTotalTimeStr, 
                                              routeTotalTime ) );
   MimePartText fake( NULL, 0, messageTextMarkupType,
                      m_defaultCharacterSet, "", false );
   MC2String ctype( fake.getContentType() );
   RouteMessageRequest::replaceString( 
      ctype, "\"", "", 
      m_contentType == RouteMessageRequestType::WML_CONTENT );
   startReplaceStrings.push_back( strStrPair( "%CONTENTTYPE%", 
                                              ctype.c_str() ) );

   repStrs( startReplaceStrings, startTextStr, 
            m_contentType == RouteMessageRequestType::WML_CONTENT );
   repStrs( startReplaceStrings, startFmtStr, 
            m_contentType == RouteMessageRequestType::WML_CONTENT );

   if ( m_defaultCharacterSet != internalCharacterSet ) {
      changeCharacterSet( startTextStr, m_defaultCharacterSet );
      changeCharacterSet( startFmtStr, m_defaultCharacterSet );
   }

   //** Add startFmtStr to message
   message.append( startFmtStr );

   if ( m_makeImages && nbrRouteOverview > 0 ) {
      usedMapImageParts.push_back( getMapImageRequestImage( 
         m_nbrImageRequests - 1, routeOverviewURI ) );
      m_nbrMMSObjects++;
      messageSize += getMimePartSize( usedMapImageParts.back() );
   }

   if ( useStartTextFile ) {
      textPartVector.push_back( new MimePartText( 
         (byte*)startTextStr.c_str(), startTextStr.size(), 
         m_textFileType, m_defaultCharacterSet,
         textFileName.c_str(), true ) );
      m_nbrMMSObjects++;
      messageSize += getMimePartSize( textPartVector.back() );
   }

   // The resources so far
   getUsedResources( message, resourcesParts );
   resourcesSize = 0;
   for ( uint32 i = 0 ; i < resourcesParts.size() ; i++ ) {
      resourcesSize += getMimePartSize( resourcesParts[ i ] );
      m_nbrMMSObjects++;
   }

   //*** Make re-start str
   MC2String* restartStr = &restartFmtStr;
   // Might have overview in restart fmt
   uint32 restartNbrRouteOverview = 
      replaceString( 
         *restartStr, routeOverviewURIStr, routeOverviewURI, 
         m_contentType == RouteMessageRequestType::WML_CONTENT );
   replaceString( *restartStr, routeOverviewWidthStr, 
                  routeOverviewWidthValue, 
                  m_contentType == RouteMessageRequestType::WML_CONTENT );
   replaceString( *restartStr, routeOverviewHeightStr, 
                  routeOverviewHeightValue, 
                  m_contentType == RouteMessageRequestType::WML_CONTENT );
   replaceString( *restartStr, routeOverviewAltStr, routeOverviewAlt, 
                  m_contentType == RouteMessageRequestType::WML_CONTENT );
   
   textFileName = restartTextFileName;
   textFileName.append( textFileNameExtension );
   nbrReplaces = replaceString( 
      *restartStr, startFileStr, textFileName.c_str(), 
      m_contentType == RouteMessageRequestType::WML_CONTENT );
   bool useReStartTextFile = (nbrReplaces > 0 && 
                              m_messageStartTextFmt != NULL);
   if ( useReStartTextFile ) {
      // A special text file for the text
      restartStr = &restartTextStr;
   }
   
   repStrs( startReplaceStrings, *restartStr, 
            m_contentType == RouteMessageRequestType::WML_CONTENT );
   if ( m_defaultCharacterSet != internalCharacterSet ) {
      changeCharacterSet( *restartStr, m_defaultCharacterSet );
   }

   // Used resources
   if ( m_makeImages && restartNbrRouteOverview ) {
      restartResourcesParts.push_back( getMapImageRequestImage( 
         m_nbrImageRequests - 1, routeOverviewURI ) );
      restartNbrMMSObjects++;
   }
   if ( useReStartTextFile ) {
      restartTextParts.push_back( new MimePartText( 
         (byte*)restartTextStr.c_str(), restartTextStr.size(), 
         m_textFileType, m_defaultCharacterSet,
         textFileName.c_str(), true ) );
   }
   
   //** Size of restart
   restartSize = restartFmtStr.size() * 4 / 3 + 4 + 
      (restartFmtStr.size() * 4 / 3 / lineLength + 1 )*2 + 1 + 200;
   getUsedResources( restartFmtStr, restartResourcesParts );
   
   for ( uint32 i = 0 ; i < restartResourcesParts.size() ; i++ ) {
      restartSize += getMimePartSize( restartResourcesParts[ i ] );
      restartNbrMMSObjects++;
   }
   for ( uint32 i = 0 ; i < restartTextParts.size() ; i++ ) {
      restartSize += getMimePartSize( restartTextParts[ i ] );
      restartNbrMMSObjects++;
   }


   //*** Make continue End str
   MC2String* contEndStr = &contEndFmtStr;
   textFileName = endTextFileName;
   textFileName.append( textFileNameExtension );
   nbrReplaces = replaceString(
      *contEndStr, endFileStr, textFileName.c_str(), 
      m_contentType == RouteMessageRequestType::WML_CONTENT );
   bool useContEndTextFile = nbrReplaces > 0 && 
      m_messageEndTextFmt != NULL;
   if ( useContEndTextFile ) {
      // A special text file for the text
      contEndStr = &contEndTextStr;
   }

   // contEndTextStr
   replaceString( *contEndStr, signatureStr, contEndTextMessageStr, 
                  m_contentType == RouteMessageRequestType::WML_CONTENT );

   // Used resources
   if ( useContEndTextFile ) {
      contEndResourcesParts.push_back( new MimePartText( 
         (byte*)contEndTextStr.c_str(), contEndTextStr.size(), 
         m_textFileType, m_defaultCharacterSet,
         textFileName.c_str(), true ) );
   }
   getUsedResources( contEndFmtStr, contEndResourcesParts );


   //** Size of contEnd
   contEndSize = contEndFmtStr.size() * 4 / 3 + 4 + 
      (contEndFmtStr.size() * 4 / 3 / lineLength + 1 )*2 + 1 + 200;
   for ( uint32 i = 0 ; i < contEndResourcesParts.size() ; i++ ) {
      contEndSize += getMimePartSize( contEndResourcesParts[ i ] );
      contEndNbrMMSObjects++;
   }

   
   //** Add route info to plaintext
   // Directions from origin originlocation to destination dest-location
   plainTextMessage += directions;
   plainTextMessage += " ";
   plainTextMessage += from;
   plainTextMessage += " ";
   plainTextMessage += m_originString;
   if ( m_originLocationString[ 0 ] != '\0' ) {
      plainTextMessage += " ";
      plainTextMessage += m_originLocationString;
   }
   plainTextMessage += " ";
   plainTextMessage += to;
   plainTextMessage += " ";
   plainTextMessage += m_destinationString;
   if ( m_destinationLocationString[ 0 ] != '\0' ) {
      plainTextMessage += " ";
      plainTextMessage += m_destinationLocationString;
   }
   plainTextMessage += eolStr;
   plainTextMessage += eolStr;
   
   // Route info
   plainTextMessage += routeInfo;
   plainTextMessage += ":";
   plainTextMessage += eolStr;
   plainTextMessage += vehicle;
   plainTextMessage += ": ";
   plainTextMessage += routeVehicleValue;
   plainTextMessage += eolStr;
   plainTextMessage += totalDistance;
   plainTextMessage += ": ";
   plainTextMessage += routeTotalDistance;
   plainTextMessage += eolStr;
   plainTextMessage += totalTime;
   plainTextMessage += ": ";
   plainTextMessage += routeTotalTime;
   plainTextMessage += eolStr;
   plainTextMessage += eolStr;
   // End of plaintext for now




   int32 maxRouteDescLen = 4096;
   char description[maxRouteDescLen];
   char routeTurnWidthValue[20];
   char routeTurnHeightValue[20];
   char routeTurnAlt[512];
   char landmarkDescription[maxRouteDescLen];
   MimePartImage* turnImagePart = NULL;

   for ( int32 i = 0 ; i < m_nbrStringItems ; i++ ) {
      MC2String turnFmtStr( routeTurnStr );
      bool reusedPictogram = false;

      if ( i == 0 && !(i == m_nbrStringItems - 1 && i > 0) )  {
//         setOriginRoadNumber(ctmp);
         if ( m_originLocationString[ 0 ] != '\0' ) {
            m_stringItems[ i ]->setOriginLocation( 
               m_originLocationString );
         }
         if ( strcmp( m_stringItems[ i ]->getText(), 
                      m_originString ) != 0 )
         {
            // Really not needed, is in header
            //m_stringItems[ i ]->setOriginCompany( m_originString );
         }
      } else if ( i == m_nbrStringItems - 1) {
//         setDestinationRoadNumber( ctmp );
         if ( m_destinationLocationString[ 0 ] != '\0' ) {
            m_stringItems[ i ]->setDestinationLocation( 
               m_destinationLocationString );
         }
         if ( strcmp( m_stringItems[ i ]->getText(), 
                      m_destinationString ) != 0 )
         {
            // Really not needed, is in header
            //m_stringItems[ i ]->setDestinationCompany( 
            //   m_destinationString );
         }
      }

      uint32 nbrBytesWritten = 0;
      m_stringItems[ i ]->getRouteDescription( descProps, 
                                               description, 
                                               maxRouteDescLen, 
                                               nbrBytesWritten );
      
      // Route turn 
      const char* routeTurnURI = NULL;
      routeTurnURI = m_imageURIs[ i ];
      turnImagePart = NULL;
      if ( m_turnImageType == 
           UserConstants::ROUTE_TURN_IMAGE_TYPE_MAP_IMAGE )
      {
         if ( m_makeImages && !m_onlyOverview ) {
            turnImagePart = getMapImageRequestImage( i );
         }
      } else if ( m_turnImageType == 
                  UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM||
                  m_turnImageType == 
                  UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_1 ||
                  m_turnImageType == 
                  UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_2 ||
                  m_turnImageType == 
                  UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_3 ||
                  m_turnImageType == 
                  UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_4 ||
                  m_turnImageType == 
                  UserConstants::ROUTE_TURN_IMAGE_TYPE_PICTOGRAM_SET_5 )
      {
         if ( !m_onlyOverview ) {
            turnImagePart = addImage( usedPictogramImages, 
                                      usedPictogramImageParts,
                                      routeTurnURI );
            if ( turnImagePart == NULL ) {
               reusedPictogram = true;
            }
         }
      }
      sprintf( routeTurnWidthValue, "%d", m_routeTurnWidth );
      sprintf( routeTurnHeightValue, "%d", m_routeTurnHeight );

      strcpy( routeTurnAlt, StringTable::getString( 
         m_stringItems[ i ]->getStringCode(), m_language ) );

      // Pre and Post landmarks
      MC2String routeTurnPreLandmark;
      MC2String routeTurnPostLandmark;
      if ( m_includeLandmarks && m_stringItems[ i ]->hasLandmarks() ) {
         LandmarkHead* landmarks = m_stringItems[ i ]->getLandmarks();
         LandmarkLink* landmark = static_cast<LandmarkLink*>(
            landmarks->first() );
         for ( uint32 j = 0 ; j < m_stringItems[ i ]->getNbrLandmarks() ;
               ++j )
         {
            landmark->getRouteDescription( descProps, 
                                           landmarkDescription,
                                           maxRouteDescLen, 
                                           nbrBytesWritten );
            if ( landmark->isEndLM() ) {
               routeTurnLandmarkPostStr = routeTurnLandmarkPostFmt;
               replaceString( 
                  routeTurnLandmarkPostStr, 
                  routeTurnLandmarkDescriptionStr, landmarkDescription, 
                  m_contentType == RouteMessageRequestType::WML_CONTENT );
               if ( addNewLineToLandmarkFormat ) {
                  routeTurnPostLandmark.append( messageEOLStr );
               }
               routeTurnPostLandmark.append( routeTurnLandmarkPostStr );
            } else {
               routeTurnLandmarkPreStr = routeTurnLandmarkPreFmt;
               replaceString( 
                  routeTurnLandmarkPreStr, 
                  routeTurnLandmarkDescriptionStr, landmarkDescription, 
                  m_contentType == RouteMessageRequestType::WML_CONTENT );
               routeTurnPreLandmark.append( routeTurnLandmarkPreStr );
               if ( addNewLineToLandmarkFormat && 
                    !(j+1 == m_stringItems[ i ]->getNbrLandmarks()) )
               {
                  routeTurnPreLandmark.append( messageEOLStr );
               }
            }
            landmark = static_cast<LandmarkLink*>( landmark->suc() );
         }
         if ( !routeTurnPostLandmark.empty() ) {
            if ( addNewLineToLandmarkFormat ) {
               routeTurnPostLandmark.insert( 0, messageEOLStr.c_str() );
            }
         }
         if ( !routeTurnPreLandmark.empty() ) {
            if ( addNewLineToLandmarkFormat ) {
               routeTurnPreLandmark.append( messageEOLStr );
            }
         }
      }
   
      // Replace in turn
      MC2String* turnStr = &turnFmtStr;
      replaceString( 
         *turnStr, routeTurnURIStr, routeTurnURI, 
         m_contentType == RouteMessageRequestType::WML_CONTENT );
      replaceString( 
         *turnStr, routeTurnWidthStr, routeTurnWidthValue, 
         m_contentType == RouteMessageRequestType::WML_CONTENT );
      replaceString( 
         *turnStr, routeTurnHeightStr, routeTurnHeightValue,
         m_contentType == RouteMessageRequestType::WML_CONTENT );
      replaceString( 
         *turnStr, routeTurnAltStr, routeTurnAlt, 
         m_contentType == RouteMessageRequestType::WML_CONTENT );

      textFileName = routeTurnTextFileName;
      char ctmp[20];
      sprintf( ctmp, "%d", i+1 );
      textFileName.append( ctmp );
      textFileName.append( textFileNameExtension );
      nbrReplaces = replaceString( 
         *turnStr, routeTurnFileStr, 
         textFileName.c_str(), 
         m_contentType == RouteMessageRequestType::WML_CONTENT );
      bool useTurnTextFile = (nbrReplaces > 0 || m_onlyOverview) && 
         m_messageRouteTurnTextFmt != NULL;
      if ( useTurnTextFile ) {
         // A special text file for the text
         turnTextStr = m_messageRouteTurnTextFmt;
         turnStr = &turnTextStr;
      }

      replaceString( 
         *turnStr, routeDescriptionStr, description, 
         m_contentType == RouteMessageRequestType::WML_CONTENT );
      replaceString( 
         *turnStr, routeTurnPreLandmarkStr, 
         routeTurnPreLandmark.c_str(), 
         m_contentType == RouteMessageRequestType::WML_CONTENT );
      replaceString( 
         *turnStr, routeTurnPostLandmarkStr, 
         routeTurnPostLandmark.c_str(), 
         m_contentType == RouteMessageRequestType::WML_CONTENT );


      if ( m_defaultCharacterSet != internalCharacterSet ) {
         changeCharacterSet( *turnStr, m_defaultCharacterSet );
      }
      
      // Add to plaintext
      plainTextMessage += description;
      plainTextMessage += eolStr;


      if ( useTurnTextFile ) {
         textPartVector.push_back( 
            new MimePartText( (byte*)turnTextStr.c_str(), 
                              turnTextStr.size(), 
                              m_textFileType, m_defaultCharacterSet,
                              textFileName.c_str(), true ) );
      }
      

      if ( i == 0 ) {
         // The resources for a turn
         getUsedResources( turnFmtStr, resourcesParts );
         resourcesSize = 0;
         for ( uint32 i = 0 ; i < resourcesParts.size() ; i++ ) {
            resourcesSize += getMimePartSize( resourcesParts[ i ] );
         }
      }
      
      //*** Check if a message's size is too big
      makeNewMessageIfMaxSize( message, messageSize, m_nbrMMSObjects,
                               headerSize,
                               turnFmtStr,
                               useTurnTextFile,
                               (textPartVector.size() > 0 ? 
                                textPartVector.back() : NULL),
                               m_makeImages, turnImagePart, 
                               reusedPictogram,
                               usedPictogramImages, 
                               usedPictogramImageParts, 
                               textPartVector, usedMapImageParts,
                               resourcesParts, resourcesSize,
                               contEndFmtStr, contEndResourcesParts,
                               contEndSize, contEndNbrMMSObjects,
                               restartFmtStr, restartResourcesParts,
                               restartTextParts,
                               restartSize, restartNbrMMSObjects,
                               messageMainContentType,
                               messageTextMarkupType,
                               messageApplicationMarkupType,
                               m_defaultCharacterSet, 
                               pageName );


      // Append to message
      message.append( turnFmtStr );

      if ( m_turnImageType == 
           UserConstants::ROUTE_TURN_IMAGE_TYPE_MAP_IMAGE &&
           turnImagePart != NULL ) 
      {
         usedMapImageParts.push_back( turnImagePart );
      }
   } // for all stringItems
   

   //** Replace in endStr
   // Signature
   MC2String sig( m_signature );
   if ( m_contentType == RouteMessageRequestType::WML_CONTENT ) {
      StringUtility::wapStr( tmp, m_signature );
      sig = tmp;
   }
   replaceString( sig, eolStr.c_str(), messageEOLStr.c_str() );
   replaceString( sig, shortEolStr.c_str(), messageEOLStr.c_str() );
   char webLink[ 4096 ];
   webLink[ 0 ] = '\0';
   const char* linkToRoute = StringTable::getString( 
      StringTable::PROBLEM_WITH_ROUTE_DESCRIPTION_CLICK_HERE_STR, 
      m_language );
   if ( m_makeLink && m_contentType == 
        RouteMessageRequestType::HTML_CONTENT &&
        Properties::getProperty( "DEFAULT_WEB_HOST" ) != NULL )
   {
      // Link to page with html version of this message
      const char* host = Properties::getProperty( "DEFAULT_WEB_HOST" ); 
      HttpUtility::makeRouteLink( webLink, host, "http",
                                  m_routeID, m_routeCreateTime, m_language,
                                  m_originString, m_originLocationString,
                                  m_destinationString, 
                                  m_destinationLocationString,
                                  m_signature );
      sig.append( messageEOLStr.c_str() );
      char href[ 4096 ]; 
      sprintf( href, "<a href=\"%s\">%s</a>", webLink, linkToRoute );
      sig.append( href );
   }

   MC2String* endStr = &endFmtStr;
   textFileName = endTextFileName;
   textFileName.append( textFileNameExtension );
   nbrReplaces = replaceString( 
      *endStr, endFileStr, textFileName.c_str(), 
      m_contentType == RouteMessageRequestType::WML_CONTENT );
   bool useEndTextFile = (nbrReplaces > 0 || m_onlyOverview) && 
      m_messageEndTextFmt != NULL;
   if ( useEndTextFile ) {
      // A special text file for the text
      endStr = &endTextStr;
   }

   replaceString( *endStr, signatureStr, sig.c_str(), false );

   // Plaintext signature
   plainTextMessage += eolStr;
   plainTextMessage += m_signature;
   plainTextMessage += eolStr;
   if ( m_makeLink ) {
      // Link to page with html version of this message
      plainTextMessage += linkToRoute;
      plainTextMessage += " ";
      plainTextMessage += webLink;
      plainTextMessage += eolStr;
   }

   if ( (m_contentType != RouteMessageRequestType::WML_CONTENT) && 
        (m_contentType != RouteMessageRequestType::SMIL_CONTENT) && 
        m_maxMessageSize == MAX_UINT32 ) 
   {
      //** Set plaintext version
      m_message.back()->setPlaintextMessage( 
         plainTextMessage.c_str(), true );
   }

   if ( m_defaultCharacterSet != internalCharacterSet ) {
      changeCharacterSet( *endStr, m_defaultCharacterSet );
   }

   if ( useEndTextFile ) {
      if ( endTextStr.size() == 0 ) {
         // Empty not good
         endTextStr.append( "   " );
      }
      textPartVector.push_back( 
         new MimePartText( 
            (byte*)endTextStr.c_str(), endTextStr.size(), 
            m_textFileType, m_defaultCharacterSet,
            textFileName.c_str(), true ) );
   }

   // The resources for end
   getUsedResources( endFmtStr, resourcesParts );
   resourcesSize = 0;
   for ( uint32 i = 0 ; i < resourcesParts.size() ; i++ ) {
      resourcesSize += getMimePartSize( resourcesParts[ i ] );
   }

   makeNewMessageIfMaxSize( message, messageSize, m_nbrMMSObjects,
                            headerSize,
                            endFmtStr,
                            useEndTextFile,
                            (textPartVector.size() > 0 ? 
                             textPartVector.back() : NULL),
                            false, NULL, false,
                            usedPictogramImages, 
                            usedPictogramImageParts, 
                            textPartVector, usedMapImageParts,
                            resourcesParts, resourcesSize, 
                            contEndFmtStr, contEndResourcesParts,
                            contEndSize, contEndNbrMMSObjects,
                            restartFmtStr, restartResourcesParts,
                            restartTextParts,
                            restartSize, restartNbrMMSObjects,
                            messageMainContentType,
                            messageTextMarkupType,
                            messageApplicationMarkupType,
                            m_defaultCharacterSet, 
                            pageName );


   //** Add endStr to message
   message.append( endFmtStr );

   if ( !m_realMapURLs && !m_contentLocation ) {
      // Turns links into Content-ID references
      replaceString( message, "img src=\"", "img src=\"cid:" );
      replaceString( message, "stylesheet\" href=\"", 
                     "stylesheet\" href=\"cid:" );
   }

   //*** Add route page to message
   m_message.back()->add( makeMimePart(
      message, messageMainContentType, messageTextMarkupType,
      messageApplicationMarkupType, m_defaultCharacterSet, pageName ) );


   //*** Add resources for route page
   getUsedResources( message, resourcesParts );
   for ( uint32 i = 0 ; i < resourcesParts.size() ; i++ ) {
      m_message.back()->add( resourcesParts[ i ] );
   }

   if ( true ) {
      uint32 messageLen = message.size() * 4 / 3 + 4 + 
         (message.size() * 4 / 3 / lineLength + 1)*2 + 1 + 202;
      uint32 addSize = 0;
      uint32 resourcesSize = 0;
      for ( uint32 i = 0 ; i < resourcesParts.size() ; i++ ) {
         resourcesSize += getMimePartSize( resourcesParts[ i ] );
      }

      if ( useEndTextFile && textPartVector.size() > 0 ) {
         if ( !m_onlyOverview ) {
            addSize += getMimePartSize( textPartVector.back() );
         } else {
            addSize += textPartVector.back()->getContentLength();
         }
      }
      mc2dbg8 << "Last message size messageLen " << messageLen
              << " addSize " << addSize << " messageSize " << messageSize
              << " resourcesSize " << resourcesSize << endl;
      mc2dbg4 << "Last message total size: " 
              << (messageLen +addSize+ messageSize + resourcesSize)
              << endl;
   }

   //*** Add text part files, if any
   if ( !m_onlyOverview ) {
      for ( uint32 i = 0 ; i < textPartVector.size() ; i++ ) {
         m_message.back()->add( textPartVector[ i ] );
      }
   } else {
      // Merge and add
      MC2String data;
      MC2String firstFileName;
      for ( uint32 i = 0 ; i < textPartVector.size() ; i++ ) {
         MimePartText* oldText = textPartVector[ i ];
         uint32 oldSize = 0;
         byte* olddata = oldText->getUnencodedContent( oldSize );
         data.append( reinterpret_cast<char*>( olddata ), oldSize );
         if ( i == 0 ) {
            firstFileName = oldText->getContentLocation();
         }
         // Remove old, not used anymore
         delete oldText;
      }
      if ( textPartVector.size() > 0 ) {
         m_message.back()->add( new MimePartText( 
                                   (byte*)data.c_str(), data.size(), 
                                   m_textFileType, m_defaultCharacterSet,
                                   firstFileName.c_str(), true ) );
      }
   }

   //*** Add used Pictogram Images
   for ( uint32 i = 0 ; i < usedPictogramImageParts.size() ; i++ ) {
      m_message.back()->add( usedPictogramImageParts[ i ] );
   }


   //*** Add map images
   for ( uint32 i = 0 ; i < usedMapImageParts.size() ; i++ ) {
      m_message.back()->add( usedMapImageParts[ i ] );
   }



   for ( uint32 i = 0 ; i < restartResourcesParts.size() ; i++ ) {
      delete restartResourcesParts[ i ];
   }
   restartResourcesParts.clear();

   for ( uint32 i = 0 ; i < restartTextParts.size() ; i++ ) {
      delete restartTextParts[ i ];
   }
   restartTextParts.clear();
   
   for ( uint32 i = 0 ; i < contEndResourcesParts.size() ; i++ ) {
      delete contEndResourcesParts[ i ];
   }
   contEndResourcesParts.clear();
  

   // **** Send message
   state newState = EMAIL_REQUEST;

   if ( m_sendEmail ) {
      mc2dbg1 << "RouteMessageRequest Sending " << getNbrMimeMessages() 
              << " mail to " << m_toMailAddr << endl;
      for ( uint32 i = 0 ; i < getNbrMimeMessages() ; i++ ) {
         SendEmailRequestPacket* p = new SendEmailRequestPacket( getID() );
         char* body = m_message[ i ]->getMimeMessageBody();
         const char* optionalHeaderTypes[ 2 ] = 
         { MimeMessage::mimeVersionHeader, 
           MimeMessage::contentTypeHeader };
         const char* optionalHeaderValues[ 2 ] = 
         { m_message[ i ]->getMimeVersion(), 
           m_message[ i ]->getContentType() };
         const char* partStr = StringTable::getString( StringTable::PART,
                                                       m_language );
         char subject[ strlen( m_subject ) + strlen( partStr ) + 10 + 1 ];
         if ( getNbrMimeMessages() != 1 ) {
            sprintf( subject, "%s %s %d", m_subject, partStr, i+1 );
         } else {
            strcpy( subject, m_subject );
         }
   
         if ( p->setData( m_toMailAddr, m_fromMailAddr, subject, body,
                          2, optionalHeaderTypes, optionalHeaderValues ) )
         {
            m_emailRequest.push_back(
               new PacketContainer( p, 0, 0 , MODULE_TYPE_SMTP, EMAILREQUESTPACKET_TIMEOUT) );
            newState = EMAIL_REQUEST;
         } else {
            mc2log << error << "RouteMessageRequest::makeEmailRequest "
               "SendEmailRequestPacket::setData failed." << endl;
            newState = ERROR;
            setDone( true );
         }

         delete body;
      }
      if ( !m_emailRequest.empty() ) {
         m_sendEmailRequest = m_emailRequest.front();
         m_emailRequest.pop_front();
      } else {
         mc2log << warn << "RouteMessageRequest::makeEmailRequest " 
                << "no email to send!" << endl;
         newState = DONE;
         setDone( true );
      }
   } else {
      newState = DONE;
      setDone( true );  
   }

   return newState;
}


int 
RouteMessageRequest::loadFile( byte*& buff, const char* fileName, 
                               bool binary ) 
{
   char filePath[ 4096 ];

   if ( fileName[ 0 ] != '/' ) { 
      // Add MESSAGE_TEMPLATE_DIR
      const char* messageTemplatePath = Properties::getProperty( 
         "MESSAGE_TEMPLATE_DIR" );
      if ( messageTemplatePath == NULL ) {
         messageTemplatePath = ".";
      }
      sprintf( filePath, "%s/%s", messageTemplatePath, fileName );
   } else {
      // Absolute path
      strcpy( filePath, fileName );
   }

   struct stat status;
   FILE* file = fopen( filePath, "rb" );
   int length = -1;

   if ( file != NULL ) {
      stat( filePath, &status );
      length = status.st_size;
      buff = new byte[ length + !binary ];
      if ( fread( buff, 1, length, file ) != size_t(length) ) {
         delete [] buff;
         buff = NULL;
         length = -1;
      }
      fclose( file );
   }

   return length;
}


int 
RouteMessageRequest::saveFile( byte*& buff, uint32 size, 
                               const char* fileName ) 
{
   char filePath[ 4096 ];

   const char* messageTemplatePath = Properties::getProperty( 
      "MESSAGE_TEMPLATE_DIR" );
   if ( messageTemplatePath == NULL ) {
      messageTemplatePath = ".";
   }

   strcpy( filePath, messageTemplatePath );
   strcat( filePath, "/" );
   if ( strncmp( fileName, "file://", 7 ) == 0 ) {
      // Remove file://
      strcat( filePath, fileName + 7 );
   } else {
      strcat( filePath, fileName );
   }

   FILE* file = fopen( filePath, "wb" );
   int length = -1;

   if ( file != NULL ) {
      length = fwrite( buff, size, 1, file );
      if ( length != 1 ) {
         mc2log << warn << "RouteMessageRequest::saveFile " << filePath
                << " falied error " << length << endl;
         length = -1;
      }
      fclose( file );
   }

   return length;
}


uint32
RouteMessageRequest::replaceString( MC2String& str, const char* findStr, 
                                    const char* replaceStr, 
                                    bool wapFormat )
{
   uint32 replaceStringLength = 0;
   uint32 findStrLength = strlen( findStr );
   MC2String::size_type pos = str.find( findStr );
   uint32 nbrReplaces = 0;
   char wapStr[ 4096 ];
   if ( wapFormat ) {
      replaceStringLength = StringUtility::wapStr( wapStr, replaceStr );
      replaceStr = wapStr;
   } else {
      replaceStringLength = strlen( replaceStr );   
   }

   while ( pos != MC2String::npos ) {
      str.replace( pos, findStrLength, replaceStr );
      nbrReplaces++;
      pos = str.find( findStr, pos + replaceStringLength );
   }

   return nbrReplaces;
}


RouteMessageRequestType::MessageContent 
RouteMessageRequest::MessageContentFromString( 
   const char* str, RouteMessageRequestType::MessageContent defaultType )
{
   for ( uint32 i = RouteMessageRequestType::HTML_CONTENT ;
         i < RouteMessageRequestType::NBR_CONTENT ; i++ ) 
   {
      if ( StringUtility::strcasecmp( m_messageContentsStr[ i ],
                                      str ) == 0 )
      {
         return RouteMessageRequestType::MessageContent( i );
      }
   }

   return defaultType;
}


const char*
RouteMessageRequest::getPictogramForTurn( StringTable::stringCode turn ) {
   switch( turn ) {
      case StringTable::LEFT_TURN :
         return "arrow_left.";
      case StringTable::RIGHT_TURN :
         return "arrow_right.";
      case StringTable::AHEAD_TURN :
         return "arrow_ahead.";
      case StringTable::U_TURN :
         return "arrow_uturn.";
      case StringTable::FOLLOWROAD_TURN :
         return "arrow_followroad.";
      case StringTable::ENTER_ROUNDABOUT_TURN :
         return "arrow_enter_roundabout.";
      case StringTable::EXIT_ROUNDABOUT_TURN :
         return "arrow_exit_roundabout.";
      case StringTable::AHEAD_ROUNDABOUT_TURN :
         return "arrow_ahead_roundabout.";
      case StringTable::RIGHT_ROUNDABOUT_TURN :
         return "arrow_right_roundabout.";
      case StringTable::LEFT_ROUNDABOUT_TURN :
         return "arrow_left_roundabout.";
      case StringTable::OFF_RAMP_TURN :
         return "arrow_off_ramp.";
      case StringTable::LEFT_OFF_RAMP_TURN :
         return "arrow_keep_left.";
      case StringTable::RIGHT_OFF_RAMP_TURN :
         return "arrow_off_ramp.";
      case StringTable::ON_RAMP_TURN :
         return "arrow_on_ramp.";
      case StringTable::PARK_CAR :
         return "arrow_park_car.";
      case StringTable::DRIVE_FINALLY :
         return "arrow_finish.";
      case StringTable::DRIVE_START :
         return "arrow_start.";
      case StringTable::KEEP_LEFT :
         return "arrow_keep_left.";
      case StringTable::KEEP_RIGHT :
         return "arrow_keep_right.";
      case StringTable::ENTER_FERRY_TURN :
         return "arrow_enter_ferry.";
      case StringTable::EXIT_FERRY_TURN :
         return "arrow_exit_ferry.";
      case StringTable::CHANGE_FERRY_TURN :
         return "arrow_change_ferry.";
      case StringTable:: DRIVE_START_WITH_UTURN:
         return "arrow_start."; // FIXME: Uturn start not just start
      case StringTable:: U_TURN_ROUNDABOUT_TURN:
         return "arrow_uturn_roundabout.";
      case StringTable::ENTER_BUS_TURN :
         return "arrow_enter_bus.";
      case StringTable::EXIT_BUS_TURN :
         return "arrow_exit_bus.";
      case StringTable::CHANGE_BUS_TURN :
         return "arrow_change_bus.";
      case StringTable::ON_MAIN_TURN :
         return "arrow_on_main."; 
      case StringTable::OFF_MAIN_TURN :
         return "arrow_off_main.";   
         
      default:
         return "arrow_empty.";
   }
}


void 
RouteMessageRequest::changeCharacterSet( 
   MC2String& str, MimePart::characterSet outCharSet)
{
   MC2String res;

   if ( outCharSet == MimePart::CHARSET_UTF_8 ) {
      res = UTF8Util::mc2ToUtf8( str );
   } else { // CHARSET_ISO_8859_1
      res = UTF8Util::mc2ToIso( str );
   }

   str = res;
}


uint32 
RouteMessageRequest::getMimePartSize( const MimePart* part ) const {
   uint32 size = 0 ;
   
   size += part->getContentLength();
   size += 18; // "Content-Location: "
   size += strlen( part->getContentLocation() );
   size += strlen( m_message.back()->contentTypeHeader );
   size += strlen( part->getContentType() );
   size += 27; //"Content-Transfer-Encoding: "
   size += strlen( part->getTransferEncoding() );
   size += strlen( m_message.back()->getBoundaryText() );
   size += 10; // 5 CRLF
   size += 2; // "--"

   return size;
}


MimePartImage* 
RouteMessageRequest::getMapImageRequestImage( 
   uint32 i,
   const char* uri )
{
   if ( i >= m_nbrImageRequests ) {
      return NULL;
   }

   MimePartImage* res = NULL;
   // Get answer image
   PacketContainer* ansCont = m_imageRequests[ i ]->getAnswer();
   GfxFeatureMapImageReplyPacket* ans = static_cast<
      GfxFeatureMapImageReplyPacket* >( ansCont->getPacket() );
   byte* buff = ans->getImageData();

   if ( buff != NULL ) {
      res = new MimePartImage( 
         buff, ans->getSize(), 
         MimePartImage::imageTypeFromImageFormat( m_format ),
         uri ? uri : m_imageURIs[ i ], false );
   } else {
      res = new MimePartImage( 
         buff, 0, 
         MimePartImage::imageTypeFromImageFormat( m_format ),
         uri ? uri : m_imageURIs[ i ], false );
      if ( !m_useNavTurnBoundingBox ) {
         mc2log << error << "RouteMessageRequest::getMapImageRequestImage "
            "GfxFeatureMapImageReplyPacket returned NULL." 
                << endl
                << "For URI " 
                << m_imageURIs[ i ] << endl;
      }
   }

   delete [] buff;
   
   return res;
}


bool
RouteMessageRequest::makeNewMessageIfMaxSize( 
   MC2String& message, uint32& messageSize, uint32& nbrMMSObjects,
   uint32 headerSize,
   const MC2String& fmtStr,
   bool useTurnTextFile, const MimePartText* textFilePart,
   bool makeImages, const MimePartImage* turnImagePart,
   bool reusedPictogram,
   vector< MC2String >& usedPictogramImages,
   vector< MimePartImage* >& usedPictogramImageParts,
   vector<MimePartText*>& textPartVector,
   vector< MimePartImage* >& usedMapImageParts,
   vector< MimePart* >& resourcesParts,  uint32& resourcesSize,
   const MC2String& contEndFmtStr,
   vector< MimePart* >& contEndResourcesParts, uint32 contEndSize,
   uint32 contEndNbrMMSObjects,
   const MC2String& restartFmtStr,
   vector< MimePart* >& restartResourcesParts, 
   vector< MimePartText* >& restartTextParts,
   uint32 restartSize, uint32 restartNbrMMSObjects,
   MimePart::mainContentType mainContentType,
   MimePartText::contentType messageTextMarkupType,
   MimePartApplication::contentType messageApplicationMarkupType,
   MimePart::characterSet characterSet, 
   const char* pageName )
{
   bool newMessage = false;
   uint32 addSize = 0;
   uint32 addNbrMMSObjects = 0;
   
   if ( turnImagePart != NULL ) {
      addSize += getMimePartSize( turnImagePart );
      addNbrMMSObjects++;
   } else if ( reusedPictogram ) {
      // Counts as an MMSObject in some models
      addNbrMMSObjects++;
   }
   if ( useTurnTextFile && textFilePart != NULL ) {
      if ( !m_onlyOverview ) {
         addSize += getMimePartSize( textFilePart );
         addNbrMMSObjects++;
      } else {
         addSize += textFilePart->getContentLength();
      }
   }
   
   uint32 lineLength = 78;
   uint32 fmtStrLen = fmtStr.size() * 4 / 3 + 4 + 
      (fmtStr.size() * 4 / 3 / lineLength + 1)*2 + 1 + 2;

   uint32 messageLen = message.size() * 4 / 3 + 4 + 
      (message.size() * 4 / 3 / lineLength + 1)*2 + 1 + 202;

   if ( (addSize + fmtStrLen + messageLen + messageSize + contEndSize +
         resourcesSize) > m_maxMessageSize ||
        ( m_contentType == RouteMessageRequestType::SMIL_CONTENT &&
          (addNbrMMSObjects + nbrMMSObjects + contEndNbrMMSObjects + 1 > 
           m_maxNbrMMSObjects ) ) ) 
   {
      newMessage = true;
      mc2dbg4 << "New message messageLen " << messageLen 
              << " messageSize " << messageSize << " contEndSize " 
              << contEndSize << " resourcesSize " << resourcesSize << endl;
      mc2dbg4 << "Total size: " 
              << (messageLen + messageSize + contEndSize + resourcesSize)
              << endl;
      mc2dbg4 << " addSize " << addSize << " fmtStrLen " << fmtStrLen 
              << endl;

      // End this message
      message.append( contEndFmtStr );
      
      MimePart* messagePart = makeMimePart(
         message, mainContentType, messageTextMarkupType,
         messageApplicationMarkupType, characterSet, pageName );
      m_message.back()->add( messagePart );
      
      // Add all resources for this message

      // Add Cont end resources
      for ( uint32 i = 0 ; i < contEndResourcesParts.size() ; i++ ) {
         resourcesParts.push_back( 
            contEndResourcesParts[ i ]->getClone() );
      }
      
      // Standard resources
      getUsedResources( message, resourcesParts );
      for ( uint32 i = 0 ; i < resourcesParts.size() ; i++ ) {
         m_message.back()->add( resourcesParts[ i ] );
      }
      resourcesParts.clear();
      resourcesSize = 0;
      // Text part files
      if ( !m_onlyOverview ) {
         for ( uint32 i = 0 ; i + 1 < textPartVector.size() ; i++ ) {
            m_message.back()->add( textPartVector[ i ] );
         }
      } else {
         // Merge and add
         MC2String data;
         MC2String firstFileName;
         for ( uint32 i = 0 ; i + 1 < textPartVector.size() ; i++ ) {
            MimePartText* oldText = textPartVector[ i ];
            uint32 oldSize = 0;
            byte* olddata = oldText->getUnencodedContent( oldSize );
            if ( olddata ) {
               data.append( reinterpret_cast<char*>( olddata ), oldSize );
               delete [] olddata;
               olddata = NULL;
            }

            if ( i == 0 ) {
               firstFileName = oldText->getContentLocation();
            }
            // Remove old, not used anymore
            delete oldText;
         }
         if ( textPartVector.size() > 1 ) {
            m_message.back()->add( new MimePartText( 
                                      (byte*)data.c_str(), data.size(), 
                                      m_textFileType, 
                                      m_defaultCharacterSet,
                                      firstFileName.c_str(), true ) );
         }
      }
      MimePartText* lastText = NULL;
      if ( textPartVector.size() > 0 ) {
         lastText = textPartVector.back();
      }
      textPartVector.clear();
      if ( lastText != NULL ) {
         textPartVector.push_back( lastText );
      }
      // Pictograms
      for ( uint32 i = 0 ; i + 1 < usedPictogramImageParts.size() ; i++ ) {
         m_message.back()->add( usedPictogramImageParts[ i ] );
      }
      MimePartImage* lastPictogram = NULL;
      MC2String lastPictogramURI;
      if ( usedPictogramImageParts.size() > 0 ) {
         if (turnImagePart == usedPictogramImageParts.back() ) 
         {
            // The last pictogram is the one that didn't fit
            lastPictogramURI = usedPictogramImages.back();
            lastPictogram = usedPictogramImageParts.back();
         } else {
            m_message.back()->add( usedPictogramImageParts.back() );
         }
      }
      usedPictogramImageParts.clear();
      usedPictogramImages.clear();
      if ( lastPictogram != NULL ) {
         usedPictogramImages.push_back( lastPictogramURI );
         usedPictogramImageParts.push_back( lastPictogram );
      }
      // Map images
      for ( uint32 i = 0 ; i < usedMapImageParts.size() ; i++ ) {
         m_message.back()->add( usedMapImageParts[ i ] );
      }
      usedMapImageParts.clear();


      // Create and start new continue message
      m_message.push_back( new MimeMessage() );
      if ( mainContentType == MimePart::MAIN_CONTENT_TYPE_TEXT ) {
         m_message.back()->setMainContentType( 
            MimePartText::getContentTypeAsString(
               messageTextMarkupType ) );
      } else if ( mainContentType == 
                  MimePart::MAIN_CONTENT_TYPE_APPLICATION )
      {
         m_message.back()->setMainContentType( 
            MimePartApplication::getContentTypeAsString(
               messageApplicationMarkupType ) );  
      }
      messageSize = headerSize;
      message = restartFmtStr;
      nbrMMSObjects = restartNbrMMSObjects;
      for ( uint32 i = 0 ; i < restartResourcesParts.size() ; i++ ) {
         resourcesParts.push_back( 
            restartResourcesParts[ i ]->getClone() );
         resourcesSize += getMimePartSize( restartResourcesParts[ i ] );
      }
      for ( uint32 i = 0 ; i < restartTextParts.size() ; i++ ) {
         // Insert first
         textPartVector.insert( textPartVector.begin(),
            static_cast< MimePartText* > ( 
               restartTextParts[ i ]->getClone() ) );
         resourcesSize += getMimePartSize( restartTextParts[ i ] );
      }
   }
   messageSize += addSize;
   nbrMMSObjects += addNbrMMSObjects;

   return newMessage;
}


MimePartImage*
RouteMessageRequest::addImage( vector< MC2String >& uris,
                               vector< MimePartImage* >& imageParts,
                               const char* uri ) const
{
   MimePartImage* res = NULL;
   if ( find( uris.begin(), uris.end(), uri ) == uris.end() ) {
      // Not found Add it
      byte* buff = NULL;
      uint32 size = loadFile( buff, uri, true );
      if ( size > 0 ) {
         res = new MimePartImage( 
            buff, size, 
            MimePartImage::imageTypeFromImageFormat( m_format ), 
            uri, true );
         imageParts.push_back( res );
         uris.push_back( uri );
      } else {
         mc2log << warn << "RouteMessageRequest::addImage "
            "couldn't load image " << uri << endl;
      }
      delete buff;
   }

   return res;
}


void 
RouteMessageRequest::getUsedResources( 
   const MC2String& str, 
   vector< MimePart* >& resourcesParts,
   bool checkUnique )
{
   for ( uint32 i = 0 ; i < m_nbrMessageResources ; i++ ) {
      if ( str.find(  m_messageResourcesURI[ i ] ) != MC2String::npos ) {
         if ( !checkUnique || 
              find_if( resourcesParts.begin(), resourcesParts.end(), 
                       bind2nd(mimePartEq(), m_messageResourcesURI[ i ] ) )
              == resourcesParts.end() )
         {
            // Resource used (and unique) -> add it
            resourcesParts.push_back( MimeMessage::createMimePartForFile(  
               m_messageResourcesURI[ i ],
               m_messageResourcesBuff[ i ], 
               m_messageResourcesBuffLength[ i ],
               m_messageResourcesURI[ i ], true ) );
         }
      }
   }
}


MimePart* 
RouteMessageRequest::makeMimePart( 
   const MC2String& content, 
   MimePart::mainContentType mainContentType,
   MimePartText::contentType messageTextMarkupType,
   MimePartApplication::contentType messageApplicationMarkupType,
   MimePart::characterSet characterSet, 
   const char* pageName )
{
   MimePart* res = NULL;
   if ( mainContentType == MimePart::MAIN_CONTENT_TYPE_TEXT ) {
      res = new MimePartText( 
         (byte*)content.c_str(), content.size(), 
         messageTextMarkupType, characterSet,
         pageName, true );
   } else if ( mainContentType == MimePart::MAIN_CONTENT_TYPE_APPLICATION )
   {
      res = new MimePartApplication(   
         (byte*)content.c_str(), content.size(), 
         messageApplicationMarkupType, pageName, true,
         characterSet );
   }

   return res;
}

   
const char* const
RouteMessageRequest::m_messageStartFmtFile[ 
   RouteMessageRequestType::NBR_CONTENT ] = {
      "startfmt.html",
      "mmstartfmt.wml",
      "smilstartfmt.smil",
   };


const char* const
RouteMessageRequest::m_messageRestartFmtFile[ 
   RouteMessageRequestType::NBR_CONTENT ] = {
      "restartfmt.html",
      "mmrestartfmt.wml",
      "smilrestartfmt.smil",
   };


const char* const
RouteMessageRequest::m_messageStartTextFmtFile[ 
   RouteMessageRequestType::NBR_CONTENT ] = {
      "",
      "",
      "smilstarttext.txt",
   };


const char* const
RouteMessageRequest::m_messageRouteTurnFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "turnfmt.html",
      "mmturnfmt.wml",
      "smilturnfmt.smil",
   };


const char* const
RouteMessageRequest::m_messageRouteTurnTextFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "",
      "",
      "smilturntext.txt",
   };


const char* const
RouteMessageRequest::m_messageRoutePreTurnLandmarkFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "turnlandmarkfmt.txt",
      "mmturnlandmarkfmt.txt",
      "smilturnlandmarkfmt.txt",
   };


const char* const
RouteMessageRequest::m_messageRoutePostTurnLandmarkFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "turnlandmarkfmt.txt",
      "mmturnlandmarkfmt.txt",
      "smilturnlandmarkfmt.txt",
   };


const char* const
RouteMessageRequest::m_messageEndFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "endfmt.html",
      "mmendfmt.wml",
      "smilendfmt.smil",
   };


const char* const
RouteMessageRequest::m_messageEndTextFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "",
      "",
      "smilendtext.txt",
   };


const char* const
RouteMessageRequest::m_messageOverviewOnlyStartFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "overviewonlystartfmt.html",
      "overviewonlymmstartfmt.wml",
      "overviewonlysmilstartfmt.smil",
   };


const char* const
RouteMessageRequest::m_messageOverviewOnlyRestartFmtFile[ 
   RouteMessageRequestType::NBR_CONTENT ] = {
      "overviewonlyrestartfmt.html",
      "overviewonlymmrestartfmt.wml",
      "overviewonlysmilrestartfmt.smil",
   };


const char* const
RouteMessageRequest::m_messageOverviewOnlyStartTextFmtFile[ 
   RouteMessageRequestType::NBR_CONTENT ] = {
      "",
      "",
      "overviewonlysmilstarttext.txt",
   };


const char* const
RouteMessageRequest::m_messageOverviewOnlyRouteTurnFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "overviewonlyturnfmt.html",
      "overviewonlymmturnfmt.wml",
      "overviewonlysmilturnfmt.smil",
   };


const char* const
RouteMessageRequest::m_messageOverviewOnlyRouteTurnTextFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "",
      "",
      "overviewonlysmilturntext.txt",
   };


const char* const
RouteMessageRequest::m_messageOverviewOnlyRoutePreTurnLandmarkFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "overviewonlyturnlandmarkfmt.txt",
      "overviewonlymmturnlandmarkfmt.txt",
      "overviewonlysmilturnlandmarkfmt.txt",
   };


const char* const
RouteMessageRequest::m_messageOverviewOnlyRoutePostTurnLandmarkFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "overviewonlyturnlandmarkfmt.txt",
      "overviewonlymmturnlandmarkfmt.txt",
      "overviewonlysmilturnlandmarkfmt.txt",
   };


const char* const
RouteMessageRequest::m_messageOverviewOnlyEndFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "overviewonlyendfmt.html",
      "overviewonlymmendfmt.wml",
      "overviewonlysmilendfmt.smil",
   };


const char* const
RouteMessageRequest::m_messageOverviewOnlyEndTextFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "",
      "",
      "overviewonlysmilendtext.txt",
   };


const char* const
RouteMessageRequest::m_messageResourcesFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "resources.txt",
      "mmresources.txt",
      "smilresources.txt",
   };


const char* 
RouteMessageRequest::directionsStr = "%DIRECTIONS%";


const char* 
RouteMessageRequest::fromStr = "%FROM%";


const char* 
RouteMessageRequest::toStr = "%TO%";


const char* 
RouteMessageRequest::originStr = "%ORIGINSTRING%";


const char* 
RouteMessageRequest::originLocationStr = "%ORIGINLOCATIONSTRING%";


const char* 
RouteMessageRequest::destinationStr = "%DESTINATIONSTRING%";


const char* 
RouteMessageRequest::destinationLocationStr = 
"%DESTINATIONLOCATIONSTRING%";


const char* 
RouteMessageRequest::routeOverviewURIStr = "%ROUTEOVERVIEW_URI%";


const char* 
RouteMessageRequest::routeOverviewWidthStr = "%ROUTEOVERVIEW_WIDTH%";


const char* 
RouteMessageRequest::routeOverviewHeightStr = "%ROUTEOVERVIEW_HEIGHT%";


const char* 
RouteMessageRequest::routeOverviewAltStr = "%ROUTEOVERVIEW_ALT%";


const char* 
RouteMessageRequest::routeInfoStr = "%ROUTE_INFO%";


const char* 
RouteMessageRequest::vehicleStr = "%VEHICLE%";


const char* 
RouteMessageRequest::routeVehicleStr = "%ROUTE_VEHICLE%";


const char* 
RouteMessageRequest::totalDistanceStr = "%TOTAL_DISTANCE%";


const char* 
RouteMessageRequest::routeTotalDistanceStr = "%ROUTE_TOTAL_DISTANCE%";


const char* 
RouteMessageRequest::totalTimeStr = "%TOTAL_TIME%";


const char* 
RouteMessageRequest::routeTotalTimeStr = "%ROUTE_TOTAL_TIME%";


const char* 
RouteMessageRequest::routeDescriptionStr = "%ROUTE_DESCRIPTION%";


const char* 
RouteMessageRequest::routeTurnURIStr = "%ROUTE_TURN_URI%";


const char* 
RouteMessageRequest::routeTurnWidthStr = "%ROUTE_TURN_WIDTH%";


const char* 
RouteMessageRequest::routeTurnHeightStr = "%ROUTE_TURN_HEIGHT%";


const char* 
RouteMessageRequest::routeTurnAltStr = "%ROUTE_TURN_ALT%";


const char*
RouteMessageRequest::routeTurnPreLandmarkStr = "%ROUTE_PRE_LANDMARKS%";


const char*
RouteMessageRequest::routeTurnPostLandmarkStr = "%ROUTE_POST_LANDMARKS%";


const char*
RouteMessageRequest::routeTurnLandmarkDescriptionStr = 
"%ROUTE_LANDMARK_DESCRIPTION%";


const char* 
RouteMessageRequest::signatureStr = "%SIGNATURE%";


const char* 
RouteMessageRequest::startFileStr = "%START_FILE%";


const char* 
RouteMessageRequest::routeTurnFileStr = "%ROUTE_TURN_FILE%";


const char* 
RouteMessageRequest::endFileStr = "%END_FILE%";


const char * const
RouteMessageRequest::m_messageContentsStr[ 
   RouteMessageRequestType::NBR_CONTENT ] = 
{
   "html",
   "wml",
   "smil",
};

/*
#ifdef undef_level_1
#   undef DEBUG_LEVEL_1
#endif
#ifdef undef_level_2
#   undef DEBUG_LEVEL_2
#endif
#ifdef undef_level_4
#   undef DEBUG_LEVEL_4
#endif
#ifdef undef_level_8
#   undef DEBUG_LEVEL_8
#endif
*/
