/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPANDEDROUTE_H
#define EXPANDEDROUTE_H

#include "config.h"
#include "ExpandedRouteMatch.h"
#include "ExpandedRouteItem.h"
#include "StringTableUtility.h"

// Forward
class ExpandRouteReplyPacket;


/**
 * Describes a route in a way that is suitable for servers.
 *
 */
class ExpandedRoute {
   public:
      /**
       * Constructs a new empty ExpandedRoute.
       */
      ExpandedRoute();


      /**
       * Constructs a new ExpandedRoute from a ExpandRouteReplyPacket.
       *
       * @param p The ExpandRouteReplyPacket to extract data from.
       */
      ExpandedRoute( ExpandRouteReplyPacket* p );


      /**
       * Destructor, deletes all data in this ExpandedRoute.
       */
      virtual ~ExpandedRoute();


      /**
       * Prints content to out stream.
       *
       * @param out the ostream to print to.
       */
      void dump( ostream& out ) const;


      /**
       * Get the route's id. 0 is no id.
       *
       * @return The route's id.
       */
      uint32 getRouteID() const;


      /**
       * Set the route's id. 0 is no id.
       *
       * @param routeID The new route's id.
       */
      void setRouteID( uint32 routeID );


      /**
       * Get the route's createTime. 0 is invalid time.
       *
       * @return The route's createTime.
       */
      uint32 getRouteCreateTime() const;


      /**
       * Set the route's createTime. 0 is invalid time.
       *
       * @param createTime The new route's createTime.
       */
      void setRouteCreateTime( uint32 createTime );


      /**
       * The total distance of the route in meters.
       *
       * @return The total distance of the route in meters.
       */
      uint32 getTotalDistance() const;


      /**
       * The total distance of the route in meters printed to string.
       *
       * @param str The string to print to.
       * @param maxSize The maximum number of chars to write to str.
       * @return True if data did fit into the str.
       */
      bool getTotalDistanceStr( char* str, uint32 maxLength ) const;


      /**
       * Set the total distance of the route in meters.
       *
       * @param dist The new total distance of the route in meters.
       */
      void setTotalDistance( uint32 dist );


      /**
       * The total time of the route in seconds.
       *
       * @return The total time of the route in seconds.
       */
      uint32 getTotalTime() const;


      /**
       * The total time the route in meters printed to string.
       *
       * @param str The string to print to.
       * @param maxSize The maximum number of chars to write to str.
       * @return True if data did fit into the str.
       */
      bool getTotalTimeStr( char* str, uint32 maxLength ) const;


      /**
       * Set the total time of the route in seconds.
       *
       * @param time The new total time of the route in seconds.
       */
      void setTotalTime( uint32 time );


      /**
       * The standstill time of the route in seconds.
       *
       * @return The standstill time of the route in seconds.
       */
      uint32 getTotalStandStillTime() const;


      /**
       * The total standstill time the route in meters printed to string.
       *
       * @param str The string to print to.
       * @param maxSize The maximum number of chars to write to str.
       * @return True if data did fit into the str.
       */
      bool getTotalStandstillTimeStr( char* str, uint32 maxLength ) const;


      /**
       * Set the standstill time of the route in seconds.
       *
       * @param time The new standstill time of the route in seconds.
       */
      void setTotalStandStillTime( uint32 time );


      /**
       * The average speed of the route printed to string.
       *
       * @param str The string to print to.
       * @param maxSize The maximum number of chars to write to str.
       * @return True if data did fit into the str.
       */
      bool getAverageSpeedStr( char* str, uint32 maxLength ) const;


      /**
       * The vehicle type at the start of the route.
       * @return The starting vehicle.
       */
      ItemTypes::vehicle_t getStartingVehicle() const;


      /**
       * Set the vehicle type at the start of the route.
       * @param vehicle The new starting vehicle.
       */
      void setStartingVehicle( ItemTypes::vehicle_t vehicle );


      /**
       * Get the boundingbox for the route.
       *
       * @return The boundingbox for the route.
       */
      const MC2BoundingBox& getRouteBoundingBox() const;


      /**
       * @name Origins.
       * Methods to get and set the origin(s) of the route.
       */
      //@{
         /**
          * Get the number of origins for the route.
          * @return The number of origins for the route.
          */
         uint32 getNbrOrigins() const;


