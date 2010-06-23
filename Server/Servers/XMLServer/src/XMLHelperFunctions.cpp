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
#include "XMLParserThread.h"


#ifdef USE_XML
#include "CoordinateRequest.h"

#include "Name.h"
#include "NameCollection.h"
#include "UserData.h"
#include "GfxFeatureMap.h"
#include "SearchReplyPacket.h"
#include "StringConversion.h"
#include "STLStringUtility.h"
#include "XMLAuthData.h"
#include "NamedServerLists.h"
#include "NamedServerList.h"
#include "NamedServerGroup.h"
#include "ClientSettings.h"
#include "XMLServerElements.h"
#include "XMLSearchUtility.h"
#include "LangUtility.h"
#include "ReverseGeocoding.h"

namespace {
/**
 * Gets the data of an name_node element.
 *
 * @param nameNode The name_node node to get data from.
 * @param lang The language of the name_node.
 * @param name Name of the name_node, user must delete this string!
 * @param errorCode If problem then this is set to error code.
 * @param errorMessage If problem then this is set to error message.
 * @return True if nameNode really is a valid name_node node,
 *         if false then errorCode and errorMessage is filled with
 *         reason why it isn't a valid name_node node.
 */
bool 
getNameNodeData( DOMNode* nameNode, 
                 StringTable::languageCode& lang, char*& name,
                 MC2String& errorCode, 
                 MC2String& errorMessage );

/**
 * Converts a ReverseGeocoding::LookupStatus to a status code
 * for the XML protocol.
 */
MC2String toXMLErrorCode( ReverseGeocoding::LookupStatus lookupStatus ) {

   switch ( lookupStatus ) {
      case ReverseGeocoding::LOOKUP_OK:
         return "0";
      case ReverseGeocoding::LOOKUP_FAILED:
         return "-1";
      case ReverseGeocoding::OUTSIDE_ALLOWED_AREA:
         return "-5";
      case ReverseGeocoding::OUTSIDE_MAP_COVERAGE:
         return "-4";
      case ReverseGeocoding::TIMEOUT_ERROR:
         return "-3";
      default:
         return "-1";
   }
}

}

bool 
XMLParserThread::readMapSymbolList( DOMNode* cur, GfxFeatureMap* map ) {
   bool ok = true;

   // Get children
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "map_symbol_item" ) ) 
            {
               ok = readAndAddMapSymbolItem( child, map );
            } else {
               mc2log << warn << "XMLParserThread::readMapSymbolList "
                         "odd Element in map_symbol_item_list element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::readMapSymbolList"
                   " odd node type in map_symbol_item_list element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType()<< endl;
            break;
      }
      child = child->getNextSibling();
   }

   return ok;
}


bool
XMLParserThread::readAndAddMapSymbolItem( DOMNode* cur, 
                                          GfxFeatureMap* map ) 
{
   bool ok = true;
   MC2String errorCode;
   MC2String errorMessage;
   
   char* href = NULL;
   char* name = NULL;
   int32 lat = MAX_INT32;
   int32 lon = MAX_INT32;
   uint16 angle = MAX_UINT16;
   
   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
         
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(), "href" ) ) {
         href = StringUtility::newStrDup( tmpStr );
      } else {
         mc2log << warn << "XMLParserThread::readAndAddMapSymbolItem "
                   "unknown attribute for map_symbol_item  element "
                << "Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() 
                << " Value " << tmpStr << endl;
      }
      delete [] tmpStr;
   }

   // Get children
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "position_item" ) ) 
            {
               ok = XMLCommonElements::getPositionItemData( 
                  child, lat, lon, angle, errorCode, errorMessage );
            } else if ( XMLString::equals( child->getNodeName(), "name" ) )
            {
               name = XMLUtility::getChildTextValue( child );
            } else {
               mc2log << warn << "XMLParserThread::"
                         "readAndAddMapSymbolItem "
                         "odd Element in map_symbol_item element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::readAndAddMapSymbolItem"
                   " odd node type in map_symbol_item element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType()<< endl;
            break;
      }
      child = child->getNextSibling();
   }
   
   if ( ok && lat != MAX_INT32 && href != NULL && name != NULL ) {
      GfxSymbolFeature::symbols symbolType = GfxSymbolFeature::PIN;
      if ( href[ 0 ] != '\0' ) {
         symbolType = GfxSymbolFeature::USER_DEFINED;
      }
      GfxSymbolFeature* feat = new GfxSymbolFeature( GfxFeature::SYMBOL,
                                                     name,
                                                     symbolType,
                                                     href );
      feat->addNewPolygon( true, 1 );
      feat->addCoordinateToLast( lat, lon );
      map->addFeature( feat );
   } else {
      mc2log << warn << "XMLParserThread::readAndAddMapSymbolItem"
                " not all data ok to make map_symbol_item" << endl;
      ok = false;
   }

   delete [] href;
   delete [] name;

   return ok;
}


