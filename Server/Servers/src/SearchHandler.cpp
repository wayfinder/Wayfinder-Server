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
#include "SearchHandler.h"

#include "Request.h"
#include "MC2BoundingBox.h"
#include "ExpandItemPacket.h"
#include "TopRegionMatch.h"
#include "SearchPacket.h"
#include "PacketContainer.h"
#include "PacketContainerTree.h"
#include "CoordinateOnItemObject.h"
#include "SearchExpandItemPacket.h"
#include "MatchInfoPacket.h"
#include "GfxUtility.h"
#include "UserRightsMapInfo.h"

#include "SearchSorting.h"
#include "MapBits.h"
#include "DeleteHelpers.h"

#include <set>
#include <sstream>
#include <algorithm>

#ifdef DEBUG_LEVEL_2
#define PRINT_STATE_CHANGES
#endif

// Resend timeout for SearchExpandItemRequestPackets
#define SEI_TIMEOUT TimeoutSequence( 1000, 5000 )

inline const char*
SearchHandler::stateAsString(SearchHandler::state inState)
{
   switch ( inState ) {
      case START:
         return "START";
      case SEND_TOP_REGION:
         return "SEND_TOP_REGION";
      case AWAIT_TOP_REGION:
         return "AWAIT_TOP_REGION";
      case  SEND_OVERVIEW:
         return "SEND_OVERVIEW";
      case AWAIT_OVERVIEW:
         return "AWAIT_OVERVIEW";
      case SEND_SEARCH:
         return "SEND_SEARCH";
      case AWAIT_SEARCH:
         return "AWAIT_SEARCH";
      case SEND_EXPAND_ITEM:
         return "SEND_EXPAND_ITEM";
      case AWAIT_EXPAND_ITEM:
         return "AWAIT_EXPAND_ITEM";
      case COORDINATES:
         return "COORDINATES";
      case OVERVIEWEXPAND:
         return "OVERVIEWEXPAND";
      case OVERVIEWEXPAND_THEN_OVERVIEW_CHECK:
         return "OVERVIEWEXPAND_THEN_OVERVIEW_CHECK";
      case UNEXPAND_OVERVIEW:
         return "UNEXPAND_OVERVIEW";
      case AWAIT_UNEXPAND_OVERVIEW:
         return "AWAIT_UNEXPAND_OVERVIEW";
      case ALMOST_DONE_2:
         return "ALMOST_DONE_2";
      case DONE:
         return "DONE";
      default:
         return "UNKNOWN";
   }
}

void
SearchHandler::setState(SearchHandler::state newState, int line)
{
#ifdef PRINT_STATE_CHANGES
   if ( newState != m_state ) {
      if ( line < 0 ) {
         mc2dbg << "[SH]: " 
                << stateAsString(m_state) << "->"
                << stateAsString(newState) << endl;
      } else {
         mc2dbg << "[SH]: " << __FILE__ << ":" << line 
                << ' ' <<  stateAsString(m_state) << "->"
                << stateAsString(newState) << endl;
      }
   } // Don't print if the state doesn't change.
#endif
   m_state = newState;
}

#define SETSTATE(s) setState(s, __LINE__)

// SearchHandler is currently very messy.
// The linked list of Matches should probably
// be replaced by a vector that can be used by
// the user of the request, for example. Don't
// know what the expanditem stuff is used for.

// What I think this class does:
// A. When searching for a city
//  1. Requests top regions even if it already has them.
//  2. Uses overview search to get the overview hits
//  3. Unexpands overview hits, i.e. converts them to lower level ids
//     if they do not cover multiple maps.
//  4. If empty item string -> return city hit as item hit
// B. When searching for something inside a city
//  1. Expands overview item into the same item or more than one item
//     if it covers several maps.
//  2. Sends SearchRequestPackets to the maps containing the search
//     item string and the regions to search in.
//  3. When replies come back they are sent into the search module so
//     that the regions are unexpanded into multimap regions if applicable.
//  4. If house numbers are involved, the matches are sent to the MapModule
//     so that the "best" offset on the best streetitem can be returned.
// C. When searching in a map
//  1. Same as B but with zero cities.

// TODO: Rewrite completely. Get rid of the OldSearchRequestPacket.
// I don't think the coordinate lookup stage is needed.
// When rewritten, inherit from RequestWithStatus and add
// outgoing packets to the queue and avoid implementing getNextPacket.

SearchHandler::SearchHandler( OldSearchRequestPacket *p, 
                              Request* request,
                              const SearchRequestParameters& params,
                              const TopRegionRequest* topReq,
                              bool uniqueOrFull,
                              bool searchOnlyIfUniqueOrFull,
                              uint32 regionsInMatches,
                              uint32 overviewRegionsInMatches,
                              bool lookupcoordinates,
                              bool lookupBBoxes,
                              const MC2Coordinate& sortOrigin,
                              const SearchRequestParameters* overParams )
{
   m_nbrExpectedMatchInfoPackets = 0;
   m_nbrOverviewMatchesToExpand = -1;
   m_sortOrigin = sortOrigin;

   m_params = params;
   
   m_topRegionRequest = topReq; 
   MC2_ASSERT(m_topRegionRequest != NULL);
   // Since the SearchModule has the coordinates now
   // it isn't necessary to lookup coordinates in MapModule
   // unless the bounding box should be looked up too.
   m_params.m_lookupCoordinates = lookupBBoxes;
   m_params.m_bboxRequested = lookupBBoxes;
#if 0
   if ( m_params.m_bboxRequested || m_sortOrigin.isValid() ) {
      // Must lookup coordinates to sort by distance
      // or lookup bounding boxes.
      m_params.m_lookupCoordinates = true;
   }

   // Confidence sorting now includes the distance.
   if ( m_params.getSortingType() == SearchTypes::ConfidenceSort &&
        m_sortOrigin.isValid() ) {
      m_params.m_lookupCoordinates = true;
   }
#endif
   
   m_coordinateHandler = NULL;
   m_firstPacket = true;
   m_packetsReadyToSend = new PacketContainerTree;
   m_topRegion = NULL;
   m_request = request;
   m_searchPacket = p;
   m_params.m_uniqueOrFull = uniqueOrFull;
   m_params.m_searchOnlyIfUniqueOrFull = searchOnlyIfUniqueOrFull;
   m_answer = NULL;
   m_currentRequestIndex = 0;
   m_nbrRequestPackets = 0;
   m_nbrReceivedPackets = 0;
   m_requestPackets = NULL;
   m_requestPacketIDs = NULL;
   m_replyPackets = NULL;
   m_matches = NULL;
   m_state = START;
   m_zipCode = NULL;
   m_nbrLocations = 0;
   m_locations = NULL;
   m_params.m_locationType = 0;
   m_nbrCategories = 0;
   m_categories = NULL;
   m_searchString = NULL;
   m_params.m_nbrHits = 0;
  
   m_params.m_searchForTypes = 0;
   m_params.m_nbrSortedHits = 0;
   m_params.m_editDistanceCutoff = 0;
   m_params.m_isUserSearch = false;
   m_nbrOverviewLocations = 0;
   m_expandItemCont = NULL;
   m_expandItemPacketID = 0;
   m_expandItemReply = 0;
   m_params.m_regionsInMatches = regionsInMatches;
   m_params.m_overviewRegionsInMatches = overviewRegionsInMatches;

   m_params.m_dbMask = SearchTypes::TEMP_ERROR; // to be NONE
   
   uint32 nbrMasks = 0;
   uint32* masks = NULL;
   uint32* maskItemIDs = NULL;
   char** maskNames = NULL;
   uint32 tmpUint32 = 0;

   if ( false ) {
      mc2dbg4 << "SearchHandler::SearchHandler Expand Category2"
              << endl;
   } else {
      m_params.m_isUserSearch = false;
      
      UserSearchRequestPacket* userP = 
      dynamic_cast< UserSearchRequestPacket* > ( m_searchPacket );
      if ( userP != NULL ) {
         // The bounding boxes from the packet will not be used.
         vector<MC2BoundingBox> dummyVector;
         m_params.m_isUserSearch = true;
         uint16 reqLanguage;
         userP->decodeRequest( 
            m_zipCode, m_nbrLocations, m_locations, m_params.m_locationType,
            m_nbrCategories, m_categories,
            nbrMasks, masks, maskItemIDs, maskNames, m_params.m_dbMask,
            m_searchString, m_params.m_nbrHits, m_params.m_matchType,
            m_params.m_stringPart, 
            m_params.m_sortingType, 
            m_params.m_searchForTypes,
            reqLanguage,
            m_params.m_nbrSortedHits, m_params.m_editDistanceCutoff,
            tmpUint32,
            m_params.m_categories,
            m_topRegionID,
            dummyVector );
         
         m_params.setRequestedLang(LangTypes::language_t(reqLanguage));
      } 
      // Check if we already have masks 
      if ( nbrMasks > 0 && 
           ( nbrMasks == m_searchPacket->getNumMapID()) )
      {         
         uint32 nbrMaps = m_searchPacket->getNumMapID();
         uint32* mapIDs = new uint32[ nbrMaps ];
         
         uint32 i;
         for ( i = 0 ; i < nbrMaps ; i++ ) {
            mapIDs[ i ] = m_searchPacket->getMapID( i );
         }
         
         if ( makeOverviewMatchesFromMasks(nbrMaps,
                                           mapIDs,
                                           nbrMasks,
                                           maskItemIDs,
                                           maskNames) ) {
            SETSTATE(OVERVIEWEXPAND);
         } else {
            SETSTATE(checkMatchInfosReceived());
         }
         delete [] mapIDs;
      } 
      // Check if we have location(s) to lookup 
      else if ( m_nbrLocations > 0 ) {
         /*
         mc2dbg4 << "SearchHandler::SearchHandler Have " 
                << m_nbrLocations << " Locations " << endl;
                for ( uint32 i = 0 ; i < m_nbrLocations ; i++ ) {
                   mc2dbg4 << m_locations[ i ] << endl;
                });
         */

         SETSTATE(SEND_TOP_REGION);
      }
      // Else just simply use the mapIDs
      else {
         mc2dbg4 << "SearchHandler::SearchHandler Have mapIDs " 
                 << m_searchPacket->getNumMapID() << endl;
         uint32 nbrMaps = m_searchPacket->getNumMapID();
         m_nbrRequestPackets = nbrMaps;
         m_nbrReceivedPackets = 0;
         m_requestPackets = new PacketContainer*[ nbrMaps ];
         m_requestPacketIDs = new uint32[ nbrMaps ];
         m_replyPackets = new PacketContainer*[ nbrMaps ];
         
         for ( uint32 i = 0 ; i < nbrMaps ; i++ ) {
            m_requestPacketIDs[ i ] = m_request->getNextPacketID();
            // Should search whole map.
            vector<IDPair_t> allowedRegions;
            vector<MC2BoundingBox> allowedBBoxes;
            m_params.setAddStreetNamesToCompanies(
               strcmp(m_zipCode, "AddStreetNamesToCompanies") == 0);

            UserRightsMapInfo rights( m_searchPacket->getMapID( i ),
                                      m_request->getUser(),
                                      ~MapRights() );
            
            SearchRequestPacket* reqPack =
               new SearchRequestPacket(m_searchPacket->getMapID( i ),
                                       m_requestPacketIDs[ i ],
                                       m_request->getID(),
                                       SearchRequestPacket::USER_SEARCH,
                                       m_searchString,
                                       m_params,
                                       allowedRegions,
                                       allowedBBoxes,
                                       m_sortOrigin,
                                       rights );
            
            m_requestPackets[ i ] = 
               new PacketContainer( reqPack, 0, 0, 
                                    MODULE_TYPE_SEARCH );
         }
         if ( nbrMaps > 0 )
            SETSTATE(SEND_SEARCH);
         else 
            SETSTATE(UNEXPAND_OVERVIEW);
      }
   }

   // Packet code above changes m_params
   if ( overParams != NULL ) {
      m_overParams = *overParams;
   } else {
      m_overParams = m_params;
   }

   // Empty answer
   VanillaSearchReplyPacket* pa = 
      new VanillaSearchReplyPacket( m_searchPacket );
   // Set status OK
   pa->setStatus( StringTable::OK );
   delete m_answer; // Might be set
   m_answer = new PacketContainer( 
      pa, 0, 0, MODULE_TYPE_INVALID );

   
   delete [] masks;
   delete [] maskItemIDs;
   delete [] maskNames;

   mc2dbg8 << "[SH]: Regions in matches 0x" << hex
          << m_params.m_regionsInMatches
          << " over 0x"
          << m_params.m_overviewRegionsInMatches << dec << endl;

   mc2dbg8 << "[SH]: YYY: Searchstring = \"" << m_searchString << "\"" << endl;
   
   DEBUG1( printData(); );
}


