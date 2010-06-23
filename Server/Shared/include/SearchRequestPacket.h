/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHREQUESTPACKET_H
#define SEARCHREQUESTPACKET_H

#include "Packet.h"
#include "SearchParams.h"
#include "SearchTypes.h"
#include "StringTable.h"
#include <vector>

class ItemIDTree;
class MC2BoundingBox;
class MC2Coordinate;
class SearchRequestParameters;
class UserRightsMapInfo;
class IDPair_t;

/**
 *   This packet should be the only searchpacket needed for now.
 *   (Overview/User/Proxy-search should be possible to do).
 *   Currently user-search is supported.
 */
class SearchRequestPacket : public RequestPacket {
public:
   enum search_t {
      /// Same as old UserSearchRequestPacket.
      USER_SEARCH      = 0,
      /// Search for cities etc. (on overview maps)
      OVERVIEW_SEARCH  = 1,
      /// Means that the items in allowedRegions should be expanded to matches
      PROXIMITY_SEARCH = 2
   };
   
   /**
    *   Constructor for use in server.
    *   @param mapID          Map to send packet to.
    *   @param searchType     Type of search to perform.
    *   @param searchString   String to search for.
    *   @param params         Search parameters.
    *   @param allowedRegions The ids of the allowed regions.
    *                         (If proximity packet the items to be converted
    *                         to matches should be here).
    *   @param alllowedBBoxes 
    */
   SearchRequestPacket(uint32 mapID,
                       uint32 packetID,
                       uint32 reqID,
                       search_t searchType,
                       const char* searchString,
                       const SearchRequestParameters& params,
                       const vector<IDPair_t>& allowedRegions,
                       const vector<MC2BoundingBox>& alllowedBBoxes,
                       const MC2Coordinate& sortOrigin,
                       const UserRightsMapInfo& rights );

   /**
    *   Returns the search type.
    */
   search_t getSearchType() const;
   
   /**
    *   Returns the sort origin.
    */
   MC2Coordinate getSortOrigin() const;

   /**
    *   Returns the regions and bounding boxes.
    */
   void getParamsAndRegions(SearchRequestParameters& params,
                            char*& searchString,
                            vector<IDPair_t>& allowedRegions,
                            vector<MC2BoundingBox>& allowedBBoxes,
                            UserRightsMapInfo& rights ) const;
   
private:

   /// Calculates packet size
   static int calcPacketSize(const char* searchString,
                             const SearchRequestParameters& params,
                             const vector<IDPair_t>& allowedRegions,
                             const vector<MC2BoundingBox>& alllowedBBoxes,
                             const UserRightsMapInfo& rights );
   
   /// The position where the search type is stored.
   static const int SEARCH_TYPE_POS = REQUEST_HEADER_SIZE;
   /// The position where the sort origin is stored
   static const int SORT_ORIGIN_POS = SEARCH_TYPE_POS + 4;
   /// The position where the parameters are stored
   static const int PARAMETERS_POS = SORT_ORIGIN_POS + 8;
};

/**
 *    Superclass to all old searchrequestpackets
 *
 *    The SearchRequestHeader lookes like this:
 *    \begin{verbatim}
 *       SortingType                   4
 *       nbrMapIDs                     4
 *       mapID * nbrMapIDs             4 * <NBR_MAP_IDS>
 *       nbrLocations                  4
 *       nbrMasks                      4
 *       nbrCategories                 4
 *       locationType                  4
 *       regionsInMatches              4
 *       topRegion                     4
 *       sourceMask                    1
 *       zipCode                       string
 *       <MASK> * nbrMasks             8 * <NBR_MASKS>
 *       <CATEGORY> * nbrCategories    4 * <NBR_CATEGORIES>
 *       <LOCATION> * nbrLocations     strings
 *       <MASK_NAME> * nbrMasks        strings
 *    \end{verbatim}
 *
 */
class OldSearchRequestPacket : public RequestPacket {
  public:
   /**
    *    Default constructor
    */
   OldSearchRequestPacket();

   /**
    * Constructor
    * @param subType the subtype of the request
    * @param packetId the packetId of the packet
    * @param reqID the request id of the packet
    */
   OldSearchRequestPacket( uint16 subType,
                        uint16 packetId,
                        uint16 reqID );

