/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLCategoryListNode.h"

#include "XMLTool.h"
#include "XMLNodeIterator.h"
#include <boost/lexical_cast.hpp>

namespace CategoryListNode {

/**
 * Read categories from within category_list node.
 * @param cur Should contain node category_list.
 * @param categories will contain parsed category ids
 */
void
readCategoryList( DOMNode* cur,
                  CompactSearch::CategoryIDs& categories ) {
   // See if we have any category_list node
   DOMNode* categoryListNode = XMLTool::findNode( cur, "category_list" );
   if ( categoryListNode == NULL ||
        categoryListNode->getFirstChild() == NULL ) {
      return;
   }

   XMLTool::ElementIterator categoryListIt( categoryListNode->getFirstChild());
   XMLTool::ElementIterator categoryListItEnd();

   // read categories
   for ( ; categoryListIt != XMLTool::ElementIterator(); ++categoryListIt ) {
      try {

         if ( XMLString::equals( (*categoryListIt)->getNodeName(),
                                 "category_id" ) ) {
            categories.push_back( boost::
                                  lexical_cast< CategoryTreeUtils::CategoryID >
                                  ( XMLUtility::
                                    getChildTextStr( **categoryListIt ) ) );
         }

      } catch ( const boost::bad_lexical_cast& ) {
         mc2dbg << "[XMLCompactSearchRequest] "
                << "invalid category id: " <<
            XMLUtility::getChildTextStr( **categoryListIt )
                << endl;
      }
   }
}

void addCategoryList( DOMNode* baseNode,
                      const CompactSearch::CategoryIDs& categories ) {
   DOMElement* catListNode =
      XMLTool::addNode( baseNode, "category_list" );

   for ( CompactSearch::CategoryIDs::const_iterator
            catIt = categories.begin(),
            catItEnd = categories.end();
         catIt != catItEnd;
         ++catIt ) {
      XMLTool::addNode( catListNode, "category_id", (uint32)*catIt );
   }
}

} // CategoryListNode