SearchHandler::~SearchHandler() {
   DEBUG4(mc2dbg4 << "SearchHandler destructor" << endl;);
   uint32 i = 0;
   delete m_answer;

   // delete the packets in m_matchInfoReplies
   STLUtility::deleteValues( m_matchInfoReplies );
   
   // delete the overviewmatches
   for ( i = 0 ; i < m_unexpandedOverviewMatches.size(); i++ ) {
      delete m_unexpandedOverviewMatches[i];
   }

   // delete the expanded overviewMatches
   for ( i = 0 ; i < m_expandedOverviewMatches.size(); i++ ) {
      delete m_expandedOverviewMatches[i];
   }
   
   
   // delete the overViewReplies
   for( map<uint32, PacketContainer*>::iterator it = m_overviewReplies.begin();
        it != m_overviewReplies.end();
        ++it ) {
      delete it->second;
   }

   // delete the requestPackets
   for ( i = m_currentRequestIndex ; i < m_nbrRequestPackets ; i++ ) {
      delete m_requestPackets[ i ];
   }
   delete [] m_requestPackets;
   delete [] m_requestPacketIDs;
   // delete the replyPackets.
   for ( i = 0; i < m_nbrReceivedPackets; i++) {
      delete m_replyPackets[ i ];
   }
   delete [] m_replyPackets;

   if ( m_matches != NULL ) {
      for (uint32 i = 0; i < m_nbrReceivedPackets; i++) {
         MatchLink* link = m_matches[ i ];
         MatchLink* next = NULL;
         while ( link != NULL ) {
            next = link->getNext();
            delete link->getMatch();
         delete link;
         link = next;
         }
      }
      delete [] m_matches;
   }
 
   // delete the expanditem packets
   delete m_expandItemCont;
   delete m_expandItemReply;

   // Delete the things from decodeRequest
   delete [] m_locations;
   delete [] m_categories;
   
   // There could be something in the packetcontainertree
   for ( PacketContainer* pc = dequeFromTree();
         pc != NULL;
         pc = dequeFromTree() ) {
      delete pc;
   }
   delete m_packetsReadyToSend;
   delete m_coordinateHandler;
}


uint32
SearchHandler::getStatus() const
{
   if ( m_answer == NULL ) {
      return StringTable::TIMEOUT_ERROR;
   } else {
      return ((ReplyPacket*)(m_answer->getPacket()))->getStatus();
   }
}

// FIXME: Spelliong
PacketContainer*
SearchHandler::dequeFromTree()
{
    // The packet container to return
   PacketContainer* retPacketCont = m_packetsReadyToSend->getMin();
   if ( retPacketCont != NULL ) {
      m_packetsReadyToSend->remove( retPacketCont );
   }

   return retPacketCont; 
}

void
SearchHandler::printData()
{
   mc2dbg8
      << "[SH]: "
      << "m_zipCode=" << m_zipCode << " m_nbrLocations="
      << m_nbrLocations << " m_locations=" << m_locations
      << " m_locationType=" << m_params.m_locationType
      << " m_searchString=" << m_searchString
      << " m_nbrHits=" << uint32(m_params.m_nbrHits)
      << " m_matchType=" << (uint16) m_params.m_matchType
      << " m_stringPart=" << (uint16) m_params.m_stringPart
      << " m_sortingType=" << (uint16) m_params.m_sortingType
      << " m_params.m_searchForTypes=" << m_params.m_searchForTypes
      << " m_reqLanguage=" << m_params.m_reqLanguage
      << " m_nbrSortedHits=" << uint32(m_params.m_nbrSortedHits) << endl;
}

inline bool
SearchHandler::idAllowed(const OverviewMatch* match,
                         const map<const OverviewMatch*, uint32>& mapIDs,
                         const set<uint32>& wholeMaps,
                         const set<IDPair_t>& allowedItems)
{
   // This functionality has been moved to the SearchModule
   return true;
}

void
SearchHandler::reduceOverviewMatches(vector<OverviewMatch*>& matches,
                                     uint32& nbrOverviewLocations,
                                     const SearchRequestParameters& params,
                                     const TopRegionMatch& topRegion,
                                     map<const OverviewMatch*, uint32>& mapIDs)
{
   mc2dbg4 << "[SH]: Entering reduceOverviewMatches" << endl;
   // This function could surely be optimized.

   // Do stuff with the ItemIDTree.
   const ItemIDTree& itemTree = topRegion.getItemIDTree();
      
   // Put the maps that are completely covered in the set.
   set<uint32> wholeMaps;
   itemTree.getWholeMaps(wholeMaps);
#ifdef DEBUG_LEVEL_2
   mc2dbg << "[SH]: Wholemaps: " << hex;
   copy(wholeMaps.begin(),
        wholeMaps.end(),
        ostream_iterator<uint32>(mc2dbg, " "));
   mc2dbg << dec << endl;
#endif

   // Get the itemIDs too.
   set<IDPair_t> items;
   itemTree.getAllItems( items );
   mc2dbg4 << "[SH]: Number of wholemaps = " << wholeMaps.size()
          << " items: " << items.size() << endl;
   
   // First check the ids and see if they are allowed due to map
   // restrictions.
   // Check if the map is allowed according to the topregion
   // The search module handles this now.
   set<uint32> allowedMapIndeces;
   for( uint32 i = 0; i < matches.size(); ++i ) {
      bool allowed = idAllowed(matches[i], mapIDs, wholeMaps, items);
      if ( allowed ) {
         allowedMapIndeces.insert(i);
      }
   }
   
   // Check for uniqueOrFull (only look among the allowed ones).
   bool uniqueOrFullFound = false;
   bool fullFound = false;
   for( set<uint32>::const_iterator it = allowedMapIndeces.begin();
         it != allowedMapIndeces.end();
         ++it ) {
      if ( matches[*it]->fromUniqueOrFullMatchPacket() ) {
         // Good
         uniqueOrFullFound = true;
      }
      if ( matches[*it]->fromFullMatchPacket() ) {
         // Best
         fullFound = true;
         break;
      }
   }
   
   // Insert the full, uniqueorfull or not into the set.
   set<uint32> allowedUniqueOrFull;
   for ( set<uint32>::const_iterator it = allowedMapIndeces.begin();
         it != allowedMapIndeces.end();
         ++it ) {
      if ( fullFound ) {
         // Full is best. Keep it if it exists.
         if ( matches[*it]->fromFullMatchPacket() ) {
            allowedUniqueOrFull.insert(*it);
         }
      } else if ( uniqueOrFullFound ) {
         // Keep the ones that are 
         if ( matches[*it]->fromUniqueOrFullMatchPacket() ) {
            allowedUniqueOrFull.insert(*it);
         }
      } else {
         // No unique or full found. All are equally bad.
         allowedUniqueOrFull.insert(*it);
      }
   }

   
   // Start mask to check for. Will be converted to uint8
   // so the first one will be 0, which is good.
   uint16 currentMask = uint16(1) << 8;
   
   set<uint32> keepIndeces;
   
   // Go through the vector once and check if the mask is ok.
   // FIXME: Should really only iterate over the intersection of
   //        allowedMapIndeces and allowedUniqueOrFull.
   while ( keepIndeces.empty() && currentMask ) {
      mc2dbg4 << "[SH]: Current mask is " << hex
             << currentMask << dec << endl;
      for ( uint32 i = 0; i < matches.size(); ++i ) {
         const OverviewMatch* match = matches[i];
         if ( allowedMapIndeces.find( i ) != allowedMapIndeces.end() ) {
            // ok
         } else {
            mc2dbg4 << "[SH]: Removing match because of wrong mapID "
                   << endl;
            continue; // Next, please
         }
         
         // It may be allowed even if it is not unique or full.
         if ( allowedUniqueOrFull.find( i ) != allowedUniqueOrFull.end() ||
              !params.m_uniqueOrFull) {
            // ok
         } else {
            mc2dbg4 << "[SH]: Removing match "
                   << match->getName() << ":" 
                   << IDPair_t( match->getMapID(),
                                match->getItemID())
                   << " because not unique or full"
                      " or somthing" << endl;
            continue; // Next, please
         }
         
         // Check the quality of the hit
         if ( (currentMask & 0xff) == 0 ) {
            // Check for zero.
            if ( ( match->getRestrictions() == 0 ) ) {
               mc2dbg4 << "[SH]: Should keep "
                       << matches[i]->getName0()
                       << endl;
               keepIndeces.insert(i);
            }
         } else {
            // Use the mask
            if ( (matches[i]->getRestrictions() & currentMask) ) {
               mc2dbg4 << "[SH]: Should keep (2) "
                       << matches[i]->getName0()
                       << endl;
               keepIndeces.insert(i);
            }
         }
      }
      // Use worse matching.
      currentMask = currentMask >> 1;
   }

   // Check nbr removed characters, and removed all the matches which
   // have greater nbr of removed characters than the minimum nbr of
   // removed characters.
   uint8 minNbr = MAX_UINT8;
   for(vector<OverviewMatch*>::iterator it = matches.begin();
       it != matches.end(); it++) {
      OverviewMatch* currMatch = *it;
      uint8 currNbr = currMatch->getNbrRemovedCharacters();
      if( currNbr < minNbr )
         minNbr = currNbr;
   }
   
   // Fix vector, map and nbrOverviewLocations.
   nbrOverviewLocations = 0;
   vector<OverviewMatch*> newVect;
   map<const OverviewMatch*, uint32> newMap;
   for ( uint32 i = 0; i < matches.size(); ++i ) {
      OverviewMatch* match = matches[i];
      uint8 nbrRemovedCharacters = match->getNbrRemovedCharacters();
      if ( (keepIndeces.find(i) != keepIndeces.end()) &&
           (nbrRemovedCharacters == minNbr) )
      {
         // Good - keep
         newVect.push_back( match );
         newMap[match] = mapIDs[match];
         uint32 nbrMasks = 1; // Can only be one
         nbrOverviewLocations += nbrMasks;
      } else {
         // Bad - remove
         mc2dbg4 << "[SH]: \"" << match->getName0()
                << "\" has to many restrictions - removing " << endl;
         // Not good - delete
         delete match;
      }
   }   
   matches = newVect;
   mapIDs  = newMap;
   mc2dbg4 << "[SH]: reduce : nbr overviewmatches " << newVect.size() << endl;  
}


