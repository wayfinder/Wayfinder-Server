/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPANDROUTELIST_H
#define EXPANDROUTELIST_H

#include "config.h"
#include "CCSet.h"
#include "ExpandRoutePacket.h"
#include "RouteableItem.h"
#include <list>
#include <vector>
#include "ExpandStringLane.h"
#include "ExpandStringSignPost.h"

// Forward declaration
class ExpandRouteHead;

/**
 *    Describes one part of the route. Used by the ExpandRouteProcessor
 *    when expanding the route into text.
 *
 */
class ExpandRouteLink : public Link {
   friend class ExpandRouteHead;

   public:

      /**
       *    Create new part of the route. Light version.
       *    @param   nodeID   Node id of the this part of the route.
       *                      MAX_UINT32 if not specified.
       *    @param   item     The item asociated with this part of the 
       *                      route. NULL if not specified.
       */
      ExpandRouteLink(uint32 nodeID = MAX_UINT32, 
                      RouteableItem* item = NULL);

     
      /**
       *    Create a new part of the route.
       *    @param   str   The name of the street.
       *    @param   stringCode  The turndescription in terms of index
       *                   into the StringTable.
       *    @param   dist  The distance for this part of the route.
       *    @param   time  The time for this part of the route.
       *    @param   transport The type of transportation for this part of 
       *                   the route.
       *    @param   lat   The latitude part of the coordinate where to 
       *                   turn.
       *    @param   lon   The longitude part of the coordinate where to 
       *                   turn.
       *    @param   nbrItems The number of items that this link refers to.
       *    @param   nameType The type of the name.
       *    @param   turnNumber  The number of the street where to turn. To
       *                   be able to say "turn left at the 3:rd street to
       *                   the left".
       *    @param   leftTurnNumber    The number of turns to the left.
       *    @param   rightTurnNumber   The number of turns to the right.
       *    @param   rbExitNumber      The number of exits in a roundabout.
       * @param lanes The lanes of this turn.
       * @param signPosts The signposts of this turn.
       *    @param   driveOnRightSide  True if driving on right side, 
       *                               otherwise false.
       */
      ExpandRouteLink( const char* str, 
                       uint32 stringCode, 
                       uint32 dist, 
                       uint32 time,
                       ItemTypes::transportation_t transport, 
                       int32 lat, 
                       int32 lon,
                       uint32 nbrItems,
                       uint8 nameType, 
                       byte turnNumber,
                       byte restrictedRbExit,
                       const ExpandStringLanesCont& lanes,
                       const ExpandStringSignPosts& signPosts,
                       bool driveOnRightSide );

      /**
       *    Delete this object and release the memory allocated here.
       */
      virtual ~ExpandRouteLink();

      /**
       *    Add the times of this link according to the parameters.
       *    However if the transportation type is walking, then the 
       *    time is calculated as the distance / 5km/h.
       *    @param   time           The time.
       *    @param   standStillTime The standstill time.
       */
      void addTimes(uint32 time, uint32 standStillTime);

      /**
       *    Set the times of this link according to the parameters.
       *    @param   time           The time.
       *    @param   standStillTime The standstill time.
       */
      void setTimes(uint32 time, uint32 standStillTime);
      
      /**
       *    Print information about this link to stdout.
       */
      void dump();
      
      /**
       *    Concatinate this link with an other one. The fields that
       *    will be updated are distance, time and turn number.
       *    @param   otherLink   The other link that will be used when
       *                         concatinating the links.
       *    @param   after       True if otherlink is after this link.
       *    @return  True if the concatination was successfull, false
       *             otherwise.
       */
      bool concatinateWith(ExpandRouteLink* otherLink, bool after);

      /**
       *    Add the information in this link to a packet.
       *    @param   p  The packet where the data should be aadded.
       *    @return  True if all data is saved correctly, false 
       *             otherwise.
       */
      bool addToPacket(ExpandRouteReplyPacket* p, bool triss);

      /**
       *    Add a new name to this part of the route.
       *    @param   newName  The new name for this part of the route. The
       *                      name is copied into this object (and the old
       *                      name is deleted).
       */
      void setNewName(const char* newName);

      /**
       *    Add all  names to this part of the route.
       *    @param   newName  All the names for this part of the route. The
       *                      names are copied into this object (and the old
       *                      names are deleted).
       */
      void setNewAllName(const char* newName);

      /**
       * Add a set of lanes.
       */
      void addLane( const ExpandStringLanes& lanes );

