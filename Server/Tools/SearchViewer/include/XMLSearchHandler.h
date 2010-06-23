/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHVIEWER_XMLSEARCHHANDLER_H
#define SEARCHVIEWER_XMLSEARCHHANDLER_H

#include "SearchHandler.h"
#include <memory>
#include "XMLUtility.h"

namespace XMLTool {
namespace XPath {
class MultiExpression;
}
}

namespace SearchViewer {
/**
 * Handles search for XML protocol
 */
class XMLSearchHandler: public SearchHandler {
public:
   XMLSearchHandler();
   /// @see SearchHandler
   void search( const IPnPort& address,
                const Auth& auth,
                const MC2String& crc,
                const CompactSearch& search );
   /// @see SearchHandler
   const Headings& getMatches() const;
   /// @see SearchHandler
   void getReplyDebugData( MC2String& data, uint32 maxColumns );
private:
   /// add result node to result window
   //   void addResult( const DOMNode* root, GtkWidget* resultWindow );
   void addResult( SearchMatch& match, const DOMNode* result );
   /// parse search results
   void parseResult( const MC2String& filename );

   void parseResultFromData( const MC2String& data );
   void parseResult( const DOMDocument* doc );

   class SearchMatchHandler;
   class HeadingMatchHandler;
   class DescMatchHandler;

   std::auto_ptr<SearchMatchHandler> m_matches;
   std::auto_ptr<HeadingMatchHandler> m_headingMatches;
   std::auto_ptr<DescMatchHandler> m_descMatches;

   std::auto_ptr<XMLTool::XPath::MultiExpression> m_searchExp;
   std::auto_ptr<XMLTool::XPath::MultiExpression> m_hitExp;
   std::auto_ptr<XMLTool::XPath::MultiExpression> m_descExp;

   Headings m_headings;
   MC2String m_replyData; ///< xml reply from server
};

}

#endif
