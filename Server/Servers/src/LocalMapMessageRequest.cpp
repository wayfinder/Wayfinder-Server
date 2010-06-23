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

#include "LocalMapMessageRequest.h"
#include "SendEmailPacket.h"
#include "HttpUtility.h"
#include "Properties.h"
#include "GfxFeatureMapImageRequest.h"
#include "GfxFeatureMapImagePacket.h"
#include "GfxUtility.h"
#include "RouteMessageRequest.h"
#include "StringUtility.h"
#include "Utility.h"
#include "ClientSettings.h"

LocalMapMessageRequest::LocalMapMessageRequest( 
   const RequestData& data,
   int32 northLat, int32 westLon,
   int32 southLat, int32 eastLon,
   StringTable::languageCode lang,
   uint16 localMapWidth,
   uint16 localMapHeight,
   const char* localMapString,
   const TopRegionRequest* topReq,
   const ClientSetting* clientSetting,
   GfxFeatureMap* markItems,
   bool makeImages,
   bool makeLink,
   bool realMapURLs,
   RouteMessageRequestType::MessageContent contentType,
   uint32 maxMessageSize,
   bool sendEmail,
   const char* toMailAddr,
   const char* fromMailAddr,
   const char* subject,
   const char* signature,
   ImageDrawConfig::imageFormat defaultFormat,
   const MC2String& copyright,
   const UserItem* user,
   bool contentLocation,
   MapSettingsTypes::defaultMapSetting mapSetting,
   struct MapSettingsTypes::ImageSettings* imageSettings,
   const char* messageFmt,
   const char* messageStartTextFmt,
   const char* messageEndTextFmt,
   uint32 nbrMessageResources,
   char** messageResourcesURI,
   uint32* messageResourcesBuffLength,
   byte** messageResourcesBuff ):
   Request( data ),
   m_copyright( copyright )
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
   m_emailRequest = NULL;
   m_sendEmail = sendEmail;
   m_toMailAddr = toMailAddr;
   m_fromMailAddr = fromMailAddr;
   m_subject = subject;
   m_contentType = contentType;
   m_makeImages = makeImages;
   m_makeLink = makeLink;
   m_realMapURLs = realMapURLs;
   m_user = user;
   m_clientSetting = clientSetting;

   bool deleteMessageFmt = false;
   if ( messageFmt != NULL && nbrMessageResources != MAX_UINT32 &&
        messageResourcesURI != NULL && 
        messageResourcesBuffLength != NULL && 
        messageResourcesBuff != NULL )
   {
      // Use user supplied message
      deleteMessageFmt = false;
   } else {
      // Get default message
      char* msf = NULL;
      char* mstf = NULL;
      char* metf = NULL;
      char** mruf = NULL;
      uint32* mrbl = NULL;
      byte** mrbf = NULL;

      getDefaultMessage( msf, mstf, metf,
                         nbrMessageResources,
                         mruf, mrbl, mrbf );
      messageFmt = msf;
      messageStartTextFmt = mstf;
      messageEndTextFmt = metf;
      messageResourcesURI = mruf;
      messageResourcesBuffLength = mrbl;
      messageResourcesBuff = mrbf;

      deleteMessageFmt = true;
   }

   // Check if message data is ok
   if ( messageFmt != NULL && nbrMessageResources != MAX_UINT32 &&
        messageResourcesURI != NULL && 
        messageResourcesBuffLength != NULL && 
        messageResourcesBuff != NULL )
   {
      if ( signature == NULL ) {
         signature = "";
      }
      // All is ok, make image requests and mimemessage
      m_state = makeInitialSetup( northLat, westLon,
                                  southLat, eastLon,
                                  lang,
                                  localMapWidth,
                                  localMapHeight,
                                  localMapString,
                                  markItems,
                                  makeImages,
                                  makeLink,
                                  realMapURLs,
                                  contentType,
                                  maxMessageSize,
                                  signature,
                                  defaultFormat,
                                  mapSetting,
                                  imageSettings,
                                  messageFmt,
                                  messageStartTextFmt,
                                  messageEndTextFmt,
                                  nbrMessageResources,
                                  messageResourcesURI,
                                  messageResourcesBuffLength,
                                  messageResourcesBuff,
                                  topReq);
   } else {
      // Error nothing to do about it
      mc2log << "LocalMapMessageRequest::LocalMapMessageRequest "
             << "some indata in NULL." << endl;
      setDone( true );
      m_state = ERROR;
   }

   if ( deleteMessageFmt ) {
      // Delete message fmt
      delete [] messageFmt;
      delete [] messageStartTextFmt;
      delete [] messageEndTextFmt;      

      for ( uint32 i = 0 ; i < nbrMessageResources ; i++ ) {
         if ( messageResourcesURI != NULL ) {
            delete messageResourcesURI[ i ];
         }
         if ( messageResourcesBuff != NULL ) {
            delete messageResourcesBuff[ i ];
         }
      }
      delete [] messageResourcesURI;
      delete [] messageResourcesBuff;
      delete [] messageResourcesBuffLength;
   }
}