   /**
    * Constructor
    * @param bufLength the length of the buffer
    * @param prio the priority of the request
    * @param subType the subtype of the request
    * @param packetId the packetId of the packet
    * @param requestID the requestID of the packet
    * @param mapID the mapID of this search request
    */
   OldSearchRequestPacket( uint32 bufLength,
                        byte prio,
                        uint16 subType,
                        uint16 packetId,
                        uint16 requestID,
                        uint32 mapID );


   /**
    * @return the number of mapIDs of this search request
    */
   uint32 getNumMapID() const {
      return readLong( REQUEST_HEADER_SIZE + 4 );
   }

   /**
    * Returns a particular mapID
    * @param index the index of the mapID
    * @return the mapID number index
    */
   uint32 getMapID( uint32 index ) const {
      return readLong( REQUEST_HEADER_SIZE + 4 + 4 + index*4 );
   }
   
   /**
    *    Get the type of sorting that should be used for the result.
    *    @return  The type of sorting used to sort the
    *             matches from the search.
    */
   const uint8 getSortingType() const {
      return readLong( REQUEST_HEADER_SIZE );
   }

   /**
    * @return the zip code, if a particular zip code
    *         was requested, or zero.
    */
   const char* getZipCode() const ;

   /**
    * @return nbrLocation
    */
   uint32 getNbrLocations() const ;
      
//   /**
//    * @return locations, delete the vector but not the strings.
//    */
//   const char** getLocations() const;

   /**
    * @return nbrMasks
    */
   uint32 getNbrMasks() const;

//   /**
//    * @return masks, delete the vector.
//    */
//   const uint32* getMasks() const;

   /**
    * Gets the number of categories.
    * @return The number of categories.
    */
   uint32 getNbrCategories() const;

   /**
    * Gets the categories.
    * @return The categories.
    */
//   const uint32* getCategories() const; anyone need this?

   /**
    * The The search region types or:ed together that should be in the 
    * reply matches.
    */
   uint32 getRegionsInMatches() const;
   
   /**
    *    Sets a mapID.
    *    @param mapIDnbr the index of the mapID
    *    @param mapID the mapID
    */
   void setMapID( uint32 mapIDnbr, uint32 mapID ) {
      int position = REQUEST_HEADER_SIZE + 4 + mapIDnbr*4 ;
      writeLong( position, mapID );
   }

   /**
    * Writes a search request header
    * @param sortingType the type of sorting used to sort the hits.
    * @param numMapID the number of mapIDs
    * @param mapID a vector(!) of mapIDs
    * @param zipCode a particular zipcode or part of a zipcode
    *        or 0 if the zip code does not matter.
    * @param nbrLocations is the number of location string in locations.
    * @param locations is a vector with strings describing locations.
    * @param nbrCategories The number of categories.
    * @param categories The categories.
    * @param nbrMasks is the number of masks in masks.
    * @param masks is the vector with the masks limiting the search in
    *        the map. If nbrMasks is 0 masks may be NULL.
    * @param maskItemIDs is the itemIDs of the masks.
    * @param maskNames is the names describing the masks.
    * @param dbMask A mask of the allowed databases.
    * @param regionsInMatches The search region types or:ed together 
    *                         that should be in the reply matches.
    * @param topRegion The top region to perform the search in.
    *                  Only applies when making an overviewsearch in 
    *                  SearchRequest.
    */
   int writeHeader( SearchTypes::SearchSorting sortingType,
                    uint32 numMapID,
                    const uint32* mapID,
                    const char* zipCode,
                    uint32 nbrLocations,
                    const char** locations,
                    uint32 locationType,
                    uint32 nbrCategories,
                    const uint32* categories,
                    uint32 nbrMasks,
                    const uint32* masks,
                    const uint32* maskItemIDs,
                    const char** maskNames,
                    uint8 dbMask,
                    uint32 regionsInMatches,
                    StringTable::countryCode topRegion,
                    const vector<MC2BoundingBox>& bboxes =
                    vector<MC2BoundingBox>());

   /**
    *    Reads the header of the search request and sets the 
    *    parameters.
    *    @param sortingType the sortingType of the packet
    *    @param zipCode the zipcode of the packet
    *    @param maskNames is the names describing the masks.
    *    @param dbMask A mask of the available databases.
    *    @param regionsInMatches The search region types or:ed together 
    *                            that should be in the reply matches.
    *    @param topRegion The top region to perform the search in.
    *                     Only applies when making an overviewsearch in 
    *                  SearchRequest.
    *    The following params are allocated here and should be deleted
    *    by the caller.
    *    masks, maskItemIDs, categories, locations, maskNames
    */
   int readHeader( SearchTypes::SearchSorting& sortingType,
                   char*& zipCode,
                   uint32& nbrLocations,
                   char**& locations,
                   uint32& locationType,
                   uint32& nbrCategories,
                   uint32*& categories,
                   uint32& nbrMasks,
                   uint32*& masks,
                   uint32*& maskItemIDs,
                   char**& maskNames,
                   uint8& dbMask,
                   uint32& regionsInMatches,
                   StringTable::countryCode& topRegion,
                   vector<MC2BoundingBox>& bboxes
                   ) const;

