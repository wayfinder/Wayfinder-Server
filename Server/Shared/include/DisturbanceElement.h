/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DISTURBANCEELEMENT_H
#define DISTURBANCEELEMENT_H

#include "MC2String.h"
#include "TrafficDataTypes.h"
#include "MapRights.h"
#include "MC2Coordinate.h"

#include <map>
#include <vector>
#include <set>


/**
 *
 */
class DisturbanceElement {
public:
   /// Typedef for DisturbanceID
   typedef uint32 DisturbanceID;
   typedef MC2String SituationReference;

   /// Invalid disturbance id
   static const uint32 INVALID_DISTURBANCE_ID = MAX_UINT32;

   DisturbanceElement();

   DisturbanceElement(DisturbanceID disturbanceID,
                      const SituationReference& situationReference,
                      TrafficDataTypes::disturbanceType type,
                      TrafficDataTypes::phrase phrase,
                      uint32 eventCode,
                      uint32 startTime,
                      uint32 endTime,
                      uint32 creationTime,
                      TrafficDataTypes::severity severity,
                      TrafficDataTypes::direction direction,
                      const MC2String& firstLocation,
                      const MC2String& secondLocation,
                      uint8 extent,
                      uint32 costFactor,
                      const MC2String& text,
                      uint32 queueLength);

   virtual ~DisturbanceElement();

   /// Adds a coordinate
   void addCoordinate(uint32 nodeID,
                      int32 latitude,
                      int32 longitude,
                      uint32 angle,
                      uint32 routeIndex);

   /// Adds a coordinate, with no nodeID
   void addCoordinate(int32 latitude,
                      int32 longitude,
                      uint32 angle,
                      uint32 routeIndex);

   /// Adds a nodeID
   void addNodeID(uint32 nodeID,
                  uint32 routeIndex,
                  uint32 offset = MAX_UINT32);

   /// Clears data that is not needed when sent as and object that is supposed
   /// to be removed from the info module
   void stripData();

   /// Removes an index
   void removeIndex(uint32 routeIndex);

   /// Returns the disturbanceID
   inline DisturbanceID getDisturbanceID() const;

   /// Sets the disturbanceID
   void setDisturbanceID(uint32 disturbanceID);

   /// sets the situation reference.
   inline void setSituationReference( const MC2String& sitRef );

   /// Returns the messageID
   inline const SituationReference& getSituationReference() const;

   /// Returns the type if available
   inline TrafficDataTypes::disturbanceType getType() const;

   /// Returns the phrase if available
   inline TrafficDataTypes::phrase getPhrase() const;

   /// Returns the event code (if available)
   inline uint32 getEventCode() const;

   /// Returns the start time
   inline uint32 getStartTime() const;

   /// Returns the end time
   inline uint32 getEndTime() const;

   /// Returns the creationTime
   inline uint32 getCreationTime() const;

   /// Returns the severity
   inline TrafficDataTypes::severity getSeverity() const;

   /// Returns the direction
   inline TrafficDataTypes::direction getDirection() const;

   typedef vector<MC2Coordinate> Coordinates;

   /// @param coordinates Will be filled with all the coordinates for this
   ///                    disturbance.
   void getCoordinates( Coordinates& coords ) const;

   typedef map<uint32, int32> RouteIndex2CoordMap;
   /// Returns the lat
   inline const RouteIndex2CoordMap& getLatMap() const;

   /// Returns the lon
   inline const RouteIndex2CoordMap& getLonMap() const;

   /// Returns the mapID
   inline uint32 getMapID() const;

   /// Sets the mapID
   void setMapID(uint32 mapID);

   /// Returns the nodeID
   inline const map<uint32, uint32>& getNodeID() const;

   /// Returns the nodeID set
   inline const set<uint32>& getNodeIDSet() const;
   typedef map<uint32, uint32> RouteIndex2AngleMap;
   /// Returns the angle
   inline const RouteIndex2AngleMap& getAngle() const;

   typedef vector<uint32> RouteIndex;
   /// Returns the route index
   inline const RouteIndex& getRouteIndex() const;

   /// Returns the first tmc point.
   inline const MC2String& getFirstLocation() const;

   /// Returns the second tmc point.
   inline const MC2String& getSecondLocation() const;

   /// Returns the extent
   inline uint8 getExtent() const;

   /// Returns the extra cost factor (* 1000)
   inline uint32 getCostFactor() const;

   /// Returns the text
   inline const MC2String& getText() const;

   /// Returns the queue length
   inline uint32 getQueueLength() const;

   /// Gets the deleted parameter
   inline bool getDeleted() const;

   /// Gets the activity bool
   inline bool isActive() const;

   /// Sets deleted
   void setDeleted(bool deleted);

   /// Sets isActive
   void setIsActive(bool isActive);

   /// Returns the nbr of coordinates.
   inline uint32 getNbrCoordinates() const;

   inline const MapRights& getNeededRights() const;