LocalMapMessageRequest::~LocalMapMessageRequest() {
   for ( uint32 i = 0 ; i < m_nbrImageRequests ; i++ ) {
      delete m_imageRequests[ i ];
      delete m_imageURIs[ i ];
   }
   delete [] m_imageRequests;
   delete [] m_imageURIs;
   delete m_mapSettings;
   delete m_answer;
   delete m_emailRequest;
}


PacketContainer*
LocalMapMessageRequest::getNextPacket() {
   PacketContainer* res = NULL; 

   switch ( m_state ) {
      case IMAGE_REQUESTS : 
         if ( m_currentImageRequest < m_nbrImageRequests ) {
            res = m_imageRequests[ m_currentImageRequest ]->
               getNextPacket();
         }
         break;
      case EMAIL_REQUEST :
         res = m_emailRequest;
         m_emailRequest = NULL;
         break;
      case DONE :
         // Done, nothing to send
         break;
      case INITIAL :
      case ERROR :
         mc2log << error << "LocalMapMessageRequest::getNextPacket called in "
            "wrong state(INITIAL|ERROR) something is very wrong" << endl;
         break;

   }

   return res;
}


void
LocalMapMessageRequest::processPacket( PacketContainer* ansCont ) {
   switch ( m_state ) {
      case IMAGE_REQUESTS : 
         if ( m_currentImageRequest < m_nbrImageRequests ) {
            m_imageRequests[ m_currentImageRequest ]->processPacket(
               ansCont );
            if ( m_imageRequests[ m_currentImageRequest ]->requestDone() )
            {
               mc2dbg2 << "LocalMapMessageRequest ImageRequest " 
                       << (m_currentImageRequest+1)
                       << " done of " << m_nbrImageRequests << endl;
               // Get answer image
               PacketContainer* ansCont = 
                  m_imageRequests[ m_currentImageRequest ]->getAnswer();
               GfxFeatureMapImageReplyPacket* ans = static_cast<
                  GfxFeatureMapImageReplyPacket* >( ansCont->getPacket() );
               byte* buff = ans->getImageData();

               if ( buff != NULL ) {
                  m_message.add( new MimePartImage( 
                     buff, ans->getSize(), 
                     MimePartImage::imageTypeFromImageFormat( m_format ),
                     m_imageURIs[ m_currentImageRequest ], false ) );
               } else {
                  mc2log << error << "LocalMapMessageRequest::"
                     "processPacket "
                     "GfxFeatureMapImageReplyPacket returned NULL." 
                         << endl
                         << "For URI " 
                         << m_imageURIs[ m_currentImageRequest ] << endl;
               }
               delete buff;
               delete ansCont;
               m_currentImageRequest++;
               if ( m_currentImageRequest >= m_nbrImageRequests ) {
                  // All image requests processed
                  m_state = makeEmailRequest();
               }
            } // Else continue with this image request
         } else {
            delete ansCont;
            mc2log << error << "LocalMapMessageRequest::processPacket "
               "called"
               " in wrong state(IMAGE_REQUESTS) something is very wrong" 
                   << endl;
         }
         break;
      case EMAIL_REQUEST :
         m_answer = ansCont;
         if ( m_answer != NULL ) {
            m_state = DONE;
         } else {
            m_state = ERROR;
         }
         setDone( true );
         break;
      case DONE :
         // Done, nothing to do
         delete ansCont;
         break;
      case INITIAL :
      case ERROR :
         delete ansCont;
         mc2log << error << "LocalMapMessageRequest::processPacket "
            "called in wrong state(INITIAL|ERROR) something is very wrong"
                << endl;
         break;

   }   
   
}


