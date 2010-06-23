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

#include "GfxUtility.h"
#include "GfxFeatureMap.h"
#include "GfxPolygon.h"
#include "GfxRoadPolygon.h"
#include "StringUtility.h"
#include "STLUtility.h"
#include <math.h>
#include <map>
#include <algorithm>

ostream& operator << ( ostream& ostr, const ScreenSize& size ) {
   ostr << "( " << size.getWidth() << " x " << size.getHeight() << " )";
   return ostr;
}
/**
 *    The size of the header on disc:
 *    m_bbox + m_screenX + m_screenY + m_scaleLevel + 
 *    (m_transportationType + m_startingAngle + m_drivingOnRightSide +
 *     m_initialUTurn + 1 byte padding)
 */
#define GFXFEATUREMAP_HEADER_SIZE (4*4 + 4 + 4 + 4 + 4)

GfxFeatureMap::GfxFeatureMap():
   m_screenSize( 0, 0 ),
   m_scaleLevel( 0 ),
   m_featureSize( 0 ),

   // Default drive as RouteModule
   // doesn't send trnspt if drive
   m_transportationType( ItemTypes::drive ),

   m_startingAngle( 64 ), // Right angle
   m_drivingOnRightSide( true ),
   m_initialUTurn( false )
{
   m_features.reserve( 100 );
   m_bbox.reset();
}


GfxFeatureMap::~GfxFeatureMap() {
   STLUtility::deleteValues( m_features );  
}


bool
GfxFeatureMap::load( DataBuffer* data ) {
   mc2dbg2 << "Entering GfxFeatureMap::load" << endl;

   // ------------------------------------------------------------------
   // --------------------------------------------- Load the header ----
   uint32 nbrFeatures = loadHeader( data );
   
   if ( nbrFeatures == MAX_UINT32 ) {
      return false;
   }

   // ------------------------------------------------------------------
   // -------------------------------------------- Load the features ---
   
   m_features.reserve( nbrFeatures );
   for ( uint32 i = 0 ; i < nbrFeatures ; i++ ) {
      addFeature( GfxFeature::createNewFeature( data ) );
   }

   mc2dbg2 << "Exiting GfxFeatureMap::load" << endl;
   return true;
}


uint32 
GfxFeatureMap::loadHeader( DataBuffer* data ) {
   // ------------------------------------------------------------------
   // --------------------------------------------- Load the header ----
   data->reset();

   // Upper left corner
   m_bbox.setMaxLat(data->readNextLong());
   m_bbox.setMinLon(data->readNextLong());
   // Lower right corner
   m_bbox.setMinLat(data->readNextLong());
   m_bbox.setMaxLon(data->readNextLong());

   // Screen height and width
   ScreenSize::ValueType width = data->readNextShort();
   ScreenSize::ValueType height = data->readNextShort();
   m_screenSize = ScreenSize( width, height );

   // Scale level
   m_scaleLevel = data->readNextLong();

   // Transportation type
   setTransportationType( ItemTypes::transportation_t(
      data->readNextByte() ) );

   // Starting angle 0-255
   setStartingAngle( data->readNextByte() );

   // Combined bool (Left right traffic and initial U-Turn)
   uint8 bools = data->readNextByte();
   setDrivingOnRightSide( (bools>>1) & 0x1 );
   setInitialUTurn( bools & 0x1 );

   // 1 byte padding, reserved
   data->readNextByte();

   uint32 nbrFeatures = data->readNextLong(); // Nbr features

   return nbrFeatures;
}


bool
GfxFeatureMap::save( DataBuffer* data ) const {
   saveHeader( data, m_features.size() );

   // ------------------------------------------------------------------
   // -------------------------------------------- Save the features ---

   
   GfxFeature* feature;
   for (uint32 i = 0; i < m_features.size(); i++) {
      feature = m_features[ i ];
      feature->save(data);
   }

   return (true);
}


bool 
GfxFeatureMap::saveHeader( DataBuffer* data, uint32 nbrFeatures ) const {
   data->reset();
   
   // ------------------------------------------------------------------
   // --------------------------------------------- Save the header ----
   
   // Upper left corner
   data->writeNextLong(m_bbox.getMaxLat());
   data->writeNextLong(m_bbox.getMinLon());
   // Lower right corner
   data->writeNextLong(m_bbox.getMinLat());
   data->writeNextLong(m_bbox.getMaxLon());
   
   // Screen width and height
   data->writeNextShort( m_screenSize.getWidth() );
   data->writeNextShort( m_screenSize.getHeight() );

   // Scale level
   data->writeNextLong(m_scaleLevel);

   // Transportation type
   data->writeNextByte( getTransportationType() );

   // Starting angle 0-255
   data->writeNextByte( getStartingAngle() );

   // Combined bool (Left right traffic and initial U-Turn)
   uint8 bools = ( (getDrivingOnRightSide()<<1) | (getInitialUTurn()) );
   data->writeNextByte(bools);

   // 1 byte padding, reserved
   data->writeNextByte(0);

   data->writeNextLong( nbrFeatures ); // Nbr features
   
   return true;
}


