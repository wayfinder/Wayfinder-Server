/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "ClipUtil.h"

void
ClipUtil::clipSegment( const MC2Point& prevVertex,
                       const MC2Point& currVertex,
                       int prevInside,
                       int currInside,
                       byte currOutcode,
                       const MC2BoundingBox* bbox,
                       byte boundaryOutcode,
                       vector<byte>& resOutcodes,
                       vector<MC2Point>& resVertices )
{
   // 1) prevVertex inside, currVertex inside: output currVertex
   // 2) prevVertex inside, currVertex outside: output intersection
   // 3) prevVertex outside, currVertex outside: no output
   // 4) prevVertex outside, currVertex inside:
   //                        output currVertex and intersection
   
   if (prevInside && currInside) {
      // Case 1
      // 1) prevVertex inside, currVertex inside: output currVertex
      resVertices.push_back( currVertex );
      resOutcodes.push_back( currOutcode );
      mc2dbg8 << "Case 1: " << "(" << currVertex.getY() << ","
              << currVertex.getX()
              << ") " << endl;
   } else if (prevInside && !currInside) {
      // Case 2
      // 2) prevVertex inside, currVertex outside: output intersection
      MC2Point intersection(0,0);
      bbox->getIntersection( prevVertex.getY(), prevVertex.getX(),
                             currVertex.getY(), currVertex.getX(),
                             boundaryOutcode,
                             (int32&)intersection.getY(),
                             (int32&)intersection.getX() );
      resVertices.push_back( intersection );
      resOutcodes.push_back(
         bbox->getCohenSutherlandOutcode( intersection.getY(),
                                          intersection.getX() ));
      mc2dbg8 << "Case 2: " << "(" << prevVertex.getY() << ","
              << prevVertex.getX()
              << ") " << " % " << "("
              << currVertex.getY() << "," << currVertex.getX()
              << ") " << " -> " << "(" << intersection.getY() << ","
              << intersection.getX() << ") " << endl;
   } else if (!prevInside && currInside) {
      // Case 4
      // 4) prevVertex outside, currVertex inside:
      //                        output intersection and currVertex
      
      MC2Point intersection(0,0);
      bbox->getIntersection( prevVertex.getY(), prevVertex.getX(),
                             currVertex.getY(), currVertex.getX(),
                             boundaryOutcode,
                             (int32&)intersection.getY(),
                             (int32&)intersection.getX() );
      resVertices.push_back( intersection );
      resOutcodes.push_back(
         bbox->getCohenSutherlandOutcode( intersection.getY(),
                                          intersection.getX() ));
      resVertices.push_back( currVertex );
      resOutcodes.push_back( currOutcode );
      mc2dbg8 << "Case 4: " << "(" << prevVertex.getY() << ","
              << prevVertex.getX()
              << ") " << " % " << "(" << currVertex.getY()
              << "," << currVertex.getX()
              << ") " << " -> " << "(" << intersection.getY() << ","
              << intersection.getX() << ") "<< "("
              << currVertex.getY() << ","
              << currVertex.getX() << ") " << endl;
   }  // else (!prevInside && !currInside) 
   // Case 3
   // 3) prevVertex outside, currVertex outside: no output             
}


int
ClipUtil::clipToBoundary( const byte boundaryOutcode,
                          const MC2BoundingBox* bbox,
                          vector<MC2Point>& vertices,
                          vector<byte>& outcodes,
                          vector<MC2Point>& resVertices,
                          vector<byte>& resOutcodes  )  {
   // This method is a block in the Sutherland-Hodgeman 
   // polygon clipping pipeline.
   
   // A polygon must consist of at least three vertices.
   if (vertices.size() < 3) {
      return (false);
   }
   
   resVertices.reserve(vertices.size());
   resOutcodes.reserve(outcodes.size());
   
   // Previous outcode
   vector<byte>::const_iterator prevOcIt = outcodes.begin();
   int prevInside = (((*prevOcIt) & boundaryOutcode) == 0);
   
   // Current outcode
   vector<byte>::const_iterator currOcIt = outcodes.begin();
   ++currOcIt;
   
   // Previous vertex
   vector<MC2Point>::const_iterator prevVxIt = vertices.begin();
   // Current vertex
   vector<MC2Point>::const_iterator currVxIt = vertices.begin();
   ++currVxIt;
   for ( ; currVxIt != vertices.end(); ++currVxIt ) {
      int currInside = (((*currOcIt) & boundaryOutcode) == 0);
      
      clipSegment( *prevVxIt, *currVxIt, prevInside, currInside,
                   *currOcIt, bbox, boundaryOutcode,
                   resOutcodes, resVertices );
      
      // Update prevVxIt
      ++prevVxIt;
      
      // Update prevInside
      prevInside = currInside;
      ++currOcIt;
   }
   
   // The same thing for the last edge:
   currOcIt = outcodes.begin();
   currVxIt = vertices.begin();
   int currInside = (((*currOcIt) & boundaryOutcode) == 0);
   clipSegment( *prevVxIt, *currVxIt, prevInside, currInside,
                *currOcIt, bbox, boundaryOutcode,
                resOutcodes, resVertices );
   
   // Done with clipping boundary. Now reset the input vectors.
   vertices.clear();
   outcodes.clear();
   
   return (resVertices.size() > 2);
}

   
int
ClipUtil::clipPolyToBBoxFast( const MC2BoundingBox& bbox, 
                              vector<MC2Point>& vertices ) {
   
   uint32 nbrVertices = vertices.size();
   
   if (nbrVertices < 3) {
      return (false);
   }
   
   
   vector<byte> outcodes1;
   outcodes1.reserve(nbrVertices);
   // Calculate the outcodes.
   for ( vector<MC2Point>::const_iterator it = vertices.begin();
         it != vertices.end(); ++it ) {
      outcodes1.push_back( 
         bbox.getCohenSutherlandOutcode( it->getY(), it->getX() ) );
   }
   
   vector<byte> outcodes2;
   vector<MC2Point> vertices2;
   
   // Clip using Sutherland-Hodgeman clipping.
   // Clip against a bbox boundary and feed the output as input when
   // clipping against the next bbox boundary...
   
   // Clip to left boundary
   clipToBoundary(MC2BoundingBox::LEFT, &bbox, vertices, outcodes1,
                  vertices2, outcodes2);
   
   // Clip to right boundary
   clipToBoundary(MC2BoundingBox::RIGHT, &bbox, vertices2, outcodes2,
                  vertices, outcodes1);
   
   // Clip to top boundary
   clipToBoundary(MC2BoundingBox::TOP, &bbox, vertices, outcodes1,
                  vertices2, outcodes2);
   
   // Clip to bottom boundary
   bool retVal = 
      clipToBoundary(MC2BoundingBox::BOTTOM, &bbox, vertices2, outcodes2,
                     vertices, outcodes1);
   
   return (retVal);
}
