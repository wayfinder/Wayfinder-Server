/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SUBROUTEVECTOR_H
#define SUBROUTEVECTOR_H

#include "config.h"

#include "Types.h"
#include "NodeBits.h"
#include "Connection.h" // For conversiton of costs.

#include "SubRoute.h"

#include <vector>

/**
 *    Vector containing pointers to SubRoutes. This is used as request
 *    and reply in RouteModule, and as result vector containing the
 *    SubRoutes making up the finished route.
 *    Note that there is no way of dequeueing from this class. To access
 *    members that you do not want to remain in the vector, use
 *    'getSubRouteAt' and then 'resetIndex'.
 *
 *    The SubRoutes still in the SubRouteVector when the SubRouteVector
 *    is deleted are deleted.
 *
 */
class SubRouteVector : public vector<SubRoute*> {
public:

   /**
    *    Default constructor. Creates an empty vector.
    */
   inline SubRouteVector();
   
   /**
    *    Copy constructor copies every route in the other vector.
    *    @param other The vector to copy.
    */
   SubRouteVector(const SubRouteVector& other);

   /**
    *    Swaps the contents of this vector with the other one.
    */
   void swap( SubRouteVector& other );
   
   /**
    *    Goes through the vector and deletes all the SubRoutes.
    *    The inherited destructor takes care of the elements of the vector
    */
   virtual ~SubRouteVector();

   /**
    *    Returns the offset for the origin in the first SubRoute,
    *    i.e. the origin for the route.
    *    @return The start offset from zero.
    */
   float getStartOffset() const;

   /**
    *    Returns the offset for the origin in the first SubRoute,
    *    i.e. the origin for the route as an unsigned 16 bit integer.
    *    @return The start offset from zero in parts of MAX_UINT16.
    */
   uint16 getStartOffsetUint16() const;

   /**
    *    Returns the offset for the destination in the last SubRoute,
    *    i.e. the destination for the complete route.
    *    @return The end offset from zero.
    */
   float getEndOffset() const;

   /**
    *    Returns the offset for the destination in the last SubRoute,
    *    i.e. the destination for the complete route.
    *    @return The end offset from zero.
    */
   uint16 getEndOffsetUint16() const;

   /**
    *    Returns the start direction. True if we're starting on
    *    node 0.
    */
   bool getStartDirFromZero() const;

   /**
    *    Returns the end direction from zero. True if we're 
    *    ending on node 1, i.e. the car is heading towards
    *    node 1 when it stops.
    */
   bool getEndDirFromZero() const;

   /**
    *    Returns an almost unique number for the route to
    *    be used when the route is saved in the database.
    */
   uint32 getRouteID() const;
   
   /**
    *    Inserts an element last in the vector.
    */
    void insertSubRoute(SubRoute* pSubRoute);
   
   /**
    *    Asks for the SubRoute on a certain position in the vector
    *
    *    @param index  The index in the vector to return
    *    @return       A pointer to the requested SubRoute
    */
   SubRoute* getSubRouteAt(uint32 index) const;

   /**
    *    Resets the pointer at a specified index.
    *
    *    @param resetIndex  The index in the vector to reset.
    */
   void resetIndex(uint32 index);

   /**
    *    Resets all entries in the vector
    */
   void resetAll( );

   /**
    *    Returns the number of elements in the SubRouteVector
    */
   inline uint32 getSize() const;

   /**
    *   Compares two SubRouteVectors and returns true if the cost
    *   for the first one is less than the cost for the second one.
    */
   static bool compareCosts(SubRouteVector* a,
                            SubRouteVector* b);

   /**
    *    Returns the sum of costsA for the route.
    */
   inline uint32 getTotalDistanceCm() const;

   /**
    *    Returns the sum of costsB or cost C for the route.
    *    @param withDisturbances True if disturbances should be
    *                            included in the time.
    */
   inline uint32 getTotalTimeSec(bool withDisturbances) const;

   /**
    *    Sets the index for the original destination sent to the
    *    RouteObject.
    *    @param i The index.
    */
   inline void setOriginalDestIndex(int i);

   /**
    *    Returns the index of the original destination for the
    *    SubRouteVector. Be careful. Will only be set in
    *    RouteObject::fixupRouteResult.
    *    @return The index used in RouteObject for the original
    *            destination of this SubRouteVector.
    */
   inline int getOriginalDestIndex() const;

private:

   /** Original index for destination added to RouteObject */
   int m_originalDestIndex;
   
};

// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -

inline
SubRouteVector::SubRouteVector()
{
}

inline uint32
SubRouteVector::getSize() const
{
   return size();
}

inline float
SubRouteVector::getStartOffset() const
{
   return getSubRouteAt(0)->getOrigOffset();
}

inline uint16
SubRouteVector::getStartOffsetUint16() const
{
   return (uint16)( getStartOffset() * float(MAX_UINT16) );
}

inline float
SubRouteVector::getEndOffset() const
{
   return getSubRouteAt( size() - 1 )->getDestOffset();
}

inline uint16
SubRouteVector::getEndOffsetUint16() const
{
   return (uint16)( getEndOffset() * float(MAX_UINT16) );
}

inline bool
SubRouteVector::getStartDirFromZero() const
{
   return MapBits::isNode0( getSubRouteAt(0)->getOrigNodeID());
}

inline bool
SubRouteVector::getEndDirFromZero() const
{
   return MapBits::isNode0( getSubRouteAt( size() - 1 )->getDestNodeID() );
}

inline uint32
SubRouteVector::getTotalDistanceCm() const
{
   return uint32(Connection::distCostToMeters(
      double(getSubRouteAt( size() - 1 )->getCostASum()) * 100.0)) ;
}

inline uint32
SubRouteVector::getTotalTimeSec(bool withDisturbances) const
{
   SubRoute* lastSubRoute = getSubRouteAt( size() - 1);
   uint32 cost = 0;
   if ( withDisturbances ) {
      cost = lastSubRoute->getCostCSum();
   } else {
      cost = lastSubRoute->getCostBSum();
   }
   return uint32(Connection::timeCostToSec( double(cost) ) );
}

inline int
SubRouteVector::getOriginalDestIndex() const
{
   return m_originalDestIndex;
}

inline void
SubRouteVector::setOriginalDestIndex(int i)
{
   m_originalDestIndex = i;
}


#endif