         /**
          * Get the origin i.
          *
          * @param i The index of the origin to get.
          * @return A ExpandedRouteMatch if i is < getNbrOrigins(),
          *         NULL is not.
          */
         const ExpandedRouteMatch* getOrigin( uint32 i ) const;


         /**
          * Add an origin.
          *
          * @param match The match to add, ownership is transfered to
          *              this class.
          */
         void addOrigin( ExpandedRouteMatch* match );
      //@}


      /**
       * @name Destinations.
       * Methods to get and set the destination(s) of the route.
       */
      //@{
         /**
          * Get the number of destinations for the route.
          * @return The number of destinations for the route.
          */
         uint32 getNbrDestinations() const;


         /**
          * Get the destination i.
          *
          * @param i The index of the destination to get.
          * @return A ExpandedRouteMatch if i is < getNbrDestinations(),
          *         NULL is not.
          */
         const ExpandedRouteMatch* getDestination( uint32 i ) const;


         /**
          * Add an destination.
          *
          * @param match The match to add, ownership is transfered to
          *              this class.
          */
         void addDestination( ExpandedRouteMatch* match );
      //@}


      /**
       * @name ExpandedRouteItems.
       * Methods to get ExpandedRouteItems and produce route descriptions.
       */
      //@{
         /**
          * Get the number of ExpandedRouteItems.
          *
          * @return The number of ExpandedRouteItems.
          */
         uint32 getNbrExpandedRouteItems() const;


         /**
          * Get the index'th ExpandedRouteItem.
          *
          * @param index The index of the ExpandedRouteItem to get.
          * @return The index'th ExpandedRouteItem or NULL if index is
          *         out of range.
          */
         const ExpandedRouteItem* getExpandedRouteItem( 
            uint32 index ) const;


         /**
          * Add ExpandedRouteItem.
          *
          * @param item The ExpandedRouteItem to add, the 
          *             ownership is transfered to this class.
          */
         void addExpandedRouteItem( ExpandedRouteItem* item );


         /**
          * Set the route description properties according to the specified
          * parameters.
          *
          * @param lang Language code of the route description.
          * @param compassLangShort If compass direction should be short.
          * @param maxTurnNumbers Maximum number of turns that should
          *                       be printed out. Eg "turn left at
          *                       the 9:th road". NB! Must be <= 9 for
          *                       now as expansion on overview maps sets
          *                       it to 10 to set it as invalid.
          * @param printSignPostText Set this to true if signpost
          *                          texts should be printed out.
          * @param distFormat Distance format used when printing
          *                   the distance.
          * @param wapFormat Set this to true if the text should be wap
          *                  formatted. All text that displayed on a wap
          *                  phone must be formatted (for instance swedish
          *                  characters must be replaced with escape-codes
          *                  and so on).
          */
         void setRouteDescriptionProperties( 
            StringTable::languageCode lang,
            bool compassLangShort,
            byte maxTurnNumbers,
            bool printSignPostText,
            StringTableUtility::distanceFormat distFormat,
            StringTableUtility::distanceUnit distUnit,
            bool wapFormat,
            const char* nameSeparator = "/" );


         /**
          * Creates a route description for a specific index.
          * For instance "Drive 660m then turn left into the
          * third street Baravägen".
          *
          * @param index The index of the description to get.
          * @param buf Preallocated buffer which will be filled with the
          *            routedescription.
          * @param maxLength The length of buf.
          * @return True If index is < getNbrExpandedRouteItems and the 
          *              route description did fit in the buffer, false 
          *              otherwise.
          */
         bool getRouteDescription( uint32 index, 
                                   char* buf, uint32 maxLength ) const;


         /**
          * Get the number of landmarks for ExpandedRouteItem i.
          *
          * @param index The index of the ExpandedRouteItem to get
          *              number of landmarks for.
          * @return The index'th ExpandedRouteItem number of landmarks
          *         or MAX_UINT32 if index is out of range.
          */
         uint32 getNbrLandmarksForExpandedRouteItem( uint32 index ) const;


