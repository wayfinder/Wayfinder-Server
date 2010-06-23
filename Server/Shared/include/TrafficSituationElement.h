/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TRAFFICSITUATIONELEMENT_H
#define TRAFFICSITUATIONELEMENT_H

#include "config.h"
#include "MC2String.h"
#include "TrafficDataTypes.h"
#include "MC2Coordinate.h"
#include <vector>

class TrafficSituationElement {

public:
   /// TODO: change to MC2Coordinate!!
   typedef vector< pair<int32, int32> > CoordCont;

   /// Vector of MC2Coordinate
   typedef std::vector< MC2Coordinate > MC2Coordinates;

   TrafficSituationElement(const MC2String& elementReference,
                           uint32 startTime,
                           uint32 expiryTime,
                           uint32 creationTime,
                           TrafficDataTypes::disturbanceType type,
                           TrafficDataTypes::phrase phrase, 
                           TrafficDataTypes::severity severity, 
                           const MC2String& text,
                           const MC2String& firstLocation,
                           const MC2String& secondLocation,
                           uint16 eventCode,
                           int16 extent,
                           TrafficDataTypes::direction direction,
                           uint32 queueLength,
                           const CoordCont& coordinates,
                           const MC2String& tmcVersion = "",
                           TrafficDataTypes::severity_factor severityFactor = TrafficDataTypes::Unspecified);
   
   /**
    *  Destructor
    */
   virtual ~TrafficSituationElement();

   // Returns the elementID
   const MC2String& getElementReference() const;
   
   // Returns the start time
   uint32 getStartTime() const;

   // Returns the expiry time
   uint32 getExpiryTime() const;

   // Returns the creation time
   uint32 getCreationTime() const;
   
   // Returns the disturbance type
   TrafficDataTypes::disturbanceType getType() const;

   // Returns the phrase
   TrafficDataTypes::phrase getPhrase() const;

   // Returns the severity
   TrafficDataTypes::severity getSeverity() const;

   // Returns the text
   const MC2String& getText() const;

   // Returns the first location code
   const MC2String& getFirstLocationCode() const;

   // Returns the second location code, if present
   const MC2String& getSecondLocationCode() const;

   // Returns the event code
   uint16 getEventCode() const;

   // Returns the extent
   int16 getExtent() const;

   // Returns the direction
   TrafficDataTypes::direction getDirection() const;

   // Returns the queue length
   uint32 getQueueLength() const;

   // Returns the coordinates
   const CoordCont& getCoordinates() const;

   /// Returns the coodirnates in MC2Coordinate form
   const MC2Coordinates getMC2Coordinates() const;

   // Returns the tmc table version
   const MC2String& getTmcVersion() const;

   // Returns the additional severity factor
   TrafficDataTypes::severity_factor getSeverityFactor() const;

  private:

   // Member varibales
   // The start time
   uint32 m_startTime;

   // The expiry time
   uint32 m_expiryTime;

   // The creation time
   uint32 m_creationTime;
   
   // The elementID
   MC2String m_elementReference;

   // The disturbance type
   TrafficDataTypes::disturbanceType m_type;

   // The phrase
   TrafficDataTypes::phrase m_phrase;

   // The severity
   TrafficDataTypes::severity  m_severity;
   
   // The text
   MC2String m_text;

   // The first location code
   MC2String m_firstLocationCode;

    // The second location code, if present
   MC2String m_secondLocationCode;
   
   // The extent
   int16 m_extent;

   // The eventcode
   uint16 m_eventCode;
   
   // The direction
   TrafficDataTypes::direction m_direction;

   // The queue length
   uint32 m_queueLength;

   // A vector with all the coordinates from the traffic data file
   CoordCont m_coordinates;

   // The version of the tmc table
   MC2String m_tmcVersion;

   // The severity factor, increases or decreases the severity
   TrafficDataTypes::severity_factor m_severityFactor;
};

#endif

