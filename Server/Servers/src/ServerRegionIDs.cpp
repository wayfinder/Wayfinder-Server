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

#include "ServerRegionIDs.h"
#include "TopRegionRequest.h"
#include "STLStringUtility.h"

uint32 
ServerRegionIDs::addRegionIDsForReq( vector<uint32>& regionIDs, 
                                     uint32 regionGroupID, 
                                     const TopRegionRequest* topReg ) const
{
   regionGroupMap::const_iterator findIt = m_regionGroupMap.find( 
      regionGroupID );
   uint32 nbr = 0;
   if ( findIt != m_regionGroupMap.end() ) {
      for ( uint32 i = 0 ; i < (*findIt).second.first.size() ; ++i ) {
         if ( topReg->getTopRegionWithID( (*findIt).second.first[ i ] ) ) {
            regionIDs.push_back( (*findIt).second.first[ i ] );
            ++nbr;
         }
      }
   }

   return nbr;   
}

#ifdef USE_XML
//#include "XMLParserHelper.h"
#include <dom/DOM.hpp>
#include <parsers/XercesDOMParser.hpp>

//#include <util/PlatformUtils.hpp>
//#include <util/XMLUniDefs.hpp>
#include <framework/LocalFileInputSource.hpp>
#include <sax/SAXParseException.hpp>

#include "XMLUtility.h"

static uint32 parseRegionGroup( const DOMNode* node, 
                                NameCollection& c,
                         const char* name = "region_group" ) {
   uint32 id = MAX_UINT32;

   if ( XMLString::equals( node->getNodeName(), name ) ) {
      // Get attributes
      DOMNamedNodeMap* attributes = node->getAttributes();
      DOMNode* attribute;
      MC2String identName;

      for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
         attribute = attributes->item( i );
      
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         char* tmpPtr = NULL;
         if ( XMLString::equals( attribute->getNodeName(),
                                 "ident" ) ) 
         {
            identName = tmpStr;
         } else if ( XMLString::equals( attribute->getNodeName(),
                                 "id" ) ) 
         {
            id = strtoul( tmpStr, &tmpPtr, 10 );
            if ( tmpPtr == NULL || tmpPtr[ 0 ] != '\0' ) {
               mc2log << warn << "parseRegionGroup Problem parsing "
                      << "id not a valid number." << endl;
            }
         } else if ( XMLString::equals( attribute->getNodeName(),
                                 "mc2stringcodes" ) ) 
         {
            vector<MC2String> sc( STLStringUtility::explode( ",", tmpStr ) );
            vector<StringTableUTF8::stringCode> strc;
            for ( uint32 i = 0 ; i < sc.size() ; ++i ) {
               uint32 scid = strtoul( sc[ i ].c_str(), &tmpPtr, 0 );
               if ( tmpPtr == NULL || tmpPtr[ 0 ] != '\0' ) {
                  mc2log << warn << "parseRegionGroup Problem parsing "
                         << "mc2stringcodes entry not a valid number." << endl;
               }
               strc.push_back( StringTableUTF8::stringCode( scid ) );
            }
            for ( uint32 l = StringTableUTF8::ENGLISH ; 
                  l < StringTableUTF8::SMSISH_ENG ; ++l ) {
               MC2String s;
               for ( uint32 i = 0 ; i < strc.size() ; ++i ) {
                  if ( i != 0 && strc[ i ] != StringTableUTF8::COMMA ) {
                     s += " ";
                  }
                  s += StringTable::getString( 
                     strc[ i ], 
                     StringTableUTF8::languageCode( l ) );
               }
               mc2dbg4 << s << " (" << id << ") in " 
                       << LangTypes::getLanguageAsString( 
                          ItemTypes::getLanguageCodeAsLanguageType(
                             StringTableUTF8::languageCode( l ) ), true )
                       << endl;

               c.addName( new Name( 
                             s.c_str(), 
                             ItemTypes::getLanguageCodeAsLanguageType( 
                                StringTableUTF8::languageCode( l ) ) ) );
            }

            // ident = 
         } else {
            mc2log << warn << "parseRegionGroup "
                   << "unknown attribute for " << name << " element "
                   << " Name " << attribute->getNodeName()
                   << " Type " << attribute->getNodeType() << endl;
         }
         delete [] tmpStr;
      }
      // If no names use the ident name
      if ( c.getSize() == 0 ) {
         c.addName( new Name( identName.c_str(), LangTypes::english ) );
      }

      // No children
   } else {
      mc2log << error << "parseRegionGroup not " << MC2CITE( name ) << " " 
             << MC2CITE( node->getNodeName() ) << endl;
   }

   return id;
}


