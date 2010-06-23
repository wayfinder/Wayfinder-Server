/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavSearchHandler.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "UserData.h"
#include "NavUtil.h"
#include "ClosestSearchRequest.h"
#include "SearchRequestParameters.h"
#include "SearchRequest.h"
#include "SearchMatch.h"
#include "CoordinateRequest.h"
#include "TopRegionRequest.h"
#include "NavParserThreadGroup.h"
#include "NavUserHelp.h"
#include "SearchParserHandler.h"
#include "ParserDebitHandler.h"
#include "ExternalSearchConsts.h"

NavSearchHandler::NavSearchHandler( InterfaceParserThread* thread,
                                    NavParserThreadGroup* group,
                                    NavUserHelp* userHelp )
      : NavHandler( thread, group ), m_userHelp( userHelp )
{
   m_expectations.push_back( 
      ParamExpectation( 1200, NParam::Uint16_array, 4, 4 ) );
   m_expectations.push_back( ParamExpectation( 1201, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1202, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1203, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1204, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 1205, NParam::Uint32 ) );
   m_expectations.push_back( 
      ParamExpectation( 1206, NParam::Uint32_array, 8, 8 ) );
   m_expectations.push_back( ParamExpectation( 1207, NParam::Uint16 ) );
   m_expectations.push_back( 
      ParamExpectation( 1208, NParam::Byte_array, 2, 2 ) );
}


void 
NavSearchHandler::addRegionTo( SearchMatch* match, 
                               map< const SearchMatch*,
                               uint16, SearchMatchIDAndNameLessThan >& 
                               regionSearchMap, NParam& pregion,
                               int version ) const
{
   vector<const SearchMatch*> printRegs;
   char matchIDStr[ SearchMatch::searchMatchBaseStrLength + 4 ];
   match->getRegionsToPrint( printRegs );
   for ( uint32 j = 0 ; j < printRegs.size() ; ++j ) {
      const SearchMatch* vrm = printRegs[ j ];
      if ( vrm != NULL && 
           regionSearchMap.find( vrm ) == regionSearchMap.end() )
      {
         // Add new region
         uint32 index = regionSearchMap.size();
         regionSearchMap.insert( make_pair( vrm, index ) );
         // Type
         if ( version >= 1 ) {
            pregion.addUint32( mc2TypeToNavRegionType( vrm->getType() ) );
         } else {
            pregion.addUint16( mc2TypeToNavRegionType( vrm->getType() ) );
         }

         SearchMatch::matchToString( 
            vrm, matchIDStr, 
            SearchMatch::searchMatchBaseStrLength, false );
         pregion.addString( matchIDStr, m_thread->clientUsesLatin1() );
         pregion.addString( vrm->getName(), m_thread->clientUsesLatin1() );
      }
   }
   if ( printRegs.empty() && *match->getLocationName() != '\0' ) {
      // Fake it from locationName
      SearchMatchPoints points;
      VanillaRegionMatch* vrm = new VanillaBuiltUpAreaMatch( 
         match->getLocationName(), 0, 0, 0, points, 0, "", 0, 0, 
         ItemTypes::builtUpAreaItem, 0 );
      if ( regionSearchMap.find( vrm ) == regionSearchMap.end() ) {
         // Add new region
         match->addRegion( vrm ,true/*delete it*/ );
         uint32 index = regionSearchMap.size();
         regionSearchMap.insert( make_pair( vrm, index ) );
         // Type
         if ( version >= 1 ) {
            pregion.addUint32( mc2TypeToNavRegionType( vrm->getType() ) );
         } else {
            pregion.addUint16( mc2TypeToNavRegionType( vrm->getType() ) );
         }
         // Empty id to avoid use
         pregion.addString( /*matchIDStr*/"", m_thread->clientUsesLatin1() );
         pregion.addString( vrm->getName(), m_thread->clientUsesLatin1() );
         mc2dbg8 << "Adding fake region " << vrm->getName() << endl;
      } else {
         mc2dbg8 << "Reusing fake region " << vrm->getName() << endl;
         match->addRegion( const_cast<VanillaRegionMatch*> (
                              static_cast<const VanillaRegionMatch*>( 
                                 (*regionSearchMap.find( vrm )).first ) ), 
                           false/*Do not delete it*/ );
         delete vrm;
      }
   }
   if ( printRegs.empty() && *match->getLocationName() == '\0' ) {
      mc2dbg2 << "No regions and empty loationname for " << match->getName()
              << endl;
   }
}


