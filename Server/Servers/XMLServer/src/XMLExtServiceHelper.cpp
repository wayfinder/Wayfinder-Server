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

#include "XMLExtServiceHelper.h"
#include "ParserThread.h"

#include "ExternalSearchRequestData.h"
#include "ExternalSearchRequest.h"
#include "ExternalSearchHelper.h"
#include "XMLParserThread.h"
#include "ItemTypes.h"
#include "XMLServerErrorMsg.h"
#include "XMLTool.h"
#include "XMLSearchUtility.h"
#include "XMLServerElements.h"
#include "SearchParserHandler.h"

#include <sstream>


static long getLongOrThrow( DOMNode* attribute,
                            DOMNode* parent = NULL)
                                            throw ( XMLServerErrorMsg ) {
   MC2String tmpStr(
      XMLUtility::transcodefrom( attribute->getNodeValue() ) );
   char* tmpPtr = NULL;
   long tmp = strtol( tmpStr.c_str(), &tmpPtr, 10 );
   if ( tmpPtr != tmpStr ) {
      // A OK.
      return tmp;
   } else {
      // Create nice error
      MC2String attribName =
         XMLUtility::transcodefrom(attribute->getNodeName());
      
      if ( parent == NULL ) {
         DOMAttr* dom_attr = dynamic_cast<DOMAttr*>(attribute);
         if ( dom_attr ) {
            parent = dom_attr->getOwnerElement();
         }
      }

      MC2String parentName = parent ?
         XMLUtility::transcodefrom(parent->getNodeName()):
         "";
      
      MC2String msg = MC2String() + "Node \"" + parentName
         + "\" contains attribute \""
         + attribName + "\" = \"" 
         + tmpStr
         + "\" which is not a number";
      mc2log << warn << __FUNCTION__ << " " << msg << endl;
      throw XMLServerErrorMsg( "-1",  msg );
                               
   }
}

static long getRequiredLongOrThrow( DOMNode* node, const char* attribName )
   throw ( XMLServerErrorMsg ) {
   DOMNamedNodeMap* attribs = node->getAttributes();
   DOMNode* attrib_to_find = attribs->getNamedItem( X( attribName ) );
   if ( attrib_to_find == NULL ) {
      MC2String nodeName =
         XMLUtility::transcodefrom(node->getNodeName());
      MC2String msg = MC2String("Node \"") + nodeName
         + "\" requires attribute \"" + attribName + '"';
      mc2log << warn << __FUNCTION__ << " " << msg << endl;
      throw XMLServerErrorMsg( "-1",  msg );
   } else {
      // Try to get the value.
      return getLongOrThrow( attrib_to_find, node );
   }                               
}

XMLExtServiceHelper::XMLExtServiceHelper(XMLParserThread* thread,
                                         ParserThreadGroup* group) :
      ParserHandler( thread, group )
{
   m_xmlParserThread = thread;
}

/**
 * Class for holing the search fields properties.
 */
class SearchField {
public:
   SearchField(MC2String name, ExternalSearchConsts::search_fields_t id, MC2String type, uint32 required) :
         m_name(name), m_id(id), 
         m_type(type), m_required(required) {};

   MC2String m_name;
   ExternalSearchConsts::search_fields_t m_id;
   MC2String m_type;
   uint32 m_required;
};

/// Vector for holding the available search fields
typedef std::vector< SearchField > FieldVec;


