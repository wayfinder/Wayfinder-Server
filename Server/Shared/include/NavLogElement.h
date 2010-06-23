/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVLOGELEMENT_H
#define NAVLOGELEMENT_H

#include "config.h"
#include "Packet.h"
#include <list>

/**
 *    Describes a log entry in the driver's log.
 *
 */
class NavLogElement {
   public:
      /**
       * Create a new NavLogElement from a data buffer.
       */
      NavLogElement( const Packet* p, int& pos );


      /**
       * Create a new NavLogElement from data sent as parameters.
       *
       * @param dist    The distance driven in this element
       * @param meterCounter  The meter reading, in km.
       * @param work    Is this distance driven when working?
       * @param comment  Comments for this driving.
       * @param startTime The start time of driving.
       * @param startLat  The start latitude of the driving.
       * @param startLon  The start longitude of the driving.
       * @param startStr  The string describing the start.
       * @param endTime The end time of driving.
       * @param endLat  The end latitude of the driving.
       * @param endLon  The end longitude of the driving.
       * @param endStr  The string describing the end.
       * @param id      The id of the NavLogElement, default MAX_UINT32.
       */
      NavLogElement( uint32 dist, uint32 meterCounter, bool work, 
                     const char* comment,
                     uint32 startTime, int32 startLat, int32 startLon,
                     const char* startStr,
                     uint32 endTime, int32 endLat, int32 endLon,
                     const char* endStr,
                     uint32 id = MAX_UINT32 );


      /**
       *    Destructor.
       */
      ~NavLogElement();

      /**
       * Get the unique ID.
       * 
       * @return The ID of this NavLogElement.
       */
      inline uint32 getID() const;

      /**
       *    Get the distance of this journey.
       *    @return The distnace.
       */
      inline uint32 getDist() const;

      /**
       *    Get values of the meter counter.
       *    @return The meter counter.
       */
      inline uint32 getMeterCounter() const;

      /**
       *    Find out if this journey was driven at work or not.
       *    @return If work.
       */
      inline bool getWork() const;


      /**
       *    Get the comment for this journey.
       *    @return The comment.
       */
      inline const char* getComment() const;

      /**
       *    Get the time, in UTC, for when the journey started (Unix time).
       *    @return The start time, in UTC.
       */
      inline uint32 getStartTime() const;

      /**
       *    Get the latitude for where the journey begun.
       *    @return Latitude for where the car started the journey.
       */
      inline int32 getStartLat() const;

      /**
       *    Get the longitude for where the journey begun.
       *    @return Longitude for where the car started the journey.
       */
      inline int32 getStartLon() const;

      /**
       *    Get a pointer to the description of the start of the journey.
       *    @return The start string.
       */
      inline const char* getStartString() const;

      /**
       *    Get the time, in UTC, for when the car parked (Unix time).
       *    @return The end time, in UTC.
       */
      inline uint32 getEndTime() const;

      /**
       *    Get the latitude for where the car parked.
       *    @return Latitude for where the car parked at the end of this 
       *            journey.
       */
      inline int32 getEndLat() const;

      /**
       *    Get the longitude for where the car parked.
       *    @return Longitude for where the car parked at the end of this 
       *            journey.
       */
      inline int32 getEndLon() const;

      /**
       *    Get a pointer to the description of the stop.
       *    @return The end string.
       */
      inline const char* getEndString() const;


      /**
       *    Write this nav log element as a byte buffer.
       *    @param p    The packet where the NavLogElement will be saved.
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
       *    @param p    The packet where the NavLogElement will be loaded 
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
       * The ID number of this nav log element (unique for all users).
       */
      uint32 m_id;


      /**
       *    The driven distance for this journey.
       */
      uint32 m_dist;

      /**
       *    The meter counter. The usage of this variable is not 
       *    completely defined yet!
       */
      uint32 m_meterCounter;

      /**
       *    Flag to set if this journey is done when working or not.
       */
      bool m_work;

      /**
       *    A comment for this journey. Might describe the purpose or 
       *    something like that.
       */
      char* m_comment;

      /**
       *    The time (in UTC, Unix time) when the car started.
       */
      uint32 m_startTime;

      /**
       *    Latitude part of the coordinate for where the car started 
       *    (in mc2-units).
       */
      int32 m_startLat;

      /**
       *    Longitude part of the coordinate for where the car started 
       *    (in mc2-units).
       */
      int32 m_startLon;

      /**
       *    The description of the start, might be the street name and/or
       *    name of visited company.
       */
      char* m_startStr;

      /**
       *    The time (in UTC, Unix time) when the car was parked.
       */
      uint32 m_endTime;

      /**
       *    Latitude part of the coordinate for where the car parked
       *    (in mc2-units).
       */
      int32 m_endLat;

      /**
       *    Longitude part of the coordinate for where the car parked
       *    (in mc2-units).
       */
      int32 m_endLon;

      /**
       *    The description of the stop, might be the street name and/or
       *    name of visited company.
       */
      char* m_endStr;
};


/**
 *    Describes a list of NavLogElements. Can store it self in a packet 
 *    and will delete all elements inserted in the list when deleted.
 *
 */
class NavLogElementsList : public list<NavLogElement*> {
   public:
      /**
       *    Default contructor, does nothing.
       */
      NavLogElementsList();

      /**
       *    Copy contructor. Currently dummy implementation to avoid 
       *    usage!
       *    @param src  The object to initialize the new List from.
       */
      NavLogElementsList( const NavLogElementsList& src);
      
      /**
       *    Delete the list and all elements in it (no matter where 
       *    they are allocated).
       */
      ~NavLogElementsList();

      /**
       *   Store the list in a packet.
       *   @param packet The packet to store this list in
       *   @param pos    Reference the position to use, will point to the
       *                 next pos after the last NavLogElement when 
       *                 returning.
       */
      void store(Packet* packet, int& pos);

      /**
       *   Restore a list from a packet.
       *   @param packet The packet to restore this list from.
       *   @param pos    Reference the position to use, will point to the
       *                 next pos after the last NavLogElement when 
       *                 returning.
       */
      void restore(const Packet* packet, int& pos);

      /**
       *    Add an already created NavLogElement to this list. The 
       *    added element will be deleted when the list is deleted!
       */
      void addNavLogElement(NavLogElement* elm);

   private:
      /**
       *    Get the size of the elements in this list when stored in 
       *    a byte-buffer.
       */
      uint32 getSizeAsBytes();
};


// ================================ Implementation of inlined methods 


uint32
NavLogElement::getID() const 
{
   return m_id;
}


uint32
NavLogElement::getDist() const 
{
   return m_dist;
}


uint32
NavLogElement::getMeterCounter() const 
{
   return m_meterCounter;
}


bool
NavLogElement::getWork() const 
{
   return m_work;
}


const char*
NavLogElement::getComment() const 
{
   return m_comment;
}


uint32 
NavLogElement::getStartTime() const 
{
   return m_startTime;
}


int32 
NavLogElement::getStartLat() const 
{
   return m_startLat;
}


int32 
NavLogElement::getStartLon() const 
{
   return m_startLon;
}


const char* 
NavLogElement::getStartString() const 
{
   return m_startStr;
}


uint32 
NavLogElement::getEndTime() const 
{
   return m_endTime;
}


int32 
NavLogElement::getEndLat() const 
{
   return m_endLat;
}


int32 
NavLogElement::getEndLon() const 
{
   return m_endLon;
}


const char* 
NavLogElement::getEndString() const 
{
   return m_endStr;
}


#endif // NAVLOGELEMENT_H

