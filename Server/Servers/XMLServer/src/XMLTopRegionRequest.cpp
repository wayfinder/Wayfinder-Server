/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserThread.h"


#ifdef USE_XML
#include "TopRegionRequest.h"
#include "UserData.h"
#include "ServerRegionIDs.h"
#include "DataBuffer.h"
#include "MC2CRC32.h"
#include "STLStringUtility.h"
#include "XMLAuthData.h"
#include "XMLCommonEntities.h"
#include "XMLTool.h"
#include "StringConvert.h"
#include "XMLServerErrorMsg.h"
#include "DeleteHelpers.h"
#include "XMLServerElements.h"
#include "XMLSearchUtility.h"
#include "XMLTopRegionElement.h"

using XMLServerUtility::appendStatusNodes;
namespace {

/**
 * Appends a top_region as last child to cur.
 *
 * @param cur Where to put the top_region node as last child.
 * @param reply The document to make the nodes in.
 * @param topRegion The top region to print.
 * @param format The coordinate format to use.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 * @param buf Top region data added to make crc later.
 */
void appendTopRegion( DOMNode* cur, DOMDocument* reply,
                      const TopRegionMatch* topRegion,
                      XMLCommonEntities::coordinateType 
                      format,
                      LangTypes::language_t language,
                      int indentLevel, bool indent,
                      DataBuffer* buf );

}

namespace StringConvert {

// specialized templates for string conversion
// used in value conversion in nodes and attributes

template <>
XMLCommonEntities::coordinateType convert<XMLCommonEntities::coordinateType>( 
   const MC2String& str ) 
   throw (StringConvert::ConvertException) {
   XMLCommonEntities::coordinateType coordType = 
      XMLCommonEntities::coordinateFormatFromString( 
         str.c_str(), XMLCommonEntities::NBR_COORDINATETYPES );
   if ( coordType == XMLCommonEntities::NBR_COORDINATETYPES ) {
      throw 
         StringConvert::
         ConvertException( MC2String( "Could not make coordinateType from:" )
                           + str );
                             
   }
   return coordType;
}
}


