/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavMapHelp.h"
#include "ExpandItemID.h"
#include "ExpandStringItem.h"
#include "GfxConstants.h"
#include "ExpandRoutePacket.h"
#include "GfxUtility.h"
#include "PacketContainer.h"
#include "Math.h"

NavMapHelp::NavMapHelp( InterfaceParserThread* thread,
                        NavParserThreadGroup* group )
      : NavHandler( thread, group )
{
}


MC2BoundingBox 
NavMapHelp::handleNavMapVectorBox( 
   uint32 mapRadius, uint16 speed, MC2Coordinate pos, 
   uint16 heading, PacketContainer* expandRouteCont ) const
{
   //      Use smart algorithm to use heading, speed and 
   //      position to make good map (perhaps check where route 
   //      goes and try to fit as much as posible)
   MC2BoundingBox vbbox;

   // This radius is default
   uint32 posSize = 60000;
   if ( mapRadius != 0 && mapRadius < 100000000 ) {
      // Not less than 100m
      posSize = uint32( rint( GfxConstants::METER_TO_MC2SCALE * 
                              MAX( 100, mapRadius ) ) );
   }
   mc2dbg4 << " posSize " << posSize << endl;

   if ( speed > 0 && speed < MAX_INT16 ) {
      // Speed in m/s
      posSize = speedToBboxSize( speed / 32 );
   }
   mc2dbg4 << " posSize after speed " << posSize << endl;

   // Try to fit route
   if ( expandRouteCont != NULL && static_cast< ReplyPacket* > ( 
           expandRouteCont->getPacket() )->getStatus() == StringTable::OK )
   {
      ExpandRouteReplyPacket* expand = 
         static_cast< ExpandRouteReplyPacket* > ( 
            expandRouteCont->getPacket() );
      ExpandItemID* exp = expand->getItemID();
      // The cossine at position
      float64 cosLat = GfxUtility::getCoslat( 
         pos.lat, pos.lat );

      // Try to fit as much as possible of the route in the 
      // radius starting at the "current" position in route
               
      // Find closest coord in route
      IntVector& lats = exp->getLat();
      IntVector& lons = exp->getLon();
      uint64 closest = MAX_UINT64;
      uint32 closestIndex = MAX_UINT32;
      uint64 tmpClosest = 0;
      for ( uint32 i = 0 ; i + 1 < lats.getSize() ; ++i ) {
         tmpClosest = GfxUtility::closestDistVectorToPoint(
            lons[ i ], lats[ i ], lons[ i + 1 ], lats[ i + 1 ],
            pos.lon, pos.lat, cosLat );

         if ( tmpClosest < closest ) {
            // Closer check heading
            //      Check if right heading... Rare case but,
            //      might look bad (finding closest earlier than it 
            //                      is in actual reality)
            //      Or if user is driving back and has the old route...
            uint16 angle = uint16( rint( 
                                      GfxConstants::radianTodegreeFactor *
               GfxUtility::getAngleFromNorth( 
                  lats[ i ], lons[ i ], lats[ i + 1 ], lons[ i + 1 ] ) ) );
            uint16 angleDiff1 = (angle - heading + 360) % 360;
            uint16 angleDiff2 = (heading - angle + 360) % 360;
            uint16 angleDiff = angleDiff1 > angleDiff2 ? 
               angleDiff2 : angleDiff1;
            mc2dbg8 << "angle " << angle << " heading " 
                    << heading << " angleDiff " << angleDiff 
                    << endl;
            if ( angleDiff < 120 ) {
               closest = tmpClosest;
               closestIndex = i + 1;
               mc2dbg8 << "Setting closest at " << i << " dist " << closest
                       << " angle " << angle << endl;
            } else {
               mc2dbg8 << "Angle and heading not matching not setting "
                       << "closest. Angle " << angle << " heading "
                       << heading << " dist " << tmpClosest
                       << endl;
            }
         }
      }
      mc2dbg4 << " Closesest " << (GfxConstants::MC2SCALE_TO_METER * 
                                   ::sqrt( closest ) ) << "m" << endl;

      // If close then begin at closest
      if ( closest < SQUARE( GfxConstants::METER_TO_MC2SCALE * 50 ))
      { // Closer than ~50 meters 
         // Check if no speed from client
         if ( speed <= 0 || speed >= MAX_INT16 ) {
            // This should rarely be needed
            // Use speed at closest if no speed from client
            uint32 i = 0;
            while ( i < exp->getNbrItems() ) {
               uint32 startIndex = exp->getCoordinateOffset()[ i ];
               uint32 endIndex = exp->nextCoordStart( i ); 
               if ( startIndex <= closestIndex && 
                    endIndex >= closestIndex ) 
               {
                  // Found it
                  break;
               }
               i++;
            }
            uint32 groupID = i;
            mc2dbg8 << "groupID " << groupID << " of " 
                    << exp->getNbrItems() << endl;
            uint16 speed = uint16( 
               rint( (exp->getSpeedLimit()[ groupID ]) / 3.6 ) );
            mc2dbg2 << " Using speed in route " 
                    << speed << "m/s = " << exp->getSpeedLimit()[ 
                       groupID ] << "km/h"<< endl;
            posSize = speedToBboxSize( speed );
         }

         // Add coords of route untill bbox has radius*2 width or 
         // height

         // Make sure current position is in bbox
         vbbox.update( pos.lat, 
                       pos.lon, false );
         vbbox.updateCosLat();

         // Closest point to position on closest segment not endpoint
         MC2Coordinate p1;
         MC2Coordinate p2;
         if ( closestIndex > 0 ) {
            p1 = MC2Coordinate( lats[ closestIndex -1],
                                lons[ closestIndex -1 ] );
            p2 = MC2Coordinate( lats[ closestIndex],
                                lons[ closestIndex ] );
         } else if ( closestIndex + 1 < lats.getSize() ) {
            // We should never get here as closestIndex = i + 1
            p1 = MC2Coordinate( lats[ closestIndex ],
                                lons[ closestIndex ] );
            p2 = MC2Coordinate( lats[ closestIndex + 1],
                                lons[ closestIndex + 1 ] );
         }
         if ( p1.isValid() && p2.isValid() ) {
            int32 lat = 0;
            int32 lon = 0;
            GfxUtility::closestDistVectorToPoint( 
               p1.lon, p1.lat, p2.lon, p2.lat, 
               pos.lon, pos.lat,
               lon, lat, vbbox.getCosLat() );
            vbbox.update( lat, lon, false );
         } else {
            vbbox.setMaxLat( lats[ closestIndex ] );
            vbbox.setMinLat( lats[ closestIndex ] );
            vbbox.setMaxLon( lons[ closestIndex ] );
            vbbox.setMinLon( lons[ closestIndex ] );
         }

         // Go throu route and expand bbox
         float64 routeFillFactor = 0.90;
         int32 routePosSize = int32( float64( posSize ) * 2 * 
                                     (routeFillFactor) );
         for ( uint32 i = closestIndex ; i < lats.getSize() ;
               ++i ) 
         {
            // Hmm a very long segment may give too big bbox
            // maybe have to get a point on the last segment 
            // that gives the desired bbox size
            MC2BoundingBox tbbox( vbbox );
            tbbox.update( lats[ i ], lons[ i ], false );
            if ( tbbox.getWidth() > routePosSize ||
                 tbbox.getHeight() > routePosSize )
            {
               // Filled bbox
               // Add the part of the last segment that is
               // inside the desired bbox size
               if ( i > 0 ) {
                  int32 clippedStartLat = 0;
                  int32 clippedStartLon = 0;
                  if ( vbbox.clipToFirstIntersectionWithEdge( 
                          lats[ i - 1 ], lons[ i - 1 ], 
                          lats[ i ], lons[ i ], 
                          clippedStartLat, clippedStartLon ) )
                  {
                     // Ok got line from bbox edge to point that is too far
                     // away.
                     // Get the lon where bbox width get too big
                     // Get the lat where bbox height get too big
                     int32 latDiff = 0;
                     int32 lonDiff = 0;
                     if ( (clippedStartLon - vbbox.getMinLon()) < 0 ) {
                        // Left
                        lonDiff = clippedStartLon - vbbox.getMinLon();
                     } else if ( (clippedStartLon - vbbox.getMaxLon()) > 0)
                     {
                        // Right
                        lonDiff = clippedStartLon - vbbox.getMaxLon();
                     }
                     if ( clippedStartLat > vbbox.getMaxLat() ) {
                        // Up
                        latDiff = clippedStartLat - vbbox.getMaxLat();
                     } else if ( clippedStartLat < vbbox.getMinLat() ) {
                        // Down
                        latDiff = clippedStartLat - vbbox.getMinLat();
                     }
                     float64 latPer = latDiff / 
                        float64(routePosSize - vbbox.getHeight());
                     float64 lonPer = lonDiff * vbbox.getCosLat() / 
                        float64(routePosSize - vbbox.getWidth());

                     if ( fabs( latPer ) > fabs( lonPer ) ) {
                        // More on lat, lat limits
                        // add latPer*latDiff + clippedStartLat to bbox
                        vbbox.update( int32(latPer*latDiff) + 
                                      clippedStartLat, 
                                      clippedStartLon, false );
                     } else {
                        // More on lon, lon limits
                        // add lonPer*lonDiff + clippedStartLon to bbox
                        vbbox.update( clippedStartLat,
                                      int32(lonPer*lonDiff) + 
                                      clippedStartLon, false );
                     }
                  } else {
                     // Darn!
                     mc2log << warn << "navMapHelp:"
                            << "handleNavMapVectorBox "
                            << "clipToFirstIntersectionWithEdge failed "
                            << lats[ i - 1 ] << "," << lons[ i - 1 ]
                            << lats[ i ] << "," << lons[ i ] << " bbox "
                            << vbbox << endl;
                     // Ok use the whole segment
                     vbbox = tbbox;
                  }
               } else {
                  // Should never get here as closestIndex = i + 1
                  vbbox = tbbox;
               }
               break;
            } else {
               // Not full bbox, continue
               vbbox = tbbox;
            }
         }
         vbbox.updateCosLat();

         // Add the border factor
         vbbox.increaseFactor( 1.0/routeFillFactor -1.0 );
         
         // Make sure box is not too small
         if ( vbbox.getWidth() > vbbox.getHeight() ) {
            if ( vbbox.getWidth() < int32(posSize*2) ) {
               float64 factor = 
                  float64( posSize*2 ) / vbbox.getWidth() - 1.0;
               vbbox.increaseFactor( factor );
            }
         } else {
            if ( vbbox.getHeight() < int32(posSize*2) ) {
               float64 factor = 
                  float64( posSize*2 ) / vbbox.getHeight() - 1.0;
               vbbox.increaseFactor( factor );
            }
         }
      } else {
         mc2dbg2 << " Closest not close enough " 
                 << (GfxConstants::MC2SCALE_TO_METER * 
                     ::sqrt( closest ) ) << "m pos " << pos
                 << endl;
      }
      mc2dbg4 << "Made route bbox " << vbbox << endl;

      delete exp;
   }

   // If bbox not set yet
   // Use angle and position and put more map "ahead"
   if ( ! vbbox.isValid() && heading <= 360 ) {
      mc2dbg2 << " Heading" << endl;
      // The cossine at position
      float64 cosLat = GfxUtility::getCoslat( 
         pos.lat, pos.lat );
      // More of the bbox in the direction of the heading.
      MC2Coordinate center = pos;
      // Move center back some
      int32 latDiff = int32( posSize / 2 * 
                             sin( M_PI/2 - heading 
                                  * GfxConstants::degreeToRadianFactor ) );
      mc2dbg4 << " latDiff " << latDiff << endl;
      int32 lonDiff = int32( posSize / 2 * 
                             cos( M_PI/2 - heading
                                  * GfxConstants::degreeToRadianFactor ) 
                             / cosLat );
      mc2dbg4 << " lonDiff " << lonDiff << endl;
      center.lat = pos.lat + latDiff;
      center.lon = pos.lon + lonDiff;

      vbbox.setMaxLat( center.lat + posSize );
      vbbox.setMinLon( center.lon - posSize );
      vbbox.setMinLat( center.lat - posSize );
      vbbox.setMaxLon( center.lon + posSize );
   }

   // If nothing else make a bbox around position
   if ( ! vbbox.isValid() ) {
      mc2dbg2 << " Position" << endl;
      vbbox.setMaxLat( pos.lat + posSize );
      vbbox.setMinLon( pos.lon - posSize );
      vbbox.setMinLat( pos.lat - posSize );
      vbbox.setMaxLon( pos.lon + posSize );
   }
   if ( vbbox.isValid() ) {
      vbbox.updateCosLat();
      mc2dbg4 << " width " << (vbbox.getWidth() * 
                               GfxConstants::MC2SCALE_TO_METER )
              << "m height " << (vbbox.getHeight() * 
                                 GfxConstants::MC2SCALE_TO_METER)
              << "m " << endl;
   }

   return vbbox;
}