PacketContainer*
LocalMapMessageRequest::getAnswer() {
   return m_answer;
}


void 
LocalMapMessageRequest::getDefaultMessage( 
   char*& messageFmt,
   char*& messageStartTextFmt,
   char*& messageEndTextFmt,
   uint32& nbrMessageResources,
   char**& messageResourcesURI,
   uint32*& messageResourcesBuffLength,
   byte**& messageResourcesBuff )
{
   byte* buff = NULL;
   int length = 0;

   length = RouteMessageRequest::loadFile( 
      buff, m_messageFmtFile[ m_contentType ] );
   if ( length > 0 ) {
      buff[ length ] = '\0';
      messageFmt = (char*)buff;
   }

   length = RouteMessageRequest::loadFile( 
      buff, m_messageStartTextFmtFile[ m_contentType ] );
   if ( length > 0 ) {
      buff[ length ] = '\0';
      messageStartTextFmt = (char*)buff;
   }

   length = RouteMessageRequest::loadFile( 
      buff, m_messageEndTextFmtFile[ m_contentType ] );
   if ( length > 0 ) {
      buff[ length ] = '\0';
      messageEndTextFmt = (char*)buff;
   }

   length = RouteMessageRequest::loadFile( 
      buff, m_messageResourcesFile[ m_contentType ] );
   if ( length > 0 ) {
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
               uint32 fileLength = RouteMessageRequest::loadFile( 
                  buffer, file, true );
               if ( fileLength > 0 ) {
                  messageResourcesBuff[ lineNbr ] = buffer;
                  messageResourcesBuffLength[ lineNbr ]  = fileLength;
               } else {
                  mc2log << info 
                         << "LocalMapMessageRequest::getDefaultMessage " 
                         << "resource " << file << " failed to load."
                         << endl;
                  ok = false;
               }
            } else {
               mc2log << error
                      << "LocalMapMessageRequest::getDefaultMessage " 
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
   }   
   delete buff;
   
}


LocalMapMessageRequest::state
LocalMapMessageRequest::makeInitialSetup( 
   int32 northLat, int32 westLon,
   int32 southLat, int32 eastLon,
   StringTable::languageCode lang,
   uint16 localMapWidth,
   uint16 localMapHeight,
   const char* localMapString,
   GfxFeatureMap* markItems,
   bool makeImages,
   bool makeLink,
   bool realMapURLs,
   RouteMessageRequestType::MessageContent 
   contentType,
   uint32 maxMessageSize,
   const char* signature,
   ImageDrawConfig::imageFormat defaultFormat,
   MapSettingsTypes::defaultMapSetting mapSetting,
   struct MapSettingsTypes::ImageSettings* 
   imageSettings,
   const char* messageFmt,
   const char* messageStartTextFmt,
   const char* messageEndTextFmt,
   uint32 nbrMessageResources,
   char** messageResourcesURI,
   uint32* messageResourcesBuffLength,
   byte** messageResourcesBuff,
   const TopRegionRequest* topReq)
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

   //*** Make message 
   MC2String message( messageFmt );
   MC2String startTextStr( messageStartTextFmt ? 
                        messageStartTextFmt : "" );
   MC2String endTextStr( messageEndTextFmt ? messageEndTextFmt : "" );
   const char* startTextFileName = "start";
   const char* endTextFileName = "end";
   const MC2String eolStr( "\r\n" );
   const MC2String shortEolStr( "\n" );
   MC2String messageEOLStr( "<br>\r\n" );
   const char* pageName = "lmap.html";
   MimePartText::contentType textFileType = 
      MimePartText::CONTENT_TYPE_TEXT_HTML;
   const char* textFileNameExtension = ".html";
   MC2String textFileName;
   MimePart::characterSet internalCharacterSet = 