bool 
GfxFeatureMap::save( DataBuffer* data, DataBuffer* featureData, 
                     uint32 nbrFeatures ) const
{
   saveHeader( data, nbrFeatures );
   data->writeNextByteArray( featureData->getBufferAddress(),
                             featureData->getBufferSize() );

   return true;
}


uint32
GfxFeatureMap::getMapSize() const {
   return (GFXFEATUREMAP_HEADER_SIZE + m_featureSize);
}


uint32
GfxFeatureMap::getNbrFeatures() const {
   return m_features.size();
}

const GfxFeature* 
GfxFeatureMap::getFeature( uint32 index ) const {
   const GfxFeature* out = NULL;
   if ( index < getNbrFeatures() ) {
      out = m_features[ index ];
   }

   return out;
}


void
GfxFeatureMap::addFeature( GfxFeature* feature ) {
   DEBUG8(
      for ( uint32 i = 0 ; i < feature->getNbrPolygons() ; i++ ) {
         GfxPolygon* tmpF = feature->getPolygon( i );
         if ((tmpF != NULL) && (tmpF->getNbrCoordinates() > 4000)) {
            DEBUG2(cerr << "Adding feature with more than "
                   "4000 coordinates: " << tmpF->getNbrCoordinates() 
                   << endl;);
            DEBUG4(feature->dump(8););
         }
      }
   );
   m_features.push_back( feature );
   m_featureSize += feature->getSize();
}


bool
GfxFeatureMap::removeFeature( uint32 index ) {
   if ( index < getNbrFeatures() ) {
      GfxFeature* feature = m_features[ index];
      m_featureSize -= feature->getSize();
               
      delete m_features[ index ];
      m_features.erase( m_features.begin() + index );
      return true;
   } else {
      return false;
   } 
}

void
GfxFeatureMap::setMC2BoundingBox(const MC2BoundingBox* bbox)
{
   // Copy into m_bbox
   m_bbox = *bbox;
}

void
GfxFeatureMap::getMC2BoundingBox(MC2BoundingBox* bbox)
{
   *bbox = m_bbox;
}


void
GfxFeatureMap::setScreenSize(const ScreenSize& size )
{
   m_screenSize = size;
}

void
GfxFeatureMap::dump(int verboseLevel) const
{
   mc2dbg << "GfxFeatureMap::dump() verboselevel = " << verboseLevel << endl;
   mc2dbg << "  Size in bytes = " << getMapSize() << endl;
   if (verboseLevel > 0) {
      mc2dbg << "  Bbox = ";
      DEBUG8(m_bbox.dump(true););
      mc2dbg << ", screenSize=" << m_screenSize
             << ", scaleLevel=" << m_scaleLevel << ", transportationType="
             << int(m_transportationType) << ", startingAngle="
             << int(m_startingAngle) << ", drivingOnRightSide="
             << m_drivingOnRightSide << ", initialUTurn="
             << m_initialUTurn << endl;
   }
   mc2dbg << "  Number features " << m_features.size() << endl;
   if (verboseLevel > 0) {
      for (uint32 i = 0; i < m_features.size(); i++) {
         m_features[ i ]->dump(verboseLevel);
      }
   }
}
      
