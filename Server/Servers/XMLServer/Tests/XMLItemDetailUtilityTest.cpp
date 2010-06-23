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

#include "XMLItemDetailUtility.h"
#include "SearchMatch.h"
#include "XMLInit.h"
#include "XMLTool.h"
#include "ItemInfoEnums.h"
#include "ItemInfoEntry.h"

XMLTool::XMLInit xmlInit;

MC2_UNIT_TEST_FUNCTION( appendDetailItemTest ) {
   // create root node
   DOMImplementation* impl = 
      DOMImplementationRegistry::
      getDOMImplementation( X( "LS" ) );

   DOMDocument* reply = impl->createDocument( NULL,
                                              X( "isab-mc2" ),
                                              NULL );
   DOMElement* rootElem = reply->getDocumentElement();
   
   // create a search match
   std::auto_ptr<SearchMatch> sm(new SearchMatch(0, 0, IDPair_t(), 
  				 "appendDetailItem", "locName") );

    // Add some item infos
   SearchMatch::ItemInfoVector itemInfos;
   itemInfos.push_back( ItemInfoEntry("key:dont_show","val:dont_show", 
                                      ItemInfoEnums::dont_show));
   itemInfos.push_back( ItemInfoEntry("key:text","val:text", 
                                      ItemInfoEnums::text) );
   itemInfos.push_back( ItemInfoEntry("key:address","val:address", 
                                      ItemInfoEnums::vis_address) );
   itemInfos.push_back( ItemInfoEntry("key:full_address","val:full_address", 
                                      ItemInfoEnums::vis_full_address) );
   itemInfos.push_back( ItemInfoEntry("key:zip_code","val:zip_code", 
                                      ItemInfoEnums::vis_zip_code) );
   itemInfos.push_back( ItemInfoEntry("key:complete_zip","val:complete_zip", 
                                      ItemInfoEnums::vis_complete_zip) );
   itemInfos.push_back( ItemInfoEntry("key:zip_area","val:zip_area", 
                                      ItemInfoEnums::Vis_zip_area) );
   
   // Add the item infos to the SearchMatch
   sm->swapItemInfos( itemInfos );

   // make call to tested function
   XMLItemDetailUtility::appendDetailItem( rootElem,
                                           sm.get() );
   
   // make check
   MC2String type, name, value;
   DOMNode* node = rootElem->getFirstChild();
   MC2_TEST_CHECK( XMLString::equals( node->getNodeName(), "detail_item" ) );
   mc2log << "Node name: " << node->getNodeName() << endl;
   node = node->getFirstChild();
   while ( node != NULL ) {
      XMLTool::getAttrib( type, "detail_type", node);

      DOMNode* fields = node->getFirstChild();
      while ( fields != NULL ) {
         if ( XMLString::equals( fields->getNodeName(), "fieldName" ) )  {
            name = XMLTool::getNodeValue( fields );
         } else if ( XMLString::equals( fields->getNodeName(), "fieldValue" ) ) {
            value = XMLTool::getNodeValue( fields );
         }
         fields = fields->getNextSibling();
      }

      mc2log << "Node name: " << node->getNodeName() << " " << type
             << ", " << name << ", " << value << " " << endl;

      MC2_TEST_CHECK( !name.empty() );
      if ( name == "key:dont_show" ) {
         MC2_TEST_CHECK( type == "dont_show" );
         MC2_TEST_CHECK( value == "val:dont_show" );
      } else if ( name == "key:text" ) {
         MC2_TEST_CHECK( type == "text" );
         MC2_TEST_CHECK( value == "val:text" );
      } else if ( name == "key:address" ) {
         MC2_TEST_CHECK( type == "street_address" );
         MC2_TEST_CHECK( value == "val:address" );
      } else if ( name == "key:full_address" ) {
         MC2_TEST_CHECK( type == "full_address" );
         MC2_TEST_CHECK( value == "val:full_address" );
      } else if ( name == "key:zip_code" ) {
         MC2_TEST_CHECK( type == "text" );
         MC2_TEST_CHECK( value == "val:zip_code" );
      } else if ( name == "key:complete_zip" ) {
         MC2_TEST_CHECK( type == "text" );
         MC2_TEST_CHECK( value == "val:complete_zip" );
      } else if ( name == "key:zip_area" ) {
         MC2_TEST_CHECK( type == "text" );
         MC2_TEST_CHECK( value == "val:zip_area" );
      }
      
      node = node->getNextSibling();
   }
   
}
