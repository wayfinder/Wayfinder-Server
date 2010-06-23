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

#include "ItemTypes.h"
#include "ProximityRequest.h"

#include "SearchHandler.h"
#include "SharedProximityPackets.h"
#include "ServerProximityPackets.h"
#include "UserData.h"
#include "UserPacket.h"

#include "SearchRequestParameters.h"
#include "MC2Coordinate.h"
#include "GfxConstants.h"
#include "UserRightsMapInfo.h"
#include "DeleteHelpers.h"
#include "TimeUtility.h"

void
ProximityRequest::init()
{
   m_savedAnswer        = NULL;
   m_positionPacket     = NULL;
   m_itemSetPacket      = NULL;
   m_coveredIDsReply    = NULL;
   m_searchAnswers      = NULL;
   m_searchRequests     = NULL;
   m_coveredIDsReplys   = NULL;
   m_coveredIDsRequests = NULL;
   m_dbMask             = SearchTypes::DefaultDB;
   
   m_innerRadius = 0;
   m_startAngle = 0;
   m_stopAngle = 360;

   // Make parameters with default params. Use these
   // instead of the other stuff later.
   m_params = new SearchRequestParameters();
   m_params->setAddStreetNamesToCompanies(true);
   m_params->setSortingType(SearchTypes::DistanceSort);
   m_params->setRequestedLang(StringTable::SWEDISH);
   m_params->setRegionsInOverviewMatches( SEARCH_REGION_TYPES );
   m_params->setRegionsInMatches( SEARCH_REGION_TYPES );
}

ProximityRequest::ProximityRequest( const RequestData& requestID, 
                                    ProximityPositionRequestPacket* 
                                    packet )
      : SearchResultRequest(requestID),
        m_state(NOTHING_SENT)
   
{
   init();
   m_positionPacket = packet;
   m_state = GET_ALL_IDS;
   m_answer = NULL;
   m_itemSetPacket = NULL;
   m_nbrMaps = 0; // Defined value
   m_searchAnswers = NULL;
   m_searchRequests = NULL;
   m_coveredIDsReply = NULL;
   m_coveredIDsReplys = NULL;
   m_coveredIDsRequests = NULL;
   m_currentSearchReply = 0;
   m_currentSearchRequest = 0;

   m_coveredIDs = makeCoveredIDsRequestPacket( m_positionPacket );
}


ProximityRequest::ProximityRequest( const RequestData& requestID, 
                                    ProximityItemSetRequestPacket* packet)
      : SearchResultRequest(requestID),
        m_state(NOTHING_SENT)
{
   init();
   m_itemSetPacket = packet;
   m_state = GET_ALL_FITTING_FROM_SET;
   m_answer = NULL;
   m_positionPacket = NULL;
   m_nbrMaps = 0; // Defined value, see below
   m_coveredIDsReply = NULL;
   m_coveredIDsReplys = NULL;
   m_coveredIDsRequests = NULL;

   SearchTypes::StringMatching matchType;
   SearchTypes::StringPart     stringPart;
   SearchTypes::SearchSorting  sortingType;
   uint8 nbrSortedHits;
   uint16 categoryType;
   uint16 maxNumberHits;
   
   m_itemSetPacket->decodeRequest( m_nbrMaps,
                                   m_itemType,
                                   maxNumberHits,
                                   m_useString,
                                   m_searchString,
                                   categoryType,
                                   matchType,
                                   stringPart,
                                   sortingType,
                                   nbrSortedHits );

   m_params->setSortingType(sortingType);
   m_params->setStringPart(stringPart);
   m_params->setMatchType(matchType);
   m_params->setNbrSortedHits(nbrSortedHits);
   m_params->setSearchForTypes(categoryType);
   m_params->setEndHitIndex(maxNumberHits);

   m_currentSearchReply = 0;
   m_currentSearchRequest = 0;
   m_searchAnswers = new PacketContainer*[m_nbrMaps];
   m_searchRequests = new PacketContainer*[m_nbrMaps];
   vector<IDPair_t> itemIDs;
   for ( uint32 i = 0 ; i < m_nbrMaps ; i++ ) {
      m_searchAnswers[i] = NULL;
      itemIDs.clear();
      // Add items
      int position = 0;
      uint32 nbrItems = m_itemSetPacket->getNumberItems(i);
      bool ok = true;
      position = m_itemSetPacket->getFirstItemPosition(i);
      for ( uint32 j = 0 ; (j < nbrItems) && ok ; j++ ) {
         mc2dbg8 << "Adding ID to ProxSearchpack" << endl;
         itemIDs.push_back(IDPair_t(m_itemSetPacket->getMapID(i),
                                    m_itemSetPacket->getNextItem(position)));
      }

      // Create the packetcontainer with the packet.
      // We cannot distance sort if we don't have a coordinate.
      UserRightsMapInfo rights( m_itemSetPacket->getMapID(i),
                                getUser(),
                                ~MapRights() );
      
      m_searchRequests[i] = new PacketContainer(
         new SearchRequestPacket(m_itemSetPacket->getMapID(i),
                                 getNextPacketID(),
                                 getID(),
                                 SearchRequestPacket::PROXIMITY_SEARCH,
                                 "", // Search string
                                 *m_params,
                                 itemIDs,
                                 vector<MC2BoundingBox>(),
                                 m_position,
                                 rights),
         0,
         0,
         MODULE_TYPE_SEARCH );
   }
}


