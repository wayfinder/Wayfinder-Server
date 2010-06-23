/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "POISetProperties.h"
#include "Packet.h"


POISet::POISet( UserEnums::userRightService userRight, 
                MapRights::Rights mapRight,
                uint32 priority, 
                bool replacesStandardPOIs, 
                const MC2String& name )
      : m_userRight( userRight ), m_mapRight( mapRight ), 
        m_priority( priority ), m_replacesStandardPOIs( replacesStandardPOIs ),
        m_name( name )
{
}

POISet::POISet( UserEnums::userRightService userRight )
      : m_userRight( userRight )
{}

POISet::POISet( MapRights::Rights mapRight ) 
      : m_mapRight( mapRight )
{}

void
POISet::load( const Packet* p, int& pos ) {
   m_userRight = UserEnums::userRightService( p->incReadLong( pos ) );
   m_mapRight = MapRights::Rights( p->incReadLong( pos ) );
   m_priority = p->incReadLong( pos );
   m_replacesStandardPOIs = p->incReadByte( pos );
   m_name = p->incReadString( pos );
}


void
POISet::save( Packet* p, int& pos ) const {
   p->incWriteLong( pos, m_userRight );
   p->incWriteLong( pos, m_mapRight );
   p->incWriteLong( pos, m_priority );
   p->incWriteByte( pos, m_replacesStandardPOIs );
   p->incWriteString( pos, m_name );
}

uint32
POISet::getSizeInPacket() const {
   
   uint32 sizeInBytes = 3/*padding*/ + 3*4/*right*2 +prio*/ + 1/*replaces*/;

   sizeInBytes += m_name.size() + 1;

   return sizeInBytes;
}




POISetProperties::POISetMap initPOISets() {
   POISetProperties::POISetMap m;

#define POI_INSP( userRight, mapRight, priority, replacesStandardPOIs, name ) \
   m.insert( POISet( UserEnums::userRight, MapRights::mapRight, \
                     priority, replacesStandardPOIs, name ) )

   // The POI sets with priority
   // Example POI_INSP( UR_POI_TEST,  POI_TEST_SHOP,  25, false, "TEST SHOP" );
   // Don't forget to add rights to m_acpLayerRights if the rights
   // are suppose to be in the ACP layer
#undef POI_INSP

   return m;
}

POISetProperties::POISetMap POISetProperties::m_poiSets = initPOISets();
MapRights POISetProperties::m_acpLayerRights = 
   MapRights( MapRights::Masks() );

class MapRightPOISetComp {
public:
   bool operator()( const POISet& a, const POISet& b ) const {
      return a.getMapRight() < b.getMapRight();
   }
};


POISetProperties::POISetMapByMapRight POISetProperties::m_poiSetsByMapRight( 
   m_poiSets.begin(), m_poiSets.end() );


const POISet* 
POISetProperties::getPOISet( UserEnums::userRightService userRight ) {
   POISetMap::const_iterator findIt = m_poiSets.find( POISet( userRight ) );
   if ( findIt != m_poiSets.end() ) {
      return &*findIt;
   } else {
      return NULL;
   }
}

const POISet*
POISetProperties::getPOISet( UserEnums::URType userRight ) {
   return getPOISet( userRight.service() );
}

const POISetProperties::POISetMap& POISetProperties::getAllPOISets() {
   return m_poiSets;
}

MC2String
POISetProperties::getPOISetName( MapRights::Rights rights ) {
   const POISet* pset = getPOISet( rights );
   return pset ? pset->getName() : "";
}

const POISet* 
POISetProperties::getPOISet( MapRights::Rights rights ) {
   POISetMapByMapRight::const_iterator findIt = 
      m_poiSetsByMapRight.find( POISet( rights ) );
   if ( findIt != m_poiSetsByMapRight.end() ) {
      return &*findIt;
   } else {
      return NULL;
   }
}
