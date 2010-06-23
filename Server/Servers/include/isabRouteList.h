/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ISABROUTELIST_H
#define ISABROUTELIST_H

#include "config.h"
#include <vector>
#include "GDRGB.h"


// forward
class IsabRouteElement;
class isabBoxSession;
class NavStringTable;
class NavSession;
class RouteList;
class WPTorTPTElement;
class WPTElement;
class RouteElement;
class LandmarkPTElement;


/**
 *    Class representing a route in the Complex isabigator format...
 */
class IsabRouteList {
public:
   typedef vector< GDUtils::Color::RGB > colorTable_t;

   /**
    * Creates a new IsabrouteList.
    *
    * @param The routeList to use when creating IsabFormat.
    * @param reqVer The request version, default 0.
    * @param colorTable The colortable to use for signposts. Default NULL
    *                   and indexes into GDColor is used.
    */
   IsabRouteList( int protoVer,
                  RouteList* routeList,
                  NavStringTable* navStringTable,
                  NavSession* session,
                  uint8 reqVer = 0,
                  colorTable_t* colorTable = NULL );
             
   /**
    *   Cleans up.
    */
   virtual ~IsabRouteList();

   /**
    *   Populates the List using RouteList.
    */
   bool fillList();

   /**
    *   Return the number of items in the List.
    */
   int getSize() const;
   int getSizeAdditional() const;

   /**
    *   Return the number of strings used in List from NavStringTable.
    */
   int getNbrStringsUsed() const;

   /**
    *   Returns the IsabRouteElement at position idx.
    *   @param idx The index in the vector.
    *   @return The isabRouteElement at that position.
    */
   const IsabRouteElement* isabRouteElementAt(int idx) const;
   const IsabRouteElement* isabAdditionalRouteElementAt(int idx) const;

   bool hasAdditionalRouteData() const;
   bool isValid(void) { return m_valid; }

   /**
    *    Get the element at position pos. {\it {\bf NB!} No check is done 
    *    against writing outside the buffer!}
    *    @param   pos   The position of the element to return.
    */
   inline const IsabRouteElement* operator[](uint32 pos) const {
      return isabRouteElementAt(pos);
   }


   /**
    *   Returns true if the route is truncated.
    */
   bool isTruncated() const;


   /**
    *   Returns the length of the route, if the route is truncated
    *   the distance is for the truncated route and not the
    *   entire roure
    */
   int getTruncatedDist() const;


   /**
    * Get the index of where the route was truncated.
    */
   uint32 getTruncatedWPTNbr() const;


   /**
    * Get the distance from the trunkation point to the next waypoint.
    *
    * @return The distance from the trunkation point to the next waypoint.
    */
   uint32 getDist2nextWPTFromTrunk() const;
   
  private:

   class historyPoint {
      public:
         historyPoint( float32 axDiff, float32 ayDiff, uint8 aspeed,
                       float64 adistLastWptTptToCurr )
            : xDiff( axDiff ), yDiff( ayDiff ), speed( aspeed ),
            distLastWptTptToCurr( adistLastWptTptToCurr )
         {}

         float32 xDiff;
         float32 yDiff;
         uint8   speed;
         float64 distLastWptTptToCurr;
   };

   typedef std::vector< IsabRouteElement* > IsabRouteElementVector;

   /**
    * Checks if a new timeDistLeft node is needed and adds it if so.
    *
    */
   static void checkTimeDistLeft( float64& distLeft,
                                  IsabRouteElementVector& isabRouteVector, int& i, int& j,
                                  const RouteElement* re, 
                                  const RouteElement* preRe, 
                                  int& lastTimeDistLeftPointIndex,
                                  uint32& totSize, uint8& currentSpeed );

   /**
    * Runs thru a RouteList and makes a isabRouteList.
    *
    */
   static void convertRouteList( RouteList* routeList, int& i, int& j,
                                 uint32 maxSize, uint32 maxDist,
                                 uint8 protoVer,
                                 bool addTDP,
                                 float64& radLon, float32& scaleX, 
                                 float32& origoX, 
                                 float64& radLat, float32& scaleY, 
                                 float32& origoY, 
                                 float32& xDiff, float32& yDiff,
                                 int32& lastMc2Lat, int32& lastMc2Lon,
                                 float64& lastRadLat, float64& lastRadLon,
                                 WPTorTPTElement*& lastWptOtTpt,
                                 float64& distLastWptTptToCurr,
                                 int& nbrStringsUsed,
                                 bool& truncated,
                                 uint32& truncatedWPTNbr,
                                 uint32& dist2nextWPTFromTrunk,
                                 WPTElement*& trunkated_end_element,
                                 uint32& totSize, float64& dist,
                                 float64& distLeft,
                                 isabBoxSession* session,
                                 IsabRouteElementVector& isabRouteVector, 
                                 NavStringTable* stringTable,
                                 int& lastTimeDistLeftPointIndex,
                                 uint8& currentSpeed );


