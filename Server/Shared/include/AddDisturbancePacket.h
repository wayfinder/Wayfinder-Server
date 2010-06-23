/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ADDDISTURBANCEPACKET_H
#define ADDDISTURBANCEPACKET_H

#include "config.h"
#include "Packet.h"
#include "MC2String.h"
#include <vector>
#include "TrafficDataTypes.h"

class DisturbanceElement;
/**
  *   Packet used for requesting the adding of a traffic cost on a node.
  *
  */
class AddDisturbanceRequestData;

class AddDisturbanceRequestPacket : public RequestPacket
{
  public:
   typedef uint8 ExtentType;
   static const ExtentType MAX_EXTENT;
   /**
    *  Creates a new AddDisturbanceRequestPacket.
    *  @param packetID     The packetID
    *  @param requestID    The requestID
    *  @param size         The size of the data
    *  @param type         The disturbance type
    *  @param phrase       The phrase
    *  @param eventCode
    *  @param startTime    The start time
    *  @param endTime      The end time
    *  @param creationTime
    *  @param severity     The severity
    *  @param direction    The direction
    *  @param extent The extent.
    *  @param costFactor
    *  @param queueLength
    */
   AddDisturbanceRequestPacket(uint16 packetID,
                               uint32 requestID,
                               uint32 size,
                               TrafficDataTypes::disturbanceType type,
                               TrafficDataTypes::phrase phrase,
                               uint32 eventCode,
                               uint32 startTime,
                               uint32 endTime,
                               uint32 creationTime,
                               TrafficDataTypes::severity severity,
                               TrafficDataTypes::direction direction,
                               uint8 extent,
                               uint32 costFactor,
                               uint32 queueLength);

   /**
    *  Creates a new AddDisturbanceRequestPacket.
    *  @param packetID     The packetID
    *  @param requestID    The requestID
    *  @param size         The size of the data
    *  @param params The rest of the construction params as sn object.
    */
   AddDisturbanceRequestPacket( uint16 packetID,
                                uint32 requestID,
                                uint32 size,
                                const AddDisturbanceRequestData& params );

   /**
    *  Creates a new AddDisturbanceRequestPacket. The packet is ready
    *  to send asd long as all params have been added to the params
    *  object.
    *  @param packetID     The packetID
    *  @param requestID    The requestID
    *  @param params The rest of the construction params as sn object.
    */
   AddDisturbanceRequestPacket( uint16 packetID,
                                uint32 requestID,
                                const AddDisturbanceRequestData& params );
private:
   /**
    * Common parts of the constructors are refactored here.
    * @param params The construction parameters.
    */
   void init(const AddDisturbanceRequestData& params);
public:
   /**
    * Destructor.
    */
   virtual ~AddDisturbanceRequestPacket() {};
        
   /**
    *  Adds a disturbance to the packet.
    *  @param nodeID The nodeID where the disturbance is starting.
    *  @param lat The latitude where the disturbance is located.
    *  @param lon The longitude where the disturbance is located.
    *  @param angle The angle, which determins in what direction the
    *               disturbance is valid.
    *  @param extraCost The extra costC factor for the Routemodule.
    *  @param routeIndex The index in the route for the disturbance.
    */
   void addDisturbance(uint32 nodeID,     
                       int32  lat,
                       int32  lon,
                       uint32 angle,
                       uint32 routeIndex);

   /**
    *   Adds the strings.
    *   @param comment The string with the comment, description of the
    *                  disturbance.
    *   @param locText The location text.
    *   @param creator The creator of the disturbance.
    *   @param lastEditUser The user last edited the disturbance the last
    *                       time, initially the creator.
    */
   void addStrings(const MC2String& firstLocation,
                   const MC2String& secondLocation,
                   const MC2String& situationReference,
                   const MC2String& text);
   
   /**
    *  Sets the information text of the disturbance.
    *  @param info The string with the text.
    */
   void setComment(const char* comment);
      
   /**
    *  Returns the DisturbanceElements that the packet is containing.
    *  @param add Set to true if the disturbances is going to be added,
    *             false if they are going to be updated.
    *  @return A vector with the DisturbanceElements.
    */
   DisturbanceElement* getDisturbance() const;

   void setDisturbanceID(uint32 disturbanceID);
  
};


// Reply packet

/**
 *  AddDisturbancePacket.h
 *  Packet used for replying an AddDisturbanceRequestPacket
 *
 *
 */

class AddDisturbanceReplyPacket : public ReplyPacket{
  public:
   /**
    * Constructor.
    * @param p The request packet.
    * @param status The result of the addition.
    */
   AddDisturbanceReplyPacket(const RequestPacket* p, uint32 status,
                             uint32 disturbanceID = MAX_UINT32);
   
   uint32 getDisturbanceID() const;

   /**
    * Destructor.
    */
   virtual ~AddDisturbanceReplyPacket();   
};


    
#endif   // ADDDISTURBANCEPACKET_H