bool 
XMLParserThread::xmlParseTopRegionRequest( DOMNode* cur, 
                                           DOMNode* out,
                                           DOMDocument* reply,
                                           bool indent )
{
   bool ok = true;
   int indentLevel = 1;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   MC2String errorCode;
   MC2String errorMessage;

   // TopRegion data
   LangTypes::language_t language = LangTypes::english;
   XMLCommonEntities::coordinateType positionSystem =
      XMLCommonEntities::MC2;
   bool addCountry = true;
   bool addState = false;
   bool addInternationalRegion = false;
   bool addMetaregion = false;
   MC2String topRegionCRC;
   bool hasTopRegionCRC = false;

   // Create top_region_reply element
   DOMElement* top_region_reply = 
      reply->createElement( X( "top_region_reply" ) );
   // Transaction ID
   top_region_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( top_region_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), top_region_reply );
   }


   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      DOMNode*  attribute = attributes->item( i );
      

      if ( XMLString::equals( attribute->getNodeName(),
                              "transaction_id" ) ) {
         // Handled above 
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "top_region_crc" ) ) {
         topRegionCRC = XMLUtility::transcodefrom( attribute->getNodeValue() );
         hasTopRegionCRC = true;
      } else {
         mc2log << warn << "XMLParserThread::xmlParseTopRegionRequest "
                   "unknown attribute for top_region_request element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }

   }


   // Get children
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "top_region_request_header" ) )
            {
               ok = xmlParseTopRegionRequestHeader( 
                  child, language, positionSystem, 
                  addCountry, addState, addInternationalRegion, 
                  addMetaregion,
                  errorCode, errorMessage );
            } else {
               mc2log << warn << "XMLParserThread::"
                         "xmlParseTopRegionRequest "
                         "odd Element in top_region_request element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                      "xmlParseTopRegionRequest odd "
                      "node type in top_region_request element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }
   

   if ( ok ) {
      // Make TopRegionRequest
      const TopRegionRequest* topRequest = 
         m_group->getTopRegionRequest( this );

      if ( topRequest != NULL  &&
           topRequest->getStatus() == StringTable::OK ) 
      {
         // Get current top region crc and add
         // top_region_list
         MC2String currCRC = appendTopRegionList( top_region_reply, reply, 
                                                  indentLevel + 1, indent,
                                                  language, positionSystem,
                                                  addCountry, addState, 
                                                  addInternationalRegion,
                                                  addMetaregion,
                                                  topRequest, 
                                                  topRegionCRC );
         top_region_reply->setAttribute( 
            X( "top_region_crc" ), X( currCRC ) );
      } else {
         // Error
         errorMessage = "Problem retieving top region data: ";
         if ( topRequest != NULL ) {
            errorMessage.append( StringTable::getString(
               topRequest->getStatus(), StringTable::ENGLISH ) );
         } else {
            errorMessage.append( StringTable::getString(
               StringTable::StringTable::TIMEOUT_ERROR, 
               StringTable::ENGLISH ) );
         }
         appendStatusNodes( top_region_reply, reply, 
                            indentLevel + 1, indent, 
                            "-1", errorMessage.c_str() );
         mc2log << info << "TopRegionRequest request failed: "
                << errorMessage << endl;
      }
   } else {
      // An error in errorCode, errorMessage
      appendStatusNodes( top_region_reply, reply, 
                         indentLevel + 1, indent, 
                         errorCode.c_str(), errorMessage.c_str() );
      // Error handled 
      ok = true;
   }

   if ( indent ) {
      // Newline and indent before end tag   
      top_region_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   return ok;
}


bool 
XMLParserThread::xmlParseTopRegionRequestHeader( 
   DOMNode* cur,
   LangTypes::language_t& language,
   XMLCommonEntities::coordinateType& positionSystem,
   bool& addCountry, bool& addState, bool& addInternationalRegion,
   bool& addMetaregion,
   MC2String& errorCode, MC2String& errorMessage )
{
   bool ok = true;

   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "position_system" ) ) 
      {
         positionSystem = XMLCommonEntities::coordinateFormatFromString( 
            tmpStr, XMLCommonEntities::NBR_COORDINATETYPES );
         if ( positionSystem == 
              XMLCommonEntities::NBR_COORDINATETYPES )
         {
            ok = false;
            errorCode = "-1";
            errorMessage = "Unknown position_system in "
               "top_region_request_header.";
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "country" ) ) 
      {
         addCountry = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(), "state" ) )
      {
         addState = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "internationalRegion" ) )
      {
         addInternationalRegion = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "metaregion" ) )
      {
         addMetaregion = StringUtility::checkBoolean( tmpStr );
      } else {
         mc2log << warn 
                << "XMLParserThread::xmlParseTopRegionRequestHeader "
            "unknown attribute for top_region_request_header element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }


   // Get children
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), "language" ) ) {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               language = getStringAsLanguage( tmpStr );
               if ( language == LangTypes::invalidLanguage ) {
                  mc2log << warn << "XMLParserThread::"
                            "xmlParseTopRegionRequestHeader "
                         << "bad language in language node: " << tmpStr
                         << endl;
                  ok = false;
                  errorCode = "-1";
                  errorMessage = "Unknown language in "
                     "top_region_request_header's language.";
               }
               delete [] tmpStr;
            } else {
               mc2log << warn << "XMLParserThread::"
                  "xmlParseTopRegionRequestHeader "
                  "odd Element in top_region_request_header element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                      "xmlParseTopRegionRequestHeader odd "
                      "node type in top_region_request_header element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }


   return ok;
}


MC2String 
XMLParserThread::appendTopRegionList( 
   DOMNode* cur, DOMDocument* reply,
   int indentLevel, bool indent,
   LangTypes::language_t language,
   XMLCommonEntities::coordinateType positionSystem,
   bool& addCountry, bool& addState, bool& addInternationalRegion,
   bool& addMetaregion,
   const TopRegionRequest* topRequest,
   MC2String clientTopRegionCRC,
   bool sortMatches )
{
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   char tmpStr[128];
   MC2String currCRC;
   DataBuffer db( 4096 );

   currCRC.reserve( 4096 );

   // Create top_region_list element
   DOMElement* top_region_list = 
      reply->createElement( X( "top_region_list" ) );
   cur->appendChild( top_region_list );
   if ( indent ) {
      // Newline
      cur->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), top_region_list );
   }

   mc2log << info << "TopRegionRequest in "
          << LangTypes::getLanguageAsString( language, true ) << " " << hex;


