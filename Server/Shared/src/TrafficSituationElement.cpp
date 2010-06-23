/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TrafficSituationElement.h"

TrafficSituationElement::TrafficSituationElement(const MC2String& elementReference,
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
                           const MC2String& tmcVersion,
                           TrafficDataTypes::severity_factor severityFactor) :
   m_startTime(startTime),
   m_expiryTime(expiryTime),
   m_creationTime(creationTime),
   m_elementReference(elementReference),
   m_type(type),
   m_phrase(phrase),
   m_severity(severity),
   m_text(text),
   m_firstLocationCode(firstLocation),
   m_secondLocationCode(secondLocation),
   m_extent(extent),
   m_eventCode(eventCode),
   m_direction(direction),
   m_queueLength(queueLength),
   m_coordinates(coordinates),
   m_tmcVersion(tmcVersion),
   m_severityFactor(severityFactor)
{
}

TrafficSituationElement::~TrafficSituationElement()
{
        
}

const MC2String&
TrafficSituationElement::getElementReference() const
{
   return m_elementReference;
}

uint32
TrafficSituationElement::getStartTime() const
{
   return m_startTime;
}

uint32
TrafficSituationElement::getExpiryTime() const
{
   return m_expiryTime;
}

uint32
TrafficSituationElement::getCreationTime() const
{
   return m_creationTime;
}

TrafficDataTypes::disturbanceType
TrafficSituationElement::getType() const
{
   return m_type;
}

TrafficDataTypes::phrase
TrafficSituationElement::getPhrase() const
{
   return m_phrase;
}

TrafficDataTypes::severity
TrafficSituationElement::getSeverity() const
{
   return m_severity;
}

const MC2String&
TrafficSituationElement::getText() const
{
   return m_text;
}

const MC2String&
TrafficSituationElement::getFirstLocationCode() const
{
   return m_firstLocationCode;
}

const MC2String&
TrafficSituationElement::getSecondLocationCode() const
{
   return m_secondLocationCode;
}

uint16
TrafficSituationElement::getEventCode() const
{
   return m_eventCode;
}

int16
TrafficSituationElement::getExtent() const
{
   return m_extent;
}

TrafficDataTypes::direction
TrafficSituationElement::getDirection() const
{
   return m_direction;
}

uint32
TrafficSituationElement::getQueueLength() const
{
   return m_queueLength;
}

const TrafficSituationElement::CoordCont&
TrafficSituationElement::getCoordinates() const
{
   return m_coordinates;
}

const TrafficSituationElement::MC2Coordinates
TrafficSituationElement::getMC2Coordinates() const {
   MC2Coordinates coordinates;
   for ( CoordCont::const_iterator it = m_coordinates.begin();
         it != m_coordinates.end(); ++it ) {
      coordinates.push_back( MC2Coordinate( it->first, it->second ) );
   }
   return coordinates;
}

const MC2String&
TrafficSituationElement::getTmcVersion() const
{
   return m_tmcVersion;
}

TrafficDataTypes::severity_factor 
TrafficSituationElement::getSeverityFactor() const
{
   return m_severityFactor;
}

