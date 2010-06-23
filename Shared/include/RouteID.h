/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTE_ID_H
#define ROUTE_ID_H

#include "config.h"
#include "MC2SimpleString.h"
#include "BitBuffer.h"

/**
 *   Class containing a route ID.
 *   Copy constructor and operator= should work.
 */
class RouteID {
public:

   /**
    *   Creates an invalid route id.
    */
   RouteID();
   
   /**
    *   Creates a RouteID from id and creation time.
    *   (Should only be used in the server!)
    */
   RouteID(uint32 id, uint32 creationTime);

   /**
    *   Returns 0 if the RouteID is invalid.
    */
   int isValid() const;

   /**
    *   Creates a new RouteID from the supplied string.
    *   isValid must be run afterwards.
    */
   explicit RouteID(const char* str);

   /**
    *   Creates a RouteID from a nav2 int64.
    */
   RouteID(int64 nav2id);
   
   /**
    *   Creates an int64 which nav2 likes to use.
    */
   int64 toNav2Int64() const;
    
   /**
    *    Check if the two RouteIDs are same.
    */
   bool operator == ( const RouteID& other ) const;
   
   /**
    *    Returns true if this is less than other.
    *    Creation time is first which is good if
    *    putting them into a map when you want to
    *    remove the oldest.
    */
   bool operator< ( const RouteID& other ) const {
      if ( m_creationTime < other.m_creationTime ) return true;
      if ( m_creationTime > other.m_creationTime ) return false;
      return m_id < other.m_id;
   }


#ifdef MC2_SYSTEM
   /**
    *   Returns a string that can be parsed using
    *   the constructor using a string.
    *   Same format is used in XMLServer.
    */
   MC2SimpleString toString() const;
   
#endif
   /**
    *   Returns the number of bits the RouteID will use in a bitbuffer.
    */
   inline int nbrBitsInBitBuffer() const;

   /**
    *   Saves the routeID in a BitBuffer.
    */
   inline int save(BitBuffer* buf) const;
   
   /**
    *   Loads the routeID from a BitBuffer.
    */
   inline int load(BitBuffer* buf);

#ifdef MC2_SYSTEM
   /**
    *   Returns the route id number. Should only be used
    *   in server.
    */
   uint32 getRouteIDNbr() const;

   /**
    *   Returns the creation time.
    *   Should only be used
    *   in server.
    */
   uint32 getCreationTime() const;

   /**
    * Print on ostream.
    *
    * @param stream The stream to print on.
    * @param routeID  The RouteID to print.
    * @return The stream.
    */
   friend ostream& operator<< ( ostream& stream,
                                const RouteID& routeID )
   {
      return stream << routeID.getRouteIDNbr() << "," 
                    << routeID.getCreationTime();
   }

#endif
private:
   /**
    *   The id of the route.
    */
   uint32 m_id;
   
   /**
    *   The creation time of the route.
    */
   uint32 m_creationTime;
};

inline int
RouteID::nbrBitsInBitBuffer() const
{
   return 64;
}

inline int
RouteID::save(BitBuffer* buf) const
{
   buf->writeNextBits(m_id, 32);
   buf->writeNextBits(m_creationTime, 32);
   return 64;
}

inline int
RouteID::load(BitBuffer* buf)
{
   m_id = buf->readNextBits(32);
   m_creationTime = buf->readNextBits(32);
   return 64;
}

#endif

