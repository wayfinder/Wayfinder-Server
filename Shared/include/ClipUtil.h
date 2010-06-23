/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CLIP_UTIL_H
#define CLIP_UTIL_H

#include "config.h"

#include <vector>
#include "MC2BoundingBox.h"
#include "MC2Point.h"
#include "MC2Coordinate.h"

class ClipUtil {
public:

   template<class ITERATOR>
      static int clipPolyToBBoxFast( const MC2BoundingBox& bbox,
                                     vector<MC2Coordinate>& result,
                                     ITERATOR begin,
                                     ITERATOR end ) {
      vector<MC2Point> pointVec;
      pointVec.clear();
      {
         for ( ITERATOR it = begin;
               it != end;
               ++it ) {
            pointVec.push_back( MC2Point( it->lon, it->lat) );
         }
      }
      int resultBool = clipPolyToBBoxFast( bbox, pointVec );
      if ( resultBool ) {
         result.clear();
         result.reserve( pointVec.size() );
         for( vector<MC2Point>::iterator it = pointVec.begin();
              it != pointVec.end();
              ++it ) {
            result.push_back( MC2Coordinate( it->getY(), it->getX() ) );
         }
      }
      return resultBool;
   }
                            
   
   /**
    *    Reduces the number of vertices outside the boundingbox and
    *    computes the Cohen-Sutherland outcodes for each of those
    *    vertices.
    *    <br />
    *    The XYHELPER is a class that should have the functions
    *    getX(POINT_SEQUENCE::value_type), getY(POINT_SEQUENCE::value_type)
    *    and getX(ITERATABLE::value_type), getY(ITERATABLE::value_type).
    *    If the value types are the same, then just two functions are
    *    needed.
    *    @see GfxUtility.
    *
    *    @param   bbox              The boundingbox to reduce the polygon.
    *    @param   vertices          The input polygon.
    *    @param   reducedVertices   [OUT] The resulting reduced polygon.
    *    @param   outcodes          The Cohen-Sutherland outcodes of the   
    *                               reduced vertices are added to this 
    *                               vector<byte> if not NULL.
    *    @return  True if the vectors got filled with any vertices / 
    *             outcodes, false otherwise.
    */
   template<class XYHELPER, class ITERATABLE, class POINT_SEQUENCE >
      inline static bool reduceByBBox( const XYHELPER& xyhelper,
                                       const MC2BoundingBox& bbox,
                                       const ITERATABLE& vertices,
                                       POINT_SEQUENCE& reducedVertices,
                                       vector<uint8>* outcodes ) {
      if ( vertices.size() < 3 ) {
         return false;
      }
      
      // Initialize the vertice and outcode vector.
      uint32 prevOutcode;
      uint32 firstOutcode;
      
      typename ITERATABLE::value_type currVertex;
      typename POINT_SEQUENCE::value_type nextVertex;
      // Add all of the polygon
      typename POINT_SEQUENCE::const_iterator it = vertices.begin();
      currVertex = *it;
      prevOutcode = firstOutcode =
         bbox.getCohenSutherlandOutcode(xyhelper.getY(currVertex),
                                        xyhelper.getX(currVertex) );
      // Add the first one.
      reducedVertices.push_back( currVertex );
      if ( outcodes ) {
         outcodes->push_back( prevOutcode );
      }
      
      ++it;
      currVertex = *it;
      uint32 currOutcode = 
         bbox.getCohenSutherlandOutcode(xyhelper.getY(currVertex),
                                        xyhelper.getX(currVertex) );
      
      
      ++it;
      for ( ; it != vertices.end(); ++it ) {
         nextVertex = *it;
         byte nextOutcode =
            bbox.getCohenSutherlandOutcode(xyhelper.getY(nextVertex),
                                           xyhelper.getX(nextVertex));
         
         if ((prevOutcode & currOutcode & nextOutcode) == 0) {
            reducedVertices.push_back( currVertex );
            if ( outcodes ) {
               outcodes->push_back( currOutcode );
            }
            prevOutcode = currOutcode;
         }
         currOutcode = nextOutcode;
         currVertex = nextVertex;
      }
      
      // Check the last one.
      if ((prevOutcode & currOutcode & firstOutcode) == 0) {
         reducedVertices.push_back( currVertex );
         if ( outcodes ) {
            outcodes->push_back( currOutcode );
         }
      }
      
      if ( xyhelper.getX(reducedVertices.back()) !=
           xyhelper.getX(reducedVertices.front()) ||
           xyhelper.getY(reducedVertices.back()) !=
           xyhelper.getY(reducedVertices.front()) ) {
         // Add first coordinate last again.
         reducedVertices.push_back( reducedVertices.front() );
      }
      
      
      // If we end up with less than three coordinates then it's not a
      // valid closed polygon anymore.
      bool retVal;
      if (reducedVertices.size() < 3) {
         if ( outcodes ) {
            outcodes->clear();
         }
         retVal = false;
      } else {
         retVal = true;
      }
      
      return (retVal);
   }
   
private:

   static void clipSegment( const MC2Point& prevVertex,
                            const MC2Point& currVertex,
                            int prevInside,
                            int currInside,
                            byte currOutcode,
                            const MC2BoundingBox* bbox,
                            byte boundaryOutcode,
                            vector<byte>& resOutcodes,
                            vector<MC2Point>& resVertices );
   
   
   static int clipToBoundary( const byte boundaryOutcode,
                              const MC2BoundingBox* bbox,
                              vector<MC2Point>& vertices,
                              vector<byte>& outcodes,
                              vector<MC2Point>& resVertices,
                              vector<byte>& resOutcodes  );

   
   int static clipPolyToBBoxFast( const MC2BoundingBox& bbox, 
                                  vector<MC2Point>& vertices );

};


#endif

