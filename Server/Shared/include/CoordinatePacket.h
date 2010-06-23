/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COORDINATEPACKET_H
#define COORDINATEPACKET_H

#include "config.h"
#include "Packet.h"
#include "StringUtility.h"
#include "LangTypes.h"

// The sizes and prios
#define COORDINATE_REQUEST_PRIO        DEFAULT_PACKET_PRIO
#define COORDINATE_REQUEST_MAX_LENGTH  (REQUEST_HEADER_SIZE+64)
#define COORDINATE_REPLY_PRIO          DEFAULT_PACKET_PRIO
#define COORDINATE_MAX_REPLY_LENGTH    (REPLY_HEADER_SIZE+2000)

class CountryCode;

/**
  *   Class describing the UDP's sent to the MapModule to get the
  *   closest item to the given coordinates.
  *
  *   After the general RequestPacket-header the format is (X =
  *   REQUEST_HEADER_SIZE):
  *   @packetdesc
  *      @row X    @sep 4 bytes   
  *                @sep The latitud, in ``our'' coordinate system.@endrow
  *      @row X+4  @sep 4 bytes   
  *                @sep The longitud, in ``our'' coordinate system.@endrow
  *      @row X+8  @sep 4 bytes    
  *                @sep The data that should be included in the reply 
  *                     packet. Choose among the flags defined in the 
  *                     request packet. If zero is specified, all outdata 
  *                     are included.@endrow
  *      @row X+12 @sep 2 byte    @sep The angle of the car.@endrow
  *      @row X+14 @sep 2 byte    @sep RESERV @endrow
  *      @row X+16 @sep 4 byte    @sep The wanted language.@endrow
  *      @row X+20 @sep 1 byte    
  *                @sep The number of allowed item types in the reply. If 
  *                     zero all item types are allowed.@endrow
  *      @row X+21 @sep 1 byte    @sep The first wanted item type.@endrow
  *      @row X+22 @sep 1 byte    @sep The second wanted item type.@endrow
  *      @row -    @sep           @sep @endrow
  *   @endpacketdesc
  *
  */
class CoordinateRequestPacket : public RequestPacket {
public:
   
   static const int LAT_POS = REQUEST_HEADER_SIZE;
   static const int LON_POS = REQUEST_HEADER_SIZE + 4;
   static const int OUTDATA_POS = REQUEST_HEADER_SIZE + 8;
   static const int ANGLE_POS = REQUEST_HEADER_SIZE + 12;
   static const int RESERV_POS = REQUEST_HEADER_SIZE + 14;
   static const int LANGUAGE_POS = REQUEST_HEADER_SIZE + 16; 
   static const int ALLOWED_NBR_POS = REQUEST_HEADER_SIZE + 20; 
   static const int FIRST_WANTED_ITEM_POS = REQUEST_HEADER_SIZE + 21; 
   static const int SECOND_WANTED_ITEM_POS = REQUEST_HEADER_SIZE + 22; 
   
#  define COORDINATE_PACKET_RESULT_ANGLE    0x00000001 
#  define COORDINATE_PACKET_RESULT_STRINGS  0x00000002 
#  define COORDINATE_PACKET_RESULT_IDS      0x00000004 
      
   /** 
    *   Creates one Requestpacket with the given parameters. 
    *   This method is inlined. 
    * 
    *   @param   packetID    The id of the packet. 
    *   @param   requestID   The id of the request. 
    *   @param   lat         The latitude in the coordinate. 
    *   @param   lon         The longitude in the coordinate. 
    *   @param   originIP    The sender's IP (default = 0). 
    *   @param   originPort  The port to reply to (default = 0). 
    *   @param   angle       The angle of the car. 
    *   @param   language    The wanted language. (default = english) 
    */ 
   CoordinateRequestPacket(
                           uint16 packetID,  
                           uint16 requestID,  
                           int32 lat,  
                           int32 lon, 
                           uint32 originIP = 0, 
                           uint16 originPort = 0, 
                           uint16 angle = MAX_UINT16, 
                           LangTypes::language_t language = 
                           LangTypes::english );  //default english  
         
   /** 
    *   Get the latitude part of the coordinate. 
    *   @return  The latitude for where to search for the item. 
    */ 
   int32 getLatitude() const { 
      return (readLong(LAT_POS)); 
   }; 
   
   /** 
    *   Get the longitude part of the coordinate. 
    *   @return  The longitude for where to search for the item. 
    */ 
   int32 getLongitude() const { 
      return (readLong(LON_POS)); 
   }; 
   
