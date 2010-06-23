/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLEXTSERVICEHELPER_H
#define XMLEXTSERVICEHELPER_H

#include "config.h"

#include "ParserHandler.h"

// This seems to be the easiest way to get around lots of trouble
#include "XMLUtility.h"

#include "XMLServerErrorMsg.h"

class LangType;
class XMLParserThread;
class HttpHeader;


class XMLExtServiceHelper : public ParserHandler {
public:

   /// Creates new helper
   XMLExtServiceHelper( XMLParserThread* thread,
                        ParserThreadGroup* group );

   /**
    *   Parse and reply to an external service list request.
    */
   bool parseExtServicesReq( DOMNode* cur, DOMNode* out,
                             DOMDocument* reply, 
                             bool indent,
                             const HttpHeader& inHeaders );

   /**
    *   Parse and reply to an external search request.
    */
   bool parseExtSearchReq( DOMNode* cur, DOMNode* out,
                           DOMDocument* reply, 
                           bool indent,
                           const HttpHeader& inHeaders );
   
   /**
    *   Appends external search desc to the search result.
    *
    */
   void appendExternalSearchDesc( DOMNode* out,
                                  DOMDocument* reply,
                                  DOMNode* cur,
                                  const MC2String& crcIn,
                                  const LangType& lang,
                                  int indentLevel,
                                  bool indent );

private:

   bool parseAndReplyToExtSearchReq( DOMNode* cur, DOMNode* out,
                                     DOMDocument* reply, 
                                     bool indent,
                                     const HttpHeader& inHeaders )
      throw ( XMLServerErrorMsg );

   // Get hard coded color for service
   uint32 getColour(uint32 serviceId );
   
   /// Needs some stuff from the parser thread.
   XMLParserThread* m_xmlParserThread;
   
};

#endif // XMLEXTSERVICEHELPER_H
