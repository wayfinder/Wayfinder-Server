/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSCOUNTRYOVERVIEWMAP
#define GMSCOUNTRYOVERVIEWMAP


#include "config.h"
#include "OldCountryOverviewMap.h"
#include "OldOverviewMap.h"
#include <list>

class OldWaterItem;
class OldForestItem;

/**
 *    Subclass to the OldCountryOverviewMap with methods and 
 *    functionallity to create the map-boundry.
 *    
 */
class GMSCountryOverviewMap : public OldCountryOverviewMap {
   public:
      /**
       *    Create a new GMSCountryOverviewMap.
       *    @param   id The ID of this map.
       */
      GMSCountryOverviewMap(uint32 id);

      /**
       *    Create a new GMSCountryOverviewMap.
       *    @param   id    The ID of this map.
       *    @param   path  The path to where this map will be loaded
       *                   and saved.
       */
      GMSCountryOverviewMap(uint32 id, const char* path);

      /**
       *    Calls the normal internalLoad-method, but after that, the
       *    table m_idsByOriginalIDs is created.
       *    @param  outfile The fd of the file where to loade the ID:s
       *                    from.
       *    @return True if the IDs are loaded, false otherwise.
       */
      virtual bool internalLoad(DataBuffer& dataBuffer);
      
      /**
       *    Delete this overviewmap.
       */
      virtual ~GMSCountryOverviewMap();

      /**
       *    Create the map gfx data from the country polygon mif file.
       *    @return  True if the map-boundry is created (the GfxData
       *             for the map is set).
       */
      bool createCountryBorder();

      /**
       *    Add data from a underview map that is in the country 
       *    this country map describes. Appropiate items will be copied
       *    from the underview map to this country overview.
       *    @param   map   The map where to add data (= items etc) from. 
       *    @return  True if the data from map is added to this country, 
       *             false otherwise.
       */
      bool addDataFromMap(OldGenericMap* map);

      /**
       *    Set the gfx filtering of this map, copied from another map.
       *    @param   otherMap The map containing the filtering info
       *                      that we want to add to this map.
       *    @return  True upon success, false otherwise.
       */ 
      bool setSimplifiedGfxFromMap(GMSCountryOverviewMap* otherMap); 

      /**
       *    Update the country polygon of this co map, by copying the 
       *    copying the country polygon from another co map. This is 
       *    used to re-use country polygon filtering from old co maps
       *    to new map generation.
       *    Will use break points in the break points file to write
       *    the countryBorders.txt, the text file that holds filtered
       *    country polygon parts.
       *    Will set the mapGfxDataIsFiltered flag true in this co map.
       *
       *    @param   otherMap The co map that has the gfx data we want to
       *                      apply to this cop map.
       *    @param   breakPointsFileName  The file containing country
       *                      polygon break points. NULL if no such file
       *                      exists.
       */
      bool copyCoPolFilteringFromMap(GMSCountryOverviewMap* otherMap,
                  const char* breakPointsFileName);

      /**
       *    Adds and concatenates the water items in m_waterCandidates.
       *
       *    @param   addedWaterOriginalIDs   Output parameter.
       *                                     Multimap with original map id
       *                                     as key and orignal item id
       *                                     as value for the water items
       *                                     that were added to the
       *                                     country map.
       *    @return  The number of added water items.
       */
      uint32 addWaterItems(multimap<uint32, uint32>& addedWaterOriginalIDs);

      /**
       *    Adds the good forest items in m_forestCandidates to the map.
       *
       *    @param   addedForestOriginalIDs  Output parameter.
       *                                     Multimap with original map id
       *                                     as key and orignal item id
       *                                     as value for the forest items
       *                                     that were added to the
       *                                     country map.
       *    @return  The number of added forest items.
       */
      uint32 addForestItems(multimap<uint32, uint32>& addedForestOriginalIDs);

      /**
       *    Make sure builtup areas and citycentre poi will have
       *    unique names.
       */
      void makeUniqueNames();
      