   /** 
    * 
    * 
    */ 
   void addOutDataType(uint32 t) { 
      uint32 curOutData = readLong(OUTDATA_POS); 
      curOutData |= t; 
      writeLong(OUTDATA_POS, curOutData); 
   } 
   
   uint32 getOutDataType() const { 
      return (readLong(OUTDATA_POS)); 
   } 
   
   uint16 getAngle() const { 
      return (readShort(ANGLE_POS)); 
   } 
   
   uint32 getLanguage() const { 
      return (readLong(LANGUAGE_POS)); 
   } 
   
   /** 
    *   Get the number of item types in this packet. 
    *   @return  The number of specified item types. If zero then 
    *            all item types are allowed in the answer. 
    */ 
   byte getNbrItemTypes() const { 
      return (readByte(ALLOWED_NBR_POS)); 
   }; 
   
   /** 
    *   Get a specified itemtype. 
    *   @param   i  The item wanted. Valid values are  
    *               [0, getNbrItemTypes()]. 
    *   @return  Itemtype number i, MAX_UINT8 if i i s too big.
    */
   byte getItemType(byte i) const {
      if ( i < getNbrItemTypes() ) 
         return (readByte(FIRST_WANTED_ITEM_POS+i));
      else
         return MAX_UINT8; 
   };
   
   /**
    *   Adds one allowed item type to the packet. If no itemtypes
    *   are added then all item types are allowed in the answer.
    *   @param type The item type to add to this packet.
    */
   void addItemType(byte type) {
      int position = getLength();
      incWriteByte(position, type);
      setLength(position);
      writeByte(ALLOWED_NBR_POS, getNbrItemTypes()+1);
   };
   
};

// Maybe these should be member constants.
#define COORD_REPLY_MAP_ID_POS         (REPLY_HEADER_SIZE)
#define COORD_REPLY_ITEM_ID_POS        (REPLY_HEADER_SIZE+4)
#define COORD_REPLY_DIST_POS           (REPLY_HEADER_SIZE+8)
#define COORD_REPLY_OFFSET_POS         (REPLY_HEADER_SIZE+12)
#define COORD_REPLY_POS_ANGLE_POS      (REPLY_HEADER_SIZE+14)
#define COORD_REPLY_NEG_ANGLE_POS      (REPLY_HEADER_SIZE+16)
// PADDING
#define COORD_REPLY_MUNICIPAL_POS      (REPLY_HEADER_SIZE+20)
#define COORD_REPLY_CITY_PART_POS      (REPLY_HEADER_SIZE+24)
#define COORD_REPLY_BUILT_AREA_POS     (REPLY_HEADER_SIZE+28)
#define COORD_REPLY_COUNTRY_ID_POS     (REPLY_HEADER_SIZE+32)
#define COORD_REPLY_ORIGINAL_LAT_POS   (REPLY_HEADER_SIZE+36)
#define COORD_REPLY_ORIGINAL_LON_POS   (REPLY_HEADER_SIZE+40)
// Map-IDs that are used if the coordinate is located in more than one map
#define COORD_REPLY_FIRST_MAPID        (REPLY_HEADER_SIZE+44)
#define COORD_REPLY_STRING_START       (REPLY_HEADER_SIZE+108)

#define MAX_NBR_MAP_IDS                (16)


/**
  *   Class descripbing the UDP's sent from the MapModule containing
  *   the answer to the CoordinatRequestPacket.
  *
  *   After the general ReplyPacket-header the format is (X = 
  *   REPLY_HEADER_SIZE):
  *   @packetdesc
  *      X        @sep  4 bytes  
  *               @sep The ID of the map where item closest to the 
  *                    coordinate in the RequestPacket is located.@endrow
  *      X+4      @sep  4 bytes  @sep The ID of the closest item.@endrow
  *      X+8      @sep  4 bytes  
  *               @sep The distance from the coordinate in the RequestPacket 
  *                    to the item in this packet.@endrow
  *      X+12     @sep  2 bytes  
  *               @sep The offset from node 0 that is closest to the 
  *                    coordinate in the RequestPacket.@endrow
  *      X+14     @sep  2 bytes  
  *               sep The angle, if heading towards node 1 (positive 
  *                   direction).@endrow
  *      X+16     @sep  2 bytes  
  *               @sep The angle, if heading towards node 0 (negative 
  *                   direction).@endrow
  *      X+18     @sep  2 bytes  
  *               @sep Padding (to be compatible with the old one).@endrow
  *      X+20     @sep  4 bytes  
  *               @sep The municipal ID where the item is. @endrow
  *      X+24     @sep  4 bytes  
  *               @sep The citypart ID where the item is. @endrow
  *      X+28     @sep  4 bytes  
  *               @sep The built up area where the item is located.@endrow
  *      X+32     @sep  4 bytes  
  *               @sep The country where the item is located.@endrow
  *      X+36     @sep  4 bytes  
  *               @sep The latitude part of the coordinate in the 
  *                    RequestPacket.@endrow
  *      X+40     @sep  4 bytes  
  *               @sep The longitude part of the coordinate in the 
  *                    RequestPacket.@endrow
  *      X+44--X+104 @sep 4 bytes each
  *               @sep ID of the maps where the coordinate is located (if 
  *                    coordinate in more than one map).@endrow
  *      X+108    @sep strings  
  *               @sep The municipal, city, district and name as 
  *                    strings.@endrow
  *   @endpacketdesc
  *
  */
