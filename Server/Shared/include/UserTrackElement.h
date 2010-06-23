/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERTRACKELEMENT_H
#define USERTRACKELEMENT_H

#include "config.h"
#include <list>

class Packet;

/**
 *    Describes one position where the user have been.
 *
 */
class UserTrackElement {
   public:
      /**
       *    Create a new UserTrackElement from a data buffer.
       */
      UserTrackElement( const Packet* p, int& pos );


      /**
       *    Create a new UserTrackElement from data sent as parameters.
       *
       *    @param lat  The latitude-part of the position.
       *    @param lon  The longitude-part of the position.
       *    @param dist The distnace traveled from last track-element.
       *    @param speed The speed at the position in m/s * 32.
       *    @param heading The heading in degrees.
       *    @param time The time (and date) for when the user was at this
       *                location.
       *    @param source The origin of the information, might be an ID of 
       *                a userigator or something else. Must be a short 
       *                string (max 16 bytes).
       */
      UserTrackElement( int32 lat, int32 lon, uint32 dist, uint16 speed,
                        uint16 heading, uint32 time, const char* source );

      /**
       *    Destructor.
       */
      ~UserTrackElement();

      /**
       *    Get the time, in UTC, for when the user was at this position.
       *    @return The time, in UTC.
       */
      uint32 getTime() const;

      /**
       *    Get the latitude.
       *    @return Latitude.
       */
      int32 getLat() const;

      /**
       *    Get the longitude.
       *    @return Longitude.
       */
      int32 getLon() const;


      /**
       * Get the distnace traveled from last track-element.
       */
      uint32 getDist() const;


      /**
       * Get the speed at the position in m/s * 32.
       */
      uint16 getSpeed() const;


      /**
       * Get the heading in degrees.
       */
      uint16 getHeading() const;


      /**
       *    Get a pointer to a string with information about the source.
       *    @return Information about the source of the position.
       */
      inline const char* getSource() const;

      /**
       *    Write this user log element as a byte buffer.
       *    @param p    The packet where the UserTrackElement will be saved.
       *    @param pos  Position in p, will be updated when calling this 
       *                method.
       */
      uint32 save(Packet* p, int& pos) const;

      /**
       *    Get the size of this element when stored in a byte buffer.
       *    @return  The number of bytes this element will use when stored 
       *             in a byte buffer.
       */
      uint32 getSizeAsBytes() const;

      /**
       *    Set all memebers with data from the buffer.
       *    @param p    The packet where the UserTrackElement will be loaded 
       *                from.
       *    @param pos  Position in p, will be updated when calling this 
       *                method.
       */
      void load(const Packet* p, int& pos);

      /**
       *    Dumps content to a given steam.
       * 
       *    @param out The ostream to print to.
       */
      void dump( ostream& out ) const;


   private:
      /**
       *    The time (in UTC, Unix time) when the user was at this position.
       */
      uint32 m_time;

      /**
       *    Latitude part of the coordinate (in mc2-units).
       */
      int32 m_lat;

      /**
       *    Longitude part of the coordinate (in mc2-units).
       */
      int32 m_lon;


      /**
       * The dist.
       */
      uint32 m_dist;


      /**
       * The speed.
       */
      uint16 m_speed;


      /**
       * The heading.
       */
      uint16 m_heading;


      /**
       *    The description of the source of the position.
       */
      char* m_source;
};


/**
 *    Describes a list of UserTrackElements. Can store it self in a packet 
 *    and will delete all elements inserted in the list when deleted.
 *
 */
class UserTrackElementsList : public list<UserTrackElement*> {
   public:
      /**
       *    Default contructor, does nothing.
       */
      UserTrackElementsList();

      /**
       *    Copy contructor. Currently dummy implementation to avoid 
       *    usage!
       *    @param src  The object to initialize the new List from.
       */
      UserTrackElementsList( const UserTrackElementsList& src);
      
      /**
       *    Delete the list and all elements in it (no matter where 
       *    they are allocated).
       */
      ~UserTrackElementsList();

      /**
       *   Store the list in a packet.
       *   @param packet The packet to store this list in
       *   @param pos    Reference the position to use, will point to the
       *                 next pos after the last UserTrackElement when 
       *                 returning.
       */
      void store(Packet* packet, int& pos);

      /**
       *   Restore a list from a packet.
       *   @param packet The packet to restore this list from.
       *   @param pos    Reference the position to use, will point to the
       *                 next pos after the last UserTrackElement when 
       *                 returning.
       */
      void restore(const Packet* packet, int& pos);

      /**
       *    Add an already created UserTrackElement to this list. The 
       *    added element will be deleted when the list is deleted!
       */
      void addUserTrackElement(UserTrackElement* elm);

   private:
      /**
       *    Get the size of the elements in this list when stored in 
       *    a byte-buffer.
       */
      uint32 getSizeAsBytes();
};


// ================================ Implementation of inlined methods 


inline uint32 
UserTrackElement::getTime() const {
   return m_time;
}


inline int32 
UserTrackElement::getLat() const {
   return m_lat;
}


inline int32 
UserTrackElement::getLon() const {
   return m_lon;
}


inline uint32
UserTrackElement::getDist() const {
   return m_dist;
}


inline uint16
UserTrackElement::getSpeed() const {
   return m_speed;
}


inline uint16
UserTrackElement::getHeading() const {
   return m_heading;
}


inline const char* 
UserTrackElement::getSource() const {
   return m_source;
}

#endif // USERTRACKELEMENT_H

