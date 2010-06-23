/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandedRoute.h"
#include "ExpandRoutePacket.h"
#include "ExpandStringItem.h"
#include "ExpandItemID.h"
#include "ExpandRoutePacket.h"
#include "HttpUtility.h"
#include "LandmarkLink.h"
#include "LandmarkHead.h"

#include "Math.h"

ExpandedRoute::ExpandedRoute() 
      : m_dist( 0 ),
        m_time( 0 ),
        m_standstillTime( 0 ),
        m_startVehicle( ItemTypes::passengerCar ),
        m_routeBoundingbox(),
        m_origins(),
        m_destinations(),
        m_routeItems(),
        m_lang( StringTable::ENGLISH ),
        m_compassLangShort( true ),
        m_maxTurnNumbers( 9 ),
        m_printSignPostText( false ),
        m_distFormat( StringTableUtility::NORMAL ),
        m_wapFormat( false ),
        m_nameSeparator( "/" )
{

}


ExpandedRoute::ExpandedRoute( ExpandRouteReplyPacket* p )
      : m_dist( p->getTotalDist() ),
        m_time( p-> getTotalTime() ),
        m_standstillTime( p->getStandStillTime() ),
        m_startVehicle( ItemTypes::passengerCar ),
        m_routeBoundingbox(),
        m_origins(),
        m_destinations(),
        m_routeItems(),
        m_lang( StringTable::ENGLISH ),
        m_compassLangShort( true ),
        m_maxTurnNumbers( 15 ),
        m_printSignPostText( false ),
        m_distFormat( StringTableUtility::NORMAL ),
        m_wapFormat( false ),
        m_nameSeparator( "/" )
{
   // Extract data from p
   ExpandStringItem** stringItems = p->getStringDataItem();
   uint32 nbrStringItems = p->getNumStringData();
   ExpandItemID* exp = p->getItemID();
   
   Vector& groupID = exp->getGroupID();
   Vector& mapID = exp->getMapID();
   Vector& itemID = exp->getItemID();
   IntVector& lat = exp->getLat();
   IntVector& lon = exp->getLon();
   Vector& speed = exp->getSpeedLimit();
   Vector& attributes = exp->getAttributes();
   Vector& offset     = exp->getCoordinateOffset();

   bool hasSpeed = speed.getSize() > 0;
   bool hasAttributes = attributes.getSize() > 0;
   bool hasCoordinates = lat.getSize() > 0;


   int32 northLat = 0;
   int32 westLon = 0;
   int32 southLat = 0;
   int32 eastLon = 0;
   HttpUtility::getRouteMC2BoundingBox( p, exp,
                                        0, nbrStringItems - 1,
                                        southLat, northLat,
                                        westLon, eastLon );
   m_routeBoundingbox.setMaxLat( northLat );
   m_routeBoundingbox.setMinLat( southLat );
   m_routeBoundingbox.setMaxLon( eastLon );
   m_routeBoundingbox.setMinLon( westLon );

   for ( uint32 i = 0 ; i < nbrStringItems ; ++i ) {
      ExpandStringItem* sItem = stringItems[ i ];
      NameCollection* roadName = new NameCollection();
      roadName->addName( new Name( sItem->getText(), 
                                   LangTypes::english ) );
      int32 minLat = MAX_INT32;
      int32 maxLat = MAX_INT32;
      int32 minLon = 0;
      int32 maxLon = 0;
      if ( hasCoordinates ) {
         HttpUtility::getRouteMC2BoundingBox( 
            p, exp, i , i, 
            minLat, maxLat, minLon, maxLon );
      }
      MC2BoundingBox turnBoundingbox( maxLat, minLon, minLat, maxLon );

      if ( i == 0 ) {
         mc2dbg2 << "ExpandedRoute::ExpandedRoute first turn: "
                 << StringTable::getString( sItem->getStringCode(),
                                            StringTable::ENGLISH )
                 << " (" << int(sItem->getStringCode()) << ")" << endl;
      }
      ExpandedRouteItem* routeItem = new ExpandedRouteItem(
         ExpandedRouteItem::routeTurnFromStringCode( 
            sItem->getStringCode() ),
         sItem->getDist(), 
         sItem->getTime(), 
         roadName, sItem->getTransportationType(),
         sItem->getLatitude(), sItem->getLongitude(), 
         sItem->getExitCount(), 
         sItem->getLanes(),
         sItem->getSignPosts(),
         sItem->getCrossingKind(),
         turnBoundingbox );
      uint32 nbrPossTurns = sItem->getNbrPossTurns();
      uint32* possTurn = sItem->getPossibleTurns();
      for(uint32 j = 0 ; j < nbrPossTurns ; j++){
         routeItem->addPossibleTurn(
            ExpandedRouteItem::routeTurnFromStringCode(
               StringTable::stringCode(*(possTurn++))));
      }
      if(sItem->isEndOfRoadTurn()){
         routeItem->setEndOfRoad(true);
      }
      
      LandmarkLink* landmark = static_cast<LandmarkLink*>(
         sItem->getLandmarks()->first() );
      for ( uint32 j = 0 ; j < sItem->getNbrLandmarks() ; ++j ) {
         NameCollection* name = new NameCollection();
         name->addName( new Name( landmark->getLMName(), 
                                  LangTypes::english ) );
         ExpandedRouteLandmarkItem* landmarkItem = 
            new ExpandedRouteLandmarkItem( 
               landmark->m_lmdesc.type,
               landmark->m_lmdesc.location,
               landmark->m_lmdesc.side,
               landmark->isEndLM(),
               landmark->m_lmdesc.importance,
               landmark->m_lmDist,
               name,
               landmark->m_lmdesc.itemID);
         landmarkItem->setIsTraffic(landmark->isTraffic());
         landmarkItem->setIsStart(landmark->isStart());
         landmarkItem->setIsStop(landmark->isStop());
         landmarkItem->setIsDetour(landmark->isDetour());
         landmarkItem->addStreetName(landmark->getStreetName());
         
         routeItem->addExpandedRouteLandmarkItem( landmarkItem );
         landmark = static_cast<LandmarkLink*>( landmark->suc() );
      }
      addExpandedRouteItem( routeItem );

   }

   int groupSize = groupID.getSize();
   bool ramp = false;
   bool controlledAccess = false;
   bool driveOnRightSide = true;

   if ( hasAttributes ) {
      ExpandRouteReplyPacket::extractAttributes(
         attributes[ 0 ], controlledAccess, ramp, driveOnRightSide );
   }

   ExpandedRouteRoadItem* roadItem = new ExpandedRouteRoadItem(
      mapID[ 0 ],
      itemID[ 0 ],
      ItemTypes::secondClassRoad, // No information currently in reply
      new NameCollection( *m_routeItems[ 0 ]
                          ->getIntoRoadName() ), // TODO: Real name 
      hasSpeed ? speed[ 0 ] : MAX_UINT32, 
      hasSpeed ? speed[ 0 ] : MAX_UINT32,
      false, // multidigitialized
      ramp, false, //roundabout
      controlledAccess,
      false, // turn
      driveOnRightSide
      );
   if ( nbrStringItems > 1 ) {
      m_routeItems[ groupID[0] + 1 ]->addExpandedRouteRoadItem( roadItem );
   } else {
      // This is a single point route
      m_routeItems[ 0 ]->addExpandedRouteRoadItem( roadItem );
   }

   bool lastRamp = ramp;
   bool lastControlledAccess = controlledAccess;
   uint32 lastSpeed = hasSpeed ? speed[ 0 ] : MAX_UINT32;
   uint32 lastGroupIdx = groupID[0] + 1;

   
   for (int32 i=0; i < groupSize; ++i ) {
      // Add to the next group - not the current one. Start at doesn't
      // have a start coordinate.
      uint32 groupIdx = groupID[i] + 1;

      if ( groupIdx < getNbrExpandedRouteItems() ) {
         ExpandedRouteItem* el = m_routeItems[ groupIdx ];
         uint32 startIndex = offset[ i ];
         uint32 endIndex = exp->nextCoordStart( i );

         for ( uint32 j = startIndex ; j < endIndex ; ++j ) {            
            if ( hasAttributes ) {
               ExpandRouteReplyPacket::extractAttributes(
                  attributes[ i ], controlledAccess, ramp, 
                  driveOnRightSide );
            }
            if ( lastRamp != ramp || 
                 lastControlledAccess != controlledAccess ||
                 lastSpeed != (hasSpeed ? speed[ i ] : MAX_UINT32) ||
                 lastGroupIdx != groupIdx )
            {
               // If this is a new group we add the coordinate last in
               // the previous element too.
               if ( lastGroupIdx != groupIdx && ( (groupIdx - 1) >= 0 )) {
                  ExpandedRouteItem* prev = m_routeItems[ groupIdx -1 ];
                  if ( prev->getNbrExpandedRouteRoadItems() > 0 ) {
                     ExpandedRouteRoadItem* roadItem = 
                        const_cast<ExpandedRouteRoadItem*>( 
                           prev->getExpandedRouteRoadItem(
                              prev->getNbrExpandedRouteRoadItems() -1 ) );
                     roadItem->addCoordinate( MC2Coordinate( 
                        lat[ startIndex ], lon[ startIndex ] ) );
                  }

                  if ( prev->getNbrExpandedRouteRoadItems() > 0 ) {
                     // Hack to have one ExpandedRouteRoadItem with 
                     // turn == true
                     const ExpandedRouteRoadItem* last = 
                        prev->getExpandedRouteRoadItem( 
                           prev->getNbrExpandedRouteRoadItems() -1 );
                     if ( last->getNbrCoordinates() > 0 ) {
                        roadItem = new ExpandedRouteRoadItem(
                           last->getMapID(),
                           last->getNodeID(),
                           last->getRoadClass(),
                           new NameCollection( *last->getRoadName() ),
                           last->getPosSpeedLimit(),
                           last->getNegSpeedLimit(),
                           last->getMultidigitialized(),
                           last->getRamp(), 
                           last->getRoundabout(),
                           last->getControlledAccess(),
                           true, // is turn
                           last->getDriveOnRightSide(),
                           last->getStartLevel(),
                           last->getEndLevel(),
                           last->getPosEntryrestrictions(),
                           last->getNegEntryrestrictions() );
                        roadItem->addCoordinate( last->getCoordinate( 
                           last->getNbrCoordinates() -1 ) );
                        prev->addExpandedRouteRoadItem( roadItem );
                     }
                  }
               }
               // New item
               roadItem = new ExpandedRouteRoadItem(
                  mapID[ i ],
                  itemID[ i ],
                  ItemTypes::secondClassRoad, // No information currently in reply
                  new NameCollection( *m_routeItems[ groupIdx -1 ]
                     ->getIntoRoadName() ), // TODO: Real name 
                  hasSpeed ? speed[ i ] : MAX_UINT32, 
                  hasSpeed ? speed[ i ] : MAX_UINT32,
                  false, // multidigitialized
                  ramp, false, //roundabout
                  controlledAccess,
                  false, // turn
                  driveOnRightSide
                  );
               el->addExpandedRouteRoadItem( roadItem );
               lastRamp = ramp;
               lastControlledAccess = controlledAccess;
               lastSpeed = hasSpeed ? speed[ i ] : MAX_UINT32;
               lastGroupIdx = groupIdx;
            }
            roadItem->addCoordinate( MC2Coordinate( lat[ j ], lon[ j ] ) );
         }



      } else {
         mc2log << error << "ExpandedRoute::ExpandedRoute "
                << "groupID larger than vectorsize" << endl;
      }
   }

   // Add the last coordinate to the last RouteItem
   if ( nbrStringItems != 0) {
      ExpandedRouteItem* el = m_routeItems[ nbrStringItems -1 ];
      if ( el->getNbrExpandedRouteRoadItems() > 0 ) {
         ExpandedRouteRoadItem* roadItem = 
            const_cast<ExpandedRouteRoadItem*>( 
               el->getExpandedRouteRoadItem(
                  el->getNbrExpandedRouteRoadItems() -1 ) );
         roadItem->addCoordinate( MC2Coordinate( 
            p->getLastItemLat(), p->getLastItemLon() ) );
      }

      if ( el->getNbrExpandedRouteRoadItems() > 0 ) {
         // Hack to have one ExpandedRouteRoadItem with 
         // turn == true
         const ExpandedRouteRoadItem* last = 
            el->getExpandedRouteRoadItem( 
               el->getNbrExpandedRouteRoadItems() -1 );
         if ( last->getNbrCoordinates() > 0 ) {
            roadItem = new ExpandedRouteRoadItem(
               last->getMapID(),
               last->getNodeID(),
               last->getRoadClass(),
               new NameCollection( *last->getRoadName() ),
               last->getPosSpeedLimit(),
               last->getNegSpeedLimit(),
               last->getMultidigitialized(),
               last->getRamp(), 
               last->getRoundabout(),
               last->getControlledAccess(),
               true, // is turn
               last->getDriveOnRightSide(),
               last->getStartLevel(),
               last->getEndLevel(),
               last->getPosEntryrestrictions(),
               last->getNegEntryrestrictions() );
            roadItem->addCoordinate( last->getCoordinate( 
               last->getNbrCoordinates() -1 ) );
            el->addExpandedRouteRoadItem( roadItem );
         }
      }

      // Add lat,lon to start
      el = m_routeItems[ 0 ];
      if ( nbrStringItems > 1 ) {
         ExpandedRouteItem* next = m_routeItems[ 1 ];
         if ( next->getNbrExpandedRouteRoadItems() > 0  )
         {
            ExpandedRouteRoadItem* nextRoadItem = 
               const_cast<ExpandedRouteRoadItem*>( 
                  next->getExpandedRouteRoadItem( 0 ) );
            // Copy from first roaditem
            ExpandedRouteRoadItem* roadItem = new ExpandedRouteRoadItem(
               nextRoadItem->getMapID(),
               nextRoadItem->getNodeID(),
               nextRoadItem->getRoadClass(),
               new NameCollection( *el->getIntoRoadName() ),
               nextRoadItem->getPosSpeedLimit(),
               nextRoadItem->getNegSpeedLimit(),
               nextRoadItem->getMultidigitialized(),
               nextRoadItem->getRamp(), 
               nextRoadItem->getRoundabout(),
               nextRoadItem->getControlledAccess(),
               true, //At turn (start that is) nextRoadItem->getTurn(),
               nextRoadItem->getDriveOnRightSide(),
               nextRoadItem->getStartLevel(),
               nextRoadItem->getEndLevel(),
               nextRoadItem->getPosEntryrestrictions(),
               nextRoadItem->getNegEntryrestrictions() );
            if ( nextRoadItem->getNbrCoordinates() > 0 ) {
               roadItem->addCoordinate( nextRoadItem->getCoordinate( 0 ) );
            } else {
               roadItem->addCoordinate( MC2Coordinate( 
                  stringItems[ 0 ]->getLatitude(), 
                  stringItems[ 0 ]->getLongitude() ) );
            }
            el->addExpandedRouteRoadItem( roadItem );
         } else if ( hasCoordinates ) {
            // Get coordinates from ExpandStringItem lat,lon
            ExpandedRouteRoadItem* roadItem = new ExpandedRouteRoadItem(
               MAX_UINT32,
               MAX_UINT32,
               ItemTypes::secondClassRoad, // No information currently in reply
               new NameCollection( *next->getIntoRoadName() ),//TODO: Real name
               MAX_UINT32, // posspeedlimit
               MAX_UINT32, // negspeedlimit
               false, // multidigitialized
               false, // ramp
               false, //roundabout
               false, // controlledAccess 
               true,  // At turn (start that is)
               true   // driveOnRightSide
               );
            roadItem->addCoordinate( MC2Coordinate( 
               stringItems[ 0 ]->getLatitude(), 
               stringItems[ 0 ]->getLongitude() ) );
            el->addExpandedRouteRoadItem( roadItem );
         }
      } else { // nbrStringItems > 1
         if ( hasAttributes ) {
            ExpandRouteReplyPacket::extractAttributes(
               attributes[ 0 ], controlledAccess, ramp, driveOnRightSide );
         } else {
            ramp = false;
            controlledAccess = false;
            driveOnRightSide = true;
         }
         ExpandedRouteRoadItem* roadItem = new ExpandedRouteRoadItem(
            mapID[ 0 ],
            itemID[ 0 ],
            ItemTypes::secondClassRoad, // No information currently in reply
            new NameCollection( *m_routeItems[ 0 ]
                                ->getIntoRoadName() ), // TODO: Real name 
            hasSpeed ? speed[ 0 ] : MAX_UINT32, 
            hasSpeed ? speed[ 0 ] : MAX_UINT32,
            false, // multidigitialized
            ramp, false, //roundabout
            controlledAccess,
            true, // At turn (start that is)
            driveOnRightSide
            );
         if ( lat.getSize() > 0 ) {
            roadItem->addCoordinate( MC2Coordinate( lat[0], lon[0] ) );
         } else {
            roadItem->addCoordinate( MC2Coordinate( 
               stringItems[ 0 ]->getLatitude(), 
               stringItems[ 0 ]->getLongitude() ) );
         }
         el->addExpandedRouteRoadItem( roadItem );
      }
   }

   
   delete exp;
   for ( uint32 i = 0 ; i < nbrStringItems ; ++i ) {
      delete stringItems[ i ];
   }
   delete [] stringItems;
}


