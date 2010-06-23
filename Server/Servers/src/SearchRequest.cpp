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
#include <algorithm>
#include <sstream>
#include "SearchRequest.h"
#include "Properties.h"
#include "SearchHandler.h"
#include "SearchPacket.h"
#include "CoveredIDsRequest.h"
#include "CoordinateOnItemPacket.h"
#include "IDPairVector.h"
#include "SearchRequestParameters.h"
#include "TopRegionRequest.h"
#include "ScopedArray.h"

/**
 *   These are the variables used by the SearchRequest.
 */
class SearchReqIntData {
  
   friend class SearchRequest;
   /// The settings for the search
   SearchRequestParameters m_params;

   /// The settings for overview search
   SearchRequestParameters m_overParams;
   
   /// The search request packet with the request
   OldSearchRequestPacket *m_packet;
   /// The search request packet with the request to delete.
   OldSearchRequestPacket *m_packetToDelete;

   
   /// The search handler
   SearchHandler* m_searchHandler;
   /// The CoveredIDs request
   CoveredIDsRequest* m_coveredIDsRequest;
   /// The status of the request. Returned if m_state == ERROR
   StringTable::stringCode m_status;
   /// The search string.
   MC2String m_itemQuery;
   /// The position if any
   MC2Coordinate m_position;
   /// The Boundingbox if any
   MC2BoundingBox m_bbox;
   

   // FIXME: Move this to the CoveredIDsRequest.
   /// The item id of the origin
   IDPair_t m_originID;
   /// The initial radius.
   int m_radiusMeters;

   /// The state of the request
   enum state_t {
      /**
       *   The request is getting a coordinate for the
       *   origin for later use in CoveredIDs.
       */
      GETTING_COORD_FOR_ORIG = 50,
      /** The request is getting covered ids from a CoveredIDsRequest */
      USING_COVERED_ID_REQ   = 100,
      /** The request is using the SearchHandler */
      SEARCHING              = 200,
      /** The request is done and OK */
      DONE_OK                = 300,
      /** The request is done and not OK */
      ERROR                  = 400
   } m_state;

};

class OverviewFoundComp {
public:
   /** Constructor */
   OverviewFoundComp(set<IDPair_t>& overviewSet) : m_ov(overviewSet) {};

   /**
    *   Looks up the region and regions of regions for match.
    *   @return True if the region of the match or the regions
    *           of the regions are among the overview matches.
    */
   bool findRegions(SearchMatch* match) const {
      if ( m_ov.find(match->getID()) != m_ov.end() ) {
         return true;
      } else {
         for ( uint32 i = 0; i < match->getNbrRegions(); ++i ) {
            if ( findRegions(match->getRegion(i)) )  {
               return true;
            }
         }
         return false;
      }
   }
   
   /**
    *   a is less than b if a is found among the overviews but not b.
    *   The regions of the regions are also checked.
    */
   bool operator()(SearchMatch* a,
                   SearchMatch* b) const {
      bool aFound = findRegions(a);
      if ( !aFound ) {
         return false;
      }
      bool bFound = findRegions(b);
      if ( ! bFound ) {
         // a found, but not b
         return true;
      } else {
         // Both found.
         return false;
      }
   }
private:
   set<IDPair_t>& m_ov;
};

// To be able to change this into printing the state
// always use SETSTATE instead of m_state=
#define SETSTATE(x) do { mc2dbg2 << "[SR]: Changing state from " \
                          << int(m_data->m_state) << " to " << #x \
                          << endl; \
                         m_data->m_state = x;} while(0)

// ******** SearchRequest *****************************

void
SearchRequest::init()
{
   mc2dbg2 << "[SR]: User = " << MC2HEX(getUser()) << endl;
   m_data = new SearchReqIntData;
   m_data->m_packet            = NULL;
   m_data->m_status            = StringTable::TIMEOUT_ERROR;
   m_data->m_packetToDelete    = NULL;
   m_data->m_coveredIDsRequest = NULL;
   m_data->m_searchHandler     = NULL;
   m_data->m_radiusMeters      = -1;
   m_data->m_state             = SearchReqIntData::ERROR;
   m_allowedMaps = NULL;
   m_topRegionRequest = NULL;
}

