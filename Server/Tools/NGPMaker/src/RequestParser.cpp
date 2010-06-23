/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RequestParser.h"
#include "NodeAssigner.h"
#include "XMLInit.h"

#include <sax/SAXParseException.hpp>

using namespace XMLTool;
using namespace XMLTool::XPath;

/// specialization for NParam_t in xml-tag handling
template <>
void Assigner<NParam::NParam_t>::assign( const MC2String& strval ) {
   NParam::NParam_t type;
   if ( NGPMaker::getTypeFromString( strval, type ) ) {
      m_value = type;
   }
}

namespace NGPMaker {

using namespace XMLTool::XPath;

class RequestMatches: public MatchHandler< Request > {
public:
   RequestMatches():m_paramMatches( new MatchHandler< Param>() ) {
      // setup sub tree for <param> tags
      Param& currMatch = m_paramMatches->getCurrMatch();
      MultiExpression::NodeDescription desc[] = {
         { "param*", m_paramMatches },
         { "param/@id", makeAssigner( currMatch.m_id ) },
         { "param/@type", makeAssigner( currMatch.m_type ) },
         { "param/value", makeAssigner( currMatch.m_value ) },
         { "param/desc", makeAssigner( currMatch.m_desc ) },
      };
      m_paramExp.reset( new MultiExpression( MultiExpression::Description
                                             ( desc, desc + 
                                               sizeof( desc ) / 
                                               sizeof ( desc[ 0 ] ) ) ) );
   }

   void operator() ( const DOMNode* n ) {
      // once we hit a request, then search for its paramters

      MatchHandler< Request >::operator()( n );
      // evaluate sub tree that contains parameters
      m_paramMatches->reset();
      m_paramExp->evaluate( n );
      m_paramMatches->handleNewMatch();
      getCurrMatch().m_params = m_paramMatches->getMatches();
      
   }
private:
   MatchHandler< Param >* m_paramMatches;
   auto_ptr<MultiExpression> m_paramExp;
};

RequestParser::RequestParser():
   m_reqMatches( new RequestMatches() ) {

   // setup main expression tree for parsing
   Request& currMatch = m_reqMatches->getCurrMatch();
   MultiExpression::NodeDescription desc[] = {
      { "request*", m_reqMatches },
      { "request/@protocolVersion", 
        makeAssigner( currMatch.m_protocolVersion ) },
      { "request/@type", makeAssigner( currMatch.m_type ) },
      { "request/@id", makeAssigner( currMatch.m_id ) },
      { "request/@version", makeAssigner( currMatch.m_version ) },
      { "request/@useGzip", makeAssigner( currMatch.m_useGzip ) },
      { "request/desc", makeAssigner( currMatch.m_desc ) },
      { "request/name", makeAssigner( currMatch.m_name ) },
   };

   m_reqExp.
      reset( new MultiExpression( MultiExpression::Description
                                  ( desc, desc + 
                                    sizeof( desc ) / 
                                    sizeof ( desc[ 0 ] ) ) ) );

   
}

RequestParser::~RequestParser() {
   
}

RequestParser::Requests& RequestParser::getRequests() {
   return m_reqMatches->getMatches();
}

bool RequestParser::parse( const MC2String& filename ) try {
   // setup xerces dom parser and parse the file
   XercesDOMParser parser;
   parser.setValidationScheme( XercesDOMParser::Val_Auto );
   parser.setIncludeIgnorableWhitespace( false );
   parser.parse( filename.c_str() );

   // check for mc2ngp root node

   DOMNode* node = parser.getDocument();
   if ( node == NULL ) {
      mc2log << "Failed to find root node." << endl;
      return false;
   }

   const char* tagname = "mc2ngp";
   const DOMElement* root = 
      static_cast<const DOMElement*>( XMLTool::
                                      findNodeConst( node, tagname ) );
   if ( root == NULL ) {
      mc2log << "Missing start tag \"" << tagname << "\"" << endl;
      return false;
   }

   // evaluate xml document using multi expression
   m_reqMatches->reset();
   m_reqExp->evaluate( root );
   m_reqMatches->handleNewMatch();

   return true;

} catch ( const XMLException& e ) {

   mc2log << error 
          << "[RequestParser] A XMLError occured "
          << "during parsing of initialize request: "
          << e.getMessage() << " line " 
          << e.getSrcLine() << endl;

   return false;

} catch ( const SAXParseException& e ) {

   mc2log << error
          << "[RequestParser]: A SAXerror occured "
          << "during parsing of initialize request: "
          << e.getMessage() << ", "
          << "line " << e.getLineNumber() << ", column " 
          << e.getColumnNumber() << endl;

   return false;
} 

}
