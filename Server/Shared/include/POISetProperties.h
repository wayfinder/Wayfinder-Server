/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POISETPROPERTIES_H
#define POISETPROPERTIES_H

#include "config.h"
#include "UserEnums.h"
#include "MapRights.h"
#include <set>

/**
 * This class holds information about a restricted poi set.
 *
 */
class POISet {
public:
   /**
    * @param userRight
    * @param mapRight 
    * @param priority
    * @param replacesStandardPOIs True if this set of POIs should replace
    *                             standard POIs.
    * @param name a name mostly for debugging purposes.
    */
   POISet( UserEnums::userRightService userRight, MapRights::Rights mapRight,
           uint32 priority, bool replacesStandardPOIs, const MC2String& name );

   /**
    * Create a POISet for finding.
    */
   explicit POISet( UserEnums::userRightService userRight );

   /**
    * Create a POISet for finding.
    */
   explicit POISet( MapRights::Rights mapRight );

   /**
    * @name Comparison of POISet objects.
    * @memo Comparison of POISet objects.
    * @doc  Comparison of POISet objects.
    */
   //@{
   bool operator == ( const POISet& b ) const;

   bool operator < ( const POISet& b ) const;

   bool operator != ( const POISet& b ) const;

   bool operator > ( const POISet& b ) const;

   bool operator <= ( const POISet& b ) const;

   bool operator >= ( const POISet& b ) const;
   //@}

   /**
    * @return UserRight needed for this set.
    */
   UserEnums::userRightService getUserRight() const;

   /**
    * @return MapRight needed for this set.
    */
   MapRights::Rights getMapRight() const;

   /**
    * @return the priority of this set. Lower value equals higher priority.
    */
   uint32 getPriority() const;

   /**
    * Get the replaces standard POIs.
    */
   bool getReplacesStandardPOIs() const;

   /**
    * Get the name.
    */
   const MC2String& getName() const;

   /**
    * Load from packet.
    * @param p The packet to load from.
    * @param pos Start loading data at this position in the packet.
    */
   void load( const Packet* p, int& pos );

   /**
    * Save to packet.
    * @param p The packet to save to.
    * @param pos Start writing data at this position in the packet.
    */
   void save( Packet* p, int& pos ) const;

   /**
    * @return Size in packet in bytes. 
    */
   uint32 getSizeInPacket() const;

private:
   /// The user right
   UserEnums::userRightService m_userRight;

   /// The map right
   MapRights::Rights m_mapRight;

   /// The priority, lower value equals higher priority.
   uint32 m_priority;

   /**
    * If this set of POIs replaces standard POIs. 
    * XXX: This might be need to be expaned to a combination of POI sets
    * to do replace.
    */
   bool m_replacesStandardPOIs;

   /// The name of the POI set.
   MC2String m_name;
};


class MapRightPOISetComp;

/**
 * This class holds information about restricted poi sets.
 *
 */
class POISetProperties {
public:
   typedef set< POISet > POISetMap;
   typedef set< POISet, MapRightPOISetComp > POISetMapByMapRight;

   /**
    * Get the access controlled POISet for userRight. Might return NULL.
    */
   static const POISet* getPOISet( UserEnums::userRightService userRight );

   /**
    * Get the access controlled POISet for userRight. Might return NULL.
    */
   static const POISet* getPOISet( UserEnums::URType userRight );

   /**
    * Get the access controlled POISet for map rights. Might return NULL.
    */
   static const POISet* getPOISet( const MapRights::Rights rights );

   /**
    * Get the access controlled POISet for mapRight. Might return "".
    */
   static MC2String getPOISetName( MapRights::Rights mapRight );

   /// @return all poi sets
   static const POISetMap& getAllPOISets();
   static const MapRights& getACPLayerRights() { 
      return m_acpLayerRights;
   }
private:
   /**
    * Default constructor. Private to avoid use.
    */
   POISetProperties();

   /// The current POI sets
   static POISetMap m_poiSets;

   /// The current POI sets sorted by MapRight
   static POISetMapByMapRight m_poiSetsByMapRight;
   static MapRights m_acpLayerRights; ///< combined rights for the acp layer
};


// ========================================================================
//                                      from user rights.Implementation of inlined methods =


inline bool POISet::operator == ( const POISet& b ) const { 
   return m_userRight == b.m_userRight;
}

inline bool POISet::operator < ( const POISet& b ) const {
   return m_userRight < b.m_userRight;
}

inline bool POISet::operator != ( const POISet& b ) const {
   return !(*this == b); 
}

inline bool POISet::operator > ( const POISet& b ) const { 
   return b < *this; 
}

inline bool POISet::operator <= ( const POISet& b ) const {
   return !(b < *this);
}

inline bool POISet::operator >= ( const POISet& b ) const {
   return !(*this < b); 
}

inline UserEnums::userRightService 
POISet::getUserRight() const {
   return m_userRight;
}

inline MapRights::Rights 
POISet::getMapRight() const {
   return m_mapRight;
}
inline uint32 
POISet::getPriority() const {
   return m_priority;
}
inline bool 
POISet::getReplacesStandardPOIs() const {
   return m_replacesStandardPOIs;
}

inline const MC2String& 
POISet::getName() const {
   return m_name;
}


#endif // POISETPROPERTIES_H