void
SearchHandler::handleAllOverviewsReceived()
{
   mc2dbg4 << "[SH]: handleAllOverviewsReceived" << endl;
   // All overview replies received - prepare for sending.
   
   // Check if there are matches without restrictions and remove the others.
   reduceOverviewMatches( m_expandedOverviewMatches, 
                          m_nbrOverviewLocations,
                          m_params,
                          *m_topRegion,
                          m_overviewMapIDsByOverviewMatch);

   // Do as before.
   // Vectors to be used for masks and similar.
   /*
   int nbrOver = m_expandedOverviewMatches.size();
   Vector* mapIDs          = new Vector ( nbrOver, 10 );
   Vector* masks           = new Vector ( nbrOver, 10 );
   Vector* maskItemIDs     = new Vector ( nbrOver, 10 );
   StringVector* maskNames = new StringVector ( nbrOver, 10 );
   */
   mc2dbg4 << "[SH]: handleAllOverviewsReceived "
      "size of m_expandedOverviewMatches" << m_expandedOverviewMatches.size()
          << endl;

   /*
   for ( uint32 i = 0; i < m_expandedOverviewMatches.size(); ++i ) {
      OverviewMatch* match =
         static_cast<OverviewMatch*>(m_expandedOverviewMatches[i]);
      
      mapIDs->addLast( match->getMapID() );
      masks->addLast( MAX_UINT32 );
      maskItemIDs->addLast( match->getItemID() );
      maskNames->addLast( (char*)match->getName0() );
   }
   */
   if ( (( !m_params.m_searchOnlyIfUniqueOrFull) ||
         ( m_params.m_searchOnlyIfUniqueOrFull && getUniqueOrFull() ) ) &&
        /* makeSearchMaskPackets( 
           mapIDs->getBuffer(), 
           mapIDs->size(), 
           masks->getBuffer(), maskItemIDs->getBuffer(),
           (const char**)maskNames->getBuffer(),
           maskNames->size() ) */
        makeSearchMaskPackets(m_expandedOverviewMatches)) {
      // Check if there are more overview replies to
      // be received.
      mc2dbg4 << "[SH]: Going into SEND_SEARCH" << endl;
      SETSTATE(SEND_SEARCH);
   } else {
      if ( m_params.m_lookupCoordinates ) {
         mc2dbg4 << "[SH]: Going into COORDINATES" << endl;
         SETSTATE(COORDINATES);
      } else {
         mc2dbg4 << "[SH]: Going into UNEXPAND_OVERVIEW" << endl;
         SETSTATE(UNEXPAND_OVERVIEW);
      }
      // Empty answer
      VanillaSearchReplyPacket* p = 
         new VanillaSearchReplyPacket( m_searchPacket );
      // Set status OK
      p->setStatus( StringTable::OK );
      delete m_answer; // Might be set
      m_answer = new PacketContainer( 
         p, 0, 0, MODULE_TYPE_INVALID );
   }
   /*
   delete mapIDs;
   delete masks;
   delete maskItemIDs;
   delete maskNames;
   */
}

inline SearchHandler::state
SearchHandler::initCoordinates()
{
   m_coordinateHandler = new CoordinateOnItemObject( m_request,
                                                     m_params.m_bboxRequested,
                                                     m_matchVector.size()+
                                                     getNbrOverviewMatches());

   // Add the overview matches.
   for( uint32 i = 0; i < getNbrOverviewMatches(); ++i ) {
      // Use the overview map id:s so that we do not have to
      // load lowerlevel maps
      if ( getOverviewMatch(i)->getOverviewMapID() != MAX_UINT32 ) {
         uint32 idx =
            m_coordinateHandler->add( getOverviewMatch(i)->getOverviewMapID(),
                                      getOverviewMatch(i)->getOverviewItemID(),
                                      MAX_INT16);
         m_lookupCoordsMap.insert( make_pair(idx,
                                             getOverviewMatch(i)));
      } else {
         mc2dbg4 << "[SH]: OvmapID = MAX_UINT32 - prolly added by user"
                << endl;
         // FIXME: Better way of detecting which matches were added by
         // user. Now we will have to look them up anyway
         uint32 idx =
            m_coordinateHandler->add( getOverviewMatch(i)->getMapID(),
                                      getOverviewMatch(i)->getItemID(),
                                      MAX_INT16);
         m_lookupCoordsMap.insert( make_pair(idx,
                                             getOverviewMatch(i)));
         
      }
   }
   
   // Add the Vanilla matches.
   for ( uint32 i = 0 ; i < m_matchVector.size() ; i++ ) {
      mc2dbg8 << "[SH]: Adding match with id "
              << IDPair_t( m_matchVector[i]->getMapID(),
                           m_matchVector[i]->getItemID()) << endl;
      uint16 offset = 0x7fff;
      uint32 itemID = m_matchVector[ i ]->getMainItemID();
      if ( m_matchVector[ i ]->getType() == SEARCH_STREETS ) {
         VanillaStreetMatch* vsm = static_cast< VanillaStreetMatch* > ( 
            m_matchVector[ i ] );
         offset = vsm->getOffset();
         if ( vsm->getStreetNbr() != 0 && 
              vsm->getStreetSegmentID() != MAX_UINT32 ) {
            itemID = vsm->getStreetSegmentID();
         }
      } else if ( m_matchVector[ i ]->getType() == SEARCH_COMPANIES ) {
         offset = static_cast< VanillaCompanyMatch* > ( 
            m_matchVector[ i ] )->getOffset(); 
      }
      uint32 idx = m_coordinateHandler->add( m_matchVector[ i ]->getMapID(),
                                             itemID,
                                             offset );
      m_lookupCoordsMap.insert( make_pair(idx,
                                          m_matchVector[i]) );
   }
   if ( m_coordinateHandler->requestDone() ) {
      return UNEXPAND_OVERVIEW;
   } else {
      return COORDINATES;
   }
}

inline SearchHandler::state
SearchHandler::initOverviewExpand()
{
   // Sort the overviewmatches by mapID and put expandpackets into queue

   // Put the mapIDs into the set or copy the underview matches
   mc2dbg4 << "[SH]: Size of m_unexpandedOverviewMatches = "
          << m_unexpandedOverviewMatches.size() << endl;
   set<uint32> ovMapIDs;
   for ( uint32 i = 0; i < m_unexpandedOverviewMatches.size(); ++i ) {
      uint32 curMap = m_unexpandedOverviewMatches[i]->getMapID();
      // All the matches must be expanded. They may have radii.
      if ( true || MapBits::isOverviewMap( curMap) ) {
         ovMapIDs.insert(m_unexpandedOverviewMatches[i]->getMapID());
      } else {
         // Have to copy the non-overview-id overviewmatch
         m_expandedOverviewMatches.push_back(
            new OverviewMatch(*m_unexpandedOverviewMatches[i]));
      }
   }

   // Init
   m_nbrOverviewMatchesToExpand = 0;
   
   for( set<uint32>::const_iterator it = ovMapIDs.begin();
        it != ovMapIDs.end();
        ++it ) {
      // This may seem strange, but the packet will only add matches
      // if match->getMapID == mapID
      SearchExpandItemRequestPacket* reqPacket =
         new SearchExpandItemRequestPacket(m_unexpandedOverviewMatches,
                                           *it,
                                           m_request->getNextPacketID(),
                                           m_request->getID() );
      
      m_packetsReadyToSend->add(new PacketContainer(reqPacket,
                                                    0,
                                                    0,
                                                    MODULE_TYPE_SEARCH,
                                                    SEI_TIMEOUT,
                                                    4));
      m_nbrOverviewMatchesToExpand++;
   } 

   // If there were no overview id:s we must go into SEND_SEARCH.
   if ( ovMapIDs.empty() ) {      
      mc2dbg4 << "[SH]: Size of m_expandedOverviewMatches = "
             << m_expandedOverviewMatches.size() << endl;

      // We can start searching if the user wants to search all the hits
      // or there is only one hit and there are not zero hits.
      const bool gotoSearch = ( (!m_params.m_searchOnlyIfUniqueOrFull) ||
                               getUniqueOrFull() ) &&
         ( !m_expandedOverviewMatches.empty() );
      
      if ( gotoSearch ) {
         makeSearchMaskPackets(m_expandedOverviewMatches);
         SETSTATE(SEND_SEARCH);
         return SEND_SEARCH;
      } else {         
         if ( m_params.m_lookupCoordinates ) {
            SETSTATE(COORDINATES);
            return COORDINATES;
         } else {
            SETSTATE(UNEXPAND_OVERVIEW);
            return UNEXPAND_OVERVIEW;
         }
      }
   } else {
      // Keep state.
      return m_state;
   }
   mc2log << warn << "[SH]: Should not be here 2" << endl;
   return m_state;
}

inline SearchHandler::state
SearchHandler::processOverviewExpand(PacketContainer* pack)
{
   SearchExpandItemReplyPacket* reply =
      static_cast<SearchExpandItemReplyPacket*>(pack->getPacket());

   // Move this functionality to the packet.
   multimap<uint32, OverviewMatch*> expandedItems;
   reply->getItems(expandedItems);

   // The result is a multimap with the index of the old match in first
   // and underview item in second.
   for( multimap<uint32, OverviewMatch*>::iterator it =
           expandedItems.begin();
        it != expandedItems.end();
        /**/ ) {
      OverviewMatch* match = m_unexpandedOverviewMatches[it->first];
      OverviewMatch* expMatch = new OverviewMatch(*match);
      expMatch->setMapID(it->second->getMapID());
      expMatch->setItemID(it->second->getItemID());     
      expMatch->setRadiusMeters(it->second->getRadiusMeters());
      expMatch->setName(it->second->getName());
      expMatch->setCoords(it->second->getCoords());
      // Why this(setting coord in unexpaned)? To set a coord if none?
      match->setCoords(it->second->getCoords());
      mc2dbg4 << "[SH]: Adding expanded ov " << *expMatch << endl;
      m_expandedOverviewMatches.push_back(expMatch);
      // Add the index back to m_unexpandedOverviewMatches
      m_expandedToUnexpandedOverviewIndex.push_back( it->first );
      mc2dbg4 << "[SH]: Expanded " << *match << " to " << *expMatch << endl;
      // Delete the match created by the packet
      delete it->second;
      expandedItems.erase(it++);
   }
   
   
   if ( (--m_nbrOverviewMatchesToExpand) <= 0 ) {
      // m_nbrOverviewMatchesToExpand must not be -1
      if ( m_nbrOverviewMatchesToExpand == -1 ) {
         m_nbrOverviewMatchesToExpand++;
      }
      if ( m_state == OVERVIEWEXPAND_THEN_OVERVIEW_CHECK ) {         
         handleAllOverviewsReceived();
         return m_state;
      } else {
         if ( makeSearchMaskPackets(m_expandedOverviewMatches) ) {
            return SEND_SEARCH;
         } else {
            return UNEXPAND_OVERVIEW;
         }
      }
   } else {
      // Not done - keep state.
      return m_state;
   }
   mc2log << warn << "[SH]: Should not be here 1" << endl;
   return m_state;
}

