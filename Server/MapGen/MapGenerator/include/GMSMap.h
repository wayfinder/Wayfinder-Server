/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSMAP_H
#define GMSMAP_H

#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include "OldMap.h"
#include "DataBuffer.h"
#include "GMSItemNames.h"
#include "ItemTypes.h"
#include "MC2Coordinate.h"
#include "ZipSideFile.h"

#include "OldExternalConnections.h"
#include <set>
#include <map>
#include <algorithm>

class OldBuiltUpAreaItem;
class ObjVector;


#ifdef _WIN32
#include <fstream.h>
#else
#include <fstream>
#endif

#include <cstdlib>

#ifdef __SVR4
   #include <stdarg.h>
   //#include <sys/varargs.h>
   // NB! The man-page for va_start on solaris is not correct.
   //     is says that the sys/vararg.h should be included, but
   //     it is the stdarg.h that works...
#endif


// Forward declarations
class GMSStreetItem;
class OldRouteableItem;
class OldZipCodeItem;
class OldMunicipalItem;

class IDPair_t;
class GDFRef;

/**
  *   Class to generate a mcm file from different raw mapdata.
  *   
  */
class GMSMap : public OldMap {

   public:
      friend class GenerateMapServer;

      ///   Create GMSMap from path and id.
      static OldGenericMap* createMap(uint32 id, const char* path, 
                                   bool loadMap = true);
      ///   Create GMSMap from mcm-name.
      static OldGenericMap* createMap(const char* mcmName);
      

   public:

      /**
        *   Delete this Map.
        */
      virtual ~GMSMap();

      /**
        *   Sets the name of the map, after deleting the old one.
        *   @param   n The new name.
        */
      void setName(char *name);

      /**
        *   Get the number of items on the map.
        *   @return  The total number of items on this map.
        */
      uint32 getNbrItems() {
         uint32 sum = 0;
         for (int i=0; i<NUMBER_GFX_ZOOMLEVELS; i++) {
            sum += getNbrItemsWithZoom(i);
         }
         return (sum);
      }

      /**
        *   Concatenate the municipals that have the same name into
        *   one municipals with many names. Also make sure that the
        *   municipals have the first IDs in the itemZooms-array.
        *
        *   @return  True if the compactation is performed ok, false
        *            otherwise.
        */
      bool compactMunicipals();

      /**
        *   Concatenate the built-up areas that have the same name into
        *   one built-up area with many names. Also make sure that the
        *   buas have the first IDs in the itemZooms-array.
        *
        *   @return  True if the compactation is performed ok, false
        *            otherwise.
        */
      bool compactBuiltUpAreas();

      /**
        *   Changes the location of all items belonging to a 
        *   municipal or builtUpArea to a new location. Used when the 
        *   locationItem is given a new ID, e.g. in compactMunicipals
        *   or mergeSameNameBuas.
        *   NB. Works for municipal location and builtUparea location,
        *   not for any other location.
        *
        *   @param   oldLocation    The location to change.
        *   @param   newLocation    The new location of items.
        *   @param   locationType   What part of the location to change, 
        *                           municipal or bultiuparea.
        *   @param   changeLocationItem States if the location of the
        *                           locationitem should be changed. The
        *                           locationitem = the item that IS the
        *                           location, i.e. the bua with the same
        *                           itemID as the bualocation to change.
        *                           Deafult is true.
        *   
        *   @return  The number of items that had their location changed.
        */
      uint32 changeLocation(uint32 oldLocation, uint32 newLocation,
                             ItemTypes::itemType locationType, 
                             bool changeLocationItem = true);

      /**
        *   Calculates and sets the gfxData for the entire map to be the
        *   convex hull of all street segments and points of interest items
        *   in the map.
        * 
        *   @return  True if the map gfx data was update to the convex hull,
        *            false e.g. if convex hull could not be created.
        */
      bool setMapGfxDataConvexHull();
      
      /** 
        *   Takes the streetSegments and joins them into streets.
        */
      void generateStreetsFromStreetSegments();
      
      OldZipCodeItem* findZipCodeItem(const char* zipCode);

      /**
       *    Adds street segments to zip codes using existing zip code
       *    gfxdata in case the zip code doesn't contain any 
       *    street segments. 
       *
       *    In case zipcodes are found that already contain 
       *    streetsegments (i.e. zipcodes have already been added from
       *    attribute data) then all zipcodes without streetsegments
       *    are removed.
       *
       *    @return  If the method added any streetsegments to the
       *             zipcodes based on geometry.
       */
      bool addSSIToZipCodesUsingGfx();

      uint32 createZipsFromGMSStreetSegmentItems();


      /**
        *   Sets the turndescriptions of the connections.
        *   @param   itemID   If given only turndirections to/from this ssi
        *                     are set, otherwise all turndirections in 
        *                     the whole map.
        *   @return  True if the turndescriptions are set properly, false
        *            otherwise.
        */
      bool initTurnDescriptions(uint32 itemID = MAX_UINT32);

      
      /**
       *    Uses values from turn descriptions to set the direction category
       *    values.
       */
      uint32 setUndefiendLaneConnDirCat();

      /**
       * @return Returns an appropriate lane direction from a turn direction.
       */
      GMSLane::laneDirection_t 
         getLaneDirCatFromTurnDir(ItemTypes::turndirection_t turnDirection,
                                  uint32 fromNodeID, uint32 toNodeID);