void
XMLExtServiceHelper::appendExternalSearchDesc( DOMNode* out,
                                               DOMDocument* reply,
                                               DOMNode* cur,
                                               const MC2String& crcIn,
                                               const LangType& lang,
                                               int indentLevel,
                                               bool indent )
{
   DOMElement* ext_service_list =
      reply->createElement( X( "ext_services_reply" ) );
   out->appendChild( ext_service_list );

   DOMNode* transaction_id =
      cur->getAttributes()->getNamedItem( X("transaction_id") );
   // Transaction id should always be there when dtd is on.
   if ( transaction_id ) {
      ext_service_list->setAttribute( transaction_id->getNodeName(),
                                      transaction_id->getNodeValue() );
   }

   
   // FIXEME: Move the transacion id 
   
   // Add CRC
   bool crcOK = false;
   {
      MC2String intCrc = m_thread->getSearchHandler().getSearchHitTypesCRC(lang);
      ext_service_list->setAttribute( X("crc"), X( intCrc.c_str() ) );
      if ( crcIn == intCrc ) {
         ext_service_list->appendChild( reply->createElement(
                                           X( "ext_services_crc_ok" ) ) );
         crcOK = true;
      }
   }
   
   bool newClient;
   XMLTool::getAttrib<bool>( newClient, "new_client", cur, false );

   CompactSearchHitTypeVector availableProviders;

   CompactSearchHitTypeVector headings =
      m_thread->getSearchHandler().getSearchHitTypes(lang, newClient);
   
   for ( CompactSearchHitTypeVector::iterator it = headings.begin();
         it != headings.end(); it++ ) {
      if( it->m_round == 1 ) {
         availableProviders.push_back( *it );
      }
   }
   
   ext_service_list->setAttribute( X("nbr_services" ), XInt32( availableProviders.size() ) );
   
   if ( crcOK ) {
      // Now indent if necessary.
      if ( indent ) {
         XMLUtility::indentPiece( *ext_service_list, indentLevel );
      }
      return;
   }


   for ( CompactSearchHitTypeVector::const_iterator it = availableProviders.begin();
         it != availableProviders.end();
         ++it ) {
      
      // Create ext_service element for each service.
      DOMElement* ext_service = reply->createElement( X("ext_service" ) );
      ext_service_list->appendChild( ext_service );
      ext_service->setAttribute( X("type"), X("search") );
      DOMElement* name = reply->createElement( X("name") );
      ext_service->appendChild( name );
      name->appendChild( reply->createTextNode( X( it->m_name ) ) );
      ext_service->setAttribute( X("service_id"), 
                                 XInt32( it->m_serviceID ) );
      
      // Add the background colour as an attribute
      ext_service->setAttribute( X("background_colour"),
                                 XHex32( getColour( it->m_serviceID  ) ) );
      
      // Add the help text - currently empty.
      ext_service->appendChild(
         reply->createElement( X("ext_service_help" ) ) );

      // Add the icons.
      DOMElement* icons_el = reply->createElement( X( "icons" ) );

      uint32 nbrIcons = 0;
      
      ext_service->appendChild( icons_el );
      
      // if we dont have any icons we need to add two empty since the client
      // side cant seem to handle xml correctly
      if ( ! newClient && nbrIcons < 2 ) {
         using namespace XMLTool;
         for ( uint32 i = 0; i < 2 - nbrIcons; ++i ) {
            DOMElement* icon = addNode( icons_el, "icon" );
            addAttrib( icon, "name", MC2String("") );
            addAttrib( icon, "xsize", (uint32)0 );
            addAttrib( icon, "ysize", (uint32)0 );
            addAttrib( icon, "client_type", MC2String("mywf") );
            ++nbrIcons;
         }
      }


      XMLTool::addAttrib( icons_el, "nbr_icons", nbrIcons );

      FieldVec fields;
      // country is always required
      fields.push_back( SearchField( ExternalSearchHelper::getFieldName( ExternalSearchConsts::
                                                                         country, lang ),
                                     ExternalSearchConsts::country,
                                     "choice", 0x1 ) );


      if ( it->m_providerType == CompactSearchHitType::yellow_pages ) {
         // Search fields for providers of yellow pages type
         fields.push_back( SearchField( ExternalSearchHelper::getFieldName( ExternalSearchConsts::
                                                                            company_or_search_word, lang ),
                                        ExternalSearchConsts::company_or_search_word,
                                        "string",0x1 ) );
         fields.push_back( SearchField( ExternalSearchHelper::getFieldName( ExternalSearchConsts::
                                                                            city_or_area, lang ),
                                        ExternalSearchConsts::city_or_area,
                                        "string", 0 ) );
         fields.push_back( SearchField( ExternalSearchHelper::getFieldName( ExternalSearchConsts::
                                                                            category, lang ),
                                        ExternalSearchConsts::category,
                                        "string", 0 ) );

         

      } else if ( it->m_providerType == CompactSearchHitType::white_pages ) {
         // Search fields for providers of white pages type
         fields.push_back( SearchField( ExternalSearchHelper::getFieldName( ExternalSearchConsts::
                                                                            name_or_phone, lang ),
                                        ExternalSearchConsts::name_or_phone,
                                        "string",0x1 ) );
         fields.push_back( SearchField( ExternalSearchHelper::getFieldName( ExternalSearchConsts::
                                                                            address_or_city, lang ),
                                        ExternalSearchConsts::address_or_city,
                                        "string", 0 ) );
      }

      for ( FieldVec::iterator it2 = fields.begin();
            it2 != fields.end(); ++it2 ) {
         DOMElement* field = reply->createElement( X("field") );
         ext_service->appendChild( field );
         DOMElement* field_name = reply->createElement( X("field_name") );
         field->appendChild( field_name );
         field_name->appendChild( reply->createTextNode( X( it2->m_name ) ) );
         char tmp_str[128];
         sprintf( tmp_str, "%u", it2->m_id );
         field->setAttribute( X("id"), X( tmp_str) );
         sprintf( tmp_str, "%X", it2->m_required );
         field->setAttribute( X("req"), X( tmp_str ) );
         field->setAttribute( X("type" ), X( it2->m_type ) );
         if ( it2->m_id == 1 ) {
            // Country is choice field. Add the country as choice
            field->setAttribute( X("nbr_choices"), X( "1" ) );
            
            StringTable::countryCode country = ExternalSearchHelper::topRegionToCountryCode( it->m_topRegionID, lang );
            
            DOMElement* field_option =
               reply->createElement( X("field_option") );
            field->appendChild( field_option );
            sprintf( tmp_str, "%u", country );
            field_option->setAttribute( X("id"), X(tmp_str) );
            DOMElement* field_option_name =
               reply->createElement( X("field_option_name" ) );
            field_option->appendChild( field_option_name );
            
            
            MC2String strCountry = StringUtility::makeFirstCapital( StringTable::getString(
                                                                       StringTable::getCountryStringCode(country), lang) ) ;
            field_option_name->appendChild( reply->createTextNode( X( strCountry ) ) );
         }
      }
      
   } // for all descs
   
   // Now indent if necessary.
   if ( indent ) {
      XMLUtility::indentPiece( *ext_service_list, indentLevel );
   }
}