      /**
       *    Loop through all items of a certain type (for instance bua) 
       *    and make sure that there are unique names for all of them. 
       *    Different items with the same official name will get a new, 
       *    unique name consisting of the old name + the name of the 
       *    municipal it is located in.
       *
       *    Generally bua:s and citycentre poi:s should have unique names.
       *       
       *    @param itemType   The item type that should have unique names.
       *    @param poiType    In case itemType is pointOfInterestType
       *                      then specify which kind of poi-type it
       *                      is.
       *    @param checkItem  If provided the method checks if this item
       *                      (typically a city centre poi added to 
       *                       production maps) needs to be assigned a 
       *                      new unique name. If so, it does.
       */
      void makeUniqueNames( ItemTypes::itemType itemType,
                            ItemTypes::pointOfInterest_t poiType
                                  = ItemTypes::nbr_pointOfInterest,
                            OldItem* checkItem = NULL);

      uint32 addZipCodeItems(OldOverviewMap* overview);

      /**
       *    Update the groups so that they are referring to item ids in
       *    this map instead of item ids in the original maps.
       */
      void updateGroupIDs();
      
      /**
       *    Method that will merge the added street segments from the
       *    underview map to new streetsegments. Street segments will
       *    be merged between intersections of other streetsegments.
       *    The resulting street segments are then filtered.
       *    @return  True upon success, false otherwise.
       */
      bool mergeStreetSegments();


      /**
       *    Method for applying changes to production country overview
       *    maps, typically when adding some extra data or changes 
       *    from WASP to the maps.
       *    The affected items could have been removed from or added to 
       *    the otherMap, or simply had some attribute updated. These 
       *    changes are here transferred to the country overview map.
       *    
       *    @param otherMap            The map from which to apply changes,
       *                               typically an underview map.
       *    @param itemsAffectedInOtherMap   OldItem ids of items that have
       *                                     changed in otherMap.
       *    @param coMapChanged        Outparam that tells if this country 
       *                               overview map was affected by applying 
       *                               the changes.
       *    @returns True.
       */
      bool applyChangesFromOtherMap(
            OldGenericMap* otherMap, vector<uint32> itemsAffectedInOtherMap,
            bool& coMapChanged);

      /**
       *    Create border items. Divides the map gfx data (country polygon)
       *    into border parts using info in the country border break 
       *    points file. The border parts that exist in the borderItems.txt
       *    file are used for creating border items in this map.
       *    The country polygon must be filtered in coordinate levels first!
       *
       *    @param   breakPointsFileName  File with country border break 
       *                                  points.
       *    @return  Number of border items that were created and added
       *             to this co map.
       */
      uint32 createBorderItems( const char* breakPointsFileName );



   private:
      
      /**
       *    Help method for copyCoPolFilteringFromMap and createBorderItems.
       *    Go through this map's gfxData (country polygon) and create
       *    borderGfxs using the country polygon break points file.
       *
       *    @param   breakPointsFileName  File with country border break points.
       *    @param   caller               Caller of this method for debug.
       *    @return  A vector with country border gfxs.
       */
      vector<GfxDataFull*>createBorderGfxs( const char* breakPointsFileName,
                                            const char* caller );
      
      /**
       *    Help method for mergeStreetSegments().
       *    Recursive method.
       */
      void mergeSSI( list<uint32>& mergedNodes,
                     set<uint32>& processedSSI,
                     OldStreetSegmentItem* ssi,
                     byte nodeNbr,
                     bool forward );

      OldItem* addItemToCountryOverview(OldItem* item, OldGenericMap* otherMap, 
                                    uint32 zoomLevel,
                                    GfxData* mapGfx = NULL );

      
      /**
       *    Reset the item-arrays etc. in this map.
       */
      void reset();