      /**
       * @return Returns true if the exit between the nodes fromNodeID and
       *         toNodeID is on the right side of the road. 
       */
      bool isRightExit(uint32 fromNodeID, uint32 toNodeID);

      

      /**
        *   Changes zoom level of the item. The old place in 
        *   m_itemsZoom is set to NULL (not NullObject). Note that
        *   this method assumes that no OldConnections are created
        *   yet. The effect is that the id of the item will be changed.
        *
        *   @param   item  The item to change zoomlevel of.
        *   @param   zoomLevel   The new zoomlevel.
        *   @return  The new ID of the item.
        */
      uint32 changeZoomLevel(OldItem* item, uint32 zoomLevel);

      /**
        *   Sets the zoomLevel to have 0 objects. Note that it does not 
        *   delete any objects in the zoomLevel if any exists, nor sets 
        *   them to NULL. This method is assumed to be used after using
        *   changeZoomLevel().
        *   @param   zoomLevel   The zoomlevel to remove object in.
        */
      void removeObjectsInZoomLevel(uint32 zoomLevel);

      /**
        *   Checks if the current citypart lies within a bua.
        */
      int setCityPartLocation();
      
      /**
        *   Set the logical location of all the items in this map.
        *   @param   onlyUpdateNonValid   If set to true, the location
        *                                 will only be updated for those
        *                                 items that does not already
        *                                 have a valid location.
        *                                 Default set to false.
        */
      void setAllItemLocation(bool onlyUpdateNonValid = false);
        
      /**
       *    Updates the location for all items that are
       *    inside the specified builtup area.
       *    @param   bua   The builtup area.
       *    @return  True if the operation was succesful, false otherwise.
       */
      bool updateLocation(OldBuiltUpAreaItem* bua);
            
      /**
        *   Updates the location of the streets based on the location
        *   of the streetsegments that it is built up of.
        *   Note that the location of the streetsegments must be
        *   set before calling this method.
        *
        *   Taking all groups of a specific type from the SSIs. Reads
        *   the hight bit and passes it to the street if present.
        *
        *   @param   type  The type of location that should be set, 
        *                  ie. ItemTypes::municipalItem or
        *                  ItemTypes::builtupAreaItem.
        *   @return  True if the operation was succesful, false otherwise
        */
      bool updateStreetLocationAll(ItemTypes::itemType type);

      /**
        *   Run through zoom level 0 and add the municipals to the
        *   municipalVector.
        */ 
      void updateMunicipalArray();
      
      /**
       *    Removes all location items of the specified type (zip, bua or mun)
       *    that don't have any other item having them as location. If the item
       *    is marked as item-not-in-search-index, it is not regarded a 
       *    location item and is hence not removed even if no items have it
       *    as group (as they shouldn't).
       *
       *    @param   itemType The location item type. Municipal or bua
       *                      supported.
       *    @return True if indata was OK, false otherwise.
       */
      bool removeEmptyLocationItems( ItemTypes::itemType itemType );


      /**
       *    Removes street names, which are an exact copy of another street
       *    name of the same street segment item. 
       *
       *    These may be present because the maps had two version of a name
       *    only different in case. We change the case of all names and
       *    this may result in two exacly alike names on the same item.
       */
      bool removeDuplicatedStreetNames();

      /**
       *    Removes all groups that are of type item type from all items.
       *
       *    @param itemType When a group of an item points at this itemType
       *                    it will be removed from the item.
       *
       *    @return True if successful, otherwise false.
       */
      bool removeGroups(ItemTypes::itemType itemType);

      /**
        *   Set the mapID ot this map to a new value. Please observe that
        *   this also means that the name of this map on disc will change,
        *   and that possible external connections to this map must be
        *   updated!
        *   @param   mapID The new ID of this map.
        */
      void setMapID(uint32 newMapID) {
         m_mapID = newMapID;
      }
      
      /**
        *   Add all items to the hashTable.
        *   Overrides protected declaration in OldGenericMap.
        */
      void buildHashTable();

      /**
        *   Add one item to this map.
        *   Overrides protected declaration in OldGenericMap.
        */
      uint32 addItem(OldItem* item, uint32 zoomLevel) {
         return (OldGenericMap::addItem(item, zoomLevel));
      }

      /**
        *   Get the items with a given string index.
        *   Overrides protected declaration in OldMap
        */
      bool getAllItemsWithStringIndex(uint32 stringIndex, Vector* result) {
         return (OldMap::getAllItemsWithStringIndex(stringIndex, result));
      }

     /**
       *    Overrides the protect-declaired method in OldGenericMap.
       *    @param path The new path to the map.
       */
      void setFilename(const char* path) {
         OldGenericMap::setFilename(path);
      }
     
      /**
       *    Get the filename.
       *    @return The filename.
       */
      const char* getFilename() const {
         return m_filename;
      }
      
      /**
        *   Add items (street segments, lakes, forests and so on) from
        *   another map to this map.
        *   The items to be added are considered to belong to this map 
        *   if they fit inside the maps gfxData.
        *   For street segments and ferries, they belong if they are within 
        *   10 km from this map's gfxdata bbox, and there is a node
        *   in this map that the ssi/ferry from the other map can connect to.
        *   @param   otherMap       The map with the extra items.
        *   @param   addedIDs       [In/Out] Set containing all the
        *                           items that have been added so far 
        *                           to the map.
        *   @param   addMunicipals  If set to false, no municipals
        *                           will be added. Default value is false.
        */
      bool addExtraItemsFromMap(OldGenericMap* otherMap,
                                set<IDPair_t>& addedIDs,
                                bool addMunicipals = false);

