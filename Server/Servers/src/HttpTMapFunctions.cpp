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

#include "HttpTMapFunctions.h"
#include "HttpParserThread.h"
#include "UserData.h"
#include "HttpBody.h"
#include "HttpHeader.h"
#include "HttpInterfaceRequest.h"
#include "MC2BoundingBox.h"
#include "ParserTileHandler.h"
#include "HttpMapFunctions.h"
#include "HttpCodes.h"

bool readIntValue( stringMap* paramsMap, const char* name, int32& value ) {
   char* tmpStr = NULL;
   bool ok = true;
   if  ( paramsMap->find( name ) != paramsMap->end() ) {
      value = strtol( (*paramsMap )[ name ]->c_str(), &tmpStr, 0 );
      if ( tmpStr == NULL || *tmpStr != '\0' ) {
         mc2log << warn << "HttpTMapFunctions readIntValue not an int "
                << MC2CITE( (*paramsMap )[ name ]->c_str() ) << endl;
         ok = false;
      }
   } else {
      mc2log << warn << "HttpTMapFunctions readIntValue no such parameter "
             << MC2CITE( name ) << endl;
      ok = false;
   }
   
   return ok;
}

bool
HttpTMapFunctions::htmlMakeTMapTile( stringVector* params,
                                     int paramc, 
                                     stringMap* paramsMap,
                                     HttpHeader* inHead, 
                                     HttpHeader* outHead,
                                     HttpBody* inBody,
                                     HttpBody* outBody,
                                     HttpParserThread* myThread,
                                     HttpVariableContainer* myVar )
{
   myThread->addRequestName( "TMAPTILE" );
   bool ok = true;

   int32 lla = 0;
   int32 llo = 0;
   int32 ula = 0;
   int32 ulo = 0;
   uint32 scale = 0;
   LangTypes::language_t lang = LangTypes::english;
   set<int> layers;
   const char* cacheName = "";
   uint32 cacheMaxSize = 1024;
   bool useGzip = false;
   bool allSet = true;
   char* tmpStr = NULL;
   UserItem* userItem = NULL;

   RouteID* routeID = NULL;
   
   // Read lla, llo, ula and ulo
   if ( allSet ) allSet = readIntValue( paramsMap, "lla", lla );
   if ( allSet ) allSet = readIntValue( paramsMap, "llo", llo );
   if ( allSet ) allSet = readIntValue( paramsMap, "ula", ula );
   if ( allSet ) allSet = readIntValue( paramsMap, "ulo", ulo );

   if ( !allSet ) {
      // Check if route id is present instead of boundingbox.
      
      if ( paramsMap->find( "r" ) != paramsMap->end() ) {
         const MC2String* str = paramsMap->find( "r" )->second;
         routeID = new RouteID( str->c_str() );
         allSet = true;
      }
   }
   
   if ( allSet ) {
      int32 tmpScale = 0;
      allSet = readIntValue( paramsMap, "scale", tmpScale );
      scale = tmpScale;
   }

   const ClientSetting* clientSetting = NULL;
   if ( !HttpMapFunctions::
        setUserFromParams( paramsMap, myThread, myVar,
                           userItem, clientSetting ) ) {
      allSet = false;
   }

   if ( userItem != NULL ) {
//       if ( !myThread->getMapIdsForUserRegionAccess( 
//               userItem->getUser(), allowedMaps ) )
//       {
//          mc2log << info << "HttpTMapFunctions::htmlMakeTMapTile "
//                 << "failed to get allowed mapIDs for user "
//                 << userItem->getUser()->getLogonID() << "(" 
//                 << userItem->getUIN() << ")" << endl;
//       }
   }

   // lang
   if ( allSet && paramsMap->find( "lang" ) != paramsMap->end() ) {
      MC2String langStr = *paramsMap->find( "lang" )->second;
      if ( langStr == "country" ) {
         // we want invalid language so map module can decide the best lang
         lang = LangTypes::invalidLanguage;
      } else {
         lang = LangTypes::getStringAsLanguage( langStr.c_str(), false );
         if ( lang == LangTypes::invalidLanguage ) {
            // Try fullName
            lang = LangTypes::getStringAsLanguage( langStr.c_str(), true );
         }
         if ( lang == LangTypes::invalidLanguage ) {
            mc2log << warn << "HttpTMapFunctions::htmlMakeTMapTile bad lang "
                   << MC2CITE( langStr.c_str() )
                << endl;
            allSet = false;
         }
      }
   } else {
      allSet = false;
   }

   if ( allSet && paramsMap->find( "layer" ) != paramsMap->end() ) {
      uint32 pos = 0;
      const MC2String* str = paramsMap->find( "layer" )->second;
      const char* lStr = HttpHeader::getFirstStringIn( str, pos );
      
      while ( lStr != NULL && allSet ) {
         layers.insert( strtol( lStr, &tmpStr, 0 ) );
         if ( tmpStr == NULL || *tmpStr != '\0' ) {
            mc2log << warn << "HttpTMapFunctions layer not an int "
                   << MC2CITE( lStr ) << endl;
            allSet = false;
         }
         lStr = HttpHeader::getNextStringIn( str, pos );
      }
   } else if ( allSet &&  paramsMap->find( "l" ) != paramsMap->end() ) {
      char* delStr = StringUtility::newStrDup( 
         paramsMap->find( "l" )->second->c_str() );
      char* stringp = delStr;
      do {
         char* lp = StringUtility::strsep( &stringp, "," );
         layers.insert( strtol( lp, &tmpStr, 0 ) );
         if ( tmpStr == NULL || *tmpStr != '\0' ) {
            mc2log << warn << "HttpTMapFunctions l not an int "
                   << MC2CITE( lp ) << endl;
            allSet = false;
         }

      } while ( stringp != NULL && allSet );

      delete [] delStr;
   } else {
      allSet = false;
   }

   if ( allSet && paramsMap->find( "cn" ) != paramsMap->end() ) {
      cacheName = paramsMap->find( "cn" )->second->c_str();
   } else {
      allSet = false;
   }
   if ( allSet ) {
      int32 cs = 0;
      allSet = readIntValue( paramsMap, "cs", cs );
      cacheMaxSize = cs;
   }
   if ( allSet ) {
      int32 gz = 0;
      allSet = readIntValue( paramsMap, "gz", gz );
      useGzip = gz != 0;
   }

   bool sfd = false;
   {
      int32 sf = 0;
      bool exists = readIntValue( paramsMap, "sf", sf );
      if ( exists ) {
         sfd = sf;
      } else {
         sfd = false;
      }
   }
   bool sf_debug = false;
   {
      int32 sf = 0;
      bool exists = readIntValue( paramsMap, "sf_debug", sf );
      if ( exists ) {
         sf_debug = sf;
      } else {
         sf_debug = false;
      }
   }
      
   if ( allSet ) {

      vector<byte> outBuf;
      if ( routeID == NULL ) {
         // Use the boundingbox.
         // The boundingbox 
         MC2BoundingBox bbox( ula, llo, lla, ulo );
         myThread->getTileHandler()->precacheClientTileMaps( sfd, sf_debug,
            outBuf, bbox, scale, lang, layers, cacheName, cacheMaxSize, 
            useGzip );
      } else {
         // Use the route id.
         myThread->getTileHandler()->getRouteTileMapCache( 
            outBuf, *routeID, lang, layers, cacheName, cacheMaxSize, 
            useGzip );
      }
      
      if ( outBuf.size() > 0 ) {
         outBody->setBody( &outBuf.front(), outBuf.size() );
      } else {
         myThread->setStatusReply( HttpCode::INTERNAL_ERROR );
         ok = false;
      }

   } else {
      mc2log << warn << "HttpTMapFunctions::htmlMakeTMapTile "
             << "some indata missing." << endl;
      myThread->setStatusReply( HttpCode::BAD_REQUEST );
      ok = false;
   }

   delete routeID;

   myThread->releaseUserItem( userItem );

   return ok;
}

