/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "StreetTextPlacer.h"
#include "GfxConstants.h"
#include "GfxUtility.h"
#include "Math.h"
#include <cmath>
#include <algorithm>

namespace {

/**
 * Calculates the length of a polyline.
 */
float64 lengthOfPolyline( const vector<MC2Coordinate>& line ) {
   float64 sum = 0.0;
   for ( size_t i = 0; i < line.size()-1; ++i ) {
      sum += sqrt( GfxUtility::squareP2Pdistance_linear( line[ i ], 
                                                         line[ i+1 ] ) );
   }
   return sum;
}

/**
 *    Calculates the drawing angles for the text.
 *
 *    @see GfxUtility::convertToDrawAngle.
 *
 *    @param textOut Outparameter. Vector containing positions and
 *                   angles of the letters in the text placed out.
 */
void convertToDrawAngle( vector<GfxDataTypes::textPos>& textOut ) {
   for(int32 i = 0; i < int32(textOut.size()); i++)
      textOut[i].angle = GfxUtility::convertToDrawAngle(textOut[i].angle);
}

/**
 * Positions the letters along a polyline. This function will only set up
 * the lat and lon for the letters, not the angle. The positions will be on
 * the polyline, the distance between two letters along the polyline is
 * determined by the letter widths.
 *
 * @param offset Length from first vertice to position to place first letter,
 *               in meters.
 */
bool 
positionLettersAlongStreet( const vector<GfxDataTypes::textPos>::iterator& textPos,
                            const vector<GfxDataTypes::textPos>::iterator& textEnd,
                            const vector<MC2Coordinate>::const_iterator& verticesPos, 
                            const vector<MC2Coordinate>::const_iterator& verticesEnd, 
                            const vector<GfxDataTypes::dimensions>::const_iterator& dimPos,
                            const vector<GfxDataTypes::dimensions>::const_iterator& dimEnd,
                            float64 offset ) {
   if ( textPos == textEnd ) { // nothing more to do
      return true;
   }
   if ( verticesPos == verticesEnd || 
        verticesPos+1 == verticesEnd ) { // out of vertices! fail
      return false;
   }

   MC2Coordinate vertex1 = *verticesPos;
   MC2Coordinate vertex2 = *(verticesPos+1);
   float64 segmentDistance = 
      sqrt(GfxUtility::squareP2Pdistance_linear( vertex1, vertex2 ));

   if ( offset >=  segmentDistance ) {
      return positionLettersAlongStreet( textPos, textEnd,
                                         verticesPos+1, verticesEnd,
                                         dimPos, dimEnd,
                                         offset-segmentDistance );
   }
   else {
      // place the first letter
      float64 relativeDist = offset/segmentDistance;
      (*textPos).lat = vertex1.lat + 
         static_cast<int32>( (vertex2.lat-vertex1.lat)*relativeDist );
      (*textPos).lon = vertex1.lon + 
         static_cast<int32>( (vertex2.lon-vertex1.lon)*relativeDist );

      // place the rest
      if ( dimPos + 1 != dimEnd ) {
         float64 distBetweenLetters = 
            ((*dimPos).width/2.0+(*(dimPos+1)).width/2.0)*GfxConstants::MC2SCALE_TO_METER;
         return positionLettersAlongStreet( textPos+1, textEnd,
                                            verticesPos, verticesEnd,
                                            dimPos+1, dimEnd,
                                            offset+distBetweenLetters );
      }
      else {
         return true;
      }
   }
}

/**
 * Sets up the angles for positioned letters.
 */
void setLetterAngles( vector<GfxDataTypes::textPos>& textOut ) {
   if ( textOut.size() > 1 ) {
      // The first and last letter are directed towards their
      // only neighbor.
      textOut[0].angle =
         GfxUtility::getAngleFromNorth( MC2Coordinate( textOut[0].lat,
                                                       textOut[0].lon ),
                                        MC2Coordinate( textOut[1].lat,
                                                       textOut[1].lon ) );
      
      int lastIndex = textOut.size()-1;
      textOut[lastIndex].angle =
         GfxUtility::getAngleFromNorth( MC2Coordinate( textOut[lastIndex-1].lat,
                                                       textOut[lastIndex-1].lon ),
                                        MC2Coordinate( textOut[lastIndex].lat,
                                                       textOut[lastIndex].lon ) );

      // The other letters are directed along the line between their
      // two neighbors.
      for ( int i = 1; i < lastIndex; ++i ) {
         textOut[i].angle = 
            GfxUtility::getAngleFromNorth( MC2Coordinate( textOut[i-1].lat,
                                                          textOut[i-1].lon ),
                                           MC2Coordinate( textOut[i+1].lat,
                                                          textOut[i+1].lon ) );
      }
   }
   else {
      // only one letter, place it horizontally
      textOut[0].angle = M_PI/2;
   }
   
}

/**
 * Checks if the text is mostly upside down.
 */
bool upsideDown( const vector<GfxDataTypes::textPos>& textOut ) {

   // The optimal angle is M_PI/2.0 (normal horizontal left to right
   // text. Angles deviating more than M_PI/2.0 from that in either
   // direction is considered to be upside down. We calculate the average
   // of these angle differences for all letters and consider the
   // whole string to be upside down if the average is a deviation larger 
   // than M_PI/2.0.

   double sum = 0.0;
   for ( size_t i = 0; i < textOut.size(); ++i ) {
      sum += GfxUtility::angleDifference( textOut[i].angle, M_PI/2.0 );
   }
   return ( sum/textOut.size() ) > M_PI/2.0;
}

/**
 *   Used to check if a street is almost straight.
 *
 *   @param text     The text to check.
 *   @param maxDist  The maximum distance the text is allowed to deviate
 *                   from the line between first and last letter.
 */
bool almostStraightStreet( const vector<GfxDataTypes::textPos>& text,
                           double maxDist ) {
   int32 startLat = text.front().lat;
   int32 startLon = text.front().lon;
   int32 endLat = text.back().lat;
   int32 endLon = text.back().lon;

   for ( size_t i = 1; i < text.size()-1; ++i ) {
      float64 cosLat = cos( (2*M_PI/ POW2_32 * text[i].lat));
      if ( sqrt(GfxUtility::closestDistVectorToPoint( startLon, startLat,
                                                      endLon, endLat,
                                                      text[i].lon, text[i].lat,
                                                      cosLat )) > maxDist ) {
         return false;
      }
   }

   return true;
}

/**
 *   Places out som of the letters directly after each other, 
 *   i.e. they all have the same angle.
 *
 *   @param startLetter The first letter to be placed out
 *   @param stopLetter  The last letter to be placed out
 *   @param startLat    MC2-coordinates for the lower left corner 
 *                      of startLetter
 *   @param startLon    MC2-coordinates for the lower left corner 
 *                      of startLetter
 *   @param angle       The common angle (from the north direction) 
 *                      of the letters written out. 
 *   @param dim         Dimensions of the rectangles, in MC2-units
 *   @param textOut     Outparameter. Vector containing positions 
 *                      and angles of the letters in the text 
 *                      placed out.
 *   @return            True if everything went ok, false otherwise
 */
bool placeStraightText( int32 startLetter, 
                        int32 stopLetter,
                        int32 startLat, 
                        int32 startLon,
                        float64 angle, 
                        const vector<GfxDataTypes::dimensions>& dim,
                        vector<GfxDataTypes::textPos>& textOut ) {
   if ((startLetter > stopLetter) || (startLetter < 0) ||
       (startLetter > int32(dim.size() - 1)) ||(stopLetter < 0) ||
       (stopLetter > int32(dim.size() - 1))) {
      return false;
   }

   int32 distance = 0;
   float64 cosLat = cos( (2*M_PI/ POW2_32 * startLat));
   textOut[startLetter].lat = startLat;
   textOut[startLetter].lon = startLon;
   textOut[startLetter].angle = angle;
   for (uint16 pos = startLetter + 1; pos <= stopLetter; pos++) {
      distance += dim[ pos - 1 ].width;
      textOut[pos].lat = startLat + int32(distance*cos(angle));
      textOut[pos].lon = startLon + int32(distance*sin(angle)/cosLat);
      textOut[pos].angle = angle;
      cosLat = cos( (2*M_PI/ POW2_32 * (textOut[pos].lat/2 + startLat/2)));
   }
   return true;
}

/**
 * This function moves each position in textOut to what would be the
 * lower left of its letter if it was placed centered at textOut's current
 * position. For a regular unrotated letter this means moving the position
 * half the letter width to the left, and half the font height downwards.
 * (The reason for using the font height rather than the letter height is
 * that we want vertically bottom aligned text)
 *
 * The math involved is pretty basic trigonometry, here is an illustration
 * of the variables involved (the point is moved from A to B):
 *
 *           North
 *            / \
 *             |
 *     +-------+--------+
 *     |       |        |
 *     |       |        |
 *     |       |        |
 *     |       |        |
 *     |       |- v     |
 *     |       | \      |
 *   - |       A--------+----> Direction of the letter
 *   | |      /|_|      |
 * y | |     / |        |
 * d | |    /  |        |
 * e | |   /\__|        |
 * l | |  / w  |        |
 * t | | /     |        |
 * a | |/      |        |
 *   - B-------+--------+
 *     |-------|
 *      xdelta
 *   
 * The direction to move the point is v + pi/2 + w. It should be moved
 * the length of the hypotenuse in the triangle above.
 */
void convertToDrawPositions( const vector<GfxDataTypes::dimensions>& dim,
                             vector<GfxDataTypes::textPos>& textOut,
                             double ydelta ) {
   for ( size_t i = 0; i < textOut.size(); ++i ) {
      double xdelta = dim[ i ].width/2.0;
      double hypotenuse = sqrt( ydelta*ydelta + xdelta*xdelta );
      double w = asin( xdelta / hypotenuse );
      double v = textOut[i].angle;
      double direction = v+M_PI/2.0+w;
      
      float64 cosLat = cos( GfxConstants::invRadianFactor * textOut[i].lat );
      textOut[i].lat += static_cast<int32>( hypotenuse * cos( direction ) );
      textOut[i].lon += static_cast<int32>( hypotenuse * sin( direction )/cosLat );
   }
}

}