      /**
       * Add a set of lanes, as first.
       */
      void addLaneFirst( const ExpandStringLanes& lanes );

      /**
       * Check if has active lanes.
       */
      bool hasNonStopOfLanesLane() const;

      /**
       * Add a signpost.
       */
      void addSignPost( const ExpandStringSignPost& signPost );

      /**
       *  Returns True if the name of the link i "missing" or NULL.
       *  @return True if the name of the link i "missing" or NULL.
       */
      bool noName();
      
      /**
       *  Add a possible turn at this location.
       *  @return True if it was possible to add more turns.
       */
      bool addPossibleTurn(StringTable::stringCode possTurn);
      
      /**
       * The number of turns that was possible BUT NOT made at this location.
       */      
      uint32 getNbrPossibleTurns();

      /**
       *  Get one possible turn at this location with the given index.
       *  @return NO_TURN if the index was wrong ( eg. 2nd possible turn in
       *          a 3-way crossing), UNDEFINED if no possible turn was set at
       *          this index.
       */
      StringTable::stringCode  getPossibleTurn(uint32 index);


      void addPassedStreet(const char* streetNames);

      /**
       *    Set the driving side of this link.
       *    @param   driveOnRightSide  True if driving on right side.
       */
      inline void setDrivingSide( bool driveOnRightSide );
      
      /**
       *    @return  True if driving on right side on this link,
       *             otherwise false.
       */
      inline bool driveOnRightSide() const;

      /**
       *  Replace the current name with a new.
       *  Will also remove the new name from the allStringCodes list.
       *  @param newStringCode StringCode for the new name.
       */
      void replaceName(uint32 newStringCode); 

   /// Sets the corresponding member variable
   inline void setTurnDescription( StringTable::stringCode standStillTime );
   /// Returns the corresponding member variable
   inline StringTable::stringCode getTurnDescription() const;

   /// Sets the corresponding member variable
   inline void setNbrPossTurns( uint32 nbrPossTurns );
   /// Returns the corresponding member variable
   inline uint32 getNbrPossTurns() const;

   /// Sets the corresponding member variable
   inline void setCrossingKind( ItemTypes::crossingkind_t crossingKind );
   /// Returns the corresponding member variable
   inline ItemTypes::crossingkind_t getCrossingKind() const;

   /// Sets the corresponding member variable
   inline void setTurnNumber( byte turnNumber );
   /// Returns the corresponding member variable
   inline byte getTurnNumber() const;

   /// Sets the corresponding member variable
   inline void setRestrictedRbExit( byte restrictedRbExit );
   /// Returns the corresponding member variable
   inline byte getRestrictedRbExit() const;

   /// Sets the corresponding member variable
   inline void setExitCount( byte exitCount );
   /// Returns the corresponding member variable
   inline byte getExitCount() const;

   /// Sets the corresponding member variable
   inline void setStandStillTime( uint32 standStillTime );
   /// Returns the corresponding member variable
   inline uint32 getStandStillTime() const;
   
   /// Sets the corresponding member variable
   inline void setLandmarks( LandmarkHead* landmarks );
   /// Returns the corresponding member variable
   inline LandmarkHead* getLandmarks() const;

   /// Sets the corresponding member variable
   inline void setSignPosts( const ExpandStringSignPosts& signPosts );
   /// Returns the corresponding member variable
   inline const ExpandStringSignPosts& getSignPosts() const;

   /// Sets the corresponding member variable
   inline void setLanes( const ExpandStringLanesCont& lanes );
   /// Returns the corresponding member variable
   inline const ExpandStringLanesCont& getLanes() const;

   /// Sets the corresponding member variable
   inline void setItem( RouteableItem* item );
   /// Returns the corresponding member variable
   inline RouteableItem* getItem() const;

   /// Sets the corresponding member variable
   inline void setStringCode( uint32 stringCode );
   /// Returns the corresponding member variable
   inline uint32 getStringCode() const;

   /// Sets the corresponding member variable
   inline void setNameIsExitNr( bool nameIsExitNr  );
   /// Returns the corresponding member variable
   inline bool getNameIsExitNr() const;

   //private:
      /**
       *    @name Return values
       *    @memo Members to hold the values to insert into the reply packet.
       *    @doc  Members to hold the values thet waill be inserted into
       *          to the reply packet when the processing is finished.
       */
      //@{
         /// The ID of the node
         uint32 m_nodeID;

         /// The item.
         RouteableItem* m_item;
      
         /// The choosen streetname.
         char* m_streetName;

         uint32 m_stringCode;

