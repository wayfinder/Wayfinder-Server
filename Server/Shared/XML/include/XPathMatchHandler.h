/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XPATH_MATCH_HANDLER_H
#define XPATH_MATCH_HANDLER_H

#include "config.h"

#include "XPathMultiExpression.h"

#include <vector>

namespace XMLTool {
namespace XPath {

/// handles new matches
template <typename MatchType>
class MatchHandler: public MultiExpression::NodeEvaluator {
public:
   typedef vector<MatchType> Matches;
   MatchHandler():m_first( true ), m_nbrHits( 0 ) { }
   virtual ~MatchHandler() { }

   void operator ()( const DOMNode* node ) {
      handleNewMatch();
   }

   /// called when the "start of match"-node is found.
   void handleNewMatch() {
      if ( ! m_first ) {
         m_matches.push_back( m_currMatch );
      }
      m_currMatch = MatchType();
      m_first = false;
   }
   // reset the all matches and start from beginning
   void reset() {
      m_first = true;
      m_nbrHits = 0;
      m_matches.clear();
      m_currMatch = MatchType();
   }
   /// @return the current match
   MatchType& getCurrMatch() { return m_currMatch; }

   /// @return a vector of ready matches
   const Matches& getMatches() const { return m_matches; }
   /// @return a vector of ready matches
   Matches& getMatches() { return m_matches; }

   /// @return total number of hits in a search
   uint32 getNbrHits() const { return m_nbrHits; }
   /// @return reference to total hits, used when creating path expression
   uint32& getNbrHitsRef() { return m_nbrHits; }
private:
   Matches m_matches; ///< processed and ready matches
   MatchType m_currMatch; ///< current match type beeing processed
   bool m_first; ///< true if no match has yet been processed
   uint32 m_nbrHits; ///< nbr of maximum total hits in a search.
};

} // XPath
} // XMLTool

#endif // XPATH_MATCH_HANDLER_H
