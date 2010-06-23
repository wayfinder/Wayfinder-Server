/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CoordinatePacket.h"
#include "StringTable.h"

CoordinateRequestPacket::
CoordinateRequestPacket(
                        uint16 packetID,  
                        uint16 requestID,  
                        int32 lat,  
                        int32 lon, 
                        uint32 originIP, 
                        uint16 originPort, 
                        uint16 angle, 
                        LangTypes::language_t language)  //default english  
      :  RequestPacket( COORDINATE_REQUEST_MAX_LENGTH, 
                        COORDINATE_REQUEST_PRIO, 
                        Packet::PACKETTYPE_COORDINATEREQUEST, 
                        packetID, 
                        requestID, 
                        MAX_UINT32 ){ 
   
   //int position = REQUEST_HEADER_SIZE; 
   setOriginIP(originIP); 
   setOriginPort(originPort); 
   writeLong(LAT_POS, lat); 
   writeLong(LON_POS, lon); 
   writeLong(OUTDATA_POS, 0);       // Outdata 
   writeShort(ANGLE_POS, angle);  // Angle 
   writeShort(RESERV_POS, 0);      // Reserve 
   writeByte(ALLOWED_NBR_POS, 0); // Number of itemtypes 
   writeLong(LANGUAGE_POS, language); 
   setLength(ALLOWED_NBR_POS + 1); 
}

int
CoordinateReplyPacket::getDirFromZero(uint16 originalAngle) const
{
   if ( originalAngle >= 360 )
      return -1;
   int32 posAngle = getPositiveAngle();
   int32 negAngle = getNegativeAngle();
   
   // Values greater than 360 indicates errors
   if ( posAngle < 361 && negAngle < 361 ) {      
      negAngle = negAngle % 360;
      posAngle = posAngle % 360;
      // Find the smallest difference in cw and ccw.
      const int nbrDiff = 4;
      int32 diff[nbrDiff];
      diff[0] = posAngle - originalAngle;
      diff[1] = negAngle - originalAngle;
      diff[2] = originalAngle - posAngle;
      diff[3] = originalAngle - negAngle;
      // Adjust for 360 and find the smallest
      int32 smallest = 30000;
      int smallestPos = 5;
      mc2dbg8 << "Startangle= " << originalAngle << endl
              << "Posangle = " << posAngle << endl
              << "Negangle = " << negAngle << endl;
      for(int i=0; i < nbrDiff; ++i ) {
         diff[i] = diff[i] < 0 ? diff[i] + 360 : diff[i];
         mc2dbg8 << "diff[" << i << "] = " << diff[i] 
                 << endl;
         if ( diff[i] < smallest ) {
            smallestPos = i;
            smallest = diff[i];
         }                  
      }
      // If posangle is the nearest dirFromZero is true.
      return ((smallestPos == 0) || (smallestPos == 2));
      
      
   } else {
      mc2dbg2
             << "CoordinatePacket: posAngle or negAngle more than "
             << "360 deg"
             << endl;
      return -1;
   }
}

CountryCode CoordinateReplyPacket::getCountryCode() const {
   return StringTable::countryCode(readLong(COORD_REPLY_COUNTRY_ID_POS));
}
