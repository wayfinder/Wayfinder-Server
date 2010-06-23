/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSMIDMIFHANDLER_H
#define GMSMIDMIFHANDLER_H

#include "GMSMap.h"
#include "MC2BoundingBox.h"

/**
 *    Class that handles mid/mif, either printing info from a GMSMap to midmif
 *    or adding items to a GMSMap from midmif.
 */
class GMSMidMifHandler {
   public:

      GMSMidMifHandler(GMSMap* theMap);

      virtual ~GMSMidMifHandler();
      
      /**
        *   Create and add items from a mid and mif file.
        *   For each item:
        *   1. Create the gfxdata for a midmif item from mif file.
        *   2. Create a new GMSXItem of correct type.
        *   3. The item creates itself from the mid file.
        *   4. Check if the item should be adde to the current map.
        *   5. Add the item to the map.
        *   6. Add names to the item.
        *   For all items:
        *   7. Misc pre-processing depending on which type of items
        *      that was created.
        *
        *   @param fileName   The file name, without extension.
        *   @param nbrItemsInFile   Number of items that is read from the
        *                           midmif file, out param.
        *   @param addToExistingMap Set this param to true (default) if the 
        *                           items should be added to already existing
        *                           map(s). It is checked that the item fits
        *                           the current map or not.
        *   @param setLocations     If set to true, locations will be set for
        *                           any added item (and all other items in
        *                           the map without location so far). Default
        *                           is false.
        *   @param setTurnDesc      If set to true and adding street segments,
        *                           turn descriptions will be generated for
        *                           the added ssi's. Default is false.
        *   @param updateNodeLevelsInMap  If set to true and adding street
        *                           segments, node levels will be updated in
        *                           the map (all nodes). Needed when the midmif
        *                           supplier has level on segments (not in 
        *                           nodes). Deafult is false.
        *   @param useCoordToFindCorrectMap Set this param (default false)
        *                           if a coordinate is given at the end of 
        *                           the mid row to decide the correct map.
        *                           The ccordinate should be within 2 meters
        *                           from any ssi in the correct map.
        *   @param addAllMidMifItems Set this param (default false)
        *                           if you want ALL midmif items to be added
        *                           to the map not checking settlement ids,
        *                           geometry or map ssi coord.
        *   @return  Number of items created and added to this map,
        *            -1 is returned upon error.
        */
      int createItemsFromMidMif(const char* fileName, 
                                uint32 &nbrItemsInFile,
                                bool addToExistingMap = true,
                                bool setLocations = false,
                                bool setTurnDesc = false,
                                bool updateNodeLevelsInMap = false,
                                bool useCoordToFindCorrectMap = false,
                                bool addAllMidMifItems = false,
                                bool tryToBuildMapGfxFromMunicipals = false );

      /**
       *    This method is used when reading a mid file and creating attributes 
       *    in GMSXXItem. There is a predefined set of attributes for each 
       *    item type which are mandatory to be present in the mid file. This
       *    method checks if there are any extra item attributes besides 
       *    the mandatory ones that also should be read from the mid file 
       *    (e.g. roundaboutish for GMSStreetSegmentItems).
       *    Any mapSsiCoordinates at the end of the mid row are not counted
       *    as extra item attributes since they are read and used by the main
       *    createItemsFromMidMif-method.
       *
       *    @param   midFile        The mid file to read.
       *    @param   mapSsiCoords   Set to true if there is a set of
       *                            mapSsiCoordinates at the end of the mid
       *                            row.
       *    @return  The number of extra item attributes for the
       *             current row in the mid file.
       */
      static uint32 getNumberExtraAttributes(ifstream &midFile,
                                             bool mapSsiCoords);

      /**
       *    The maximum length of a mid line.
       */
      static const uint32 maxMidLineLength;
      

      /**
        *   @name Clean up functions for removing duped midmif items
        *   @doc  If adding items from midmif to mcm maps results in
        *         same midmif item added to more than one mcm map. Then
        *         these area the methods to utilize to get rid of 
        *         the items from the incorrect maps. Call them
        *         from GenerateMapServer removeDupItemsFromMidMif.
        */
      //@{
         typedef map<uint32, uint64> mapDistances_t;
         /**
          *    For one mcm map (class member):
          *    reads from the midmif ref which midmif items were added
          *    to this map, and fills the distToSSIinMap with the
          *    distance to the closest ssi for all of the miditems.
          *    @return Nbr of distances filled in the multimap for
          *            the mcm map.
          */
         uint32 fillDistancesForItemsFromMidMif(
               ItemTypes::itemType itemType,
               set<uint32> midmifIDs, 
               map<uint32, mapDistances_t> &distToSSIinMap);
         /**
          *    For one mcm map (class member):
          *    Check the distances in distToSSIinMap for the midmif items
          *    that were added to this map (loading midmifref). The midmif
          *    items that has a ssi closer in another map,
          *    will be removed from this map.
          *    @return Nbr of items removed from the map
          */
         uint32 removeDupItemsFromMidMif(
               ItemTypes::itemType itemType,
               set<uint32> midmifIDs, 
               map<uint32, mapDistances_t> distToSSIinMap);
      //@}


   private:

      /**
       *    The GMSMap to process.
       */
      GMSMap* m_map;

      /**
       *    The map gfx data.
       */
      const GfxData* m_mapGfxData;

      /**
       *    The map bounding box.
       */
      MC2BoundingBox m_mapBBox;