SearchRequest::SearchRequest( const RequestData& parentOrID,
                              OldSearchRequestPacket* p,
                              const TopRegionRequest* topReq,
                              bool uniqueOrFull,
                              bool searchOnlyIfUniqueOrFull,
                              uint32 regionsInMatches,
                              uint32 overviewRegionsInMatches,
                              bool lookupCoordinates,
                              bool lookupBBoxes,
                              const MC2Coordinate& sortOrigin) 
      : SearchResultRequest(parentOrID)
{
   init();
   // remember the packet
   m_data->m_packet = p;
   
   m_topRegionRequest = topReq;
   m_data->m_coveredIDsRequest = NULL;
   m_data->m_searchHandler = new SearchHandler( p, this, 
                                        // TODO: Add SearchRequestPara... param
                                        SearchRequestParameters(), 
                                        m_topRegionRequest,
                                        uniqueOrFull, 
                                        searchOnlyIfUniqueOrFull,
                                        regionsInMatches, 
                                        overviewRegionsInMatches,
                                        lookupCoordinates,
                                        lookupBBoxes,
                                        sortOrigin,
                                                NULL );
   
   if ( enqueuePacketsFromSearchHandler() != 0 ) {
      SETSTATE(SearchReqIntData::SEARCHING);
   } else {
      m_data->m_status =
         StringTable::stringCode(m_data->m_searchHandler->getStatus());
      SETSTATE(SearchReqIntData::ERROR);
   }
}

uint32
SearchRequest::getRadiusMeters() const
{
   return MIN(uint32(m_data->m_radiusMeters),
              Properties::getUint32Property(
                 "SEARCH_PROXIMITY_MAX_RADIUS_METERS", 100000) );
}

void
SearchRequest::initCoveredIDs(const MC2Coordinate& origin,
                              int radiusMeters)
{
   set<ItemTypes::itemType> types;
   types.insert(ItemTypes::municipalItem);
   types.insert(ItemTypes::builtUpAreaItem);

   if ( m_data->m_bbox.isValid() ) {
      // Use bbox
      m_data->m_coveredIDsRequest = new CoveredIDsRequest(
         this, m_data->m_bbox, types, m_topRegionRequest );
   } else { // Use radius
      int useRadius = getRadiusMeters();

      if ( useRadius < radiusMeters ) {
         mc2log << info << "[SR]: Decreasing search radius to "
                << useRadius << " from " << radiusMeters << endl;
      }
   
   
      m_data->m_coveredIDsRequest = new CoveredIDsRequest(this,
                                                          origin,
                                                          useRadius,
                                                          types,
                                                          m_topRegionRequest);
   }
   
   if ( enqueuePacketsFromRequest(m_data->m_coveredIDsRequest) != 0 ) {
      SETSTATE(SearchReqIntData::USING_COVERED_ID_REQ);
   } else {
      m_data->m_status = m_data->m_coveredIDsRequest->getStatus();
      SETSTATE(SearchReqIntData::ERROR);
   }
}

SearchRequest::SearchRequest(const RequestData& parentOrID,
                             const SearchRequestParameters& params,
                             const MC2Coordinate& position,
                             const MC2String& itemQuery,
                             const TopRegionRequest* topReq,
                             int radiusMeters,
                             const SearchRequestParameters* overParams)
      : SearchResultRequest(parentOrID)
{
   init();
   
   m_topRegionRequest = topReq;
   m_data->m_itemQuery = itemQuery;
   m_data->m_searchHandler = NULL;
   m_data->m_position = position;
   m_data->m_params = params;
   m_data->m_radiusMeters = radiusMeters;
   if ( overParams != NULL ) {
      m_data->m_overParams = *overParams;
   } else {
      m_data->m_overParams = params;
   }
   initCoveredIDs(position, radiusMeters);
}