      /**
        *   Add a connection between two nodes.
        *
        *   @param   curNode        One of the nodes.
        *   @param   otherNode      The other node.
        *   @param   distOtherNode  Distance between them.
        *   @param   vtByNode       Optional parameter.
        *                           STL map containing allowed vehicle
        *                           types by node ID. If specified, 
        *                           used to set the correct 
        *                           restrictions.
        */
      void addConnection(OldNode* curNode, 
                         OldNode* otherNode, 
                         uint32 distOtherNode,
                         map<uint32, uint32>* vtByNode = NULL);


      /**
       *    Get the items names of this map.
       *    @warning Make sure that you know what you are doing when you 
       *             call this method!! 
       *    @return  The itemsnames of this map.
       */
      GMSItemNames* getItemNames() {
         return dynamic_cast<GMSItemNames*>(m_itemNames);
      }

      
      /**
        *   Structure needed when adding external connections 
        *   for the turndescriptions to be set alright.
        */
      struct extdata_t {
            uint32 mapID;
            uint32 nodeID;
            int32 lat;
            int32 lon;
            float64 angle;
            float64 endNodeAngle;
            bool ramp;
            bool roundabout;
            bool multiDig;
            byte roadClass;
            ItemTypes::entryrestriction_t entryRestrictions;
            int8 level;
            ItemTypes::itemType type;
      };
      
      virtual void createAllocators();

     /**
       *    Initializes the non item allocators.
       *    @param   nbrGfxDatas                nbrGfxDatas.
       *    @param   nbrGfxDataSingleSmallPoly  nbrGfxDataSingleSmallPoly.
       *    @param   nbrGfxDataSingleLine       nbrGfxDataSingleLine.
       *    @param   nbrGfxDataSinglePoint      nbrGfxDataSinglePoint.
       *    @param   nbrGfxDataMultiplePoints   nbrGfxDataMultiplePoints.
       *    @param   nbrNodes                   nbrNodes.
       *    @param   nbrConnections             nbrConnections.
       */
      virtual void initNonItemAllocators( uint32 nbrGfxDatas, 
                                          uint32 nbrGfxDataSingleSmallPoly,
                                          uint32 nbrGfxDataSingleLine, 
                                          uint32 nbrGfxDataSinglePoint,
                                          uint32 nbrGfxDataMultiplePoints, 
                                          uint32 nbrNodes, 
                                          uint32 nbrConnections );

      /**
        *   Lopps through all nodes to set the level in the nodes to
        *   the same value as of their connected neighbours.
        *   @return  Number of nodes who had their level updated.
        */
      uint32 updateNodeLevels();
      
      /**
       *    Make sure that all municipals in the map have a name and
       *    a valid name. If there is no name or the name is the $$$-name
       *    the name is updated to be the country name in the native
       *    language(s).
       *    @return The number of municipals that have their name changed.
       */
      uint32 updateMunicipalNames();
       
      /**
       *    Make sure that all buas in the map have a name. Any no-name
       *    buas will be given the name(s) of the municipal it is located
       *    in (first municipal).
       *    @return The number of noname buas that had a new name.
       */
      uint32 setNamesOnNoNameBuas();
       
      /**
       *    Loop all built-up areas. All no-name built-up area that are
       *    located in a no-name municipal are removed from the map.
       *    Also no-name buas that are not located in any municipal
       *    are removed.
       *    Will speed up mergeSameNameAdminAreas. The merge-method was
       *    really slow in e.g. Nigeria. Also needed in countries where 
       *    we want to avoid that
       *    no-name buas inherit their name from the no-name municipal
       *    that inherited from the name of the country.
       *    @return The number of noname buas that were removed.
       */
      uint32 removeNoNameBuasInNoNameMunicipals();

      /**
       *    Loop all city parts and remove all no-name city parts 
       *    the map.
       *    City parts comes e.g. from TA order9 features
       *    @return The number of noname city parts that were removed.
       */
      uint32 removeNoNameCityParts();


      /**
       *    Merge admin areas (bua or municipal) area items when 
       *    the following is true:
       *    (1) The items have the same name.
       *    (2) The items are close to each other, either:
       *       (1) The item have gfxdata, and the gfxdatas are close.
       *       (2) The item doesn't have gfxdata but the convex hull
       *           made up by all other items having it as location
       *           is close.
       *           
       *    The gfxDatas of all mergeable admin areas are added 
       *    to one of the admin areas, and the rest of are removed
       *    from the map.
       *    @param itemType      The itemtype of the admin area to merge.
       *                         Can either be bua or municipal.
       *    @param maxMergeDist  The maximum distance (meters) between two
       *                         built-up areas for them to be considered
       *                         close enough to be merged.
       *    @param updateLocation   If set, the location of all items that
       *                         were in a removed bua is updated to the
       *                         new bua. (Default is false = don't update)
       *    @param simplifyMergedGfxs  If set, any gfxData that has been
       *                         merged will be simplifyed to ensure that 
       *                         it is not divided by any line across it.
       */
      bool mergeSameNameAdminAreas( ItemTypes::itemType itemType,
                                    uint32 maxMergeDist, 
                                    bool updateLocation = false,
                                    bool simplifyMergedGfxs = false );
      