inline SearchHandler::state
SearchHandler::handleAllCoordinatesReceived()
{
   mc2dbg2 << "[SH]: All coords received" << endl;

    // Get results
   uint32 mapID = 0;
   uint32 itemID = 0;
   uint16 offset = 0;
   int32 lat = 0;
   int32 lon = 0;
   bool hasLat = false;
   MC2BoundingBox bbox;
   for ( uint32 i = 0 ; i < m_coordinateHandler->getNbrItems() ; i++ ) {
      m_coordinateHandler->getResultWithBoundingbox(
         i, mapID, itemID, offset, lat, lon, hasLat, bbox );
      mc2dbg2 << "[SH]: Coords for " << mapID << ":" << hex
             << itemID << dec << " are " << lat << ":" << lon << endl;
      mc2dbg2 << "[SH]: BBox is " << bbox << endl;
      map<uint32, SearchMatch*>::const_iterator it =
         m_lookupCoordsMap.find(i);
      if ( it != m_lookupCoordsMap.end() ) {
         mc2dbg8 << "[SH]: Match found" << endl;
         if ( hasLat ) {
            it->second->setCoords(lat,lon);
            it->second->setBBox(bbox);
         }
      } else {
         mc2dbg2 << "[SH]: Match not found" << endl;
      }
   }

   // Do the distance sorting if it should be done
   if ( (m_params.getSortingType() == SearchTypes::DistanceSort) ||
        (m_params.getSortingType() == SearchTypes::ConfidenceSort ) ) {
      if ( m_sortOrigin.isValid() ) {
         calcDistsAndSort(m_unexpandedOverviewMatches,
                          m_sortOrigin,
                          m_params.getSortingType() );
         calcDistsAndSort(m_matchVector,
                          m_sortOrigin,
                          m_params.getSortingType() );
      }
   }   

   return UNEXPAND_OVERVIEW;
}

inline void
SearchHandler::handleTopRegionReceivedInUnexpand()
{
   set<uint32> lowerMaps;
   // Make vector of all SearchMatches that we have...
   // If it is possible we should maybe add some check for which
   // matches to expand.
   for( uint32 i = 0; i < m_matchVector.size(); ++i ) {
      VanillaMatch* curMatch = m_matchVector[i];
      m_matchesForUnexpansion.push_back(curMatch);
      lowerMaps.insert(curMatch->getMapID());
      // Add the region matches too. (They are most important, really)
      for( uint32 j = 0; j < curMatch->getNbrRegions(); ++j ) {
         m_matchesForUnexpansion.push_back(curMatch->getRegion(j));
         lowerMaps.insert(curMatch->getRegion(j)->getMapID());
      }
   }

/*   
   // Top region data
   TopRegionMatchesVector topRegions;

   // Get Top Region data
   topRep->getTopRegions( topRegions );
*/
   // Figure which maps to send to.
   map<uint32,uint32> underToOverMap;

   // Maps to send packets to
   set<uint32> packetMaps;
   
   // Fill in map ID Trans map.
   vector<uint32> overviewMaps;
   for ( set<uint32>::const_iterator it = lowerMaps.begin();
         it != lowerMaps.end();
         ++it ) {
      if ( ! MapBits::isUnderviewMap(*it) ) {
         // Must be underview map.
         continue;
      }
      for ( uint32 i = 0 ; i < m_topRegionRequest->getNbrTopRegions(); ++i ) {
         overviewMaps.clear();
         if ( m_topRegionRequest->getTopRegion(i)->getItemIDTree().
              getOverviewMapsFor(*it, overviewMaps) ) {
            underToOverMap[*it] = overviewMaps.back();
            packetMaps.insert(overviewMaps.back());
            break;
         }        
      }
   }

/*
   // Delete top regions
   for ( uint32 i = 0 ; i < topRegions.size(); ++i ) {
      delete topRegions[ i ];
   }
*/
   // Reuse counter
   m_nbrOverviewSearchesLeft = 0;
   
   mc2dbg2 << "[SH]: Nbr overview maps " << underToOverMap.size()
          << endl;
   for( set<uint32>::const_iterator it = packetMaps.begin();
        it != packetMaps.end();
        ++it ) {
      mc2dbg2 << "[SH]: Enqueueing packet for map "
             << prettyMapIDFill(*it) << endl;
      SearchExpandItemRequestPacket* reqPack =
         new SearchExpandItemRequestPacket(m_matchesForUnexpansion,
                                           underToOverMap,
                                           *it,
                                           m_request->getNextPacketID(),
                                           m_request->getID() ) ;
      m_packetsReadyToSend->add( new PacketContainer(reqPack,
                                                     0,
                                                     0,
                                                     MODULE_TYPE_SEARCH,
                                                     SEI_TIMEOUT,
                                                     4) );
      // Reuse the old counter...
      m_nbrOverviewSearchesLeft++;
   }
}

inline void
SearchHandler::sendMatchInfoPacket(VanillaSearchReplyPacket* packet)
{
   vector<VanillaMatch*> packetMatches;
   
   packet->getAllMatches(packetMatches);

   if ( packetMatches.empty() ) {
      return;
   }
   
   MatchInfoRequestPacket* reqPack =
      new MatchInfoRequestPacket(packetMatches,
                                 m_params.getRegionsInMatches(),
                                 m_params.getRequestedLang(),
                                 packetMatches.front()->getMapID(),
                                 m_request->getNextPacketID(),
                                 m_request->getID() ) ;
   
   if ( reqPack->getNbrItems() != 0 ) {      
      m_packetsReadyToSend->add( new PacketContainer( reqPack,
                                                      0,
                                                      0,
                                                      MODULE_TYPE_MAP ) );
      m_nbrExpectedMatchInfoPackets++;
   } else {
      // Generated no items
      delete reqPack;
   }
   
   // Usch. We have to delete all the matches. It would have
   // been nice to keep them.
   STLUtility::deleteValues( packetMatches );
}

inline void
SearchHandler::handleUnexpandItemPackets(const ReplyPacket* rep)
{
   const SearchExpandItemReplyPacket* sei =
      static_cast<const SearchExpandItemReplyPacket*>(rep);
   
   mc2dbg2 << "[SH]: handleUnexpandItemPackets" << endl;
   multimap<uint32, IDPair_t> expandedItems;
   sei->getItems(expandedItems);
   mc2dbg2 << "[SH]: Nbr unexpanded items " << expandedItems.size()
          << endl;

   for ( multimap<uint32, IDPair_t>::const_iterator it(expandedItems.begin());
         it != expandedItems.end();
         ++it ) {
      mc2dbg2 << "[SH]: Exchanging " <<
         IDPair_t(m_matchesForUnexpansion[it->first]->getMapID(),
                  m_matchesForUnexpansion[it->first]->getItemID())
             << " for " << it->second << endl;
      m_matchesForUnexpansion[it->first]->setMapID(it->second.getMapID());
      m_matchesForUnexpansion[it->first]->setItemID(it->second.getItemID());
   }
   
   if ( --m_nbrOverviewSearchesLeft == 0 ) {
      // Go through the matches and remove duplicate regions
      for( uint32 i = 0; i < m_matchVector.size(); ++i ) {
         bool regionWasRemoved = false;
         do {
            regionWasRemoved = false;
            set<IDPair_t> existingRegions;
            for( uint32 j = 0; j < m_matchVector[i]->getNbrRegions(); ++j ) {
               VanillaMatch* curRegion = m_matchVector[i]->getRegion(j);
               IDPair_t curID( curRegion->getMapID(),
                               curRegion->getItemID() );
               if ( existingRegions.find( curID ) == existingRegions.end() ) {
                  mc2dbg8 << "[SH]: Inserting region " << curID << endl;
                  existingRegions.insert( curID );
               } else {
                  mc2dbg2 << "[SH]: Removing dup-region "
                         << curID << endl;
                  m_matchVector[i]->removeRegion(j);
                  regionWasRemoved = true;
                  break; // Break the for loop. Indeces have changed.
               }
            }
         } while ( regionWasRemoved == true );
      }
      // FIXME: Remove duplicate ordinary hits too.
      m_matchesForUnexpansion.clear();
      SETSTATE(checkMatchInfosReceived());
   }
}

SearchHandler::state
SearchHandler::checkMatchInfosReceived()
{
   int nbrReceived = m_matchInfoReplies.size();
   if ( nbrReceived == m_nbrExpectedMatchInfoPackets ) {
      for( int i = 0; i < nbrReceived; ++i ) {
         m_matchInfoReplies[i]->fixupMatches(m_matchVector);
      }
      // Sort the matches again
      if ( m_sortOrigin.isValid() ) {
         calcDistsAndSort(m_unexpandedOverviewMatches,
                          m_sortOrigin,
                          m_params.getSortingType() );
         calcDistsAndSort(m_matchVector,
                          m_sortOrigin,
                          m_params.getSortingType() );
      } else {
         SearchSorting::sortSearchMatches(m_matchVector,
                                          m_params.getSortingType(),
                                          m_params.getNbrSortedHits());
      }
      return DONE;
   } else {
      return ALMOST_DONE_2;
   }
}