SearchRequest::SearchRequest( const RequestData& parentOrID,
                              const SearchRequestParameters& params,
                              const MC2String& itemQuery,
                              const MC2BoundingBox& bbox,
                              const TopRegionRequest* topReq,
                              const SearchRequestParameters* overParams )
      : SearchResultRequest(parentOrID)
{
   init();
   
   m_topRegionRequest = topReq;
   m_data->m_itemQuery = itemQuery;
   m_data->m_searchHandler = NULL;
   m_data->m_params = params;
   m_data->m_bbox = bbox;
   MC2Coordinate position;
   bbox.getCenter( position.lat, position.lon );
   m_data->m_position = position;
   if ( overParams != NULL ) {
      m_data->m_overParams = *overParams;
   } else {
      m_data->m_overParams = params;
   }

   initCoveredIDs( position );
}


SearchRequest::SearchRequest(const RequestData& parentOrID,
                             const SearchRequestParameters& params,
                             const IDPair_t& origin,
                             const MC2String& itemQuery,
                             const TopRegionRequest* topReq,
                             int radiusMeters,
                             const SearchRequestParameters* overParams )
      : SearchResultRequest(parentOrID)
{
   init();
   if ( radiusMeters < 0 ) {
      // Should really ask the MapModule and not use
      // radius here.
      radiusMeters = 10000;
   }
   
   m_topRegionRequest = topReq;
   m_data->m_radiusMeters = radiusMeters;
   m_data->m_itemQuery = itemQuery;
   m_data->m_searchHandler = NULL;
   m_data->m_params = params;
   if ( overParams != NULL ) {
      m_data->m_overParams = *overParams;
   } else {
      m_data->m_overParams = params;
   }

   uint32 nextPacketID = getNextPacketID();
   
   CoordinateOnItemRequestPacket* coip =
      new CoordinateOnItemRequestPacket( getID(),
                                         nextPacketID,
                                         false); // No BBox
   
   coip->setMapID(origin.getMapID() );
   coip->add(origin.getItemID(), MAX_UINT32 );

   // Seems buggy, this packet. Set id:s again.
   coip->setRequestID( getID() );
   coip->setPacketID( nextPacketID );
   
   enqueuePacket(coip, MODULE_TYPE_MAP);
   SETSTATE( SearchReqIntData::GETTING_COORD_FOR_ORIG );
}

SearchRequest::SearchRequest( const RequestData& parentOrID,
                              const SearchRequestParameters& params,
                              const vector<uint32>& mapIDs,
                              const char* itemQuery,
                              const TopRegionRequest* topReq,
                              const MC2Coordinate& sortOrigin,
                              const SearchRequestParameters* overParams )
      : SearchResultRequest(parentOrID)
{
   init();
   m_data->m_params = params;
   if ( overParams != NULL ) {
      m_data->m_overParams = *overParams;
   } else {
      m_data->m_overParams = params;
   }

   m_topRegionRequest = topReq;

   UserSearchRequestPacket* p = new UserSearchRequestPacket( 0, getID() );

   uint32* mapIDArray = new uint32[mapIDs.size()];
   int nbrMapIDs = 0;
   for( vector<uint32>::const_iterator it = mapIDs.begin();
        it != mapIDs.end();
        ++it ) {
      mapIDArray[nbrMapIDs] = *it;
      ++nbrMapIDs;
   }
   
   p->encodeRequest( nbrMapIDs,
                     mapIDArray,
                     params.getAddStreetNamesToCompanies() ? 
                     "AddStreetNamesToCompanies" : "", // ZipCode
                     0, // NbrLocations
                     NULL, // Locations
                     params.getSearchForLocationTypes(), // Location type
                     0, NULL,   // nbrCategories, categories
                     0,  // NbrMasks
                     NULL, // Masks
                     NULL, // MaskItemIDs
                     NULL, // MaskNames
                     params.getDBMask(),
                     itemQuery,
                     params.getMinNbrHits(), // NbrHits
                     params.getMatchType(),
                     params.getStringPart(),
                     params.getSortingType(),
                     params.getSearchForTypes(),
                     params.getRequestedLang(),
                     params.getNbrSortedHits(), 
                     params.getEditDistanceCutOff(),
                     params.getRegionsInMatches(),
                     params.getCategories(),
                     StringTable::countryCode( 0 ) ); // Not important

   delete [] mapIDArray;
   
   m_data->m_packetToDelete = m_data->m_packet = p;
   m_data->m_coveredIDsRequest = NULL;
   m_data->m_params = params;
   if ( overParams != NULL ) {
      m_data->m_overParams = *overParams;
   } else {
      m_data->m_overParams = params;
   }

   m_data->m_searchHandler = new SearchHandler( 
      p, this, params, m_topRegionRequest, params.uniqueOrFull(), 
      params.searchOnlyIfUniqueOrFull(),
      params.getRegionsInMatches(), params.getRegionsInOverviewMatches(),
      params.getLookupCoordinates(), params.getBBoxRequested(), sortOrigin,
      &m_data->m_overParams );
   
   if ( enqueuePacketsFromSearchHandler() != 0 ) {
      SETSTATE( SearchReqIntData::SEARCHING );
   } else {
      m_data->m_status =
         StringTable::stringCode( m_data->m_searchHandler->getStatus() );
      SETSTATE( SearchReqIntData::ERROR );
   }

}