bool
XMLExtServiceHelper::parseExtServicesReq( DOMNode* cur, DOMNode* out,
                                          DOMDocument* reply, 
                                          bool indent,
                                          const HttpHeader& inHeaders )
{
   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();

   // Language of the request.
   LangTypes::language_t lang = LangTypes::english;
   // Semi-optional crc
   MC2String crc;
   
   for( int i = 0, n = attributes->getLength(); i < n; ++i ) {
      DOMNode* attrib = attributes->item( i );
      if ( XMLString::equals( attrib->getNodeName(), "language" ) ) {
         lang = m_xmlParserThread->getStringAsLanguage( 
            XMLUtility::getChildTextStr( *attrib ).c_str() );
      } else if ( XMLString::equals( attrib->getNodeName(), "crc" ) ) {
         crc = XMLUtility::getChildTextStr( *attrib );
      }
   }

   // Make the answer.
   appendExternalSearchDesc( out,
                             reply,
                             cur,
                             crc,
                             lang,
                             1,
                             indent );

   return true;
}
                                          
bool
XMLExtServiceHelper::parseExtSearchReq( DOMNode* cur, DOMNode* out,
                                        DOMDocument* reply, 
                                        bool indent,
                                        const HttpHeader& inHeaders )
{
   DOMElement* search_reply = reply->createElement( X( "search_reply" ) );

   DOMNode* transaction_id =
      cur->getAttributes()->getNamedItem( X("transaction_id") );
   // Transaction id should always be there when dtd is on.
   if ( transaction_id ) {
      search_reply->setAttribute( transaction_id->getNodeName(),
                                  transaction_id->getNodeValue() );
   }
   out->appendChild( search_reply );
   // Transaction ID
   bool ok = true;
   try {
      ok = parseAndReplyToExtSearchReq( cur, search_reply,
                                        reply, indent, inHeaders );
   } catch ( XMLServerErrorMsg& err ) {      
      // Error has occured
      XMLServerUtility::
         appendStatusNodes( search_reply, reply, 1,
                            false, err.getCode().c_str(),
                            err.getMsg().c_str() );
      // Error handled
   }
   
   if ( indent ) {
      XMLUtility::indentPiece( *search_reply, 1 );
   }

   return ok;
}