bool 
XMLParserThread::checkUserAccess( UserUser* user, const char* service,
                                  bool checkTime, 
                                  UserEnums::userRightLevel levelmask )
{
   bool authorized = false;

   // XXX: Only rights when users have been converted.
   if ( user->getNbrOfType(UserConstants::TYPE_RIGHT ) > 0 ) {
      UserEnums::userRightService serv = getRightService( service );
      authorized = checkAccessToService( user, serv, levelmask,
                                         MAX_UINT32/*regionID*/, 
                                         checkTime );
   } else {
   if ( StringUtility::strcasecmp( service, "HTML" ) == 0 ) {
      // As of MyWf 2.0 no MYWAYFINDER right is needed
      authorized = true;
   } else if ( StringUtility::strcasecmp( service, "WAP" ) == 0 ) {
      // As of MyWf 2.0 no MYWAYFINDER right is needed
      authorized = true;
   } else {
      authorized = user->getExternalXmlService();
   } 
   if ( authorized ) {
      mc2dbg2 << "XMLParserThread::checkUserAccess "
                 "User accepted. " << "UserName " 
              << user->getLogonID() << endl;
   } else {
      mc2log << warn <<  "XMLParserThread::checkUserAccess " 
                "User has no access to service " << service
             << " UserName " << user->getLogonID() << endl;
   }
   } // End else old way

   return authorized;
}

VanillaMatch* 
XMLParserThread::getVanillaMatchFromPosition( 
   const MC2Coordinate& pos,
   uint16 angle,
   MC2String& errorCode,
   MC2String& errorMessage,
   LangTypes::language_t language,
   bool locationNameOnlyCountryCity )
{
   // Get allowed maps
   set< uint32 >* allowedMaps = NULL;
   UserUser* user = m_user->getUser();
   uint32 now = TimeUtility::getRealTime();
   if ( !getMapIdsForUserRegionAccess( user, allowedMaps, now,
                                       m_authData->urmask ) ) {
      // Problem
      errorCode = "-1";
      errorMessage.append( "Failed to get allowed area for user." );
      return NULL;
   }
   
   // Do the reverse geocoding
   ReverseGeocoding::LookupResult result = 
      ReverseGeocoding::lookup( 
         pos, 
         angle, 
         ItemTypes::getLanguageTypeAsLanguageCode( language ),
         allowedMaps, 
         this );

   if ( result.m_status != ReverseGeocoding::LOOKUP_OK ) {
      errorCode = ::toXMLErrorCode( result.m_status );
      errorMessage = result.m_errorString;
      return NULL;
   }

   MC2String location;
   vector<MC2String> strings;
   bool removeEmptyStringsInLocatioName = true;
   if ( locationNameOnlyCountryCity ) {
      strings.push_back( result.m_country );
      strings.push_back( result.m_city );
      removeEmptyStringsInLocatioName = false;
   } else {
      strings.push_back( result.m_country );
      strings.push_back( result.m_municipal );
      strings.push_back( result.m_city );
      strings.push_back( result.m_district );
   }
   location = STLStringUtility::join( strings,
                                      removeEmptyStringsInLocatioName,
                                      ", " );


   // Create VanillaMatch
   auto_ptr<VanillaStreetMatch>
      match( new VanillaStreetMatch( IDPair_t(result.m_mapID,
                                              result.m_itemID ),
                                     result.m_street.c_str(),
                                     location.c_str(),
                                     result.m_streetSegmentOffset,
                                     0 ) );


   VanillaCityPartMatch
      cityPartRegion( result.
                      m_district.c_str(),
                      0, // nameInfo
                      result.m_mapID, // mapID
                      0, // itemID
                      SearchMatchPoints(),
                      0, // source
                      "", // alphaSortingName
                      0, // location
                      0, // restrictions
                      ItemTypes::cityPartItem, // type
                      0  // itemSubType
                      );

   VanillaBuiltUpAreaMatch
      cityRegion( result.m_city.c_str(),
                  0, // nameInfo
                  result.m_mapID, // mapID
                  0, // itemID
                  SearchMatchPoints(),
                  0, // source
                  "", // alphaSortingName
                  0, // location
                  0, // restrictions
                  ItemTypes::builtUpAreaItem, // type
                  0  // itemSubType
                  );

   VanillaMunicipalMatch
      municipalRegion( result.m_municipal.c_str(),
                       0, // nameInfo
                       result.m_mapID, // mapID
                       0, // itemID
                       SearchMatchPoints(),
                       0, // source
                       "", // alphaSortingName
                       0, // location
                       0, // restrictions
                       ItemTypes::municipalItem, // type
                       0 // itemSubType
                       );

   // create country area, and if requested; include valid top region
   VanillaCountryMatch countryRegion( result.m_country.c_str(),
                                      result.m_topRegionID );

   // Must clone regions here, to get the "name" allocation correctly
   // in the VanillaRegionMatch

   // Adding municipal to city and city to city part
   // and then cloning city part to the original match actually clones the
   // other ones too, which causes more allocation than needed.
   // By doing it this way instead we reduce the cloning processes.

   VanillaRegionMatch* municipalClone =
      static_cast< VanillaRegionMatch* >( municipalRegion.clone() );
   VanillaRegionMatch* cityClone =
      static_cast< VanillaRegionMatch* >( cityRegion.clone() );
   VanillaRegionMatch* cityPartClone =
      static_cast< VanillaRegionMatch* >( cityPartRegion.clone() );
   VanillaRegionMatch* countryClone =
      static_cast< VanillaRegionMatch* >( countryRegion.clone() );

   municipalClone->addRegion( countryClone, true );
   cityClone->addRegion( municipalClone, true );
   cityPartClone->addRegion( cityClone, true );
   match->addRegion( cityPartClone, true );


   return match.release();
}