// typedef vector< TopRegionMatch* > TopRegionMatchesVector;
// typedef vector< const TopRegionMatch* > ConstTopRegionMatchesVector;

   //sort topregions according to locale. 
   const TopRegionMatchesVector& c_topregions = 
      topRequest->getTopRegionVector();
   TopRegionMatchesVector topregions( c_topregions.begin(), 
                                      c_topregions.end() );
   if ( sortMatches ) {
      sort( topregions.begin(), topregions.end(), 
            topRegionMatchCompareLess( language ) );
   }

   uint32 nbrAddedTopRegions = 0;
   for ( TopRegionMatchesVector::iterator tr = topregions.begin(); 
         tr != topregions.end(); 
         ++tr ) {
      //      for ( uint32 i = 0 ; i < topregions.size() ; ++i ) {
      TopRegionMatch& trMatch = **tr;
      bool add = true;
      switch ( trMatch.getType() ) {
         case TopRegionMatch::country :
            add = addCountry;
            break;
         case TopRegionMatch::state :
            add = addState;
            break;
         case TopRegionMatch::internationalRegion :
            add = addInternationalRegion;
            break;
         case TopRegionMatch::metaregion :
            add = addMetaregion;
            break;
         case TopRegionMatch::nbr_topregion_t :
            add = false; // Can't be good to add nbr_topregion_t region
            break;
      }
      if ( !checkUserRegionAccess( trMatch.getID(), 
                                   m_user->getUser(), 
                                   m_authData->urmask ) )
      {
         add = false;
      }
      if ( add ) {
         ++nbrAddedTopRegions;
         appendTopRegion( top_region_list, reply, 
                          &trMatch, positionSystem,
                          language, indentLevel + 1, indent, &db );
         if ( nbrAddedTopRegions != 1 ) {
            mc2log << ",";
         }
         mc2log << trMatch.getID();
         currCRC.append( reinterpret_cast<const char*>( 
                            db.getBufferAddress() ), 
                         db.getCurrentOffset() );
         db.reset();
      }
   }
   // addMetaregion
   if ( addMetaregion ) {
      vector<pair<uint32, NameCollection> > regionGroupIDs;
      m_group->getRegionIDs()->addAllRegionGroups( regionGroupIDs );
      for ( uint32 i = 0 ; i < regionGroupIDs.size() ; ++i ) {
         // Make fake TopRegion
         // TODO: Fix bbox 
         TopRegionMatch match( regionGroupIDs[ i ].first, 
                               TopRegionMatch::metaregion  );
         const Name* name = regionGroupIDs[ i ].second.getBestName(
            language );
         match.addName( name->getName(), name->getLanguage() );
         ++nbrAddedTopRegions;
         appendTopRegion( top_region_list, reply, 
                          &match, positionSystem,
                          language, indentLevel + 1, indent, &db );
         if ( nbrAddedTopRegions != 1 ) {
            mc2log << ",";
         }
         mc2log << regionGroupIDs[ i ].first;
         currCRC.append( reinterpret_cast<const char*>( 
                            db.getBufferAddress() ), 
                         db.getCurrentOffset() );
         db.reset();
      }
   }
   mc2log << dec << endl;

   uint32 crc = MC2CRC32::crc32( reinterpret_cast<const byte*>( 
                                    currCRC.data() ), currCRC.size() );
   currCRC = "";
   STLStringUtility::uint2strAsHex( crc, currCRC );

   if ( clientTopRegionCRC == currCRC ) {
      // Uptodate
      // remove top_region_list
      cur->replaceChild( reply->createElement( 
                            X( "top_region_crc_ok" ) ),
                         top_region_list );
   }

   // numberitems
   sprintf( tmpStr, "%d", nbrAddedTopRegions );
   top_region_list->setAttribute( X( "numberitems" ), X( tmpStr ) );

   // Indent before end tag
   if ( indent ) {
      top_region_list->appendChild( reply->createTextNode( 
         XindentStr.XMLStr() ) );
   }

   return currCRC;
}

bool
XMLParserThread::xmlParseExpandTopRegion(  DOMNode* cur, 
                                           DOMNode* out,
                                           DOMDocument* reply,
                                           bool indent )