      /**
       *    Adds extended versions of abbreviated names as synonym names.
       * 
       *    @param type The type of items to replace abbreviations for.
       *   
       *    @return Returns false if anything went wrong, otherwise true.
       */
      bool extendAbbreviations( ItemTypes::itemType type );
      
      /**
       *    Does everything needed in order to add a node to the
       *    boundary. This includes adding a virtual 0-length segment
       *    and adding that virtual segment to the boundrysegments vector.
       *    @param   nodeID   The node id to add to the boundary.
       *    @return  The id of the added virtual segment if sucessful, 
       *             otherwise MAX_UINT32.
       */
      uint32 addToBoundary( uint32 nodeID );

      /**
       *    Does everything needed in order to add a node to the boundary,
       *    except for creating the connection to the node. This includes
       *    adding a virtual 0-length segment and adding that virtual 
       *    segment to the boundrysegments vector.
       *    @param   nodeID   The node id to add to the boundary.
       *    @return  The id of the node not close to boundry of the added
       *             virtual segment if sucessful, 
       *             otherwise MAX_UINT32.
       */
      uint32 addToBoundaryNoConn( uint32 nodeID );

      /**
       *    Remove connections that connect two nodes being on different
       *    z-level. Is necessary for TeleAtlas maps.
       *    @return  The number of connections removed.
       */
      uint32 removeConnsWithDiffLevel();

      /**
       *    Go through all bua:s and update the names so that they 
       *    have the same name as their municipal in case they already
       *    have a common name. TeleAtlas usually stores the names of the
       *    large municipals in several languages whereas the bua name
       *    is just in the native language (i.e. in case the bua and
       *    municipal have the same name).
       *    
       *    @return  The number of names added to the builtupareas.
       */
      uint32 updateForeignBuaNames();

      /**
       *    Will update the speedlimits for certain (hardcoded) cities
       *    that have severe traffic congestion.
       *    Given a city with traffic congestion, the streets at the
       *    citycentre will get the lowest speedlimit (a percentage of
       *    the ordinary speedlimit). The speedlimits will lineary increase
       *    up to 100% of normal speed when reaching the max radius of
       *    the congestion area.
       *
       *    @return  The number of updated speedlimits.
       *    
       */
      uint32 updateCongestionSpeedLimits();
      
      /**
       *    Method makes sure that metropolitan bua:s don't have any 
       *    municipal location and that municipals inside metropolitan 
       *    bua:s have bua location. Metropolitan bua is a bua that is
       *    covering several municipals.
       *
       *    @return  The number of bua:s and municipals that had their
       *             location changed.
       */
      uint32 updateMetropolitanBuaMunLoc();      
      
      /**
       *    Searches for all multi connection starting from fromNodeID
       *    and add their to-nodes to the toNodes list.
       *    Linear complexity.
       *    
       *    @param   fromNodeID  The from node ID.
       *    @param   toNodes     [Outparameter] Will be filled with
       *                         all to-nodes of the multi connections
       *                         starting from fromNodeID.
       *    @return  The number of multi connections starting from 
       *             fromNodeID.
       */
      uint32 getMultiConnsFromNode( uint32 fromNodeID, 
                                    list<uint32>& toNodes );
      
      /**
       *    Checks if the connection between the specified node ids
       *    are part of a multi connection.
       *    Linear complexity!
       *    @param   firstNodeID    The first node id.
       *    @param   secondNodeID   The second node id.
       *    @return  True if part of a multi connection.
       */
      bool isPartOfMultiConnection( uint32 firstNodeID,
                                    uint32 secondNodeID );



      /**
       *    Add a multi connection to the expansion table.
       *    Note that the actual connection must be added to the
       *    node as usual, using the OldNode::addConnection method.
       *    @param   firstMultiNodeID  The first node id of the 
       *                               multi connection.
       *    @param   lastMultiNode     The last node id of the 
       *                               multi connection.
       *    @param   expandedNodeIDs   List of nodes covered by the
       *                               multi connection between the
       *                               first and last node. I.e. the
       *                               first and last node should
       *                               not be included in the list.
       */
      void addMultiConnection( uint32 firstMultiNodeID,
                               OldNode* lastMultiNode,
                               const list<uint32>& expandedNodeIDs,
                               uint32 vehicleRestriction,
                               ItemTypes::turndirection_t turnDirection);
      /**
       * Reads gdf ref file from disc and fills in the gdfRef parameter.
       *
       * @param gdfRef Outparameter This one should be empty and is filled
       *                            whith the content of the gdf ref file.
       * @return True if successful, false otherwise.
       */
      bool getGDFRef(GDFRef& gdfRef);


      /**
       *  Adds zip codes from zip POI file.
       *
       *  1. Loads the zip POI file in the sectionedZipsPath dir. For format
       *     of the zip POI file, see fucntion in cpp-file.
       *  2. Removes all existing zip codes from the map
       *  3. Add zip code for each zip POI to the map
       *  4. Set centre coordinates for each created zip code from the coord
       *     the zip poi file.
       *  5. Set the zip codes as group for close street segments with
       *     the setZipForSSIs method.
       *
       *  @param zipSideFile  An object which will have the zip poi data
       *                      loaded into it.
       *  @param sectionedZipsPath Directory where to find the zip POI files.
       */
      bool addZipsFromZipPOIFile( ZipSideFile zipSideFile,
                                  MC2String sectionedZipsPath );

