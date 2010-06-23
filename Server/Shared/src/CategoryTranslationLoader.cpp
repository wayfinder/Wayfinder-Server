/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CategoryTranslationLoader.h"
#include "XPathExpression.h"
#include "XMLNodeIterator.h"
#include "XMLUtility.h"

using namespace XMLTool;

bool 
CategoryTranslationLoader::loadTranslations( const MC2String& filename ) try {
   // load and parse file
   XercesDOMParser parser;
   parser.setValidationScheme( XercesDOMParser::Val_Auto );
   parser.setIncludeIgnorableWhitespace( false );

   parser.parse( filename.c_str() );

   if ( parser.getDocument() == NULL ) {
      return false;
   }

   loadTranslations( parser.getDocument() );

   return true;

} catch ( const XMLTool::Exception& e ) {
   // any xml exception here is bad.
   return false;
}

/**
 * @param node Current category node to parse.
 */
void 
CategoryTranslationLoader::loadTranslations( const DOMNode* node ) 
   throw ( XMLTool::Exception ) {

   if ( node == NULL ) {
      return;
   }

   // get category nodes and find "name" nodes
   XPath::Expression::result_type rootNodes = 
      XPath::Expression( "/category_data/categories/category*" ).
      evaluate( node->getFirstChild() );
   ElementConstIterator nameEndIt( NULL );
   for ( uint32 i = 0; i < rootNodes.size(); ++i ) {
      uint32 catID;
      getAttrib( catID, "id", rootNodes[ i ] );

      ElementConstIterator nameIt( rootNodes[ i ]->getFirstChild() );
      for ( ; nameIt != nameEndIt; ++nameIt ) {
         // is name "name"? :)
         bool isName = XMLString::equals( nameIt->getNodeName(), "name" );
         bool isSearchName = XMLString::equals( nameIt->getNodeName(), 
                                                "search_name" );
         if ( isName || isSearchName ) {
            // get language attribute
            LangTypes::language_t lang;
            getAttrib( lang, "language", *nameIt );
            void (CategoryTranslationLoader::*callFunction)( 
               uint32, LangTypes::language_t, const MC2String& ) =
               &CategoryTranslationLoader::newTranslationValue;
            if ( isSearchName ) {
               callFunction = &CategoryTranslationLoader::newSearchNameValue;
            }
            (this->*callFunction)( catID, lang,
                                   XMLUtility::getChildTextStr( **nameIt ) );
         }
      }
   }
}

void
CategoryTranslationLoader::newSearchNameValue( uint32 catID, 
                                               LangTypes::language_t lang, 
                                               const MC2String& stringValue )
{
   // Default is to ignore such nodes.
}
