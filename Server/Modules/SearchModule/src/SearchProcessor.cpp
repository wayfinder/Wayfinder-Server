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

#include "Packet.h"
#include "Processor.h"

#include "ServerProximityPackets.h"

#include "SystemPackets.h"

#include "SearchProcessor.h"

#include "LoadMapPacket.h"
#include "DeleteMapPacket.h"
#include "StringTable.h"

#include "SearchParameters.h"

#include "SearchPacket.h"
#include "SearchExpandItemPacket.h"

#include "SearchableSearchUnit.h"
#include "SearchMatch.h"
#include "SearchResult.h"
#include "DeleteHelpers.h"

#include <map>
#include <vector>
#include <signal.h>
// time to wait in seconds before exiting.
// 10*60 s = 10 minutes

const uint32 defaultAlarmLimit = 1*60; // one minute
const uint32 loadMapAlarmLimit = 10*60; // ten minutes

uint32 SearchProcessor::alarmLimit = defaultAlarmLimit;

SearchProcessor::SearchProcessor(MapSafeVector* loadedMaps)
      : MapHandlingProcessor(loadedMaps)
{
   m_nbrPackets = 0;
   m_nbrFailedPackets = 0;
}

SearchProcessor::~SearchProcessor()
{
   STLUtility::deleteAllSecond( m_searchUnits );
   m_searchUnits.clear();
}

StringTable::stringCode
SearchProcessor::loadMap( uint32 mapID, uint32& mapSize )
{
   setAlarm(loadMapAlarmLimit);
   mapSize = 1; // Change this to something better.

   // Check if it is already loaded...
   if ( m_searchUnits.find(mapID) != m_searchUnits.end() ) {
      mc2dbg << "[SProc]: Map " << mapID << " is already loaded"
             << endl;
      return StringTable::ERROR_MAP_LOADED;
   }

   // Check for MAX_UINT32.
   if ( mapID == MAX_UINT32 ) {
      mc2log << warn << "[SProc]: Loading of map MAX_UINT32 is not legal. "
             << "Request ignored." << endl;
      return StringTable::MAPNOTFOUND;
   }

   // Load
   SearchableSearchUnit* newUnit = new SearchableSearchUnit;
   if ( newUnit->loadFromMapModule(mapID) ) {
      m_searchUnits.insert(make_pair(mapID, newUnit));
      // The size in the databuffer should pretty much correspond
      // to the real size in memory since the stuff is stored rather
      // compactly.
      mapSize = newUnit->getSizeInDataBuffer();

      return StringTable::OK;
   } else {
      mc2log << error << "[SProc]: Load map not ok, deleting failed attempt "
             << "for map " << mapID << "."
             << endl;
      delete newUnit;
      return StringTable::ERROR_LOADING_MAP;
   }
}


StringTable::stringCode
SearchProcessor::deleteMap( uint32 mapID )
{
   mc2dbg2 << "unloadMap " << mapID << endl;
   StringTable::stringCode result;

   if ( m_searchUnits.find(mapID) == m_searchUnits.end() ) {
      mc2dbg << "Non fatal error: \n"
         << "unloadMapRequest received with mapID that "
         << "was not loaded.\n"
         << "Request Ignored." << endl;
      result = StringTable::MAPNOTFOUND;
   } else {
      delete m_searchUnits[mapID];
      m_searchUnits.erase(mapID);
      result = StringTable::OK;
   }
   return result;
}


const SearchableSearchUnit*
SearchProcessor::getSearchUnitByMapID(uint32 mapID) const
{
   map<uint32, SearchableSearchUnit*>::const_iterator it =
      m_searchUnits.find(mapID);
   
   if ( it == m_searchUnits.end() ) {
      return NULL;
   } else {
      return it->second;
   }
}

void
SearchProcessor::handleAlarm(int iMsg)
{
   ::mc2log << warn << "SearchProcessor: Received an alarm! iMsg is " 
          << iMsg << endl;
   ::mc2log << error << "Query was not finished within "
            << SearchProcessor::alarmLimit 
            << " seconds." << endl;
   abort();
}

void
SearchProcessor::setAlarm(uint32 seconds)
{
#  ifndef profiling
   struct itimerval it;
   it.it_interval.tv_sec = 0;  // no second alarm
   it.it_interval.tv_usec = 0;
   it.it_value.tv_sec = seconds;
   it.it_value.tv_usec = 0;
   alarmLimit = seconds;
   signal(SIGVTALRM, SearchProcessor::handleAlarm);
   if (setitimer(ITIMER_VIRTUAL, &it, NULL) != 0) {
      MC2WARNING2("Could not set alarm timer", PERROR;);
   }
#  endif
}