      /**
       *    Creates a NEW GfxData based on the GfxData of the specified
       *    item. The polygons are filtered according to maxDist, and
       *    only polygons that should be included in the 
       *    OldCountryOverviewMap are added (OldStreetItems may have 
       *    polygons with too low roadclass; these will not be added, 
       *    however if it's optional to have those polygon indices
       *    added to an output vector).
       *
       *    @param   origItem The item who's GfxData should be used
       *                      when creating the new GfxData.
       *    @param   maxDist  The maxDist parameter, used when filtering
       *                      polygons. If maxDist is set to MAX_UINT32,
       *                      then no filtering is performed.
       *    @param   skippedPolygons   Optional parameter. Output
       *                               parameter. Vector that will be 
       *                               filled with the indices of the
       *                               polygons that are not included
       *                               in the returned GfxData..
       *    @return  A new GfxData. Note that the caller of this method
       *             must handle the destruction of this GfxData.
       */
      GfxDataFull* createNewGfxData(OldItem* origItem, 
                                uint32 maxDist = MAX_UINT32,
                                vector<uint16>* skippedPolygons = NULL);

      /**
       *    Uses a lookuptable so it's not O(n) anymore.
       *    Get the ID on this overview map when the orignal map- and 
       *    item-ID are known. 
       *
       *    Note that buildIDsByOriginalIDsTable() must be called
       *    in order to create the lookuptable. This is automatically
       *    done in the internalLoad method.
       *    
       *    @param origMapID  The ID of the map where the item is located
       *                      "for real".
       *    @param origItemID The ID of the item on the map where it is 
       *                      located "for real".
       *    @return The ID of the item in this map.
       */
      inline uint32 getOverviewIDFast(uint32 origMapID, uint32 origItemID);
      
      /**
       *    Build the table containing OriginalIDs keyed by the current
       *    IDs.
       */
      void buildIDsByOriginalIDsTable();
      

      /**
       *    The maximum zoomlevel for the street items.
       */
      static const uint32 MAX_STREET_ZOOMLEVEL;
       
      /**
       *    The maximum zoomlevel for the streetsegment items.
       */
      static const uint32 MAX_STREETSEGMENT_ZOOMLEVEL;
      

      struct waterNotice_t {
         list<OldWaterItem*> waterItems;
         vector<originalIDs_t> origIDs;
         float64 area;
         char* name;
      };
      struct forestNotice_t {
         list<OldForestItem*> forestItems;
         vector<originalIDs_t> origIDs;
         float64 area;
         char* name;
      };

      /**
       *    Checks whether the specified item should be included
       *    in the OldCountryOverviewMap.
       *    @param   origItem The item to check.
       *    @param   origMap  The map that the item comes from.
       *    @param   zoomLevel   [IN/OUT] The zoomlevel for the item.
       *                         May be modified by the method.
       *    @return  True if the item should be included,
       *             false otherwise.
       */
      bool toIncludeItem(OldItem* origItem, OldGenericMap* origMap,
                         uint32& zoomLevel );
      
      
      /**
       *    Method used by toIncludeItem to determine if an item is a
       *    mountain pass to add.
       *    $return Returns false if the item is not a mountain pass, or
       *            there is some other reason for not adding it.
       */
      bool isMountainPassToAdd( OldItem* item );

      /**
       *    Checks whether the specified polygon of a item 
       *    should be included in the OldCountryOverviewMap.
       *    NB! Note that this method requires that toIncludeItem()
       *    has returned true for this item.
       *
       *    @param   item  The item to check.
       *    @param   poly  The polygon of the item. 
       *    @return  True if the polygon should be included,
       *             false otherwise.
       */
      bool toIncludePolygon(OldItem* item, uint16 poly);                         

      
      /**
       *    A temporary array with the water-items that might be included
       *    in this overview-map.
       */
      vector<waterNotice_t> m_waterCandidates;

      struct LessWaterAreaOrder:
         public binary_function<waterNotice_t, waterNotice_t, bool> {
            bool operator()(waterNotice_t x, waterNotice_t y) {
               return (x.area > y.area);
            }
      };

      vector<forestNotice_t> m_forestCandidates;

      struct LessForestAreaOrder:
         public binary_function<forestNotice_t, forestNotice_t, bool> {
            bool operator()(forestNotice_t x, forestNotice_t y) {
               return (x.area > y.area);
            }
      };