SearchRequest::SearchRequest( const RequestData& parentOrID,
                              const SearchRequestParameters& params,
                              const vector<IDPair_t>& selectedAreas,
                              const char* itemQuery,
                              const TopRegionRequest* topReq,
                              const MC2Coordinate& sortOrigin,
                              const SearchRequestParameters* overParams,
                              int32 radiusMeters )
      : SearchResultRequest( parentOrID )
{   
   init();
   m_topRegionRequest = topReq;
   m_data->m_radiusMeters = radiusMeters;
   initPacket( params, selectedAreas, "", 0, itemQuery, sortOrigin, 
               overParams );
}

SearchRequest::SearchRequest( const RequestData& parentOrID,
                              const SearchRequestParameters& params,
                              uint32 topRegionID,
                              const char* areaQuery,
                              const char* itemQuery,
                              const TopRegionRequest* topReq,
                              const MC2Coordinate& sortOrigin,
                              const SearchRequestParameters* overParams)
      : SearchResultRequest( parentOrID )
{
   init();
   vector<IDPair_t> emptyIDs;
   m_topRegionRequest = topReq;
   MC2_ASSERT(m_topRegionRequest != NULL);
   initPacket( params, emptyIDs, areaQuery, topRegionID, itemQuery,
               sortOrigin, overParams );
}