bool
SearchHandler::processPacket( PacketContainer* cont ) {
   if ( cont != NULL ) {
      DEBUG8(mc2dbg8 << "SearchHandler::processPacket received packetID: " 
             << cont->getPacket()->getPacketID() << endl;);

      // Receive this kind of packet in any state.
      if ( cont->getPacket()->getSubType() ==
           Packet::PACKETTYPE_MATCHINFOREPLY) {
         mc2dbg4 << "[SH]: Got matchinfo reply number "
                << (m_matchInfoReplies.size()+1) << " of "
                << m_nbrExpectedMatchInfoPackets << endl;
         // Save the packet and delete the container.
         m_matchInfoReplies.push_back(
            static_cast<MatchInfoReplyPacket*>(
               cont->getPacket()->getClone(false)));
         delete cont;
         if ( m_state == ALMOST_DONE_2 ) {
            SETSTATE(checkMatchInfosReceived());
         }
         return true;
      }
      
      switch ( m_state ) {
         case START :
            DEBUG2(mc2dbg8 << "SearchHandler::processPacket START: " 
                   << "Didn't expect packet: " << endl;
                   cont->getPacket()->dump() );
            return false;
            break;
         case SEND_OVERVIEW :
         case AWAIT_OVERVIEW :
            if ( cont->getPacket()->getSubType() == 
                  Packet::PACKETTYPE_OVERVIEWSEARCHREPLYPACKET) {

               // Take care of this packet.
               { // New scope to avoid using the variables further down.
                  mc2dbg2 << "[SH]: Overview packet received" << endl;
                  // Save the reply.
                  uint32 packetID = cont->getPacket()->getPacketID();
                  m_overviewReplies[cont->getPacket()->getPacketID()] = cont;
                  // Decrease the number of packets we're waiting for.
                  --m_nbrOverviewSearchesLeft;
                  OverviewSearchReplyPacket* overviewReply =
                     static_cast<OverviewSearchReplyPacket*>
                     ( cont->getPacket() );
                  cont = NULL;
                  
                  mc2dbg4 << "[SH]: " << m_overviewReplies.size()
                          << " overviews received" << endl;
                  mc2dbg4 << "[SH]: " << m_nbrOverviewSearchesLeft
                          << " overviews to go" << endl;
                  mc2dbg4 << "[SH]: getUniqueOrFull returns "
                          << overviewReply->getUniqueOrFull()
                          << " and getFull returns "
                          << overviewReply->getFull() << endl;
                     
                  // Extract masks and mapids and call the function...
                  int pos;
                  int currentNbrMatches = 0;
                  OverviewMatch* match = static_cast<OverviewMatch*>(
                     overviewReply->getFirstMatch( pos, currentNbrMatches ) );
                  
                  for ( int32 j = 0 ; j < currentNbrMatches ; j++ ) {
                     if ( match == NULL ) {
                        mc2dbg2 << "SearchHandler::processPacket "
                               << "OverViewMatchs was NULL!" << endl;
                        continue;
                     }
                     mc2dbg8 << "[SH]: Adding overviewmatch to vector" << endl;
                     mc2dbg8 << "[SH]: Match has " << match->getPoints()
                             << " points and the restrictions are 0x" << hex
                             << uint32(match->getRestrictions())
                             << dec << endl;
                     m_unexpandedOverviewMatches.push_back( match );

                     // Keep track of MAPID.
                     map<uint32, uint32>::const_iterator mapIDIt =
                        m_overviewRequestMaps.find( packetID );
                     if ( mapIDIt != m_overviewRequestMaps.end() ) {
                        uint32 ovMapID = mapIDIt->second;
                        mc2dbg4 << "[SH]: Found map ID 0x" 
                                << prettyMapIDFill(ovMapID) << " in map " 
                                << endl;
                        m_overviewMapIDsByOverviewMatch.insert(
                           make_pair(match, ovMapID) );
                     } else {
                        mc2log << warn << "[SH]: Did not find map ID in map"
                               << endl;
                     }
                     
                     m_nbrOverviewLocations++;
                     if ( j < currentNbrMatches ) {
                        match =  static_cast<OverviewMatch*>(
                           overviewReply->getNextMatch( pos ) );
                     }
                  }
               }
               
               // Handle all packets received.
               
               mc2dbg4 << "SearchHandler::processPacket "
                           "nbrLocationMatches "
                       << m_unexpandedOverviewMatches.size()
                       << " nbrOverviewLocations " 
                       << m_nbrOverviewLocations << endl;
               
               if ( m_nbrOverviewSearchesLeft == 0 ) {
                  reduceOverviewMatches( m_unexpandedOverviewMatches,
                                         m_nbrOverviewLocations,
                                         m_params,
                                         *m_topRegion,
                                         m_overviewMapIDsByOverviewMatch);
                  if ( m_overviewReplies.size() > 1 ) {
                     // Sorting may be needed now.
                     calcDistsAndSort( m_unexpandedOverviewMatches,
                                       m_sortOrigin,
                                       m_params.getSortingType() );
                  }
                  SETSTATE(OVERVIEWEXPAND_THEN_OVERVIEW_CHECK);
                  SETSTATE(initOverviewExpand());                  
               } else {
                  // Keep waiting
               }
            } else {
               DEBUG2(mc2dbg2 << "SearchHandler::processPacket : "
                      "[Await|SEND]_OVERVIEW " << "Wrong packet: " << endl;
                      cont->getPacket()->dump() );
               DEBUG8(mc2dbg8 << "Expected overviewpackettype" 
                      << endl;);
               return false;
            }
            break;
         case SEND_SEARCH :
         case AWAIT_SEARCH: {
            // Shouldn't the server czech this?
            bool packetIDOk = false;            
            for ( uint32 i = 0 ; i < m_nbrRequestPackets ; i++ ) {
               if ( cont->getPacket()->getPacketID() == 
                    m_requestPacketIDs[ i ] ) {
                  packetIDOk = true;
                  break;
               }
            }
            if ( packetIDOk &&
                 cont->getPacket()->getSubType() == 
                 Packet::PACKETTYPE_VANILLASEARCHREPLY ) {
               m_replyPackets[m_nbrReceivedPackets++] = cont;
               
               // Add matchinfopacket to the outgoing queue.
               sendMatchInfoPacket(static_cast<VanillaSearchReplyPacket*>
                                   (cont->getPacket()));
               cont = NULL;
               
               if ( m_nbrReceivedPackets >= m_nbrRequestPackets ) {
                  mc2dbg2 << "[SH]: All " << m_nbrReceivedPackets 
                          << " SearchPackets received" << endl;
                  
                  // Check if we want to expand empty location to street
                  // If there is a region without names on the streets
                  // or if the user only searched for an area, we can
                  // return a street inside the area instead.
                  bool allEmptyLocations = true;
                  uint32 nbrMatches = 0;
                  for (uint32 i = 0; i < m_nbrReceivedPackets; i++) {
                     if ( !(static_cast<VanillaSearchReplyPacket *>
                            (m_replyPackets[i]
                             ->getPacket())->getLocationEmptyHack() ) ) {
                        allEmptyLocations = false;
                     }
                     nbrMatches += (static_cast<VanillaSearchReplyPacket *>
                                    (m_replyPackets[i]->getPacket()))
                        ->getNumberOfMatches();
                  }
                  mc2dbg4 << "[SH]: Number of search matches: "
                         << nbrMatches << endl;

                  bool dont_do_it = ( getNbrOverviewMatches() == 1 ) &&
                     ( m_expandedOverviewMatches[0]->getItemType() ==
                     ItemTypes::zipCodeItem );

                  // Try to detect the UK.
                  if ( m_topRegion && m_topRegion->getID() != 0 ) {
                     dont_do_it = false;
                  }

                  // Ah. Do it
                  dont_do_it = false;
                       
                  // 
                  // If expand city center
                  // AND
                  // "do it" and nbr matches is zero
                  // AND 
                  // ( ( unique or full search ) AND have overview matches) OR
                  //   overview matches is exacly one ) 
                  // AND
                  // ( all empty location OR searchstring is empty )
                  //
                  // THEN expand city location overview
                  // 
                  if ( m_params.expandCityCenter() && 
                       ( ! dont_do_it ) && ( nbrMatches == 0 &&
                       ((getUniqueOrFull() &&
                         getNbrOverviewMatches() > 0) ||
                        getNbrOverviewMatches() == 1 ) &&
                       (allEmptyLocations || 
                        (m_searchString != NULL && 
                         m_searchString[0] == '\0'))) ) {
                     mc2dbg4 << "[SH]: processPacket : "
                                "[AWAIT|SEND]SEARCH expanding city"
                             << endl;
                     OverviewMatch* ov = m_unexpandedOverviewMatches[ 
                        m_expandedToUnexpandedOverviewIndex[ 0 ] ];
                     
                     m_expandItemPacketID = m_request->getNextPacketID();
                     CoordinateOnItemRequestPacket* p = new
                        CoordinateOnItemRequestPacket( m_expandItemPacketID,
                                                       m_request->getID() );
                     p->setMapID( ov->getMapID() );
                     p->add( ov->getItemID(), MAX_UINT32/*"Best" coord*/ );
                     m_expandItemCont = new PacketContainer( 
                        p, 0, 0, MODULE_TYPE_MAP );
                     SETSTATE( SEND_EXPAND_ITEM );
                     mc2dbg4 << "[SH]: Expanding " << *ov << endl;
                  } else { 
                     // Process packets and make answer
                     PacketContainer *result = NULL;
                     if ( m_nbrReceivedPackets == 1 ) { 
                        // Only one, nothing to be done
                        result = m_replyPackets[0];
                        // Save the matches for coordinate
                        m_replyPackets[0] = NULL;
                        m_matches = new MatchLink *[ m_nbrRequestPackets ];
                        int32 nbrMatches = 0;
                        if (result != NULL) {
                              m_matches[0] = 
                                 (static_cast<VanillaSearchReplyPacket *>
                                  (result->getPacket()))
                                 ->getMatchesAsLinks( nbrMatches );
                        }
                     } else {
                        mc2dbg << "[SH]: Merging starts" << endl;
                        // merge the replyPackets, by:
                        // unpacking the replyPackets into 
                        // linked lists of MatchLinks.
                        SearchTypes::SearchSorting sorting =
                           m_params.m_sortingType;
                        
                        m_matches = new MatchLink *[ m_nbrRequestPackets ];
                        int32 nbrMatches = 0;
                        uint32 totNbrMatches = 0;
                        for (uint32 i = 0; i < m_nbrReceivedPackets; i++) {
                           if (m_replyPackets[i] != NULL) {
                              m_matches[i] = 
                                 (static_cast<VanillaSearchReplyPacket *>
                                  (m_replyPackets[i]->getPacket()))
                                 ->getMatchesAsLinks( nbrMatches );
                              totNbrMatches += nbrMatches;
                           }
                           else {
                              mc2dbg8 << "replyPacket: " << i 
                                      << " == NULL " << endl;
                           }
                        }
                        // merge the result
                        merge( 0, m_nbrReceivedPackets-1, 0, 
                               m_matches, m_nbrReceivedPackets,
                               sorting );
                        mc2dbg4 << "[SH]: Merging ends. Nbr = "
                               << totNbrMatches
                               << endl;
                        // pack into a new replyPacket
                        VanillaSearchReplyPacket *replyPacket = 
                           new VanillaSearchReplyPacket( m_searchPacket,
                                                         m_nbrRequestPackets );

                        convMatchLinkToPacket( m_matches[0], replyPacket );

                        // Set status OK
                        replyPacket->setStatus( StringTable::OK );

                        // make packet container
                        result = new PacketContainer( 
                           replyPacket, 0, 0, 
                           MODULE_TYPE_INVALID );
                        mc2dbg4 << "[SH]: ResultPacket Nbr = "
                               << replyPacket->getNumberOfMatches()
                               << endl;
                     } // else
                     delete m_answer; // Might be set
                     m_answer = result;
                     // Create the vector of mathes
                     makeMatchVector();
                     if ( m_params.m_lookupCoordinates ) {
                        mc2dbg2 << "[SH]: Going into COORDINATES" << endl;
                        SETSTATE(COORDINATES);
                     } else {
                        SETSTATE(UNEXPAND_OVERVIEW);
                     }
                  } // else
               } // if all packets received
            } else { // If correct packet
               DEBUG4(mc2dbg4 << "SearchHandler::processPacket wrong packet:"
                      << endl;
                      cont->getPacket()->dump());
               return false;
            }
            break;
         }
         case SEND_EXPAND_ITEM :
            mc2dbg2 << "SearchHandler::processPacket : "
               "SEND_EXPAND_ITEM " << "received packet." << endl;
            break;                
         case OVERVIEWEXPAND:
         case OVERVIEWEXPAND_THEN_OVERVIEW_CHECK:
            // This function will do what's necessary after all are
            // received.
            SETSTATE(processOverviewExpand( cont ));
            break;
         case COORDINATES: {
            m_coordinateHandler->processPacket(cont);
            // CoordinateOnItemObject deletes the container.
            cont = NULL;
            if ( m_coordinateHandler->requestDone() ) {
               SETSTATE(handleAllCoordinatesReceived());
            }
            break;
         }
         case AWAIT_EXPAND_ITEM :
            // This is where we convert locations without streetnames
            // or searches for nothing but location into a street inside
            // the location.
            if ( cont->getPacket()->getPacketID() ==
                 m_expandItemPacketID && 
                 cont->getPacket()->getSubType() == 
                 Packet::PACKETTYPE_COORDINATEONITEMREPLY ) {
               DEBUG2(mc2dbg2 << "SearchHandler::processPacket : "
                      "AWAIT_EXPAND_ITEM expanditempacket received" 
                      << endl;);
               PacketContainer* result = NULL;
               CoordinateOnItemReplyPacket* p = 
                  static_cast< CoordinateOnItemReplyPacket* > ( 
                     cont->getPacket() );
               if ( p->getNbrItems() == 1 ) {
                  VanillaSearchReplyPacket* res = new 
                     VanillaSearchReplyPacket( m_searchPacket );
                  typedef const char* const_char_p;
                  const_char_p* names = new const char*[ 1 ];
                  uint32 mapID = MAX_UINT32;
                  uint32 location = MAX_UINT32;
                  uint32 itemID = MAX_UINT32;
                  MC2Coordinate coord;
                  OverviewMatch* match = m_expandedOverviewMatches[ 0 ];
                  names[ 0 ] = match->getName0();
                  mapID = match->getMapID();
                  location = MAX_UINT32;
                  p->getLatLong( 0, itemID, coord.lat, coord.lon );
                  mc2dbg2 << "Adding city as street: " << endl
                         << hex
                         << "name:\t " << names[ 0 ] << endl
                         << "mapID:\t " << mapID << endl
                         << "itemID:\t " << itemID << endl
                         << "location:\t " << location << endl
                         << "coord:\t " << coord << endl
                         << dec;
                  VanillaStreetMatch* vsm = static_cast<VanillaStreetMatch*>(
                     SearchMatch::createMatch(SEARCH_STREETS,
                                              IDPair_t(mapID, itemID)));
                  vsm->setName(names[0]);
                  vsm->setLocationName(names[0]);
                  if ( coord.isValid() ) {
                     vsm->setCoords( coord );
                  } else {
                     // If no good coordinate from MM use expanded item's
                     vsm->setCoords( match->getCoords() );
                  }
                                              
                  res->addMatch( vsm );

                  delete vsm;
                  delete [] names;
                  // Set status OK
                  res->setStatus( StringTable::OK );  
                  result = new PacketContainer( 
                     res, 0, 0, 
                     MODULE_TYPE_INVALID );
               } else {
                  mc2dbg2 << "SearchHandler::processPacket : "
                            "AWAIT_EXPAND_ITEM " << "not one item: " 
                         << p->getNbrItems() << endl;
                  VanillaSearchReplyPacket* p = 
                     new VanillaSearchReplyPacket( m_searchPacket );
                  p->setStatus( StringTable::OK );
                  result = new PacketContainer( 
                     p, 0, 0, 
                     MODULE_TYPE_INVALID );
               }
               delete m_answer; // Might be set
               m_answer = result;
               m_nbrReceivedPackets = 1;
               m_matches = new MatchLink *[ m_nbrRequestPackets ];
               int32 nbrMatches = 0;
               m_matches[ 0 ] = 
                  ( static_cast<VanillaSearchReplyPacket *>
                    ( m_answer->getPacket() ) )
                  ->getMatchesAsLinks( nbrMatches );
               // Create the result vector
               makeMatchVector();
               if ( m_params.m_lookupCoordinates ) {
                  mc2dbg2 << "[SH]: Going into coords2" << endl;
                  SETSTATE( COORDINATES );
               } else {
                  SETSTATE( UNEXPAND_OVERVIEW );
               }
            } else {
               DEBUG4(mc2dbg4 << "SearchHandler::processPacket wrong packet:"
                      << endl;
                      cont->getPacket()->dump());
               return false; 
            }
            break;
         case UNEXPAND_OVERVIEW:
         case AWAIT_UNEXPAND_OVERVIEW: {
            mc2dbg2 << "[SH]: Got packet in " << stateAsString(m_state)
                   << endl;
            ReplyPacket* rep = static_cast<ReplyPacket*>(cont->getPacket());
            switch ( rep->getSubType() ) {
               case Packet::PACKETTYPE_TOPREGIONREPLY:
                  // May change state (to done if something went wrong).
                  //handleTopRegionReceivedInUnexpand(rep);
                  MC2_ASSERT(false);  // should never get here now!
                  break;
               case Packet::PACKETTYPE_SEARCHEXPANDITEMREPLY:
                  // May change state
                  handleUnexpandItemPackets(rep);
                  break;
               default:
                  mc2log << error << "[SH]: Wrong packettype "
                         << uint32(rep->getSubType())
                         << " in UNEXPAND_OVERVIEW" << endl;
                  SETSTATE(checkMatchInfosReceived());
                  break;
            }
         }
         break;
         case DONE :
            DEBUG2(mc2dbg2 << "SearchHandler::processPacket : "
                   "DONE " << "received packet." << endl;
//                   cont->getPacket()->dump() 
                   );
            return false;
            break;
         case ALMOST_DONE_2:
         break;
         case SEND_TOP_REGION:
         break;
         case AWAIT_TOP_REGION:
         break;
      };
      delete cont;
   }
   return true;
}


