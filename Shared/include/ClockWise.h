/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CLOCKWISE_H
#define CLOCKWISE_H

#include "config.h"
#include <utility>
#include "MC2Coordinate.h"

namespace GfxUtil {

// Eh. Private.
namespace {
template<class ITERATOR> class ThreeCoords {
public:
   
   ThreeCoords( const ITERATOR& prev,
                const ITERATOR& middle,
                const ITERATOR& next )
         : m_prev( prev ),
           m_middle( middle ),
           m_next( next ) {}

   ITERATOR m_prev;
   ITERATOR m_middle;
   ITERATOR m_next;
   
};

template<class ITERATOR>
class ThreeCoords<ITERATOR>
getNorthestCorner3( const ITERATOR& begin,
                   const ITERATOR& end )
{
   const ThreeCoords<ITERATOR> invalid(end, end, end);
   if ( begin == end ) {
      mc2dbg8 << "[getNorthestCorner]: Empty range" << endl;
      // Empty range
      return invalid;
   }

   // Do stuff
   ITERATOR it = begin;
   ++it;
   if ( it == end ) {
      // One coordinate in range
      mc2dbg8 << "[getNorthestCorner]: One coordinate in range" << endl;
      return invalid;
   }

   {
      ITERATOR next = it;
      ++next;
      if ( next == end ) {
         // Only two coordinates in range
         mc2dbg8 << "[getNorthestCorner]: Only two coordinates range" << endl;
         return invalid;
      }
   }

   ITERATOR maxYItPrev = begin; // Will be invalid if the first one is the one
   ITERATOR maxYIt     = begin; // One before "it"

   ITERATOR prev = begin; // The iterator before "it".
   ITERATOR last = begin; // The last iterator, since we canno step backwards
   
   // Go through the coordinates and select the one that is most
   // to the north. If there are more than one, compare lon too.
   // Keep prev lagging behind "it" and save the last iterator to be
   // able to set prev to that if it hasn't moved.
   //
   // Try to find the coordinate most to the east of the northest ones.
   for ( ; it != end; prev = it, ++it ) {
      if ( getCoordY( *maxYIt ) < getCoordY( *it ) ||
          ( ( getCoordY( *maxYIt ) == getCoordY( *it ) ) &&
            ( getCoordX( *it ) >= getCoordX( *maxYIt ) ) ) ) {
         maxYItPrev  = prev;
         maxYIt      = it;
      }
      last = it;
   }
   if ( maxYIt == begin ) {
      // Has not moved. Take the last one before end.
      mc2dbg8 << "[getNorthestCorner]: has not moved" << endl;
      maxYItPrev = last;
      
      // We cannot go backwards, so the prev better not be
      // same as middle. The loop above should take care of that
      // if the polygon is manually closed, but not if it contains
      // duplicate end points.
      MC2_ASSERT( *maxYIt != *maxYItPrev );
   }
   ITERATOR maxYItNext = maxYIt;
   ++maxYItNext;
   if ( maxYItNext == end ) {
      // Past end
      mc2dbg8 << "[getNorthestCorner]: past end" << endl;
      maxYItNext = begin;
      // Handle that the polygon is manually closed, 
      // i.e. first and last coordinate is same.
      if ( *maxYIt == *maxYItNext ) {
         ++maxYItNext;
      }
   }
   return ThreeCoords<ITERATOR>( maxYItPrev, maxYIt, maxYItNext );
}
}

template<class ITERATOR>
ITERATOR
getNorthestCorner( const ITERATOR& begin,
                   const ITERATOR& end )
{
   return getNorthestCorner3(begin, end).m_middle;
}

/**
 *    Check if the (closed) polygon described by the range
 *    is clockwise or not.
 *    @return -1 if the range is smaller than three elements or
 *            there is something wrong with the polygon, e.g.
 *            two consecutive points are the same where clockwiseness
 *            is measured. <br />
 *            1 if CW      <br />
 *            0 if CCW    
 */
template<class ITERATOR>
int isClockWise( const ITERATOR& begin,
                 const ITERATOR& end )
{
   ThreeCoords<ITERATOR> coords = getNorthestCorner3( begin, end );
   
   if ( coords.m_prev == end || coords.m_middle == end || coords.m_next == end ) {
      // To few coordinates
      return -1;
   }

#if 0 
   mc2dbg8 << "[isClockWise]: "
           << coords.m_prev << ", " << coords.m_middle  << ", "
           << coords.m_next << endl;
#endif
  
   // Could this be isLeft?
   int32 x1 = getCoordX( *coords.m_prev ) - getCoordX( *coords.m_middle );
   int32 y1 = getCoordY( *coords.m_prev ) - getCoordY( *coords.m_middle );
   int32 x2 = getCoordX( *coords.m_next ) - getCoordX( *coords.m_middle );
   int32 y2 = getCoordY( *coords.m_next ) - getCoordY( *coords.m_middle );

   int64 crossprod = ((int64)x1)*y2 - ((int64)y1)*x2;
   
   if (crossprod > 0) {
      return 1; // Clockwise
   } else if (crossprod < 0) {
      return 0; // Counterclockwise
   } else {
      return -1; // This polygon is corrupt
   }
}

}

#endif