void
SearchRequest::initPacket( const SearchRequestParameters& params,
                           const vector<IDPair_t>& selectedAreas,
                           const char* areaQuery,
                           uint32 searchTopRegion,
                           const char* itemQuery,
                           const MC2Coordinate& sortOrigin,
                           const SearchRequestParameters* overParams )
{   
   m_data->m_params = params;
   if ( overParams != NULL ) {
      m_data->m_overParams = *overParams;
   } else {
      m_data->m_overParams = params;
   }
   typedef vector<IDPair_t>IDPairVector;
   IDPairVector::size_type nbrSelectedAreas = selectedAreas.size();
   // Dummy masks
   ScopedArray<uint32> masks( new uint32[ nbrSelectedAreas + 1 ] );
   for ( IDPairVector::size_type i = 0 ; i < nbrSelectedAreas ; ++i ) {
      masks[ i ] = MAX_UINT32;
   }
   // Dummy maskNames - changed into using new instead of
   // static looking dynamic allocation - won't work in
   // some compilers.
   
   ScopedArray<const char*> maskNames( new const char*[ nbrSelectedAreas + 1 ] );
   for ( IDPairVector::size_type i = 0 ; i < nbrSelectedAreas ; ++i ) {
      maskNames[ i ] = "";
   }

   ScopedArray<uint32> selectedAreaMapIDs( new uint32[ nbrSelectedAreas + 1] );
   ScopedArray<uint32> selectedAreaItemIDs( new uint32[ nbrSelectedAreas + 1] );

   for( IDPairVector::size_type i = 0; i < nbrSelectedAreas; ++i ) {
      selectedAreaMapIDs[i] = selectedAreas[i].getMapID();
      selectedAreaItemIDs[i] = selectedAreas[i].getItemID();
   }
   
   IDPairVector::size_type nbrMapIDs = nbrSelectedAreas;
#if 0
   // This case must maybe be supported some other way.
   if ( nbrSelectedAreas == 0 ) {
      selectedAreaMapIDs[0] = MAX_UINT32;
      nbrMapIDs = 1;
   }
#endif

   UserSearchRequestPacket* p = new UserSearchRequestPacket( 0, getID() );
   
   p->encodeRequest( nbrMapIDs, // numMapIDs
                     selectedAreaMapIDs.get(), // MapIDs
                     params.getAddStreetNamesToCompanies() ? 
                     "AddStreetNamesToCompanies" : "", // ZipCode
                     nbrSelectedAreas == 0 ? 1 : 0, // NbrLocations
                     nbrSelectedAreas == 0 ? &areaQuery : NULL,
                     params.getSearchForLocationTypes(), // Location type
                     0, NULL,   // nbrCategories, categories
                     nbrSelectedAreas,  // NbrMasks
                     masks.get(), // Masks
                     selectedAreaItemIDs.get(), // MaskItemIDs
                     maskNames.get(),
                     params.getDBMask(),
                     itemQuery,
                     params.getMinNbrHits(), // NbrHits
                     params.getMatchType(),
                     params.getStringPart(),
                     params.getSortingType(),
                     params.getSearchForTypes(),
                     params.getRequestedLang(),
                     params.getNbrSortedHits(), 
                     params.getEditDistanceCutOff(),
                     params.getRegionsInMatches(), 
                     params.getCategories(),
                     StringTable::countryCode( searchTopRegion ) );

   // remember the packet
   m_data->m_packetToDelete = m_data->m_packet = p;
   
   m_data->m_coveredIDsRequest = NULL;
   m_data->m_params = params;
   m_data->m_searchHandler = new SearchHandler( 
      p, this, params, m_topRegionRequest, params.uniqueOrFull(), 
      params.searchOnlyIfUniqueOrFull(),
      params.getRegionsInMatches(), params.getRegionsInOverviewMatches(),
      params.getLookupCoordinates(), params.getBBoxRequested(), sortOrigin,
      &m_data->m_overParams );
   
   if ( enqueuePacketsFromSearchHandler() != 0 ) {
      SETSTATE( SearchReqIntData::SEARCHING );
   } else {
      m_data->m_status =
         StringTable::stringCode( m_data->m_searchHandler->getStatus() );
      SETSTATE( SearchReqIntData::ERROR );
   }
}



SearchRequest::~SearchRequest() {
   delete m_data->m_searchHandler;
   delete m_data->m_coveredIDsRequest;
   delete m_data->m_packetToDelete;
   delete m_data;
}


void
SearchRequest::deleteRequestPacket()
{
   mc2dbg4 << "SearchRequest::deleteRequestPacket()" << endl;
   delete m_data->m_packet;
   m_data->m_packetToDelete = NULL;
}

void
SearchRequest::handleCoordForOrigReceived(PacketContainer* pack)
{
   CoordinateOnItemReplyPacket* reply =
      static_cast<CoordinateOnItemReplyPacket*>(pack->getPacket());

   // Check the status of the packet.
   if ( reply->getStatus() != StringTable::OK || reply->getNbrItems() < 1 ) {
      if ( reply->getStatus() == StringTable::OK ) {
         m_data->m_status = StringTable::NOTOK;
      } else {
         m_data->m_status = (StringTable::stringCode)reply->getStatus();
      }
      SETSTATE ( SearchReqIntData::ERROR );
      return;
   } else {
      // Get the position from the packet
      int32 lat;
      int32 lon;
      uint32 itemID;
      reply->getLatLong(0, itemID, lat, lon);
      m_data->m_position = MC2Coordinate(lat, lon);
      initCoveredIDs(m_data->m_position, m_data->m_radiusMeters);
   }   
}

int
SearchRequest::enqueuePacketsFromSearchHandler()
{
   int nbrPacks = 0;
   for ( PacketContainer* pack = m_data->m_searchHandler->getNextPacket();
         pack != NULL;
         pack = m_data->m_searchHandler->getNextPacket() ) {
      enqueuePacketContainer(pack);
      ++nbrPacks;
   }
   mc2dbg2 << "[SR]: " << nbrPacks << " packets from SearchHandler" 
           << endl;
   return nbrPacks;
}