ExpandedRoute::~ExpandedRoute() {
   for ( uint32 i = 0 ; i < m_origins.size() ; ++i ) {
      delete m_origins[ i ];
   }
   for ( uint32 i = 0 ; i < m_destinations.size() ; ++i ) {
      delete m_destinations[ i ];
   }
   for ( uint32 i = 0 ; i < m_routeItems.size() ; ++i ) {
      delete m_routeItems[ i ];
   }
}


void 
ExpandedRoute::dump( ostream& out ) const {
   out << "###ExpandedRoute >>>" << endl
       << "# dist (m) " << m_dist << endl
       << "# time (s) " << m_time << endl
       << "# standstillTime (s) " << m_standstillTime << endl
       << "# lang " << StringTable::getString( 
          StringTable::getLanguageAsStringCode( m_lang ), 
          StringTable::ENGLISH ) << endl
       << "# compassLangShort " << BP(m_compassLangShort) << endl
       << "# maxTurnNumbers " << uint32(m_maxTurnNumbers) << endl
       << "# distFormat " << ( 
          m_distFormat == StringTableUtility::EXACT ? "EXACT" : 
          m_distFormat == StringTableUtility::NORMAL ? "NORMAL" : 
          m_distFormat == StringTableUtility::COMPACT ? "COMPACT" :
          "unknown" ) << endl
       << "# wapFormat " << BP(m_wapFormat) << endl
       << "# nameSeparator " << m_nameSeparator << endl
       << "# nbr origins " << m_origins.size() << endl
       << "# nbr destinations " << m_destinations.size() << endl
       << "# nbr ExpandedRouteItems " << m_routeItems.size() << endl;
   for ( uint32 i = 0 ; i < m_origins.size() ; ++i ) {
      out << "# origin " << i << endl;
      m_origins[ i ]->dump( out );
   }
   for ( uint32 i = 0 ; i < m_destinations.size() ; ++i ) {
      out << "# destination " << i << endl;
      m_destinations[ i ]->dump( out );
   }
   for ( uint32 i = 0 ; i < m_routeItems.size() ; ++i ) {
      out << "# ExpandedRouteItem " << i << endl;
      m_routeItems[ i ]->dump( out );
   }
   out << "###ExpandedRoute <<<" << endl;
}