ProximityRequest::ProximityRequest( 
   const RequestData& requestID, 
   const SearchRequestParameters& params,
   const MC2BoundingBox& bbox,
   const set<ItemTypes::itemType>& itemTypes,
   const char* searchString )
      :  SearchResultRequest( requestID )
{
   // Set some NULLs and others
   init();
   // And NULL answer too
   m_answer = NULL;
   m_nbrMaps = 0; // Defined value
   m_coveredIDsReply = NULL;
   m_currentSearchReply = 0;
   m_currentSearchRequest = 0;
   m_itemType = PROXIMITY_SEARCH_ITEMS_MATCHES;
      // Set search parameters (init creates default parameters)
   *m_params = params;
   m_searchString = const_cast< char* > ( searchString );
   // Make center point and radius in meters
   MC2Coordinate center;
   uint32 radius = 0;
   uint32 radiusMeters = 0;
//   bbox.updateCosLat();
   bbox.getCenter( center.lat, center.lon );
   if ( bbox.getWidth() > bbox.getHeight() ) {
      radius = bbox.getWidth() / 2;
   } else {
      radius = bbox.getHeight() / 2;
   }
   radiusMeters = uint32( radius * GfxConstants::MC2SCALE_TO_METER );
   // Hidden feature that makes the center and radius be a bbox
   m_startAngle        = MAX_UINT16;
   m_stopAngle         = MAX_UINT16;
   // Set some more vars used later
   m_position = center;
   m_dist = radiusMeters;
   uint32 searchTypes = 0;
   for ( set<ItemTypes::itemType>::const_iterator it = itemTypes.begin() ;
         it != itemTypes.end() ; ++it )
   {
      searchTypes |= ItemTypes::itemTypeToSearchType( *it );
   }
   m_params->setSearchForTypes( searchTypes );
   // Coverids
   m_coveredIDsID = getNextPacketID();
   CoveredIDsRequestPacket* covPacket =
      new CoveredIDsRequestPacket( getUser(), m_coveredIDsID, getID(), 
                                   MAX_UINT32/*MapID*/,
                                   itemTypes );
   covPacket->setLat( center.lat );
   covPacket->setLon( center.lon );
   covPacket->setInnerRadius( 0 );
   covPacket->setOuterRadius( radiusMeters );
   covPacket->setStartAngle( m_startAngle );
   covPacket->setStopAngle( m_stopAngle );
   covPacket->setMapID( MAX_UINT32/*MapID*/ );
   m_coveredIDs = new PacketContainer( covPacket, 0, 0,
                                       MODULE_TYPE_MAP );
   m_state = GET_ALL_IDS;
}