#ifdef MC2_UTF8
      MimePart::CHARSET_UTF_8;
#else
      MimePart::CHARSET_ISO_8859_1;
#endif
   MimePart::characterSet defaultCharacterSet = internalCharacterSet;
   MimePart::mainContentType messageMainContentType = 
      MimePart::MAIN_CONTENT_TYPE_TEXT;
   MimePartText::contentType messageTextMarkupType = 
      MimePartText::CONTENT_TYPE_TEXT_HTML;
   MimePartApplication::contentType messageApplicationMarkupType =
      MimePartApplication::CONTENT_TYPE_APPLICATION_OCTETSTREAM;
   uint32 nbrReplaces = 0;
   vector<MimePartText*> textPartVector;
   bool wmlContent = m_contentType == RouteMessageRequestType::WML_CONTENT;
   uint32 mapImageNbr = 0;
   char mapImageLocalFmt[ 512 ];
   char tmp[ 4096 ];
   
   if ( m_contentType == RouteMessageRequestType::HTML_CONTENT ) {
      pageName = "lmap.html";
      messageEOLStr = "<br>\r\n";
      textFileNameExtension = ".html";
      textFileType = MimePartText::CONTENT_TYPE_TEXT_HTML;
      messageTextMarkupType = MimePartText::CONTENT_TYPE_TEXT_HTML;
      m_message.setMainContentType( 
         MimePartText::getContentTypeAsString(
            MimePartText::CONTENT_TYPE_TEXT_HTML ) );
   } else if ( m_contentType == RouteMessageRequestType::WML_CONTENT ) {
      pageName = "lmap.wml";
      messageEOLStr = "<br/>\r\n";
      textFileNameExtension = ".wml";
      textFileType = MimePartText::CONTENT_TYPE_TEXT_WML;
      messageTextMarkupType = MimePartText::CONTENT_TYPE_TEXT_WML;
      m_message.setMainContentType( 
         MimePartText::getContentTypeAsString(
            MimePartText::CONTENT_TYPE_TEXT_WML ) );
      m_format = defaultFormat = ImageDrawConfig::WBMP;
      mapSetting = MapSettingsTypes::MAP_SETTING_WAP;
   } else if ( m_contentType == RouteMessageRequestType::SMIL_CONTENT ) {
      pageName = "lmap.smil";
      messageEOLStr = "\r\n";
      textFileNameExtension = ".txt";
      textFileType = MimePartText::CONTENT_TYPE_TEXT_PLAIN;
      messageMainContentType = MimePart::MAIN_CONTENT_TYPE_APPLICATION;
      messageApplicationMarkupType = 
         MimePartApplication::CONTENT_TYPE_APPLICATION_SMIL;
      m_message.setMainContentType( 
         MimePartApplication::getContentTypeAsString(
            MimePartApplication::CONTENT_TYPE_APPLICATION_SMIL ) );
      localMapWidth = MIN( 160, localMapWidth );
      localMapHeight = MIN( 120, localMapHeight );
//      m_mapSetting = MapSettingsTypes::MAP_SETTING_WAP;
      defaultCharacterSet = MimePart::CHARSET_UTF_8;
   }

   m_mapSettings = MapSettings::createDefaultMapSettings( mapSetting );
   // Set what to show on maps
   m_mapSettings->setMapContent( true,    // Map
                                 true,    // Topography
                                 true,    // POI
                                 false ); // Route
   m_mapSettings->setShowTraffic( true );

   if ( imageSettings != NULL ) {
      m_mapSettings->mergeImageSettings( *imageSettings );
   }
   // Draw scale on images
   m_mapSettings->setDrawScale( true );

   // Image type
   char* ext = StringUtility::newStrDup(
      StringUtility::copyLower( MC2String(ImageDrawConfig::imageFormatMagick[ m_format ] ) ).c_str());
   sprintf( mapImageLocalFmt, "Map%%d.%s", ext );

   // Client type for the URLs
   const char* clientType = NULL;
   if ( m_clientSetting != NULL ) {
      clientType = m_clientSetting->getClientType();
   }

   //** Make replace strings
   const char* localMap = StringTable::getString( 
      StringTable::YOUR_LOCAL_MAP, lang );
   char localMapURI[4096];
   char localMapWidthValue[20];
   char localMapHeightValue[20];
   char localMapAlt[512];
   
   // Local map
   // The propotionall size of the image
   GfxUtility::getDisplaySizeFromBoundingbox( southLat, westLon,
                                              northLat, eastLon,
                                              localMapWidth, 
                                              localMapHeight );
   if ( m_realMapURLs ) {
      HttpUtility::makeMapURL( localMapURI, localMapWidthValue,
                               localMapHeightValue, NULL,
                               MAX_UINT32, MAX_UINT32, MAX_UINT32,
                               MAX_UINT32, MAX_UINT32, localMapWidth,
                               localMapHeight, southLat, westLon,
                               northLat, eastLon, defaultFormat, 
                               mapSetting, 
                               m_mapSettings->getShowMap(),
                               m_mapSettings->getShowTopographMap(),
                               m_mapSettings->getShowPOI(),
                               m_mapSettings->getShowRoute(), 
                               m_mapSettings->getDrawScale(),
                               m_mapSettings->getShowTraffic(),
                               imageSettings,
                               markItems,
                               clientType );
   } else {
      sprintf( localMapWidthValue, "%d", localMapWidth );
      sprintf( localMapHeightValue, "%d", localMapHeight );
      sprintf( localMapURI, mapImageLocalFmt, mapImageNbr++ );
   }
   strcpy( localMapAlt, StringTable::getString( 
      StringTable::YOUR_LOCAL_MAP, lang ) );


   //** Replace in message
   MC2String* messageStr = &message;
   
   
   

   RouteMessageRequest::replaceString( *messageStr, localMapStr, localMap,
                                       wmlContent );
   RouteMessageRequest::replaceString( *messageStr, localMapNameStr, 
                                       localMapString, wmlContent );
   RouteMessageRequest::replaceString( *messageStr, localMapURIStr, 
                                       localMapURI, wmlContent );
   RouteMessageRequest::replaceString( *messageStr, localMapWidthStr, 
                                       localMapWidthValue, wmlContent );
   RouteMessageRequest::replaceString( *messageStr, localMapHeightStr, 
                                       localMapHeightValue, wmlContent );
   RouteMessageRequest::replaceString( *messageStr, localMapAltStr, 
                                       localMapAlt, wmlContent );
   MimePartText fake( NULL, 0, messageTextMarkupType,
                      defaultCharacterSet, "", false );
   MC2String ctype( fake.getContentType() );
   RouteMessageRequest::replaceString( ctype, "\"", "", wmlContent );
   RouteMessageRequest::replaceString( *messageStr, "%CONTENTTYPE%", 
                                       ctype.c_str(), wmlContent );
   
   textFileName = startTextFileName;
   textFileName.append( textFileNameExtension );
   nbrReplaces =  RouteMessageRequest::replaceString( 
      *messageStr, startFileStr, textFileName.c_str(), wmlContent );
   bool useStartTextFile = (nbrReplaces > 0 && 
                            messageStartTextFmt != NULL);
   if ( useStartTextFile ) {
      // A special text file for the text
      messageStr = &startTextStr;
   }
   
   RouteMessageRequest::replaceString( *messageStr, localMapStr, localMap,
                                       wmlContent );
   RouteMessageRequest::replaceString( *messageStr, localMapNameStr, 
                                       localMapString, wmlContent );

   if ( useStartTextFile ) {
      if ( defaultCharacterSet != internalCharacterSet ) {
         RouteMessageRequest::changeCharacterSet( 
            startTextStr, defaultCharacterSet );
      }
      textPartVector.push_back( new MimePartText( 
         (byte*)startTextStr.c_str(), startTextStr.size(), 
         textFileType, defaultCharacterSet,
         textFileName.c_str(), true ) );
   }

   messageStr = &message;

   MC2String sig( signature );
   if ( m_contentType == RouteMessageRequestType::WML_CONTENT ) {
      StringUtility::wapStr( tmp, signature );
      sig = tmp;
   }
   RouteMessageRequest::replaceString( 
      sig, eolStr.c_str(), messageEOLStr.c_str() );
   RouteMessageRequest::replaceString( 
      sig, shortEolStr.c_str(), messageEOLStr.c_str() );
   char webLink[ 4096 ];
   webLink[ 0 ] = '\0';
   const char* linkToRoute = StringTable::getString( 
      StringTable::PROBLEM_WITH_MESSAGE_CLICK_HERE_STR, 
      lang );
   if ( m_makeLink && m_contentType == 
        RouteMessageRequestType::HTML_CONTENT && 
        Properties::getProperty( "DEFAULT_WEB_HOST" ) != NULL )
   {
      // Link to page with html version of this message
      const char* host = Properties::getProperty( "DEFAULT_WEB_HOST" ); 
      HttpUtility::makeLocalMapLink( 
         webLink, host, "http",
         northLat, westLon, southLat, eastLon,
         markItems, lang,
         localMapString, signature, "lmap.html",
         clientType
         );
      sig.append( messageEOLStr.c_str() );
      char href[ 4096 ]; 
      sprintf( href, "<a href=\"%s\">%s</a>", webLink, linkToRoute );
      sig.append( href );
   }


   textFileName = endTextFileName;
   textFileName.append( textFileNameExtension );
   nbrReplaces = RouteMessageRequest::replaceString( 
      *messageStr, endFileStr, textFileName.c_str(), wmlContent );
   bool useEndTextFile = nbrReplaces > 0 && messageEndTextFmt != NULL;
   if ( useEndTextFile ) {
      // A special text file for the text
      messageStr = &endTextStr;
   }
   
   RouteMessageRequest::replaceString( *messageStr, signatureStr, 
                                       sig.c_str() );

   if ( useEndTextFile ) {
      if ( defaultCharacterSet != internalCharacterSet ) {
         RouteMessageRequest::changeCharacterSet( 
            endTextStr, defaultCharacterSet );
      }
      textPartVector.push_back( new MimePartText( 
         (byte*)endTextStr.c_str(), endTextStr.size(), 
         textFileType, defaultCharacterSet,
         textFileName.c_str(), true ) );
   }


   //** Make image requests
   if ( m_makeImages ) {
      m_nbrImageRequests = 1;
      m_imageRequests = new GfxFeatureMapImageRequest*[ 
         m_nbrImageRequests ];
      m_imageURIs = new char*[ m_nbrImageRequests  ];

      // Add localMap image request
      MC2BoundingBox bbox( northLat, westLon, southLat, eastLon );
      MC2BoundingBox ccBBox;
      MapSettings *mapSettings = new MapSettings( *m_mapSettings );
      if ( m_clientSetting != NULL ) {
         mapSettings->setImageSet( m_clientSetting->getImageSet() );
      }
      m_imageRequests[ m_nbrImageRequests - 1 ] = 
         new GfxFeatureMapImageRequest( 
            this, &bbox, localMapWidth, localMapHeight, 
            ItemTypes::getLanguageCodeAsLanguageType( lang ),
            NULL, true, false, true, defaultFormat, 
            (localMapWidth*localMapHeight / 8), // size
            mapSettings, topReq, ccBBox );
      m_imageRequests[ m_nbrImageRequests - 1 ]->setParentRequest( this );
      m_imageRequests[ m_nbrImageRequests - 1 ]->ownMapSettings();
      m_imageRequests[ m_nbrImageRequests - 1 ]->setCopyright( m_copyright );
      m_imageRequests[ m_nbrImageRequests - 1 ]->setUser( m_user );
      m_imageURIs[ m_nbrImageRequests - 1 ] = 
         StringUtility::newStrDup( localMapURI );



      // Add markItems
      if ( markItems != NULL ) {
         m_imageRequests[ m_nbrImageRequests - 1 ]->addSymbolMap( 
            markItems );
      }
   } else {
      m_nbrImageRequests = 0;
      m_imageRequests = NULL;
   }

   // * Make right charset of message
   if ( defaultCharacterSet != internalCharacterSet ) {
      RouteMessageRequest::changeCharacterSet( 
         message, defaultCharacterSet );
   }


   if ( !m_realMapURLs && !m_contentLocation ) {
      // Turns links into Content-ID references
      RouteMessageRequest::replaceString( 
         message, "img src=\"", "img src=\"cid:" );
      RouteMessageRequest::replaceString( 
         message, "stylesheet\" href=\"", "stylesheet\" href=\"cid:" );
   }

   //*** Add local map page to message
   m_message.add( RouteMessageRequest::makeMimePart( 
      message, messageMainContentType, messageTextMarkupType, 
      messageApplicationMarkupType, defaultCharacterSet,
      pageName ) );

   //*** Add text part files, if any
   for ( uint32 i = 0 ; i < textPartVector.size() ; i++ ) {
      m_message.add( textPartVector[ i ] );
   }

   //*** Add resources for local map page
   for ( uint32 i = 0 ; i < nbrMessageResources ; i++ ) {
      m_message.add( MimeMessage::createMimePartForFile(  
         messageResourcesURI[ i ],
         messageResourcesBuff[ i ], messageResourcesBuffLength[ i ],
         messageResourcesURI[ i ], true ) );
   }

   delete [] ext;
   
   return newState;
}