PacketContainer*
SearchHandler::getNextPacket() {
   if ( m_firstPacket ) {
      mc2dbg2 << "----------SearchHandler - firstPacket-----"
             << endl;
      m_firstPacket = false;
   }

   // First try to dequeue from the tree
   PacketContainer* res = dequeFromTree();
   if ( res != NULL ) {
      return res;
   }
      
   switch ( m_state ) {
      case START :
         mc2dbg2 << "SearchHandler::getNextPacket START " 
                << "nothing to send!" << endl;
         break;
      case SEND_TOP_REGION :
      case AWAIT_TOP_REGION : {      // get rid of these states, no longer used! FIXME
         set<uint32> overviewMapIDs;
         ItemIDTree itemTree;
         bool topRegionFound = false;
         for( uint32 i=0; i < m_topRegionRequest->getNbrTopRegions(); ++i ) {
            if ( m_topRegionRequest->getTopRegion(i)->getID() == (uint32)m_topRegionID) {
               topRegionFound = true;
               break;
            }
         }
         if ( ! topRegionFound ) {
            // Exit with error.
            if ( m_topRegionRequest->getNbrTopRegions() != 0) {
               SETSTATE(DONE);
               // Ugh. Packet
               delete m_answer;
               VanillaSearchReplyPacket* vsp =
                  new VanillaSearchReplyPacket(m_searchPacket);
               vsp->setStatus(StringTable::NOTOK);
               m_answer = new PacketContainer(vsp,0,0,MODULE_TYPE_MAP);
               mc2log << warn << "[SH]: Top region "
                      << uint32(m_topRegionID) << " not found" << endl;
               break;
               m_topRegionID =
                  StringTable::countryCode(m_topRegionRequest->getTopRegion(0)->getID());
            } else {
               mc2log << fatal << "[SH]: No topregions!!" << endl;
               abort();
               exit(1);
            }
         }
         
         for( uint32 i=0; i < m_topRegionRequest->getNbrTopRegions(); ++i ) {
            if ( m_topRegionRequest->getTopRegion(i)->getID() == (uint32)m_topRegionID ) {
               // Found it. Add the maps to the overview maps.
               // TODO: Check for underview maps and itemid:s too.
               itemTree = m_topRegionRequest->getTopRegion(i)->getItemIDTree();
               itemTree.getTopLevelMapIDs(overviewMapIDs);
               MC2_ASSERT( m_topRegion == NULL );
               m_topRegion = m_topRegionRequest->getTopRegion(i);
            }
         }
                                
         if (overviewMapIDs.empty()) {
            mc2log << warn << here << "[SH]: Could not find top region"
                   << " among all countries. Hardcoding overview"
                   << " to FIRST_OVERVIEWMAP_ID." << endl;
            overviewMapIDs.insert(FIRST_OVERVIEWMAP_ID);
         } else if ( overviewMapIDs.size() >= 1 ) {
            mc2dbg2 << "[SH]: Top level maps are: " << hex;
            for( set<uint32>::const_iterator it = overviewMapIDs.begin();
                 it != overviewMapIDs.end();
                 ++it ) {
               mc2dbg2 << *it << " ";
            }
            mc2dbg2 << dec << endl;
         }
         
         // Add the outgoing overview search requests to the packet
         // queue.
         static const vector<IDPair_t> emptyRegions;
         static const vector<MC2BoundingBox> emptyBBoxes;
         for( set<uint32>::const_iterator it = overviewMapIDs.begin();
              it != overviewMapIDs.end();
              /* Nothing */ ) {
            if ( MapBits::isOverviewMap(*it) ) {
               UserRightsMapInfo rights( *it,
                                         m_request->getUser(),
                                         ~MapRights() );
               const char* searchString = m_locations[0];
               SearchRequestPacket* p2 =
                  new SearchRequestPacket(
                     *it,
                     m_request->getNextPacketID(),
                     m_request->getID(),
                     SearchRequestPacket::OVERVIEW_SEARCH,
                     searchString, // Only one string per packet
                     m_overParams,
                     emptyRegions,
                     emptyBBoxes,
                     m_sortOrigin,
                     rights );
               
               // Add overview map id to map of overview maps.
               m_overviewRequestMaps[p2->getPacketID()] = *it;
               
               // Add. Reduces the number of member variables needed.
               m_packetsReadyToSend->add(
                  new PacketContainer( p2, 0, 0, 
                                       MODULE_TYPE_SEARCH ) );
               ++it; // Important
            } else {
               // Remove it. We can only send overview searches
               // to overview maps.
               mc2log << warn << "[SH]: Not overview map in top of tree"
                      << endl;
               overviewMapIDs.erase(it++);
            }
         }

         // Set the expected number of replies to the overview search.
         m_nbrOverviewSearchesLeft = overviewMapIDs.size();
         
         SETSTATE(SEND_OVERVIEW);
         // copied from SEND_OVERVIEW, these should be fixed/merged!
         res = dequeFromTree();
         
         } break;
      case SEND_OVERVIEW : {
         mc2dbg2 << "[SH]: getNextPacket - SEND_OVERVIEW" << endl;
         // Use the m_packetsReadyToSend queue.
         res = dequeFromTree();

         if ( res == NULL ) {
            // Nothing more to send.
            SETSTATE(AWAIT_OVERVIEW);
         } else {
            SETSTATE(SEND_OVERVIEW);
         }
      }
      break;
      case AWAIT_OVERVIEW :      
         mc2dbg4 << "SearchHandler::getNextPacket AWAIT_OVERVIEW  " 
                 << "nothing to send!" << endl;
         break;
      case SEND_SEARCH :
         res = m_requestPackets[ m_currentRequestIndex ];
         m_currentRequestIndex++;
         if ( m_currentRequestIndex >= m_nbrRequestPackets ) {
            SETSTATE(AWAIT_SEARCH);
         }
         break;
      case AWAIT_SEARCH:
         mc2dbg2 << "SearchHandler::getNextPacket AWAIT_SEARCH  " 
                 << "nothing to send!" << endl;
         break;
      case SEND_EXPAND_ITEM :
         res = m_expandItemCont;
         m_expandItemCont = NULL;
         SETSTATE(AWAIT_EXPAND_ITEM);
         break;
      case AWAIT_EXPAND_ITEM :
         mc2dbg2 << "SearchHandler::getNextPacket AWAIT_EXPAND_ITEM  " 
                << "nothing to send!" << endl;
         break;
      case COORDINATES:
         mc2dbg2 << "[SH]: getNextPacket - COORDINATE" << endl;
         // Use the m_packetsReadyToSend queue.
         if ( m_coordinateHandler == NULL ) {
            SETSTATE(initCoordinates());
         }
         // Check if ok
         if ( m_state == COORDINATES ) {
            // OK - Run the coordinate object.
            res = m_coordinateHandler->getNextPacket();
         }
         break;
      case OVERVIEWEXPAND:
      case OVERVIEWEXPAND_THEN_OVERVIEW_CHECK:
         if ( m_nbrOverviewMatchesToExpand < 0 ) {
            initOverviewExpand();
         }
         if ( m_state == OVERVIEWEXPAND ||
              m_state == OVERVIEWEXPAND_THEN_OVERVIEW_CHECK ) {
            // OK - Get next packet from m_packetsReadyToSend
            res = dequeFromTree();
         } else if ( m_state == SEND_SEARCH ) {
            // Have to dequeue a packet
            // Call this function again.
            res = getNextPacket();            
         }
         break;
      case UNEXPAND_OVERVIEW:
         if ( ! m_matchVector.empty() ) {
            handleTopRegionReceivedInUnexpand();
            mc2dbg2 << "[SH]: Putting topregionrequest into queueueu" 
                    << endl;
            // Signal that we do not have to create new packets.
            SETSTATE(AWAIT_UNEXPAND_OVERVIEW); // FIXME these states not used as before now!
         } else {
            SETSTATE(checkMatchInfosReceived());
         }
         // DO NOT BREAK
      case AWAIT_UNEXPAND_OVERVIEW:  // FIXME these states not used before now!
         res = dequeFromTree();
         break;
         
      case DONE :
         mc2dbg2 << "------------ SearchHandler DONE ----------"
                << endl
                << "SearchHandler::getNextPacket DONE  " 
                << "nothing to send!" << endl;
         break;
      case ALMOST_DONE_2:
      break;
   };
   
   return res;
}