ProximityRequest::~ProximityRequest()
{
   delete m_params;
   delete m_positionPacket;
   delete m_itemSetPacket;
   delete m_coveredIDsReply;
   if ( m_searchAnswers != NULL ) {      
      for ( uint32 i = 0; i < m_nbrMaps; ++i ) {
         delete m_searchAnswers[i];
      }
      delete [] m_searchAnswers;
   }
   if ( m_searchRequests != NULL ) {
      // Remove unsent requests
      for ( uint32 i = m_currentSearchRequest ; i < m_nbrMaps ; i++ ) {
         delete m_searchRequests[i];
         // doesn't seem necessary: m_searchRequests[i] = NULL;
      }
      delete [] m_searchRequests;
   }

   if ( m_coveredIDsReplys ) {      
      for ( uint32 i = 0; i < m_nbrMaps; ++i ) {
         delete m_coveredIDsReplys[i];
      }
      delete [] m_coveredIDsReplys;
   }
  
   if ( m_coveredIDsRequests != NULL ) {
      // Remove unsent requests
      for ( uint32 i = m_currentCoveredRequest ; i < m_nbrMaps ; i++ ) {
         delete m_coveredIDsRequests[i];
      }
      delete [] m_coveredIDsRequests; 
   }

   STLUtility::deleteValues( m_matches );
   // m_savedAnswer is put into m_deleteLater
   STLUtility::deleteValues( m_deleteLater );

}


PacketContainer* 
ProximityRequest::getNextPacket() {
   PacketContainer* result = NULL;
   switch ( m_state ) {
      case NOTHING_SENT:
         mc2dbg2 << "ProximityRequest::getNextPacket NOTHING_SENT"
                << endl;
         break;
      case GET_ALL_IDS:
         DEBUG2( cerr << "ProximityRequest::getNextPacket GET_ALL_IDS" 
                 << endl);
         result = m_coveredIDs;
         m_coveredIDs = NULL;
         m_state = AWAITING_ALL_IDS;
         break;
      case AWAITING_ALL_IDS:
         DEBUG2( cerr << "ProximityRequest::getNextPacket AWAITING_ALL_IDS"
                 << endl);
         break;
      case GET_ALL_ITEMS_COVERED:
         DEBUG2( cerr 
                 << "ProximityRequest::getNextPacket GET_ALL_ITEMS_COVERED"
                 << endl);
         if ( m_currentCoveredRequest < m_nbrMaps ) {
            result = m_coveredIDsRequests[m_currentCoveredRequest];
            m_currentCoveredRequest++;
         } else {
            m_state = AWAITING_ALL_ITEMS_COVERED;
         }
         break;
      case AWAITING_ALL_ITEMS_COVERED:
         DEBUG2( cerr << "ProximityRequest::getNextPacket "
                 "AWAITING_ALL_ITEMS_COVERED" << endl);
         break;
      case GET_ALL_FITTING_ITEMS:
         // Same as GET_ALL_FITTING_FROM_SET
      case GET_ALL_FITTING_FROM_SET:
         DEBUG2( cerr << "ProximityRequest::getNextPacket "
                 "GET_ALL_FITTING_FROM_SET" << endl);
         if ( m_currentSearchRequest < m_nbrMaps ) {
            result = m_searchRequests[m_currentSearchRequest];
            m_currentSearchRequest++;
         } else {
            m_state = AWAITING_ALL_FITTING_ITEMS; 
         }
         break;
      case AWAITING_ALL_FITTING_ITEMS:
         DEBUG2( cerr << "ProximityRequest::getNextPacket "
                 "AWAITING_ALL_FITTING_ITEMS" << endl);
         break;
      case DONE:
         DEBUG2( cerr << "ProximityRequest::getNextPacket DONE" << endl);
         break;
   } // End switch ( m_state )

   return result;
}


