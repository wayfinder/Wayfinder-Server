/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REGIONIDS_H
#define REGIONIDS_H


#include "config.h"
#include "NameCollection.h"
#include "MC2String.h"
#include <vector>
#include <map>

/**
 * Container of a set of region group ids and region ids.
 *
 */
class RegionList {
   public:
      /**
       * Holder of a list of regiongroupIDs and regionIDs.
       */
      RegionList() {
      }

      /**
       * Add a group id.
       */
      void addGroup( uint32 id ) {
         m_groupIDs.push_back( id );
      }

      /**
       * Get a reference to the group ids.
       */
      const vector< uint32 >& getGroups() const {
         return m_groupIDs;
      }

      /**
       * Add a region id.
       */
      void addRegion( uint32 id ) {
         m_regionIDs.push_back( id );
      }

      /**
       * Get a reference to the region ids.
       */
      const vector< uint32 >& getRegions() const {
         return m_regionIDs;
      }

   private:
      /// The groups IDs
      vector< uint32 > m_groupIDs;

      /// The region IDs.
      vector< uint32 > m_regionIDs;
};


/**
 *   Holds the region information from region_ids.xml.
 *   Will be empty by default.
 *   Use ServerRegionIDs if you want to read the file too.
 *
 */
class RegionIDs {
public:
   
      /**
       * Destructor.
       */
      virtual ~RegionIDs();

      /**
       * Check if an id is an region group id.
       */
      bool isRegionGroupID( uint32 id ) const;


      /**
       * Fill in the region ids that a region group has.
       *
       * @param regionIDs The vector to add region ids to.
       * @param regionGroupID The region group to add region ids for.
       * @return The number of added region ids.
       */
      uint32 addRegionIDsFor( vector<uint32>& regionIDs, 
                              uint32 regionGroupID ) const;

      /**
       * Fill in the region groups that a region has.
       *
       * @param regionGroupIDs The vector to add region groups to.
       * @param regionID The region to add region groups for.
       * @return The number of added region groups.
       */
      uint32 addRegionGroupsFor( vector<uint32>& regionGroupIDs, 
                                 uint32 regionID ) const;


      /**
       * Fill in all the region group ids.
       *
       * @param regionGroupIDs The vector to add region groups to.
       * @return The number of added region groups.
       */
      uint32 addAllRegionGroups( vector<uint32>& regionGroupIDs ) const;


      /**
       * Fill in all the region group ids and their ident.
       *
       * @param regionGroupIDs The vector to add region groups to.
       * @return The number of added region groups.
       */
      uint32 addAllRegionGroups( vector<pair<uint32, NameCollection> >& 
                                 regionGroupIDs ) const;

      /**
       * Get region id for MCC
       */
      uint32 getRegionIdFromMCC( const MC2String& mcc ) const;

      /**
       * Get region id for isoName
       */
      uint32 getRegionIdFromIsoName( const MC2String& isoName ) const;

      /// Returns true if the expandedID is present in the regionGroupID.
      bool isTheRegionInGroup( uint32 regionGroupID,
                               uint32 expandedID ) const;

      /**
       * Get a region list.
       */
      const RegionList* getRegionList( const MC2String& name ) const;

      /**
       * uint32 -> (uint32,name)*
       */
      typedef map< uint32, pair< vector<uint32>, NameCollection> > 
         regionGroupMap;
      /**
       * uint32 -> uint32*
       */
      typedef map< uint32, vector<uint32> > regionIDMap;

      /**
       * Region list.
       */
      typedef map< MC2String, RegionList > regionListMap;

      /**
       * MCC to id
       */
      typedef map < uint32, uint32 > mccMap;

      /**
       * ISO name to id
       */
      typedef map < MC2String, uint32 > isoNameMap;

protected:
      /**
       * The region ids for each region group.
       * The key value is the region_group id. 
       * The value is a pair, where the first value is a vector
       * containing the region ids of all regions that make up the
       * region_group, and the second value is the name or ident of
       * the region_group
       */
      regionGroupMap m_regionGroupMap;


      /**
       * The region groups for each region id.
       * The key value is the region id.
       * The value is an array containg all the ids of the
       * region_groups that the region is a part of.
       */
      regionIDMap m_regionIDMap;

      /**
       * The names lists of grouped regiongroups and regions.
       */
      regionListMap m_regionListMap;

      /**
       * The region id for each MCC
       */
      mccMap m_mccMap;

     /**
      * The region id for each ISO 3166-1 alpha-3 name
      */
     isoNameMap m_isoNameMap;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline bool
RegionIDs::isRegionGroupID( uint32 id ) const {
   return (id >= 0x200000);
}


#endif // REGIONIDS_H