bool 
StreetTextPlacer::placeStreetText( const MC2Coordinate* streetStart,
                                   const MC2Coordinate* streetEnd,
                                   const vector<GfxDataTypes::dimensions>& dim,
                                   vector<GfxDataTypes::textPos>& textOut ) const {
   // place the vertices in a vector so we can easily reverse it if needed
   vector<MC2Coordinate> vertices;
   for ( const MC2Coordinate* pos = streetStart; pos != streetEnd; ++pos ) {
      vertices.push_back( *pos );
   }

   textOut.clear();

   // An empty string is considered not to be placeable, better safe than sorry
   if ( dim.empty() ) {
      return false;
   }

   if ( vertices.empty() ) {
      mc2log << warn << "[StreetTextPlacer::placeStreetText]: Less than one coordinate - "
         "avoiding more trouble" << endl;
      return false;
   }

   textOut.resize( dim.size() );
   float64 widthSum = 0;
   for ( size_t i = 0; i < textOut.size(); ++i) {
      GfxDataTypes::textPos p;
      p.lat = 0;
      p.lon = 0;
      p.angle = 0;
      textOut[ i ] = p;
      // calculate the namewidth in MC2-units
      widthSum += dim[ i ].width;
   }

   widthSum *= GfxConstants::MC2SCALE_TO_METER;

   float64 wholeStreetLength = lengthOfPolyline( vertices );


   // if the text width ( widthSum ) is larger than the whole street, then
   // bale out.
   if ( wholeStreetLength < widthSum){
      return false;
   }


   /*
    *
    *        <-------- wholeStreetLength -------->
    *        |                                   |
    *        |----------------|------------------|
    *                      center
    *
    *        "A B C D E F G "
    *        <-- widthSum -->
    *
    *  Calculate start offset for the placement of the letters.
    *  The text placement centered in the line would be:
    *  wholeStreetLength / 2 - widthSum / 2 =>  ( wholeStreetLength - widthSum )/2
    *   ^ line center           ^ text center
    */
   float64 offset = ((wholeStreetLength-widthSum)/2.0);

   /*
    * Since we place out positions at what we consider to be the center
    * of the letters, and the offset we've calculated so far is where we
    * want the text to start, lets add half the width of the first letter
    * to the offset.
    */
   offset += (dim[0].width*GfxConstants::MC2SCALE_TO_METER)/2.0;

   // place the letters along the street
   if ( !positionLettersAlongStreet( textOut.begin(), 
                                     textOut.end(),
                                     vertices.begin(), 
                                     vertices.end(),
                                     dim.begin(),
                                     dim.end(),
                                     offset ) ) {
      return false;
   }

   // set the angles for the letters
   setLetterAngles( textOut );

   if ( upsideDown( textOut ) ) {
      reverse( vertices.begin(), vertices.end() );

      // try again
      if ( !positionLettersAlongStreet( textOut.begin(), 
                                        textOut.end(),
                                        vertices.begin(), 
                                        vertices.end(),
                                        dim.begin(),
                                        dim.end(),
                                        offset ) ) {
         return false;
      }
      
      // set the angles for the letters
      setLetterAngles( textOut );
   }

   // if there are turns which are too sharp, abort
   for ( size_t i = 0; i < textOut.size()-1; ++i ) {
      double diff = GfxUtility::angleDifference( textOut[ i ].angle,
                                                 textOut[ i+1 ].angle );

      const double LARGEST_ALLOWED_ANGLE_DIFF_FOR_STREET_LETTERS = M_PI/3.0;
      if ( diff > LARGEST_ALLOWED_ANGLE_DIFF_FOR_STREET_LETTERS )
         return false;
   }

   // calc average letter height
   int32 heightSum = 0;
   for ( size_t i = 0; i < dim.size(); ++i ) {
      heightSum += dim[ i ].height;
   }
   double avgHeight = static_cast<double>(heightSum)/dim.size();

   // So far we have set the positions along the street,
   // but when drawing text we want the position for each letter
   // to represent the lower left corner of the letter.
   convertToDrawPositions( dim, textOut, avgHeight/2 );

   // draw almost straight text as straight
   if ( almostStraightStreet( textOut, avgHeight/3 ) ) {
      double angle = 
         GfxUtility::getAngleFromNorth( MC2Coordinate( textOut.front().lat,
                                                       textOut.front().lon ),
                                        MC2Coordinate( textOut.back().lat,
                                                       textOut.back().lon ) );

      bool ret = placeStraightText(0, 
                                   textOut.size()-1, 
                                   textOut[0].lat,
                                   textOut[0].lon,
                                   angle,
                                   dim,
                                   textOut);
      if ( ret ) {
         convertToDrawAngle( textOut );
      }
      return ret;
   }

   convertToDrawAngle(textOut);
   return true;
}
