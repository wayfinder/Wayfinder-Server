/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2UnitTestMain.h"
#include "XMLInit.h"
#include "StringSearchUtility.h"

XMLTool::XMLInit xmlInit;

MC2_UNIT_TEST_FUNCTION( testSimpleHouseNumber ) {
   XercesDOMParser parser;
   parser.parse( "SimpleHouseNumbers.xml" );
   MC2_TEST_REQUIRED( parser.getDocument() != NULL );

   DOMDocument* doc = parser.getDocument();
   DOMElement* root = static_cast<DOMElement*>( doc->getDocumentElement() );
   DOMNodeList* nodelist = root->getElementsByTagName( X( "housenumber" ) );

   for ( XMLSize_t i = 0; i < nodelist->getLength(); ++i ) {
      DOMElement* element = static_cast<DOMElement*>( nodelist->item( i ) );
      MC2String value = 
         XMLUtility::transcodefrom( element->getAttribute( X( "value" ) ) );
      MC2String good =
         XMLUtility::transcodefrom( element->getAttribute( X( "good" ) ) );

      int number;
      MC2_TEST_CHECK_EXT( 
         StringSearchUtility::isSimpleHouseNumber( value, number ) ==
         ( good == "true" ),
         MC2String("Failed: \"") + value + "\"" );
   }
}

MC2_UNIT_TEST_FUNCTION( testSimpleGetStreetNumberAndName ) {
   XercesDOMParser parser;
   parser.parse( "SimpleStreetNumberAndName.xml" );
   MC2_TEST_REQUIRED( parser.getDocument() != NULL );

   DOMDocument* doc = parser.getDocument();
   DOMElement* root = static_cast<DOMElement*>( doc->getDocumentElement() );
   DOMNodeList* nodelist = root->getElementsByTagName( X( "streetname" ) );

   for ( XMLSize_t i = 0; i < nodelist->getLength(); ++i ) {
      DOMElement* element = static_cast<DOMElement*>( nodelist->item( i ) );
      MC2String name = 
         XMLUtility::transcodefrom( element->getAttribute( X( "name" ) ) );
      MC2String street =
         XMLUtility::transcodefrom( element->getAttribute( X( "street" ) ) );
      MC2String number =
         XMLUtility::transcodefrom( element->getAttribute( X( "number" ) ) );

      int resultNumber;
      MC2String resultStreet;
      StringSearchUtility::simpleGetStreetNumberAndName( name, 
                                                         resultNumber,
                                                         resultStreet );

      MC2_TEST_CHECK_EXT( resultNumber == boost::lexical_cast<int>( number ) &&
                          resultStreet == street,
                          MC2String("Failed: \"") + name + "\"" );
   }   
}
