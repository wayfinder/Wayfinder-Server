/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDCOUNTRYOVERVIEWMAP_H
#define OLDCOUNTRYOVERVIEWMAP_H

#include "config.h"
#include "OldGenericMap.h"
#include <map>

class Stack;

/**
 *    Objects of this class serves as an overview of one country. So 
 *    this class contains information that is general about the whole
 *    country, such as countryname, currency, drive on left or right 
 *    side of the road and the categories in the yellow pages. The ID 
 *    of the maps in the country is also stored here.
 *
 *    The inherritance-structure for the maps should be better. 
 *    Something like:
 *    @verbatim
 *                OldGenericMap
 *                     |
 *            ________/_\________
 *            |                 |
 *           OldMap            OldOverviewMap
 *                              |
 *                      _______/_\________
 *                      |                |
 *             OldCountryOverviewMap  WorldOverviewMap
 *
 * 
 *       might be better than the current structure:
 *
 *                OldGenericMap
 *                     |
 *       _____________/_\_____________
 *       |             |             |
 *      OldMap       OldOverviewMap OldCountryOverviewMap
 *
 *    @endverbatim
 *
 */
class OldCountryOverviewMap : public OldGenericMap {
   public:
      /**
       *    The number of simplifications (filtered) of the GfxData of the 
       *    country map. 
       */
      static const byte NBR_SIMPLIFIED_COUNTRY_GFX;

      /**
       *    Create an new OldCountryOverviewMap with given ID. No data
       *    is inserted into the map.
       *    @param   id The ID of the new map.
       */
      OldCountryOverviewMap(uint32 id);

      /**
       *    Create a new OldCountryOverviewMap. The map is {\bf not} filled 
       *    with data from the file. This must be done by calling the 
       *    load()-method after creation.
       *
       *    @param id      The ID of the map.
       *    @param path    The path to the file on the filesystem.
       */
      OldCountryOverviewMap(uint32 id, const char* path);

      /**
       *    Delete this country map.
       */
      virtual ~OldCountryOverviewMap();

      /**
       *    Get the number of maps in this country.
       *    @return The number of maps in this county.
       */
      inline uint32 getNbrMaps();

      /**
       *    Get the ID of one of the maps in this country.
       *    @param  i   The index of the map to return. Valid values are
       *                0 <= i < getNbrMaps().
       *    @param  
       *    @return The ID of map number i in this country. MAX_UINT32
       *            is returned upon failure.
       */
      inline uint32 getMapData(uint32 i, uint32& creationTime,
                               int32& maxLat, int32& minLon,
                               int32& minLat, int32& maxLon);

      /**
       *
       *
       */
      inline uint32 getMaximunSizeOfStringItems();

      /**
       *    Save the string items into the data buffer. Uses the protocol
       *    described in the design-document, {\tt <STRING ITEMS>}.
       */
      bool saveStringItems(DataBuffer* dataBuffer);

      /**
       *    Get a stack of indices of a certain level of simplified
       *    country gfxdata.
       *    @param   level    The level of simplification. 0 is the most
       *                      simplified level,
       *                      NBR_SIMPLIFIED_COUNTRY_GFX - 1 the least.
       *    @param   polygon  The polygon of the country gfxdata.
       *    @return  A const stack containing the indices of the specified
       *             country gfxdata polygon. If the level or polygon is
       *             out of bounds, NULL is returned.
       */
      inline const Stack* getFilterStack(uint32 level, uint32 polygon);

      /**
       *    Get the ID of one item in the map where it is stored "for real".
       *    @param itemID     The ID of the item in this country overview map.
       *    @param origMapID  Outparameter that is set to the ID of the map
       *                      where the item is stored for real.
       *    @param origItemID Outparameter that is set to the ID of the item
       *                      in map with origMapID.
       *    @return True if the outparameters are set, false otherwise.
       */
      inline bool getOriginalIDs(uint32 itemID, 
                                 uint32& origMapID,
                                 uint32& origItemID);
      
      /**
       *    Get the ID on this overview map when the orignal map- and 
       *    item-ID are known. A linear search among all items on the
       *    ma is performed on each call, so the call is quite 
       *    expensive.
       *    
       *    @param origMapID  The ID of the map where the item is located
       *                      "for real".
       *    @param origItemID The ID of the item on the map where it is 
       *                      located "for real".
       *    @return The ID of the item in this map.
       */
      inline uint32 getOverviewID(uint32 origMapID, uint32 origItemID);
     
      /**
       *    Updates the creation times stored in this map.
       *    @param   creationTimeByMap STL map with map id as key and
       *                               creation time of that map as
       *                               value. Note that the STL map may
       *                               contain map ids not present in
       *                               the country map.
       *    @param   onlyPerformCheck  Optional parameter that if specified
       *                               indicates that no actual update will be 
       *                               performed.
       *    @return  True if any creation times were updated.
       */
      bool updateCreationTimes( map<uint32, uint32> creationTimeByMap,
                                bool onlyPerformCheck = false );
 

      /**
       *    Create the array of different levels of simplified country
       *    gfxdata.
       *    @return  True if successful, false otherwise.
       */
      bool makeSimplifiedCountryGfx();
      
   /**
    *
    *
    */
   struct originalIDs_t {
      // Less than operator, used for sorting.
      bool operator < ( const originalIDs_t& other ) const {
         if ( origMapID < other.origMapID ) 
            return true;
         if ( origMapID > other.origMapID ) 
            return false;
         // Same map id, use id instead.
         return (origItemID < other.origItemID);
      };
      
      uint32 origItemID;
      uint32 origMapID;
         
   };

