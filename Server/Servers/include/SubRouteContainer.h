/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SUBROUTECONTAINER_H
#define SUBROUTECONTAINER_H

#include "config.h"

#include <map>
#include <set>
#include "SubRoute.h"

class ServerSubRouteVector; // forward decl
class SubRouteVector;       // forward decl
class OrigDestInfo;         // forward decl
class OrigDestInfoList;     // forward decl
class DriverPref;           // forward decl
class LevelTransitObject;   // forward decl
class RouteAllowedMap;      // forward decl


typedef pair<uint32, SubRoute*> SubRoutePair;

/**
 *    Main server class for holding information about a group of
 *    SubRoutes while routing. When routing is finished, the SubRoutes
 *    will instead be put in a RouteResultVector.
 *    Entries consist of SubRoutePairs (see multimap in STL).
 *    Key is estimated cost in the node where the SubRoute ends.
 *    By using this key, two things will go faster:
 *    <ul>
 *      <li>Assigning a new best SubRoute when the previous is dequeued.</li>
 *      <li>Finding and removing SubRoutes above cutoff cost.</li>
 *    </ul>
 *    Second is a pointer to a SubRoute. 
 *
 */
class SubRouteContainer : protected multimap<uint32, SubRoute*> {
public:
   
   /**
    *    Creates an empty SubRouteContainer.
    */
   SubRouteContainer( );

   /**
    *    Creates a new SubRouteContainer and resets the member variables.
    *    @param destInfoList is used to help routing get an early cutoff.
    */
   SubRouteContainer(const OrigDestInfoList* destInfoList);

   /**
    *    Goes through all the elements and deletes the respective
    *    SubRoutes. Then deletes the elements in the container.
    */
   virtual ~SubRouteContainer();

   /**
    *    Finds and dequeues all remaining SubRoutes in the container
    *    and moves them to resultVector.
    *    @param resultVector    A Vector containing pointers
    *                           to returned SubRoutes. An empty Vector
    *                           if we are finished routing.    
    */
   void moveAllSubRoutes(SubRouteVector& resultVector,
                         ServerSubRouteVector& finishedSubRouteVector);

   /**
    *    Puts all the subroutes in this container into
    *    another one and clears this one.
    */
   void putAllInto(SubRouteContainer& other, ServerSubRouteVector* srVect);

   /**
    *    Copies all the subRoutes of this container into
    *    the other one.
    *    @param other Container to copy the routes into.
    */
   void copyAllInto(SubRouteContainer& other, ServerSubRouteVector* srVect);
   
   /**
    *    Finds and dequeues all SubRoutes ending in the same map
    *    as the currently best subroute, with the exception of the
    *    case where no destination is reached yet. In that case
    *    a map containing a destination will take preference.
    *    @param resultVector    A Vector containing pointers
    *                           to returned SubRoutes. An empty Vector
    *                           if we are finished routing.
    *    @param findDestObject  Object containing which maps we are
    *                           allowed to route on.
    *                           Omit or NULL if no restrictions.
    *    @return The mapID or MAX_UINT32 if empty.
    */
   uint32 dequeueSubRoutesFromBestMap(
      SubRouteVector& resultVector,
      LevelTransitObject* levelTransitObject = NULL,
      int level = 0);

   /**
    *    Dequeues subroutes that lead to the specified map and
    *    puts them into the resultvector. Checks if there
    *    is a cheaper one in vector already.
    *    @param resultvector The vector to put the routes in.
    *    @param mapID        The map to dequeue from.
    *    @return The number of routes added.
    */
   uint32 dequeueSubRoutesFromMap( SubRouteVector& resultVector,
                                   uint32 mapID );
   
   /**
    *    Uses multimap function 'insert' on a SubRoute pointer.
    *    Note that you have to call 'updateCutOff' and 'updateContainer'
    *    before getting information from the container again.
    *    @return True if the route was inserted.
    */
   bool checkAndInsertSubRoute(SubRoute* pSubRoute,
                               ServerSubRouteVector* pVector );

   /**
    *    Uses multimap function 'insert' on a SubRoute pointer.
    *    Note that you have to call 'updateCutOff' and 'updateContainer'
    */
   void insertSubRoute(SubRoute* pSubRoute);

   /**
    *    Inserts an OrigDestInfo as a new SubRoute into the container
    */
   void insertOrigDestInfo(const OrigDestInfo& insertInfo);

   /**
    *    Clear the SubRoutes with EstimatedCost greater than cutoff
    */
   void updateContainer(ServerSubRouteVector* srVect);
      
   /**
    *    Returns the value of m_cutOff.
    */
   inline uint32 getCutOff();

   /**
    *    Sets the value of m_cutOff if cutOff < m_cutOff
    *
    *    @param cutOff
    */
   inline void updateCutOff(uint32 cutOff);

   /**
    *    Returns the size of the SubRouteContainer
    */
   inline uint32 getSize() const;

   /**
    *    Returns the cost of the cheapest subroute in the
    *    SubRouteContainer.
    */
   inline uint32 getMinEstCost() const;

   /**
    *    Returns true if the container is empty.
    */
   inline bool empty() const;
   
   /**
    *    Returns the number of destination nodes on the
    *    supplied level.
    *    @param level The level to check.
    *    @return The number of nodes on that level.
    */
   int countNodesOnLevel(int level) const;

   /**
    * Set the allowed maps.
    *
    * @param maps The allowed maps. NULL means all maps are allowed.
    */
   inline void setAllowedMaps( RouteAllowedMap* maps );
   
private:

   /**
    *    First deletes a SubRoute in a SubRoutePair and then
    *    erases the pair.
    *
    *    @param it  iterator with the pair to delete
    */
   inline void deleteSubRoute(iterator& it);

   /**
    *    Sets the value of m_cutOff.
    *
    *    @param cutOff
    */
   inline void setCutOff(uint32 cutOff);

   /**
    *    On what distance to stop routing
    */
   uint32 m_cutOff;

   /**
    *    Set of destination maps for the route.
    *    Are used to decide which map to route on next.
    */
   set<uint32> m_destMaps;

   /**
    *    Set of mapids used when searching for the cutoff.
    *    To avoid to many visits.
    */
   set<uint32> m_mapsUsedForCutOff;

   /**
    * The allowed maps, may be NULL.
    */
   RouteAllowedMap* m_allowedMaps;
};

// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -

inline void
SubRouteContainer::deleteSubRoute(iterator& it)
{
   delete it->second;
   erase( it++ );
}

inline void
SubRouteContainer::setCutOff(uint32 cutOff)
{
   m_cutOff = cutOff;
}

inline void
SubRouteContainer::updateCutOff(uint32 cutOff)
{
   if ( cutOff < m_cutOff )
      setCutOff(cutOff);
}

inline uint32
SubRouteContainer::getCutOff()
{
   return m_cutOff;
}

inline uint32
SubRouteContainer::getSize() const
{
   return size();
}

inline bool
SubRouteContainer::empty() const
{
   return multimap<uint32, SubRoute*>::empty();
}

inline uint32
SubRouteContainer::getMinEstCost() const
{
   SubRouteContainer::const_iterator it = begin();
   if ( it != end() )
      return it->second->getEstimatedCost();
   else
      return MAX_UINT32;
}

inline void 
SubRouteContainer::setAllowedMaps( RouteAllowedMap* maps )
{
   m_allowedMaps = maps;
}

#endif



