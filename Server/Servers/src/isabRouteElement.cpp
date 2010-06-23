/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*
 *   ********* This file will be compiled in mc2 and
 *             thinclient (linux and windows) *********
 *   This file is copied from mc2, where the original file is located.
 *   Change the mc2 version, or your changes will be overwritten.
 */

#include "isabRouteElement.h"

/*
 * write functions are in isabRouteElementMc2.cpp
 * constructors reading for databuffer are in isabRouteElementThinClient.cpp
 */

IsabRouteElement::IsabRouteElement(ObjectType_t objType) : m_objectType(objType)
{
}

IsabRouteElement::~IsabRouteElement()
{
   //no, read the paper for new:s... 
}


int
IsabRouteElement::write(uint8* buf,
                        int pos) const
{
// isabBoxNavMessageUtil::incWriteShort(buf, pos, m_dummy);
   return pos;
}

OrigoElement::OrigoElement(int32 origoLon,
                           int32 origoLat,
                           int16 nextOrigo)
      : IsabRouteElement(OrigoType), m_origoLon(origoLon),
        m_origoLat(origoLat), m_nextOrigo(nextOrigo)
{
   m_objectType = OrigoType;
}

OrigoElement::~OrigoElement()
{
   // no news
}

void
OrigoElement::setNextOrigo(uint16 pos)
{
   m_nextOrigo = pos;
}


WPTorTPTElement::WPTorTPTElement( 
      ObjectType_t objType, 
      int16 lon, 
      int16 lat, 
      uint8 flags, 
      uint16 meters,
      uint8 speedlimit,
      uint16 nameIndex ) : 
   IsabRouteElement(objType), 
	m_lon(lon), m_lat(lat), m_flags(flags),
   m_speedLimit(speedlimit), m_meters(meters), m_nameIndex( nameIndex )
{
}

WPTorTPTElement::WPTorTPTElement( 
      ObjectType_t objType) :
   IsabRouteElement(objType)
{
}


WPTorTPTElement::~WPTorTPTElement()
{
}


void 
WPTorTPTElement::setNameIndex( uint16 nameIndex ) {
   m_nameIndex = nameIndex;
}


WPTElement::WPTElement(uint16 type,
                       uint8 speedLimit,
                       int16 lon,
                       int16 lat,
		       uint8 flags,
                       uint16 meters,
                       uint16 nameIndex,
                       uint8 exitCount,
                       RouteElement::navcrossingkind_t crossingKind) :
   WPTorTPTElement(WPTType, lon, lat, flags, meters, speedLimit,nameIndex),
   m_type(type), m_exitCount(exitCount),
   m_crossingKind(crossingKind)
{
}

WPTElement::~WPTElement()
{
   // Don't like new...
}

TPTElement::TPTElement(int16 lon,
                       int16 lat,
		       uint8 flags,
                       uint16 meters,
                       uint8 speedlimit)
      : WPTorTPTElement(TPTType, lon, lat, flags, meters, speedlimit, 0)
{
}

TPTElement::~TPTElement()
{
   m_objectType = TPTType;
}


ScaleElement::ScaleElement(uint32 integerPart,
                           uint32 restPart,
                           uint16 refLon,
                           uint16 refLat)
      : IsabRouteElement(ScaleType), m_integerPart(integerPart), m_restPart(restPart),
        m_refLon(refLon), m_refLat(refLat)
{
   m_objectType = ScaleType;
}

ScaleElement::~ScaleElement()
{

}

MiniElement::MiniElement() : IsabRouteElement(MiniType)
{
   m_lon1 = 0;
   m_lon2 = 0;
   m_lat1 = 0;
   m_lat2 = 0;
   m_objectType = MiniType;
}


MiniElement::MiniElement(int16 lon1,
                         int16 lat1,
                         int16 lon2,
                         int16 lat2,
                         uint8 speedLimit1,
                         uint8 speedLimit2)
      : IsabRouteElement(MiniType), m_lon1(lon1), m_lat1(lat1), m_lon2(lon2), m_lat2(lat2),
        m_speedLimit1(speedLimit1), m_speedLimit2(speedLimit2)
{
   
}
   
    
MiniElement::~MiniElement()
{
   // nope
}

void
MiniElement::set1(int16 lon, int16 lat)
{
   m_lon1 = lon;
   m_lat1 = lat;
}

