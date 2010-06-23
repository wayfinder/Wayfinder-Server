/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPANDEDROUTELANDMARKITEM_H
#define EXPANDEDROUTELANDMARKITEM_H

#include "config.h"
#include "ItemTypes.h"
#include "SearchTypes.h"
#include "StringUtility.h"


class Packet;
class NameCollection;


/**
 *    Describes a landmark of a route.
 *
 */
class ExpandedRouteLandmarkItem {
   public:
      /**
       * Creates a new ExpandedRouteLandmarkItem.
       *
       * @param type The type of landmark, BUA, railway etc.
       * @param location How the landmark is located relative the route.
       * @param side Which side of the road the landmark is if appropriate.
       * @param atTurn If the landmark is at the turn or if it is passed
       *               on the way to the turn.
       * @param importance How important the landmark is, undefined.
       * @param dist How far on the route-item is the landmark located.
       * @param name The name(s) of the landmark, the ownership is 
       *             transfered to this class.
       */
      ExpandedRouteLandmarkItem( ItemTypes::landmark_t type,
                                 ItemTypes::landmarklocation_t location,
                                 SearchTypes::side_t side,
                                 bool atTurn,
                                 uint32 importance,
                                 uint32 dist,
                                 NameCollection* name,
                                 uint32 landmarkID,
                                 bool isTraffic = false,
                                 bool isDetour = false,
                                 bool isStart = true,
                                 bool isStop = true,
                                 const char* streetName = NULL);

      /**
       *  Create LandmarkItem from data buffer.
       */
      ExpandedRouteLandmarkItem( const Packet* packet, int& pos );
      
      /**
       * Destructor.
       */
      virtual ~ExpandedRouteLandmarkItem();


      /**
       * Prints content to out stream.
       *
       * @param out the ostream to print to.
       */
      void dump( ostream& out ) const;


      /**
       * Get the landmark type.
       *
       * @return The landmark type.
       */
      ItemTypes::landmark_t getType() const;


      /**
       * Set the landmark type.
       *
       * @param type The landmark type.
       */
      void setType( ItemTypes::landmark_t type );


      /**
       * Get the landmark location.
       *
       * @return The landmark location.
       */
      ItemTypes::landmarklocation_t getLocation() const;


      /**
       * Set the landmark location.
       *
       * @param location The landmark location.
       */
      void setLocation( ItemTypes::landmarklocation_t location );


      /**
       * Get the side of the road the landmark is.
       *
       * @return The side of the road the landmark is.
       */
      SearchTypes::side_t getRoadSide() const;


      /**
       * Set the side of the road the landmark is.
       *
       * @param side The side of the road the landmark is.
       */
      void setRoadSide( SearchTypes::side_t side );


      /**
       * Get if the landmark is at the turn.
       *
       * @return If the landmark is at the turn.
       */
      bool getAtTurn() const;


      /**
       * Get if the landmark is at the turn.
       *
       * @return If the landmark is at the turn.
       */
      void setAtTurn( bool atTurn );


      /**
       * Get the importance of the landmark.
       * NB! The importance is undefined.
       *
       * @return The importance of the landmark.
       */
      uint32 getImportance() const;


      /**
       * Set the importance of the landmark.
       *
       * @param importance The importance of the landmark.
       */
      void setImportance( uint32 importance );


      /**
       * Get the distance.
       *
       * @return How far on the route-item is the landmark located.
       */
      uint32 getDist() const;


      /**
       * Set the distance.
       *
       * @param dist How far on the route-item is the landmark located.
       */
      void setDist( uint32 dist );


      /**
       * Get the name(s) of the landmark.
       *
       * @return The NameCollection.
       */
      const NameCollection* getName() const;


      /**
       * Set the name(s) of the landmark.
       *
       * @param name The name(s) of the landmark, the ownership is 
       *             transfered to this class.
       */
      void setName( NameCollection* name );

      /**
       *  Get the size of this landmark in bytes.
       */
      uint32 getSizeAsBytes() const;

      /**
       * Load from packet.
       */
      void load( const Packet* packet, int& pos );

      /**
       * Save to packet.
       */
      uint32 save( Packet* packet, int& pos ) const;

      void setIsTraffic(bool traffic)
         { m_isTraffic = traffic;}
      
      void setIsStart(bool start)
         { m_isStart = start;}
      
      void setIsStop(bool stop)
         { m_isStop = stop;}
       
      void setIsDetour(bool detour)
         { m_isDetour = detour; }

      bool isTraffic() const
         { return m_isTraffic; }
      
      bool isStart() const
         { return m_isStart; }
      
      bool isStop() const
         { return m_isStop; }
      
      bool isDetour() const
         {return m_isDetour; }

      void addStreetName(const char* name)
         { m_streetName = StringUtility::newStrDup(name); }
      
      const char* getStreetName() const
         { return m_streetName;}
      
      uint32 getLandmarkID() const
         {return m_landmarkID; }

      
   private:
      /**
       * The type of landmark.
       */
      ItemTypes::landmark_t m_type;


      /**
       * How the landmark is located relative the route.
       */
      ItemTypes::landmarklocation_t m_location;


      /**
       * Which side of the road the landmark is if appropriate.
       */
      SearchTypes::side_t m_side;


      /**
       * If the landmark is at the turn.
       */
      bool m_atTurn;


      /**
       * How important the landmark is, undefined.
       */
      uint32 m_importance;


      /**
       * How far on the route-item is the landmark located.
       */
      uint32 m_dist;


      /**
       * The name(s) of the landmark
       */
      NameCollection* m_name;

      /**
       *  The id of the landmark.
       */
      uint32 m_landmarkID;
      

      bool m_isTraffic;
      bool m_isDetour;
      bool m_isStart;
      bool m_isStop;
      const char* m_streetName;
      
};

/**
 *    
 */
class ExpandedRouteLandmarkVector : public vector<ExpandedRouteLandmarkItem*>
{
   public:
      /**
       *    Delete the vector and all elements in it (no matter where 
       *    they are allocated).
       */
      ~ExpandedRouteLandmarkVector();

      /**
       *   Store the list in a packet.
       *   @param packet The packet to store this vector in.
       *   @param pos    Reference the position to use, will point to the
       *                 next pos after the last ExpandedRouteLandmarkItem 
       *                 when returning.
       */
      void store( Packet* packet, int& pos ) const;

      /**
       *   Restore a vector from a packet.
       *   @param packet The packet to restore this lvector from.
       *   @param pos    Reference the position to use, will point to the
       *                 next pos after the last ExpandedRouteLandmarkItem 
       *                 when returning.
       */
      void restore( const Packet* packet, int& pos );

      /**
       *    Add an already created ExpandedRouteLandmarkItem to this list. 
       *    The added element will be deleted when the vector is deleted!
       */
      void addExpandedRouteLandmark(ExpandedRouteLandmarkItem* item);

      /**
       *    Get the size of the elements in this vector when stored in 
       *    a byte-buffer.
       */
      uint32 getSizeAsBytes()const ;

};


#endif // EXPANDEDROUTELANDMARKITEM_H