bool
NavSearchHandler::handleSearch( UserItem* userItem, 
                                NavRequestPacket* req, 
                                NavReplyPacket* reply )
{
   if ( !checkExpectations( req->getParamBlock(), reply ) ) {
      return false;
   }
   bool ok = true;

   uint32 startTime = TimeUtility::getCurrentTime();
   uint32 now = TimeUtility::getRealTime();

   // The params
   const NParamBlock& params = req->getParamBlock();
   NParamBlock& rparams = reply->getParamBlock();
   // The user
   UserUser* user = userItem->getUser();

   const ClientSetting* clientSetting = 
      m_userHelp->getClientSetting( params, user );
   UserEnums::URType urmask = m_userHelp->getUrMask( clientSetting );

   // Start parameter printing
   mc2log << info << "handleSearch:";

   // Language
   StringTable::languageCode language = user->getLanguage();
   if ( params.getParam( 6 ) ) {
      language = NavUtil::mc2LanguageCode( 
         params.getParam( 6 )->getUint16() );
      mc2log << " Lang " << StringTable::getString( 
         StringTable::getLanguageAsStringCode( language ), 
         StringTable::ENGLISH );
   }

   SearchRequestParameters searchParams;
   searchParams.setTryHarder( true );
   searchParams.setLookupCoordinates( true  );
   searchParams.setAddStreetNamesToCompanies( true );
   searchParams.setSearchForTypes( user->getSearchForTypes() );
   searchParams.setSearchForLocationTypes( 
      user->getSearchForLocationTypes() );
   searchParams.setMatchType( SearchTypes::StringMatching( 
                                 user->getSearch_type() ) );
   searchParams.setStringPart( SearchTypes::StringPart( 
                                  user->getSearch_substring() ) );
   searchParams.setSortingType( SearchTypes::DistanceSort );
   searchParams.setRequestedLang( language );
   SearchRequestParameters areaSearchParams( searchParams );
   
   // Start index
   uint16 areaStartIndex = 0;
   uint16 itemStartIndex = 0;
   if ( params.getParam( 1200 ) ) {
      areaStartIndex = params.getParam( 1200 )->getUint16Array( 0 );
      itemStartIndex = params.getParam( 1200 )->getUint16Array( 1 );
      mc2log << " IN " << areaStartIndex << "," << itemStartIndex;
   }

   // Search area string
   MC2String areaStrStr;
   const char* areaStr = NULL;
   if ( params.getParam( 1201 ) ) {
      areaStrStr = params.getParam( 1201 )->getString(
         m_thread->clientUsesLatin1() );
      areaStr = areaStrStr.c_str();
      mc2log << " A " << areaStr;
   }

   // Search AreaID
   MC2String areaIDStr;
   const char* areaID = NULL;
   if ( params.getParam( 1202 ) ) {
      areaIDStr = params.getParam( 1202 )->getString(
         m_thread->clientUsesLatin1());
      areaID = areaIDStr.c_str();
      mc2log << " SA " << areaID;
   }

   // Search item string
   MC2String itemStr;
   if ( params.getParam( 1203 ) ) {
      itemStr = StringUtility::trimStartEnd( 
         params.getParam( 1203 )->getString(
            m_thread->clientUsesLatin1() ) );
      mc2log << " I " << itemStr;
   }

   // Search category
   MC2String categoryStr;
   if ( params.getParam( 1204 ) ) {
      categoryStr = params.getParam( 1204 )->getString(
         m_thread->clientUsesLatin1() );
      mc2log << " C " << categoryStr;
      SearchParserHandler::Category cat = 
         m_thread->getSearchHandler().
         findCategoryFromListLowerCase( categoryStr );
      if ( cat == SearchParserHandler::INVALID_CATEGORY ) {
         // do old style search
         searchParams.setMatchType( SearchTypes::FullMatch );
      } else {
         // set category id and do a faster new category search.
         set<SearchParserHandler::Category::CategoryID> categories;
         categories.insert( cat.getCategoryID() );
         searchParams.setCategories( categories );
         // must clear it to search all categories.
         categoryStr.clear();
         itemStr.clear();
      }

   }

   // TopRegionID
   uint32 topRegionID = MAX_UINT32;
   if ( params.getParam( 1205 ) ) {
      topRegionID = params.getParam( 1205 )->getUint32();
      mc2log << " TID " << MC2HEX( topRegionID );
   }

   // Search pos
   MC2Coordinate searchPos;
   if ( params.getParam( 1206 ) ) {
      searchPos = MC2Coordinate( 
         Nav2Coordinate( params.getParam( 1206 )->getInt32Array( 0 ),
                         params.getParam( 1206 )->getInt32Array( 1 ) ) );
      mc2log << " Pos " << searchPos;//.lat << ", " << searchPos.lon;
   }

   // Max Nbr Search Matches
   uint32 maxNbrMatches = 10;
   if ( params.getParam( 1207 ) ) {
      maxNbrMatches = params.getParam( 1207 )->getUint16();
      mc2log << " MAX " << maxNbrMatches;
   }

   // Sorting
   if ( params.getParam( 1208 ) ) {
      areaSearchParams.setSortingType( 
         mc2SortType( params.getParam( 1208 )->getByteArray()[ 0 ] ) );
      searchParams.setSortingType( 
         mc2SortType( params.getParam( 1208 )->getByteArray()[ 1 ] ) );
      mc2log << " AS " << SearchTypes::getSortingString( 
                areaSearchParams.getSortingType() ) << " IS " 
             << SearchTypes::getSortingString( 
                searchParams.getSortingType() );
   }

   // End parameter printing
   mc2log << endl;

   // AURA
   set< uint32 >* allowedMaps = NULL;
   if ( ok && !m_thread->getMapIdsForUserRegionAccess( user, allowedMaps,
                                                       now, urmask ) )
   {
      mc2log << warn << "handleSearch: getMapIdsForUserRegionAccess"
             << " failed. ";
      if ( TimeUtility::getCurrentTime() - startTime > 3000 ) {
         reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
         mc2log << "Timeout";
      } else {
         reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         mc2log << "Error";
      }
      mc2log << endl;
      ok = false;
   }

   // Store user position
   if ( ok && searchPos.isValid() ) {
      m_userHelp->storeUserPosition( user->getUIN(), searchPos, "Search" );
   }


   // Do we have selected location?
   SearchMatch* selectedArea = NULL;
   if ( areaID != NULL ) {
      selectedArea = SearchMatch::createMatch( areaID );
      if ( selectedArea == NULL ) {
         mc2log << warn << "handleSearch: Bad areaID " 
                << MC2CITE( areaID ) << endl;
         ok = false;
         reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_PARAMETER_INVALID );
         reply->setStatusMessage( "Search AreaID" );
      } else {
         // Check if area is in allowed maps
         if ( allowedMaps != NULL && allowedMaps->find( 
                 selectedArea->getMapID() ) == allowedMaps->end() )
         {
            mc2log << warn << "handleSearch: selected area unallowed map "
                   << MC2CITE( areaID ) << endl;
            ok = false;
            reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_OUTSIDE_ALLOWED_AREA );
         }
      }
   }

   // Which item search string
   MC2String searchString;
   if ( categoryStr.length() > 0 ) {
      searchString = categoryStr;
   } else if ( itemStr.length() > 0 ) {
      searchString = itemStr;
   }
   bool searchForArea = true;

   // Search!
   if ( ok ) {
      SearchResultRequest* sr = NULL;
      RequestData reqID = m_thread->getNextRequestID();

      if ( selectedArea != NULL ) {
         vector < IDPair_t > selectedAreas;
         selectedAreas.push_back( selectedArea->getID() );

         SearchRequest* s = new SearchRequest( 
            reqID, searchParams, selectedAreas, searchString.c_str(),
            m_thread->getTopRegionRequest(),
            searchPos, &areaSearchParams );
         s->setAllowedMaps( allowedMaps );
         sr = s;
         searchForArea = false;
      } else {
         // Get topRegionID from position and check position if proximity
         if ( topRegionID == MAX_UINT32 || areaStr == NULL ) {
            if ( searchPos.isValid() ) {
               CoordinateRequest* navReq = new 
                  CoordinateRequest( m_thread->getNextRequestID(),
                                     m_thread->getTopRegionRequest());
               const byte itemType = ItemTypes::streetSegmentItem;
               navReq->addCoordinate( 
                  searchPos.lat, searchPos.lon,
                  ( COORDINATE_PACKET_RESULT_ANGLE |
                    COORDINATE_PACKET_RESULT_STRINGS |
                    COORDINATE_PACKET_RESULT_IDS ), 1, &itemType);
               m_thread->putRequest( navReq );
               CoordinateReplyPacket* crp = navReq->getCoordinateReply();
               if ( crp == NULL || crp->getStatus() != StringTable::OK ) {
                  // We have an error report it to the user!
                  mc2log << warn << "handleSearch: Lookup of pos falied ";
                  uint8 errorCode = NavReplyPacket::NAV_STATUS_NOT_OK;
                  if ( crp != NULL ) {
                     if ( crp->getStatus() == StringTable::MAPNOTFOUND ) {
                        mc2log << "OUTSIDE_MAP";
                        errorCode = NavReplyPacket::NAV_STATUS_OUTSIDE_MAP;
                     } else {
                        mc2log << "NOT_OK";
                        errorCode = NavReplyPacket::NAV_STATUS_NOT_OK;
                     }
                  } else {
                     if ( TimeUtility::getCurrentTime() - startTime > 3000 ) {
                        mc2log << "REQUEST_TIMEOUT";
                        errorCode = 
                           NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT;
                     } else {
                        mc2log << "Reply null NOT_OK";
                        errorCode = NavReplyPacket::NAV_STATUS_NOT_OK;
                     }
                  }
                  mc2log << endl;
                  reply->setStatusCode( errorCode );
                  ok = false;
               } else { // Reply is ok
                  const TopRegionRequest* topRegReq = 
                     m_group->getTopRegionRequest( &*m_thread );

                  // Note that the topRegionID is a topRegionID - not CC.
                  if ( topRegionID == MAX_UINT32 ) {
                     // Find the topRegionMatch of type country.
                     const TopRegionMatch* topReg =
                        topRegReq->getCountryForMapID( crp->getMapID() );
                     if ( topReg != NULL ) {
                        topRegionID = topReg->getID();
                     }
                  }
               }
               delete navReq;
            } else if ( topRegionID != MAX_UINT32 && areaStr == NULL ) {
               // Have topregion and proximity search but no pos is ok now
            } else {
               mc2log << warn << "handleSearch: not valid pos";
               if ( topRegionID == MAX_UINT32 ) {
                  mc2log << " for unknown topRegionID ";
               }
               if ( areaStr == NULL ) {
                  mc2log << " for proximity ";
               }
               mc2log << endl;
               ok = false;
               reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_MISSING_PARAMETER );
               reply->setStatusMessage( "Search Position" );
            }
         } // End if needs to lookup pos


         if ( ok ) {
            if ( areaStr != NULL ) {
               // Check if topRegionID is in allowed regions
               if ( m_thread->checkUserRegionAccess( topRegionID, user,
                                                     urmask ) )
               {
                  sr = new SearchRequest( 
                     reqID, searchParams, topRegionID, areaStr,
                     searchString.c_str(), m_thread->getTopRegionRequest(),
                     searchPos, &areaSearchParams );
               } else {
                  mc2log << warn << "handleSearch: selected topRegionID "
                         << hex << topRegionID << dec << " not allowed."
                         << endl;
                  ok = false;
                  reply->setStatusCode( 
                     NavReplyPacket::NAV_STATUS_OUTSIDE_ALLOWED_AREA );
               }
            } else { // Search around position
               // Will search the whole country.
               if ( searchPos.isValid() ) {
                  sr = new ClosestSearchRequest( 
                     reqID, searchParams, searchPos, searchString,
                     m_thread->getTopRegionRequest(),
                     allowedMaps ? *allowedMaps : set<uint32>() );
               } else if ( topRegionID != MAX_UINT32 ) {
                  if ( m_thread->checkUserRegionAccess( topRegionID, 
                                                        user, urmask ) )
                  {
                     sr = new ClosestSearchRequest( 
                        reqID, searchParams, topRegionID, searchString,
                        m_thread->getTopRegionRequest(),
                        allowedMaps ? *allowedMaps : set<uint32>() );
                  } else {
                     mc2log << warn << "handleSearch: proximity: selected "
                            << "topRegionID " << hex << topRegionID << dec
                            << " not allowed." << endl;
                     ok = false;
                     reply->setStatusCode( 
                        NavReplyPacket::NAV_STATUS_OUTSIDE_ALLOWED_AREA );
                  }
               } else {
                  mc2log << warn << "handleSearch: not valid pos nor "
                         << "topRegion ";
                  mc2log << endl;
                  ok = false;
                  reply->setStatusCode( 
                     NavReplyPacket::NAV_STATUS_MISSING_PARAMETER );
                  reply->setStatusMessage( 
                     "Search Position or TopRegion" );
               }
               searchForArea = false;
            }
         }
         
      } // End else if have selected search area
      
      if ( ok ) {
         m_thread->putRequest( sr );

         // Check sr
         mc2log << info << "handleSearch: Reply:";
         if ( sr->getStatus() != StringTable::OK ) {
            if ( sr->getStatus() == StringTable::TIMEOUT_ERROR ) {
               mc2log << " REQUEST_TIMEOUT";
               reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
            } else if ( sr->getStatus() == 
                        StringTable::OUTSIDE_ALLOWED_AREA )
            {
               mc2log << " OUTSIDE_ALLOWED_AREA";
               reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_OUTSIDE_ALLOWED_AREA );
            } else { 
               mc2log << " NOT_OK";
               reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
            }
         } else {
            // Add the result
            addMatches( rparams, sr, searchForArea, maxNbrMatches, 
                        areaStartIndex, itemStartIndex, 1302, 
                        1303, 1300, 1304, 1301 );
         } // End else sr is ok
         mc2log << endl;
         
         PacketContainer* searchAnswer = sr->getAnswer();
         
         // Debit search
         MC2String extraInfo = m_userHelp->makeExtraUserInfoStr( params );
         if ( !m_thread->getDebitHandler()->makeSearchDebit( 
                 userItem, sr, searchAnswer, areaStr,
                 searchString.c_str(), extraInfo.c_str() ) )
         {
            mc2log << warn << "handleSearch: debit failed." << endl;
         }


         delete searchAnswer;
         delete sr->getAnswer();
         delete sr;
      } // End if ok
   } // End if ok


   delete allowedMaps;
   delete selectedArea;

   return ok;
}