void
ProximityRequest::processPacket(PacketContainer *ans) {
   DEBUG2( cerr << "ProximityRequest::processPacket" << endl);
   if ( ans != NULL ) {
      switch ( m_state ) {
         case NOTHING_SENT:
            DEBUG2(
            cerr << "ProximityRequest::processPacket:: Packet received,"
                 << " but I haven't sent any!!?(NOTHING_SENT)" << endl;
            );
            delete ans;
            break;
         case GET_ALL_IDS:
            DEBUG2(
            cerr << "ProximityRequest::processPacket::Packet received,"
                 << " but I haven't sent any!!?(GET_ALL_IDS)" << endl;
            );
            delete ans;
            break;
         case AWAITING_ALL_IDS:
            if ( (ans->getPacket()->getSubType() ==
                  Packet::PACKETTYPE_COVEREDIDSREPLYPACKET) &&
                 (ans->getPacket()->getPacketID() == m_coveredIDsID) ) {
               m_coveredIDsReply = ans;
               CoveredIDsReplyPacket* p = 
                  static_cast<CoveredIDsReplyPacket*>(ans->getPacket());
               if ( p->isMapIDs() ) {
                  mc2dbg4 << "ProximityRequest::processPacket " 
                         << " AWAITING_ALL_IDS got MapIDs" << endl;
                  m_state = GET_ALL_ITEMS_COVERED;
                  m_nbrMaps = p->getNumberIDs();
                  m_currentCoveredRequest = 0;
                  m_currentCoveredReply = 0;
                  m_coveredIDsReplys = new PacketContainer*[m_nbrMaps];
                  for ( uint32 i = 0 ; i < m_nbrMaps ; i++ ) {
                     m_coveredIDsReplys[i] = NULL;
                  }
                  m_coveredIDsRequests = new PacketContainer*[m_nbrMaps];
                  set<ItemTypes::itemType> types;
                  ItemTypes::searchTypeToItemTypes(
                     types,
                     m_params->getSearchForTypes());
                  for ( uint32 i = 0 ; i < m_nbrMaps ; i++ ) {
                     m_coveredIDsRequests[i] = new PacketContainer(
                        new CoveredIDsRequestPacket(getUser(),
                                                    getNextPacketID(),
                                                    getID(),
                                                    p->getID(i),
                                                    types),
                        0,
                        0,
                        MODULE_TYPE_MAP );
                     CoveredIDsRequestPacket* p = 
                        static_cast<CoveredIDsRequestPacket*>(
                           m_coveredIDsRequests[i]->getPacket() );
                     p->setLat( m_position.lat );
                     p->setLon( m_position.lon );
                     p->setOuterRadius( m_dist );
                     p->setInnerRadius( m_innerRadius );
                     p->setStartAngle( m_startAngle );
                     p->setStopAngle( m_stopAngle );
                  }
               } else {
                  DEBUG4(cerr << "ProximityRequest::processPacket " 
                         << " AWAITING_ALL_IDS got ItemIDs" << endl;);
                  m_state = GET_ALL_FITTING_ITEMS;
                  m_nbrMaps = 1;
                  m_currentSearchReply = 0;
                  m_currentSearchRequest = 0;
                  m_searchAnswers = new PacketContainer*[m_nbrMaps];
                  m_searchRequests = new PacketContainer*[m_nbrMaps];
                  for ( uint32 i = 0 ; i < m_nbrMaps ; i++ ) {
                     m_searchAnswers[i] = NULL;

                     mc2dbg8 << "MapID of CoveredIDsReplyPacket is "
                            << p->getMapID() << endl;


                     mc2dbg8 << "searchPack->encodeRequest( "
                            "GET_ALL_FITTING_ITEMS )" << endl
                            << "m_itemType " << (int)m_itemType << endl
                            << "m_useString " << m_useString << endl
                            << "m_searchString " << m_searchString << endl
                            << endl;
                     
                     // Add items
                     vector<IDPair_t> itemIDs;
                     uint32 nbrItems = p->getNumberIDs();
                     bool ok = true;
                     uint32 itemID = 0;
                     mc2dbg8 << "Items in map: " << p->getMapID()
                            << " nbr " << nbrItems << endl;
                     uint32 nbr = 0;
                     while ((nbr < nbrItems) && ok) {
                        itemID = p->getID(nbr);
//                        mc2dbg8 << itemID << endl;
                        itemIDs.push_back(IDPair_t(p->getMapID(), itemID));
                        nbr++;
                     }
                     if (!ok) {
                        mc2log << warn << "ProximityRequest:: "
                               << "Not all items added"
                               << " to ProximitySearchRequestPacket.";
                     }

                     UserRightsMapInfo rights( p->getMapID(),
                                               getUser(),
                                               ~MapRights() );
                     
                     m_searchRequests[i] = new PacketContainer(
                        new SearchRequestPacket(
                           p->getMapID(),
                           getNextPacketID(),
                           getID(),
                           SearchRequestPacket::PROXIMITY_SEARCH,
                           "", // Search string
                           *m_params,
                           itemIDs,
                           vector<MC2BoundingBox>(),
                           m_position,
                           rights),
                        0,
                        0,
                        MODULE_TYPE_SEARCH );
                  }
               }
            } else {
               mc2dbg << "ProximityRequest::processPacket"
                      << " AWAITING_ALL_IDS odd packet received type: " 
                      << (int) ans->getPacket()->getSubType() << "="
                      << ans->getPacket()->getSubTypeAsString()
                      << " packID: " << ans->getPacket()->getPacketID()
                      << endl;
            }
            break;
         case GET_ALL_ITEMS_COVERED:
            // Same as AWAITING_ALL_ITEMS_COVERED 
         case AWAITING_ALL_ITEMS_COVERED:
            if ( (ans->getPacket()->getSubType() ==
                  Packet::PACKETTYPE_COVEREDIDSREPLYPACKET) ) {
               m_coveredIDsReplys[m_currentCoveredReply] = ans;
               m_currentCoveredReply++;
               if ( m_currentCoveredReply >= m_nbrMaps ) { 
                  // All answers gotten
                  m_state = GET_ALL_FITTING_ITEMS;
                  m_currentSearchReply = 0;
                  m_currentSearchRequest = 0;
                  m_searchAnswers = new PacketContainer*[m_nbrMaps];
                  m_searchRequests = new PacketContainer*[m_nbrMaps];
                  CoveredIDsReplyPacket* p;
                  for ( uint32 i = 0 ; i < m_nbrMaps ; i++ ) {
                     m_searchAnswers[i] = NULL;
                     p = static_cast<CoveredIDsReplyPacket*>( 
                        m_coveredIDsReplys[i]->getPacket());
                     
                     // Add items
                     vector<IDPair_t> itemIDs;
                     uint32 nbrItems = p->getNumberIDs();
                     bool ok = true;
                     for ( uint32 j = 0 ; (j < nbrItems) && ok ; j++ ) {
                        itemIDs.push_back(IDPair_t(p->getMapID(),
                                                   p->getID(j)));
                     }
                     UserRightsMapInfo rights( p->getMapID(),
                                               getUser(),
                                               ~MapRights() );
                     m_searchRequests[i] = new PacketContainer(
                        new SearchRequestPacket(
                           p->getMapID(),
                           getNextPacketID(),
                           getID(),
                           SearchRequestPacket::PROXIMITY_SEARCH,
                           "", // Search string
                           *m_params,
                           itemIDs,
                           vector<MC2BoundingBox>(),
                           m_position,
                           rights),
                        0,
                        0,
                        MODULE_TYPE_SEARCH );
                  }
               }
            } else {
               mc2dbg << "ProximityRequest::processPacket"
                      << " ALL_ITEMS_COVERED odd packet received type: " 
                      << (int) ans->getPacket()->getSubType() << "="
                      << ans->getPacket()->getSubTypeAsString()
                      << " packID: " << ans->getPacket()->getPacketID()
                      << endl;
            }
            break;
         case GET_ALL_FITTING_ITEMS:
            // Same as AWAITING_ALL_FITTING_ITEMS
         case GET_ALL_FITTING_FROM_SET:
            // Same as AWAITING_ALL_FITTING_ITEMS
         case AWAITING_ALL_FITTING_ITEMS:
            mc2dbg2 << "AWAITING_ALL_FITTING_ITEMS" << endl;
            if ( m_itemType == PROXIMITY_SEARCH_ITEMS_NUMBERS ) {
               if ( (ans->getPacket()->getSubType() ==
                     Packet::PACKETTYPE_PROXIMITYSEARCHREPLY) ) {  
                  m_searchAnswers[m_currentSearchReply] = ans;
                  m_currentSearchReply++;
                  if ( m_currentSearchReply >= m_nbrMaps ) { 
                  // All answers gotten
                     /// MERGE ANSWERS in getAnswer 
                     
                     m_state = DONE;
                     m_done = true;
                  }
               } else {
                  mc2dbg << "ProximityRequest::processPacket"
                         << " ALL_FITTING_ITEMS odd packet received type:" 
                         << (int) ans->getPacket()->getSubType() << "="
                         << ans->getPacket()->getSubTypeAsString()
                         << " packID: " << ans->getPacket()->getPacketID()
                         << endl;
               }
            } else if ( m_itemType == PROXIMITY_SEARCH_ITEMS_MATCHES ) {
               mc2dbg2 << "PROXIMITY_SEARCH_ITEMS_MATCHES" << endl;
               if ( (ans->getPacket()->getSubType() ==
                     Packet::PACKETTYPE_VANILLASEARCHREPLY) ) {  
                  m_searchAnswers[m_currentSearchReply] = ans;
                  m_currentSearchReply++;
                  if ( m_currentSearchReply >= m_nbrMaps ) { 
                     m_state = DONE;
                     m_done = true;
                  }
               } else {
                  mc2dbg << "ProximityRequest::processPacket"
                         << " ALL_FITTING_ITEMS odd packet received type:" 
                         << (int) ans->getPacket()->getSubType() << "="
                         << ans->getPacket()->getSubTypeAsString()
                         << " packID: " << ans->getPacket()->getPacketID()
                         << endl;
               }

            } else {
               mc2dbg << "ProximityRequest::processPacket"
                      << " ALL_FITTING_ITEMS odd packet received type: " 
                      << (int) ans->getPacket()->getSubType() << "="
                      << ans->getPacket()->getSubTypeAsString()
                      << " packID: " << ans->getPacket()->getPacketID()
                      << endl;
            }
            break;
         case DONE:
            delete ans;
            break;
      } // End switch ( m_state )
   }
}

