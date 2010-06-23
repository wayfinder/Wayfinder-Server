/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POLYGONDEFECTS_H 
#define POLYGONDEFECTS_H 

#include "config.h"
#include <iterator>
#include <vector>
#include "GnuPlotDump.h"
#include "InsideUtil.h"

namespace GfxUtil {

/**
 *    Remove remove all middle points
 *    of angles that neither left nor right, i.e. 180 or 0 degrees,
 *    or duplicate coordinates.
 */
template <class VECTOR, class ITERATOR>
void
removeSomePolygonDefects( VECTOR& output,
                          const ITERATOR& begin,
                          const ITERATOR& end )
{
   output.clear();
   back_insert_iterator<VECTOR> outIt( output );

   if ( begin == end ) {
      mc2dbg8 << "[removeSomePolygonDefects]: Empty range" << endl;
      // Empty range
      return;
   }

   // Do stuff
   ITERATOR prev = begin;
   
   ITERATOR middle = prev;
   ++middle; 

   if ( middle == end ) {
      // One coordinate in range
      mc2dbg8 << "[removeSomePolygonDefects]: One coordinate in range" << endl;
      return;
   }

   ITERATOR next = middle;
   ++next;
   if ( next == end ) {
      // Only two coordinates in range
      mc2dbg8 << "[removeSomePolygonDefects]: Only two coordinates range" << endl;
      return;
   }


   // We have at least three coordinates.
   bool firstTime = true;
   ITERATOR stopIt = next;
   ITERATOR last = end; 

   bool stupid = false;
 
   // Placeholder for first coordinate.
   *outIt++ = *begin;
   
   while ( firstTime || next != stopIt ) {
      firstTime = false;
      // Handle wrapping.
      if ( next == end ) {
         next = begin;
         // Also store the last coordinate in the original.
         last = middle;
      }
      MC2_ASSERT( middle != end );



      if ( InsideUtil::isLeft( *prev, *middle, *next ) == 0 ) {
         // Is neither left or right.
         // The middle coordinate is bad.
         mc2dbg8 << "[removeSomePolygonDefects]: Defects found." << endl;
         stupid = true;
      } else {
         // Left or right could be determined. Good.
         *outIt++ = *middle;
         
         // Step forward.
         prev = middle;
      }
      
      middle = next;
      ++next;
   }
   MC2_ASSERT( last != end );

   // Make sure the coordinates are in the same order as before if nothing
   // has been removed.
   if ( output.size() >= 2 ) {
      output.front() = output.back();
      output.pop_back();
   } else {
      // Better to have an empty vector than having a stupid polygon.
      output.clear();
      MC2_ASSERT( stupid );
      // Blank and stupid looking!
   }
     
   if ( stupid && ! output.empty() ) {
      // There may still be stupid coordinates. Run again until no stupidness found.
      VECTOR tmp;
      tmp.swap( output );
      removeSomePolygonDefects( output, tmp.begin(), tmp.end() );
   }

   // Maintain manually closed polygons.
   if ( *begin == *last && !output.empty() && output.front() != output.back() ) {
      output.push_back( output.front() ); 
   }

   if ( stupid ) { 
      if ( false ) {
         mc2log << "[removeSomePolygonDefects]: Dumping defect poly" << endl;
         cout << "cat > after.gnu << EOF" << endl;
         cout << GnuPlotDump::gp_vec_dump( output.begin(),
                                           output.end() ) << endl;
         cout << "EOF" << endl;
         cout << "cat > before.gnu << EOF" << endl;
         cout << GnuPlotDump::gp_vec_dump( begin, 
                                           end ) << endl;
         cout << "EOF" << endl;
         cout << "gnuplot" << endl;
         cout << "plot \"before.gnu\" with vectors, "
              << "\"after.gnu\" with vectors";
         cout << endl << flush;
      }
   } else {
      // No stupidness found, so double check that all is OK.
      VECTOR orig( begin, end );
      MC2_ASSERT( output == orig );
   }

}

}

#endif