      /**
       *  Adds a zip code as group to an item. If the zip code exists
       *  in the map, this item is added, otherwise a new zip code is 
       *  created.
       *
       *  NB! Be careful not to add zip codes any other way after calling
       *      this method and use it again. The new zip wont be found by
       *      the method.
       *
       *  @param item The new zip will be group to this item. Set to NULL
       *              if no group should be set (only adding zip item).
       *  @param zipCodeName Name used for finding any already existing 
       *                     zip code or set to the new zip code item.
       */
      bool addZipFromName(OldItem* item, MC2String zipCodeName );
      

      /**
       * Takes the coordinate from the admin area center table and sets
       * it as gfx data for the zips having them as center.
       */
      void setZipGfxDataFromAdminCenters();

      /**
       * Add zips close to a SSI as zip group for it.
       */ 
      uint32 setZipForSSIs();
      

      /**
       *   Removes all zip codes with names shorter than minLength.
       *   @param minLength The lowest number of characters reqiered of a 
       *                    zip code name for the zip code to stay in the
       *                    map after calling this method.
       *   @return Number of removed zip codes.
       */
      uint32 removeShorterZipCodes(uint32 minLength);
      

      /**
       *   Removes all names not having the name type ItemTypes::roadNumber.
       *   @returns Number of items names were removed from.
       *
       */
      uint32 removeNonNumericZipCodeNames();

      /**
       * Looks at all items in the map. All items having origGroupItemID
       * as group gets newGroupItemID as group instead.
       *
       * @param origGroupItemID The item ID of the group to change.
       * @param newGroupItemID The item ID of the group to change to.
       * @return Number of groups changed.
       */
      uint32 changeGroupOfItems(uint32 origGroupItemID, 
                                uint32 newGroupItemID);
      

      /**
        *   @name Index areas.
        */
      //@{
      /// Validates the values of the index area order table.
      bool validateIndexAreaOrderTable() const;

      /**
       * Removes groups that are items that shoud not be used in search index
       * or location.
       * 
       * @return Returns the number of non search index groups that was 
       *         removed.
       */ 
      uint32 removeNonSearchIdxGroups();
      
      /**
       * Finish index areas. 
       * Removes the geometry of index areas and
       * adopts hierarchies and naming of index areas to mc2 searching and
       * location presentation.
       * 
       * @return Returns true on success. (something changed in the map)
       */
      bool finishIndexAreas();
      //@}

      /**
      *    Method to compute area feature draw display class for all
      *    item in item.
      */
      void computeAreaFeatureDrawDisplayClasses();

 protected:

      typedef map<MC2String, OldZipCodeItem*> zipByName_t;

      /// Used by the addZipFromName method.
      zipByName_t m_zipByName;

      /**
       *   A vector used when adding items from an extraItemMap to this map.
       *   It will then hold the id's of items that have been added from 
       *   the extra item map to this map, to enable some extra check if 
       *   a extraSSI fits this map or not.
       */
      Vector* m_itemsFromExtraItemMap;


      /**
       *   Adds a virtual item to the map.
       *
       *   @param rItem Routable item to use as a model for the virtual 
       *                 item. I.e. the attributes of this item is used 
       *                 when creating the virtual item.
       *   @param closeNodeVal Determines which one of the nodes of rItem
       *                 to use as GfxData for the new virtual segment.
       *                 The close node will be used.
       *   @return Returns a virtual item created from rItem, or NULL if 
       *           unsuccessfull.
       */
      OldRouteableItem* createVirtualItem(  OldRouteableItem* rItem, 
                                       OldBoundrySegment::closeNode_t
                                       closeNodeVal );
      
      /**
       *  Removes all but one of all streets including exactly the same
       *  SSIs and add names from all streets removed to the remaining one.
       *
       *  @return Returns the number of removed streets, i.e. the number
       *          of streets  merged into some other street.
       */
      uint32 mergeStreetsWithSameSSIs();

      /// Tells if these items have a name with the same string index.
      bool itemsShareNameStrIdx(OldItem* firstItem, OldItem* secondItem);

      /**
       * Add a group to an item from a index post (nav index or locality 
       * index). If there are no group corresponding to the index post already
       * in the map,it is created.
       *
       * @param item       The item to add the group to.
       * @param indexPost The index post to add as a group.
       *                   first:  index name
       *                   second: locality distinction
       * @param groupIDsByIndex Outparameter 
       *                        Groups corresponding to index already
       *                        created in the map. This one is updated
       *                        when a group is created.
       *         Key: first: Index name, 
       *              second: Index locality distinction.
       *         Value: Group ID in the map corresponding to the Index 
       *                post.
       * @param onlySearchable When this one is true, the group is added
       *                       as a group not to use when showing location.
       *
       * @return Returns true if the successful, otherwise false.
       */ 
      bool addIndexGroupToItem(OldItem* item, 
                               pair<MC2String, MC2String> indexPost,
                               map< pair<MC2String, MC2String>, uint32>&
                               groupIDsByIndex,
                               bool onlySearchable = false );

   private:
      