void
SearchRequest::handleCoveredIDsDone()
{
   const CoveredIDsRequest::result_t& coveredIDs =
      m_data->m_coveredIDsRequest->getCoveredIDs();

   // Get the map ids. (Move this code to the SearchHandler later)
   uint32* mapIDs = new uint32[coveredIDs.size()];
   uint32* maskItems = new uint32[coveredIDs.size()];
   const char** maskNames = new const char*[coveredIDs.size()];

   int idx = 0;
   for ( CoveredIDsRequest::result_t::const_iterator it = coveredIDs.begin();
         it != coveredIDs.end();
         ++it ) {
      if ( m_allowedMaps == NULL || m_allowedMaps->find( 
              it->second.getMapID() ) != m_allowedMaps->end() )
      {
         mapIDs[idx] = it->second.getMapID();
         maskItems[idx] = it->second.getItemID();
         maskNames[idx] = "";
         ++idx;
      }
   }
   
   // Use the ID:s as masks for SearchHandler.
   UserSearchRequestPacket* usrPack;

   // Remember the packet so that we can delete it.
   m_data->m_packetToDelete = m_data->m_packet =
      usrPack = new UserSearchRequestPacket();
   const char* zipCode = "";
   if ( m_data->m_params.getAddStreetNamesToCompanies() ) {
      // Set the zipCode to "AddStreetNamesToCompanies".
      zipCode = "AddStreetNamesToCompanies";
   }

   usrPack->encodeRequest( idx,
                           mapIDs,
                           zipCode,
                           0,
                           NULL,
                           SEARCH_REGION_TYPES, // Should not be used
                           0,    // nbrcat
                           NULL, // cat
                           idx,   
                           maskItems,
                           maskItems,
                           maskNames,
                           m_data->m_params.getDBMask(),
                           m_data->m_itemQuery.c_str(),
                           m_data->m_params.getMinNbrHits(),
                           m_data->m_params.getMatchType(),
                           m_data->m_params.getStringPart(),
                           m_data->m_params.getSortingType(),
                           m_data->m_params.getSearchForTypes(),
                           m_data->m_params.getRequestedLang(),
                           m_data->m_params.getNbrSortedHits(),
                           m_data->m_params.getEditDistanceCutOff(),
                           m_data->m_params.getRegionsInMatches(),
                           m_data->m_params.getCategories(),
                           StringTable::countryCode(0));
   
   m_data->m_searchHandler =
      new SearchHandler( m_data->m_packet, this,
                         m_data->m_params,
                         m_topRegionRequest,
                         false,  // unique or full
                         false,  // search only if u.o.f
                         m_data->m_params.getRegionsInMatches(),
                         m_data->m_params.getRegionsInOverviewMatches(),
                         true,   // Lookup coordinates
                         false,  // Lookup bboxes
                         m_data->m_position,
                         &m_data->m_overParams );
   
   // Get some packets...
   if ( enqueuePacketsFromSearchHandler() > 0 ) {
      SETSTATE( SearchReqIntData::SEARCHING );
   } else {
      m_data->m_status =
         StringTable::stringCode( m_data->m_searchHandler->getStatus() );
      SETSTATE( SearchReqIntData::ERROR );
   }

   delete [] mapIDs;
   delete [] maskItems;
   delete [] maskNames;   
}