static uint32 parseRegion( const DOMNode* node, vector<uint32>& groups,
                           RegionIDs::mccMap& mccIdMap,
                           RegionIDs::isoNameMap& isoNameIdMap) {
   uint32 id = MAX_UINT32;

   if ( XMLString::equals( node->getNodeName(), "region" ) ) {
      // Get attributes
      DOMNamedNodeMap* attributes = node->getAttributes();
      DOMNode* attribute;

      vector< MC2String > mccVec;
      MC2String isoName;

      for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
         attribute = attributes->item( i );
      
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         char* tmpPtr = NULL;
         if ( XMLString::equals( attribute->getNodeName(),
                                 "ident" ) ) 
         {
            // Not used
         } else if ( XMLString::equals( attribute->getNodeName(),
                                 "id" ) ) 
         {
            id = strtoul( tmpStr, &tmpPtr, 10 );
            if ( tmpPtr == NULL || tmpPtr[ 0 ] != '\0' ) {
               mc2log << warn << "parseRegion Problem parsing "
                      << "id not a valid number." << endl;
               id = MAX_UINT32;
            }
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "mcc" ) ) {
            mccVec =  STLStringUtility::explode( ",", tmpStr ) ;
          } else if ( XMLString::equals( attribute->getNodeName(),
                                        "iso_name" ) ) {
            if( strlen( tmpStr ) > 0) {
               isoName = tmpStr;
            }
         } else {
            mc2log << warn << "parseRegion "
                   << "unknown attribute for region element "
                   << " Name " << attribute->getNodeName()
                   << " Type " << attribute->getNodeType() << endl;
         }
         delete [] tmpStr;
      }

      for(uint32 i = 0; i < mccVec.size(); i++) {
         // Add the MCC to the map
         uint32 mcc = STLStringUtility::strtoul( mccVec[ i ] );
         mccIdMap.insert( make_pair( mcc, id ) );
      }

      if ( isoName.length() > 0) {
         // Add iso name to map
          isoNameIdMap.insert( make_pair( isoName, id ) );
      }

      // Children (groups)
      DOMNode* child = node->getFirstChild();
      bool ok = true;
   
      while ( child != NULL && ok ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               // See if the element is a known type
               if ( XMLString::equals( child->getNodeName(), 
                                       "region_group_id" ) )
               {
                  // Get id and add to groups
                  NameCollection c;
                  uint32 gid = parseRegionGroup( child, c,
                                                 "region_group_id" );
                  if ( gid != MAX_UINT32 ) {
                     groups.push_back( gid );
                  } else {
                     mc2log << warn << "parseRegion failed parsing "
                            << "region_group_id." << endl;
                     ok = false;
                     id = MAX_UINT32;
                  }
               } else {
                  mc2log << warn << "parseRegion "
                         << "odd with Element in region, "
                         << "element: " << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            case DOMNode::TEXT_NODE :
               // Ignore stray texts
               break;
            default:
               mc2log << warn << "parseRegion odd "
                      "node type in region element: " 
                      << child->getNodeName() 
                      << " type " << child->getNodeType() << endl;
               break;
         }
         child = child->getNextSibling();
      } // End while child != NULL


   } else {
      mc2log << error << "parseRegion not region " 
             << MC2CITE( node->getNodeName() ) << endl;
   }

   return id;
}