VanillaMatch* 
XMLParserThread::getVanillaMatchFromPositionItem( 
   DOMNode* positionItem,
   MC2String& errorCode,
   MC2String& errorMessage,
   LangTypes::language_t language,
   bool locationNameOnlyCountryCity )
{
   VanillaMatch* match = NULL;

   // Get lat, lon from positionItem
   MC2Coordinate pos;
   uint16 angle = MAX_UINT16;
   if ( XMLCommonElements::getPositionItemData( 
           positionItem, pos.lat, pos.lon, angle, errorCode, errorMessage ) ) {
      // Get closest item 
      match = getVanillaMatchFromPosition( pos, angle,
                                           errorCode, errorMessage, language,
                                           locationNameOnlyCountryCity );
   } else {
      mc2log << warn << "XMLParserThread::getVanillaMatchFromPositionItem "
             << "getPositionItemData returned false" << endl;
   }

   return match;
}


bool 
XMLParserThread::readImageSettings( 
   DOMNode* cur, struct MapSettingsTypes::ImageSettings& imageSettings )
{
   if ( XMLString::equals( cur->getNodeName(), "image_settings" ) ) {
      bool ok = true;
      // Attributes
      DOMNamedNodeMap* attributes = cur->getAttributes();
      DOMNode* attribute;

      for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
         attribute = attributes->item( i );
      
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         if ( XMLString::equals( attribute->getNodeName(),
                                 "image_show_street_main" ) )
         {
            imageSettings.m_image_show_street_main = 
               StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "image_show_street_first" ) )
         {
            imageSettings.m_image_show_street_first = 
               StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "image_show_street_second" ) )
         {
            imageSettings.m_image_show_street_second = 
               StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "image_show_street_third" ) ) 
         {
            imageSettings.m_image_show_street_third = 
               StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "image_show_street_fourth" ) )
         {
            imageSettings.m_image_show_street_fourth = 
               StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "image_show_builtup_area" ) )
         {
            imageSettings.m_image_show_builtup_area = 
               StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "image_show_park" ) )
         {
            imageSettings.m_image_show_park = 
               StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "image_show_forest" ) )
         {
            imageSettings.m_image_show_forest = 
               StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "image_show_building" ) )
         {
            imageSettings.m_image_show_building = 
               StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "image_show_water" ) )
         {
            imageSettings.m_image_show_water = 
               StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "image_show_island" ) )
         {
            imageSettings.m_image_show_island = 
               StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "image_show_pedestrianarea" ) )
         {
            imageSettings.m_image_show_pedestrianarea = 
               StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "image_show_aircraftroad" ) )
         {
            imageSettings.m_image_show_aircraftroad = 
               StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "image_show_land" ) )
         {
            imageSettings.m_image_show_land = 
               StringUtility::checkBoolean( tmpStr );
         } else {
            mc2log << warn << "XMLParserThread::readImageSettings "
                   << "unknown attribute for image_settings element "
                   << "Name " << attribute->getNodeName()
                   << "Type " << attribute->getNodeType() << endl;
         }
         delete [] tmpStr;
      }
      
      return ok;
   } else {
      return false;
   }
}

