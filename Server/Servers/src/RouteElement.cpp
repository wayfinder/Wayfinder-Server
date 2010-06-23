/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RouteElement.h"
#include "GfxConstants.h"
#include "ExpandedRouteLandmarkItem.h"

#include <math.h>

RouteElement::RouteElement()
{
   // Set impossible values
   m_textIdx = -1;
   m_signIdx = -1;   
   m_totTime = 0;
   m_trans = ItemTypes::drive;
}

RouteElement::~RouteElement()
{
   // No news yet
   for ( uint32 i = 0 ; i < m_landmarks.size() ; ++i ) {
      delete m_landmarks[ i ];
   }
}

void
RouteElement::setText(int index)
{
   m_textIdx = index;
}


int
RouteElement::getText() const
{
   return m_textIdx;
}


void
RouteElement::setSignPostText(int index)
{
   m_signIdx = index;
}

bool
RouteElement::hasSignPostText() const
{
   return m_signIdx != -1;
}

int
RouteElement::getSignPostText() const
{
   return m_signIdx;
}

void
RouteElement::addCoord(uint32 lat,
                       uint32 lon)
{
   m_latVect.addLast(lat);
   m_lonVect.addLast(lon);
}

void
RouteElement::addSpeed(uint8 speed)
{
   m_speedVect.addLast(speed);
}


void
RouteElement::addAttribute(uint8 attribute)
{
   m_attrVect.addLast(attribute);
}


void
RouteElement::setDist(uint32 dist)
{
   m_dist = dist;
}

uint32
RouteElement::getDist() const
{
   return m_dist;
}

void
RouteElement::setTurnCode(StringTable::stringCode turn)
{
   m_turn = turn;
}

StringTable::stringCode
RouteElement::getTurnCode() const
{
   return m_turn;
}

void
RouteElement::setExitCount(byte count)
{
   m_exitCount = count;
}

byte
RouteElement::getExitCount() const
{
   return m_exitCount;
}

void 
RouteElement::convertAndSetCrossingKind(ItemTypes::crossingkind_t t)
{
   switch (t) {
      case ItemTypes::UNDEFINED_CROSSING :
      case ItemTypes::NO_CROSSING :
      case ItemTypes::CROSSING_2ROUNDABOUT :
         return (setCrossingKind(NO_CROSSING));
      case ItemTypes::CROSSING_3WAYS_T :
      case ItemTypes::CROSSING_3WAYS_Y :
      case ItemTypes::CROSSING_3ROUNDABOUT :
         return (setCrossingKind(THREE_WAY));
      case ItemTypes::CROSSING_4WAYS :
      case ItemTypes::CROSSING_4ROUNDABOUT :
      case ItemTypes::CROSSING_4ROUNDABOUT_ASYMMETRIC :
         return (setCrossingKind(FOUR_WAY));
      case ItemTypes::CROSSING_5WAYS :
      case ItemTypes::CROSSING_6WAYS :
      case ItemTypes::CROSSING_7WAYS :
      case ItemTypes::CROSSING_8WAYS :
      case ItemTypes::CROSSING_5ROUNDABOUT :
      case ItemTypes::CROSSING_6ROUNDABOUT :
      case ItemTypes::CROSSING_7ROUNDABOUT :
         return (setCrossingKind(MULTI_WAY));         
     default:
        return (setCrossingKind(NO_CROSSING));                 
   }
}

void 
RouteElement::setCrossingKind(navcrossingkind_t t)
{
  m_crossingKind = t;
}

RouteElement::navcrossingkind_t 
RouteElement::getCrossingKind() const
{  
   return (m_crossingKind);
}


uint32
RouteElement::getLastLat() const
{
   return m_latVect[m_latVect.getSize() - 1];
}

uint32
RouteElement::getLastLon() const
{
   return m_lonVect[m_lonVect.getSize() - 1];
}

uint32
RouteElement::getLatAt(int idx) const
{
   return m_latVect[idx];
}

uint32
RouteElement::getLonAt(int idx) const
{
   return m_lonVect[idx];
}

uint8
RouteElement::getLastSpeed() const
{
   return m_latVect[m_speedVect.getSize() - 1];
}

uint8
RouteElement::getSpeedAt(int idx) const
{
   return m_speedVect[idx];
}


uint8
RouteElement::getAttributeAt(int idx) const
{
   return m_attrVect[idx];
}

uint8
RouteElement::getAttributeSafeAt( int idx ) const {
   if ( idx >= 0 && uint32(idx) < m_attrVect.size() ) {
      return m_attrVect[ idx ];
   } else {
      return 0;
   }
}

int32
RouteElement::getDiffDist() const
{
   /* Calculate the distance the same way as the eBox does */

   /* First convert the lats and lons to milliminutes and truncate */

   int32 lat1mm = (int32)( getLatAt(0)
                       * GfxConstants::invDegreeFactor * 60.0 * 1000.0 );
   int32 lon1mm = (int32)( getLonAt(0)
                       * GfxConstants::invDegreeFactor * 60.0 * 1000.0 );
   int32 lat2mm = (int32)( getLastLat()
                       * GfxConstants::invDegreeFactor * 60.0 * 1000.0 );
   int32 lon2mm = (int32)( getLastLon()
                       * GfxConstants::invDegreeFactor * 60.0 * 1000.0 );

   /* Then convert to radians */

   const double MMINUTES_TO_RAD = M_PI/10800000.0;
   
   double lat1 = lat1mm * MMINUTES_TO_RAD;
   double lon1 = lon1mm * MMINUTES_TO_RAD;
   double lat2 = lat2mm * MMINUTES_TO_RAD;
   double lon2 = lon2mm * MMINUTES_TO_RAD;
      
   double y = 1852.0 * 60.0 * 180.0 * (lat1 - lat2) / M_PI;  
   double x = 1852.0 * 60.0 * 180.0 * (lon1 - lon2) / M_PI * cos(lat2);
   
   return (int32)(getDist() - sqrt( x * x + y * y ) );
                      
}

int
RouteElement::getNbrCoord() const
{
   return m_lonVect.getSize();
}


uint32 
RouteElement::getTime() const {
   return m_time;
}


void 
RouteElement::setTime( uint32 time ) {
   m_time = time;
}


uint32 
RouteElement::getTotalTimeSoFar() const {
   return m_totTime;
}


void 
RouteElement::setTotalTimeSoFar( uint32 totalTime ) {
   m_totTime = totalTime;
}


uint32
RouteElement::getNbrLandmarks() const {
   return m_landmarks.size();
}


const ExpandedRouteLandmarkItem*
RouteElement::getLandmark( uint32 i ) const {
   return m_landmarks[ i ];
}


void
RouteElement::addLandmark( ExpandedRouteLandmarkItem* landmark ) {
   m_landmarks.push_back( landmark );
}


void
RouteElement::setTransportationType( ItemTypes::transportation_t trans ) {
   m_trans = trans; 
}


ItemTypes::transportation_t
RouteElement::getTransportationType() const {
   return m_trans;
}
