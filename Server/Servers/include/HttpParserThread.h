/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPPARSERTHREAD_H
#define HTTPPARSERTHREAD_H

#include "config.h"
#include "InterfaceParserThread.h"
#include "HttpParserThreadConfig.h"
#include "DrawingProjection.h"
#include "TimeUtility.h"
#include "NotCopyable.h"

class HttpFunctionHandler;
class HttpParserThreadGroup;
class HttpHeader;
class HttpBody;
class HttpInterfaceRequest;


/**
 * Parses HTTP-requests and calls handleHttpRequest.
 *
 */
class HttpParserThread : public InterfaceParserThread, private NotCopyable {
   public:   
      /**
       * Creates a new HttpParserThread.
       * @param group The HttpParserThreadGroup that this HttpParserThread
       *              is part of.
       */
      HttpParserThread( HttpParserThreadGroup* group );


      /**
       * Destructor deletes all local data.
       * Don't call this use terminate.
       */
      virtual ~HttpParserThread();


      /**
       * This function is called when a InterfaceRequest has been received.
       *
       * @param ireq The InterfaceRequest to process.
       */
      virtual void handleInterfaceRequest( InterfaceRequest* ireq );


      /**
       * Adds a handled request to the current handleHttpRequest.
       */
      void addRequestName( const char* name );


      /**
       * Gets the time when the current request start time.
       */
      uint32 getCurrRequestStartTime() const;
      

      /**
       * Set the reply to a HTTP response.
       */
      void setStatusReply( uint32 statusCode );


      /**
       * Set the username to use in logprefix, updates logprefix.
       *
       * @param userName The new username, use NULL to unset username.
       */
      void setLogUserName( const char* userName );


      /**
       * Checks method and other Http requirements.
       *
       * @param ireq The HttpInterfaceRequest with in header.
       * @return True if request is ok, false if not and reply status is
       *         set.
       */
      static bool checkHttpHeader( HttpInterfaceRequest* ireq );


      /**
       * Sets Keep-Alive in outHead if requested by client in inHead.
       *
       * @param ireq The HttpInterfaceRequest with in header.
       * @param outHead The HttpHeader to set Keep-Alive in.
       */
      static void setKeepAlive( HttpInterfaceRequest* ireq, 
                                HttpHeader* outHead );


      /**
       * Sets date in outHead to the time in now.
       *
       * @param outHead The HttpHeader to set Date in.
       * @param now The time to use, default the current time.
       */
      static void setHttpHeaderDate( HttpHeader* outHead, 
                                     uint32 now = TimeUtility::getRealTime() );


      /**
       * Sets date in outHead to the time in now.
       *
       * @param dateStr The string to print date into.
       * @param now The time to use, default the current time.
       */
      static void makeDateStr( char* dateStr, 
                               uint32 date = TimeUtility::getRealTime() );

      static uint32 dateStrToInt( const char* dateStr );
      
      /**
       * Sets the default 200 OK response line in outHead.
       *
       * @param The HttpInterfaceRequest with in header.
       * @param outHead The HttpHeader to set response line in.
       */
      static void setDefaultResponseLine( HttpInterfaceRequest* ireq, 
                                          HttpHeader* outHead );



      /**
       * This function is called when a new HTTP request has been received.
       * The possible parameters in inBody is not added to paramsMap,
       * make a parseParameters( inBody->getBody(), paramsMap, inHead );
       * call if there are parameters in the body to add.
       * The Content-Type in the outHead is set to the mime-type of the
       * fileexternsion of the pagename.
       * This can be called recursively by HttpFunction handlers.
       *
       * @param inHead The HTTP header of the request.
       * @param inBody The body content, may be empty, of the HTTP request.
       * @param paramsMap The startline's parameters.
       * @param outHead The HTTP header of the reply.
       * @param outBody The content of the reply.
       * @param now The time of the request.
       * @param dateStr The time of the request as a string.
       * @return True if the request was handled and outBody was filled 
       *         with the reply.
       */
      virtual bool handleHttpRequest( HttpHeader* inHead, 
                                      HttpBody* inBody,
                                      stringMap* paramsMap,
                                      HttpHeader* outHead, 
                                      HttpBody* outBody,
                                      uint32 now,
                                      const char* dateStr );