namespace {
/**
 * Gets the data from a top_region node.
 *
 * @param topRegion The top_region node to get data from.
 * @param id The ID of the top region.
 * @param type The type of top region.
 * @param name Name of the top region, user must delete this string!
 * @param lang The language of the name.
 * @param bbox The boundingbox of the top region.
 * @param errorCode If problem then this is set to error code.
 * @param errorMessage If problem then this is set to error message.
 * @return True if topRegion really is a valid top_region node,
 *         if false then errorCode and errorMessage is filled with
 *         reason why it isn't a valid top_region node.
 */
bool 
getTopRegionData( DOMNode* topRegion, 
                  uint32& id, TopRegionMatch::topRegion_t& type,
                  char*& name, StringTable::languageCode& lang,
                  MC2BoundingBox& bbox,
                  MC2String& errorCode, MC2String& errorMessage )  {
   if ( XMLString::equals( topRegion->getNodeName(), "top_region" ) ) {
      bool ok = true;
      // Get attributes
      DOMNamedNodeMap* attributes = topRegion->getAttributes();
      DOMNode* attribute;

      for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
         attribute = attributes->item( i );
         
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );

         if ( XMLString::equals( attribute->getNodeName(),
                                 "top_region_type" ) ) 
         {
            type = XMLCommonEntities::stringToTopRegionType( tmpStr );
            if ( type == TopRegionMatch::nbr_topregion_t ) {
               ok = false;
               errorCode = "-1";
               errorMessage = "Unknown top_region_type.";
            }
         } else {
            mc2log << warn << "XMLParserThread::getTopRegionData "
                   "unknown attribute: Name " << attribute->getNodeName()
                   << " Type " << attribute->getNodeType() << endl;
         }
         delete [] tmpStr;
      }

      // Get children
      DOMNode* child = topRegion->getFirstChild();
   
      while ( child != NULL ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(),
                                       "name_node" ) ) 
               {
                  ok = ::getNameNodeData( child, lang, name, 
                                          errorCode, errorMessage );
               } else if ( XMLString::equals( child->getNodeName(),
                                              "top_region_id" ) )
               {
                  char* tmpStr = XMLUtility::getChildTextValue( child );
                  if ( sscanf( tmpStr, "%u", &id ) != 1 )
                  {
                     ok = false;
                     mc2log << warn 
                            << "XMLParserThread::getTopRegionData "
                               "Bad top_region_id "
                            << tmpStr << endl;
                     errorCode = "-1";
                     errorMessage = "Bad top_region_id in top_region.";
                  }
                  delete [] tmpStr;
               } else if ( XMLString::equals( child->getNodeName(),
                                              "boundingbox" ) ) 
               {
                  if ( !XMLCommonElements::readBoundingbox( 
                           child, bbox, errorCode, errorMessage ) ) 
                  {
                     ok = false;
                     mc2log << warn 
                            << "XMLParserThread::getTopRegionData "
                               "Bad boundingbox." << endl;
                     errorMessage.insert( 
                        0, "Bad boundingbox in top_region: ");
                  }
               } else {
                  mc2log << warn << "XMLParserThread::getTopRegionData "
                            "odd Element in top_region element: " 
                         << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            default:
               mc2log << warn << "XMLParserThread::getTopRegionData odd "
                         "node type in top_region element: " 
                      << child->getNodeName() 
                      << " type " << child->getNodeType()<< endl;
               break;
         }
         child = child->getNextSibling();
      }
      return ok;
   } else {
      mc2log << warn << "XMLParserThread::getTopRegionData not "
             << "top_region" << endl;
      errorCode = "-1";
      errorMessage = "Not top_region where expected got ";
      char* nodeName = XMLUtility::transcodefromucs( 
         topRegion->getNodeName() );
      errorMessage.append( nodeName );
      errorMessage.append( "." );
      delete [] nodeName;
      return false;
   }  
}

}