uint32 
ExpandedRoute::getRouteID() const {
   return m_routeID;
}


void 
ExpandedRoute::setRouteID( uint32 routeID ) {
   m_routeID = routeID;
}


uint32 
ExpandedRoute::getRouteCreateTime() const {
   return m_routeCreateTime;
}


void 
ExpandedRoute::setRouteCreateTime( uint32 createTime ) {
   m_routeCreateTime = createTime;
}


uint32 
ExpandedRoute::getTotalDistance() const {
   return m_dist;
}


bool 
ExpandedRoute::getTotalDistanceStr( char* str, 
                                    uint32 maxLength ) const
{
   str[ 0 ] = '\0';
   uint32 nbrBytesWritten = 0;

   // Add "Total distance"
   if ( ! ExpandStringItem::addString(  
             StringTable::getString(
                StringTable::TOTAL_DISTANCE, m_lang ),
             str, maxLength, nbrBytesWritten, m_wapFormat)  ) 
   {
      return false;
   } 

   // Add space
   if ( ! ExpandStringItem::addString(
             " ", str, maxLength, nbrBytesWritten, m_wapFormat ) ) 
   {
      return false;
   }

   // Create a temporary buffer
   char distBuf[1024];
   StringTableUtility::printDistance( distBuf, getTotalDistance(), 
                                 m_lang, m_distFormat, m_distUnit );
   
   // Add the "5 km"
   if (! ExpandStringItem::addString(
            distBuf, str, maxLength, nbrBytesWritten, m_wapFormat  )  ) 
   {
      return false;
   }
   
   return true;
}