static bool parseRegionGroupList( const DOMNode* node, 
                           RegionIDs::regionGroupMap& groupmap ) 
{
   bool ok = true;

   DOMNode* child = node->getFirstChild();
   vector<uint32> uint32Array;

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), 
                                    "region_group" ) ) 
               {
                  MC2String ident;
                  NameCollection c;
                  uint32 id = parseRegionGroup( child, c );
                  if ( id != MAX_UINT32 ) {
                     groupmap.insert( make_pair( id, 
                                                 make_pair( uint32Array,
                                                            c ) ) );
                  } else {
                     mc2log << fatal << "parseRegionGroupList failed "
                            << "parsing region_group." << endl;
                     ok = false;
                  }
               } else {
                  mc2log << warn << "parseRegionGroupList "
                         << "odd Element in region_group_list element: "
                         << child->getNodeName() << endl;
               }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         default:
            ok = false;
            mc2log << warn << "parseRegionGroupList odd "
               "node type in region_group_list element: " 
                    << child->getNodeName() 
                    << " type " << child->getNodeType()<< endl;
            break;
      }
      
      child = child->getNextSibling();
   }

   
   return ok;
}


static bool parseRegionIDs( const DOMNode* node, 
                     RegionIDs::regionIDMap& regionmap,
                     RegionIDs::regionGroupMap& groupmap,
                     RegionIDs::mccMap& mccIdMap,
                     RegionIDs::isoNameMap& isoNameIdMap )
{
   bool ok = true;

   DOMNode* child = node->getFirstChild();
   vector<uint32> uint32Array;

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), 
                                    "region" ) ) 
               {
                  uint32Array.clear();
                  uint32 id = parseRegion( child, uint32Array, mccIdMap,
                                           isoNameIdMap );
                  if ( id != MAX_UINT32 ) {
                     regionmap.insert( make_pair( id, uint32Array ) );
                     // Add region to the groups
                     for ( uint32 i = 0 ; i < uint32Array.size() ; ++i ) {
                        RegionIDs::regionGroupMap::iterator findIt = 
                           groupmap.find( uint32Array[ i ] );
                        if ( findIt != groupmap.end() ) {
                           (*findIt).second.first.push_back( id );
                        } else {
                           mc2log << fatal << "RegionIDs unknown "
                                  << "region_group id "
                                  << uint32Array[ i ] << endl;
                           ok = false;
                        }
                     }
                  } else {
                     mc2log << fatal << "parseRegionIDs failed "
                            << "parsing region." << endl;
                     ok = false;
                  }
               } else {
                  mc2log << warn << "parseRegionIDs "
                         << "odd Element in region_ids element: "
                         << child->getNodeName() << endl;
               }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         default:
            ok = false;
            mc2log << warn << "parseRegionIDs odd "
               "node type in region_ids element: " 
                    << child->getNodeName() 
                    << " type " << child->getNodeType()<< endl;
            break;
      }
      
      child = child->getNextSibling();
   }

   
   return ok;
}


static uint32 parseRegionID( const DOMNode* node ) {
   uint32 id = MAX_UINT32;

   if ( XMLString::equals( node->getNodeName(), "region_id" ) ) {
      // Get attributes
      DOMNamedNodeMap* attributes = node->getAttributes();
      DOMNode* attribute;

      for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
         attribute = attributes->item( i );
      
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         char* tmpPtr = NULL;
         if ( XMLString::equals( attribute->getNodeName(), "id" ) ) {
            id = strtoul( tmpStr, &tmpPtr, 10 );
            if ( tmpPtr == NULL || tmpPtr[ 0 ] != '\0' ) {
               mc2log << warn << "parseRegionID Problem parsing "
                      << "id not a valid number." << endl;
               id = MAX_UINT32;
            }
         } else {
            mc2log << warn << "parseRegionID "
                   << "unknown attribute for region_id element "
                   << " Name " << attribute->getNodeName()
                   << " Type " << attribute->getNodeType() << endl;
         }
         delete [] tmpStr;
      }

   } else {
      mc2log << error << "parseRegionID not region_id " 
             << MC2CITE( node->getNodeName() ) << endl;
   }

   return id;
}


