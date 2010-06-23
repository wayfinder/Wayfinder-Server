/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REGIUONITEMIDTREE_H
#define REGIUONITEMIDTREE_H

#include "config.h"
#include "ItemIDTree.h"

/**
 *   Class describing the relationship between regions,
 *   which region that contains which and so on.
 *   This is a lazy implementation which makes use of the
 *   ItemIDTree even if the RegionIDTree does not need to
 *   save two uint32 values for each region. This could be
 *   fixed when it is really necessary.
 *
 *   @see ItemIDTree
 */
class RegionIDTree : private ItemIDTree {
public:

   // -------- Get methods
   
   /**
    *   Returns the top level regions in regionIDs.
    *   @param regionIDs Set to put the regionIDs into.
    */
   inline void getTopLevelRegionIDs(set<uint32>& regionIDs) const;

   
   /**
    *   Puts the lowest level regions in regionIDs.
    *   @param regionIDs Set to put the result into.
    */
   inline void getLowestLevelRegionIDs(set<uint32>& regionIDs) const;

   
   /**
    *   Puts the regions inside <code>regionID</code> into
    *   <code>contents</code>.
    *   @param regionID The region to look for.
    *   @param contents The regionIDs to be shown as part of
    *                   the region regionID
    */
   inline void getContents(uint32 regionID, set<uint32>& contents) const;

   
   // -------- Methods for adding

   /**
    *   Adds the region in <code>regionID</code> as part of the
    *   <code>parentRegionID</code>.
    */
   inline void addRegion( uint32 parentRegionID,
                          uint32 regionID );


   /**
    *   Moves region id that belong to region ids that do not exist
    *   to the top level.
    */
   void fixupTopLevel();
   
   // -------- Packet and DataBuffer - related methods
   
   /**
    *    Loads the RegionIDTree from the 
    *    supplied DataBuffer.
    *    @param buf Buffer to load from.
    *    @return True if ok.
    */
   inline bool load(DataBuffer* buf);

   /**
    *    Saves the RegionIDTree into the supplied DataBuffer.
    *
    *    @param  buf Buffer to save into.
    *    @return     True if ok.
    */
   inline bool save(DataBuffer* buf) const;

   /**
    *    Returns the number of bytes that this ItemIDTree
    *    needs in a databuffer.
    */
   inline int getSizeInDataBuffer() const;
   
   /**
    *    Loads the RegionIDTree from the supplied Packet.
    *
    *    @param p   Packet to read from.
    *    @param pos Position in the packet. Will be updated.
    *    @return    True if ok.
    */   
   inline bool load(const Packet* p, int& pos);

   /**
    *    Saves the RegionIDTree to the packet.
    *    @param p   Packet to save the tree into.
    *    @param pos Position in packet.
    */
   inline bool save(Packet* p, int& pos) const;

   /**
    *    Returns the number of bytes that this ItemIDTree
    *    needs in a Packet.
    */
   inline int getSizeInPacket() const;
};

inline void
RegionIDTree::getTopLevelRegionIDs(set<uint32>& regionIDs) const
{
   ItemIDTree::getTopLevelMapIDs(regionIDs);
}

inline void
RegionIDTree::getLowestLevelRegionIDs(set<uint32>& regionIDs) const
{
   ItemIDTree::getLowestLevelMapIDs(regionIDs);
}


inline void
RegionIDTree::getContents(uint32 regionID, set<uint32>& contents) const
{
   set<IDPair_t> items;
   ItemIDTree::getContents(regionID, items);
   for( set<IDPair_t>::const_iterator it(items.begin());
        it != items.end();
        ++it ) {
      contents.insert(it->first);
   }
}


inline void
RegionIDTree::addRegion( uint32 parentRegionID,
                         uint32 regionID )
{
   ItemIDTree::addMap( parentRegionID, regionID);
}


inline bool
RegionIDTree::load(DataBuffer* buf)
{
   return ItemIDTree::load(buf);
}


inline bool
RegionIDTree::save(DataBuffer* buf) const
{
   return ItemIDTree::save(buf);
}


inline int
RegionIDTree::getSizeInDataBuffer() const
{
   return ItemIDTree::getSizeInDataBuffer();
}


inline bool
RegionIDTree::load(const Packet* p, int& pos)
{
   return ItemIDTree::load(p, pos);
}


inline bool
RegionIDTree::save(Packet* p, int& pos) const
{
   return ItemIDTree::save(p, pos);
}


inline int
RegionIDTree::getSizeInPacket() const
{
   return ItemIDTree::getSizeInPacket();
}

#endif