void 
ExpandedRoute::setTotalDistance( uint32 dist ) {
   m_dist = dist;
}


uint32 
ExpandedRoute::getTotalTime() const {
   return m_time;
}


bool 
ExpandedRoute::getTotalTimeStr( char* str, uint32 maxLength ) const {
   str[0] = '\0';
   uint32 nbrBytesWritten = 0;

   // Set the variables depending on if it's standstill time or not.
   const char* timeString = StringTable::getString(
      StringTable::TOTAL_TIME, m_lang );
   uint32 time = getTotalTime();
   
   // Add "Total time"
   if ( ! ExpandStringItem::addString(
             timeString, str, maxLength, nbrBytesWritten, m_wapFormat ) ) 
   {
      return false;
   } 

   // Add space
   if ( ! ExpandStringItem::addString(
             " ", str, maxLength, nbrBytesWritten, m_wapFormat) ) 
   {
      return false;
   }

   // Create a temporary buffer
   char timeBuf[16];
   StringUtility::splitSeconds( time, timeBuf );
   
   // Add time "14:12"
   return ExpandStringItem::addString( timeBuf, str, maxLength, 
                                       nbrBytesWritten, m_wapFormat );
}


void 
ExpandedRoute::setTotalTime( uint32 time ) {
   m_time = time;
}


