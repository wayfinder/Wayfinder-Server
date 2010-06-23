/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TrafficMapInfo.h"

#include "DisturbanceElement.h"
#include "DeleteHelpers.h"

#include "MC2BoundingBox.h"
#include "MapModuleCom.h"
#include "IDTranslationTable.h"
#include "InfoSQL.h"
#include "UserRightsMapInfo.h"
#include "DisturbanceList.h"

struct TrafficMapInfo::Impl {
   typedef TrafficMapInfo::Disturbances Disturbances;
   typedef TrafficMapInfo::IDToDisturbanceMap IDToDisturbanceMap;
   typedef map< DisturbanceElement::SituationReference, DisturbanceElement* > SitToDisturbanceMap;

   Impl():m_mapID( ~0 ) { }
   ~Impl() {
      STLUtility::deleteValues( m_disturbances );
   }

   /// Add disturbance.
   void addDisturbance( DisturbanceElement* disturbance );
   /// Remove a disturbance
   void removeDisturbance( const DisturbanceElement* disturbance );


   /// All disturbances
   Disturbances m_disturbances;
   /// Maps disturbance id to situation reference
   IDToDisturbanceMap m_distMap;
   /// Maps situation reference to disturbance element
   SitToDisturbanceMap m_sitRefMap;
   /// The current maps bounding box.
   MC2BoundingBox m_mapBox;
   /// item id translation table, empty.
   IDTranslationTable m_idTranslationTable;
   /// Current map id.
   uint32 m_mapID;
};

TrafficMapInfo::TrafficMapInfo( MapID id ):
   m_impl( new Impl() ) {
   m_impl->m_mapID = id;
}

TrafficMapInfo::~TrafficMapInfo() {
   delete m_impl;
}


bool TrafficMapInfo::load( InfoSQL& database ) {
   MC2BoundingBox mapBox = MapModuleCom::getBBoxForMap( m_impl->m_mapID );
   // must have a valid box
   if ( mapBox == MC2BoundingBox() ) {
      return false;
   }

   Disturbances disturbances;
   if ( ! database.getDisturbancesWithinBBox( mapBox, disturbances ) ) {
      return false;
   }

   if ( ! MapModuleCom::getDisturbanceNodes( m_impl->m_mapID,
                                             disturbances ) ) {
      STLUtility::deleteValues( disturbances );
      return false;
   }

   // All data was loaded ok.

   m_impl->m_mapBox = mapBox;

   m_impl->m_disturbances.swap( disturbances );
   STLUtility::deleteValues( disturbances );

   // setup lookup tables
   // recreate id -> disturbance map
   // recreate situation ref -> distubance map
   m_impl->m_sitRefMap.clear();
   m_impl->m_distMap.clear();
   for ( Disturbances::const_iterator it = m_impl->m_disturbances.begin(),
            itEnd = m_impl->m_disturbances.end(); it != itEnd; ++it ) {
      m_impl->m_distMap[ (*it)->getDisturbanceID() ] = *it;
      m_impl->m_sitRefMap[ (*it)->getSituationReference() ] = *it;
   }

   return true;
}
   
const TrafficMapInfo::Disturbances&
TrafficMapInfo::getDisturbances() const {
   return m_impl->m_disturbances;
}

TrafficMapInfo::NodeIDsToDisturbances
TrafficMapInfo::findNodeID( const NodeIDs& ids,
                                const UserRightsMapInfo& userRights ) const {
   NodeIDsToDisturbances distVect;

   // For each disturbance; check nodes and the rights for that node.
   // Add only nodes with the correct right.
   for ( Disturbances::const_iterator 
            it = m_impl->m_disturbances.begin(), 
            itEnd = m_impl->m_disturbances.end();
         it != itEnd;
         ++it ) {

      const DisturbanceElement* distElem = *it;

      // Add nodes with the correct rights
      const NodeIDs& nodeIDSet = distElem->getNodeIDSet();
      for ( NodeIDs::const_iterator
               si = nodeIDSet.begin(), 
               siEnd = nodeIDSet.end();
            si != siEnd; ++si ) {
         NodeIDs::value_type currNodeID = *si;
         if ( ids.find( currNodeID ) != ids.end() ) {
            if ( userRights.
                 itemAllowed( distElem->getNeededRights(),
                              IDPair_t( getMapID(), currNodeID ),
                              m_impl->m_idTranslationTable ) ) {
               distVect.push_back( make_pair( currNodeID, distElem ) );
            }
         }
      }
   }

   return distVect;
}

