/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPMODULENOTICECONTAINER_H
#define MAPMODULENOTICECONTAINER_H

#include "config.h"

#include "STLStringUtility.h"
#include "ItemIDTree.h"
#include "RegionIDTree.h"
#include "MapTopRegion.h"
#include "MapGenEnums.h"

#include <map>

class MapModuleNotice;

/**
 *    Class containing MapModuleNotices and TopRegions.
 *    XXX: Not fully implemented.
 *
 */
class MapModuleNoticeContainer 
{
   public:

      /**
       *    Deletes all dynamically allocated memory in the
       *    MapModuleNoticeContainer.
       */
      virtual ~MapModuleNoticeContainer();

      /**
       *    Returns the number of MapModuleNotices in the container.
       *    Called getSize for backwards compatilbility.
       *    @return The size of the internal vector of MapModuleNotices.
       */
      inline uint32 getSize() const;

      
      /**
       *    Get the element at position pos. <b>NB!</b> No check 
       *    is done against reading outside the buffer in the
       *    release version.
       *    @param pos   The position of the element to return.
       *    @return The element at position pos.  
       */
      inline const MapModuleNotice* getElementAt( uint32 pos ) const;

      
      /**
       *    Get the element at position pos. <b>NB!</b> No check 
       *    is done against reading outside the buffer in the
       *    release version.
       *    @param pos   The position of the element to return.
       *    @return The element at position pos.  
       */
      inline const MapModuleNotice* operator[](uint32 pos) const; 

      
      /**
       *    Loads an <code>index.db</code>-type-of-file.
       *    Implemented by loading the MapModuleObjVector and
       *    adding everything in it to this container.
       *    @param fileName The fileName of the index.db file.
       *    @param mapSet Map set ID to use, if default value MAX_UINT32 used
       *                  mapSet is ignored
       *    @return True if loading was succesful.
       */
      bool load(const char* fileName, uint32 mapSet = MAX_UINT32);
      
      
      /**
       *   Loads the new format of index.db.
       *   @param buf the data buffer that holds map data
       *   @param mapSet Map set ID to use, if MAX_UINT32 is used mapSet 
       *                 is ignored
       *   @return false if error.
       */
      bool load(DataBuffer& buf, uint32 mapSet);
      
      /**
       *    XXX: To be implemented.
       *
       */
      //bool save(DataBuffer* buf);


      /**
       *    Gets the notice with the supplied mapID.
       *    @return The notice for the supplied mapid or NULL if
       *            not found.
       */
      const MapModuleNotice* getMap(uint32 mapID) const;

      /**
       *    Returns the mapid of the map with the supplied name.
       *    @return MAX_UINT32 if not found or the map id if
       *            found.
       */
      uint32 getMapID(const char* mapName) const;
      
      /**
       *    Returns a pointer to the MapTopRegion with the
       *    supplied id.
       *    @param regionID ID of the region.
       *    @return The topregion with the supplied id or NULL.
       */
      inline const MapTopRegion* getRegion(uint32 regionID) const;

      
      /**
       *    Add all top regions of a certain type to a vector.
       *    @param   regions  The vector to add top regions to.
       *    @param   type     The type of top region. Default set to
       *                      nbr_top_region_t which corresponds to 
       *                      all available top region types.
       */
      void getTopRegions( ConstTopRegionMatchesVector& regions,
                          TopRegionMatch::topRegion_t type =
                              TopRegionMatch::nbr_topregion_t ) const; 

      
      /**
       *    Puts the id:s of the overview maps above the supplied mapid
       *    into the vector <code>res</code>.
       *    @param mapID The id of the interresting map.
       *    @param res   The overview maps above the map are put here.
       *    @return true If there is a map with the requested id.
       */
      bool getMapsAbove( uint32 mapID, vector<uint32>& res ) const;
     

      /**
       *    Gets a const reference to the map tree (ItemIDTree), 
       *    containing the map hierarchy.
       *    @return  A const reference to the map tree.
       */
      inline const ItemIDTree& getMapTree() const;