SearchTypes::SearchSorting 
NavSearchHandler::mc2SortType( byte navSortType ) const {
   switch ( navSortType ) {
      case 0x00 : // No sort
         return SearchTypes::NoSort;
      case 0x01 : // Alfa sort
         return SearchTypes::AlphaSort;
      case 0x02 : // Confidence sort
         return SearchTypes::ConfidenceSort;
      case 0x04 : // Distance sort
         return SearchTypes::DistanceSort;

      default:
         mc2log << error << "NavSearchHandler::mc2SortType bad sort type: "
                << int(navSortType) << endl;
         return SearchTypes::DistanceSort;
   }
}


uint16
NavSearchHandler::mc2TypeToNavRegionType( uint32 type ) {
/*
      enum RegionType {
         streetnumber = 0x01,
         address      = 0x02,
         city_part    = 0x03,
         city         = 0x04,
         municipal    = 0x05,
         county       = 0x06,
         state        = 0x07,
         country      = 0x08,
         zipcode      = 0x09,
         zipArea      = 0x0a,
      };
*/
   switch ( type ) {
      case SEARCH_STREETS :
         return 0x02;
      case SEARCH_COMPANIES :
         return 0x02;
      case SEARCH_CATEGORIES :
         return 0x02;
      case SEARCH_MISC :
         return 0x02;
      case SEARCH_MUNICIPALS :
         return 0x05;
      case SEARCH_BUILT_UP_AREAS :
         return 0x04;
      case SEARCH_CITY_PARTS :
         return 0x03;
      case SEARCH_ZIP_CODES :
         return 0x09;
      case SEARCH_ZIP_AREAS :
         return 0x0a;

      default:
         mc2log << warn << "mc2TypeToNavRegionType unknown type: " << type 
                << endl;
         return 0x02;
   }
}