void
MiniElement::setSpeedLimit1(uint8 speedLimit)
{
   m_speedLimit1 = speedLimit;
}

void
MiniElement::set2(int16 lon, int16 lat)
{
   m_lon2 = lon;
   m_lat2 = lat;
}

void
MiniElement::setSpeedLimit2(uint8 speedLimit)
{
   m_speedLimit2 = speedLimit;
}


MicroDeltaElement::MicroDeltaElement() : IsabRouteElement(MicroDeltaType)
{
   m_x1 = 0;
   m_x2 = 0;
   m_x3 = 0;
   m_x4 = 0;
   m_x5 = 0;
   m_y1 = 0;
   m_y2 = 0;
   m_y3 = 0;
   m_y4 = 0;
   m_y5 = 0;
   m_objectType = MicroDeltaType;
}
   

MicroDeltaElement::MicroDeltaElement(int8  x1,
                                     int8  y1,
                                     int8  x2,
                                     int8  y2,
                                     int8  x3,
                                     int8  y3,
                                     int8  x4,
                                     int8  y4,
                                     int8  x5,
                                     int8  y5) :
      IsabRouteElement(MicroDeltaType), 
      m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2),
      m_x3(x3), m_y3(y3), m_x4(x4), m_y4(y4), m_x5(x5), m_y5(y5)
{

}
      
MicroDeltaElement::~MicroDeltaElement()
{
   // nope
}

void
MicroDeltaElement::setDelta1(int8 x, int8 y)
{
   m_x1 = x;
   m_y1 = y;
}


void
MicroDeltaElement::setDelta2(int8 x, int8 y)
{
   m_x2 = x;
   m_y2 = y;
}


void
MicroDeltaElement::setDelta3(int8 x, int8 y)
{
   m_x3 = x;
   m_y3 = y;
}


void
MicroDeltaElement::setDelta4(int8 x, int8 y)
{
   m_x4 = x;
   m_y4 = y;
}


void
MicroDeltaElement::setDelta5(int8 x, int8 y)
{
   m_x5 = x;
   m_y5 = y;
}
   

/* TimeDistLeftElement            */


TimeDistLeftElement::TimeDistLeftElement( 
   uint32 timeLeft, uint32 distLeft )
      : IsabRouteElement( TimeDistLeftType ),
        m_timeLeft( timeLeft ), m_distLeft( distLeft )
{
}


TimeDistLeftElement::~TimeDistLeftElement() {
   // No buffers to delete
}


uint32
TimeDistLeftElement::getTimeLeft() const {
   return m_timeLeft;
}



void 
TimeDistLeftElement::setTimeLeft( uint32 time) {
   m_timeLeft = time;
}


uint32
TimeDistLeftElement::getDistLeft() const {
   return m_distLeft;
}


/* MetaPTElement */


MetaPTElement::MetaPTElement( metapoint_t type, 
                              uint16 a, uint16 b, uint16 c, uint16 d )
   : IsabRouteElement( MetaPTType ),
     m_type( type ), m_dataa( a ), m_datab( b ), m_datac( c ), m_datad( d )
{
}


MetaPTElement::~MetaPTElement() {
   // Nothing newed in constructor
}


MetaPTElement::metapoint_t
MetaPTElement::getType() const {
   return m_type;
}


uint16
MetaPTElement::getDataA() const {
   return m_dataa;
}


uint16
MetaPTElement::getDataB() const {
   return m_datab;
}


uint16
MetaPTElement::getDataC() const {
   return m_datac;
}


uint16
MetaPTElement::getDataD() const {
   return m_datad;
}


void 
MetaPTElement::setDataA( uint16 value ) {
   m_dataa = value;
}


void 
MetaPTElement::setDataB( uint16 value ) {
   m_datab = value;
}


void 
MetaPTElement::setDataC( uint16 value ) {
   m_datac = value;
}


void 
MetaPTElement::setDataD( uint16 value ) {
   m_datad = value;
}

/* LandmarkPTElement */


