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

#include "XPathMultiExpression.h"
#include "XMLInit.h"

//
// This is a test for XPath evaluation.
//


using namespace XMLTool;
using namespace XPath;

/// Compares a predefined value to a parsed value.
struct TestExpression: public MultiExpression::NodeEvaluator {
   /// @param value Compare to this value.
   TestExpression( const MC2String& value,
                   int countdown ):
      m_value( value ),
      m_evaluatedCountdown( countdown )  {
   }

   ~TestExpression() {
      // all expressions must be evaluated.
      MC2_TEST_CHECK_EXT( m_evaluatedCountdown == 0, m_value );
   }

   void operator()( const DOMNode* node ) {
      MC2_TEST_CHECK_EXT( XMLUtility::getChildTextStr( *node ) == m_value,
                          m_value );
      m_evaluatedCountdown--;
   }

   MC2String m_value; ///< expected value
   int m_evaluatedCountdown; ///< Countdown on number of times evaluated
};


MC2_UNIT_TEST_FUNCTION( xpathTest ) {
   // setup expression
   MultiExpression::Description desc;
   typedef MultiExpression::Description::value_type NodeDesc;
   desc.push_back( (NodeDesc){ "main/a/b/c", new TestExpression( "the_c", 1 ) } );
   desc.push_back( (NodeDesc){ "main/a/b/d", new TestExpression( "the_d", 1 ) } );
   desc.push_back( (NodeDesc){ "main/e/f", new TestExpression( "the_f", 1 ) } );
   desc.push_back( (NodeDesc){ "main/g", new TestExpression( "the_g", 1 ) } );
   desc.push_back( (NodeDesc){ "main/m*", new TestExpression( "the_m*", 3 ) } );
   desc.push_back( (NodeDesc){ "main/a/b/@battr",
            new TestExpression( "b_attr", 1 ) } );
   MultiExpression expr( desc );

   // data to evaluate
   MC2String xmlData( "<main> \
    <a> \
      <b battr=\"b_attr\"> \
        <c>the_c</c> \
        <d>the_d</d> \
      </b> \
      <e> \
        <f>not_the_f</f> \
      </e> \
    </a> \
    <e> \
      <f>the_f</f> \
    </e> \
    <e> \
      <f>not_the_f</f> \
    </e> \
    <g>the_g</g> \
    <g>the_g</g> \
    <m>the_m*</m> \
    <m>the_m*</m> \
    <m>the_m*</m> \
    </main>"  );

   XMLTool::XMLInit initXML;

   XercesDOMParser xmlParser;
   xmlParser.setValidationScheme( XercesDOMParser::Val_Auto );
   xmlParser.setIncludeIgnorableWhitespace( false );

   XStr resourceName( "TestParser" );
   MemBufInputSource
      source( reinterpret_cast<const XMLByte*>( xmlData.c_str() ), xmlData.size(),
              resourceName.XMLStr() );

   xmlParser.parse( source );
   const DOMNode* document = xmlParser.getDocument();
   MC2_TEST_REQUIRED( document );
   MC2_TEST_REQUIRED( document->getFirstChild() );

   // parse and evaluate data
   expr.evaluate( document );
}
