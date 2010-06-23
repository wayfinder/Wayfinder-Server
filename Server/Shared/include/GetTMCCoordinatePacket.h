/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GETTMCCOORDINATEPACKET_H
#define GETTMCCOORDINATEPACKET_H

#include "Types.h"
#include "Packet.h"
#include "TrafficDataTypes.h"
#include "StringUtility.h"
#include "MC2String.h"

#include <vector>

/**
 * GetTMCCoordinateRequestPacket
 */
class GetTMCCoordinateRequestPacket : public RequestPacket
{
  public:
   /**
    * Constructor..
    */
   GetTMCCoordinateRequestPacket(uint16 packetID,
                                 uint16 reqID);
   
   /**
    * Constructor..
    */
   GetTMCCoordinateRequestPacket();
   
   /**
    * Destructor..
    */
   virtual ~GetTMCCoordinateRequestPacket(){};
   
   /**
    *  Add one or two TMC points to the packet.
    *  @param firstPoint The first point to add.
    *  @param secondPoint The second point to add.
    *  @param extent The extent of the disturbance.
    *  @direction direction The direction of the disturbance.
    *  @return True if the point was added successfully.
    */
   bool addPoints(MC2String firstLocation,
                  MC2String secondLocation,
                  int32 extent,
                  TrafficDataTypes::direction direction);
   
   /**
    *  Extract one or TMC points from the packet.
    *  @param firstPoint The first extracted point.
    *  @param secondPoint The second extracted point.
    *  @param extent The extent of the disturbance.
    *  @param direction The direction of the disturbance.
    *  @return True if a point was found.
    */
   bool getPoints(MC2String &firstPoint,
                  MC2String &secondPoint,
                  int32 &extent,
                  TrafficDataTypes::direction &direction) const;
};


/// Reply packet
class GetTMCCoordinateReplyPacket : public ReplyPacket
{
  public:
   /**
    * Constructor.
    * @param p The request packet.
    * @param status The result of the addition.
    * @param size The total size of the streetNames.
    */
   GetTMCCoordinateReplyPacket(const RequestPacket* p, uint32 status,
                               uint32 size);

   /**
    *  Destructor, removes memory allocated here.
    */
   virtual ~GetTMCCoordinateReplyPacket(){};

   /**
    *  Adds two coordinates to the packet.
    *  @param x1 The first x-coordinate to add.
    *  @param x2 The second x-coordinate to add.
    *  @param y1 The first y-coordinate to add.
    *  @param y2 The second y-coordinate to add.
    *  @firstName1 The first name from the database.
    *  @firstName2 The first name from the database.
    *  @secondName1 The second name from the database.
    *  @secondName2 The second name from the database.
    *  @return True if the coordinate fitted into the packet. (The packet
    *               one or none coordinates before.)
    */
   void addCoordinatesToPacket(vector< pair<int32, int32> > firstCoords,
                               vector< pair<int32, int32> > secondCoords);
   
   /**
    *  Gets the coordinates and the names from the packet.
    *  @param x1 The first x-coordinate.
    *  @param x2 The second x-coordinate.
    *  @param y1 The first y-coordinate.
    *  @param y2 The second y-coordinate.
    *  @firstName1 The first name from the database.
    *  @firstName2 The second name from the database.
    *  @secondName1 The first name from the database.
    *  @secondName2 The second name from the database.
    *  @return True if the coordinates were read, otherwise false.
    */
   void getCoordinates(vector< pair<int32, int32> > &firstCoords,
                       vector< pair<int32, int32> > &secondCoords) const;

   /**
    *  Gets the coordinates and the names from the packet.
    *  @param firstCoords The coordinates for the first TMC location.
    *  @param firstCoords The coordinates for the second TMC location.
    */
   void getMC2Coordinates( std::vector< MC2Coordinate >& firstCoords,
                           std::vector< MC2Coordinate >& secondCoords ) const;
   
};

#endif //GETTMCCOORDINATEPACKET_H



