static bool parseRegionList( const DOMNode* node, 
                             RegionIDs::regionIDMap& regionmap,
                             RegionIDs::regionGroupMap& groupmap,
                             RegionIDs::regionListMap& listmap )
{
   bool ok = true;

   DOMNode* child = node->getFirstChild();
   RegionList list;
   MC2String name;

   DOMNamedNodeMap* attributes = node->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(), "ident" ) ) {
         name = tmpStr;
      } else {
         mc2log << warn << "parseRegionList "
                << "unknown attribute for region list element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), 
                                    "region_id" ) ) 
            {
               // Get the id-attribute of the region_id
               uint32 id = parseRegionID( child );
               if ( id != MAX_UINT32 ) {
                  list.addRegion( id );
               } else {
                  mc2log << warn << "parseRegionList failed parsing "
                         << "region_id." << endl;
                  ok = false;
               }
            } else if ( XMLString::equals( child->getNodeName(), 
                                           "region_group_id" ) ) 
            {
               // Get id and add to list
               NameCollection c;
               uint32 gid = parseRegionGroup( child, c,
                                              "region_group_id" );
               if ( gid != MAX_UINT32 ) {
                  list.addGroup( gid );
               } else {
                  mc2log << warn << "parseRegionList failed parsing "
                         << "region_group_id." << endl;
                  ok = false;
               }
            } else {
               mc2log << warn << "parseRegionList "
                      << "odd Element in region_list element: "
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         default:
            ok = false;
            mc2log << warn << "parseRegionList odd "
               "node type in region_lists element: " 
                    << child->getNodeName() 
                    << " type " << child->getNodeType()<< endl;
            break;
      }
      
      child = child->getNextSibling();
   }

   if ( ok ) {
      listmap.insert( make_pair( name, list ) );
   }
   
   return ok;
}


static bool parseRegionLists( const DOMNode* node, 
                              RegionIDs::regionIDMap& regionmap,
                              RegionIDs::regionGroupMap& groupmap,
                              RegionIDs::regionListMap& listmap )
{
   bool ok = true;

   DOMNode* child = node->getFirstChild();

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), 
                                    "region_list" ) ) 
            {
               ok = parseRegionList( child, regionmap, groupmap, listmap );
            } else {
               mc2log << warn << "parseRegionLists "
                      << "odd Element in region_list element: "
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         default:
            ok = false;
            mc2log << warn << "parseRegionLists odd "
                   << "node type in region_lists element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType()<< endl;
            break;
      }
      
      child = child->getNextSibling();
   }
   
   return ok;
}

#endif