StringTable::stringCode
ProximityRequest::getStatus() const
{
   ProximityRequest* nonConstThis = const_cast<ProximityRequest*>(this);
   PacketContainer* tmpAnswer = m_answer;

   // We must save an answerpacket since the strings in the matches
   // can point into it.
   if ( m_savedAnswer == NULL ) {
      // Get a new answer
      nonConstThis->m_answer = NULL;
      nonConstThis->m_savedAnswer = nonConstThis->getAnswer();     
      // Must delete later.
      if ( m_savedAnswer ) {
         nonConstThis->m_deleteLater.push_back( m_savedAnswer );
      }
   }
   nonConstThis->m_answer = tmpAnswer;
   
   StringTable::stringCode code = StringTable::OK;
   
   if ( m_savedAnswer == NULL ) {
      code = StringTable::NOT;
   } else {
      // Urk. This is terrible, but at least it is contained
      // inside the request, which is nice.
      code = StringTable::stringCode(
         ((ReplyPacket*)(m_savedAnswer->getPacket()))->getStatus());
   }
   
   mc2dbg4 << "[ProxReq]: Will return status = "
          << code << endl;
   return code;
}

const vector<VanillaMatch*>&
ProximityRequest::getMatches() const
{
   // Should only do this once.
   if ( ! m_matches.empty() ) {
      return m_matches;
   }

   // Be certain that the saved answer is there if there is one.
   getStatus();

   // Let's get this thing over with, shall we?
   if ( m_savedAnswer != NULL &&
        m_savedAnswer->getPacket()->getSubType() ==
        Packet::PACKETTYPE_VANILLASEARCHREPLY) {
      
      VanillaSearchReplyPacket* pack =
         static_cast<VanillaSearchReplyPacket*>(m_savedAnswer->getPacket());
      
      // This method is not really const.
      ProximityRequest* nonConstThis = const_cast<ProximityRequest*>(this);
      // Move the matches over to the vector and then sort them.
      pack->getAllMatches( nonConstThis->m_matches );
      // It has to be examined if the sorting is necessary.
      SearchSorting::sortSearchMatches( nonConstThis->m_matches,
                                        m_params->getSortingType(), -1 );
   }

   return m_matches;
}

