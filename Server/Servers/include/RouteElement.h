/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTEELEMENT_4711_H
#define ROUTEELEMENT_4711_H

#include "config.h"
#include "Vector.h"
#include "StringTable.h"
#include "ItemTypes.h"
#include <vector>
#include "ExpandStringLane.h"
#include "ExpandStringSignPost.h"

// Forward
class ExpandedRouteLandmarkItem;


/**
 *   Class that represents a middle step when creating routes
 *   that should be sent to eBoxes and Navigators.
 *   The RouteElements should contain all the coordinates of the item.
 *   The first coordinate is the same as the last one of the previous item.
 *   The text and the turndescription are the ones to \emph{drive to},
 *   except for the first item which is a bit strange. It only contains one
 *   coordinate and that one is not useful, I think.
 *
 *   @see RouteList.
 */
class RouteElement {
public:
   /**
    *
    */
   enum navcrossingkind_t {
      NO_CROSSING = 0,
      THREE_WAY = 1,
      FOUR_WAY = 2,
      MULTI_WAY = 3
   };

   /**
    *   Creates a new, empty RouteElement.
    */
   RouteElement();

   /**
    *   Cleans up.
    */
   virtual ~RouteElement();

   /**
    *   Sets the text to an index in a NavStringTable.
    *   The stringtable must be held outside of the
    *   element.
    *   @param index The index to set.
    */
   void setText(int index);

   /**
    *   Gets the index in the NavStringTable for this element.
    *   @return The string index for the element.
    */
   int getText() const;
   
   /**
    *   Sets the text for a signpost on this element.
    */
   void setSignPostText(int index);

   /**
    *   @return True if this item has a signpost.
    */
   bool hasSignPostText() const;

   /**
    *   @return The index in the stringtable that this
    *           signpost was given in setSignPostText().
    *   @see hasSignPostText
    */
   int getSignPostText() const;
   
   /**
    *   Adds a coordinate pair to the element.
    *   @param lat The latitude.
    *   @param lon The longitude.
    */
   void addCoord(uint32 lat,
                 uint32 lon);

   /**
    *   Adds a speed to the element.
    *   @param speed The speed for the corresponding item
    */
   void addSpeed(uint8 speed);
   
   /**
    *   Adds a road attribute to the element.
    *   @param attribute The attribute for the corresponding item
    */
   void addAttribute(uint8 attribute);
   
   /**
    *   Get the last latitude.
    */
   uint32 getLastLat() const;

   /**
    *   Get the last longitude.
    */ 
   uint32 getLastLon() const;

   /**
    *   Get the latitude at index <code>idx</code>.
    */
   uint32 getLatAt(int idx) const;

   /** 
    *   Get the longitude at index <code>idx</code>.
    */ 
   uint32 getLonAt(int idx) const;


   /**
    *   Get the last speed.
    */
   uint8 getLastSpeed() const;
   
   /**
    *   Get the speed at index <code>idx</code>.
    */
   uint8 getSpeedAt(int idx) const;

   /**
    *   Get the attributes at index <code>idx</code>.
    */
   uint8 getAttributeAt(int idx) const;

   /**
    *   Get the attributes at index <code>idx</code> but check idx first.
    */
   uint8 getAttributeSafeAt( int idx ) const;

   /**
    *  Get the number of coordinates for this element.
    */
   int getNbrCoord() const;
   
   
   /**
    *   Sets the distance in the RouteElement.
    */
   void setDist(uint32 dist);

   /**
    *   Returns the distance to drive.
    */
   uint32 getDist() const;
   
   /**
    *   Returns the difference between the "bird-distance"
    *   and the real length of the segment.
    */
   int32 getDiffDist() const;
   
   /**
    *   Sets the turncode for the RouteElement.
    */
   void setTurnCode(StringTable::stringCode turn);

   /**
    *   Returns the turncode.
    */
   StringTable::stringCode getTurnCode() const;
   
   /**
    *   Sets the exit cound for the RouteElement.
    */
   void setExitCount(byte count);