void
SearchRequest::processPacket( PacketContainer *pack ) 
{
   // Set to true if we got a packet in an unexpected state
   bool notHandled = false;
   
   mc2dbg2 << "[SR]: Nbroutstanding packets = "
           << getNbrOutstandingPackets()
           << endl;
   switch ( m_data->m_state ) {
      case SearchReqIntData::GETTING_COORD_FOR_ORIG:
         mc2dbg2 << "[SR]: Got coord for orig" << endl;
         handleCoordForOrigReceived(pack);
         delete pack;
         pack = NULL;
         break;
      case SearchReqIntData::USING_COVERED_ID_REQ:
         if ( processSubRequestPacket(m_data->m_coveredIDsRequest,
                                      pack,
                                      m_data->m_status) ) {
            // Other request done
            if ( m_data->m_status == StringTable::OK ) {
               handleCoveredIDsDone();
            } else {
               SETSTATE(SearchReqIntData::ERROR);
            }
         } else {
            // Keep on truckin' 
         }
         break;
      case SearchReqIntData::SEARCHING:
         if ( !m_data->m_searchHandler->processPacket( pack ) ) {
            mc2dbg << "SearchRequest::processPacket unknown packet received"
                   << endl;
            m_data->m_status =
               StringTable::stringCode(m_data->m_searchHandler->getStatus());
            SETSTATE(SearchReqIntData::ERROR);
         } else {
            if ( ! m_data->m_searchHandler->requestDone() ) {
               enqueuePacketsFromSearchHandler();
            }

            // Check again to make sure.
            if ( m_data->m_searchHandler->requestDone() ) {
               if ( m_data->m_searchHandler->getStatus() == StringTable::OK ) {
                  SETSTATE(SearchReqIntData::DONE_OK);
               } else {
                  SETSTATE(SearchReqIntData::ERROR);
                  m_data->m_status =
                     StringTable::stringCode(
                        m_data->m_searchHandler->getStatus());
               }
            }
         }
         break;
      default:
         mc2dbg << "[SearchReq]: Unexpected packet in unexpected state"
                << endl;
         notHandled = true;
         break;
   }
   if ( notHandled == false &&
        m_data->m_state == SearchReqIntData::DONE_OK ) {
      handleDoneOK();
   }
}


PacketContainer*
SearchRequest::getAnswer() 
{
   if ( m_data->m_searchHandler ) {
      // getAnswer in searchHandler sets answer to NULL.
      PacketContainer* temp = m_data->m_searchHandler->getAnswer();
      if ( temp ) {
         return temp;
      }
   }

   // Something was NULL. It is possible that we have not created
   // a SearchHandler at all.
   // Create vanillareply to put the status in for old servers.
   UserSearchRequestPacket* userPack = new UserSearchRequestPacket();
   VanillaSearchReplyPacket* pack =
      new VanillaSearchReplyPacket(userPack);
   pack->setStatus(getStatus());
   PacketContainer* answerContainer =
      new PacketContainer(pack, 0, 0, MODULE_TYPE_INVALID);
   delete userPack;
   return answerContainer;
}

void SearchRequest::printMatches()
{
#if 0
   strstream strea;
   const vector<VanillaMatch*>& matches = getMatches();
   for (uint32 i=0; i < matches.size(); ++i ) {
      VanillaMatch* match = matches[i];
      strea << "[SR]: YYY: Match \"" << match->getName();
      VanillaCompanyMatch* vcm = dynamic_cast<VanillaCompanyMatch*>(match);
      VanillaStreetMatch* vsm = dynamic_cast<VanillaStreetMatch*>(match);
      int streetNbr = 0;
      if ( vcm != NULL ) {
         streetNbr = vcm->getStreetNbr();
      } else if ( vsm != NULL ) {
         streetNbr = vsm->getStreetNbr();
      }
      if ( streetNbr != 0 ) {
         strea << " " << int(streetNbr);
      }
      if ( match->getLocationName() != NULL &&
           strlen(match->getLocationName()) != 0 ) {
         strea << ", " << match->getLocationName();
      }
      strea << "\" : "; 
      strea << match->getPoints() << endl;
      strea << "[SR]: Integer points " << match->getPoints().getPoints()
            << endl;
   }
   strea << ends;
   mc2dbg << strea.str();
   strea.freeze(0); // Will leak mem otherwise.
#endif
}