      /**
       *    Which item type is to be created from midmif.
       */
      ItemTypes::itemType m_itemType;
     
      /**
       *    Which mid line number is beeing processed.
       */
      uint32 m_midLineNbr;

      /**
       *    Used if country gfx is needed to decide in which map a midmif
       *    item should be added (if the item is located on the map border).
       */
      const GfxData* m_countryGfx;
     
      ///  A map of uint32:s.
      typedef map<uint32, uint32> u32map;
      
      /**
       *    Map with mid id - mc2 id for items added to the map from
       *    the current mid mif file.
       */
      u32map m_addedItems;
      
      ///   Map with mid id and mc2 id.
      typedef map<uint32, uint32> mc2ByMid_t;

      ///   Map with item type as key and combined mid id and mc2 id as value.
      typedef map<uint32, mc2ByMid_t> midIdsRef_t;
     
      /**
       *    May contain midId -> mc2 id for all items added to the map 
       *    from mid mif.
       */
      midIdsRef_t m_midIdsRef;

      /**
       *    Tells if the midId-mc2Id reference file has been read or not.
       */
      bool m_midIdRefLoaded;

      /**
       *    The file name of the file with references between midmif features
       *    and mc2 ids for the current map.
       */
      MC2String m_midmifRefFilename;
     

      /**
        *   Extract and set restrictions from an ESRI turntable.
        *   When creating items from mid/mif (shape), restrictions can 
        *   be given in an ESRI turntable. It may hold information about all 
        *   possible turns (or just some turns) in the midmif-map and state 
        *   whether each turn is restricted in any way.
        *
        *   A turn table is tab-separated and has a header that gives the
        *   names of the columns. I must hold at least the coulmns:
        *   KEY, ARC1_, ARC2_ and IMPEDANCE which corresponds to
        *   turnId, fromId, toId, restrictionType.
        *   The defined restriction types are: -1 (0x80) -2 (bifurcation).
        *   If fromId is -1, all connections to toId is given 0x80.
        *
        *   @param turnTable  The table (textfile) to read the 
        *                     restrictions from
        *   @return  True if all restrictions were extracted and set.
        */
      bool extractRestrictionsFromTurnTable( ifstream& turnTable );

      /**
       *    Store a file on disk containing references between midmif
       *    features and mc2 ids for a certain map.
       *    Will append information to the existing file (since this 
       *    needs to be done for each item type of the map).
       *    @param   The filename. Typically something like 
       *             000000000.mcm.midmif_ref.
       *    @return  True if succesful, false otherwise.
       */
      bool writeMidMifReferences( const MC2String& fileName );
      
      /**
       *    Read from disk a file containing references between 
       *    midmif features and mc2 ids for a certain map.
       *    @param   The filename. Typically something like 
       *             000000000.mcm.midmif_ref.
       *    @return  True if succesful, false otherwise.
       */
      bool readMidMifReferences( const MC2String& fileName,
                                 bool printOut = false );
      
      /**
       *    Help method for createItemsFromMidMif.
       *    Check if a midmif item should be added in this map or not.
       *    The rules for deciding if the map is correct:
       *    1. Via a provided map ssi coordinate (the item fits the current
       *       map if there is a ssi within 2 meters from the coordinate).
       *    2. Via provided left and right settlement ids from map supplier
       *       mid relationships.
       *    3. Via the geographical location itemGfx-mapGfx.
       *
       *    @param   itemGfx  The gfxData of the midmif item
       *    @param   mapSsiCoords True if map ssi coordinates are provided
       *                      and should be used to decide, false if not.
       *    @param   itemLat  The map ssi coord latitude.
       *    @param   itemLon  The map ssi coord longitude.
       *    @param   leftSettlementId The mid id of the item's left settlement
       *    @param   rightSettlementId The midid of the item's right settlement
       *    @param   settlementOrder   The order of the left and right
       *                      settlements.
       *    
       *    @return  True if the midmif item should be added to this map,
       *             false if this was not the correct map.
       */
      bool correctMapForItem( GfxData* itemGfx,
                              bool mapSsiCoords,
                              int32 itemLat, int32 itemLon,
                              uint32 leftSettlementId,
                              uint32 rightSettlementId,
                              uint32 settlementOrder,
                              uint32& itemRegionId,
                              bool& itemRegionIsGroupItem);

      /**
       *    Search the m_midIdsRef for an item, given item type and mid id.
       *    If the item exists, return the mc2 id.
       *    @param   type  The type of the mc2 item to find.
       *    @param   midId The midId of the item to find.
       *    @param   itemTypeExist  Outparam that is set to false if there are
       *                   no items at all of type type in the references.
       *    @return  The mc2 id of the item, MAX_UINT32 if no item is found.
       */
      uint32 getMc2Id( uint32 type, uint32 midId, bool& itemTypeExist );

      /**
       *    Get the convex hull of the map
       *    Code pretty much copied from GMSMap::setMapGfxDataConvexHull
       */
      GMSGfxData* getConvexHullForMapGfxData( );

      /**
       *    Build a gfxdata combining the gfx datas of the municipals
       *    of the map. Merge the polygons of the gfxdata to only get 
       *    the outer line of the municipals.
       *    @param   skipNoNameMunicipals If you want to skip municipals
       *                with no name, set this to true.
       */
      GMSGfxData* buildMapGfxFromMunicipals( 
               bool skipNoNameMunicipals = false );

};

#endif