uint8
NavSearchHandler::mc2TypeToNavItemType( uint32 type ) {
   /*
      enum SearchItemType {
         street   = 0x01,
         poi      = 0x02,
         misc     = 0x03,
         category = 0x04,
      };
   */
   switch ( type ) {
      case SEARCH_STREETS :
         return 0x01;
      case SEARCH_COMPANIES :
         return 0x02;
      case SEARCH_CATEGORIES :
         return 0x04;
      case SEARCH_MISC :
         return 0x03;
      case SEARCH_MUNICIPALS :
         return 0x01;
      case SEARCH_BUILT_UP_AREAS :
         return 0x01;
      case SEARCH_CITY_PARTS :
         return 0x01;
      case SEARCH_ZIP_CODES :
         return 0x01;
      case SEARCH_ZIP_AREAS :
         return 0x01;
      case SEARCH_PERSONS :
         return 0x05;

      default:
         mc2log << warn << "mc2TypeToNavItemType unknown type: " << type 
                << endl;
         return 0x03;
   }
}


uint8
NavSearchHandler::mc2ItemTypeToNavSubType( ItemTypes::itemType type )
{
   // Up to 254
   if ( type <= 254 ) {
      return type;
   } else if ( type == MAX_UINT16 ) {
      return MAX_UINT8; // No subtype
   } else {
      mc2log << "mc2ItemTypeToNavSubType unknown itemType " << int( type )
             << endl;
      return 0;
   }
}