      /**
       * Copies in data to out data, avoiding to copy connection
       * specific headers like keep-alive.
       *
       * @param inHead    The incoming header.
       * @param inBody    The incoming body.
       * @param outHead   The outgoing header.
       * @param outBody   The outgoing body.
       * @return True.
       */
      static bool copyEchoRequestToReply( HttpHeader* inHead, 
                                          HttpBody* inBody,
                                          HttpHeader* outHead, 
                                          HttpBody* outBody );

protected:      
      /**
       *    Get a string that describes the type of server. 
       *    E.g. this method
       *    coule return "XML" (the XMLParserThread) or "MS" 
       *    (MapCentralParserThread).
       *    @return  A string that describes the server that does the 
       *             debiting.
       */
      virtual const char* getServerType();


     /**
      * Checks for special files, e.g. TileMaps.
      *
      * @param inHead    The incoming header.
      * @param inBody    The incoming body.
      * @param paramsMap Map of parameters.
      * @param outHead   The outgoing header.
      * @param outBody   The outgoing body.
      * @param now       Current time.
      * @param res       The result bool, false if error is set.
      * @return True if the special file was handled.
      */
     bool handleSpecialFiles( HttpHeader* inHead, 
                              HttpBody* inBody,
                              stringMap* paramsMap,
                              HttpHeader* outHead, 
                              HttpBody* outBody,
                              uint32 now,
                              bool& res );

     /**
      * Writes info about the authorized user's last client to the log.
      */
      void logLastClient();

      /**
       * The logname if any, set by handleHttpRequest.
       */
      MC2String m_logname;


      /**
       * The current HttpInterfaceRequest.
       */
      HttpInterfaceRequest* m_irequest;

      /// @return current HttpInterfaceRequest.
      const HttpInterfaceRequest* getHttpInterfaceRequest() const {
         return m_irequest;
      }


      /**
       * The name of the request processed, set by handleHttpRequest.
       */
      MC2String m_requestName;


   private:
      /**
       * Parses a string starting with ?paramname=paramvalue&paramname=...
       * @param params The parameterstring.
       * @param paramsMap Parameters are inserted into this.
       * @param inHead The header with possible content-type charset.
       */
      void parseParameters( MC2String& params, stringMap* paramsMap,
                            HttpHeader* inHead );


      /**
       * Parse a page with <ISAB_ ...> tags.
       * Skips <!--ISAB TAGS...-->, where ... is any string.
       *
       * @param page is the page with tags.
       * @param inHead The HTTP header of the request.
       * @param inBody The body content, may be empty, of the HTTP request.
       * @param paramsMap The requests parameters.
       * @param outHead The HTTP header of the reply.
       * @param outBody The content of the reply.
       * @return True if parsing and function calls where ok, false if
       *         not.
       */ 
      bool parsePage(const MC2String& page,
                     HttpHeader* inHead, 
                     HttpBody* inBody,
                     stringMap* paramsMap,
                     HttpHeader* outHead, 
                     HttpBody* outBody);


      /**
       * Deletes and removes all the strings in v.
       * @param v the stringVector to remove all string from
       */
      void clearStringVector(stringVector* v);


      /**
       * Deletes and removes all the string in params.
       * @param params The stringMap to clear.
       */
      void clearStringMap(stringMap* params);


      /**
       * Checks the Accept-Charset in inHead and tries to use one of the
       * charsets in it. Sets the used Charset in outHead's Content-Type.
       * Currently ISO-8859-1 and UTF-8 is supported.
       *
       * If charSetMayBeChanged returns false for the outBody the
       * character set will not be changed and getCharSet will be 
       * added to the outgoing header.
       * 
       *
       * @param outBody The complete body.
       * @param body The body in ISO-8859-1 might be changed to another
       *             charset.
       * @param inHead The request's header.
       * @param outHead The reply's header.
       * @param bodyLength Set to the actual length of the body,
       *        not changed if body not changed.
       */
      static void setBodyCharset( HttpBody& outBody,
                                  MC2String& body,
                                  HttpHeader* inHead,
                                  HttpHeader* outHead,
                                  uint32& bodyLength );


