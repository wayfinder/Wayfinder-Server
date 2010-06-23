/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COUNTRYOVERVIEWMAP_H
#define COUNTRYOVERVIEWMAP_H

#include "config.h"
#include "GenericMap.h"
#include "SimpleArrayObject.h"

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
 *                GenericMap
 *                     |
 *            ________/_\________
 *            |                 |
 *           Map            OverviewMap
 *                              |
 *                      _______/_\________
 *                      |                |
 *             CountryOverviewMap  WorldOverviewMap
 *
 * 
 *       might be better than the current structure:
 *
 *                GenericMap
 *                     |
 *       _____________/_\_____________
 *       |             |             |
 *      Map       OverviewMap CountryOverviewMap
 *
 *    @endverbatim
 *
 */
class CountryOverviewMap : public GenericMap {
public:
   /**
    *    The number of simplifications (filtered) of the GfxData of the 
    *    country map. 
    */
   static const byte NBR_SIMPLIFIED_COUNTRY_GFX;

   /**
    *    Create an new CountryOverviewMap with given ID. No data
    *    is inserted into the map.
    *    @param   id The ID of the new map.
    */
   CountryOverviewMap(uint32 id);

   /**
    *    Create a new CountryOverviewMap. The map is {\bf not} filled 
    *    with data from the file. This must be done by calling the 
    *    load()-method after creation.
    *
    *    @param id      The ID of the map.
    *    @param path    The path to the file on the filesystem.
    */
   CountryOverviewMap(uint32 id, const char* path);

   /**
    *    Delete this country map.
    */
   virtual ~CountryOverviewMap();

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
    *
    *
    */
   struct originalIDs_t {
      uint32 itemID;
      uint32 origItemID;
      uint32 origMapID;
   };

   // operator to sort ids 
   struct SortIDs:
      public binary_function<
      const originalIDs_t,
      const originalIDs_t, bool > {
      
      bool operator () ( const originalIDs_t& a,
                         const originalIDs_t& b ) const {
         return a.itemID < b.itemID;
      }                     
   };

   // operator to search ids 
   struct SearchIDs:
      public binary_function<
      const originalIDs_t,
      uint32, bool > {
      bool operator () ( const originalIDs_t& a, uint32 b ) const {
         return a.itemID < b;
      }
   };
      


   typedef SimpleArrayObject<originalIDs_t> OriginalIDs;

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

   MapNoticeVector& getMapsInCountry() { return m_mapsInCountry; }
   const MapNoticeVector& getMapsInCountry() const { return m_mapsInCountry; }
   OriginalIDs& getOriginalIDs() { return m_originalIDs; }
   const OriginalIDs& getOriginalIDs() const { return m_originalIDs; }
   

   void setNbrGfxPolygons(uint32 nbr) { m_nbrGfxPolygons = nbr; }

private:

   // for m_simplifiedStack
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
   OriginalIDs m_originalIDs;

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
CountryOverviewMap::getNbrMaps()
{
   return m_mapsInCountry.size();
}

inline uint32 
CountryOverviewMap::getMapData(uint32 i, uint32& creationTime,
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
CountryOverviewMap::getMaximunSizeOfStringItems()
{
   return (4 + m_originalIDs.size() * STRING_ITEM_SIZE);
}


inline bool
CountryOverviewMap::getOriginalIDs(uint32 itemID,
                                   uint32& origMapID,
                                   uint32& origItemID)
{
   
   OriginalIDs::iterator it = lower_bound( m_originalIDs.begin(),
                                           m_originalIDs.end(),
                                           itemID,
                                           SearchIDs() );

   if ( it != m_originalIDs.end() && itemID == (*it).itemID ) {
      origItemID = (*it).origItemID;
      origMapID = (*it).origMapID;
      return true;
   }

   origItemID = MAX_UINT32;
   origMapID = MAX_UINT32;

   return false;
}

inline const Stack* 
CountryOverviewMap::getFilterStack(uint32 level, uint32 polygon)
{
   if ( (level < NBR_SIMPLIFIED_COUNTRY_GFX) && 
        (polygon < m_nbrGfxPolygons) ) {
      return ((const Stack*) m_simplifiedGfxStack[level][polygon]);
   } else { 
      return (NULL);
   }
}
      
#endif

