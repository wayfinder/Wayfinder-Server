/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERVERSUBROUTEVECTOR_H
#define SERVERSUBROUTEVECTOR_H

#include "config.h"
#include <map>

#include "SubRouteVector.h"

/**
 *    Vector containing pointers to SubRoutes.
 *    The class is used by RouteSender and RouteObject to manipulate SubRoutes
 *    towards what will be the finished route.
 *    The class inherits from SubRouteVector
 *    Useful functions inherited from SubRouteVector are:
 *    void resetIndex(uint32 index) : Resets the pointer at a specified index.
 *    uint32 getSize() const : Returns the number of elements in the vector
 *    SubRoute* getSubRouteAt(uint32 index) const :
 *              Asks for the SubRoute on a certain position in the vector
 *    Note that there is no way of dequeueing from this class. To access
 *    members that you do not want to remain in the vector, use
 *    'getSubRouteAt' and then 'resetIndex'
 * 
 */
class ServerSubRouteVector : public SubRouteVector {
public:
   
   /**
    *    Constructor.
    *    Creates a new SubRouteVector and initializes the member
    *    functions.
    *    @param nbrDests Number of destinations to give routes
    *                    to. Default is 0.
    */
   ServerSubRouteVector(uint32 nbrDest = 0);

   /**
    *    Deletes m_destIndexArray.
    *    The inherited destructor (SubRouteVector) takes care of
    *    the SubRoutes and the pointers.
    */
   virtual ~ServerSubRouteVector();

   /**
    *    Returns a map with the nodeid as key containing the SubRoutes
    *    on the supplied map. Temporary function for testing if it is 
    *    good to resend all the old subroutes to a certain map again,
    *    since they disappear from the SubRouteContainer.
    *    @param mapID The mapID to check for.
    *    @param subRoutes The subroutes that end on the map. NB! No copies!
    *
    */
   void getSubRoutesToMap(uint32 mapID,
                          map<uint32, SubRoute*>& subRoutes);
   
   /**
    *    Inserts an element last in the vector.
    *    Also tells the SubRoute what index it is.
    *
    *    @param pSubRoute  The pointer to be added
    *    @param isDest     True if SubRoute ends in a destination
    *                      (default: false)
    *    @param isEnd      True if destination is final destination
    *                      (default: false)
    */
   void insertSubRoute(SubRoute* pSubRoute,
                       bool      isDest = false);

   /**
    *    Returns the greatest of the costs to get to a destination
    *    if we should return the routes to all destinations. Returns
    *    the cheapest cost to one destination if this is the last 
    *    part of the (via)route. Currently contains a cheat that
    *    removes a lot of the dests if we have too few reached
    *    destinations. This way we will get a cutoff faster.
    *    MAX_UINT32 if we have not reached all dests yet.
    */
   uint32 getCutOff() const;

   /**
    *    Returns true if the vector contains a subroute with
    *    cheaper or same real cost to the same destination node.<br>
    *    FIXME: Create a separate datastructure for finding cheaper.
    *    @param subRoute The SubRoute to compare to.
    *    @return True if there is a cheaper SubRoute in the vector.
    */
   const SubRoute* containsCheaperThan(const SubRoute* subRoute) const;

   /**
    *    Adds all the SubRoutes from sourceVector, except the last one.
    *    SourceVector is not changed, so it still contains the
    *    pointers to the SubRoutes.
    *    @param sourceVector  Where the SubRoutes come from.
    *    @return  A pointer to the last SubRoute.
    */
   SubRoute* merge( ServerSubRouteVector* sourceVector );

   /**
    *    Function for finding the SubRoutes that makes up a route
    *    when routing is finished. This functions can only be called
    *    once since the SubRoutes that are added to the result vector
    *    are removed from this vector.
    *
    *    @param  index  The requested index of the m_destIndexArray
    *                   for which we want the route.
    *    @return        A new SubRouteVector with the SubRoutes making
    *                   up the route.
    */
   ServerSubRouteVector* getResultVector(uint32 index);
   
