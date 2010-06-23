/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ALL_COUNTRYPACKET_H
#define ALL_COUNTRYPACKET_H

#include "config.h"
#include "Packet.h"
#include "Vector.h"

class MC2BoundingBox;


#define ALL_COUNTRY_REPLY_PRIO   DEFAULT_PACKET_PRIO
#define ALL_COUNTRY_REQUEST_PRIO DEFAULT_PACKET_PRIO

#define ALL_COUNTRY_REPLY_HEADER_SIZE (REPLY_HEADER_SIZE+4)
/**
 *   Packet that is used for asking the MapModule for all countries.
 *   Cointains nothing but the header.
 *
 */
class AllCountryRequestPacket : public RequestPacket {
   public:
      /**
       *   Create a new CountryRequestPacket.
       *   @param reqID unique request ID
       *   @param packetID unique packet ID
       */
      AllCountryRequestPacket( uint16 reqID, uint16 packetID );
};


/**
 *   Packet that is send as reply to the AllCountryRequest-packet.
 *
 *   After the normal header this packet contains
 *   @packetdesc
 *      @row REPLY_HEADER_SIZE @sep 4  bytes @sep Number of countries @endrow
 *      @row For each country  @sep 4  bytes @sep mapID @endrow
 *      @row                   @sep 4  bytes @sep map version @endrow
 *      @row                   @sep 16 bytes @sep boundingBox @endrow
 *      @row                   @sep 4  bytes @sep Number of strings @endrow
 *      @row For each string   @sep 4  bytes @sep String index @endrow
 *   @endpacketdesc
 *
 */
class AllCountryReplyPacket : public ReplyPacket {

public:
   
      /**         
       *   Default constructor.
       */
      AllCountryReplyPacket();

      /**
       *   Create a CountryReplyPacket to the requestpacket given as 
       *   parameter.
       *
       *   @param packet is the request packet.
       */
      AllCountryReplyPacket( AllCountryRequestPacket* packet );

      /**
       * Returns the number of countries added to this packet.
       * 
       * @return the number of countries.
       */
      uint32 getNbrCountries();

      /**
       * Add a country to this packet.
       *
       * @param mapID is the mapID of the overview map for this country..
       * @param version is the version of the map.
       * @param boundingBox is the boundingbox of the country.
       * @param stringIndex The string indecis for this country.
       */
      void addCountry( uint32 mapID, uint32 version, 
                       const MC2BoundingBox& boundingBox,
                       Vector& stringIndex );

      /**
       * Get data about a country.  
       *
       * @param index the index of the map to get.
       * @param mapID is the mapID of the country.
       * @param version is the verion of the map.
       * @param boundingBox is the boundingbox of the country.
       * @param stringIndex The string indeces for this country.
       *                    Will be reset before adding.
       * @return true if the string is added, false otherwise.
       */
      bool getCountryData( uint32 index, uint32& mapID, 
                           uint32& version, MC2BoundingBox& boundingBox,
                           Vector& stringIndex );

   private:

      /**
       * Saves the number of countries.
       * 
       * @param the number of countries.
       */
      void setNbrCountries( uint32 number );

      /**
       * Returns the start position of a certain country item.
       *
       * @param index is the index of the country.
       * @return the start position.
       */
      uint32 getStartPos( uint32 index );

};


#endif