class CoordinateReplyPacket : public ReplyPacket {
public:
   /**
    *    Create a new CoordinateReplyPacket as a reply to a 
    *    CoordinateRequestPacket.
    *    @param p       The CoordinateRequestPacket that this is a
    *                   reply to.
    *    @param status  The status of this reply.
    *    @param mapID   The ID  of the map where the closest item is 
    *                   located.
    *    @param itemID  The ID of the item that is closest to the 
    *                   given coordinates (in the request packet).
    *    @param offset  The offset at item that is closest to the given
    *                   coordinates
    *    @param dist    The distance from the coordinates to the clostest
    *                   point on the item (from (lat,lon) to 
    *                   mapID.itemID.offset).
    */
   CoordinateReplyPacket(  const CoordinateRequestPacket* p,
                           uint32 status,
                           uint32 mapID,
                           uint32 itemID,
                           uint16 offset,
                           uint32 dist,
                           uint16 posAngle = MAX_UINT16, 
                           uint16 negAngle = MAX_UINT16,
                           uint32 countryID = MAX_UINT32,
                           uint32 municipalID = MAX_UINT32,
                           uint32 builtUpAreaID = MAX_UINT32,
                           uint32 cityPartID = MAX_UINT32 
                           )
      
         : ReplyPacket( COORDINATE_MAX_REPLY_LENGTH,
                        Packet::PACKETTYPE_COORDINATEREPLY,
                        p, status) {
      
      writeLong(COORD_REPLY_MAP_ID_POS, mapID);
      mc2dbg4 << "mapID written, " << mapID << endl;
      writeLong(COORD_REPLY_ITEM_ID_POS, itemID);
      mc2dbg4 << "itemID written, " << itemID << endl;
      writeLong(COORD_REPLY_DIST_POS, dist);
      mc2dbg4 << "dist written, " << dist << endl;
      writeShort(COORD_REPLY_OFFSET_POS, offset);
      mc2dbg4 << "offset written, " << offset << endl;
      writeShort(COORD_REPLY_POS_ANGLE_POS, posAngle);
      mc2dbg4 << "posAngle written, " << posAngle << endl;
      writeShort(COORD_REPLY_NEG_ANGLE_POS, negAngle);
      mc2dbg4 << "negAngle written, " << negAngle << endl;
      writeLong(COORD_REPLY_MUNICIPAL_POS, municipalID);
      mc2dbg4 << "municipalID written, " << municipalID << endl;
      writeLong(COORD_REPLY_CITY_PART_POS, cityPartID);
      mc2dbg4 << "cityPartID writted, " << cityPartID << endl;
      writeLong(COORD_REPLY_BUILT_AREA_POS, builtUpAreaID);
      mc2dbg4 << "BuiltUpAreaID written, " << builtUpAreaID << endl;
      writeLong(COORD_REPLY_COUNTRY_ID_POS, countryID);
      mc2dbg4 << "Country code written, " << countryID << endl;
      
      // Set the original cooridnate
      setOriginalCoordinate(p->getLatitude(), p->getLongitude());
      
      // Reset all possible map IDs
      for (int i=0; i<MAX_NBR_MAP_IDS; i++) {
         writeLong(COORD_REPLY_FIRST_MAPID + i*4, MAX_UINT32);
      }
      
      // Write some empty strings (will also set the length of 
      // the packet)
      writeStrings(NULL, NULL, NULL, NULL, NULL);
   };
   
   /**
    *    Get the ID of the map where the closest item is located.
    *    @return  The ID of the map where the closest item i located.
    */
   inline uint32 getMapID() const;
   
   /**
    *    Get the ID of the closest item.
    *    @return  The ID of the closest item.
    */
   inline uint32 getItemID() const;
   
   /**
    *    Get the offset on item that is closest to the given coordinates.
    *    @return  The offset on item that is closest to the given 
    *             coordinates.
    */
   inline uint16 getOffset() const;
   