inline void
SearchRequest::checkDistances( vector<VanillaMatch*>& matches )
{
   if ( m_data->m_bbox.isValid() ) {

      for ( vector<VanillaMatch*>::iterator it = matches.begin();
            it != matches.end(); )
      {
         if ( !m_data->m_bbox.inside( (*it)->getCoords().lat, 
                                      (*it)->getCoords().lon ) )
         {
            // Remove match if it has regions (i.e. not an airport).
            if ( (*it)->getNbrRegions() > 0 ) {
               // Ok - remove.
               mc2dbg4 << "[SReq]: Removing match "
                       << "\"" << (*it)->getName() << " with lat,lon "
                       << (*it)->getCoords() << " dist outside bbox " 
                       << ::sqrt(m_data->m_bbox.squareDistTo(
                                  (*it)->getCoords().lat, 
                                  (*it)->getCoords().lon )) << "m" << endl;
               it = matches.erase(it);
               continue;
            }
         }
         ++it; 
      }
   } else {
      if ( m_data->m_radiusMeters < 0 ) {
         return;
      }

      uint32 compDistance = m_data->m_radiusMeters;

      for ( vector<VanillaMatch*>::iterator it = matches.begin();
            it != matches.end();
            /* */ ) {
         if ( (*it)->getDistance() > compDistance ) {
            // Remove match if it has regions (i.e. not an airport).
            if ( (*it)->getNbrRegions() > 0 ) {
               // Ok - remove.
               mc2dbg4 << "[SReq]: Removing match "
                       << "\"" << (*it)->getName() << " with distance "
                       << (*it)->getDistance() << endl;
               it = matches.erase(it);
               continue;
            }
         }
         ++it;
      }
   }
}

void
SearchRequest::handleDoneOK()
{
   mc2dbg2 << "[SReq]: HandledoneOK" << endl;
   //const vector<VanillaMatch*>& matches = getMatches();

   // FIXME: Don't do this all the times.
   m_data->m_searchHandler->sortOverviews();
   
   // Check distance of matches,
   checkDistances(m_data->m_searchHandler->getMatchesForWriting());   
   
   // Update locationnames
   printMatches();
}

bool 
SearchRequest::requestDone()
{
   return (m_data->m_state == SearchReqIntData::DONE_OK) ||
          (m_data->m_state == SearchReqIntData::ERROR );
}


uint32
SearchRequest::getNbrOverviewMatches() const
{
   if ( m_data->m_searchHandler != NULL ) {
      return m_data->m_searchHandler->getNbrOverviewMatches();
   } else {
      return 0;
   }
}


OverviewMatch* 
SearchRequest::getOverviewMatch( uint32 index ) const
{
   return m_data->m_searchHandler->getOverviewMatch( index );
}


OverviewMatch** 
SearchRequest::getOverviewMatches()
{
   if ( m_data->m_searchHandler ) {
      return m_data->m_searchHandler->getOverviewMatches();
   } else {
      return NULL;
   }
}

uint32 
SearchRequest::getUniqueOrFull()
{
   return m_data->m_searchHandler->getUniqueOrFull();
}

StringTable::stringCode
SearchRequest::getStatus() const
{   
   switch ( m_data->m_state ) {
      case SearchReqIntData::DONE_OK:
         return StringTable::OK;
      case SearchReqIntData::ERROR:
         mc2dbg2 << "[SR]: Returning status "
                << int(m_data->m_status) << endl;
         return m_data->m_status;
      default:
         return StringTable::TIMEOUT_ERROR;
   }
      
}

const vector<VanillaMatch*>&
SearchRequest::getMatches() const
{
   // Seems like someone uses the matches even when
   // status is !OK.
   static const vector<VanillaMatch*> staticMatches;
   if ( m_data->m_searchHandler != NULL ) {
      return m_data->m_searchHandler->getMatches();
   } else {
      return staticMatches;
   }
}

const SearchRequestParameters& 
SearchRequest::getSearchParameters() const {
   return m_data->m_params;
}


MC2Coordinate 
SearchRequest::getPosition() const {
   return m_data->m_position;
}
EmptySearchRequest::EmptySearchRequest():
   SearchResultRequest( RequestData( 0, (const UserUser*)NULL ) ) {
}
const vector<VanillaMatch*>& EmptySearchRequest::getMatches() const {
   static vector<VanillaMatch*> empty;
   return empty;
}

const SearchRequestParameters& 
EmptySearchRequest::getSearchParameters() const {
   static SearchRequestParameters params;
   return params;
}

void EmptySearchRequest::processPacket( PacketContainer* pack ) {
   // nothing to do here
}

StringTable::stringCode EmptySearchRequest::getStatus() const {
   // empty is always ok
   return StringTable::OK;
}