LandmarkPTElement::LandmarkPTElement(bool detour,
                                     uint8 lm_side,
                                     uint8 lm_type,
                                     uint8 lm_location,
                                     int32 distance,
                                     uint16 strNbr,
                                     bool isStart,
                                     bool isStop,
                                     int16 landmarkID,
                                     uint32 distID)
   : IsabRouteElement( LandmarkPTType )
{
   // Set flags
   if(detour)
      m_flags = 0x0000;
   else
      m_flags = 0x2000;
   
   m_flags =  (m_flags | 
               (lm_side << 10) |  // Side 3
               (lm_type << 5) |// LM_t 5
               (lm_location));
   
   // Set distance 
   m_distance   = distance;
   
   m_strNameIdx = strNbr;
   

   // Set idStartStop
   m_idStartStop = 0x3fff & landmarkID; //  To make sure we only use 14 bits
   if(isStart)
      m_idStartStop |= 0x8000;
   if(isStop)
      m_idStartStop |= 0x4000;

   // disturbance ID.
   m_distID = distID;
}


LandmarkPTElement::~LandmarkPTElement()
{}

uint32
LandmarkPTElement::getLandmarkRouteID() const
{
   return m_idStartStop & 0x3fff;
}

uint32
LandmarkPTElement::getLandmarkServerID() const
{
   return m_distID; 
}

void
LandmarkPTElement::setisStart( bool on ) {
   if ( on ) {
      m_idStartStop |= 0x8000;
   } else {
      m_idStartStop &= ~0x8000;
   }
}

void
LandmarkPTElement::setisEnd( bool on ) {
   if ( on ) {
      m_idStartStop |= 0x4000;
   } else {
      m_idStartStop &= ~0x4000;
   }
}

bool
LandmarkPTElement::getisStart() const {
   return m_idStartStop & 0x8000;
}

bool
LandmarkPTElement::getisEnd() const {
   return m_idStartStop & 0x4000;
}

void
LandmarkPTElement::setisDetour( bool on ) {
   if ( on ) {
      m_flags &= 0x1FFF;
   } else {
      m_flags |= 0x2000;
   }
}

bool
LandmarkPTElement::getisDetour() const {
   return !(m_flags & 0x2000);
}

void
LandmarkPTElement::setDistance( int32 distance ) {
   m_distance = distance;
}

int32
LandmarkPTElement::getDistance() const {
   return m_distance;
}

uint16
LandmarkPTElement::getStrNameIdx() const {
   return m_strNameIdx;
}


///////////////// LaneInfoPTElement ///////////////////////////////////////


LaneInfoPTElement::LaneInfoPTElement( bool stopOfLanesFlag,
                                      bool reminderOfLanesFlag,
                                      uint8 nbrLanes,
                                      uint8 lane0,
                                      uint8 lane1,
                                      uint8 lane2,
                                      uint8 lane3,
                                      int32 distance )
      : IsabRouteElement( LaneInfoPTType )
{
   m_flags = (stopOfLanesFlag) | (reminderOfLanesFlag<<1 );
   m_nbrLanes = nbrLanes;
   m_lane0 = lane0;
   m_lane1 = lane1;
   m_lane2 = lane2;
   m_lane3 = lane3;
   m_distance = distance;
}

LaneInfoPTElement::~LaneInfoPTElement() {
}


///////////////// LaneDataPTElement ///////////////////////////////////////


LaneDataPTElement::LaneDataPTElement( uint8 lane0,
                                      uint8 lane1,
                                      uint8 lane2,
                                      uint8 lane3,
                                      uint8 lane4,
                                      uint8 lane5,
                                      uint8 lane6,
                                      uint8 lane7,
                                      uint8 lane8,
                                      uint8 lane9 )
      : IsabRouteElement( LaneDataPTType )
{
   m_lane0 = lane0;
   m_lane1 = lane1;
   m_lane2 = lane2;
   m_lane3 = lane3;
   m_lane4 = lane4;
   m_lane5 = lane5;
   m_lane6 = lane6;
   m_lane7 = lane7;
   m_lane8 = lane8;
   m_lane9 = lane9;
}

LaneDataPTElement::~LaneDataPTElement() {
}

///////////////// SignPostPTElement ///////////////////////////////////////


SignPostPTElement::SignPostPTElement( uint16 signPostTextIndex,
                                      byte textColor,
                                      byte backgroundColor,
                                      byte frontColor,
                                      int32 distance )
      : IsabRouteElement( SignPostPTType )
{
   m_signPostTextIndex = signPostTextIndex;
   m_textColor = textColor;
   m_backgroundColor = backgroundColor;
   m_frontColor = frontColor;
   m_reserved = 0;
   m_distance = distance;
}

SignPostPTElement::~SignPostPTElement() {
}
