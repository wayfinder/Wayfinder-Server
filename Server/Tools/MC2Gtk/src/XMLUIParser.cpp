/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLUIParser.h"
#include "XMLParser.h"
#include "XMLUtility.h"
#include "XMLTool.h"

namespace MC2Gtk {

namespace XMLUI {

UIParser UIParser::s_instance;

UIParser::UIParser() {
   // autoregister xml objects
   autoRegister();
}

UIParser& UIParser::instance() {
   return s_instance;
}

GtkWidget* UIParser::parseElement( const MC2String& name,
                                   const DOMElement* element ) {
   Parsers::iterator it = instance().m_parsers.find( name );
   if ( it == instance().m_parsers.end() ) {
      return NULL;
   }

   GtkWidget *widget = (*it).second->parse( element );

   // parse the common id="specific id" and
   // register it to our id register.
   
   try {
      MC2String id;
      XMLTool::getAttrib( id, "id", element );
      registerID( widget, id );
   } catch ( const XMLTool::Exception& e ) {
   }

   return widget;
}

void UIParser::registerParser( Parser* parser, const char* tagName ) {
   instance().m_parsers[ tagName ] = parser;
}

UIParser::WidgetVector UIParser::parseFromData( const MC2String& data ) try {
   XercesDOMParser parser;
   parser.setValidationScheme( XercesDOMParser::Val_Auto );
   parser.setIncludeIgnorableWhitespace( false );
   // setup data source
   MemBufInputSource 
      source( reinterpret_cast<const XMLByte*>( data.data() ), 
              data.size(),
              X( "data" ) );

   parser.parse( source );
   // get first node <mc2gtk> and start parsing
   DOMNode* node = parser.getDocument();
   if ( node == NULL ) {
      return WidgetVector();
   }

   const DOMElement* root = 
      static_cast<const DOMElement*>( XMLTool::
                                      findNodeConst( node,
                                                     "mc2gtk" ) );
   if ( root == NULL ) {
      throw Exception( "Missing start tag \"mc2gtk\"" );
   }

   // parse all children
   return parseChildren( static_cast<const DOMElement*>( root  ) );
} catch ( const XMLException& e ) {
   mc2log << error << "An error occurred during parsing\n   Message: "
          << e.getMessage() << endl;
   
   return WidgetVector();
} catch (const DOMException& e) {
   mc2log << error << "\nDOM Error during parsing data." << endl
          << "DOMException code is:  " << e.code << endl;
   
   return WidgetVector();

} catch (...) {
   mc2log << error << "An error occurred during parsing " << endl;
   return WidgetVector();
}

UIParser::WidgetVector UIParser::parseFile( const MC2String& filename ) try {

   mc2log << "Parse user interface file: " << filename << endl;
   XercesDOMParser parser;
   parser.setValidationScheme( XercesDOMParser::Val_Auto );
   parser.setIncludeIgnorableWhitespace( false );

   parser.parse( filename.c_str() );
   
   // get first node <mc2gtk> and start parsing
   DOMNode* node = parser.getDocument();
   if ( node == NULL ) {
      return WidgetVector();
   }

   const DOMElement* root = 
      static_cast<const DOMElement*>( XMLTool::
                                      findNodeConst( node,
                                                     "mc2gtk" ) );
   if ( root == NULL ) {
      throw Exception( "Missing start tag \"mc2gtk\"" );
   }

   // parse all children
   return parseChildren( static_cast<const DOMElement*>( root  ) );

} catch ( const XMLException& e ) {
   mc2log << error << "An error occurred during parsing\n   Message: "
          << e.getMessage() << endl;
   
   return WidgetVector();
} catch (const DOMException& e) {
   mc2log << error << "\nDOM Error during parsing: '" << filename << "'\n"
          << "DOMException code is:  " << e.code << endl;
   
   return WidgetVector();

} catch (...) {
   mc2log << error << "An error occurred during parsing " << endl;
   return WidgetVector();
}

void UIParser::registerID( GtkWidget* widget, const MC2String& id ) {
   if ( ! instance().m_widgets.
        insert( make_pair( id, widget ) ).second  ) {
      mc2log << warn << "Duplicate widget id! id \"" 
             << id << "\" already exist." << endl;
   }
}

GtkWidget* UIParser::findWidget( const MC2String& id ) throw () {
   WidgetIDs::iterator it = instance().m_widgets.find( id );
   if ( it == instance().m_widgets.end() ) {
      return NULL;
   }
   return (*it).second;
}

GtkWidget* UIParser::getWidget( const MC2String& id ) throw (Exception) {
   GtkWidget* widget = findWidget( id );
   if ( widget == NULL ) { 
      throw Exception( MC2String("Could not find widget with id \"") +
                       id + "\"" );
   }
   return widget;
}

vector<MC2String> UIParser::getWidgetIDs() {
   vector<MC2String> results( instance().m_widgets.size() );

   WidgetIDs::iterator it = instance().m_widgets.begin();
   WidgetIDs::iterator itEnd = instance().m_widgets.end();
   for (uint32 i = 0; it != itEnd; ++it, ++i ) {
      results[ i ] = (*it).first;
   }

   return results;
}

UIParser::WidgetVector parseChildren( const DOMElement* root ) {
   UIParser::WidgetVector children;
   if ( root->getFirstChild() == NULL ) {
      return children;
   }

   for ( DOMNode* child = root->getFirstChild();
         child != NULL; child = child->getNextSibling() ) {
      // we are only interested in element nodes
      if ( child->getNodeType() != DOMNode::ELEMENT_NODE ) {
         continue;
      }

      GtkWidget* widget = 
         UIParser::
         parseElement( XMLUtility::transcodefrom( child->getNodeName() ),
                       static_cast<DOMElement*>( child ) );
      // only add non null objects
      if ( widget ) {
         children.push_back( widget );
      }
   }

   return children;
}

} // XMLUI



} // MC2Gtk