void 
GfxFeatureMap::printStatistics(int verboseLevel)
{
   mc2dbg1 << "GfxFeatureMap::printStatistics() verboselevel = "
           << verboseLevel << endl;

   
   map<int, int> size;
   map<int, int> nbrItems;
   map<int, int> nbrPolygons;
   map<int, int> nbrCoordinates;
   for (uint32 i = 0; i < GfxFeature::NBR_GFXFEATURES; i++) {
      size[i] = 0;
      nbrItems[i] = 0;
      nbrPolygons[i];
      nbrCoordinates[i] = 0;      
   }
   
   for (uint32 i = 0; i < m_features.size(); i++) {
      GfxFeature* feature = m_features[ i ];
      
      int type = feature->getType();
      nbrItems[type]++;
      size[type] += feature->getSize();
      nbrPolygons[type] += 
         feature->getNbrPolygons(); 
      for (uint32 j = 0; j < feature->getNbrPolygons(); j++) {
         nbrCoordinates[type] += 
            feature->getPolygon(j)->getNbrCoordinates();
      }
   }

   for (uint32 i = 0; i < GfxFeature::NBR_GFXFEATURES; i++) {
      if (nbrItems[i] > 0) {
         cout << "-------------------------" << endl;
         cout << "GfxFeatureType = " << i << endl;
         cout << "  Size = " << size[i] << endl;
         cout << "  Nbr features = " << nbrItems[i] << endl;
         cout << "  Nbr polygons = " << nbrPolygons[i] << endl;
         cout << "  Nbr coordinates = " << nbrCoordinates[i] << endl;


      }
      size[i] = 0;
      nbrItems[i] = 0;
      nbrPolygons[i];
      nbrCoordinates[i] = 0;      
   }
   

   
}

int
GfxFeatureMap::mergeInto(GfxFeatureMap* otherMap,
                         bool keepOrder)
{
   // Check inparameter
   if (otherMap == NULL) {
      return (-1);
   }

   // Add the features to this map and remove them from the other
   uint32 nbrAddedFeatures = 0;
   GfxFeature* curFeature = NULL;
   if ( keepOrder ) {
      curFeature = otherMap->getAndRemoveFirstFeature();
   } else {
      curFeature = otherMap->getAndRemoveLastFeature();
   }
   while (curFeature != NULL) {
      addFeature(curFeature);
      nbrAddedFeatures++;
      if ( keepOrder ) {
         curFeature = otherMap->getAndRemoveFirstFeature();
      } else {
         curFeature = otherMap->getAndRemoveLastFeature();
      }
   }

   return (nbrAddedFeatures);
}

void
GfxFeatureMap::mergeRouteSettings( const GfxFeatureMap* otherMap ) {
   setTransportationType( otherMap->getTransportationType() );
   setStartingAngle( otherMap->getStartingAngle() );
   setDrivingOnRightSide( otherMap->getDrivingOnRightSide() );
   setInitialUTurn( otherMap->getInitialUTurn() );
}

GfxFeature*
GfxFeatureMap::getAndRemoveFirstFeature() 
{
   GfxFeature* returnValue = NULL;
   if (m_features.size() > 0) {
      returnValue = m_features.front();
      m_features.erase( m_features.begin() );
      m_featureSize -= returnValue->getSize();
   }
   return (returnValue);
}