try {   
   bool ok = true;
   int indentLevel = 1;
   MC2String errorCode;      //errorcode if one should be sent in reply.
   MC2String errorMessage;   //errorMessage if one should be sent in reply

   using namespace XMLTool;

   DOMElement* e_reply = 
      reply->createElement( X( "expand_top_region_reply" ) );
   out->appendChild( e_reply );

   MC2String transaction_id;
   getAttrib( transaction_id, "transaction_id", cur );
   e_reply->setAttribute( X( "transaction_id" ), X( transaction_id ) );

   LangTypes::language_t language = LangTypes::english;
   getAttrib( language, "language", cur, LangTypes::english );

   XMLCommonEntities::coordinateType positionSystem =
      XMLCommonEntities::MC2;
   getAttrib( positionSystem, "position_system", cur, XMLCommonEntities::MC2 );
   bool addCountry = true;
   bool addState = false;
   bool addInternationalRegion = false;
   bool addMetaregion = false;
   getAttrib( addCountry, "country", cur, true );
   getAttrib( addState, "state", cur, false );
   getAttrib( addInternationalRegion, "internationalRegion", cur, false );
   getAttrib( addMetaregion, "metaregion", cur, false );


   STLUtility::AutoContainer< vector< TopRegionMatch* > > topregions;

   for ( DOMNode* child = cur->getFirstChild();
         child != NULL;
         child = child->getNextSibling() ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE:
            if ( XMLString::equals( child->getNodeName(),
                                    "top_region" ) ) {
               TopRegionMatch* topRegion = getTopRegionMatchFromTopRegion( 
                  child, errorCode, errorMessage );
               if ( topRegion == NULL ) {
                  mc2log << "XMLParserThread::"
                         << "xmlParseExpandTopRegion "
                         << "bad top_region." << endl;
                  // errorCode, errorMessage already filled in
                  ok = false;
               } else {
                  topregions.push_back( topRegion );
               }
            } else {
               throw XMLServerErrorMsg( 
                  "-1", MC2String( "Unexpected node in "
                                   "expand_top_region_request: " )
                  + XMLUtility::transcodefrom( child->getNodeName() ) );
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseExpandTopRegion: Odd node "
                   << MC2CITE( child->getNodeName() )
                   << " especially the type " << child->getNodeType()<< endl;
            break;
      }
   }

   if ( ok ) {
      // Make TopRegionRequest
      const TopRegionRequest* topRequest = 
         m_group->getTopRegionRequest( this );

      for ( uint32 i = 0 ; i < topregions.size() ; ++i ) {
         m_group->getRegionIDs();
         auto_ptr<TopRegionRequest> topRequestAdd( 
            new TopRegionRequest( 0U ) );
         if ( m_group->getRegionIDs()->isRegionGroupID( 
                 topregions[ i ]->getID() ) ) {
            vector<uint32> regionIDs;
             m_group->getRegionIDs()->addRegionIDsFor( 
                regionIDs, topregions[ i ]->getID() );
             for ( uint32 j = 0 ; j < regionIDs.size() ; ++j ) {
                const TopRegionMatch* match = 
                   topRequest->getTopRegionWithID( regionIDs[ j ] );
                if ( match != NULL  ) {
                   topRequestAdd->addTopRegion( new TopRegionMatch( *match ) );
                }
             }
         } else {
            const TopRegionMatch* match = topRequest->getTopRegionWithID(
               topregions[ i ]->getID() );
            if ( match != NULL  ) {
               topRequestAdd->addTopRegion( new TopRegionMatch( *match ) );
            }
         }

         bool sortMatches = false;
         addMetaregion = false;/*This will add all metas if set.addMetaregion*/
         appendTopRegionList( e_reply, reply, 
                              indentLevel + 1, false/*indent*/,
                              language, positionSystem,
                              addCountry, addState, 
                              addInternationalRegion,
                              addMetaregion,
                              topRequestAdd.get(), 
                              ""/*topRegionCRC*/,
                              sortMatches );
         
      } // End for all incoming topregions
   }

   if ( !ok ) {
      appendStatusNodes( e_reply, reply, 0, false,
                         errorCode.c_str(), errorMessage.c_str() );
      ok = true; // Error handled
   }
   

   if ( indent ) {
      XMLUtility::indentPiece( *e_reply, indentLevel );
   }
   return ok;
} catch ( const XMLServerErrorMsg& error ) {
   appendStatusNodes( out->getFirstChild(), reply, 1, false, 
                      error.getCode().c_str(), error.getMsg().c_str() );
   return true;
} catch ( const StringConvert::ConvertException& error ) {
   appendStatusNodes( out->getFirstChild(), reply, 1, false, 
                      "-1", error.what() );
   return true;
} catch ( const XMLTool::Exception& error ) {
   appendStatusNodes( out->getFirstChild(), reply, 1, false, 
                      "-1", error.what() );
   return true;
}

namespace {


void 
appendTopRegion( DOMNode* cur, DOMDocument* reply,
                 const TopRegionMatch* topRegion,
                 XMLCommonEntities::coordinateType format,
                 LangTypes::language_t language,
                 int indentLevel, bool indent,
                 DataBuffer* buf ) {
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   
   DOMElement* topRegionNode =
      XMLTopRegionElement::
      makeTopRegionElement( topRegion, format,
                            language, reply,
                            indentLevel, indent, buf );

   // Add topRegionNode to cur
   if ( indent ) {
      cur->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
   }
   cur->appendChild( topRegionNode );
   if ( indent ) {
      topRegionNode->appendChild( reply->createTextNode( 
         XindentStr.XMLStr() ) );
   }
}

}
#endif // USE_XML