uint32 
ExpandedRoute::getTotalStandStillTime() const {
   return m_standstillTime;
}


bool 
ExpandedRoute::getTotalStandstillTimeStr( char* str, 
                                          uint32 maxLength ) const 
{
   str[0] = '\0';
   uint32 nbrBytesWritten = 0;

   // Set the variables depending on if it's standstill time or not.
   const char* timeString = StringTable::getString(
      StringTable::TOTAL_STANDSTILL_TIME, m_lang );
   uint32 time = getTotalStandStillTime();
   
   // Add "Total time"
   if ( ! ExpandStringItem::addString(
             timeString, str, maxLength, nbrBytesWritten, m_wapFormat ) ) 
   {
      return false;
   } 

   // Add space
   if ( ! ExpandStringItem::addString(
             " ", str, maxLength, nbrBytesWritten, m_wapFormat) ) 
   {
      return false;
   }

   // Create a temporary buffer
   char timeBuf[16];
   StringUtility::splitSeconds( time, timeBuf );
   
   // Add time "14:12"
   return ExpandStringItem::addString( timeBuf, str, maxLength, 
                                       nbrBytesWritten, m_wapFormat );
}


void 
ExpandedRoute::setTotalStandStillTime( uint32 time ) {
   m_standstillTime = time;
}


