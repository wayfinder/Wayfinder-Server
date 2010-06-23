/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NGPMAKER_REQUESTPARSER_H
#define NGPMAKER_REQUESTPARSER_H

#include "MC2String.h"
#include "Request.h"

#include <memory>
#include <vector>

namespace XMLTool {
namespace XPath {
class MultiExpression;
}
}

namespace NGPMaker {

class RequestMatches;

/**
 * Parses XML document and creates Request from it. 
 *
 */
class RequestParser {
public:
   RequestParser();
   ~RequestParser();

   /// Container to hold requests
   typedef vector<Request> Requests;

   /// @return requests created from the latest document parsed
   Requests& getRequests();

   /**
    * Parses xml document and fill in request container
    * @param filename the xml document filename 
    * @return true if parsing was successful
    */
   bool parse( const MC2String& filename );

private:
   RequestMatches* m_reqMatches;  ///< holds all requests
   /// xml xpath expression used for parsing xml document
   auto_ptr<XMLTool::XPath::MultiExpression> m_reqExp;
};

}
#endif
