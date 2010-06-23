/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserHelper.h"

#ifdef USE_XML

#include <dom/DOM.hpp>
#include <parsers/XercesDOMParser.hpp>
#include <util/XercesDefs.hpp>
#include <sax/ErrorHandler.hpp>
#include <sax/SAXParseException.hpp>
#include <framework/LocalFileInputSource.hpp>
#include <util/PlatformUtils.hpp>
#include <util/XMLString.hpp>
#include <util/XMLUniDefs.hpp>
#include <framework/XMLFormatter.hpp>
#include "XMLUtility.h"
#include "XMLParserErrorReporter.h"

XMLParserHelper::XMLParserHelper( const XMLCh* xmlTopTag,
                         const domStringSet_t& xmlNodesByOrder,
                         const domStringSet_t& xmlNodesByTag ) :
                     m_toSortByOrder( xmlNodesByOrder ),
                     m_toSortByTag( xmlNodesByTag ),
                     m_xmlTopTag( xmlTopTag)
{
   m_xmlDocuments = NULL; 

   // Init parser
   m_xmlParser.setValidationScheme( XercesDOMParser::Val_Auto );
   m_xmlParser.setErrorHandler( new XMLParserErrorReporter() );
//   parser.setToCreateXMLDeclTypeNode( true );
   m_xmlParser.setIncludeIgnorableWhitespace( false );


}

XMLParserHelper::~XMLParserHelper()
{
   delete[] m_xmlDocuments;
   delete m_xmlParser.getErrorHandler();
}

bool 
XMLParserHelper::load( const set<const char*>& xmlFiles )
{
   // Load
   if ( xmlFiles.empty() )
      return false;

   set<InputSource*> xmlBuffs;
   
   for ( set<const char*>::const_iterator it = xmlFiles.begin();
         it != xmlFiles.end(); ++it ) {
      mc2dbg << "XMLParserHelper loading: " << *it << endl;
      XMLCh* fileName = XMLUtility::transcodetoucs( *it );
      xmlBuffs.insert( new LocalFileInputSource(fileName) );
      delete [] fileName;
      mc2dbg8 << "XMLParserHelper loaded: " << *it << endl;
   }
   
   // Parse and sort xml.
   mc2dbg << "XMLParserHelper parsing and sorting" << endl;
   bool retval = parseAndSort( xmlBuffs );
   for( set<InputSource*>::iterator it = xmlBuffs.begin();
         it != xmlBuffs.end();
         ++it) {
      delete *it;
   }
   
   mc2dbg << "XMLParserHelper load done." << endl;
   return retval;
}

bool
XMLParserHelper::parseAndSort( const set<InputSource*>& xmlBuffs )
{
   delete[] m_xmlDocuments;
   m_xmlDocuments = new DOMDocument*[ xmlBuffs.size() ];    
   mc2dbg << "Size of XMLParserHelper::m_xmlDocuments = " << xmlBuffs.size() 
          << endl;
   
   uint32 i = 0; 
   
   for ( set<InputSource*>::const_iterator it = xmlBuffs.begin();
         it != xmlBuffs.end(); ++it ) {
      
      bool ok = true;
      try {
         m_xmlParser.parse( **it );
      } catch ( const XMLException& e ) {
         mc2log << error
                << "XMLParserHelper::parseAndSort an XMLerror occured "
                << "during parsing of initialize request: "
                << e.getMessage() << " line "
                << e.getSrcLine() << endl;
         ok = false;
      } catch( const SAXParseException& e) {
         mc2log << error
                << "XMLParserHelper::parseAndSort an SAXerror occured "
                << "during parsing of initialize request: "
                << e.getMessage() << ", "
                << "line " << e.getLineNumber() << ", column "
                << e.getColumnNumber() << endl;
         ok = false;
      } catch ( ... ){
          mc2log << error 
                 << "XMLParserHelper::parseAndSort an exception occured "
                 << endl;
      }
      
      if ( !ok ) {
         mc2log << fatal << "XMLParserHelper::parseAndSort "
                << "initialization of xml parser failed, quiting." << endl;
         return false;
      }
      
      m_xmlDocuments[ i ] = m_xmlParser.getDocument();
      mc2dbg << "Added xmlDocument from parser, i=" << i << endl;
      DOMNode* child = m_xmlDocuments[ i ]->getFirstChild();      
      bool foundTopTag = false;
      while ( (child != NULL) && (! foundTopTag) ) {
         if ( ( child->getNodeType() == DOMNode::ELEMENT_NODE ) &&
              ( XMLString::equals( child->getNodeName(), m_xmlTopTag ) ) )
         {
            // Found the top tag.
            child = child->getFirstChild();
            foundTopTag = true;
         } else {
            child = child->getNextSibling();
         }
      }

      if ( ! foundTopTag ) {
         // Not the type of xml file we want it to be.
         // Exiting.
         mc2log << error << " XMLParserHelper::parseAndSort Not a valid "
                << m_xmlTopTag
                << " xml-file. Don't know what to do. Exiting method."
                << endl;
         delete[] m_xmlDocuments;
         m_xmlDocuments = NULL;
         return (false);
      }
      
      // -----------------------------------------------------------------
      // Store interesting xml nodes.
      while ( child != NULL ) {
         switch ( child->getNodeType() ) {
            
            case DOMNode::ELEMENT_NODE :
               if ( m_toSortByOrder.find( child->getNodeName() ) != 
                    m_toSortByOrder.end() ) {
                  m_xmlNodesByOrder.push_back( child );
                  mc2dbg4 << "Adding " << child->getNodeName()
                          << " to xmlNodesByOrder." << endl;
               }
               
               if ( m_toSortByTag.find( child->getNodeName() ) != 
                    m_toSortByTag.end() ) {
                  m_xmlNodesByTag.insert( 
                        make_pair( child->getNodeName(), child ) );
                  mc2dbg4 << "Adding " << child->getNodeName()
                          << " to xmlNodesByTag." << endl;
               }
               break;
            
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
               
            default:
               mc2log << warn << "XMLParserHelper::parseAndSort odd "
                      "node type : \""
                      << child->getNodeName() 
                      << "\" type " << child->getNodeType()<< endl;
               break;
         }
         child = child->getNextSibling();
      }
      i++;
   }   

   return true;
}

#endif // USE_XML