PacketContainer* 
ProximityRequest::getAnswer() {
   // I feel sick now.
   DEBUG4(cerr << "ProximityRequest::getAnswer" << endl;);
   if ( (m_currentSearchReply >= m_nbrMaps) && (m_answer == NULL) ) {
      // Merge answers
      DEBUG4(cerr << "Merge answer together" << endl;);
      if ( m_itemType == PROXIMITY_SEARCH_ITEMS_NUMBERS ) {
         DEBUG4(cerr << "Is PROXIMITY_SEARCH_ITEMS_NUMBERS" << endl;);
         if ( m_positionPacket != NULL ) {
            m_answer = new PacketContainer(
               new ProximityReplyPacket( 
                  static_cast< ProximityPositionRequestPacket* > ( 
                     m_positionPacket ) ),
               0,
               0,
               MODULE_TYPE_INVALID );
         } else if ( m_itemSetPacket != NULL ) {
            m_answer = new PacketContainer(
               new ProximityReplyPacket( m_itemSetPacket ),
               0,
               0,
               MODULE_TYPE_INVALID );
         } else { 
            m_answer = NULL;
         }
         if ( m_answer != NULL ) {
            ProximityReplyPacket* p = (ProximityReplyPacket*) 
               m_answer->getPacket();
         
            p->setNumberMaps(m_nbrMaps);
            bool moreHits = false;
            int position;
            for ( uint32 i = 0 ; i < m_nbrMaps ; i++ ) {
               if ( m_searchAnswers[i] != NULL ) {
                  ProximitySearchReplyPacket* searchPack = 
                     static_cast<ProximitySearchReplyPacket*>( 
                        m_searchAnswers[i]->getPacket());
                  if ( searchPack->getMoreHits() ) {
                     moreHits = true;
                  }
                  p->setMapID(i, searchPack->getMapID());
                  uint32 nbrItems = searchPack->getNumberItems();
                  int replyPos = p->setNumberItems(i, nbrItems);
                  // Add items
                  position = searchPack->getFirstItemPosition();
                  bool ok = true;
                  uint32 itemID = 0;
                  for ( uint32 j = 0 ; (j < nbrItems) && ok ; j++ ) {
                     itemID = searchPack->getItem(position);
                     ok = p->addItem(itemID, 
                                     replyPos );
                  }
                  
               } else { // No valid data for this map
                  p->setMapID(i, MAX_UINT32);
                  p->setNumberItems(i, 0);
               }
            }
            p->setMoreHits(moreHits);
         }
      } else if ( m_itemType == PROXIMITY_SEARCH_ITEMS_MATCHES ) { 
         DEBUG4(cerr << "Is PROXIMITY_SEARCH_ITEMS_MATCHES" << endl;);
         if ( (m_nbrMaps == 1) && (m_searchAnswers[0] != NULL) ) {

            DEBUG4(cerr << "Only one answer no merging needed" << endl;);
            DEBUG2({
               uint32 nbrMatches =
                  (static_cast<VanillaSearchReplyPacket*>
                   (m_searchAnswers[0]->getPacket()))
                  ->getNumberOfMatches();
               cerr << "nbrMatches=" << nbrMatches << endl;
            } );
            m_answer = new PacketContainer(
               m_searchAnswers[0]->getPacket()->getClone(),
               0,0, MODULE_TYPE_INVALID );
            
         } else if ( m_currentSearchReply > 0 ) {
            DEBUG4(cerr << "Not one answer merging..." << endl;);
            UserSearchRequestPacket* tmpReqPack = 
               new UserSearchRequestPacket( 0,
                                            getID() );
            m_answer = new PacketContainer(
               new VanillaSearchReplyPacket(tmpReqPack),
               0,
               0,
               MODULE_TYPE_INVALID );
            delete tmpReqPack;
            VanillaSearchReplyPacket* vp = 
               static_cast<VanillaSearchReplyPacket*>(
                  m_answer->getPacket());
//         bool moreHits = false;
            
            // merge the replyPackets, by:
            // unpack the replyPackets into linked lists of MatchLinks.
            SearchTypes::SearchSorting sorting = m_params->getSortingType();
            MatchLink** matches = new MatchLink*[m_nbrMaps];
            
            int32 nbrMatches = 0;
            uint32 totNbrMatches = 0;
            DEBUG2({ cerr << "search reply, m_nbrMaps: " << m_nbrMaps
                          << endl; } );
            for ( uint32 i = 0 ; i < m_nbrMaps ; i++) {
               if(m_searchAnswers[i] != NULL){                  
                  matches[i] = 
                     (static_cast<VanillaSearchReplyPacket *>
                      (m_searchAnswers[i]->getPacket()))
                     ->getMatchesAsLinks( /*sorting,*/ nbrMatches );
                  DEBUG2({ cerr << "nbrMatches=" << nbrMatches
                                << "for map-index " << i << endl; } );
                  totNbrMatches += nbrMatches;
               }
               else {
                  mc2dbg8 << "SearchAnswer: " << i << " == NULL " 
                          << endl;
               }
            }
            
            // merge the result
            if ( m_nbrMaps > 1) {
               SearchHandler::merge( 0, 
                                     m_nbrMaps - 1, 
                                     0, 
                                     matches, 
                                     m_nbrMaps,
                                     sorting );
            } else {
               // no need to merge just 1 packet.
            }
            // pack into a new replyPacket
            convMatchLinkToPacket( matches[0],
                                   vp,
                                   m_params->getEndHitIndex() );
            // Delete matches
            for (uint32 i = 0 ; i < m_nbrMaps ; i++) {
               MatchLink* link = matches[i];
               MatchLink* next = NULL;
               while ( link != NULL ) {
                  next = link->getNext();
                  delete link->getMatch();
                  delete link;
                  link = next;
               }
            }
            delete[] matches;
         } else { // No answers
            m_answer = NULL;
         }
      } else { // Odd ProximitySearchItemType
         m_answer = NULL;
      }
   } else {
      mc2dbg2 << "SearchNbr, nbrMaps = " << m_currentSearchReply 
             << "," << m_nbrMaps << " ans: " << m_answer << endl;
   }
   return m_answer;
}


