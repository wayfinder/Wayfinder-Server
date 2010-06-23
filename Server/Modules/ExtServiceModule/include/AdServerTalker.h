/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ADSERVERTALKER_H
#define ADSERVERTALKER_H

#include "config.h"
#include "ExtServiceTalker.h"

#include <memory>

namespace XMLTool {
namespace XPath {
class MultiExpression;
class Expression;
}
}

class AdServerMatches;
class URLFetcher;

/**
 * Handles Advertisement matches
 *
 */
class AdServerTalker: public ExtServiceTalker {
public:
   AdServerTalker();
   ~AdServerTalker();

   /**
    * The function called to initiate a xml query to advertisement
    * server.
    *
    * @param reply        Where we store the search result to return
    * @param searchData   The user input data to create a query from
    * @param nbrRetries   The number of times to retry the xml query
    */
   int doQuery( SearchReplyData& reply,
                const ExternalSearchRequestData& searchData, 
                int nbrRetries = 3 );

   /**
    * Get more info about one item
    *
    * @param reply           Where we store the search result to return
    * @param ExtServicethe   type of service
    * @param externalID      The id of the info
    * @param lang            language
    * @param nbrRetries      The number of times to retry the xml query
    */
   int doQuery( SearchReplyData& reply,
                const ExtService& service, 
                const MC2String& externalId, 
                const LangType& lang, 
                int nbrRetries = 5 );
private:
   int parseResult( SearchReplyData& reply, 
                    const ExternalSearchRequestData& searchData,
                    const MC2String& xml_result,
                    const MC2String& url );
   int sendRequest( MC2String& xml_result, MC2String& url, 
                    const ExternalSearchRequestData& searchData );

   AdServerMatches* m_matches; //< holds information about matches during parsing

   /// path expression for adserver
   auto_ptr<XMLTool::XPath::MultiExpression> m_searchExp;
   /// root path for search expression, will be evaluated first
   auto_ptr<XMLTool::XPath::Expression> m_rootExp;
   /// number of hits in the search reply
   auto_ptr<XMLTool::XPath::Expression> m_docCountExp;
   /// fetches the results
   auto_ptr<URLFetcher> m_urlFetcher;
};

#endif // ADSERVERTALKER_H
