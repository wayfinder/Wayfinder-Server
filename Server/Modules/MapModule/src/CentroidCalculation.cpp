/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CentroidCalculation.h"

#include "GfxFeature.h"
#include "Item.h"
#include "MapUtility.h"
#include "NonStdStl.h"
#include "ArrayTools.h"
#include "GfxData.h"

#include <strings.h>
#include <algorithm>

namespace CentroidCalculation {

/**
 * Center lookup table for countries.
 */
struct CentroidLookup {
   const char* m_name;
   MC2Coordinate m_center;
} centroidLookup[] = {
   { "angola", MC2Coordinate( -162299096, 202174304 ) },
   { "australia", MC2Coordinate( -277372129,1611013057 ) },
   { "belarus", MC2Coordinate( 629478061, 329681148 ) },
   { "columbia", MC2Coordinate( 35600523, -877681880 ) },
   { "ethiopia", MC2Coordinate( 83132796, 472250201 ) },
   { "finland", MC2Coordinate( 756900185, 311107365 ) },
   { "greenland", MC2Coordinate( 804918791, -524987520 ) },
   { "iceland", MC2Coordinate( 773092133,-220760669 ) },
   { "iran", MC2Coordinate( 384593597, 649417600 ) },
   { "mali", MC2Coordinate( 220207309, -19881469 ) },
   { "mauritania", MC2Coordinate( 242712109, -127255651 ) },
   { "morocco", MC2Coordinate( 377506637, -114728664 ) },
   { "nicaragua", MC2Coordinate( 155355878, -1007127422 ) },
   { "norway", MC2Coordinate( 734035313, 105903372 ) },
   { "oman", MC2Coordinate( 239913428, 679840288 ) },
   { "pakistan", MC2Coordinate( 340678216, 795714923 ) },
   { "peru", MC2Coordinate( -106518702, -900349763 ) },
   { "saudi arabia", MC2Coordinate( 260521534, 543832989 ) },
   { "somalia", MC2Coordinate( 77801255, 587975710 ) },
   { "sweden", MC2Coordinate( 748006449, 185322523 ) },
   { "turkey", MC2Coordinate( 476350789, 456889726 ) }
};

/**
 * Compare less on the m_name in CentroidLookup
 */
struct CentroidLookupLess {
   bool operator() ( const CentroidLookup& lhs,
                     const CentroidLookup& rhs ) {
      return strcasecmp( lhs.m_name, rhs.m_name ) < 0;
   }

   bool operator() ( const CentroidLookup& lhs,
                     const char* rhs ) {
      return strcasecmp( lhs.m_name, rhs ) < 0;
   }
};

/// Whether the table was sorted or not.
bool isSorted = false;

MC2Coordinate getCentroidForLandFeature( const GfxFeature& feature ) {
   // check sorted once.
   if ( ! isSorted ) {
      MC2_ASSERT( is_sorted( BEGIN_ARRAY( centroidLookup ),
                             END_ARRAY( centroidLookup ),
                             CentroidLookupLess() ) );
      isSorted = true;
   }

   CentroidLookup* pos =
      std::lower_bound( BEGIN_ARRAY( centroidLookup ),
                        END_ARRAY( centroidLookup ),
                        feature.getBasename().c_str(),
                        CentroidLookupLess() );

   if ( pos != END_ARRAY( centroidLookup ) &&
        strcasecmp( pos->m_name, feature.getBasename().c_str() ) == 0 ) {
      return pos->m_center;
   }

   return MC2Coordinate();
}

void addCentroidForAreas( GfxFeature& feature,
                          const Item& item ) {
   // calculate center coord from original gfx data
   // and append it last
   if ( MapUtility::isLargeAreaType( feature.getType() )  ) {
      MC2Coordinate center;
      item.getGfxData()->getPolygonCentroid( 0, center );
      feature.addNewPolygon( true, 1 );
      feature.addCoordinateToLast( center );
   }
}

} // CentroidCalculation