void
ProximityRequest::convMatchLinkToPacket( MatchLink *matchLink,
                                         VanillaSearchReplyPacket *p,
                                         uint32 maxNumberMatches ) 
{
   VanillaMatch *vm = NULL;
   uint32 nbr = 0;
   // No limit - we must sort anyway. Shouldn't use packets for merging.
   while ( (matchLink != NULL) && (true || nbr < maxNumberMatches) ) {
      vm = matchLink->getMatch();
      if( vm == NULL ){
         break;
      }
      else {
         if ( p->getBufSize() < p->getLength() + 1024 ) {
            p->resize( p->getBufSize() * 2 );
            mc2dbg2 << "[ProxReq]: Resized packet - size"
                   << p->getBufSize()
                   << endl;
         }
         vm->addToPacket(p);
         nbr++;
      }
      matchLink = matchLink->getNext();
   }
}


PacketContainer* 
ProximityRequest::makeCoveredIDsRequestPacket( Packet* posPacket ) {
   PacketContainer* res = NULL;
   if ( posPacket != NULL ) {
      if ( posPacket->getSubType() == 
           Packet::PACKETTYPE_PROXIMITYPOSITIONREQUESTPACKET ) 
      {
         ProximityPositionRequestPacket* posPack = 
            static_cast< ProximityPositionRequestPacket* > ( posPacket );
         uint32 packetLang;

          SearchTypes::StringMatching matchType;
          SearchTypes::StringPart     stringPart;
          SearchTypes::SearchSorting  sortingType;
          uint8 nbrSortedHits;
          uint32 regionsInMatches;
          uint16 categoryType;
          uint16 maxNumberMatches;
         
         posPack->decodeRequest( m_packetContent,
                                 m_position.lat,
                                 m_position.lon,
                                 m_dist,
                                 m_itemID,
                                 m_offset,
                                 m_itemType,
                                 maxNumberMatches,
                                 m_useString,
                                 m_searchString,
                                 categoryType,
                                 matchType,
                                 stringPart,
                                 sortingType,
                                 nbrSortedHits,
                                 packetLang,
                                 regionsInMatches,
                                 m_innerRadius, 
                                 m_startAngle,
                                 m_stopAngle );
         
         // Put some stuff into the parameters.
         m_params->setRequestedLang(StringTable::languageCode(packetLang));
         m_params->setSortingType(sortingType);
         m_params->setStringPart(stringPart);
         m_params->setMatchType(matchType);
         m_params->setNbrSortedHits(nbrSortedHits);
         m_params->setRegionsInMatches(regionsInMatches);
         m_params->setSearchForTypes(categoryType);
         m_params->setEndHitIndex(maxNumberMatches);

         // GetAllIDs packet
         m_coveredIDsID = getNextPacketID();
         m_mapID = posPack->getMapID();
         set<ItemTypes::itemType> types;
         ItemTypes::searchTypeToItemTypes(types,
                                          m_params->getSearchForTypes());
         CoveredIDsRequestPacket* IDs = 
            new CoveredIDsRequestPacket( getUser(),
                                         m_coveredIDsID,
                                         getID(),
                                         m_mapID,
                                         types);
         IDs->setLat(m_position.lat);
         IDs->setLon(m_position.lon);
         IDs->setOuterRadius(m_dist);
         IDs->setItemID(m_itemID);
         IDs->setOffset(m_offset);
         if ( m_packetContent == COVEREDIDSREQUEST_CONTENT_ITEMID ) {
            IDs->setItemIDAsValid();
         } else if ( m_packetContent == COVEREDIDSREQUEST_CONTENT_LAT_LON )
         {
            IDs->setLatLonAsValid();
         }
         
         res = new PacketContainer(
            IDs,
            0,
            0,
            MODULE_TYPE_MAP );
      }
   }

   return res;
}