   /**
    *    Get the distance from the given coordinates to the closest point
    *    on the item (in meters).
    *    @return The closest distance in meters between the given 
    *            coordinate and the item.
    */
   inline uint32 getDistance() const;
   
   /**
    *    Get the angle at offset at the item with ID = itemID if
    *    traveling in positive direction (towards node 1, that is
    *    the same as following the coordinate order). The angle is
    *    calculated clockwise from the north direction in degrees.
    *    @return The angle from north at given offset an heading in
    *            positive direction. A value greater than 360 
    *            indicate an error!
    */
   inline uint16 getPositiveAngle() const;
   
   /**
    *    Get the angle at offset at the item with ID = itemID if
    *    traveling in negative direction (towards node 0, that is
    *    the same as traversing the item, not following the order
    *    of the coordinates). The angle is calculated clockwise 
    *    from the north direction in degrees.
    *    @return The angle from north at given offset an heading in
    *            positive direction. A value greater than 360 
    *            indicate an error!
    */
   inline uint16 getNegativeAngle() const;
   
   /**
    *    Get the municipal id of the item that we are looking up.
    *    Can be useful if you know the position and you only want to
    *    search the municipal where you are.
    *    @return The municipal id of the item.
    */
   inline uint32 getMunicipalID() const;
   
   /**
    *    Get the cityPartID where the found item is.
    *    @return The cityPartID for the city part where the item is.       
    */
   inline uint32 getCityPartID() const;
   
   /**
    *    Gets the built up area for the item.
    *    @return The built up area for the item.
    */
   inline uint32 getBuiltUpAreaID() const;

   /**
    *    Get the country code.
    *    @return The country code.
    */
   CountryCode getCountryCode() const;
   
   /**
    *
    */
   inline bool setOriginalCoordinate(int32 lat, int32 lon);
   inline bool getOriginalCoordinate(int32& lat, int32& lon) const;
   inline int32 getOriginalLatitude() const;
   inline int32 getOriginalLongitude() const;
   
   
   inline bool addPossibleMap(uint32 mapID);
   inline uint32 getNbrPossibleMaps() const;
   inline uint32 getPossibleMap(byte i) const;
   
   /**
    *    Get the names of the item and the location of that item. 
    *    The strings might be NULL if the information not is available
    *    or not applyable. The names are written in the language 
    *    that is set in the request packet.
    *
    *    @param country    The name of the country where the item 
    *                      is located.
    *    @param municipal  The name of the municipal where the item 
    *                      is located.
    *    @param city       The name of the city where the item is 
    *                      located.
    *    @param district   The name of the district (called "city part"
    *                      in some places in the MC2-system )where the 
    *                      item is located.
    *    @param name       The name of the item.
    */
   inline bool writeStrings(const char* country,
                            const char* municipal,
                            const char* city,
                            const char* district,
                            const char* name);
   
   /**
    *    Get a copy of the strings in this packet. The outparameters
    *    will never be set to NULL, if no string is available the
    *    empty string ("\0") will be returned. The strings that are
    *    uses as outparameters must be allocated by the caller!
    *
    *    @param country    Outparameter that is set to the name of 
    *                      the country where the item is located.
    *    @param municipal  Outparameter that is set to the name of the 
    *                      municipal where the item is located.
    *    @param city       Outparameter that is set to the name of the 
    *                      city where the item is located.
    *    @param district   Outparameter that is set to the name of the 
    *                      district (called "city part" in some places 
    *                      in the MC2-system )where the item is located.
    *    @param name       Outparameter that is set to the name of the 
    *                      item.
    *    @param maxSize    The maximum number of characters that are 
    *                      copied into each outparameter.
    */
   inline bool getStrings(char* country,
                          char* municipal,
                          char* city,
                          char* district,
                          char* name,
                          int maxSize) const;
   
   /**
    *    @param originalAngle The angle 
    *    @return 1 if we are heading towards node 1 and 0
    *    if we are heading towards node 0. -1 if we dont know.
    */
   int getDirFromZero(uint16 originalAngle) const;
private:
};


// =======================================================================
//                                     Implementation of inlined methods =


inline uint32 
CoordinateReplyPacket::getMapID() const
{
   return (readLong(COORD_REPLY_MAP_ID_POS));
}

inline uint32 
CoordinateReplyPacket::getItemID() const
{
   return (readLong(COORD_REPLY_ITEM_ID_POS));
}

inline uint16 
CoordinateReplyPacket::getOffset() const
{
   return (readShort(COORD_REPLY_OFFSET_POS));
}