      /**
        *   Constructor that creates a GMS map from a mcm-file. Since
        *   this method use virtual methods to create most of the 
        *   objects the map is almost a "true" GMSMap with GMSItems, 
        *   GSMGfxData, GMSNodes and GMSConnections. But {\it be aware 
        *   that not {\bf all} the objects in the database are 
        *   GMS-objects -- some are created in the OldMap-class without 
        *   using virtual method!}
        *
        *   @param   mapID    ID of the map that should be loaded.
        *   @param   path     The path to the new map.
        *   @param   mapOK    Outparameter that is set to true if the
        *                     is loaded alright, false otherwise.
        */
      GMSMap(uint32 mapID, const char* path);

      /**
       *   Checks if the BUA and the municipal share a common name, in that
       *   case adds the names in diffrent languages to the BUA from the 
       *   municipal.
       *
       *   @param bua The BUA to try adding names to.
       *   @param mun The municipal to get the names from.
       *
       *   @return Returns number of names added to the BUA.
       */
      uint32 updateBUANamesFromMun(OldBuiltUpAreaItem* bua, OldMunicipalItem* mun);

      /**
        *   @name Create methods.
        *   @memo Virtual methods to create new items.
        *   @doc  Virtual methods to create new items. This methods are 
        *         identical to the ones i OldMap, except that these methods 
        *         creates GMSItem's etc. These excist to make it 
        *         possible to create a GMSMap (with GMSItem's and 
        *         GMSGfxData) from a .mcm-file.
        */
      //@{
         /**
           *   Create a new item.
           *   @param   type        The type of the new item.
           *   @param   dataBuffer  Rawdata for the new item.
           *   @param   itemID      The ID of the new item.
           *   @return  The item that was created inside this method.
           */
         virtual OldItem* createNewItem(DataBuffer* dataBuffer);

         /**
           *   Virtual method that creates a new OldConnection out of a 
           *   databuffer. This method excists only to make it possible
           *   to create subclasses of this class (e.g. GMSConnection).
           *   @param   dataBuffer  The buffer where the data for the 
           *                        OldConnection is loacted.
           *   @return  A new OldConnection.
           */
         virtual OldConnection* createNewConnection(DataBuffer* dataBuffer);
         
         /**
           *   Create a new gfx data.
           *   @param   dataBuffer  Rawdata for the new gfx data..
           *   @return  The gfx data that was created inside this method.
           *
           */
         GMSGfxData* createNewGfxData(DataBuffer* dataBuffer);
      //@}



      /**
       *    A set of uint32:s. 
       */
      typedef set<uint32> u32set;

      /**
       *    A map of uint32:s.
       */
      typedef map<uint32, uint32> u32map;
      
      /**
       *    A multimap of uint32:s.
       */
      typedef multimap<uint32, uint32> u32mmap;

      /**
       *    @name Help methods to generateStreetsFromStreetSegments()
       */
      //@{
         /**
          *    Splits streets so that each street will only consist of 
          *    streetsegments that are close to each other.
          *    The result will end up in a vector of set<uint32>, 
          *    where each set<uint32> represents the streetsegment id:s
          *    of a seperate street.
          *    
          *    @param street           The street to be split.
          *    @param theSplitStreets  Resulting split streets. Each
          *                            set<uint32> represents the ssi id:s
          *                            of a seperate street.
          *    @return The number of resulting split streets. Is the same
          *            as theSplitStreets.size().
          */
         uint32 splitStreets(GMSStreetItem* street, 
                             vector<u32set>& theSplitStreets);
         

         /**
          *    Help method to splitStreets.
          *    
          *    Tries to merge together streets in the theSplitStreets that
          *    have common streetsegments.
          *    In order to merge all streets in the street array, this 
          *    method should be called until no streets are merged.
          *    
          *    @param theSplitStreets     Input/Output param.
          *                               Vector of set<uint32>.
          *                               Each set<uint32> represents
          *                               the ssi id:s of a seperate
          *                               street. 
          *    @return True if any streets were merged, false otherwise.
          */
         bool mergeStreets(vector<u32set>& theSplitStreets); 

         
         /**
          *    Method that should be called after mergeStreets.
          *    Tries to merge streets lying on a line, which is 
          *    often the case in the US.
          *    @param theSplitStreets     Input/Output param.
          *                               Vector of set<uint32>.
          *                               Each set<uint32> represents
          *                               the ssi id:s of a seperate
          *                               street. 
          *    @return True if any streets were merged, false otherwise.
          */
         bool mergeStreetsOnLine( vector<u32set>& theSplitStreets );
         
         /**
          *    Help-method to mergeStreetsOnLine.
          *    Checks if two street segments can be considered to be the
          *    same street based on their graphics.
          *
          *    @param   firstNodeID    First node id.
          *    @param   secondNodeID   Second node id.
          *    @return  True if they are consided to be the same street, 
          *             false otherwise.
          */
         bool sameStreet( uint32 firstNodeID, uint32 secondNodeID );
         
         /**
          *    Sorts the order of streetsegments in the street based on
          *    addressranges.
          *    
          *    @param   street         The street to be sorted.
          *    @param   ssiIDs         Set of ssi id:s that should be 
          *                            sorted.
          *    @param   sortedSSIIDs   Output parameter. Will contain
          *                            the streetsegments sorted based
          *                            on the address ranges.
          */
         void sortStreetOrder(GMSStreetItem* street, 
                              u32set& ssiIDs, 
                              u32mmap& sortedSSIIDs);
         
