/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "XMLUtility.h"
#include "UTF8Util.h"
#include "CharEncoding.h"
#include "TextIterator.h"
#include "StackOrHeap.h"

#ifdef USE_XML

void
XMLUtility::transcodeToXML( XMLCh* dest,
                            const char* src )
{
   uint32 out_pos = 0;
   // Do the needful.
   mc2TextIterator it ( src );
   uint32 cur_char;
   while ( ( cur_char = *it ) != 0 ) {
      // Will this work for characters > 65535 ? XMLCh is utf-16
      dest[ out_pos++ ] = cur_char;
      ++it;
   }
   // Always zero terminate
   dest[ out_pos++ ] = 0;
}


XMLCh* 
XMLUtility::transcodetoucs( const char* const str ) {
   // There will be max as many chars in the outstring as
   // in the instring. (utf8 -> utf-16)
   XMLCh* ret_val = new XMLCh[ strlen(str) + 1 ];
   transcodeToXML( ret_val, str );
   return ret_val;
}


MC2String 
XMLUtility::transcodefrom( const XMLCh* str ) {
   MC2String result;
   if ( str && str[0] != 0 ) {
      result.reserve(32);
      int pos = 0;
      while ( str[ pos ] != 0 ) {
         result += UTF8Util::ucsToMc2( str[ pos++ ] );
      }
   }   
   return result;
}


char*
XMLUtility::transcodefromucs( const XMLCh* str) {
   return NewStrDup::newStrDup( transcodefrom( str ).c_str() );
}

/// Used by indentPiece
static inline void putTerm( XMLCh* indentStr,
                            int indentLevel,
                            int nbr_spaces,
                            int max_indent )
{
   int total_space = nbr_spaces * MIN(indentLevel, max_indent);
   indentStr[total_space+1] = 0;
}

/// Used by indentPiece
static inline void remTerm( XMLCh* indentStr,
                            int indentLevel,
                            int nbr_spaces,
                            int max_indent )
{
   int total_space = nbr_spaces * MIN(indentLevel, max_indent);
   indentStr[total_space+1] = 32;
}

inline void
XMLUtility::indentPiece( DOMNode* top,
                         DOMNode* parent,
                         int indentLevel,
                         unsigned int nbr_spaces,
                         XMLCh* indentStr,
                         int max_indent )
{
   DOMDocument* docu = top->getOwnerDocument();
   if ( parent != NULL ) {
      putTerm( indentStr, indentLevel, nbr_spaces, max_indent );
      parent->insertBefore( docu->createTextNode( indentStr ),
                            top );
      remTerm( indentStr, indentLevel, nbr_spaces, max_indent );
   }

   // Don't put text after end tag if the node contains text.
   bool appendAfter = top->getFirstChild() &&
      top->getFirstChild()->getNodeType() != DOMNode::TEXT_NODE;
  
   
   
   for ( DOMNode* node = top->getFirstChild();
         node != NULL;
         node = node->getNextSibling() ) {
      switch ( node->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
         {
            indentPiece( node, top, indentLevel + 1, nbr_spaces, indentStr,
                         max_indent);            
         }
         break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Don't indent text.
            break;
         default:
            // What this?
            break;
      }       
   }
   if ( appendAfter ) {
      putTerm( indentStr, indentLevel, nbr_spaces, max_indent );
      top->appendChild( docu->createTextNode( indentStr ));
      remTerm( indentStr, indentLevel, nbr_spaces, max_indent );
   }
}

void
XMLUtility::indentPiece( DOMNode& top,
                         int indentLevel,
                         unsigned int nbr_spaces )
{
   // Note that this function can throw some XML-exceptions.
   if ( indentLevel < 0 ) {
      return;
   }

   nbr_spaces = MIN(8, nbr_spaces);

   static const int max_indent = 10;
   const int size = nbr_spaces*max_indent+1+1;

   // Will be quick if less than 40 chars are needed
   StackOrHeap<40, XMLCh> indentStr( size );
   
   for( int i = 0; i < size; ++i ) {
      indentStr[i] = 32; // Space
   }
   indentStr[0] = '\n';

   DOMNode* parent = NULL;
   if( top.getParentNode() ) {
      parent = top.getParentNode();
    }
   
   if ( nbr_spaces != 3 ) {
      indentPiece( &top, parent, indentLevel, nbr_spaces,
                   indentStr, max_indent );
   } else {
      // Optimize for 3
      indentPiece( &top, parent, indentLevel, 3,
                   indentStr, max_indent );
   }
}


bool
XMLUtility::copyAttribute( DOMElement& dest, 
                           const DOMNode& src,
                           const char* attribName ) {
   const DOMNode* srcNode = src.getAttributes()->
      getNamedItem( X( attribName ) );
   if ( srcNode == NULL ) {
      return false;
   }

   dest.setAttribute( X( attribName ), srcNode->getNodeValue() );

   return true;
}

DOMElement* 
XMLUtility::createStandardReply( DOMDocument& root,
                                 const DOMNode& request,
                                 const char* replyName ) {
   DOMElement* reply = root.createElement( X( replyName ) );
   copyAttribute( *reply, request, "transaction_id" );

   return reply;
}

MC2String
XMLUtility::getChildTextStr( const DOMNode& cur, MC2String defaultValue ) {
   bool nodeValueFound = false;
   MC2String retVal;
   DOMNode* child = cur.getFirstChild();
   while ( child != NULL ) {
      if ( child->getNodeType() == DOMNode::TEXT_NODE ||
           child->getNodeType() == DOMNode::CDATA_SECTION_NODE ) {
         retVal.append( XMLUtility::transcodefrom( child->getNodeValue()) );
         nodeValueFound = true;
      }
      child = child->getNextSibling();
   }

   if ( !nodeValueFound ) {
      return defaultValue;
   }
   
   return retVal;
}



#endif // USE_XML