         list<uint32> m_allStringCodes;

         /**
          *  All streetnames of this link. If this link consists of
          *  many items this is all the names of the first.
          *  (The name you are turning into.)
          */
         char* m_allStreetNames;
         
         /// The turndescription, in terms of the string table.
         StringTable::stringCode m_turnDescription;
             
         /**
          * The other turns that was possible BUT NOT made at this location.
          */
         StringTable::stringCode m_possTurns[7];
         
private:
         bool m_nameIsExitNr;

         /**
          * The number of turns that was possible BUT NOT made at this
          * location.
          */
         uint32 m_nbrPossTurns;
         
         /**
          *    The crossing kind for this part of the route, used during
          *    filtering the route description.
          */
         ItemTypes::crossingkind_t m_crossingKind;
public:
         /// The distance for this part of the route.
         uint32 m_dist;

         /// The time for this part of the route.
         uint32 m_time;

private:
         /// The time for this part of the route.
         uint32 m_standStillTime;

public:
         /// The transportation type for this part of the route.
         ItemTypes::transportation_t m_transportation;
         
         /// The latitude part of the turn-position
         int32 m_latitude;
      
         /// The longitude part of the turn-position
         int32 m_longitude;

         /// The number of items this turn descriptions refers to.
         uint32 m_nbrItems;
      
         /// The type of name
         uint8 m_nameType;

         /**
          *    The turn-number (e.g. take the m_turnNumber:th street to 
          *    the right).
          */
         byte m_turnNumber;

private:
         /**
          *  The number of restricted roundabout exits passed.
          */
         byte m_restrictedRbExit; 
public:
         /**
          * The lanes.
          */
         ExpandStringLanesCont m_lanes;

         /**
          * The signs.
          */
         ExpandStringSignPosts m_signPosts;

         /**
          *    Is this a "E-road" or not. E-road are the roads that have 
          *    name like "E4", "E20" and "E22". 
          */
         bool m_eroad;

         /**
           *   A list with the landmarks of this routelink
           */
         LandmarkHead* m_landmarks;

         ///  The exit count of the turn.
         byte m_exitCount;

         /// Names of streets passed to the left of the route.
         list<char*> m_passedStreets;

         /**
          * The last item in the link. This will be the same as m_item
          * before concatinations.
          */
         RouteableItem* m_lastItem;

         uint32 m_lastNodeID;

         /// True if driving on right side, false otherwise.
         bool m_driveOnRightSide;

      //@}
};

/**
 *    The list that have the route-items (objects of the ExpandRouteLink
 *    class). This list have the possibility to filter the route 
 *    description in some different ways by looping over the Links. It
 *    can also save the route-items into a packet.
 *
 */
class ExpandRouteHead : public Head {
   public:
      /**
       *    Default constructor. Does not do anything except calling
       *    the constructor in the superclass (Head).
       */
      ExpandRouteHead();
   
      /**
       *    Delete memory allocated here.
       */
      virtual ~ExpandRouteHead();

      /**
       *    @return  The number of items in the list. A negative value
       *             is returned upon error.
       */
      int dump();
      
      /**
       *    This method could be used to remove parts of the route without
       *    name when all route description items are inserted in the list.
       *    Only no-names when driving straight ahead is removed.
       *    
       *    @param   maxDist  The maximum length of the part without name
       *                      that will be removed.
       *    @return  The number of items removed. A negative value is 
       *             returned upon error.
       */
      int removeStreetsWithoutName(uint32 maxDist);

      /**
       *    Loop through the route and remove all "ahead turns". That is
       *    for example when a street changes the name, but the turn 
       *    description is straight ahead. No roundabout will be removed!
       *    @return  The number of items that have been removed. A negative 
       *             value is returned upon error.
       */
      //int removeAheadTurns();

     
      /**
       *    Save the string data route-items in this route into a packet.
       *    @param   reply The packet where the route will be saved.
       *                   Must be allocated before calling this method!
       *    @return  True if the route is saved correctly into the
       *             packet, false otherwise.
       */
      bool saveStringDataIntoPacket(ExpandRouteReplyPacket* reply,
                                    bool triss);

      /**
       *    Save the number of items per string in this route into a packet.
       *    Note that this method must be called after 
       *    saveStringDataIntoPacket() and after adding item data to the
       *    packet.
       *    @param   reply The packet where the items per string 
       *                   will be saved. saveStringDataIntoPacket()
       *                   must be called before calling this method!
       *                   Must be allocated before calling this method!
       *    @return  True if the number of items per string is saved 
       *             correctly into the packet, false otherwise.
       */
      bool saveItemsPerStringIntoPacket(ExpandRouteReplyPacket* reply);
      