ServerRegionIDs::ServerRegionIDs() {
#ifdef USE_XML

   try {
      const char* inputFile = "region_ids.xml";
      LocalFileInputSource s( X( "./" ), X( inputFile ) );
//X( "./region_ids.xml" ) );

//X( "./" ), X( "region_ids.xml" ) );

//X( "./region_ids.xml" ) );

      XercesDOMParser parser;
      parser.setIncludeIgnorableWhitespace( false );

      bool ok = true;
      try {
         parser.parse( s );
      } catch ( const XMLException& e ) {
         mc2log << error 
                << "RegionIDs an XMLerror occured "
                << "during parsing of region file: "
                << e.getMessage() << " line " 
                << e.getSrcLine() << endl;
         ok = false;
      } catch( const SAXParseException& e) {
         mc2log << error
                << "RegionIDs an SAXerror occured "
                << "during parsing of region file: "
                << e.getMessage() << ", "
                << "line " << e.getLineNumber() << ", column " 
                << e.getColumnNumber() << endl;
         ok = false;
      }
      if ( !ok ) {
         mc2log << fatal << "RegionIDs failed parsing region file." 
                << endl;
         exit( 1 );
      }

      DOMDocument* doc = parser.getDocument();
      
      DOMElement* rootEl = doc ? doc->getDocumentElement() : NULL;
      
      if ( rootEl != NULL && XMLString::equals( rootEl->getNodeName(), 
                                                "map_generation-mc2" ) )
      {
         // Go throu the elements and handle them.
         DOMNode* child = rootEl->getFirstChild();
         vector<uint32> uint32Array;
         
         while ( child != NULL && ok ) {
            switch ( child->getNodeType() ) {
               case DOMNode::ELEMENT_NODE :
                  // See if the element is a known type
                  if ( XMLString::equals( child->getNodeName(), 
                                          "region_group_list" ) ) 
                  {
                     ok = parseRegionGroupList( child, m_regionGroupMap );
                  } else if ( XMLString::equals( child->getNodeName(), 
                                                 "region_ids" ) ) 
                  {
                     ok = parseRegionIDs( child, m_regionIDMap,
                                          m_regionGroupMap,
                                          m_mccMap,
                                          m_isoNameMap
                                          );
                  } else if ( XMLString::equals( child->getNodeName(), 
                                                 "region_lists" ) ) 
                  {
                     ok = parseRegionLists( child, m_regionIDMap,
                                            m_regionGroupMap, 
                                            m_regionListMap);
                     
                  } else {
                     mc2log << warn << "RegionIDs "
                            << "odd Element in map_generation-mc2 "
                            << "element: " << child->getNodeName() << endl;
                  }
                  break;
               case DOMNode::COMMENT_NODE :
                  // Ignore comments
                  break;
               case DOMNode::TEXT_NODE :
                  // Ignore stray texts
                  break;
               default:
                  ok = false;
                  mc2log << warn << "XMLParserThread::xmlParseIsabmc2 odd "
                          << "node type in isab-mc2 element: " 
                          << child->getNodeName() 
                       << " type " << child->getNodeType()<< endl;
                  break;
            }
            
            child = child->getNextSibling();
         }

         if ( !ok ) {
            exit( 1 );
         }

      } else {
         if ( rootEl == NULL ) {
            mc2log << fatal << "RegionIDs no document in region file. File: " 
                   << inputFile << endl;
         } else {
            mc2log << fatal << "RegionIDs not map_generation-mc2 root "
                   << "element! " << MC2CITE( rootEl->getNodeName() )
                   << endl;
         }
         exit( 1 );
      }


   } catch ( const XMLException& e ) {
      mc2log << fatal 
             << "RegionIDs an XMLerror occured "
             << "during opening of region file: "
             << e.getMessage() << endl;
      exit( 1 );
   }

#else
   mc2log << error << "RegionIDs not compiled with XML support no region "
          << "data loaded." << endl;
#endif

   // Print all region groups and their regions
   vector<pair<uint32, NameCollection> > regionGroupIDs;
   addAllRegionGroups( regionGroupIDs );
   for ( uint32 i = 0 ; i < regionGroupIDs.size() ; ++i ) {
      mc2dbg4 << "RegionGroup: " << hex << regionGroupIDs[ i ].second 
             << " (" << regionGroupIDs[ i ].first << ") : ";
      vector<uint32> regionIDs;
      addRegionIDsFor( regionIDs, regionGroupIDs[ i ].first );
      for ( uint32 j = 0 ; j < regionIDs.size() ; ++j ) {
         if ( j != 0 ) {
            mc2dbg4 << ", ";
         }
         mc2dbg4 << regionIDs[ j ];
      }
      mc2dbg4 << dec << endl;
   }
}