void
GfxFeatureMap::recalculateLevelAndSplitRoads()
{
   uint32 orgNbrFeatures = getNbrFeatures();
   // Check all polygons in all road features
   for (uint32 i=0; i<orgNbrFeatures; ++i) {
      GfxRoadFeature* roadFeature = 
         dynamic_cast<GfxRoadFeature*>( m_features[ i ] );
      if (roadFeature != NULL) {
         uint32 orgNbrPolygons = roadFeature->getNbrPolygons();
         for (uint32 j=0; j<orgNbrPolygons; ++j) {
            GfxRoadPolygon* curRoadPoly = 
               static_cast<GfxRoadPolygon*>(roadFeature->getPolygon(j));
            // Dubbel the level so that we have room for virtual levels
            // on the ramps (will always be even)
            mc2dbg4 << "level orig: l0="
                    << (int)curRoadPoly->getLevel0() << ", l1=" 
                    << (int)curRoadPoly->getLevel1() << endl;
            curRoadPoly->setLevel0(2*curRoadPoly->getLevel0());
            curRoadPoly->setLevel1(2*curRoadPoly->getLevel1());
            // Are the levels same, create new polgons with virtual level 
            // (odd)
            if (curRoadPoly->getLevel0() != curRoadPoly->getLevel1()) {
               int virtualLevel = ( curRoadPoly->getLevel0() + 
                                    curRoadPoly->getLevel1()) / 2;
               mc2dbg4 << "vLevel set to=" << virtualLevel << ", l0="
                       << (int)curRoadPoly->getLevel0() << ", l1=" 
                       << (int)curRoadPoly->getLevel1() << endl;

               GfxRoadPolygon* newPoly = new GfxRoadPolygon(
                           curRoadPoly->usesCoordinates16Bits(),
                           0,    // startSize
                           curRoadPoly->getPosSpeedlimit(),
                           curRoadPoly->getNegSpeedlimit(),
                           curRoadPoly->isMultidigitialized(),
                           curRoadPoly->isRamp(),
                           curRoadPoly->isRoundabout(),
                           virtualLevel,
                           curRoadPoly->getLevel1());
               curRoadPoly->setLevel1(virtualLevel);
               
               mc2dbg8 << "NbrCoords before: " 
                       << curRoadPoly->getNbrCoordinates() << endl;
               if (curRoadPoly->getNbrCoordinates() > 2) {
                  // Split at the "middle" coordinate 
                  int nbrRemove = 0;
                  for (uint32 k=curRoadPoly->getNbrCoordinates()/2; 
                       k<curRoadPoly->getNbrCoordinates(); ++k) {
                     newPoly->addCoordinate(curRoadPoly->getLat(k), 
                                            curRoadPoly->getLon(k));
                     mc2dbg4 << "Adding (" << curRoadPoly->getLat(k) << ","
                             << curRoadPoly->getLon(k) << ") to newPoly" 
                             << endl;
                     ++nbrRemove;
                  }
                  mc2dbg4 << "Case1: nbrRemove=" << nbrRemove << endl;
                  while (nbrRemove > 1) {
                     curRoadPoly->removeLastCoordinate();
                     mc2dbg8 << "Removing last coord from curRoadPoly" << endl;
                     --nbrRemove;
                  }

               } else if (curRoadPoly->getNbrCoordinates() == 2) {
                  // Only two coordinates calculate a new coordinate in the 
                  // middle
                  float64 length = sqrt(GfxUtility::squareP2Pdistance_linear(
                           curRoadPoly->getLat(0), curRoadPoly->getLon(0), 
                           curRoadPoly->getLat(1), curRoadPoly->getLon(1)));
                  int32 newLat, newLon;
                  if (GfxUtility::getPointOnLine(
                           curRoadPoly->getLat(0), curRoadPoly->getLon(0), 
                           curRoadPoly->getLat(1), curRoadPoly->getLon(1), 
                           length / 2.0, newLat, newLon)) {
                     newPoly->addCoordinate(newLat, newLon);
                     newPoly->addCoordinate(curRoadPoly->getLat(1), 
                                            curRoadPoly->getLon(1));
                     curRoadPoly->removeLastCoordinate();
                     curRoadPoly->addCoordinate(newLat, newLon);
                     mc2dbg4 << "Case2: " << endl;
                     mc2dbg4 << "   curRoadPoly: (" 
                             << curRoadPoly->getLat(0) << "," 
                             << curRoadPoly->getLon(0) << ") (" 
                             << curRoadPoly->getLat(1) << "," 
                             << curRoadPoly->getLon(1) << ")" << endl;
                     mc2dbg4 << "   newPoly: (" 
                             << newPoly->getLat(0) << "," 
                             << newPoly->getLon(0) << ") (" 
                             << newPoly->getLat(1) << "," 
                             << newPoly->getLon(1) << ")" << endl;
                  } else {
                     mc2dbg8 << "Case2: failed!" << endl;
                  }
               } else {
                  // Very strange, a road with only one coordinate
                  //mc2log << fatal << here << " GfxRoadPoly with " 
                  //       << curRoadPoly->getNbrCoordinates() << " coords"
                  //       << endl;
               }

               mc2dbg8 << "NbrCoords after: " 
                       << curRoadPoly->getNbrCoordinates() << ", newPoly: " 
                       << newPoly->getNbrCoordinates() << endl;
               DEBUG8(mc2dbg << "curRoadPoly:" << endl; curRoadPoly->dump(20));
               DEBUG8(mc2dbg << "newPoly:" << endl; newPoly->dump(20));

               // Add the new polygon to the GfxRoadFeature
               roadFeature->addNewPolygon(newPoly);

               DEBUG8(mc2dbg << "roadFeature:" << endl; roadFeature->dump(2));
            }
         }
      }
   }

}

/**
 *    Functor, used for comparing GfxFeatures.
 */
class GfxFeatureSorter {

   public:

      /**
       *    @return True if a < b in some weird way.
       */
      bool operator() ( const GfxFeature* a, const GfxFeature* b ) const 
      {
         // Check type first.
         if ( a->getType() != b->getType() ) {
            return a->getType() < b->getType();
         } 
         
         // Same type. Check name instead.
         int nameCmp = strcmp( a->getName(), b->getName() );
         if ( nameCmp != 0 ) {
            return nameCmp < 0;
         }

         // Same name also. Check size of data.
         uint32 aSize = a->getSize();
         uint32 bSize = b->getSize();

         if ( aSize != bSize ) {
            return aSize < bSize;
         }

         // Ok, we really need to compare the databuffers.
         DataBuffer aBuf( aSize );
         DataBuffer bBuf( bSize );

         // Fill them with zeros so we can compare the memory afterwards.
         aBuf.fillWithZeros();
         bBuf.fillWithZeros();
        
         // Eeek.
         a->save( &aBuf );
         b->save( &bBuf );

         return memcmp( aBuf.getBufferAddress(), 
                        bBuf.getBufferAddress(), aSize ) < 0;
      }
   
};