int TrafficMapInfo::
getNodesWithDelays( const UserRightsMapInfo& userRights,
                    NodesWithDelays& delays ) const {
   delays.clear();

   if ( userRights.empty() ) {
      return delays.size();
   }

   // iterate over all maps with disturbances.
   for ( Disturbances::const_iterator
            it = m_impl->m_disturbances.begin(),
            itEnd = m_impl->m_disturbances.end();
         it != itEnd;
         ++it ) {

      const DisturbanceElement* dist = *it;

      uint32 factor = DisturbanceListElement::
         infoModuleFactorToRaw( dist->getCostFactor() );

      const NodeIDs& nodes = dist->getNodeIDSet();
         
      for ( NodeIDs::const_iterator si = nodes.begin();
            si != nodes.end(); ++si ) {
         NodeIDs::value_type nodeID = *si; 
         if ( userRights.itemAllowed( dist->getNeededRights(),
                                      IDPair_t( getMapID(), nodeID ),
                                      m_impl->m_idTranslationTable ) ) {
            delays.push_back( make_pair( nodeID, factor ) );
         }
      }
   }

   return delays.size();
}

const IDToDisturbanceMap&
TrafficMapInfo::getDisturbanceList() const {
   return m_impl->m_distMap;
}

uint32 TrafficMapInfo::mapSize() const {
   return m_impl->m_disturbances.size();
}

const MC2BoundingBox& TrafficMapInfo::getMapBox() const {
   return m_impl->m_mapBox;
}

TrafficMapInfo::MapID
TrafficMapInfo::getMapID() const {
   return m_impl->m_mapID;
}

void TrafficMapInfo::
Impl::addDisturbance( DisturbanceElement* disturbance ) {

   // Find current situation using situation reference
   SitToDisturbanceMap::iterator sitIt =
      m_sitRefMap.find( disturbance->getSituationReference() );

   if ( sitIt == m_sitRefMap.end() ) {
      // Did not find any, this means that this is a new disturbance.
      m_disturbances.push_back( disturbance );
      m_sitRefMap[ disturbance->getSituationReference() ] = disturbance;
      m_distMap[ disturbance->getDisturbanceID() ] = disturbance;
   } else {
      // Else we have this disturbance already, destroy the old one
      // and replace it with this
      Disturbances::iterator oldDistIt = find( m_disturbances.begin(),
                                               m_disturbances.end(),
                                               sitIt->second );
      if ( oldDistIt == m_disturbances.end() ) {
         // The disturbance container is out of sync!
         // This is bad, and should not happen.
         mc2log << error
                << "[TrafficMapInfo]"
                << " AddDisturbances."
                << " Disturbance container is out of sync!" << endl;
      } else {
         // Found old disturbance.

         // The disturbance ID could have changed so we need to remove it
         // from the id -> disturbance map.
         m_distMap.erase( (*oldDistIt)->getDisturbanceID() );
         // Remove it from the situation ref -> disturbance map 
         m_sitRefMap.erase( (*oldDistIt)->getSituationReference() );

         delete *oldDistIt;
         m_disturbances.erase( oldDistIt );
      }

      m_distMap[ disturbance->getDisturbanceID() ] = disturbance;
      m_sitRefMap[ disturbance->getSituationReference() ] = disturbance;
      m_disturbances.push_back( disturbance );
   }

}

void TrafficMapInfo::Impl::
removeDisturbance( const DisturbanceElement* disturbance ) {
   SitToDisturbanceMap::iterator sitIt =
      m_sitRefMap.find( disturbance->getSituationReference() );

   if ( sitIt == m_sitRefMap.end() ) {
      // no disturbance in this unit.
      return;
   }

   Disturbances::iterator distIt = find( m_disturbances.begin(),
                                         m_disturbances.end(),
                                         sitIt->second );

   if ( distIt == m_disturbances.end() ) {
      // The disturbance container is out of sync!
      // This is bad, and should not happen.
      mc2log << error
             << "[TrafficMapInfo]"
             << " RemoveDisturbances."
             << " Disturbance container is out of sync!" << endl;
      m_sitRefMap.erase( sitIt );
      return;
   }

   m_distMap.erase( (*distIt)->getDisturbanceID() );
   m_sitRefMap.erase( (*distIt)->getSituationReference() );
   m_disturbances.erase( distIt );
}

void TrafficMapInfo::
updateDisturbances( Disturbances& updated, const Disturbances& removed ) {
   Disturbances notAdded;
   for ( Disturbances::iterator it = updated.begin(), itEnd = updated.end();
         it != itEnd; ++it ) {
      Disturbances::value_type disturbance = *it;

      // We need a valid input situation reference
      if ( disturbance->getSituationReference().empty() ) {
         notAdded.push_back( disturbance );
         continue;
      }

      m_impl->addDisturbance( disturbance );
   }
   updated.clear();
   // destroy the disturbances that wasn't added or updated.
   STLUtility::deleteValues( notAdded );

   for ( Disturbances::const_iterator
            it = removed.begin(), itEnd = removed.end();
         it != itEnd; ++it ) {
      const Disturbances::value_type disturbance = *it;
      if ( disturbance->getSituationReference().empty() ) {
         continue;
      }
      m_impl->removeDisturbance( disturbance );
   }
}

