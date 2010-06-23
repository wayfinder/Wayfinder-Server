/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPMAPFUNCTIONS_H
#define HTTPMAPFUNCTIONS_H

#include "config.h"
#include "HttpFunctionsConfig.h"
#include "ImageDrawConfig.h"
#include "MapSettings.h"
#include "DrawingProjection.h"

// Forward declarations of classes
class ExpandItemID;
class ExpandRouteReplyPacket;
class HttpBody;
class HttpHeader;
class LangType;
class RouteReplyPacket;
class RouteRequest;
class UserItem;
class ClientSetting;


/**
 * Functions for MapImage generation in HttpParserThread.
 *
 */
class HttpMapFunctions {
   public:
      /**
       *    @name Map dynamic html generation functions.
       *    @memo Map dynamic html generation functions.
       *    @doc  Map dynamic html generation functions.
       */
      //@{

         static void getSpeedSettings( RedLineSettings& redLineSettings,
                                       stringMap* paramsMap,
                                       bool& allData );
   
         /**  
          * Makes an image and sets it as the outBody, don't append 
          * anything to the body before or after this function.
          */
         static bool htmlMakeImage( stringVector* params,
                                    int paramc, 
                                    stringMap* paramsMap,
                                    HttpHeader* inHead, 
                                    HttpHeader* outHead,
                                    HttpBody* inBody,
                                    HttpBody* outBody,
                                    HttpParserThread* myThread,
                                    HttpVariableContainer* myVar );
      //@}

      /**
       *    @name Map support functions.
       *    @memo Map support functions.
       *    @doc  Map support functions.
       */
      //@{

         

         /**
          * Makes a web-page for a local map.
          */
         static bool htmlShowLocalMap( stringVector* params,
                                       int paramc, 
                                       stringMap* paramsMap,
                                       HttpHeader* inHead, 
                                       HttpHeader* outHead,
                                       HttpBody* inBody,
                                       HttpBody* outBody,
                                       HttpParserThread* myThread,
                                       HttpVariableContainer* myVar );


         /**
          * Gets user from params if params are present.
          *
          * @param paramsMap params from http request
          * @param myThread thread that handled the http request.
          * @param myVar
          * @param userItem Set to the user, caller must release it.
          * @param clientSetting Is set to the setting for this client.
          * @return False if failed to get user. True if got user or
          *         no params to get user from.
          */
         static bool setUserFromParams( stringMap* paramsMap,
                                        HttpParserThread* myThread,
                                        HttpVariableContainer* myVar,
                                        UserItem*& userItem,
                                        const ClientSetting*& clientSetting );

         /**
          * Gets the client setting if client type is present in params.
          * @param paramsMap params from http request.
          * @param myThread thread that handled the http request.
          * @return NULL if no client setting, else the client setting for the
          *              user.
          */
         static const ClientSetting* getClientSettingFromParams( 
            stringMap* paramsMap,
            HttpParserThread* myThread );

      //@}
         /**
          *  Reads the projection parameters from the URL.
          */
         static bool readProjectionParameters( stringMap* paramsMap,
                                               int& x,
                                               int& y,
                                               int& zoom,
                                               MC2String& lang);
         
         /**
          *  Processes the projection request.
          */ 
         static bool processProjectionRequest(
                                     DrawingProjection* projection,
                                     const LangType& lang,
                                     HttpHeader* inHead,
                                     HttpBody* inBody,
                                     stringMap* paramsMap,
                                     HttpHeader* outHead,
                                     HttpBody* outBody,
                                     HttpParserThread* myThread,
                                     uint32 now,
                                     stringVector* params,
                                     uint32 zoom );
         /**
          *  Sets the cache headers.
          */
         static void setProjectionCacheHeaders(
                                          HttpHeader* outHead,
                                          HttpParserThread* myThread,
                                          uint32 data,
                                          uint32 modifyTime );
         
         /**
          *  Handles a request for a tile with the Mercator or
          *  Braun projection.
          */ 
         static bool handleProjectionMapRequest(
            DrawingProjection::projection_t projectionType,
            HttpHeader* inHead,
            HttpBody* inBody,
            stringMap* paramsMap,
            HttpHeader* outHead,
            HttpBody* outBody,
            HttpParserThread* myThread,
            uint32 now,
            const uint32 modifyTime,
            const uint32 etagVersion,
            stringVector* params );

         static bool gmap( stringVector* params,
                           int paramc, 
                           stringMap* paramsMap,
                           HttpHeader* inHead, 
                           HttpHeader* outHead,
                           HttpBody* inBody,
                           HttpBody* outBody,
                           HttpParserThread* myThread,
                           HttpVariableContainer* myVar );

          static bool mmap( stringVector* params,
                            int paramc, 
                            stringMap* paramsMap,
                            HttpHeader* inHead, 
                            HttpHeader* outHead,
                            HttpBody* inBody,
                            HttpBody* outBody,
                            HttpParserThread* myThread,
                            HttpVariableContainer* myVar );

          static bool zoomSettings( stringVector* params,
                                    int paramc, 
                                    stringMap* paramsMap,
                                    HttpHeader* inHead, 
                                    HttpHeader* outHead,
                                    HttpBody* inBody,
                                    HttpBody* outBody,
                                    HttpParserThread* myThread,
                                    HttpVariableContainer* myVar );

   private:
      /**
       * Private constructor to avoid usage.
       */
      HttpMapFunctions();
};


#endif // HTTPMAPFUNCTIONS_H