static inline void
writePacketInfo(int nbrHits,
                char* packetInfo,
                int nbrMasks,
                char** maskNames,
                const uint32* masks,
                const char* searchString,
                uint32 categoryType,
                int maxPackInfo,
                LangTypes::language_t lang,
                SearchTypes::StringMatching matching =
                                 SearchTypes::MaxDefinedMatching,
                SearchTypes::StringPart part =
                                 SearchTypes::MaxDefinedStringPart,
                SearchTypes::SearchSorting sorting =
                                 SearchTypes::MaxDefinedSort)
{
   int maxPackInfoLeft = maxPackInfo - 1;
   if ( packetInfo != NULL ) {       
      int res = 0;
      for(int i=0; i < (int)nbrMasks && res >= 0 && maxPackInfoLeft > 0; ++i ){
         if ( maskNames && maskNames[i] != NULL && maskNames[i][0] != '\0' ) {
            res = snprintf(packetInfo, maxPackInfoLeft,
                           "%x:\"%s\", ", masks[i], maskNames[i]);
            packetInfo += res;
            maxPackInfoLeft -= res;
         } else {
            res = snprintf(packetInfo, maxPackInfoLeft,
                           "0x%x, ", masks[i]);
            packetInfo += res;
            maxPackInfoLeft -= res;
         }
         if ( i > 20 ) {
            res = snprintf(packetInfo, maxPackInfoLeft,
                           "...  ");
            packetInfo += res;
            maxPackInfoLeft -= res;
            break;
         }
      }

      
      if ( res >=0 && maxPackInfoLeft > 0 ) {
         packetInfo -= 2; // Overwrite the ,
         *packetInfo++ = ':';
         *packetInfo = '\0';
         maxPackInfoLeft--;
      }
      
      if ( res >= 0 && maxPackInfoLeft > 0 && searchString != NULL ) {
         res =
            snprintf(packetInfo, maxPackInfoLeft, "\"%s\"", searchString);
         maxPackInfoLeft -= res;
         packetInfo += res;
      }

      if ( res >= 0 && maxPackInfoLeft >= 12 ) {
         res = snprintf(packetInfo, maxPackInfoLeft, ",%#010x", categoryType);
         maxPackInfoLeft -= res;
         packetInfo += res;
      }
      
      if ( res >= 0 && maxPackInfoLeft >= 20 &&
           matching != SearchTypes::MaxDefinedMatching) {
         const char* matchingString = SearchTypes::getMatchingString(
            matching );
         const char* partString     = SearchTypes::getPartString( part );
         const char* sortingString  = SearchTypes::getSortingString(
            sorting );
         const char* langString =
         LangTypes::getLanguageAsString(lang, false);
         res = snprintf(packetInfo, maxPackInfoLeft, ",%s,%s,%s [%s]",
                        partString, matchingString, sortingString,
                        langString);
         maxPackInfoLeft -= res;
         packetInfo += res;
      }

      if ( maxPackInfoLeft >= 10 ) {
         char hits[30];
         sprintf(hits, ", n=%d", nbrHits);
         strcat(packetInfo, hits);
         packetInfo += strlen(hits);
      }      
   }
}

static inline void writePacketInfo(char* packetInfo,
                                   int maxPacketInfo,
                                   const UserSearchParameters& params,
                                   int nbrHits)
{
   writePacketInfo(nbrHits, packetInfo, params.getNbrMasks(),
                   NULL, params.getMaskItemIDs(),
                   params.getSearchString(),
                   params.getRequestedTypes(), maxPacketInfo,
                   params.getRequestedLanguage(),
                   params.getMatching(),
                   params.getStringPart(),
                   params.getSorting());
                   
}


SearchExpandItemReplyPacket*
SearchProcessor::
handleSearchExpandItem(const SearchExpandItemRequestPacket* req)
{
   vector<pair<uint32, IDPair_t> > reqIDs;
   STLUtility::AutoContainerMap< vector<pair<uint32, OverviewMatch*> > >
      expandedIDs;

   req->getItems(reqIDs);

   const SearchableSearchUnit* ssunit =
      getSearchUnitByMapID(req->getMapID());
   
   ssunit->expandIDs( expandedIDs,
                      reqIDs,
                      req->getExpand() );
   
   return new SearchExpandItemReplyPacket( req, expandedIDs );

}

