/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPDRAWINGCOMMON_OBJECTBOXES_H
#define MAPDRAWINGCOMMON_OBJECTBOXES_H

#include "MC2BoundingBox.h"
#include "PixelBox.h"

class DrawingProjection;

namespace MapDrawingCommon {

/**
 * Creates a PixelBox from a bounding box and projection.
 * @param box
 * @param proj
 * @return PixelBox
 */
PixelBox createPixelBox( const MC2BoundingBox& box,
                         const DrawingProjection& proj );

/**
 * Holds world boxes and their associated pixel boxes.
 */
class ObjectBoxes {
public:

   typedef std::vector< MC2BoundingBox > WorldBoxes;

   typedef WorldBoxes::size_type SizeType;

   /**
    * Adds a bounding box. It will create a matching pixel box.
    * @param proj Current projection.
    * @param worldCoordinates bounding box in world coordinates.
    */
   void addBox( const MC2BoundingBox& worldCoordinates,
                const DrawingProjection& proj ) {
      m_worldBoxes.push_back( worldCoordinates );
      m_pixelBoxes.push_back( createPixelBox( worldCoordinates, proj ) );
   }

   /**
    * Fetch world box at index. Undefined behavior if index is not inside
    * getNbrBoxes().
    * @return world box at index.
    */
   const MC2BoundingBox& getWorldBox( SizeType pos ) const {
      return m_worldBoxes[ pos ];
   }

   /**
    * Fetch pixel box at index. Undefined behavior if index is not inside
    * getNbrBoxes().
    * @return pixel box at index \c pos.
    */
   const PixelBox& getPixelBox( SizeType pos ) const {
      return m_pixelBoxes[ pos ];
   }

   /// @return Number of boxes.
   SizeType getNbrBoxes() const {
      return m_worldBoxes.size();
   }

   /// @return world boxes.
   const WorldBoxes& getWorldBoxes() const {
      return m_worldBoxes;
   }

public:
   /// World boxes.
   WorldBoxes m_worldBoxes;
   /// Matching pixel boxes for the world boxes.
   std::vector< PixelBox > m_pixelBoxes;
};

} // MapDrawingCommon

#endif // MAPDRAWINGCOMMON_OBJECTBOXES_H