   /**
    *    Returns the number of elements in m_destIndexArray
    */
   inline uint32 getNbrDest() const;

   /**
    *    Returns the index of the SubRouteVector preceding
    *    in a SubRouteVectorVector.
    */
   inline uint32 getPrevSubRouteVectorIndex() const;

   /**
    *    Sets the value of m_prevSubRouteVectorIndex
    */
   inline void setPrevSubRouteVectorIndex(uint32 index);

   inline uint32* getDestIndexArray() const;

   inline void setExternalCutOff(uint32 cutOff);

   /**
    *     Sets the "don't" delete flag for the SubRoute
    *     at index <code>index</code>.
    *     @param index The SubRoute that has been moved to
    *                  another place.
    */
   void resetIndex(uint32 index);

   /**
    *     Sets the owned by me flag to false for all SubRoutes.
    */
   void resetAll();

   /**
    *     Returns the number of routes that has destination
    *     on the supplied level.
    *     XXX: This method could be optimized.
    *     @param level The level to look for.
    *     @return The number of nodes on the level <code>level</code>.
    */
   int countNodesOnLevel( int level ) const;
   
private:

   /**
    *    Initialization function, to be used after constructor.
    *    Not necessary if m_destIndexArray is not to be used.
    *
    *    @param  nbrDest  The number of destinations that needs to be
    *                     reached before routing is finished.
    *                     Via routing uses several destinations.
    *                     The number of different SubRouteVectors that
    *                     getResultSubrouteVector can return is nbrDest.
    */
   void initDests(uint32 nbrDest);

   /**
    *    Inserts an index of a SubRoute containing a destination into
    *    the m_destIndexArray. If that destination already exists, it will
    *    replace if cost of new SubRoute is lower.
    */
   void insertDestSubRoute(uint32 index);

   /**
    *    The index of the SubRouteVector preceding
    *    in a SubRouteVectorVector.
    */
   uint32 m_prevSubRouteVectorIndex;

   /**
    *    An array of indeces that contains SubRoutes with destinations.
    *    This array is initialized in 'init' and this function must
    *    hence have been called before tampering with this member.
    */
   uint32 *m_destIndexArray;

   /**
    *    The size of m_destIndexArray. Set when calling 'init'.
    */
   uint32 m_nbrDest;

   /**
    *    The allocated size of the m_destIndexArray.
    */
   uint32 m_maxNbrDest;

   /**
    *    Cut off given by lower level routing.
    */
   uint32 m_externalCutOff;

   /**
    *    Vector of bools. If m_ownedByMe[index] is true
    *    the SubRoute at the corresponding index should be deleted
    *    when this ServerSubRouteVector is deleted.
    */
   vector<bool> m_ownedByMe;
};

/**
 *    Class for returning a number of SubRouteVectors from
 *    RouteSender to RouteObject.
 */
class ServerSubRouteVectorVector : public vector<ServerSubRouteVector*> {
public:
   
   /**
    *   Deletes all ServerSubRouteVectors in this vector.
    */
   ~ServerSubRouteVectorVector();
   
};

// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -

inline uint32
ServerSubRouteVector::getNbrDest() const
{
   return m_nbrDest;
}

inline uint32
ServerSubRouteVector::getPrevSubRouteVectorIndex() const
{
   return m_prevSubRouteVectorIndex;
}

inline void
ServerSubRouteVector::setPrevSubRouteVectorIndex(uint32 index)
{
   m_prevSubRouteVectorIndex = index;
}


inline uint32*
ServerSubRouteVector::getDestIndexArray() const
{
   return m_destIndexArray;
}

inline void
ServerSubRouteVector::setExternalCutOff(uint32 cutOff) {
   m_externalCutOff = cutOff;
}

#endif