ReplyPacket*
SearchProcessor::handleSearchRequestPacket(const SearchRequestPacket* req,
                                           char* packetInfo)
{
   // Do not return early from this function since we want to
   // write JT-info.
   const uint32 mapID = req->getMapID();
   const SearchableSearchUnit* unit = getSearchUnitByMapID(mapID);

   // Create the params. They know what to do.
   UserSearchParameters params(req);
      
   ReplyPacket* result = NULL;

   int nbrHits = -1;
   
   // What to do?
   switch ( req->getSearchType() ) {
      case SearchRequestPacket::USER_SEARCH:
      case SearchRequestPacket::PROXIMITY_SEARCH: {
         // These searches use VanillaReply.
         VanillaSearchReplyPacket* vanillaReply =
            new VanillaSearchReplyPacket(req);
         vanillaReply->setStatus(StringTable::OK);
         result = vanillaReply;
         
         SearchUnitSearchResult searchResult;
         switch ( req->getSearchType() ) {
            case SearchRequestPacket::USER_SEARCH:
               // Normal searching
               unit->search(searchResult, params);
               break;
            case SearchRequestPacket::PROXIMITY_SEARCH:
               // Convert the id:s to matches.
               unit->proximitySearch(searchResult, params,
                                     params.getProximityItems());
               break;
            case SearchRequestPacket::OVERVIEW_SEARCH:
               // NOP, just to make the compiler stop complaning
               // should never be here.
            break;
         } // End inner switch.
         // And then add the results to the packet.
         
         for(SearchUnitSearchResult::iterator it = searchResult.begin();
             it != searchResult.end();
             ++it ) {
            (*it)->addToPacket(vanillaReply);
         }
         // For log
         nbrHits = searchResult.size();

      }
      break;
      case SearchRequestPacket::OVERVIEW_SEARCH: {
         // Create the result vector and search.
         vector<OverviewMatch*> overviewResult;
         unit->overviewSearch( overviewResult, params );

         // For log
         nbrHits = overviewResult.size();
         
         result =
            new OverviewSearchReplyPacket(req, overviewResult);
         result->setStatus(StringTable::OK);
         // Clean up
         STLUtility::deleteValues( overviewResult );
      }
      break;  
      default: {
         // Not implemented
         result =
            new VanillaSearchReplyPacket(req);
         result->setStatus(StringTable::NOT);
      }
      break;
   }
   
   writePacketInfo(packetInfo, 
                   c_maxPackInfo,
                   params,
                   nbrHits);
   
   return result;
}


Packet*
SearchProcessor::handleRequestPacket( const RequestPacket& p,
                                      char* packetInfo )
{
   setAlarm(defaultAlarmLimit);
   DEBUG2(uint32 startTime = TimeUtility::getCurrentMicroTime(););

   DEBUG2({
      mc2log << info << "SearchProcessor received packet" << endl
             << "IP " << p.getOriginIP() << endl
             << "Port " << p.getOriginPort() << endl
             << "PacketID " << p.getPacketID() << endl
             << "RequestID " << p.getRequestID() << endl
             << "Subtype as string: " << p.getSubTypeAsString() << endl;
   });

   Packet* result = NULL;
   StringTable::stringCode resultStatus = StringTable::NOTOK; // by default
   
 
   m_nbrPackets++;
   
   int subType = p.getSubType();
   DEBUG2( mc2dbg2 << "Packet subType : " << subType << "\n" );
   switch (subType) {
      case Packet::PACKETTYPE_SEARCHEXPANDITEMREQUEST: {
         const SearchExpandItemRequestPacket* seirp =
            static_cast<const SearchExpandItemRequestPacket*>( &p );
         result = handleSearchExpandItem(seirp);
         resultStatus =
            StringTable::stringCode((
               static_cast<ReplyPacket*>(result))->getStatus());
         break;
      }
      case Packet::PACKETTYPE_SEARCHREQUEST: {
         const SearchRequestPacket* srpack =
            static_cast<const SearchRequestPacket*>( &p );
         result = handleSearchRequestPacket(srpack, packetInfo);
         resultStatus =
            StringTable::stringCode((
               static_cast<ReplyPacket*>(result))->getStatus());
         break;
      }
      default: {
         mc2log << error << "[SProc]: Unhandled packet of type "
                << subType << endl;
         resultStatus = StringTable::UNKNOWN;
      }
   } // switch
   
   if((dynamic_cast<ReplyPacket*>(result) != NULL) &&
      (static_cast<ReplyPacket*>(result)->getStatus() ==
       StringTable::MAPNOTFOUND))
   {
      delete result;
      result = new AcknowledgeRequestReplyPacket( &p,
                                                  StringTable::OK,
                                                  ACKTIME_NOMAP );
   }
   
   if (result == NULL) {
      mc2dbg2 << "Unknown subtype (or unhandled request) "
           << (int) p.getSubType() << endl;
      mc2dbg4 << "IP " << p.getOriginIP() << endl
           << "Port " << p.getOriginPort() << endl
           << "PacketID " << p.getPacketID() << endl
           << "RequestID " << p.getRequestID() << endl;
   } else {
      mc2dbg2 << "Returning packet to IP=" << result->getOriginIP()
              << ", port=" << result->getOriginPort() << endl ;
   }

   setAlarm(0);
   
   return result;
}

int
SearchProcessor::getCurrentStatus()
{
   return 0;
}