      /**
       * @name Methods for accessing the stored map supplier coverage
       *       areas data.
       *
       * getMapSupCoverageAreas Returns a vector with areas mapped to 
       *                        supplier.
       * 
       * getMapSupCoverageTree  Returns what areas areas are gemetrically
       *                        inside other areas. Larger area mapped to
       *                        smaller area. 
       *                        
       *                        Identifies areas with index in the vector 
       *                        returned by getMapSupCoverageAreas.
       *
       * getCopyrightByMapSup   Returns the strings to show when creating
       *                        copyright notice from a map supplier 
       *                        referenced in the vector returned by
       *                        getMapSupCoverageAreas.
       */ 
      //@{

      
      /**
       * Vector with bounding boxes mapped to copyright string.
       */
      typedef vector<pair<MC2BoundingBox, MapGenEnums::mapSupplier> > 
         mapSupCoverage_t;

      /**
       *  @return Returns all map supplier coverage information.
       *
       *  Get the strings of the map suppliers returned with 
       *  the method getCopyrightByMapSup.
       */
      const mapSupCoverage_t& getMapSupCoverageAreas() const;

      /** 
       *  Key:   Map supplier
       *  Value: All names we have for this map supplier, in all languages.
       */
      typedef map<MapGenEnums::mapSupplier, NameCollection> 
         mapSupNamesByMapSup_t;
      

      /**
       *  @param  mapSupplier The map supplier to return the names of.
       *
       *  @return A name collection with all names in different languages 
       *          we have of a specific map supplier. If the names are missing
       *          an empty NameCollection is returned.
       */
      const NameCollection& getMapSupNames(MapGenEnums::mapSupplier 
                                           mapSupplier) const;

      /**
       *  @return All map supplier names.
       */ 
      mapSupNamesByMapSup_t getAllMapSupNames();

      /**
       * Type for storing the hierarchy of map supplier coveratge areas
       * using the indexes in the mapSupCov vector.
       *
       * First:  Index of the larger box.
       * Second: Index of a box covered by the larger box.
       */
      typedef vector<pair < uint32, uint32> > coverageTree_t;

      /** @return Returns the hierarchy of the map supplier coverage 
       *          returned by getMapSupCoverageAreas.
       */
      const coverageTree_t& getMapSupCoverageTree() const;

      /**
       * Map with map supplier name string to be used for copyright
       * mapped to map supplier type.
       */
      typedef map< MapGenEnums::mapSupplier, MC2String > 
         copyrightByMapSup_t;

      /**
       *  @return Returns the copyright map supplier names of the map
       *          suppliers which are used as supplier to the maps this
       *          index keeps information of. I.e. not the copyright of 
       *          all map suppliers. If a map supplier not present in the
       *          code is used, the empty string is returned for that
       *          map supplier.
       */
      const copyrightByMapSup_t& 
         getCopyrightByMapSup(LangTypes::language_t 
                              lang = LangTypes::invalidLanguage) const;

      //@}


      
      /**
       *    XXX: To be implemented.
       *
       */
      //getTopRegion(uint32 mapID, 
      //      topRegion_t type, 
      //      vector<TopMapRegion*>& regions);

  protected:

      /**
       *   Loads the old format of index.db.
       *   Does not close the file.
       *   @param fd Filedescriptor to load from.
       *   @param totalLength The length to read.
       *   @param nbrMaps     The number of maps to read.
       *   @return false if error.
       */
      bool oldLoad( int fd,
                    uint32 totalLength,
                    uint32 nbrMaps);

      
      /**
       *   Adds the notice to the internal structures.
       *   If the notice contains a country map a TopRegion is created.
       *   @param notice The notice to add.
       *   @param fakeRegions True if the old kind of index.db is used,
                              i.e. one without region information.
       */
      void addNotice( MapModuleNotice* notice,
                      bool fakeRegions = false);

      /**
       *    Initialize the MapModuleNoticeContainer with old
       *    MapModuleNotices and use the country code to fake the
       *    regions.
       *    This method should be removed, and instead the information
       *    should come from index.db.
       */
      void initUsingCC( const vector<MapModuleNotice*>& oldVector );