         /**
          * Creates a route landmark description for a ExpandedRouteItem.
          * For instance "After 15 km pass Landskrona".
          *
          * @param index The index of the ExpandedRouteItem to get landmark
          *              description for.
          * @param i The landmark's index for the ExpandedRouteItem. Get
          *          valid landmark indexes from 
          *          getNbrLandmarksForExpandedRouteItem().
          * @param buf Preallocated buffer which will be filled with the
          *            route landmark description.
          * @param maxLength The length of buf.
          * @param nbrBytesWritten The number of bytes written into buf.
          * @return True If index is < getNbrExpandedRouteItems and i
          *              < getNbrLandmarksForExpandedRouteItem( index ) and
          *              the route landmark description did fit in the
          *              buffer, false otherwise.
          */
         bool getRouteLandmarkDescription( uint32 index, uint32 i,
                                           char* buf, uint32 maxLength ) const;


         /**
          * A static version of getRouteLandmarkDescription.
          *
          * @param buf Preallocated buffer which will be filled with the
          *            route landmark description.
          * @param maxLength The length of buf.
          * @param land The landmark to print.
          * @param turn The turn that thelandmark is for.
          * @param lang The language.
          * @param distFormat The format to use for distances.
          * @param transport The method of transportation at the turn.
          * @param nameSeparator The separator to separate names with.
          * @param showDist If to add "After 40km" to text, default true.
          * @param wapFormat If to wap escape text, default false.
          */
         static bool makeRouteLandmarkDescription( 
            char* buf, uint32 maxLength,
            const ExpandedRouteLandmarkItem* land,
            ExpandedRouteItem::routeturn_t turn,
            StringTable::languageCode lang,
            StringTableUtility::distanceFormat distFormat,
            StringTableUtility::distanceUnit distUnit,
            ItemTypes::transportation_t transport,
            const char* nameSeparator = "/",
            bool showDist = true,
            bool wapFormat = false );
      //@}


      /**
       * Write this ExpandedRoute  as a byte buffer.
       *
       * @param p    The packet where the ExpandedRoute will be saved.
       * @param pos  Position in p, will be updated when calling this 
       *             method.
       */
      uint32 save( Packet* p, int& pos ) const;

      /**
       * Get the size of this when stored in a byte buffer.
       *
       * @return  The number of bytes this will use when stored 
       *          in a byte buffer.
       */
      uint32 getSizeAsBytes() const;

      /**
       * Set all members with data from the buffer.
       *
       * @param p    The packet where the ExpandedRoute will be loaded 
       *             from.
       * @param pos  Position in p, will be updated when calling this 
       *             method.
       */
      void load( Packet* p, int& pos );

   private:
      /**
       * The routeID, 0 is no id.
       */
      uint32 m_routeID;


      /**
       * The route create time, 0 is invalid.
       */
      uint32 m_routeCreateTime;


      /**
       * The total distance.
       */
      uint32 m_dist;


      /**
       * The total time.
       */
      uint32 m_time;


      /**
       * The total standstill time.
       */
      uint32 m_standstillTime;


      /**
       * The start vehicle.
       */
      ItemTypes::vehicle_t m_startVehicle; 


      /**
       * The boundingbox for the route
       */
      MC2BoundingBox m_routeBoundingbox;


      /**
       * The origins.
       */
      vector<ExpandedRouteMatch*> m_origins;


      /**
       * The destinations.
       */
      vector<ExpandedRouteMatch*> m_destinations;


      /**
       * The route items.
       */
      vector<ExpandedRouteItem*> m_routeItems;


      /**
       * @name Route description parameters.
       * @memo Route description parameters.
       * @doc  Route description parameters.
       */
      //@{
         /**
          * The prefered language.
          */
         StringTable::languageCode m_lang;


         /**
          * If the compass should be in short language.
          */
         bool m_compassLangShort;


         /**
          * Maximum number of turns that should be printed out.
          */
         byte m_maxTurnNumbers;

         /**
          * If signpost texts should be printed out.
          */
         bool m_printSignPostText;

         /**
          * Distance format used when printing the distance.
          */
         StringTableUtility::distanceFormat m_distFormat;

         /**
          * Distance unit used when printing the distance.
          */
         StringTableUtility::distanceUnit m_distUnit;


         /**
          * If the text should be wap formatted.
          */
         bool m_wapFormat;


         /**
          * The separator to use between different names on roads.
          */
         MC2String m_nameSeparator;
      //@}

};


#endif // EXPANDEDROUTE_H