LocalMapMessageRequest::state 
LocalMapMessageRequest::makeEmailRequest() {
   state newState = EMAIL_REQUEST;

   if ( m_sendEmail ) {
      SendEmailRequestPacket* p = new SendEmailRequestPacket( getID() );
      char* body = m_message.getMimeMessageBody();
      const char* optionalHeaderTypes[ 2 ] = 
        { MimeMessage::mimeVersionHeader, MimeMessage::contentTypeHeader };
      const char* optionalHeaderValues[ 2 ] = 
        { m_message.getMimeVersion(), m_message.getContentType() };
   
      mc2dbg1 << "LocalMapMessageRequest Sending mail to " 
             << m_toMailAddr << endl;
      if ( p->setData( m_toMailAddr, m_fromMailAddr, m_subject, body,
                       2, optionalHeaderTypes, optionalHeaderValues ) )
      {
         m_emailRequest = new PacketContainer( p, 0, 0 , 
                                               MODULE_TYPE_SMTP );
   
         newState = EMAIL_REQUEST;
      } else {
         delete p;
         mc2log << error << "LocalMapMessageRequest::makeEmailRequest "
            "SendEmailRequestPacket::setData failed." << endl;

         m_emailRequest = NULL;
         newState = ERROR;
         setDone( true );
      }
   
      delete [] body;
   } else {
      m_emailRequest = NULL;
      newState = DONE;
      setDone( true );
   }
   
   return newState;
}