bool
NavSearchHandler::addMatches( 
   NParamBlock& rparams, const SearchResultRequest* sr,
   bool addArea, uint32 maxNbrMatches, 
   uint32 areaStartIndex, uint32 itemStartIndexReal, 
   uint16 regionParamID, 
   uint16 areaParamID, uint16 areaIndexParamID,
   uint16 itemParamID, uint16 itemIndexParamID,
   NParam* areaIndexParam, NParam* areaParam,
   NParam* itemIndexParam, NParam* itemParam,
   NParam* regionParam,
   int version,
   bool addAreaIndex,
   bool addItemIndex,
   uint32 headingID )
{
   bool added = true;
   vector < OverviewMatch* > areaMatches;
   vector < VanillaMatch* >  itemMatches;
   const vector<VanillaMatch*>& matches = sr->getMatches();
   char matchIDStr[ SearchMatch::searchMatchBaseStrLength + 4 ];

   // Transform index to suit external search
   uint32 itemStartIndex = sr->translateMatchIdx( itemStartIndexReal );

   while ( added ) {
      added = false;
      if ( addArea ) {
         // Add area
         if ( areaMatches.size() < maxNbrMatches ) 
         {
            if ( areaMatches.size() + areaStartIndex < 
                 sr->getNbrOverviewMatches() ) 
            {
               areaMatches.push_back( 
                  sr->getOverviewMatch( areaMatches.size() + 
                                        areaStartIndex) );
               added = true;
            }
         }
      }
      if ( itemMatches.size() < maxNbrMatches ) 
      {
         if ( itemMatches.size() + itemStartIndex < 
              matches.size() )
         {
            itemMatches.push_back( 
               matches[ itemMatches.size() + itemStartIndex ] );
            added = true;
//                     mc2dbg << "Adding " << matches[ itemMatches.size() + itemStartIndex ]->getName() << " pos " << matches[ itemMatches.size() + itemStartIndex ]->getCoords() << endl;
         }
      }
   }

   // Make regions
   map< const SearchMatch*, uint16, 
      SearchMatchIDAndNameLessThan > regionSearchMap;
   NParam& pregion = regionParam ? *regionParam : 
      rparams.addParam( NParam( regionParamID ) );
   for ( uint32 i = 0 ; i < areaMatches.size() ; ++i ) {
      addRegionTo( areaMatches[ i ], regionSearchMap, pregion, version );
   }
   for ( uint32 i = 0 ; i < itemMatches.size() ; ++i ) {
      addRegionTo( itemMatches[ i ], regionSearchMap, pregion, version );
   }
   if ( regionSearchMap.size() > 0 ) {
      mc2log << " NR " << regionSearchMap.size();
   }

   
   // TODO: Hmm, merging adding of area and item might reduce code size.

   if ( areaMatches.size() > 0 ) {
      if ( addAreaIndex ) {
         NParam& p1 = areaIndexParam ? *areaIndexParam :
            rparams.addParam( NParam( areaIndexParamID ) );
         p1.addUint16( areaStartIndex );
         p1.addUint16( sr->getNbrOverviewMatches() );
      }
      // Area matches
      NParam& p = areaParam ? *areaParam : 
         rparams.addParam( NParam( areaParamID ) );
      mc2log << " AM " << areaMatches.size() << "(" << areaStartIndex << "-"
             << (areaStartIndex+(areaMatches.size() > 0 ? 
                                 areaMatches.size() -1 : 0)) << ") of total "
             << sr->getNbrOverviewMatches();
      for ( uint32 i = 0 ; i < areaMatches.size() ; ++i ) {
         const OverviewMatch* match = areaMatches[ i ];
         p.addUint16( mc2TypeToNavRegionType( 
                         match->getType() ) );
         SearchMatch::matchToString( 
            match, matchIDStr, 
            SearchMatch::searchMatchBaseStrLength, false );
         p.addString( matchIDStr, m_thread->clientUsesLatin1() );
         p.addString( match->getName(),
                      m_thread->clientUsesLatin1() );
         if ( version >= 1 ) {
            p.addString( m_thread->getSearchHandler().
                         getImageName( *match, headingID ), 
                         m_thread->clientUsesLatin1() );
         }
         vector<const SearchMatch*> printRegs;
         match->getRegionsToPrint( printRegs );
         p.addByte( printRegs.size() );
         for ( uint32 j = 0 ; j < printRegs.size() ; ++j ) {
            map< const SearchMatch*, uint16, 
               SearchMatchIDLessThan >::const_iterator it = 
               regionSearchMap.find( printRegs[ j ] );
            uint16 index = 0;
            if ( it != regionSearchMap.end() ) {
               index = it->second;
            } else {
               mc2log << error << "handleSearch: region not found"
                      << " among unique regions! Memory must be "
                      << "corrupt! (areaMatches)" << endl;
            }
            p.addUint16( index );
         } // End for all printRegs
      } // End for all areaMatches
   } // End if areaMatches.size() > 0

   uint32 nbrAdverts = 0;
   if ( itemMatches.size() > 0 ) {
      if ( addItemIndex ) {
         NParam& p1 = itemIndexParam ? *itemIndexParam : 
            rparams.addParam( NParam( itemIndexParamID ) );
         p1.addUint16( itemStartIndexReal );
         p1.addUint16( matches.size() ); 
      }
      NParam& p = itemParam ? *itemParam : 
         rparams.addParam( NParam( itemParamID ) );
      mc2log << " IM " << itemMatches.size() << "(" << itemStartIndexReal 
             << "-" 
             << (itemStartIndexReal+(itemMatches.size() > 0 ? 
                                     itemMatches.size() -1 : 0)) 
             << ") of total " << sr->getTotalNbrMatches();
      for ( uint32 i = 0 ; i < itemMatches.size() ; ++i ) {
         const VanillaMatch* match = itemMatches[ i ];
         p.addByte( mc2TypeToNavItemType( match->getType() ) );
         p.addByte( mc2ItemTypeToNavSubType( 
                       ItemTypes::itemType(
                          match->getItemSubtype() ) ) );
         if ( version >= 2 ) {
            p.addByte( match->getExtSource() == ExternalSearchConsts::adserver &&
                       nbrAdverts < 25 );
            ++nbrAdverts;
         }

         SearchMatch::matchToString( 
            match, matchIDStr, 
            SearchMatch::searchMatchBaseStrLength, false );
         p.addString( matchIDStr, m_thread->clientUsesLatin1() );
         // TODO: Get the address of poi add as region type address
         p.addString( match->getName(),
                      m_thread->clientUsesLatin1() );
         if ( version >= 1 ) {
            p.addString( m_thread->getSearchHandler().
                         getImageName( *match, headingID ),
                         m_thread->clientUsesLatin1() );
         }

         Nav2Coordinate coord( match->getCoords() );
//                    mc2dbg << "Adding " << match->getName() << " pos " << match->getCoords() << " dist " << uint32( rint( sqrt( GfxUtility::squareP2Pdistance_linear( match->getCoords(), searchPos ) ) ) ) << endl;
         p.addInt32( coord.nav2lat );
         p.addInt32( coord.nav2lon );
         vector<const SearchMatch*> printRegs;
         match->getRegionsToPrint( printRegs );
         p.addByte( printRegs.size() );
         for ( uint32 j = 0 ; j < printRegs.size() ; ++j ) {
            map< const SearchMatch*, uint16, 
               SearchMatchIDLessThan >::const_iterator it = 
               regionSearchMap.find( printRegs[ j ] );
            uint16 index = 0;
            if ( it != regionSearchMap.end() ) {
               index = it->second;
            } else {
               mc2log << error << "handleSearch: region not found"
                      << " among unique regions! Memory must be "
                      << "corrupt! (itemMatches)" << endl;
            }
            p.addUint16( index );
         } // End for all printRegs
      } // End for all itemMatches
   } // End if itemMatches.size() > 0

   return added;
}