   /**
    *    Get the position for the end of the header in this packet.
    *    @return  Position of the end of the header in this packet.
    */
   int getHeaderEndPosition() const;

   /**
    *    Dump the packet content as text to standard error.
    *    @param   headerOnly  Optional parameter that should be set to
    *                         true if oly the header of the packet
    *                         should be printed.
    *    @param   do a dns lookup of the IP. Can be very time
    *             consuming if the IP is invalid.
    */
    virtual void dump(bool headerOnly = false, 
                      bool lookupIP = false ) const;

};


/**
 * Packet for searching out locations from a string naming a location.
 *
 *   \\ After the normal header the packet contains:
 *   \begin{tabular}{lll}
 *      Pos                   & Size     & Destription \\ \hline
 *      REQUEST_HEADER_SIZE   & 4 bytes  & locationType \\
 *      +4                    & 4 bytes  & requestedLanguage \\
 *      +8                    & 4 bytes  & nbrLocations \\
 *      +12                   & 4 bytes  & mapID \\
 *      +16                   & 4 bytes  & regionsInMatches \\
 *      +20                   & 1 byte   & uniqueOrFull \\
 *      +21                   & 1 byte   & matching \\
 *      +22                   & 1 byte   & stringpart \\
 *      +23                   & 1 byte   & dbMask \\
 *      +24                   & 1 byte   & minNbrHits \\
 *      for nbrLocations      & string   & location(s) \\
 *   \end{tabular}
 *
 */
class OverviewSearchRequestPacket : public RequestPacket {
  public:
   /**
    * Constructor.
    * @param packetID is the id of the packet in the request
    * @param requestID is the id of the request sending the packet
    * @param location is the string to match for locations
    * @param locationType is locationType mask.
    * @param uniqueOrFull if true, only unique locations or those matching
    *                     the string fully are returned.
    * @param dbMask A mask of the available databases.
    * @param regionsInMatches The search region types or:ed together 
    *                         that should be in the reply matches.
    * @param itemTree         ItemTree containing the allowed maps.
    */
   OverviewSearchRequestPacket( uint16 packetID,
                                uint16 requestID,
                                uint32 nbrLocations,
                                const char** locations,
                                uint32 locationType,
                                uint32 requestedLanguage,
                                uint32 mapID,
                                SearchTypes::StringMatching matching,
                                SearchTypes::StringPart stringPart,
                                SearchTypes::SearchSorting searchType,
                                bool uniqueOrFull,
                                uint8 dbMask,
                                uint8 minNbrHits,
                                uint32 regionsInMatches,
                                const ItemIDTree& idTree);

   /**
    *   The strings to match locations with, delete the vector but not
    *   the strings.
    *   @param idTree Place to put the ItemIDTree into.
    */
   const char** getLocationStrings(ItemIDTree& idTree) const;


   /**
    * The locationType.
    */
   uint32 getLocationType() const;

   /**
    * The requested language.
    * @return The requested language.
    */
   uint32 getRequestedLanguage() const;

   /**
    * The number of location strings.
    */
   uint32 getNbrLocations() const;


   /**
    *    The uniqueorfull bool
    */
   bool getUniqueOrFull() const;

   /** @return requested mapID, or MAX_UINT32 */
   uint32 getMapID() const;

   /**
    * the matching
    */
   SearchTypes::StringMatching getMatching() const;

   /**
    * the stringpart
    */
   SearchTypes::StringPart getStringPart() const;

   /**
    *  the type of sorting
    */
   SearchTypes::SearchSorting getSortingType() const;
   
   /**
    * The database mask.
    */
   uint8 getDBMask() const;

   /**
    *  Gets the minimum number of hits in the reply.
    *  @ return The min number of hits.
    */
   uint8 getMinNbrHits() const;

   /**
    * The The search region types or:ed together that should be in the 
    * reply matches.
    */
   uint32 getRegionsInMatches() const;
};

#endif