bool 
ExpandedRoute::getAverageSpeedStr( char* str, uint32 maxLength ) const
{
   str[0] = '\0';
   uint32 nbrBytesWritten = 0;
   if ( getTotalTime() > 0 ) { // Avoid divide by zero
      // Add "Average speed"
      if ( ! ExpandStringItem::addString(  
                StringTable::getString( StringTable::AVERAGE_SPEED, 
                                        m_lang ),
                str, maxLength, nbrBytesWritten, m_wapFormat ) ) 
      { 
         return false;
      } 

      // Create a temporary buffer
      char speedBuf[16];
      
      // Compute average speed
      uint16 avgSpeed = (uint16)( Math::MS_TO_KMH * (
         ((float64) getTotalDistance()) /
         getTotalTime() ) );
      if(m_distUnit != StringTableUtility::METERS){
         sprintf(speedBuf, " %d", (int)(avgSpeed*0.62137));
         // Add the speed "55"
         if ( ! ExpandStringItem::addString(
                 speedBuf, str, maxLength, nbrBytesWritten, m_wapFormat  ) )
         {
            return false;
         }
         
         // Add space
         if ( ! ExpandStringItem::addString(
                 " ", str, maxLength, nbrBytesWritten, m_wapFormat) ) {
            return false;
         }
         
         // Add "mph"
         if ( ! ExpandStringItem::addString(
                 StringTable::getString( StringTable::MPH, 
                                         m_lang ),
                 str, maxLength, nbrBytesWritten, m_wapFormat  )  ) {
            return false;
         }
         
      
      } else {
         sprintf(speedBuf, " %d", avgSpeed);
         // Add the speed "70"
         if ( ! ExpandStringItem::addString(
                 speedBuf, str, maxLength, nbrBytesWritten, m_wapFormat  ) )
         {
            return false;
         }
         
         // Add space
         if ( ! ExpandStringItem::addString(
                 " ", str, maxLength, nbrBytesWritten, m_wapFormat) ) {
            return false;
         }
         
         // Add "km/h"
         if ( ! ExpandStringItem::addString(
                 StringTable::getString( StringTable::KM_PER_HOUR, 
                                         m_lang ),
                 str, maxLength, nbrBytesWritten, m_wapFormat  )  ) {
            return false;
         }
      }

   }
   return true;
}


ItemTypes::vehicle_t 
ExpandedRoute::getStartingVehicle() const {
   return m_startVehicle;
}


void 
ExpandedRoute::setStartingVehicle( ItemTypes::vehicle_t vehicle ) {
   m_startVehicle = vehicle;
}


const MC2BoundingBox& 
ExpandedRoute::getRouteBoundingBox() const {
   return m_routeBoundingbox;
}


uint32 
ExpandedRoute::getNbrOrigins() const {
   return m_origins.size();
}


const ExpandedRouteMatch* 
ExpandedRoute::getOrigin( uint32 i ) const {
   if ( i < m_origins.size() ) {
      return m_origins[ i ];
   } else {
      return NULL;
   }
}


void 
ExpandedRoute::addOrigin( ExpandedRouteMatch* match ) {
   m_origins.push_back( match );
}


uint32 
ExpandedRoute::getNbrDestinations() const {
   return m_destinations.size();
}


const ExpandedRouteMatch* 
ExpandedRoute::getDestination( uint32 i ) const {
   if ( i < m_destinations.size() ) {
      return m_destinations[ i ];
   } else {
      return NULL;
   }
}