inline uint32 
CoordinateReplyPacket::getDistance() const
{
   return (readLong(COORD_REPLY_DIST_POS));
}

inline uint16 
CoordinateReplyPacket::getPositiveAngle() const
{
   return (readShort(COORD_REPLY_POS_ANGLE_POS));
}

inline uint16
CoordinateReplyPacket::getNegativeAngle() const
{
   return (readShort(COORD_REPLY_NEG_ANGLE_POS));
}

inline uint32
CoordinateReplyPacket::getMunicipalID() const
{
   return (readLong(COORD_REPLY_MUNICIPAL_POS));
}

inline uint32
CoordinateReplyPacket::getCityPartID() const
{
   return (readLong(COORD_REPLY_CITY_PART_POS));
}

inline uint32
CoordinateReplyPacket::getBuiltUpAreaID() const
{
   return (readLong(COORD_REPLY_BUILT_AREA_POS));
}

inline bool 
CoordinateReplyPacket::setOriginalCoordinate(int32 lat, int32 lon)
{
   writeLong(COORD_REPLY_ORIGINAL_LAT_POS, lat);
   writeLong(COORD_REPLY_ORIGINAL_LON_POS, lon);
   return (true);
}

inline bool 
CoordinateReplyPacket::getOriginalCoordinate(int32& lat, int32& lon) const
{
   lat = readLong(COORD_REPLY_ORIGINAL_LAT_POS);
   lon = readLong(COORD_REPLY_ORIGINAL_LON_POS);
   return (true);
}

inline int32
CoordinateReplyPacket::getOriginalLatitude() const
{
   return (readLong(COORD_REPLY_ORIGINAL_LAT_POS));

}

inline int32
CoordinateReplyPacket::getOriginalLongitude() const
{
   return (readLong(COORD_REPLY_ORIGINAL_LON_POS));
}


inline bool 
CoordinateReplyPacket::addPossibleMap(uint32 mapID)
{
   uint32 n = getNbrPossibleMaps();
   if (n < MAX_NBR_MAP_IDS) {
      writeLong(COORD_REPLY_FIRST_MAPID+n*4, mapID);
      return (true);
   } else {
      return (false);
   }
}

inline uint32 
CoordinateReplyPacket::getNbrPossibleMaps() const
{
   int pos = COORD_REPLY_FIRST_MAPID;
   while ( (((pos-COORD_REPLY_FIRST_MAPID)/4) < MAX_NBR_MAP_IDS) && 
           (readLong(pos) < MAX_UINT32)) {
      pos += 4;
   }

   return ((pos-COORD_REPLY_FIRST_MAPID)/4);
}

inline uint32 
CoordinateReplyPacket::getPossibleMap(byte i) const
{
   return readLong(COORD_REPLY_FIRST_MAPID + 4*i);
}


inline bool 
CoordinateReplyPacket::writeStrings(const char* country,
             const char* municipal,
             const char* city,
             const char* district,
             const char* name)
{
   int position = COORD_REPLY_STRING_START;
   // Write the country
   if (country != NULL) {
      incWriteString(position, country);
   } else {
      incWriteString(position, "");
   }

   // Write the municipal
   if (municipal != NULL) {
      incWriteString(position, municipal);
   } else {
      incWriteString(position, "");
   }

   // Write the city
   if (city != NULL) {
      incWriteString(position, city);
   } else {
      incWriteString(position, "");
   }

   // Write the district
   if (district != NULL) {
      incWriteString(position, district);
   } else {
      incWriteString(position, "");
   }

   // Write the name
   if (name != NULL) {
      incWriteString(position, name);
   } else {
      incWriteString(position, "");
   }

   // Set length and return
   setLength(position);
   return (true);
}

inline bool 
CoordinateReplyPacket::getStrings(char* country,
                                  char* municipal,
                                  char* city,
                                  char* district,
                                  char* name,
                                  int maxSize) const
{
   int position = COORD_REPLY_STRING_START;
   char* tmpStr;

   // Get country
   incReadString(position, tmpStr);
   StringUtility::strlcpy(country, tmpStr, maxSize);

   // Get municipal
   incReadString(position, tmpStr);
   StringUtility::strlcpy(municipal, tmpStr, maxSize);

   // Get city
   incReadString(position, tmpStr);
   StringUtility::strlcpy(city, tmpStr, maxSize);

   // Get district
   incReadString(position, tmpStr);
   StringUtility::strlcpy(district, tmpStr, maxSize);

   // Get name
   incReadString(position, tmpStr);
   StringUtility::strlcpy(name, tmpStr, maxSize);

   // Return
   return (true);
}



#endif

