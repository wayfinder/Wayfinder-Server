/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TRAFFICELEMENT_H
#define TRAFFICELEMENT_H

#include "config.h"
#include "StreetSegmentItem.h"
#include "MC2Coordinate.h"
#include <vector>
#include <map>

/**
 * A class holding information about a traffic situation.
 */
class TrafficElement {
public:
   /// Constructor
   TrafficElement();

   /// Time def.
   typedef time_t Time;
   typedef MC2String SituationRef;
   typedef MC2String SituationType;
   typedef uint32 Cost;
   typedef std::vector< MC2Coordinate > Coordinates;

   struct SSIID {
      uint32 m_id;
      Cost m_cost;
      Coordinates m_coords;
   };

   typedef std::vector< SSIID > SSICont;
   
   /**
    * Constructor.
    * @param sitRef The situation reference.
    * @param type The disturbance type.
    * @param startTime The start time.
    * @param endTime The end time.
    * @param text The description of the situation.
    * @param ssiInfo The ssi information for the element.
    */
   TrafficElement( const SituationRef& sitRef,
                   const SituationType& type, 
                   Time startTime, 
                   Time endTime,
                   const MC2String& description, 
                   SSICont& ssiInfo );

   /// Destructor
   ~TrafficElement();

   /**
    * Returns the situation id of the element.
    * @return The id.
    */
   inline const SituationRef& getSitRef() const;

   /**
    * Returns the start time for the situation.
    * @return The start time.
    */
   inline const Time getStartTime() const;

   /**
    * Returns the end time for the situation.
    * @return The end time.
    */
   inline const Time getEndTime() const;

   /**
    * Returns the description text for the traffic element.
    * @return The description.
    */
   inline const MC2String& getDescription() const;


   /// The container with SSI information.
   inline const SSICont& getSSIInformation() const;

   /**
    * Sets the situation id of the element.
    */
   inline void setSitRef( const MC2String& sitRef );

   /**
    * Sets the start time for the situation.
    */
   inline void setStartTime( const Time& starTime );

   /**
    * Sets the end time for the situation.
    */
   inline void setEndTime( const Time& endTime );

   /**
    * Sets the description text for the traffic element.
    */
   inline void setDescription( const MC2String& text );

   /// @return situation type
   inline const SituationType& getType() const;

   /// The container with SSI information.
   inline void setSSIInformation( SSICont& ssiInfo );

   /// Compare with other elements.
   inline bool operator == ( const TrafficElement& other ) const;
   inline bool operator != ( const TrafficElement& other ) const {
      return ! ( *this == other );
   }
private:
   /// The situation id.
   SituationRef m_sitRef;

   /// The type of traffic situation
   SituationType m_type;

   /// Start time for the element
   Time m_startTime; 

   /// End time for the element
   Time m_endTime;

   /// Text describing the traffic element.
   MC2String m_description;
   
   /// Information about the SSIs for this element.
   SSICont m_ssiInfo;



};

// INLINE STARTS --------------------------------------------------------------

/// Compare with other elements.
inline bool TrafficElement::operator == ( const TrafficElement& other ) const {
   return
      m_sitRef == other.m_sitRef &&
      m_type == other.m_type &&
      m_startTime == other.m_startTime &&
      m_endTime == other.m_endTime &&
      m_description == other.m_description;
   // TODO: compare m_ssiInfo
}

inline const TrafficElement::SituationRef&
TrafficElement::getSitRef() const {
   return m_sitRef;
}

inline const TrafficElement::Time 
TrafficElement::getStartTime() const {
   return m_startTime;
}

inline const TrafficElement::Time 
TrafficElement::getEndTime() const {
   return m_endTime;
}

inline const MC2String&
TrafficElement::getDescription() const {
   return m_description;
}

inline const TrafficElement::SSICont&
TrafficElement::getSSIInformation() const {
   return m_ssiInfo;
}

inline void 
TrafficElement::setSitRef( const MC2String& sitRef ) {
   m_sitRef = sitRef;
}

inline void 
TrafficElement::setStartTime( const Time& starTime ) {
   m_startTime = starTime;
}

inline void 
TrafficElement::setEndTime( const Time& endTime ) {
   m_endTime = endTime;
}

inline void 
TrafficElement::setDescription( const MC2String& description ) {
   m_description = description;
}

inline void 
TrafficElement::setSSIInformation( SSICont& ssiInfo ) {
   m_ssiInfo.swap( ssiInfo );
}

inline const TrafficElement::SituationType&
TrafficElement::getType() const {
   return m_type;
}

#endif // TRAFFICELEMENT_H