PacketContainer*
SearchHandler::getAnswer() {
   PacketContainer* answer = m_answer;
   m_answer = NULL;
   return answer;
}


bool
SearchHandler::requestDone() {
   return (m_state == DONE);
}


uint32 
SearchHandler::getNbrLocationsFound() {
   return m_unexpandedOverviewMatches.size();
}


uint32
SearchHandler::getNbrOverviewMatches() {
   return m_unexpandedOverviewMatches.size();
}


OverviewMatch* 
SearchHandler::getOverviewMatch( uint32 i ) const
{
   return m_unexpandedOverviewMatches[i];
}

template<class SEARCHMATCH>
inline void
SearchHandler::calcDistsAndSort(vector<SEARCHMATCH*>& matches,
                                const MC2Coordinate& origin,
                                SearchTypes::SearchSorting sorting)
{
   int nbrMissingCoords = 0;
   // Calc distances
   for(uint32 i = 0; i < matches.size(); ++i ) {
      SEARCHMATCH* match = matches[i];
      if ( match->getCoords().isValid() ) {
         double distanceSq =
            GfxUtility::squareP2Pdistance_linear(origin.lat,
                                                 origin.lon,
                                                 match->getCoords().lat,
                                                 match->getCoords().lon);
         double distance = ::sqrt(distanceSq);
         uint32 intDist = uint32(distance);
         match->setDistance(intDist);
         mc2dbg4 << "[SH]: Setting distance for "
                 << IDPair_t(match->getMapID(), match->getItemID())
                 << " with points = " << match->getPoints() 
                 << " to " << intDist << endl;
      } else {
         if ( sorting == SearchTypes::DistanceSort ) {
            // Only warn for distance sort. It may not be needed
            // when using confidence.
            mc2dbg8 << "[SH]: Cannot distance sort invalid coords!"
                       " ID is "
                   << IDPair_t(match->getMapID(), match->getItemID())
                   << " points = " << match->getPoints() 
                   << endl;
            ++nbrMissingCoords;
            
         }
         match->setDistance(MAX_UINT32);
      }
   }
   // Sort by distance

   if ( nbrMissingCoords != 0 ) {
      mc2log << warn << "[SH]: " << nbrMissingCoords
             << " matches lack coordinates and could not be distance sorted"
             << endl;
   }
   
   // Use the STL stuff.
#if 0
   switch ( sorting ) {
      case SearchTypes::DistanceSort:
         
         stable_sort(matches.begin(), matches.end(),
                     DistanceComparator<SearchMatch>());
         break;
      case SearchTypes::ConfidenceSort:
         mc2dbg4 << "[SH]: Confidence sorting ... " << endl;
         sort( matches.begin(), matches.end(),
               ConfidenceComparator<SearchMatch>());
         break;
      default:
         break;
   }
#else
   if ( sorting == SearchTypes::DistanceSort &&
        !origin.isValid() )
   {
      
      mc2dbg << "[SH]: Invalid origin coord " << origin
             << " sorting with confidence instead of dist" << endl;
      sorting = SearchTypes::ConfidenceSort;    
   }
   SearchSorting::sortSearchMatches( matches, sorting );
#endif
}

void
SearchHandler::makeMatchVector()
{
   if ( m_matches != NULL ) {
      m_matchVector.clear();
      for (uint32 i = 0; i < m_nbrReceivedPackets; i++) {
         MatchLink* link = m_matches[ i ];
         MatchLink* next = NULL;
         while ( link != NULL ) {
            next = link->getNext();
            m_matchVector.push_back(link->getMatch());
            link = next;
         }
      }
   }
}

const vector<VanillaMatch*>&
SearchHandler::getMatches() const
{
   return m_matchVector;
}


OverviewMatch** 
SearchHandler::getOverviewMatches()
{
   OverviewMatch** matches =
      new OverviewMatch*[ m_unexpandedOverviewMatches.size() ];

   for ( uint32 i = 0 ; i < m_unexpandedOverviewMatches.size() ; i++ ) {
      matches[ i ] = new OverviewMatch( *static_cast<OverviewMatch*>( 
         m_unexpandedOverviewMatches[i] ) );
   }
   return matches;
}


uint32
SearchHandler::getUniqueOrFull()
{
   if ( m_unexpandedOverviewMatches.size() == 1 ) {
      mc2dbg2 << "[SH]: Get unique or full will return true" << endl;
      return 1;
   } else {
      mc2dbg2 << "[SH]: Get unique or full will return false " << endl;
      return 0;
   }
}


bool
SearchHandler::makeSearchMaskPackets( 
   const uint32* mapIDs, uint32 nbrMapIDs, 
   const uint32* masks, const uint32* maskItemIDs, 
   const char** maskNames, uint32 nbrMasks ) 
{ 
#ifdef DEBUG_LEVEL_8
   mc2dbg << "SearchHandler::makeSearchMaskPackets " << endl
          << "nbrMapIDs: " << nbrMapIDs << endl
          << hex;
   for ( uint32 i = 0 ; i < nbrMapIDs ; i++ ) {
      mc2dbg << mapIDs[ i ] << endl;
   }
   mc2dbg << dec;
   mc2dbg << "nbrMasks: " << nbrMasks << endl;
   mc2dbg << hex;
   for ( uint32 i = 0 ; i < nbrMasks ; i++ ) {
      mc2dbg << maskItemIDs[ i ] << ':' << maskNames[i] << endl; 
   } 
   mc2dbg << dec;
#endif
   
   // Delete the things from prevous decodeRequest
   delete [] m_locations;
   delete [] m_categories;

   UserSearchRequestPacket* userP = 
      dynamic_cast< UserSearchRequestPacket* > ( m_searchPacket );
   uint32* tmpMasks = NULL;
   uint32* tmpMaskIDs = NULL;
   uint32  tmpNbrMasks = 0;
   char** tmpMaskNames = NULL;
   uint32 tmpUint32 = 0;
   if ( userP != NULL ) {
      m_params.m_isUserSearch = true;
      vector<MC2BoundingBox> dummyVector;
      uint16 reqLanguage;
      userP->decodeRequest( 
         m_zipCode, m_nbrLocations, m_locations, m_params.m_locationType,
         m_nbrCategories, m_categories,
         tmpNbrMasks, tmpMasks, tmpMaskIDs, tmpMaskNames, m_params.m_dbMask,
         m_searchString,
         m_params.m_nbrHits, m_params.m_matchType,
         m_params.m_stringPart, m_params.m_sortingType, 
         m_params.m_searchForTypes,
         reqLanguage, m_params.m_nbrSortedHits,
         m_params.m_editDistanceCutoff, tmpUint32,
         m_params.m_categories,
         m_topRegionID,
         dummyVector);
      m_params.setRequestedLang(LangTypes::language_t(reqLanguage));
   } 
   delete [] tmpMasks;
   delete [] tmpMaskNames;
   delete [] tmpMaskIDs;

   uint32* sortMapIDs = new uint32[ nbrMapIDs ];

   memcpy( sortMapIDs, mapIDs, nbrMapIDs*sizeof(uint32) );

   std::sort( sortMapIDs, sortMapIDs + nbrMapIDs );

   uint32 uniqueueMapIDs = 0;
   uint32 lastID = MAX_UINT32;

   for ( uint32 i = 0; i < nbrMapIDs ; i++ ) {
      if ( sortMapIDs[ i ] != lastID ) {
         uniqueueMapIDs++;
         lastID = sortMapIDs[ i ];
      }
   }

   // One packet per uniqueue mapID and all masks for the map
   m_requestPackets = new PacketContainer*[ uniqueueMapIDs ];
   m_requestPacketIDs = new uint32[ uniqueueMapIDs ];
   m_replyPackets = new PacketContainer*[ uniqueueMapIDs ];

   uint32 mapID = MAX_UINT32;
   int32 lastMapIDIndex = -1;
   for ( uint32 i = 0 ; i < uniqueueMapIDs ; i++ ) {
      for ( int32 k = lastMapIDIndex + 1 ; k < (int32)nbrMapIDs ; k++ ) {
         if ( sortMapIDs[ k ] != mapID ) {
            mapID = sortMapIDs[ k ];
            lastMapIDIndex = k;
            break;
         }
      }
      
      uint32 nbrMasks = 0;
      for ( uint32 j = 0 ; j < nbrMapIDs ; j++ ) {
         if ( mapIDs[ j ] == mapID ) { // Add mask
            nbrMasks++;  
         }
      }
      uint32 msks[ nbrMasks ];
      uint32 msksItemIDs[ nbrMasks ];
      const char* mskNames [ nbrMasks ];
      nbrMasks = 0;
      // The vector thing could be done much less complicated by using
      // the overview matches, but I don't understand everything here
      // right now.
      vector<IDPair_t> allowedRegions;
      for ( uint32 j = 0 ; j < nbrMapIDs ; j++ ) {
         if ( mapIDs[ j ] == mapID ) { // Add mask
            allowedRegions.push_back(IDPair_t(mapID, maskItemIDs[ j ]));
            DEBUG8(mc2dbg8 << "Make masks MapID: " << mapID << " mask: " 
                   << hex << masks[ j ] << dec << endl;);
            msks[ nbrMasks ] = masks[ j ];
            msksItemIDs[ nbrMasks ] = maskItemIDs[ j ];
            mskNames[ nbrMasks ] = maskNames[ j ];
            nbrMasks++;  
         }
      }
      m_requestPacketIDs[ i ] = m_request->getNextPacketID();
      DEBUG8(mc2dbg8 << "SearchHandler packetID: " 
             << m_requestPacketIDs[ i ] << endl;);
      
      m_params.setAddStreetNamesToCompanies(
         strcmp(m_zipCode, "AddStreetNamesToCompanies") == 0);
     
      UserRightsMapInfo rights( mapID,
                                m_request->getUser(),
                                ~MapRights() );
     
      SearchRequestPacket* reqPack =
         new SearchRequestPacket(mapID,
                                 m_requestPacketIDs[ i ],
                                 m_request->getID(),
                                 SearchRequestPacket::USER_SEARCH,
                                 m_searchString,
                                 m_params,
                                 allowedRegions,
                                 m_bboxesToSearchIn,
                                 m_sortOrigin,
                                 rights );
                                       
      m_requestPackets[ i ] = new PacketContainer( reqPack, 0, 0, 
                                                   MODULE_TYPE_SEARCH );
      
   }

   delete [] sortMapIDs;
   
   m_currentRequestIndex = 0;
   m_nbrRequestPackets = uniqueueMapIDs;
   m_nbrReceivedPackets = 0;


   if ( uniqueueMapIDs > 0 ) 
      return true;
   else {
      return false;
   }
}