      //@}
      


      /** 
        *   Used in GMSStreetSegmentItem and GMSStreetItem to 
        *   mark unused items 
        */
      static const uint32 UNUSED = 0xffffffff;

      /**
        *   @name The municipals.
        *   
        *   Member variables used to maintain a list of the municipals 
        *   of this map.
        */
      //@{
         /// The nbr of items currently in the municipalVector.
         uint32 municipalOffset;

         /// Vector with id for the municipals on this map.
         uint32 *municipalVector;
      //@}

      /**
        *   Sets the turn descriptions and crossing kinds of the connections
        *   to the specified node.
        *   
        *   @param   node  Pointer to the node which connections
        *                  should be set.
        *   @param   gfx   Pointer to GfxData describing the polygon
        *                  for the streetsegment where node is located.
        */
      void setTurnDescriptions(OldNode* node, GfxData* gfx);
      

      /**
        *   @name Methods used in addExtraItemsFromMap
        */
      //@{
      /**
       *    When adding items from extra item maps and one extra item
       *    has been added to another map,
       *    this method is called to make sure that there is a
       *    virtual item in this map were to make external connections
       *    to the virtual item that was added from extra item.
       *    @param   otherMap    The external map where the item
       *                         originally comes from.
       *    @param   otherItem   The external item that was added to
       *                         another map.
       */
      bool addVirtualToMatchExtraItem( OldGenericMap* otherMap,
                                       OldItem* otherItem );
      /**
        *   Checks if the item coming from another map (extra item map) 
        *   fits in the map and if that is the case, then it will be 
        *   added to this map.
        *   
        *   @param   externalMap    The external map where the item
        *                           originally comes from.
        *   @param   externalItem   The external item to be added to this
        *                           map (if it fits).
        *   @param   addedIDs       Set containing the ids of the
        *                           items being added to the map.
        *                           Both input and output parameter.
        *   @param   parseOnly   Optional param, default is false. If
        *                        set to true the externalItem will not be
        *                        added, but the method returns true if it fits.
        *   @return  True if the item was found to fit in this map and
        *            that the item was succesfully added. False otherwise.
        */
      bool addExternalItemToMap(OldGenericMap* externalMap,
                                OldItem* externalItem,
                                set<IDPair_t>& addedIDs,
                                bool parseOnly = false);
      
      /**
       *    Help method to addExternalItemToMap. Recursive method.
       */
      void internalAddExtItem( OldItem* externalItem, 
                               OldGenericMap* externalMap, 
                               set<IDPair_t>& addedIDs,
                               bool onlyConnectToFerries );
      
      /**
        *   Checks if the specified item exists in this map, ie. checks
        *   if there exists another item with the same gfxdata.
        *   @param   item  The item to check for.
        *   @return  True if the item already existed, false otherwise.
        */   
      bool itemExists(OldItem* item);
      
      friend class GMSMidMifHandler;
      /**
        *   Update or check if any connections need to be updated
        *   from a RoutableItem.
        *   @param   item        The routeable item.
        *   @param   fakeUpdate  Optional parameter. If set to true
        *                        the method will not actually add
        *                        any connections, however the return
        *                        value will indicate whether any
        *                        connections need to be updated.
        *                        This is used in order to check if
        *                        a routeableitem fits this map.
        *                        Default set to false.
        *   @param   maxDist     Maximum allowed distance (in meters)
        *                        to other nodes in order
        *                        to allow a connection. 
        *                        Default set to 2 m.
        *   @param   itemIsForeign  Optional parameter that indicates
        *                           whether item is foreign (ie. not in
        *                           this map). Default set to not 
        *                           being foregin.
        *   @param   onlyConnectToFerries Optional parameter that 
        *                                 should be set to true if
        *                                 the item should only be
        *                                 allowed to connect to other
        *                                 ferry items.
        *                        
        *   @return  True if any connections were added, false otherwise.
        */  
      bool updateConnections(OldRouteableItem* item,
                             bool fakeUpdate = false,
                             uint32 maxDist = 2, 
                             bool itemIsForeign = false,
                             bool onlyConnectToFerries = false);
      //@}
      
      
     
      /**
       *    Help struct for updateCongestionSpeedLimits method.
       */
      struct congestionInfo_t {
         /** How many percent of the original speed that the speed should
          *  be set to in the citycentre.
          */
         float64 m_percentOfMaxSpeedInCC;

         /// Radius in meter from the citycentre.
         float64 m_sqCongestionRadiusMeter;
         
         /// City center position coordinate
         MC2Coordinate m_ccCoord;
      };
      

      
       /**
        *   Check all items on the map and set the location.
        *   @param   type                 The typs of the location to set.
        *   @param   onlyUpdateNonValid   If set to true, the location
        *                                 will only be updated for those
        *                                 items that does not already
        *                                 have a valid location.
        *                                 Default set to false.
        */
      void setItemLocation(ItemTypes::itemType type,
                           bool onlyUpdateNonValid = false);
      