void 
ExpandedRoute::addDestination( ExpandedRouteMatch* match ) {
   m_destinations.push_back( match );
}


uint32 
ExpandedRoute::getNbrExpandedRouteItems() const {
   return m_routeItems.size();
}


const ExpandedRouteItem* 
ExpandedRoute::getExpandedRouteItem( uint32 index ) const {
   if ( index < m_routeItems.size() ) {
      return m_routeItems[ index ];
   } else {
      return NULL;
   }
}


void 
ExpandedRoute::addExpandedRouteItem( ExpandedRouteItem* item ) {
   m_routeItems.push_back( item );
}


void 
ExpandedRoute::setRouteDescriptionProperties( 
   StringTable::languageCode lang,
   bool compassLangShort,
   byte maxTurnNumbers,
   bool printSignPostText,
   StringTableUtility::distanceFormat distFormat,
   StringTableUtility::distanceUnit distUnit,
   bool wapFormat,
   const char* nameSeparator )
{
   m_lang = lang;
   m_compassLangShort = compassLangShort;
   m_maxTurnNumbers = maxTurnNumbers;
   m_printSignPostText = printSignPostText;
   m_distFormat = distFormat;
   m_distUnit = distUnit;
   m_wapFormat = wapFormat;
   m_nameSeparator = nameSeparator;
}


bool 
ExpandedRoute::getRouteDescription( uint32 index, 
                                    char* buf, uint32 maxLength ) const
{
   if ( index < m_routeItems.size() ) {
      const ExpandedRouteItem* routeItem = m_routeItems[ index ];

      // TODO: Don't use ExpandStringItem, replace ExpandStringItem with
      //       This. EI. move it here when ExpandStringItem is retired.
      string name;
      const NameCollection* nameC = routeItem->getIntoRoadName();
      const ExpandedRouteMatch* origin = getOrigin( 0 );
      
      for ( uint32 i = 0 ; i < nameC->getSize() ; ++i ) {
         if ( i != 0 ) {
            // Add separator TODO: Setting for separator
            name += " / ";
         }
         name += nameC->getName( i )->getName();
      }
      uint32 possibleTurns[7];
      for(uint32 j = 0 ; j < routeItem->getNbrPossibleTurns() ; j++){
         possibleTurns[j] = uint32(ExpandedRouteItem::routeTurnToStringCode(
                                      routeItem->getPossibleTurn(j),
                                      routeItem->getTransportation()));
      }
      
      ExpandStringItem expItem( routeItem->getDist(),
                                routeItem->getTime(),
                                ExpandedRouteItem::routeTurnToStringCode(
                                   routeItem->getTurnType(), 
                                   routeItem->getTransportation() ),
                                name.c_str(),
                                routeItem->getTransportation(),
                                routeItem->getLat(),
                                routeItem->getLon(),
                                0, // nameType
                                routeItem->getTurnNumber(),
                                routeItem->getLanes(),
                                routeItem->getSignPosts(),
                                routeItem->getCrossingType(),
                                NULL,
                                routeItem->getNbrPossibleTurns(),
                                possibleTurns);
      DescProps descParam = ExpandStringItem::createRouteDescProps( 
         m_lang, m_compassLangShort, m_maxTurnNumbers, m_printSignPostText,
         m_distFormat, m_distUnit, m_wapFormat, origin->getDirectionOddEven(),
         origin->getDirectionHousenumber() );
                                              
      uint32 nbrBytesWritten = 0;
      return expItem.getRouteDescription( descParam, buf, maxLength,
                                          nbrBytesWritten );
   } else {
      return false;
   }
}


uint32 
ExpandedRoute::getNbrLandmarksForExpandedRouteItem( uint32 index ) const {
   if ( index < m_routeItems.size() ) {
      return m_routeItems[ index ]->getNbrExpandedRouteLandmarkItems();
   } else {
      return MAX_UINT32;
   } 
}


bool 
ExpandedRoute::getRouteLandmarkDescription( uint32 index, uint32 i,
                                            char* buf, uint32 maxLength ) const
{
   if ( index < m_routeItems.size() && 
        i < getNbrLandmarksForExpandedRouteItem( index ) )
   {
      const ExpandedRouteItem* routeItem = m_routeItems[ index ];
      const ExpandedRouteLandmarkItem* routeLandmarkItem = 
         routeItem->getExpandedRouteLandmarkItem( i );

      return makeRouteLandmarkDescription( 
         buf, maxLength, routeLandmarkItem, routeItem->getTurnType(),
         m_lang, m_distFormat, m_distUnit, routeItem->getTransportation(), 
         m_nameSeparator.c_str(), true, m_wapFormat );
   } else {
      return false;
   }
}


