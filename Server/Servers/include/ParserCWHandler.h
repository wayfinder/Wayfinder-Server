/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSERCWHANDLER_H
#define PARSERCWHANDLER_H

#include "config.h"
#include "ParserHandler.h"
#include "MC2String.h"
#include <map>
#include "LangTypes.h"


class HttpHeader;


/**
 * Class handling cw operations.
 *
 */
class ParserCWHandler : public ParserHandler {
   public:
      /**
       * Constructor.
       */
      ParserCWHandler( ParserThread* thread,
                       ParserThreadGroup* group );

      /**
       * Update CW request.
       */
      void updateRequest( MC2String& urlStr, 
                          LangTypes::language_t clientLang ) const;

      /**
       * Get an URL.
       * On error sets an error reply.
       *
       * @param urlStr The URL to get.
       * @param postData The post data if any.
       * @param peerIP The client's IP address.
       * @param fromByte The index of the first byte to get.
       * @param toByte The index of the last byte to get.
       * @param clientLang The language to get.
       * @param inHeaders The client http headers if any, may be NULL.
       * @param outHeaders Set to the reply headers.
       * @param reply Set to the reply body.
       * @param startByte Set to the index of first byte in the reply.
       * @param endByte Set to the index of last byte in the reply.
       * @return The URLFetcher returncode.
       */
      int getURL( const MC2String& urlStr, const MC2String& postData,
                  uint32 peerIP, uint32 fromByte, uint32 toByte,
                  LangTypes::language_t clientLang,
                  const HttpHeader* inHeaders,
                  HttpHeader& outHeaders, MC2String& reply,
                  uint32& startByte, uint32& endByte );

  private:
      class CWReplace {
         public:
         CWReplace( const MC2String& replaceWithArg, 
                    const MC2String& defaultPageArg = "" )
            : replaceWith( replaceWithArg ), defaultPage( defaultPageArg )
            { }

         MC2String replaceWith;
         MC2String defaultPage;
      };

      typedef map< MC2String, CWReplace > repmap;
      repmap m_repurls;
};


#endif // PARSERCWHANDLER_H