      /**
       *    Returns a topregion with the supplied id or NULL
       *    if it wasn't found. For internal use.
       *    @param regionID The region id to search for.
       *    @return The found region or NULL.
       */
      MapTopRegion* findRegion(uint32 regionID) const;
      
      /**
       *   Empties the internal structures and deletes the
       *   MapModuleNotices.
       */
      void deleteAllObjs();


      /**
       * Loads the map supplier coverage information from the databuffer 
       * supplied as an argument.
       *
       * @param db The data buffer to load the map supplier coverage 
       *           data from.
       */
      void loadMapSupCoverage( DataBuffer* db );


      /// Type of the vector of MapModuleNotices
      typedef vector<MapModuleNotice*> noticeVect_t;
         
      /**
       *   Vector containing all the MapModuleNotices.
       *   This vector owns the notices and it is where they should
       *   be deleted from.
       */ 
      noticeVect_t m_allNotices;

      /**
       *    Tree containing mapID:s and their relations
       *    (overview map containing underviewmaps etc.).
       */
      ItemIDTree m_mapTree;
      
      /**
       *    Tree containing regions ID:s and their relations
       *    (region containing other regions etc.).
       */
      RegionIDTree m_regionTree;

      /// TopRegion map type
      typedef map<uint32, MapTopRegion*> topRegionMap_t;
      
      /// Map of topregion id:s and topregions.
      topRegionMap_t m_regions;


      
  protected:
      
      /// Type of the map of maps
      typedef map<uint32, MapModuleNotice*> noticeMap_t;

      /// Map of map id:s and MapModuleNotice:s.
      noticeMap_t m_noticeMap;

      /// Type of map of map names and MapModuleNotices.
      typedef map<const char*,
                  MapModuleNotice*,
                  STLStringUtility::ltstr> noticesByName_t;
      
      /// Map of map names and MapModuleNotices.
      noticesByName_t m_noticesByName;

      /**
       * Vector with bounding boxes telling what map supplier 
       * copyright string to use in a specific area. Indexes of this
       * vector is used as ID in m_mapSupCoverageTree.
       * 
       */
      mapSupCoverage_t m_mapSupCoverage;

      /**
       * Tells what map coverage boxes that include what other boxes. 
       * Parent is larger than child. ID is index of m_mapSupCoverage.
       * Frist:  Parent m_mapSupCoverage index, MAX_UINT32 for top parent.
       * Second: Child m_mapSupCoverage index
       */
      coverageTree_t m_mapSupCoverageTree;

      /**
       * Keeps the copyright string versions of the mapsuppliers stored in
       * m_mapSupCoverage. Check that it is filled in before using.
       */
      mutable copyrightByMapSup_t m_copyrightByMapSup;

      /// All names we have for all map suppliers in all languages.
      mapSupNamesByMapSup_t m_mapSupNamesByMapSup;

      /// Used for map suppliers with no name.
      static const NameCollection m_emptyNameCollection;
      
}; // class MapModuleNoticeContainer

/**
 *   Subclass to MapModuleNoticeContainer to be used when creating
 *   index.db. Has addLast and save methods to be used for this.
 */
class WriteableMapModuleNoticeContainer : public MapModuleNoticeContainer
{
public:
   
   /**
    *   Adds a MapModuleNotice to the container.
    *   @param notice The notice to add.
    */
   void addLast( MapModuleNotice* notice );

   /**
    *    Adds a region. Will not be copied.
    *    @param   region   The region.
    */
   void addRegion( MapTopRegion* region );

   /**
    *    Set item id tree.
    *    @param   tree  The item id tree.
    */ 
   void setMapTree( const ItemIDTree& tree );
   
   /**
    *    Gets a reference to the map tree (ItemIDTree), containing the
    *    map hierarchy.
    *    @return  A reference to the map tree.
    */
   inline ItemIDTree& getMapTree();
   