TopRegionMatch* 
XMLParserThread::getTopRegionMatchFromTopRegion( 
   DOMNode* topregion, 
   MC2String& errorCode, MC2String& errorMessage ) const
{
   uint32 id = 0;
   TopRegionMatch::topRegion_t type = TopRegionMatch::nbr_topregion_t;
   char* name = NULL;
   StringTable::languageCode lang = StringTable::ENGLISH;
   MC2BoundingBox bbox;
   TopRegionMatch* match = NULL;

   if ( getTopRegionData( topregion, id, type, name, lang, bbox,
                          errorCode, errorMessage ) )
   {
      match = new TopRegionMatch( id, type );
      //match->addName( name, lang );
      match->setBoundingBox( bbox );
   }

   delete [] name;
   return match;
}

namespace {

bool 
getNameNodeData( DOMNode* nameNode, 
                 StringTable::languageCode& lang, char*& name,
                 MC2String& errorCode, 
                 MC2String& errorMessage ) {

   if ( XMLString::equals( nameNode->getNodeName(), "name_node" ) ) {
      bool ok = true;
      // Get attributes
      DOMNamedNodeMap* attributes = nameNode->getAttributes();
      DOMNode* attribute;

      for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
         attribute = attributes->item( i );
         
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         if ( XMLString::equals( attribute->getNodeName(), "language" ) ) {
            lang = LangUtility::getLanguageCode( tmpStr );
            if ( lang == StringTable::SMSISH_ENG ) {
               mc2log << warn << "XMLParserThread::getNameNodeData "
                      << "bad language in name_node: " << tmpStr
                      << endl;
               ok = false;
               errorCode = "-1";
               errorMessage = "Unknown language in name_node's language "
                  "attribute->";
            }
         } else {         
            mc2log << warn << "XMLParserThread::getNameNodeData "
                   "unknown attribute: Name " << attribute->getNodeName()
                   << " Type " << attribute->getNodeType() << endl;
         }
         delete [] tmpStr;
      }

      // Get text
      name = XMLUtility::getChildTextValue( nameNode );

      return ok;
   } else {
      mc2log << warn << "XMLParserThread::getNameNodeData not name_node"
             << endl;
      errorCode = "-1";
      errorMessage = "Not name_node where expected got ";
      char* nodeName = XMLUtility::transcodefromucs( 
         nameNode->getNodeName() );
      errorMessage.append( nodeName );
      errorMessage.append( "." );
      delete [] nodeName;
      return false;
   }
}
}

bool
XMLParserThread::needsNewServerList() const {
   bool res = false;
   if ( m_authData->server_list_crcSet ) {
      const MC2String* serverListName = 
         &m_authData->clientSetting->getServerListName();
      if ( *serverListName == "" ) {
         serverListName = &NamedServerLists::DefaultGroup;
      }
      const NamedServerList* serverList = 
         m_group->getServerList( *serverListName, 
                                 NamedServerLists::XmlServerType );
      if ( serverList != NULL ) {
         if ( m_authData->server_list_crc != 
              serverList->getServerListCRC() )
         {
            res = true;
         }
      }
   }
   return res;
}


#endif // USE_XML

const NamedServerList* 
XMLParserThread::getServerList( const MC2String& fixedServerListName ) const {
   MC2String serverListName = fixedServerListName; 
   if ( serverListName.empty() ) {
      // try client settings
      serverListName = m_authData->clientSetting->getServerListName();
      // fallback to default group
      if ( serverListName.empty() ) {
         serverListName = NamedServerLists::DefaultGroup;
      }
   }

   return m_group->getServerList( serverListName,
                                  NamedServerLists::XmlServerType );
}