   /**
    * FillList for version seven and up.
    */
   bool fillList7p();


   /**
    * Handle coordinate cache in fillList7p.
    *
    * @param history Vector of coordinates.
    * @param lastXDiff The last added coordinate, may be updated.
    * @param lastYDiff The last added coordinate, may be updated.
    * @param lastSpeed The speed of the last added coordinate, may be 
    *                  updated.
    * @param totSize The total size of the route data.
    * @param isabRouteVector The vector to add to.
    * @param xDiff The coordinate to add.
    * @param yDiff The coordinate to add, set to MAX_INT16 if no
    *              coordiante to add.
    * @param speed The speed at the coordiante.
    * @param flag The flags at the coordiante, used if adding TPT.
    * @param lastWptOtTpt Updated if adding TPT.
    * @param distLastWptTptToCurr Set to zero if adding TPT.
    * @param nameIndex The last name, used if adding TPT.
    * @param forceClean If cache should be emptied, default false.
    */
   void coordCache( vector< historyPoint >& history,
                    int16& lastXDiff, int16& lastYDiff, uint8& lastSpeed,
                    uint32& totSize, IsabRouteElementVector& isabRouteVector,
                    float32 xDiff, float32 yDiff, uint8 speed,
                    uint8 flag, WPTorTPTElement*& lastWptOtTpt,
                    float64& distLastWptTptToCurr,
                    uint16 nameIndex,
                    bool forceClean = false );


   /**
    * Adds a new Origo and Scale to addElem.
    * 
    * @param radLat The current latitude.
    * @param radLon The current longitude.
    * @param lastRadLat The last latitude.
    * @param lastRadLon The last longitude.
    * @param xDiff Set to the new dist from radLon to new origo in scale.
    * @param yDiff Set to the new dist from radLat to new origo in scale.
    * @param scaleX Set to new longitude scale.
    * @param scaleY Set to new latitude scale, not changed currently.
    * @param origoX Set to new origo at radLon.
    * @param origoY Set to new origo at radLat.
    * @param totSize Size of Origo and Scale added to totSize.
    * @param addElem Origo and Scale added to addElem.
    */
   void addNewOrigoScale( float64 radLat, float64 radLon,
                          float64 lastRadLat, float64 lastRadLon,
                          float32& xDiff, float32& yDiff,
                          float32& scaleX, float32& scaleY,
                          float32& origoX, float32& origoY,
                          uint32& totSize,
                          vector< IsabRouteElement* >& addElem );


   /**
    *  Workaround for old clients that uses an unsafe method to calculate
    *  the angle between any waypoint and the end point.
    *  This method will perturb the coordinate of the last waypoint
    *  (since this one is the most likely to have the same coordinate
    *  as the endpoint) in case it has the same coordinate as the
    *  endpoint.
    */
   void perturbLastWptToAvoidDivideByZero();

   typedef map<uint32, LandmarkPTElement*> activeLMMap;

   /**
    * Add a landmark.
    *
    * @param re The wpt with the landmarks to add.
    */
   void addLandmark( const RouteElement* re, 
                     NavStringTable* stringTable,
                     const map< uint32, MC2String >& landmarkStrings, 
                     uint16& landmarkID,
                     activeLMMap& activeLM, uint32& totSize, 
                     int32& maxSize, int& nbrStringsUsed,
                     vector< IsabRouteElement* >& addElem ) const;

   /**
    * Add lanes and signposts for a wpt.
    *
    * @param re The wpt with the lanes and signposts to add.
    */
   void addLanesAndSignPosts( const RouteElement* re, 
                              NavStringTable* stringTable,
                              uint32& totSize, int32& maxSize, 
                              int& nbrStringsUsed,
                              vector< IsabRouteElement* >& addElem,
                              colorTable_t* colorTable,
                              bool reminder,
                              StringTable::languageCode lang ) const;


   /* The Vector that contains the isabRouteElements */
   IsabRouteElementVector m_isabRouteVector;
   IsabRouteElementVector m_isabAdditionalRouteVector;

   RouteList* m_routeList;

   int m_nbrStringsUsed;


      /// If the route is truncated this one is true
      bool m_truncated;


      /// The distance where the route is truncated.
      int m_truncatedDist;


      /// The WPT where the route is truncated.
      uint32 m_truncatedWPTNbr;


   /**
    * The distance from the trunkation point to the next waypoint.
    */
   uint32 m_dist2nextWPTFromTrunk;
   
   bool m_additionalRouteData;

   // If the route is valid or not.
   bool m_valid;
   
   NavStringTable* m_stringTable;

   // Protocol version to be used in the reply
   int m_protoVer;

   /**
    * The request version.
    */
   uint8 m_reqVer;

   /**
    * The color table, if any.
    */
   colorTable_t* m_colorTable;

   /**
    *   The associated session that holds state during a 
    *   conversation
    */
   isabBoxSession* m_session;
};

#endif
