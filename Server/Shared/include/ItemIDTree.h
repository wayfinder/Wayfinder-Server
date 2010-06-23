/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMIDTREE_H
#define ITEMIDTREE_H

#include<set>
#include<map>

#include "config.h"
#include "IDPairVector.h"

class DataBuffer;
class Packet;

/**
 *   ItemIDTree is a "tree" of map and ItemID:s. 
 *   It is used to describe the contents of a TopRegion but
 *   could also be used to describe the relationship between
 *   overview maps and underviewmaps.
 *   @see RegionIDTree if you want to describe the relationship
 *                     between TopRegions.
 */
class ItemIDTree {
   
public:

   // -------- Get methods

   /**
    *    Returns true if the ItemIDTree is empty.
    */
   bool empty() const;
   
   /**
    *    Returns the IDs of the maps in the top of the tree.
    *    @param mapIDs Set where the top level maps will be added.
    */
   void getTopLevelMapIDs(set<uint32>& mapIDs) const;

   /**
    *    Returns the IDs of the maps in the bottom of the tree.
    *    These may or may not contain items.
    *    @param mapIDs The map ids of the bottom maps will be added
    *                  here.
    */
   void getLowestLevelMapIDs(set<uint32>& mapIDs) const;

   /**
    *    Returns the ids inside the map. If the itemid
    *    of a pair is MAX_UINT32 then the whole map is
    *    inside.
    *    @param mapID The mapid to return the contents for.
    *    @param items The items inside that map.
    */
   void getContents(uint32 mapID, set<IDPair_t>& items) const;
   
   /**
    *    Updates the ItemIDTree with the contents of the 
    *    specified map id.
    *    @param mapID  The mapid to return the contents for.
    *    @param idTree The ItemIDTree to be updated with the contents.
    */
   void getContents( uint32 mapID, ItemIDTree& idTree ) const;

   /**
    *    Returns true if the map with id <code>mapID</code>
    *    does not contain any submaps.
    *    @param mapID The map id to check.
    *    @return True if the whole map is included.
    */
   bool wholeMap(uint32 mapID) const;

   /**
    *    Adds the ids of completely covered maps into the
    *    set <code>wholeMaps</code>.
    *    @param wholeMaps Place to put the ids
    *    @return Nbr of added map ids.
    */
   int getWholeMaps( set<uint32>& wholeMaps ) const;

   /**
    *    Checks if the specified map is present in the tree.
    *    @param   mapID    The map id.
    *    @return  True if the map is found in the tree.
    */
   inline bool containsMap( uint32 mapID ) const;

   /**
    *    Adds the IDPair_t:s of the items that are in
    *    the not completely covered maps to the
    *    set <code>items</code>.
    *    @param items Set to add the items to.
    *    @return The number of items.
    */
   int getAllItems( set<IDPair_t>& items ) const;

   /**
    *    Returns the map ID of the map on level up. 
    *    @param mapID The map ID.
    *    @return The map ID of the higher level map, or MAX_UINT32 if
    *            there is no higher level map.
    */
   uint32 getHigherLevelMap( uint32 mapID ) const;

   /**
    *    Returns the overview maps for the supplied map.
    *    Probably mostly used when the ItemIDTree is used as
    *    a container for mapID:s.
    *    @param mapID     Map to look for.
    *    @param overviews The overview maps from the top down will
    *                     be put here. If the requested map is a
    *                     top level overview map the vector will
    *                     be empty.
    *    @return True if the map could be found.
    */
   bool getOverviewMapsFor(uint32 mapID, vector<uint32>& overviews) const;

   /**
    * Get lower level mapID for a top level mapID.
    *
    * @param topMapID The top level mapID to lookup lower level mapIDs for.
    * @param lower The lower level mapIDs will be added to this vector.
    * @return True if topMapID could be found.
    */
   bool getLowerMapsFor( uint32 topMapID, vector<uint32>& lower ) const;

   // -------- Methods for adding

   
   /**
    *    Adds a map to the tree.
    *    @param parentMapID The id of the map that contains the map.
    *    @param mapID       The map.
    */
   void addMap( uint32 parentMapID, uint32 mapID);

   /**
    *    Adds an item to the tree. 
    *    @param parentMapID The id of the map that contains the item.
    *    @param itemID      The id of the Item.
    */
   void addItem( uint32 parentMapID, uint32 itemID);

   
   // -------- Packet and DataBuffer - related methods
   
   /**
    *    Loads the ItemIDTree from the 
    *    supplied DataBuffer.
    *    @param buf Buffer to load from.
    *    @param mapSet The map set instance this tree corresponds to, not used
    *                  if default value MAX_UINT32 is passed
    *    @return True if ok.
    */
   bool load(DataBuffer* buf, uint32 mapSet = MAX_UINT32);

   /**
    *    Saves the ItemIDTree into the supplied DataBuffer.
    *
    *    @param  buf Buffer to save into.
    *    @return     True if ok.
    */
   bool save(DataBuffer* buf) const;

   /**
    *    Returns the number of bytes that this ItemIDTree
    *    needs in a databuffer.
    */
   int getSizeInDataBuffer() const;
   
   /**
    *    Loads the ItemIDTree from the supplied Packet.
    *
    *    @param p   Packet to read from.
    *    @param pos Position in the packet. Will be updated.
    *    @return    True if ok.
    */   
   bool load(const Packet* p, int& pos);

   /**
    *    Saves the ItemIDTree to the packet.
    *    @param p   Packet to save the tree into.
    *    @param pos Position in packet.
    */
   bool save(Packet* p, int& pos) const;

   /**
    *    Returns the number of bytes that this ItemIDTree
    *    needs in a Packet.
    */
   int getSizeInPacket() const;

protected:

   /**
    *    Adds map or item, depending on the value of
    *    itemID. MAX_UINT32 means map.
    */
   void addMapOrItem( uint32 parentMapID, uint32 mapID,
                      uint32 itemID );
   
   /**
    *    Type for the map containing the mapping from
    *    overviewmap to contents.
    *    
    *    Key:   parent (overview) map ID
    *    Value: child map and item ID
    */
   typedef multimap<uint32, IDPair_t> overviewKeyed_t;

   /**
    *    Storage used for finding maps by using the overview
    *    map id of the map.
    */
   overviewKeyed_t m_byOverview;
   
   
};

// Inlines
   
inline bool 
ItemIDTree::containsMap( uint32 mapID ) const
{
   
   if ( mapID == MAX_UINT32 ) {
      // This is not a valid map id. No need to search for it.
      return false;
   }
   
   if ( m_byOverview.find( mapID ) != m_byOverview.end() ) {
      return true;
   }
   
   overviewKeyed_t::const_iterator it = m_byOverview.begin();
   while ( it != m_byOverview.end() ) {
      if ( it->second.first == mapID ) {
         return true;
      }
      ++it;
   }
   return false;
}

#endif