uint32 
NavMapHelp::speedToBboxSize( uint16 speed ) const {
   // Make posSize from speed
   uint32 meterSize = 400;
#if 0
   // Linear
   meterSize = 200 + 1400/40 * speed;
#elif 0
   // Hand fixed "exponential"
   if ( speed <= 5 ) {
      // 0- 20 km/h
      meterSize = 150;
   } else if ( speed <= 11 ) {
      // 20 - 40
      meterSize = 200;
   } else if ( speed <= 16 ) {
      // 40 - 60
      meterSize = 250;
   } else if ( speed <= 22 ) {
      // 60 - 80
      meterSize = 350;
   } else if ( speed <= 27 ) {
      // 80 - 100
      meterSize = 500;
   } else if ( speed <= 33 ) {
      // 100 - 120
      meterSize = 800;
   } else if ( speed <= 38 ) {
      // 120 - 140
      meterSize = 1100;
   } else  {
      // 140 + 
      meterSize = 1400;
   }
#else
   // True exponential
   // Speed in km/h
   const uint32 maxX = 110;
   uint32 x = uint32( rint( speed * 3.6 ) );
   uint32 extra = 0;
   if ( x > maxX ) {
      extra = x - maxX;
      x = maxX;
   }

   // For x in 0-maxXkm/h
   // a*x^3 + b*x + c, a = 0.0006, b = 3, c = 150
   meterSize = uint32( rint( 0.0006*x*x*x + 3*x + 150 ) );
   
   // More than maxX then add linear (3*a*(x)^2 + b) = 25
   if ( extra > 0 ) {
      meterSize += 25*x;
   }

#endif
   mc2dbg4 << "speedToBboxSize size " << meterSize << "m" << endl;
   return uint32( rint( GfxConstants::METER_TO_MC2SCALE * 
                        meterSize ) );
}