   /**
    *   Returns the exit count.
    */
   byte getExitCount() const;
   

   /**
    *
    */
   void convertAndSetCrossingKind(ItemTypes::crossingkind_t t);
   void setCrossingKind(navcrossingkind_t t);
   navcrossingkind_t getCrossingKind() const;


   /**
    * Get the travell time of this part of the route description.
    *
    * @return  The time of this part of the routedescription.
    */
   uint32 getTime() const;

   
   /**
    * Set the travell time of this part of the route description.
    *
    * @param time  The time of this part of the routedescription.
    */
   void setTime( uint32 time );


   /**
    * Get the total travel time from start to this part of the route 
    * description.
    *
    * @return The total travel time from start to this part of the route 
    *         description.
    */
   uint32 getTotalTimeSoFar() const;


   /**
    * Set the total travel time from start to this part of the route 
    * description.
    *
    * @param totalTime The total travel time from start to this part of
    *                  the route description.
    */
   void setTotalTimeSoFar( uint32 totalTime );


   /**
    * Get the number of landmarks.
    */
   uint32 getNbrLandmarks() const;


   /**
    * Get the landmark at index i.
    *
    * @param i The index of the landmark to get.
    */
   const ExpandedRouteLandmarkItem* getLandmark( uint32 i ) const;


   /**
    * Add a new landmark.
    *
    * @param landmark The landmark to add. Is now owned by this class.
    */
   void addLandmark( ExpandedRouteLandmarkItem* landmark );


   /**
    * Set the type of transportation.
    */
   void setTransportationType( ItemTypes::transportation_t trans );


   /**
    * Get the type of transportation.
    */
   ItemTypes::transportation_t getTransportationType() const;

   /**
    * Set the lanes.
    */
   void setLanes( const ExpandStringLanesCont& lanes );

   /**
    * Set the SignPosts.
    */
   void setSignPosts( const ExpandStringSignPosts& signPosts );

   /**
    * Get the lanes.
    */
    const ExpandStringLanesCont& getLanes() const;

   /**
    * Get the SignPosts.
    */
    const ExpandStringSignPosts& getSignPosts() const;

private:

   /** Index for the text of this element */
   int m_textIdx;

   /** Index for the signpost of this element */
   int m_signIdx;

   /** SpeedLimits */
   Vector m_speedVect;
   
   /** Attributes such as motorway etc */
   Vector m_attrVect;
   
   /**  Latitude vector */
   Vector m_latVect;

   /** Longitude vector */
   Vector m_lonVect;

   /** The distance to drive on this Element */
   uint32 m_dist;

   /** 
    * The time to drive on this Element 
    */
   uint32 m_time;

   /** The turncode to the next element */
   StringTable::stringCode m_turn;

   /** The exit count */
   byte m_exitCount;
   
   /** 
    *    The crossing kind, in terms of RouteElement::navcrossingkind_t 
    *    (that is a subset of the ItemTypes::crossingkind_t).
    */
   navcrossingkind_t m_crossingKind;


   /**
    * The total travel time from start to this part of the route 
    * description.
    */
   uint32 m_totTime;


   /**
    * The landmarks for this part of the route. May be empty.
    */
   vector< ExpandedRouteLandmarkItem* > m_landmarks;

   /**
    * The lanes. Distance is reversed to be distance from turn.
    */
   ExpandStringLanesCont m_lanes;

   /**
    * The signs. Distance is reversed to be distance from turn.
    */
   ExpandStringSignPosts m_signPosts;

   /**
    * The transportation type.
    */
   ItemTypes::transportation_t m_trans;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline void
RouteElement::setLanes( const ExpandStringLanesCont& lanes ) {
   m_lanes = lanes;
}

inline void
RouteElement::setSignPosts( const ExpandStringSignPosts& signPosts ) {
   m_signPosts = signPosts;
}

inline const ExpandStringLanesCont& 
RouteElement::getLanes() const {
   return m_lanes;
}

inline const ExpandStringSignPosts& 
RouteElement::getSignPosts() const {
   return m_signPosts;
}

#endif
