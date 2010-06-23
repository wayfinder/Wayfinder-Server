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

#include "HttpSimplePoiFunctions.h"
#include "HttpParserThread.h"
#include "HttpBody.h"
#include "HttpHeader.h"
#include "ParserMapHandler.h"
#include "LangTypes.h"
#include "HttpHeaderLines.h"
#include "UserSwitch.h"
#include "HttpMapFunctions.h"
#include "ClientSettings.h"
#include "ProjectionSettings.h"
#include "HttpCodes.h"

#include <stdlib.h>
#include <memory>

using namespace HttpHeaderLines;

/// Translate code from ParserMapHandler to HTTP
static inline int translCode( ParserMapHandler::errorCode inErr )
{
   switch ( inErr ) {
      case ParserMapHandler::OK:
         return HttpCode::OK;
      case ParserMapHandler::ERROR:
         return HttpCode::NOT_FOUND;
      case ParserMapHandler::TIMEOUT:
         return HttpCode::SERVICE_UNAVAILABLE;
      case ParserMapHandler::BAD_PARAMS:
         // TODO: This can't be right, BAD_REQUEST might be better.
         // Need to look into this a bit further.
         return HttpCode::GONE;
   }

   // Should be unreachable, but old compiler doesn't know that
   return HttpCode::NOT_FOUND;
}

bool
HttpSimplePoiFunctions::htmlMakeSimplePoiDesc( stringVector* params,
                                               int paramc, 
                                               stringMap* paramsMap,
                                               HttpHeader* inHead, 
                                               HttpHeader* outHead,
                                               HttpBody* inBody,
                                               HttpBody* outBody,
                                               HttpParserThread* myThread,
                                               HttpVariableContainer* myVar )
{
   myThread->addRequestName( "SIMPLE_POI_DESC" );
   // Get the needed parameters
   LangTypes::language_t lang = LangTypes::invalidLanguage;
   try {
      lang = paramsMap->getISO639Thrw( "lang" );
   } catch ( MC2String& str ) {
      mc2dbg << "[htmlMakeSimplePoiDesc]: " << str << endl;
      return false;
   }

   // Parameters OK
   mc2dbg << "[HttpSimplePoiFunctions::htmlMakeSimplePoiDesc]" << endl;
   ParserMapHandler& mapHandler = myThread->getMapHandler();

   const ClientSetting* clientSetting = 
      HttpMapFunctions::getClientSettingFromParams( paramsMap, myThread );
   ImageTable::ImageSet imageSet = ImageTable::DEFAULT;
   if ( clientSetting != NULL ) {
      imageSet = clientSetting->getImageSet();
   }

   SimplePoiDescParams mparams;
   ParserMapHandler::bufPair_t bufPair =
      mapHandler.getSimplePoiDesc( mparams, imageSet );
   auto_ptr<SharedBuffer> buf ( bufPair.second );
   
   if ( bufPair.first == ParserMapHandler::OK && buf.get() ) {
      outHead->addHeaderLine( CONTENT_TYPE,
                              "application/wf-poi-desc" );
      outBody->setBody( *buf );
      return true;
   }
   myThread->setStatusReply( translCode( bufPair.first ) );
   return false;
}



static inline bool parseSquareMapParams( SquareMapParams& outParams,
                                         const stringMap& paramsMap,
                                         const stringVector& params )
{
   // FIXME: Set the params in the SquareMapParams
   try {
      int x = paramsMap.getIntParamThrw( "x" );
      int y = paramsMap.getIntParamThrw( "y" );
      int zoom = paramsMap.getIntParamThrw( "zoom" );
      LangType lang = paramsMap.getISO639Thrw( "lang" );      
      // Set the outParams
      outParams = SquareMapParams( x , y, zoom, lang,
                                   ProjectionSettings::
                                   getPixelSize( params ) );
   } catch ( const MC2String& error ) {
      return false;
   }
   return true;
}

bool
HttpSimplePoiFunctions::htmlMakeSimplePoiMap( stringVector* params,
                                              int paramc, 
                                              stringMap* paramsMap,
                                              HttpHeader* inHead, 
                                              HttpHeader* outHead,
                                              HttpBody* inBody,
                                              HttpBody* outBody,
                                              HttpParserThread* myThread,
                                              HttpVariableContainer* myVar )
{
   myThread->addRequestName( "SIMPLE_POI_MAP" );
   mc2dbg << "[HttpSimplePoiFunctions::htmlMakeSimplePoiMap]" << endl;
   ParserMapHandler& mapHandler = myThread->getMapHandler();
      
   SquareMapParams mparams;
   if ( ! parseSquareMapParams( mparams, *paramsMap, *params ) ) {
      // False seems to mean 404 if nothing is set.
      return false;
   }
   const ClientSetting* clientSetting = NULL;
   UserItem* userItem = NULL;
   auto_ptr<UserSwitch> userSwitch;
   if ( HttpMapFunctions::
        setUserFromParams( paramsMap, myThread, myVar, 
                           userItem, clientSetting ) ) {
      userSwitch.reset( new UserSwitch( *myThread,
                                        userItem, 
                                        clientSetting ) );
   }

   // Get the map
   ParserMapHandler::bufPair_t bufPair =
      mapHandler.getSimplePoiMap( mparams );   
   auto_ptr<SharedBuffer> buf( bufPair.second );

   // FIXME: Use the return code
   if ( bufPair.first == ParserMapHandler::OK && buf.get() ) {
      outHead->addHeaderLine( CONTENT_TYPE,
                              "application/wf-poi-map" );
      outBody->setBody( *buf );
      return true;
   }
   myThread->setStatusReply( translCode( bufPair.first ) );
   return false;
}