   /**
    *    Set region tree.
    *    @param   tree  The region tree.
    */ 
   void setRegionTree( const RegionIDTree& tree );
   
   /**
    *    Clear all regions.
    */ 
   void clearRegions();
   
   /**
    *   Saves the container to the file <code>filename</code>.
    *   NB! Creates a MapModuleObjVector and saves it.
    *   @param filename The filename of the file to save the container in.
    *   @return true if successful.
    */
   bool save( const char* filename );

   /**
    *    Set or get the element at position pos.
    *    <br>
    *    <b>NB!</b> No check 
    *    is done against writing outside the buffer in the release
    *    version.
    *    <br>
    *    @param   pos   The position of the element to return.
    */
   inline MapModuleNotice* &operator[](uint32 pos);
   
   /**
    *    Gets the notice with the supplied mapID.
    *    The notice may be modified.
    *    @return The notice for the supplied mapid or NULL if
    *            not found.
    */
   inline MapModuleNotice* getMap(uint32 mapID);

   /**
    *    Set the map supplier coverage data.
    *
    *    @mapSupCov Complete map supplier coverage data. Any present data
    *               is replaced.
    *    @coverageTree Tells what coverage areas include other coverage
    *                  areas. Parent mapped to child, a child is smaller
    *                  than a parent.
    */
   void setMapSupCoverage( mapSupCoverage_t& mapSupCov, 
                           coverageTree_t& coverageTree );

   /**
    *  Set all names, of map all map suppliers, in all langauges we have.
    *  @param The names to set, sorted by map supplier.
    */ 
   void setMapSupNames(mapSupNamesByMapSup_t& 
                       mapSupNamesByMapSup);



 private:

   /**
    * @name Load and save map supplier coverage data.
    *
    */
   //@{

   /**
    * Calculates the maximum size required for storing the maps supplier
    * coverage data to a data buffer.
    */
   uint32 mapSupCovSizeInDataBuffer();

   /**
    * Saves the map supplier coverage information to the databuffer 
    * supplied as an argument.
    *
    * @param db The data buffer to save the map supplier coverage 
    *           data to.
    */
   void saveMapSupCoverage( DataBuffer* db );

   //@}
};

// ----------- Implementation of inlined methods ----

inline uint32
MapModuleNoticeContainer::getSize() const
{
   return m_allNotices.size();
}


inline const MapModuleNotice*
MapModuleNoticeContainer::getElementAt(uint32 pos) const
{
   MC2_ASSERT( pos < m_allNotices.size() );
   return m_allNotices[pos];
}


inline const MapModuleNotice*
MapModuleNoticeContainer::operator[](uint32 pos) const
{
   MC2_ASSERT( pos < m_allNotices.size() );
   return m_allNotices[pos];
}


inline MapTopRegion*
MapModuleNoticeContainer::findRegion(uint32 regionID) const
{
   // This is the internal function which does not return
   // a constant MapTopRegion
   topRegionMap_t::const_iterator it = m_regions.find( regionID );
   if ( it != m_regions.end() ) {
      return it->second;
   } else {
      return NULL;
   }
}

inline const MapTopRegion*
MapModuleNoticeContainer::getRegion(uint32 regionID) const
{
   return findRegion(regionID);
}


inline const ItemIDTree& 
MapModuleNoticeContainer::getMapTree() const
{
   return m_mapTree;
}


// -- WriteableMapModuleNoticeContainer -------------------

inline MapModuleNotice* & 
WriteableMapModuleNoticeContainer::operator[](uint32 pos) 
{
   MC2_ASSERT( pos < m_allNotices.size() );
   return m_allNotices[pos];
}

inline MapModuleNotice* 
WriteableMapModuleNoticeContainer::getMap( uint32 mapID ) 
{
   noticeMap_t::iterator it = m_noticeMap.find( mapID );
   if ( it != m_noticeMap.end() ) {
      return it->second;
   } else {
      return NULL;
   }
}


inline ItemIDTree& 
WriteableMapModuleNoticeContainer::getMapTree()
{
   return m_mapTree;
}


#endif

