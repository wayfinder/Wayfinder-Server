/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "isabRouteElement.h"
#include "isabBoxNavMessage.h"

int
OrigoElement::write(uint8* buf, int pos) const
{
   // Write the Action type
   pos = isabBoxNavMessageUtil::incWriteShort(buf,
                                              pos,
                                              ORIGO);     
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_nextOrigo);
   pos = isabBoxNavMessageUtil::incWriteLong(buf, pos, m_origoLon);
   pos = isabBoxNavMessageUtil::incWriteLong(buf, pos, m_origoLat);
   
   return pos;   
}

int
WPTElement::write(uint8* buf,
                  int pos) const
{
   // Write the Action type
   uint16 pointDesc = m_type & 0x07ff;
   // Include the exitcount, only has 3 bits for this so be carefull
   if (m_exitCount < 8 ) {
      pointDesc |= (m_exitCount << 10);
      mc2dbg8 << "Adding EXITCOUNT" << endl;
   }
   // Include the crossing kind. Has only 2 bits!
   pointDesc |= ( byte(m_crossingKind) << 13);
   mc2dbg8 << "Added CROSSINGKIND: " << int(m_crossingKind) << endl;
   mc2dbg8 << "pointDesc = 0x" << hex << pointDesc << dec << endl;
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, pointDesc);
   
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_flags);
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_speedLimit);
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_lon);
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_lat);
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_meters);
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_nameIndex);
  
   return pos;
}

int
TPTElement::write(uint8* buf,
                  int pos) const
{
   // Write the Action type
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, FULL_TPT);   
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_flags);
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_speedLimit);
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_lon);
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_lat);
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_meters);
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_nameIndex);

   return pos;
}

int
ScaleElement::write(uint8* buf,
                    int pos) const
{
   // Write the Action type
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, SCALE);      
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_refLon);
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_refLat);
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_restPart);
   pos = isabBoxNavMessageUtil::incWriteLong(buf, pos, m_integerPart);

   return pos;
}

int
MiniElement::write(uint8* buf, int pos) const
{
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, nav_meta_point_mini_delta_points);   
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_lon1);      
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_lat1);
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_lon2);
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, m_lat2);
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_speedLimit1);
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_speedLimit2);
   return pos;
}

int
MicroDeltaElement::write(uint8* buf, int pos) const
{
   pos = isabBoxNavMessageUtil::incWriteShort(buf, pos, nav_meta_point_micro_delta_points);   
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_x1);
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_y1);
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_x2);
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_y2);
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_x3);
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_y3);
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_x4);
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_y4);
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_x5);
   pos = isabBoxNavMessageUtil::incWriteByte(buf, pos, m_y5);
   return pos;
}


int
TimeDistLeftElement::write( uint8* buf, int pos ) const {
   // Write the Action type
   pos = isabBoxNavMessageUtil::incWriteShort( 
      buf,pos, nav_meta_point_time_dist_left_points );
   pos = isabBoxNavMessageUtil::incWriteShort( buf,pos, 0 ); // Reserved
   pos = isabBoxNavMessageUtil::incWriteLong( buf, pos, m_timeLeft );
   pos = isabBoxNavMessageUtil::incWriteLong( buf, pos, m_distLeft );
   
   return pos;      
}


int
MetaPTElement::write( uint8* buf, int pos ) const {
   // Write the Action type
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, 
                                               nav_meta_point_meta );
   // type
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, m_type );
   // a
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, m_dataa );
   // b
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, m_datab );
   // c
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, m_datac );
   // d
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, m_datad );
   
   return pos;      
}

int
LandmarkPTElement::write( uint8* buf, int pos ) const {
   // Write the Action type
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, 
                                               nav_meta_point_landmark );
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, m_flags);
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, m_strNameIdx);
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, m_idStartStop);
   pos = isabBoxNavMessageUtil::incWriteLong( buf, pos, m_distance);
   mc2dbg8 << "isabRouteLandmarkElement:<" << hex
        << (int)nav_meta_point_landmark << dec << ">, flags<"<< (int)m_flags
        << ">, strNameIndex<" << m_strNameIdx << ">, idStartStop<"
        << m_idStartStop << ">, distance<" << m_distance << ">" << endl;
   return pos;      
}

int
LaneInfoPTElement::write( uint8* buf, int pos ) const {
   // Write the Action type
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, 
                                               nav_meta_point_lane_info );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_flags );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_nbrLanes );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane0 );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane1 );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane2 );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane3 );
   pos = isabBoxNavMessageUtil::incWriteLong( buf, pos, m_distance);
   return pos;      
}

int
LaneDataPTElement::write( uint8* buf, int pos ) const {
   // Write the Action type
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, 
                                               nav_meta_point_lane_data );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane0 );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane1 );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane2 );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane3 );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane4 );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane5 );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane6 );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane7 );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane8 );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_lane9 );
   return pos;      
}

int
SignPostPTElement::write( uint8* buf, int pos ) const {
   // Write the Action type
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, 
                                               nav_meta_point_signpost );
   pos = isabBoxNavMessageUtil::incWriteShort( buf, pos, m_signPostTextIndex );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_textColor );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_backgroundColor );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_frontColor );
   pos = isabBoxNavMessageUtil::incWriteByte( buf, pos, m_reserved );
   pos = isabBoxNavMessageUtil::incWriteLong( buf, pos, m_distance);
   return pos;      
}