      /**
       *    Help method to updateMetropolitanBuaMunLoc.
       *    Assumes that streetsegments have correct regions.
       *    Will go through all ssi:s that have the item as region, 
       *    and will count the number of regions of the specified itemtype
       *    that the ssi:s also have.
       *    The outparameter regionByNbrOccasions is a map containing:
       *    key: number of occurances of region in the item
       *    value: item id of region.
       *    Since it's a map, the keys are sorted in increasing order (i.e.
       *    the most common region is last in the map).
       *    Note that ssi:s that doesn't contain any region of the
       *    specified type, will also be counted. They receive region id
       *    MAX_UINT32.
       *
       *    @param   item  The item to check regions for.
       *    @param   regionByNbrOccasions [Out] Map containing the result.
       *    @param   type  Region item type to check.
       *    @return  The total number of ssi:s inside the item.
       *             (Also equal to the sum of regionByNbrOccasions keys.)
       */
      uint32 calcRegionStats( OldItem* item, 
                              map<uint32, uint32>& regionByNbrOccasions,
                              ItemTypes::itemType type );

      /**
        *   @name Index areas.
        */
      //@{
      
      /**
       * @return Returns the number of index areas that has geometry updated
       *         to convex hull.
       */
      uint32 setIdxAreaGeometryConvexHull();

      /**
       * Adopts hierarchies and naming of index areas to mc2 searching and 
       * location presentation.
       * 
       * @return Returns true on success.
       */
      bool customizeIndexAreaHierarchies();
      

      /**
       * From a set of index area items, select the one that has
       * the highest index area order (10 higher than 8).
       * @param   indexAreaItems The index area items to pick from
       * @param   indexAreasBasedOnOrder  A multimap with item IDs
       *             of all index areas keyed on index area order
       * @param   indexAreaOrders   A set with all possible
       *             insed area orders that exists in the
       *             indexAreasBasedOnOrder.
       *
       * @returns The ID of the index area with the highest order.
       */
      static uint32 getMostDetailedIndexArea(
                  vector<OldItem*> indexAreaItems, 
                  multimap<uint32,uint32> indexAreasBasedOnOrder,
                  set<uint32> indexAreaOrders );

      //@}

      
      
      /**
        *   @name Area feature draw display class.
        *   Methods for detect water-in-park etc area feature draw
        *   display classes, which are needed due to the fact that 
        *   the MC2 drawing methods cannot handle holes.
        */
      //@{
      
      /*
      *     Def. of map between item id and corresponding pre-computed
      *     bounding box
      */      
      typedef map<uint32, MC2BoundingBox> mapItemIdToBB_t;

      /**
      *     Def. of multimap from item type to item*, in order to store
      *     interior items of a specific type.
      */
      typedef multimap<ItemTypes::itemType, OldItem*> mapTypeToItemId_t;

      /**
       *    Init which types of items in items we want to
       *    consider for new area feature display classes
       */
      void areaFeatureDrawDisplayClassesInit(mapItemIdToBB_t&
                                                mapItemIdToBB,
                                             mapTypeToItemId_t& 
                                                mapTypeToItem );


      /**
       *    Method to compute which items belong to one specific
       *    area feature draw display class. As an example, an item belongs
       *    to display class waterInCityPark if it is a water (interior) item that
       *    lies, partially or completely, inside an (exterior) city park item.
       *
       *    @param   displayClass            the display class
       *    @param   interiorItemMaxLength   the maximum total lenght
       *                                     (circumference) of the interior item. 
       *                                     NOTE: param disregarded if 
       *                                     compareIntWithExt = true (see
       *                                     below). 
       *    @param   exteriorItemMinLength   the minimun total length
       *                                     (circumference) of the exterior item. 
       *                                     NOTE: param disregarded if 
       *                                     compareIntWithExt = true (see
       *                                     below). 
       *    @param   compareIntWithExt       If true then interiorItemMaxLength
       *                                     and exteriorItemMinLength are both
       *                                     disregarded, and instead we
       *                                     require that the total interior item
       *                                     length is smaller than the
       *                                     exterior item total length.
       *    @param   quotaRequired           [The number of coordinates from the
       *                                     interior item that lie inside the
       *                                     exterior item] / [total number of
       *                                     coords in the interior item] >
       *                                     quotaRequired.
       *    @param   interiorItemsToUse      If != NULL, we will not loop
       *                                     all items of the interior item
       *                                     type, but instead only consider
       *                                     these specific items. For example
       *                                     used when water in parks have been
       *                                     previously identified, and we want
       *                                     to identify islands in these water
       *                                     in parks.
       *
       *    @param   exteriorItemsToUse      If != NULL, we will not loop
       *                                     all items of the exterior item
       *                                     type, but instead only consider
       *                                     these specific items.  
       *
       *    @param   mapItemIdToBB            Mapping from item id to
       *                                     item bounding box.
       *
       *    @param   mapTypeToItemId         Mapping from an item type
       *                                     to all items (ids) of that type.
       * 
       *    @param   foundInteriorItems      This variable stores all items
       *                                     that are found to be interior
       *                                     items.
       */
      void itemInItemToDisplayClass( 
               ItemTypes::areaFeatureDrawDisplayClass_t displayClass,
               uint32 interiorItemMaxLength,
               uint32 exteriorItemMinLength,
               bool compareIntWithExt,
               float quotaRequired,
               mapTypeToItemId_t* interiorItemsToUse,
               mapTypeToItemId_t* exteriorItemsToUse,
               mapItemIdToBB_t mapItemIdToBB,
               mapTypeToItemId_t mapTypeToItemId,
               mapTypeToItemId_t* foundInteriorItems );
                                              
      //@}
};

#endif