   //private:
      /**
       *    Concatinate two links into one. The one given as parameter
       *    must be the one that have the turndescription that will be 
       *    used after the concatination. {\it {\bf NB!} One link is
       *    removed from the list and could therefore not be used 
       *    anymore!}.
       *    @param   masterLink  The link that will be left in the list,
       *                         but with updated memebers.
       *    @param   after       True if masterLink should be concatinated
       *                         with the one that comes after, false it
       *                         is should be concatinated with the one
       *                         before.
       *                         
       */
      void concatinateLinks(ExpandRouteLink* masterLink, 
                            bool after = true);

      /**
       *    The streetCounts of the last segment to include in the reply
       */
      byte m_lastSegmentLeftStreetCount; 
      byte m_lastSegmentRightStreetCount;
};

// Inlines
      
inline void 
ExpandRouteLink::setDrivingSide( bool driveOnRightSide ) {
   m_driveOnRightSide = driveOnRightSide;
}

inline bool
ExpandRouteLink::driveOnRightSide() const {
   return m_driveOnRightSide;
}

inline void 
ExpandRouteLink::setTurnDescription(StringTable::stringCode turnDescription) {
   m_turnDescription = turnDescription;
}

inline StringTable::stringCode 
ExpandRouteLink::getTurnDescription() const {
   return m_turnDescription;
}

inline void 
ExpandRouteLink::setNbrPossTurns( uint32 nbrPossTurns ) {
   m_nbrPossTurns = nbrPossTurns;
}

inline uint32 
ExpandRouteLink::getNbrPossTurns() const {
   return m_nbrPossTurns;
}

inline void 
ExpandRouteLink::setCrossingKind( ItemTypes::crossingkind_t crossingKind ) {
   m_crossingKind = crossingKind;
}

inline ItemTypes::crossingkind_t 
ExpandRouteLink::getCrossingKind() const {
   return m_crossingKind;
}

inline void 
ExpandRouteLink::setTurnNumber( byte turnNumber ) {
   m_turnNumber = turnNumber;
}

inline byte 
ExpandRouteLink::getTurnNumber() const {
   return m_turnNumber;
}

inline void 
ExpandRouteLink::setRestrictedRbExit( byte restrictedRbExit ) {
   m_restrictedRbExit = restrictedRbExit;
}

inline byte 
ExpandRouteLink::getRestrictedRbExit() const {
   return m_restrictedRbExit;
}

inline void 
ExpandRouteLink::setExitCount( byte exitCount ) {
   m_exitCount = exitCount;
}

inline byte 
ExpandRouteLink::getExitCount() const {
   return m_exitCount;
}

inline void 
ExpandRouteLink::setStandStillTime(uint32 standStillTime) {
   m_standStillTime = standStillTime;
}

inline uint32 
ExpandRouteLink::getStandStillTime() const {
   return m_standStillTime;
}

inline void 
ExpandRouteLink::setLandmarks( LandmarkHead* landmarks ) {
   m_landmarks = landmarks;
}

inline LandmarkHead* 
ExpandRouteLink::getLandmarks() const {
   return m_landmarks;
}

inline void 
ExpandRouteLink::setSignPosts( const ExpandStringSignPosts& signPosts ) {
   m_signPosts = signPosts;
}

inline const ExpandStringSignPosts& 
ExpandRouteLink::getSignPosts() const {
   return m_signPosts;
}

inline void 
ExpandRouteLink::setLanes( const ExpandStringLanesCont& lanes ) {
   m_lanes = lanes;
}

inline const ExpandStringLanesCont& 
ExpandRouteLink::getLanes() const {
   return m_lanes;
}

inline void 
ExpandRouteLink::setItem( RouteableItem* item ) {
   m_item = item;
}

inline RouteableItem* 
ExpandRouteLink::getItem() const {
   return m_item;
}

inline void 
ExpandRouteLink::setStringCode( uint32 stringCode ) {
   m_stringCode = stringCode;
}

inline uint32 
ExpandRouteLink::getStringCode() const {
   return m_stringCode;
}

inline void ExpandRouteLink::setNameIsExitNr( bool nameIsExitNr  ) {
   m_nameIsExitNr = nameIsExitNr;
}

inline bool ExpandRouteLink::getNameIsExitNr() const {
   return m_nameIsExitNr;
}

#endif