      /**
       * Checks the Accept-Encoding in inHead and applies supported 
       * encodings. 
       * Currently gzip (alias x-gzip) and deflate is supported.
       *
       * @param body The body might be encoded.
       * @param inHead The request's header.
       * @param outHead The reply's header.
       * @param outBody The reply body, binary flag is set if body is
       *                compressed.
       * @param bodyLength Set to the actual length of the body,
       *        not changed if body not changed.
       */
      static void encodeBody( MC2String& body, 
                              HttpHeader* inHead,
                              HttpHeader* outHead,
                              HttpBody* outBody,
                              uint32& bodyLength );


      /**
       * Sets the cache variables in the outheader to the appropriate 
       * values.
       *
       * @param outHead The header of the reply.
       * @param command The HTTP command in the HTTP request.
       * @param generatedPage If the page contained functions.
       * @param cacheElement If the reply is from a cacheElement.
       * @param date The date of the HTTP reply.
       * @param dateStr The date of the HTTP reply formated in a string.
       * @param fStat The file status struct.
       */
      void setCacheHeaders( HttpHeader* outHead, 
                            const MC2String& command,
                            bool generatedPage, bool cacheElement,
                            uint32 date,
                            const char* dateStr,
                            struct stat* fStat );

      
      /** 
       * Reads a line from the m_reqSock into target.
       * @return how many bytes accually read.
       * @param target the string to append read line to.
       */
      int readLine(MC2String* target);


      /**
       * Reads nbrBytes bytes from the m_reqSock into target.
       * @param target the string to append the bytes to.
       * @param nbrBytes the number of bytes to read.
       * @return the number of bytes accualy read.
       */
      int readBytes(MC2String* target, uint32 nbrBytes);


      /**
       * Prints the stringmap into out stream.
       * @param out The ostream to print into.
       * @param strMap The stringMap to print.
       */
      void printStringMap( ostream& out, stringMap& strMap,
                           const char* preLineString = "   " ) const;


     /**
      *   Handles TileMapRequest.
      *   @param inHead    The incoming header.
      *   @param inBody    The incoming body.
      *   @param paramsMap Map of parameters.
      *   @param outHead   The outgoing header.
      *   @param outBody   The outgoing body.
      *   @param now       Current time.
      *   @return True if the special file was handled.
      */
     bool handleTileMapRequest( HttpHeader* inHead, 
                                HttpBody* inBody,
                                stringMap* paramsMap,
                                HttpHeader* outHead, 
                                HttpBody* outBody,
                                uint32 now );


     /**
      * Handles TileMapsRequest.
      *
      * @param inHead    The incoming header.
      * @param inBody    The incoming body.
      * @param paramsMap Map of parameters.
      * @param outHead   The outgoing header.
      * @param outBody   The outgoing body.
      * @param now       Current time.
      * @return True if the special file was handled.
      */
     bool handleTileMapsRequest( HttpHeader* inHead, 
                                 HttpBody* inBody,
                                 stringMap* paramsMap,
                                 HttpHeader* outHead, 
                                 HttpBody* outBody,
                                 uint32 now );

      /**
       * Echos the request data in reply.
       *
       * @param inHead    The incoming header.
       * @param inBody    The incoming body.
       * @param paramsMap Map of parameters.
       * @param outHead   The outgoing header.
       * @param outBody   The outgoing body.
       * @param now       Current time.
       * @return True if the special file was handled.
       */
      bool handleEchoRequest( HttpHeader* inHead, 
                              HttpBody* inBody,
                              stringMap* paramsMap,
                              HttpHeader* outHead, 
                              HttpBody* outBody,
                              uint32 now );

     /**
      * Handles a XSMap request.
      *
      * @param inHead    The incoming header.
      * @param inBody    The incoming body.
      * @param paramsMap Map of parameters.
      * @param outHead   The outgoing header.
      * @param outBody   The outgoing body.
      * @param now       Current time.
      * @return True if the special file was handled.
      */
      bool handleXSMapRequest( HttpHeader* inHead, 
                               HttpBody* inBody,
                               stringMap* paramsMap,
                               HttpHeader* outHead, 
                               HttpBody* outBody,
                               uint32 now );


     bool handleProjectionSettingsRequest( HttpHeader* inHead,
                                           HttpBody* inBody,
                                           stringMap* paramsMap,
                                           HttpHeader* outHead,
                                           HttpBody* outBody,
                                           uint32 now );
     
      /**
       * The HTTP functions.
       */
      HttpFunctionHandler* m_functions;
};


#endif // HTTPPARSERTHREAD_H