bool
SearchHandler::
makeSearchMaskPackets(const vector<OverviewMatch*>& overviewMatches)
{
   // Vectors to be used for masks and similar.
   int nbrOver = overviewMatches.size();

   Vector* mapIDs          = new Vector ( nbrOver, 10 );
   Vector* masks           = new Vector ( nbrOver, 10 );
   Vector* maskItemIDs     = new Vector ( nbrOver, 10 );
   StringVector* maskNames = new StringVector ( nbrOver, 10 );

   // FIXME: Move the creation of SearchRequestPackets here.
   for ( uint32 i = 0; i < overviewMatches.size(); ++i ) {
      OverviewMatch* match =
         static_cast<OverviewMatch*>(overviewMatches[i]);
      
      mapIDs->addLast( match->getMapID() );
      masks->addLast( MAX_UINT32 );
      maskItemIDs->addLast( match->getItemID() );
      maskNames->addLast( (char*)match->getName0() );
   }
   // Call the old function
   bool res = makeSearchMaskPackets( mapIDs->getBuffer(), 
                                     overviewMatches.size(), 
                                     masks->getBuffer(),
                                     maskItemIDs->getBuffer(),
                                     (const char**)maskNames->getBuffer(),
                                     overviewMatches.size() );
   delete mapIDs;
   delete masks;
   delete maskItemIDs;
   delete maskNames;
   return res;
}

bool
SearchHandler::makeOverviewMatchesFromMasks(uint32 nbrMapIDs,
                                            const uint32* mapIDs,
                                            uint32 nbrMasks,
                                            const uint32* maskItemIDs,
                                            const const_char_p* maskNames)
{
   MC2_ASSERT( nbrMapIDs == nbrMasks || nbrMapIDs == 1 );
   
   for( uint32 i = 0; i < nbrMasks; ++i ) {
      uint32 curMapID = mapIDs[MIN(i, (nbrMapIDs-1))];
      OverviewMatch* ov = new OverviewMatch(curMapID,
                                            ItemTypes::itemType(0), 0, 0);
      ov->setItemID(maskItemIDs[i]);
      if ( MapBits::isOverviewMap(curMapID) ) {
         ov->setOverviewID(IDPair_t(ov->getMapID(), ov->getItemID()));
      }
      ov->setName0(maskNames[i]);
      m_unexpandedOverviewMatches.push_back(ov);
   }

   return true;
}

void debug(MatchLink *lolink,
           MatchLink *hilink,
           MatchLink *reslink)
{
   mc2dbg << "lolink != 0 " << (lolink!=0)
        << "hilink != 0 " << (hilink!=0)
        << "reslink!= 0 " << (reslink!=0) << endl;
   VanillaMatch *vm = NULL;
   if (lolink != NULL) {
      mc2dbg << "hej1";
      vm = lolink->getMatch();
      mc2dbg << "hej2";
      mc2dbg << vm->getID() << endl;
   }
   if (hilink != NULL) {
      mc2dbg << "hej3";
      vm = hilink->getMatch();
      mc2dbg << "hej4";
      mc2dbg << vm->getID() << endl;
   }
   if (reslink!= NULL) {
      mc2dbg << "hej5";
      vm = reslink->getMatch();
      mc2dbg << "hej6";
      mc2dbg << vm->getID() << endl;
   }
   mc2dbg << "hej7" << endl;
}


void 
SearchHandler::merge( uint32 loNdx,  
                      uint32 hiNdx,
                      uint32 resNdx, 
                      MatchLink **matches,
                      uint32 nbrLists,
                      SearchTypes::SearchSorting sorting )
{
   // FIXME: Merge cannot be this way when the distances are used
   //        in confidence sorting. The distance will probably need
   //        normalizing. Implement in SearchSorting.
   
   if (loNdx != hiNdx) {
      uint32 mid = (loNdx + hiNdx) / 2;
      DEBUG4( mc2dbg4 << "merge direkt\n"
              << "merge(...)\n"
              << loNdx << " londx\n"
              << hiNdx << " hindx\n"
              << resNdx << " resndx\n"
              << mid << " mid\n"
              << endl );
   
      // recursive part
      // Merge lower half
      if ( loNdx < mid ) {
         merge( loNdx, mid,   loNdx, matches, nbrLists, sorting );
      }
      // Merge higher half
      if ( mid+1 < hiNdx ) {
         merge( mid+1, hiNdx, hiNdx, matches, nbrLists, sorting );
      }
      // merging part
      MatchLink *resListStart = NULL;
      MatchLink *resLink = NULL;
      MatchLink *loLink = matches[loNdx];
      MatchLink *hiLink = matches[hiNdx];

      DEBUG8( mc2dbg8 << "before if" << endl;
              debug( loLink, hiLink, resLink ); );
      // FIXME: Should probably use SearchSorting instead.
      if ( (loLink != NULL) && 
           ( hiLink == NULL || loLink->isLessThan( hiLink, sorting) ) )
      {
         resLink = loLink;
         loLink = loLink->getNext();
      } else if (hiLink != NULL) {
         resLink = hiLink;
         hiLink = hiLink->getNext();
      } else {
         resListStart = NULL;
      }
      DEBUG8( mc2dbg8 << "after if" << endl;
              debug( loLink, hiLink, resLink ); );
      resListStart = resLink;
      // iterations (time consuming part)
      DEBUG4( mc2dbg4 << "merge före while loopen\n"
              << "merge(...)\n"
              << loNdx << " londx\n"
              << hiNdx << " hindx\n"
              << resNdx << " resndx\n" << endl );
      while ((loLink != NULL) && (hiLink != NULL)) {
         if ( loLink->isLessThan(hiLink, sorting) ) {
            DEBUG4( mc2dbg4 << "lolink<hilink" << endl );
            DEBUG4( mc2dbg4 << "in while1" << endl;
                    debug( loLink, hiLink, resLink ); );
            resLink->setNext( loLink );
            loLink = loLink->getNext();
            DEBUG4( mc2dbg4 << "in while2" << endl;
                    debug( loLink, hiLink, resLink ); );
         } else {
            DEBUG4( mc2dbg4 << "lolink>hilink" << endl );
            resLink->setNext( hiLink );
            hiLink = hiLink->getNext();
         }
         resLink = resLink->getNext();
         DEBUG4( mc2dbg4 << "in while3" << endl;
                 debug( loLink, hiLink, resLink ); );
      }
      // Add remaining matches of the remaining list to resList
      if (loLink != NULL) {
         mc2dbg4 << "lolink != null" << endl;
         resLink->setNext( loLink );
      } else if (hiLink != NULL) {
         mc2dbg4 << "hilink != null" << endl;
         resLink->setNext( hiLink );
      }
      // put the result at the right position.
      matches[loNdx] = NULL;
      matches[hiNdx] = NULL;
      matches[resNdx] = resListStart; // overwrites one of the above NULLs.
   } else {
         mc2dbg << "Error: attempting to merge list number "
         << loNdx << " with itself (number " << hiNdx << ")." << endl;
   } // else
}


void 
SearchHandler::convMatchLinkToPacket( MatchLink *matchLink,
                                      VanillaSearchReplyPacket *p )
{
   
   uint32 smallestRestriction = MAX_UINT32;

   MatchLink* startLink = matchLink;

   // Find out which match that has the smallest restriction
   // FIXME: Are the restrictions worse for higher numbers?
   if ( true ) {
      while (matchLink != NULL) {
         VanillaMatch *vm = matchLink->getMatch();
         if ( vm->getRestrictions() < smallestRestriction ) {
            const char* str = vm->getName();
            mc2dbg2 << "[SH]: Changing smallest restriction to "
                    << uint32(vm->getRestrictions()) << " for " 
                    << str << endl;
            smallestRestriction = vm->getRestrictions();
         }
         matchLink = matchLink->getNext();
      }
      
      mc2dbg2 << "[SH]: smallestRestriction = "
             << uint32(smallestRestriction) << endl;
   }
   
   // Restart
   matchLink = startLink;
   while (matchLink != NULL) {
      VanillaMatch *vm = matchLink->getMatch();
      
      if ( vm == NULL ) {
         break;
      } else {
         if ( vm->getRestrictions() <= smallestRestriction ) {
            vm->addToPacket(p);
         }
      }
      matchLink = matchLink->getNext();
   }
}

/*
#ifdef undef_level_1
#   undef DEBUG_LEVEL_1
#endif
#ifdef undef_level_2
#   undef DEBUG_LEVEL_2
#endif
#ifdef undef_level_4
#   undef DEBUG_LEVEL_4
#endif
#ifdef undef_level_8
#   undef DEBUG_LEVEL_8
#endif
*/
