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
 *    Stuff that will not really work in mc2.
 *
 *   ********* This file will be compiled in mc2 and
 *             thinclient (linux and windows) *********
 *   This file is copied from mc2, where the original file is located.
 *   Change the mc2 version, or your changes will be overwritten.
 */

#include "isabRouteElement.h"



#ifndef THINCLIENT
//  differences in databuffer implementation.
#   define readNextUnalignedLong  readNextLong
#   define readNextUnalignedShort readNextShort
#endif

IsabRouteElement*
IsabRouteElement::createFromBuf(DataBuffer* buf) {
   // read the action field, then call subclass
   uint16 typefield = buf->readNextUnalignedShort();
   PointType switchVal = nav_meta_point_max;
   if (typefield & 0x8000) {
      // high bit set, meta point.
      switchVal = static_cast<PointType>(typefield);
   } else {
      // action, filter away crossing and exitcount.
      switchVal = static_cast<PointType>(typefield & 0x3ff);
   }
//   PointType action = static_cast<PointType>(typefield & 0x03ff);

   switch (switchVal) {
      case ORIGO: {
         return new OrigoElement(buf);
      } break;
      case SCALE: {
         return new ScaleElement(buf);
      } break;
      case FULL_TPT: {
         return new TPTElement(buf);
      } break;
      case nav_meta_point_mini_delta_points: {
         return new MiniElement(buf);
      } break;
      case nav_meta_point_micro_delta_points: {
         return new MicroDeltaElement(buf);
      } break;
      case nav_route_point_end://     case EPT:
      case nav_route_point_start://      case SPT:
      case nav_route_point_ahead:
      case nav_route_point_left:
      case nav_route_point_right:
      case nav_route_point_uturn:
      case nav_route_point_startat:
      case nav_route_point_finally:
      case nav_route_point_enter_rdbt:
      case nav_route_point_exit_rdbt:
      case nav_route_point_ahead_rdbt:
      case nav_route_point_left_rdbt:
      case nav_route_point_right_rdbt:
      case nav_route_point_exitat:
      case nav_route_point_on:
      case nav_route_point_park_car:
      case nav_route_point_keep_left:
      case nav_route_point_keep_right:
      {
         return new WPTElement(buf, typefield);
      } break;

      case nav_route_point_start_with_uturn:
      case nav_rotue_point_uturn_rdbt:
      case nav_route_point_follow_road:
      case nav_route_point_enter_ferry:
      case nav_route_point_exit_ferry:
      case nav_route_point_change_ferry:
      {
         return new WPTElement(buf, typefield);
      } break;

      case nav_meta_point_time_dist_left_points:
      {
         return new TimeDistLeftElement( buf );
      } break;
      case nav_meta_point_meta:
         return new MetaPTElement( buf );
         break;
      case nav_meta_point_landmark:
         return new LandmarkPTElement( buf );
         break;

      case nav_meta_point_lane_info :
         return new LaneInfoPTElement( buf );
         break;
      case nav_meta_point_lane_data :
         return new LaneDataPTElement( buf );
         break;

      case nav_meta_point_signpost :
         return new SignPostPTElement( buf );
         break;
         
      case nav_route_point_max:
      case nav_meta_point_max:
      {
#ifdef THINCLIENT
         showMessage(
            TEXT("sorry, action not implemented in createFromBuf,="),
            switchVal);
#endif
}
      // NO default here!!
   }
#ifdef THINCLIENT
   showMessage(TEXT("Unhandled action in createFromBuf, ="), switchVal);
#endif
   return NULL;
}

OrigoElement::OrigoElement(DataBuffer* buf) : IsabRouteElement(OrigoType)
{
   m_nextOrigo = buf->readNextUnalignedShort();
   m_origoLon = buf->readNextUnalignedLong();
   m_origoLat = buf->readNextUnalignedLong();
}

WPTElement::WPTElement(DataBuffer* buf, uint16 pointDesc) : WPTorTPTElement(WPTType)
{
//   uint16 pointDesc = buf->readNextShort();
   // try to decode pointDesc
   m_exitCount = static_cast<uint8>((pointDesc & (0x7 << 10)) >> 10);
   m_crossingKind = static_cast<RouteElement::navcrossingkind_t>
      ((pointDesc & (0x3 << 13)) >> 13);
   m_type = static_cast<uint16>(pointDesc & (0xFFFF - (0x7 << 10) - (0x3 << 13)));
   
   m_flags = buf->readNextByte();
   m_speedLimit = buf->readNextByte();
   m_lon = buf->readNextUnalignedShort();
   m_lat = buf->readNextUnalignedShort();
   m_meters = buf->readNextUnalignedShort();
   m_nameIndex = buf->readNextUnalignedShort();
}

TPTElement::TPTElement(DataBuffer* buf) : WPTorTPTElement(TPTType) 
{
   m_flags = buf->readNextByte();
   m_speedLimit = buf->readNextByte();
   m_lon = buf->readNextUnalignedShort();
   m_lat = buf->readNextUnalignedShort();
   m_meters = buf->readNextUnalignedShort();
   m_nameIndex = buf->readNextUnalignedShort();
}

ScaleElement::ScaleElement(DataBuffer* buf) : IsabRouteElement(ScaleType)
{
   m_refLon = buf->readNextUnalignedShort();
   m_refLat = buf->readNextUnalignedShort();
   m_restPart = buf->readNextUnalignedShort();
   m_integerPart = buf->readNextUnalignedLong();
}