void
GfxFeatureMap::removeDuplicates()
{
   // A set is used to check for duplicates.
   typedef set<GfxFeature*, GfxFeatureSorter> set_t;
   set_t featuresToChk;

   // The vector is used because we don't want to 
   // change the order of the features.
   vector<GfxFeature*> featuresToAdd;
   
   // Add all features to the set.
   for ( uint32 i = 0; i < getNbrFeatures(); ++i ) {
      GfxFeature* feature =  m_features[ i ];
      pair<set_t::iterator, bool> res = featuresToChk.insert( feature  );
      if ( ! res.second ) {
         // Was not added because it was already present in the set.
         m_featureSize -= feature->getSize();
         delete feature;
      } else {
         // It was added. Also add to the vector.
         featuresToAdd.push_back( feature );
      }
   }

   // Clear the features in the map.
   m_features.clear();

   // Add the unique ones again. Add from the vector so the order of
   // the features remains the same as before.
   for ( vector<GfxFeature*>::iterator it = featuresToAdd.begin(); 
         it != featuresToAdd.end(); ++it ) {
      addFeature( *it );
   }
}

void
GfxFeatureMap::getUniqueFeatures( vector<const GfxFeature*>& uniques,
                                  GfxFeature::gfxFeatureType type ) const
{
   // A set is used to check for duplicates, but the vector is to
   // avoid changign the order of the features.
   typedef set<const GfxFeature*, GfxFeatureSorter> set_t;
   set_t featuresToChk;

   // Add all features to the set.
   for ( uint32 i = 0; i < getNbrFeatures(); ++i ) {
      const GfxFeature* feature = 
         (const GfxFeature*) m_features[ i ];
      if ( feature->getType() == type ) {
         pair<set_t::iterator, bool> res = featuresToChk.insert( feature );
         if ( res.second ) {
            // It was added. Also add to the vector.
            uniques.push_back( feature );
         }
      }
   }
}


byte* 
GfxFeatureMap::getAsSimpleMapData( int version,
                                   uint32& outSize ) const
{
   // Currently only handles version 1.
   if ( version != 1 ) {
      outSize = 0;
      return NULL;
   }
   
   // 50 MB should be enough. 
   SharedBuffer buf( 50*1024*1024 );
  
   // Nbr features.
   uint32 nbrFeatures = 0;
   
   // Total number coords.
   uint32 nbrCoords = 0;
   
   // Size of data in bytes. To be filled in later.
   buf.writeNextBALong( 0 );
   // Nbr features. To be filled in later.
   buf.writeNextBALong( 0 );
   // Total number coordinates. To be filled in later.
   buf.writeNextBALong( 0 );
  
   // Write all features.
   for ( uint32 i = 0; i < getNbrFeatures(); ++i ) {
      // All polygons become a new feature. Hopefully not many features
      // will have more than one polygon.
      const GfxFeature* feature = getFeature( i );
      for ( uint32 p = 0; p < feature->getNbrPolygons(); ++p ) {
         ++nbrFeatures;
         // Write the type.
         buf.writeNextBAShort( feature->getType() );

         const GfxPolygon* gfxPoly = feature->getPolygon( p );
         // Nbr coords.
         buf.writeNextBAShort( gfxPoly->getNbrCoordinates() );
         nbrCoords += gfxPoly->getNbrCoordinates();
          
         // The coords.
         for ( uint32 j = 0; j < gfxPoly->getNbrCoordinates(); ++j ) {
            // Lat.
            buf.writeNextBALong( gfxPoly->getLat( j ) );
            // Lon.
            buf.writeNextBALong( gfxPoly->getLon( j ) );
         }
      }
   }

   outSize = buf.getCurrentOffset();

   buf.reset();
   
   // Fill in the new values.
   // Size
   buf.writeNextBALong( outSize );
   // Nbr features.
   buf.writeNextBALong( nbrFeatures );
   // Nbr coords.
   buf.writeNextBALong( nbrCoords );

   byte* returnBuf = new byte[ outSize ];
   memcpy( returnBuf, buf.getBufferAddress(), outSize );

   return returnBuf; // Caller must delete this one.
}

void
GfxFeatureMap::reverseFeatures()
{
   std::reverse( m_features.begin(), m_features.end() );   
}