   bool operator == ( const DisturbanceElement& other ) const;

private:
   /// The disturbanceID
   DisturbanceID m_disturbanceID;

   /// The situation reference
   SituationReference m_situationReference;

   /// The disturbance type
   const TrafficDataTypes::disturbanceType m_type;

   /// The phrase
   const TrafficDataTypes::phrase m_phrase;

   /// The eventcode
   const uint32 m_eventCode;

   /// The start time
   const uint32 m_startTime;

   /// The end time
   const uint32 m_endTime;

   /// The creation time
   const uint32 m_creationTime;

   /// The severity
   const TrafficDataTypes::severity m_severity;

   /// The direction
   const TrafficDataTypes::direction m_direction;

   /// The latitudes
   map<uint32, int32> m_latMap;

   /// The longitudes
   map<uint32, int32> m_lonMap;

   /// The mapID
   uint32 m_mapID;

   /// The nodeIDs together with the routeIndex
   map<uint32, uint32> m_nodeID;

   /// The nodeIDs in a set, easy to find
   set<uint32> m_nodeIDSet;

   /// The angles together with the routeIndex
   map<uint32, uint32> m_angle;

   /// The route indexes
   vector<uint32> m_routeIndex;

   /// The first tmc point
   const MC2String m_firstLocation;

   /// The second tmc point
   const MC2String m_secondLocation;

   /// The extent
   const uint8 m_extent;

   /// The extra cost
   const uint32 m_costFactor;

   /// The text
   MC2String m_text;

   /// The queue length
   const uint32 m_queueLength;

   /// If the disturbance has been deleted
   bool m_deleted;

   /// The nbr of coordinates
   uint32 m_nbrCoordinates;

   /// True if the disturbance is active
   bool m_isActive;

   MapRights m_neededRights;

   uint32 m_startOffset;

};

typedef map<DisturbanceElement::DisturbanceID, const DisturbanceElement*> IDToDisturbanceMap;

inline DisturbanceElement::DisturbanceID
DisturbanceElement::getDisturbanceID() const
{
   return m_disturbanceID;
}
inline void
DisturbanceElement::setSituationReference( const SituationReference& sitRef )
{
   m_situationReference = sitRef;
}

inline const DisturbanceElement::SituationReference&
DisturbanceElement::getSituationReference() const
{
   return m_situationReference;
}

inline TrafficDataTypes::disturbanceType
DisturbanceElement::getType() const
{
   return m_type;
}

inline TrafficDataTypes::phrase
DisturbanceElement::getPhrase() const
{
   return m_phrase;
}

inline uint32
DisturbanceElement::getEventCode() const
{
   return m_eventCode;
}

inline uint32
DisturbanceElement::getStartTime() const
{
   return m_startTime;
}

inline uint32
DisturbanceElement::getEndTime() const
{
   return m_endTime;
}

inline uint32
DisturbanceElement::getCreationTime() const
{
   return m_creationTime;
}

inline TrafficDataTypes::severity
DisturbanceElement::getSeverity() const
{
   return m_severity;
}

inline TrafficDataTypes::direction
DisturbanceElement::getDirection() const
{
   return m_direction;
}

inline const map<uint32, int32>&
DisturbanceElement::getLatMap() const
{
   return m_latMap;
}

inline const map<uint32, int32>&
DisturbanceElement::getLonMap() const
{
   return m_lonMap;
}

inline uint32
DisturbanceElement::getMapID() const
{
   return m_mapID;
}

inline const map<uint32, uint32>&
DisturbanceElement::getNodeID() const
{
   return m_nodeID;
}

inline const set<uint32>&
DisturbanceElement::getNodeIDSet() const
{
   return m_nodeIDSet;
}

inline const map<uint32, uint32>&
DisturbanceElement::getAngle() const
{
   return m_angle;
}

inline const vector<uint32>&
DisturbanceElement::getRouteIndex() const
{
   return m_routeIndex;
}

inline const MC2String&
DisturbanceElement::getFirstLocation() const
{
   return m_firstLocation;
}

inline const MC2String&
DisturbanceElement::getSecondLocation() const
{
   return m_secondLocation;
}

inline uint8
DisturbanceElement::getExtent() const
{
   return m_extent;
}

inline uint32
DisturbanceElement::getCostFactor() const
{
   return m_costFactor;
}

inline const MC2String&
DisturbanceElement::getText() const
{
   return m_text;
}

inline uint32
DisturbanceElement::getQueueLength() const
{
   return m_queueLength;
}

inline bool
DisturbanceElement::getDeleted() const
{
   return m_deleted;
}

inline uint32
DisturbanceElement::getNbrCoordinates() const
{
   return m_nbrCoordinates;
}

inline bool
DisturbanceElement::isActive() const
{
   return m_isActive;
}

inline const MapRights&
DisturbanceElement::getNeededRights() const
{
   return m_neededRights;
}

#endif