bool
XMLExtServiceHelper::parseAndReplyToExtSearchReq( DOMNode* cur, DOMNode* out,
                                                  DOMDocument* reply, 
                                                  bool indent,
                                                  const HttpHeader& inHeaders )
   throw ( XMLServerErrorMsg )
{

   // Be careful! This method throws, which means stuff has to be cleaned
   // up.
   
// <ext_search_request language="english"
//                          search_item_starting_index="1"
//                          search_item_ending_index="10"
//                          service_id = "2"
//                          transaction_id = "xoxox">
//   <field_val id="1">3</field_val>
//   <field_val id="7">svend</field_val>
//   <field_val id="6">tromsö</field_val>
// </ext_search_request>

   
   // Get the attribs   
   DOMNamedNodeMap*  attributes = cur->getAttributes();

   // Needed stuff
   ExternalSearchRequestData::stringMap_t fields;
   LangType lang;
   uint32 extService = 0;

   // Default to MC2. Add paramter to request if needed only.
   XMLCommonEntities::coordinateType positionSystem =
      XMLCommonEntities::MC2;

   int search_item_starting_index = 0;
   int search_item_ending_index   = 25;
   
   for( int i = 0, n = attributes->getLength(); i < n; ++i ) {
      DOMNode* attrib = attributes->item( i );
      if ( XMLString::equals( attrib->getNodeName(), "language" ) ) {
         lang = m_xmlParserThread->getStringAsLanguage(
            XMLUtility::getChildTextStr( *attrib ).c_str() );
      } else if ( XMLString::equals( attrib->getNodeName(),
                                     "search_item_starting_index" ) ) {
         search_item_starting_index = getLongOrThrow( attrib );
      } else if ( XMLString::equals( attrib->getNodeName(),
                                     "search_item_ending_index" ) ) {
         search_item_ending_index = getLongOrThrow( attrib );
      } else if ( XMLString::equals( attrib->getNodeName(),
                                     "service_id" ) ) {
         extService = getLongOrThrow( attrib );
      } 
                                     
   }
   // Good. Now take the other stuff.

   for ( DOMNode* child = cur->getFirstChild();
         child != NULL;
         child = child->getNextSibling() ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE:
            if ( XMLString::equals( child->getNodeName(),
                                    "field_val" ) ) {
               pair<int, MC2String> cur_field;
               cur_field.first  = getRequiredLongOrThrow( child, "id" );
               cur_field.second = XMLUtility::getChildTextStr( *child );
               fields.insert( cur_field );
            } else {
               throw XMLServerErrorMsg( "-1",
                                        MC2String("Unexpected node in "
                                                  "external_search_request: ")
                                        + XMLUtility::transcodefrom(
                                           child->getNodeName() ));
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         default:
            mc2log << warn << "[XMLExtServiceHelper::"
                   << "parseAndReplyToExtSearchReq]: Odd node "
                   << MC2CITE( child->getNodeName() )
                   << " especially the type " << child->getNodeType()<< endl;
            break;
      }
   }

   // Check the field values
   CompactSearchHitTypeVector headings =
      m_thread->getSearchHandler().getSearchHitTypes(lang, 0);
   
   for ( CompactSearchHitTypeVector::iterator it = headings.begin();
         it != headings.end(); ++it ) {
      if ( it->m_serviceID == extService ) {
         ExternalSearchRequestData::stringMap_t::const_iterator findit;
         if( it->m_providerType == CompactSearchHitType::white_pages ) { 
            // Check fields required for white pages
            findit = fields.find( ExternalSearchConsts::name_or_phone );
         } else {
            // Check required fields yellow pages
            findit = fields.find( ExternalSearchConsts::company_or_search_word );
         }

         if( findit != fields.end() ) {
            // country is required for both wp and yp
            findit = fields.find( ExternalSearchConsts::country );
            if( findit != fields.end() ) {
               // Country provided. Check that it is valid
               // Try strtoul
               char* endPtr;
               uint32 val = strtoul( findit->second.c_str(), &endPtr, 0 );
               if ( *endPtr != '\0' || 
                    ( val != MAX_UINT32 && val != 
                      static_cast<uint32>( ExternalSearchHelper::topRegionToCountryCode( it->m_topRegionID, lang ) ) ) ) {
                  mc2dbg << "[XMLExtServiceHelper]: param " << findit->first
                         << " = " << findit->second << " is bad" << endl;
                  findit = fields.end();
               }
            }
         }
         
         if ( findit == fields.end() ) {
            throw XMLServerErrorMsg( "-1", "Fields filled in the wrong way" );
         }
         // Service found, break loop
         break;
      }
   }
   
   // Prepare for searching
   SearchRequestParameters params;
   params.setRequestedLang( lang );
   int nbrWanted = search_item_ending_index - search_item_starting_index + 1;
   ExternalSearchRequestData reqData( params,
                                      extService,
                                      fields,
                                      search_item_starting_index,
                                      nbrWanted );
   // Do the searching
   ExternalSearchRequest req ( m_thread->getNextRequestID(),
                               reqData );
   
   m_thread->putRequest( &req );

   if ( req.getStatus() != StringTable::OK ) {
      throw XMLServerErrorMsg( req.getStatus() );
   }
   
   XMLSearchUtility::appendSearchItemList( out, reply, 1,
                                            false, // indent off
                                            req,
                                            "", //currentLocation, 
                                            search_item_starting_index,
                                            search_item_ending_index,
                                            false,
                                            true, // positionSearchItems,
                                            positionSystem,
                                            params );
   
   return true;
}

uint32 
XMLExtServiceHelper::getColour(uint32 serviceId ) {
   uint32 col = 0;
   switch ( serviceId ) {
      case 1:
      case 2:
      case 3:
         col = 0xD2EBC9;
         break;
      case 4:
      case 5:
      case 6:
      case 7:
         col = 0xF6EDBB;
         break;
      case 8:
         col = 0xFFCC00;
         break;
      case 9:
      case 10:
         col = 0xFFFFFF;
         break;
      default:
         col = MAX_UINT32;
   }

   return col;
}
