/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLPARSERHELPER_H
#define XMLPARSERHELPER_H

#ifdef USE_XML
#include "config.h"
#include <map>
#include <list>
#include <set>

#include "XMLUtility.h"
#include <dom/DOM.hpp>

/**
 * Class that can make handling XML documents easier.
 *
 * Loads and parses XML documents.
 * It is possible to get the top xml nodes either sorted by tag
 * or sorted by the order that they come in.
 *
 */
class XMLParserHelper {
   public:

      /**
       *    Function object comparing XMLCh*s (less than).
       */
      struct domltstr {
         bool operator()( const XMLCh* str1, 
                          const XMLCh* str2 ) const {
            return ( XMLString::compareString( str1, str2 ) < 0 );
         }
      };
      
      /**
       *    Multimap that holds xml-nodes grouped by their xml tag.
       */
      typedef multimap<const XMLCh*, DOMNode*, domltstr> xmlByTag_t;
      
      /**
       *    List of xml-nodes.
       */
      typedef list<DOMNode*> xmlByOrder_t;

      /**
       *    Set of strings.
       */
      typedef set<const XMLCh*, domltstr> domStringSet_t;
      
      /**
       *    Constructor.
       *    
       *    @param   xmlTopTag         The top xml tag, 
       *                               ie. "map_generation-mc2".
       *    @param   xmlNodesByOrder   List of xml tags of the nodes 
       *                               that should be grouped by the 
       *                               order they come in.
       *    @param   xmlNodesByTag     List of xml tags of the nodes 
       *                               that should be grouped by their
       *                               xml tag.
       */
      XMLParserHelper( const XMLCh* xmlTopTag,
                       const domStringSet_t& xmlNodesByOrder,
                       const domStringSet_t& xmlNodesByTag );

      /**
       *    Destructor.
       */
      virtual ~XMLParserHelper();

      /**
       *    Loads, parses and sorts the specified xml files.
       *    Requires that the XML system has been initialized first.
       *    @see XMLPlatformUtils::Initialize().
       *    
       *    @param   xmlFiles Set of xml filenames to load.
       *    @return  If the xml files could be loaded, parsed and sorted.
       */
      bool load( const set<const char*>& xmlFiles );

      /**
       *    Get the xml nodes that are sorted by order.
       *    Note that only direct childs to the top xml node will 
       *    be included in the list.
       *    @return The xml nodes that are sorted by order.
       */
      inline xmlByOrder_t& getXMLNodesByOrder();

      /**
       *    Get the xml nodes that are sorted by tags.
       *    Note that only direct childs to the top xml node will 
       *    be included in the list.
       *    @return The xml nodes that are sorted by tags.
       */
      inline xmlByTag_t& getXMLNodesByTag();

   private:
      
      /**
       *    Parses and sorts the xml bufferts.
       *    @param   xmlBuffs Set of xml bufferts.
       */
      bool parseAndSort( const set<InputSource*>& xmlBuffs );

      /**
       *    XML nodes that are sorted by order.
       */
      xmlByOrder_t m_xmlNodesByOrder;

      /**
       *    XML nodes that are sorted by tag.
       */
      xmlByTag_t m_xmlNodesByTag;

      /**
       *    The xml tags of the xml nodes that should be sorted by order.
       */
      domStringSet_t m_toSortByOrder;
      
      /**
       *    The xml tags of the xml nodes that should be sorted by tag.
       */
      domStringSet_t m_toSortByTag;

      /**
       *    The top xml tag.
       */
      const XMLCh* m_xmlTopTag;

      /**
       *    Array of xml documents.
       */
      DOMDocument** m_xmlDocuments;

      /**
       *    The parser of this parser helper.
       */
      XercesDOMParser m_xmlParser;

};



// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -

inline XMLParserHelper::xmlByOrder_t&
XMLParserHelper::getXMLNodesByOrder() 
{
   return m_xmlNodesByOrder;
}

inline XMLParserHelper::xmlByTag_t&
XMLParserHelper::getXMLNodesByTag()
{
   return m_xmlNodesByTag;
}

#endif // USE_XML

#endif // XMLUTILITY_H