      /**
       *   Reverse lookup table for fast getting the underview id given
       *   the overview id.
       */
      map<struct originalIDs_t, uint32> m_idsByOriginalIDs;

      /**
       *    Help method when updating group ids for all items in the map.
       *    Update groups for one item so they refer to item ids in this
       *    map istead of item ids in the original maps. If one group
       *    does not exist in this map it is removed from the item.
       *    @param   item  The item for which group ids should be updated.
       */
      void updateGroupIDs(OldItem* item);
      
      /**
       *    Help method when updating names (string indexes) for
       *    a new overview item. All names are removed from the 
       *    newItem, and then added from otherItem to update the
       *    string table in this country overview map.
       *    @param   newItem   The overview item to update names for.
       *    @param   otherItem The item from which to copy names.
       *    @param   otherMap  The map where otherItem is located.
       */
      void updateNames(OldItem* newItem,
                       OldItem* otherItem, OldGenericMap* otherMap);

      /**
       *    Help method when adding one item to this co-map. Removes
       *    all unwanted groups. Keeps bua and municipal groups. Note 
       *    that groupids in the new item refer to ids in the map from 
       *    which the new item originates.
       *    @param   newItem  The new overview item from which unwanted
       *                      groups should be removed.
       *    @param   otherMap The map from which the new item originates.
       */
      void removeUnwantedGroups(OldItem* newItem, OldGenericMap* otherMap);
      
      /**
       *    Help method when adding items to this map. Adds bua and
       *    municipal groups to newItem if it is a point of interest item.
       *    The groups are copied from the ssi this poi (other item) 
       *    belongs to in the other map. Note that the group ids will
       *    be ids in the otherMap (underview IDs possible to translate to
       *    overview IDs).
       *
       *    @param   newItem   The overview item (poi) to which bua and 
       *                       municipal groups should be added.
       *    @param   otherItem The item corresponding to the new item.
       *    @param   otherMap  The map where otherItem is located and
       *                       from which newItem originates.
       */
      void addGroupsToPoi(OldItem* newItem,
                          OldItem* otherItem, OldGenericMap* otherMap);

      /**
       *    Update the attribute values of an overviewItem to the 
       *    values of another item.
       *    Typically used when applying changes to country overview
       *    production maps from underview maps.
       *    This method updates attributes that requires lookup of
       *    true ids in overview/underview, e.g. groups (from underview 
       *    to overview group id). It also updates names and gfxData.
       *    
       *    @param overviewItem The overviewItem to update attributes for.
       *    @param otherItem    The item from which to get attribute values.
       *    @param otherMap     The map where otherItem is located.
       */
      void updateAttributes(OldItem* overviewItem, 
                            OldItem* otherItem, OldGenericMap* otherMap);

      /**
       *    Check if a new item (typically added when applying changes
       *    to production maps) needs to have a unique name. If so, a
       *    unique name is added to the item and any items that had the 
       *    same (official) name.
       *    Group ids for these item(s) must be correct before calling 
       *    this method.
       *    
       *    @param checkItem  The item which (perhaps) should be given 
       *                      a unique name.
       */
      void updateUniqueNames(OldItem* checkItem);
      
      /**
       *    Help method in makeUniqueNames. Used for deciding if one name
       *    of an item in this co-map is among the names in a vector of 
       *    check-names (stringIndex and laguage considered).
       */
      bool hasWantedName(OldItem* item, byte nameNbr,
                         const vector<uint32>& checkStringIdxs,
                         const vector<LangTypes::language_t>& checkLanguages);

};

// ========================================================================
//                                      Implementation of inlined methods =


inline uint32 
GMSCountryOverviewMap::getOverviewIDFast(uint32 origMapID, 
                                         uint32 origItemID)
{
   originalIDs_t origID;
   origID.origMapID = origMapID;
   origID.origItemID = origItemID;
   
   map<struct originalIDs_t, uint32>::iterator p = 
      m_idsByOriginalIDs.find( origID );
   if ( p != m_idsByOriginalIDs.end() ) {
      return p->second;
   } else {
      return MAX_UINT32;
   }
}

#endif