bool 
ExpandedRoute::makeRouteLandmarkDescription( 
   char* buf, uint32 maxLength,
   const ExpandedRouteLandmarkItem* land,
   ExpandedRouteItem::routeturn_t turn,
   StringTable::languageCode lang,
   StringTableUtility::distanceFormat distFormat,
   StringTableUtility::distanceUnit distUnit,
   ItemTypes::transportation_t transport,
   const char* nameSeparator,
   bool showDist,
   bool wapFormat )
{
   string name;
   const NameCollection* nameC = land->getName();

   for ( uint32 i = 0 ; i < nameC->getSize() ; ++i ) {
      if ( i != 0 ) {
         name += " ";
         name += nameSeparator;
         name += " ";
      }
      name += nameC->getName( i )->getName();
   }

   ItemTypes::lmdescription_t lmdesc;
   lmdesc.itemID = 0;
   lmdesc.importance = land->getImportance();
   lmdesc.side = land->getRoadSide();
   lmdesc.location = land->getLocation();
   lmdesc.type = land->getType();
   uint32 dist = land->getDist();
   if ( !showDist ) {
      dist = 0;
   }
   LandmarkLink landmark ( lmdesc, dist, false, // distUpdated
                           name.c_str() );
   landmark.setEndLM( land->getAtTurn() );
   if(land->isDetour())
      landmark.setDetour();
   landmark.setIsStart(land->isStart());
   landmark.setIsStop(land->isStop());
   if(land->isTraffic()){
      landmark.setIsTraffic();
      landmark.setStreetName(land->getStreetName());
   }
   
   
   DescProps descParam = ExpandStringItem::createRouteDescProps( 
      lang, true, 9, false, distFormat, distUnit,
      wapFormat, ItemTypes::unknown_oddeven_t, ItemTypes::unknown_nbr_t );

   //landmark.dump();
   uint32 nbrBytesWritten = 0;
   bool res = landmark.getRouteDescription( 
      descParam, buf, maxLength,
      nbrBytesWritten,
      ExpandedRouteItem::routeTurnToStringCode(
         turn, transport ) );

   MC2String capital = 
      StringUtility::makeFirstCapital( MC2String(buf) );
   strcpy( buf, capital.c_str() );

   return res;
}


template <class T>
class ExpandedRouteCountFunc {
public:
   ExpandedRouteCountFunc( uint32& size ) 
         : s( size ) {}

   void operator() (T* e) {
      s += e->getSizeAsBytes();
   }

   uint32& s;
};


uint32
ExpandedRoute::getSizeAsBytes() const {
   uint32 size = 0;
   // m_routeID, m_routeCreateTime, m_dist, m_standstillTime, m_startVehicle
   size += 6*4;
   // m_routeBoundingbox
   size += 4*1;

   for_each( m_origins.begin(), m_origins.end(), ExpandedRouteCountFunc<ExpandedRouteMatch>(
                size ) );
   for_each( m_destinations.begin(), m_destinations.end(), ExpandedRouteCountFunc<ExpandedRouteMatch>(
                size ) );

   for_each( m_routeItems.begin(), m_routeItems.end(), ExpandedRouteCountFunc<ExpandedRouteItem>(
                size ) );

   return size;
}

void
ExpandedRoute::load( Packet* p, int& pos ) {

}


template <class T>
class ExpandedRouteMatchFunc {
public:
   ExpandedRouteMatchFunc( Packet* pack, int& position ) 
         : p( pack ), pos( position ) {}

   void operator() (T* e) {
      e->save( p, pos );
   }

   Packet* p;   
   int& pos;
};

uint32
ExpandedRoute::save( Packet* p, int& pos ) const {
   uint32 startOffset = p->getLength();

   p->incWriteLong(pos, m_routeID);
   p->incWriteLong(pos, m_routeCreateTime);
   p->incWriteLong(pos, m_dist);
   p->incWriteLong(pos, m_time);
   p->incWriteLong(pos, m_standstillTime);
   p->incWriteLong(pos, m_startVehicle);
   p->incWriteBBox( pos, m_routeBoundingbox );

   for_each( m_origins.begin(), m_origins.end(), ExpandedRouteMatchFunc<ExpandedRouteMatch>(
                p, pos ) );
   for_each( m_destinations.begin(), m_destinations.end(), ExpandedRouteMatchFunc<ExpandedRouteMatch>(
                p, pos ) );

   for_each( m_routeItems.begin(), m_routeItems.end(), ExpandedRouteMatchFunc<ExpandedRouteItem>(
                p, pos ) );

   p->setLength(pos); 
   return (p->getLength() - startOffset);
}
