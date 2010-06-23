/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EVENTFINDER_VENUE_H
#define EVENTFINDER_VENUE_H

#include "MC2Coordinate.h"
#include "MC2String.h"

namespace EventFinder {

/**
 * Describes a venue for an event.
 */
class Venue {
public:
   typedef MC2String IDType;
   inline Venue();
   inline Venue( const IDType& id,
                 const MC2String& name,
                 const MC2String& address,
                 const MC2Coordinate& position );

   /// @return Position.   
   inline const MC2Coordinate& getCoordinate() const;
   /// @return Name.
   inline const MC2String& getName() const;
   /// @return Unique ID.
   inline const IDType& getID() const;
   /// @return Street and city address.
   inline const MC2String& getAddress() const;

   inline void setID( const IDType& id );
   
private:
   MC2Coordinate m_coord; ///< Position.
   MC2String m_name; ///< Location name.
   IDType m_id; ///< Unique ID for this event.
   MC2String m_address; ///< Street + city address.
};

inline bool operator < ( const Venue& lhs, const Venue& rhs ) {
   return lhs.getID() < rhs.getID();
}

struct VenueIDCompare {
   bool operator ()( const Venue* lhs, const Venue* rhs ) const {
      return *lhs < *rhs;
   }

   bool operator () ( const Venue::IDType& id, const Venue* rhs ) const {
      return id < rhs->getID();
   }

   bool operator () ( const Venue* lhs, const Venue::IDType& id ) const {
      return lhs->getID() < id;
   }
};

Venue::Venue( const IDType& id,
              const MC2String& name,
              const MC2String& address,
              const MC2Coordinate& position ):
   m_coord( position ),
   m_name( name ),
   m_id( id ),
   m_address( address )
{
}

inline Venue::Venue():
   m_coord(),
   m_name(),
   m_id( 0 ),
   m_address() {
}

inline const MC2Coordinate& Venue::getCoordinate() const {
   return m_coord;
}

inline const MC2String& Venue::getName() const {
   return m_name;
}

inline const Venue::IDType& Venue::getID() const {
   return m_id;
}

inline const MC2String& Venue::getAddress() const {
   return m_address;
}

inline void Venue::setID( const IDType& id ) {
   m_id = id;
}

}

#endif //  EVENTFINDER_VENUE_H