const char* 
LocalMapMessageRequest::m_messageFmtFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "local_map.html",
      "local_map.wml",
      "local_map.smil",
   };


const char* const 
LocalMapMessageRequest::m_messageStartTextFmtFile[ 
   RouteMessageRequestType::NBR_CONTENT ] = {
      "",
      "",
      "local_map_smil_text.txt"
   };


const char* const 
LocalMapMessageRequest::m_messageEndTextFmtFile[ 
   RouteMessageRequestType::NBR_CONTENT ] = {
      "",
      "",
      "local_map_smil_end.txt"
   };


const char* 
LocalMapMessageRequest::m_messageResourcesFile[
   RouteMessageRequestType::NBR_CONTENT ] = {
      "local_map_resources.txt",
      "local_map_wml_resources.txt",
      "local_map_smil_resources.txt",
   };


const char* 
LocalMapMessageRequest::localMapStr = "%LOCAL_MAP%";


const char* 
LocalMapMessageRequest::localMapURIStr = "%LOCAL_MAP_URI%";


const char* 
LocalMapMessageRequest::localMapWidthStr = "%LOCAL_MAP_WIDTH%";


const char* 
LocalMapMessageRequest::localMapHeightStr = "%LOCAL_MAP_HEIGHT%";


const char* 
LocalMapMessageRequest::localMapAltStr = "%LOCAL_MAP_ALT%";


const char* 
LocalMapMessageRequest::localMapNameStr = "%LOCAL_MAP_NAME%";


const char* 
LocalMapMessageRequest::signatureStr = "%SIGNATURE%";


const char* 
LocalMapMessageRequest::startFileStr = "%START_FILE%";


const char* 
LocalMapMessageRequest::endFileStr = "%END_FILE%";