MiniElement::MiniElement(DataBuffer* buf) : IsabRouteElement(MiniType)
{
   m_lon1 = buf->readNextUnalignedShort();
   m_lat1 = buf->readNextUnalignedShort();
   m_lon2 = buf->readNextUnalignedShort();
   m_lat2 = buf->readNextUnalignedShort();
   m_speedLimit1 = buf->readNextByte();
   m_speedLimit2 = buf->readNextByte();
   // test some stuff
/*   if ((m_lon1 == 0) && (m_lat1 == 0)) {
#ifdef THINCLIENT
      showMessage(
         TEXT("warning, optimize: all points invalid in MiniElement"));
#endif
   }
*/
}

MicroDeltaElement::MicroDeltaElement(DataBuffer* buf) : IsabRouteElement(MicroDeltaType)
{
   m_x1 = buf->readNextByte();
   m_y1 = buf->readNextByte();
   m_x2 = buf->readNextByte();
   m_y2 = buf->readNextByte();
   m_x3 = buf->readNextByte();
   m_y3 = buf->readNextByte();
   m_x4 = buf->readNextByte();
   m_y4 = buf->readNextByte();
   m_x5 = buf->readNextByte();
   m_y5 = buf->readNextByte();
   // test some stuff
   if ((m_x1 == 0) && (m_y1 == 0)) {
#ifdef THINCLIENT
      showMessage(
         TEXT("warning, optimize: all points invalid in MicroDeltaElement"));
#endif
   } else if ((m_x2 == 0) && (m_y2)) {
#ifdef THINCLIENT
      showMessage(
         TEXT("warning, optimize: 3 points invalid in MicroDeltaElement"));
#endif
   }
}


TimeDistLeftElement::TimeDistLeftElement( DataBuffer* buf ) 
      : IsabRouteElement( TimeDistLeftType )
{
   m_timeLeft = buf->readNextUnalignedLong();
   m_distLeft = buf->readNextUnalignedLong();
   buf->readNextUnalignedShort(); // Reserved
}


MetaPTElement::MetaPTElement( DataBuffer* buf ) 
      : IsabRouteElement( MetaPTType )
{
   m_type = metapoint_t( buf->readNextUnalignedShort() );
   m_dataa = buf->readNextUnalignedShort();
   m_datab = buf->readNextUnalignedShort();
   m_datac = buf->readNextUnalignedShort();
   m_datad = buf->readNextUnalignedShort();
}

LandmarkPTElement::LandmarkPTElement(DataBuffer* buf)
      : IsabRouteElement(LandmarkPTType)
{
   m_flags       = buf->readNextUnalignedShort();
   m_strNameIdx  = buf->readNextUnalignedShort();
   m_idStartStop = buf->readNextUnalignedShort();
   m_distance    = buf->readNextUnalignedLong();
}

LaneInfoPTElement::LaneInfoPTElement( DataBuffer* buf )
      : IsabRouteElement( LaneInfoPTType )
{
   m_flags = buf->readNextByte();
   m_nbrLanes = buf->readNextByte();
   m_lane0 = buf->readNextByte();
   m_lane1 = buf->readNextByte();
   m_lane2 = buf->readNextByte();
   m_lane3 = buf->readNextByte();
   m_distance = buf->readNextUnalignedLong();
}

LaneDataPTElement::LaneDataPTElement( DataBuffer* buf )
      : IsabRouteElement( LaneDataPTType )
{
   m_lane0 = buf->readNextByte();
   m_lane1 = buf->readNextByte();
   m_lane2 = buf->readNextByte();
   m_lane3 = buf->readNextByte();
   m_lane4 = buf->readNextByte();
   m_lane5 = buf->readNextByte();
   m_lane6 = buf->readNextByte();
   m_lane7 = buf->readNextByte();
   m_lane8 = buf->readNextByte();
   m_lane9 = buf->readNextByte();
}

SignPostPTElement::SignPostPTElement( DataBuffer* buf )
      : IsabRouteElement( SignPostPTType )
{
   m_signPostTextIndex = buf->readNextUnalignedLong();
   m_textColor = buf->readNextByte();
   m_backgroundColor = buf->readNextByte();
   m_frontColor = buf->readNextByte();
   m_reserved = buf->readNextByte();
   m_distance = buf->readNextUnalignedLong();
}


#ifdef THINCLIENT
// Some fake write function implementations, to make windows happy:

int
OrigoElement::write(uint8* buf, int pos) const
{
   return pos;   
}

int
WPTElement::write(uint8* buf,
                  int pos) const
{
   return pos;
}

int
TPTElement::write(uint8* buf,
                  int pos) const
{
   return pos;
}

int
ScaleElement::write(uint8* buf,
                    int pos) const
{
   return pos;
}

int
MiniElement::write(uint8* buf, int pos) const
{
   return pos;
}

int
MicroDeltaElement::write(uint8* buf, int pos) const
{
   return pos;
}

int
TimeDistLeftElement::write( uint8* buf, int pos ) const {
   return pos;
}

int
MetaPTElement::write( uint8* buf, int pos ) const {
   return pos;
}

int
LandmarkPTElement::write( uint8* buf, int pos ) const {
   return pos;
}

int
LandmarkPTElement::write( uint8* buf, int pos ) const {
   return pos;
}

int
LaneInfoPTElement::write( uint8* buf, int pos ) const {
   return pos;
}

int
LaneDataPTElement::write( uint8* buf, int pos ) const {
   return pos;
}

int
SignPostPTElement::write( uint8* buf, int pos ) const {
   return pos;
}



#endif