   typedef std::map<uint32, struct originalIDs_t > OriginalIDsMap;
   /**
    *
    */
   struct mapnotice_t {
      uint32 mapID;
      uint32 creationTime;
      int32 maxLat;
      int32 minLon;
      int32 minLat;
      int32 maxLon;
   };

   typedef std::vector< struct mapnotice_t > MapNoticeVector;

   const MapNoticeVector& getMapsInCountry() const { return m_mapsInCountry; }
   const OriginalIDsMap& getOriginalIDs() const { return m_originalIDs; }
   
   uint32 getNbrGfxPolygons() const { return m_nbrGfxPolygons; }
   Stack*** const getStack() const { return m_simplifiedGfxStack; }

protected:
   friend class M3Creator;

      /**
       *    Delete the array representing the simplified gfxdata.
       */
      void deleteSimplifiedGfxStack();

      /**
       *    Calls the normal internalSave-method, but after that, the
       *    ID:s of the maps in this country are saved.
       *    @param  outfile The fd of the file where to write the ID:s.
       *    @retutn True if the IDs are saved, false otherwise.
       */
      virtual bool internalSave(int outfile);

      /**
       *    Calls the normal internalLoad-method, but after that, the
       *    ID:s of the maps in this country are loaded.
       *    @param  outfile The fd of the file where to loade the ID:s
       *                    from.
       *    @retutn True if the IDs are loaded, false otherwise.
       */
      virtual bool internalLoad(DataBuffer& dataBuffer);


      /**
       *    The ID, creationtime and boundingbox of the maps that are 
       *    located in this country. One map can only be part of one 
       *    country!
       */
      MapNoticeVector m_mapsInCountry;

      /**
       *
       */
      OriginalIDsMap m_originalIDs;

      /**
       *
       */
      static const uint32 STRING_ITEM_SIZE = 44;

      /**
       *    The number of polygons in the country gfxdata.
       */
      uint16 m_nbrGfxPolygons;
      
      /**
       *    An array containing the indices of the simplified GfxDatas
       *    of the country map. 
       *    The array will be allocated to be 
       *    [NBR_SIMPLIFIED_COUNTRY_GFX][m_nbrGfxPolygons] large.
       *
       *    The first index refers to the level of simplification
       *    (lower index means more simplification), and the second
       *    refers to a certain polygon.
       *    
       *    Each of the elements contain a Stack with
       *    indices of the coordinates of the GfxData of the country 
       *    (ie. the usual output that you would get using
       *    GfxData::getSimplifiedPolygon()).
       */
      Stack*** m_simplifiedGfxStack;
      
      
      /**
       *    @name Static constants for the filtering parameters.
       */
      //@{
         /**
          *    Max distance between two coordinates 
          *    for the different simplifications of the 
          *    country gfxdata.
          */
         static const uint32 filtMaxDistance[];
         
         /**
          *    Minimum distance between two coordinates 
          *    for the different simplifications
          *    of the country gfxdata.
          */
         static const uint32 filtMinDistance[];
         
         /**
          *    The filter levels of map gfx data (filtered in levels) that 
          *    should be used for the different simplifications of the
          *    country polygon.
          */
         static const uint32 mapGfxFilterLevels[];
      //@}
};

// ========================================================================
//                                      Implementation of inlined methods =

inline uint32 
OldCountryOverviewMap::getNbrMaps()
{
   return m_mapsInCountry.size();
}

inline uint32 
OldCountryOverviewMap::getMapData(uint32 i, uint32& creationTime,
                               int32& maxLat, int32& minLon,
                               int32& minLat, int32& maxLon)
{
   if (i < getNbrMaps()) {
      creationTime = m_mapsInCountry[i].creationTime;
      maxLat = m_mapsInCountry[i].maxLat;
      minLon = m_mapsInCountry[i].minLon;
      minLat = m_mapsInCountry[i].minLat;
      maxLon = m_mapsInCountry[i].maxLon;
      return (m_mapsInCountry[i].mapID);
   }
   return (MAX_UINT32);
}

inline uint32 
OldCountryOverviewMap::getMaximunSizeOfStringItems()
{
   return (4 + m_originalIDs.size() * STRING_ITEM_SIZE);
}


inline bool
OldCountryOverviewMap::getOriginalIDs(uint32 itemID,
                                   uint32& origMapID,
                                   uint32& origItemID)
{
   OriginalIDsMap::iterator p = m_originalIDs.find(itemID);
   if (p != m_originalIDs.end()) {
      origItemID = p->second.origItemID;
      origMapID = p->second.origMapID;
      DEBUG8(cerr << "OldCountryOverviewMap::getOriginalIDs returning "
                  << origMapID << "." << origItemID << endl);
      return (true);
   } else {
      origItemID = MAX_UINT32;
      origMapID = MAX_UINT32;
      return (false);
   }
}

inline uint32 
OldCountryOverviewMap::getOverviewID(uint32 origMapID, uint32 origItemID)
{
   // Linear search among all the items in the m_originalIDs-structure
   OriginalIDsMap::const_iterator p = m_originalIDs.begin();
   while ( (p!=m_originalIDs.end()) && 
           ( ( (*p).second.origMapID != origMapID) ||
             ( (*p).second.origItemID != origItemID) ) ) {
      ++p;
   }

   // Return the id in the found iterator, or MAX_UINT32 if not found
   if (p != m_originalIDs.end()) {
      return (*p).first;
   }
   return MAX_UINT32;
}

      
inline const Stack* 
OldCountryOverviewMap::getFilterStack(uint32 level, uint32 polygon)
{
   if ( (level < NBR_SIMPLIFIED_COUNTRY_GFX) && 
        (polygon < m_nbrGfxPolygons) ) {
      return ((const Stack*) m_simplifiedGfxStack[level][polygon]);
   } else { 
      return (NULL);
   }
}
      
#endif

