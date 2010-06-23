/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
#   ifndef DEBUG_LEVEL_1
#      define DEBUG_LEVEL_1
#      define undef_level_1
#   endif
#   ifndef DEBUG_LEVEL_2
#      define DEBUG_LEVEL_2
#      define undef_level_2
#   endif
#   ifndef DEBUG_LEVEL_4
#      define DEBUG_LEVEL_4
#      define undef_level_4
#   endif
#   ifndef DEBUG_LEVEL_8
#      define DEBUG_LEVEL_8
#      define undef_level_8
#   endif
*/

#include "RouteElement.h"
#include "RouteList.h"
#include "isabRouteList.h"
#include "isabRouteElement.h"
#include "isabBoxNavMessage.h"
#include "isabBoxRouteMessage.h"
#include "CoordinateTransformer.h"
#include "GfxUtility.h"
#include "GfxConstants.h"
#include "NavSession.h"
#include "isabBoxSession.h"
#include "RouteRequest.h"
#include "ExpandedRoute.h"
#include "ExpandedRouteItem.h"
#include "ExpandedRouteLandmarkItem.h"
#include "Name.h"
#include "NavStringTable.h"
#include <algorithm>
#include "GDColor.h"


IsabRouteList::IsabRouteList( int protoVer, 
                              RouteList* routeList,
                              NavStringTable* stringTable,
                              NavSession* session,
                              uint8 reqVer,
                              colorTable_t* colorTable )
{
   m_protoVer = protoVer;
   m_routeList = routeList;
   m_stringTable = stringTable;
   m_session = static_cast<isabBoxSession*>( session );
   m_reqVer = reqVer;
   m_colorTable = colorTable;
   m_valid = fillList();
}

IsabRouteList::~IsabRouteList()
{
   uint32 i;

   // Delete everything in the List.
   for(i = 0; i < m_isabRouteVector.size(); ++i ) {
      IsabRouteElement* el = (IsabRouteElement*)m_isabRouteVector[i];
      delete el;
      // Set to NULL so no-one else will delete it.
      m_isabRouteVector[i] = NULL;
   }
   for (i = 0; i < m_isabAdditionalRouteVector.size(); i++) {
      IsabRouteElement* el = (IsabRouteElement*)m_isabAdditionalRouteVector[i];
      delete el;
      m_isabAdditionalRouteVector[i] = NULL;
   }
}

bool
IsabRouteList::fillList()
{
   // Check if we have valid route
   mc2dbg2 << "IsabRouteList::fillList()" << endl;
   mc2dbg2 << "m_routeList->getSize " << m_routeList->getSize() << endl;
      
   if (m_routeList->getSize() < 2) {
      return false;
   }
   
   // For version seven+ use fillList7p
   if ( m_routeList->getReq()->getProtoVer() >= 0x07 ) {
      return fillList7p();
   }

   // This variables will keep track of our current position
   float32 scaleX = 0;   
   // Fix scale, earth is considerd to be round in this direction
   float32 scaleY = GfxConstants::EARTH_RADIUS;
   float32 origoX = 0;
   float32 origoY = 0;
   float32 xDiff = 0, yDiff = 0;
   const int maxDist = 14000;
   bool addTdp = false; // Not supported by client yet 20030130

   // The total length of the sent route
   float64 dist = 0;   
   // The accumulated distance from the last WPT/(full)TPT to the next
   float64 distLastWptTptToCurr = 0;

   // Used to calc total length of sent route
   int32 lastMc2Lat , lastMc2Lon;
   
   // totalSize of the route added
   uint32 totSize = 0;
   // how much data can the Navigator handle.
   int maxRouteSize = m_session->getMaxBufferLength();
   // Calc the max size of route, remeber to fit in one EPT in the end and
   // that 3 points may be added in one loop... scale, orio and tpt/wpt
   uint32 maxSize;
   // Used to update the field "meters" in the last WPT/TPT when the
   // next one is found. Needed since there may be several minipoints 
   // between them.
   WPTorTPTElement* lastWptOtTpt;
   // Left time and dist
   float64 distLeft = 0; // Calculated below
   int lastTimeDistLeftPointIndex = 0;
   uint8 currentSpeed = 0;
   
   // Calculate real dist for route
   lastMc2Lat = m_routeList->routeElementAt( 0 )->getLastLat();
   lastMc2Lon = m_routeList->routeElementAt( 0 )->getLastLon();
   uint32 routeTotalTime = m_routeList->routeElementAt( 0 )->getTime();
   for ( int i = 1 ; i < m_routeList->getSize() ; ++i ) {
      const RouteElement* re = m_routeList->routeElementAt( i );
      float64 radLat, radLon, out3;
      routeTotalTime += re->getTime();
      
      // there are always atleast 2 coordinates i every one
      int j = 0;
      for ( j = 0 ; j < re->getNbrCoord() - 1 ; ++j ) {
         CoordinateTransformer::transformFromMC2(
            re->getLatAt( j ), re->getLonAt( j ),
            CoordinateTransformer::sweref93_LLA,
            radLat, radLon, out3 );

         // calculate distance between two coords and add to total dist
         distLeft += sqrt( GfxUtility::squareP2Pdistance_linear(
                              re->getLatAt( j ), re->getLonAt( j ),
                              lastMc2Lat, lastMc2Lon ) );
         lastMc2Lat = re->getLatAt( j );         
         lastMc2Lon = re->getLonAt( j );
      }
   }
   mc2dbg2 << "Total length of route: " << distLeft << " module says: "
           << m_routeList->getTotalDist() << endl;
   // Send the actual distance
   m_routeList->setTotalDist( uint32( rint( distLeft ) ) );
   mc2dbg2 << "Total time of route: " << routeTotalTime << " module says: "
           << m_routeList->getTotalTime( false ) << " w.stand.t. " 
           << m_routeList->getTotalTime( true ) << endl;
   m_routeList->setTotalTime( routeTotalTime );
   
   if (m_routeList->getReq()->getProtoVer() == 0) {
      maxSize = maxRouteSize -
         ISABBOX_MESSAGE_HEADER_SIZE_VER_0 -
         isabBoxNavMessageUtil::navigatorStructSize*4;
   } else if ( m_routeList->getReq()->getProtoVer() >= 0x05 ) {
      maxSize = maxRouteSize -
         - isabBoxTypeMessage::getHeaderSize( 
            m_routeList->getReq()->getProtoVer() ) -
         isabBoxNavMessageUtil::navigatorStructSize*4*3; // End and trunkated wpt, timedistleft
   } else {
      maxSize = maxRouteSize -
         ISABBOX_MESSAGE_HEADER_SIZE_VER_1 -
         isabBoxNavMessageUtil::navigatorStructSize*4;
   }
   

   // Find the start element to determine if the route starts with a 
   // u-turn or not
   bool start_with_uturn = ( m_routeList->routeElementAt( 0 )->getTurnCode() == 
                             StringTable::DRIVE_START_WITH_UTURN );
   mc2dbg2 << "Adding SPT " << (start_with_uturn?"with":"without") 
           << " uturn" << endl;
   WPTElement* element12 = new WPTElement((int)IsabRouteElement::SPT,
                                          0, // speedlimit
                                          0, // on origoX
                                          0, // on origoY
                                          start_with_uturn ? 0x01 : 0x00, // flags
                                          0, // meters
                                          0, // nameIndex
                                          0, // crossingKind
                                          RouteElement::NO_CROSSING);
   m_isabRouteVector.push_back(element12);
   lastWptOtTpt=NULL;
   totSize += isabBoxNavMessageUtil::navigatorStructSize;


   if ( m_protoVer >= 0x05 && addTdp ) { // Not used in < 0x05
      // Create and add TimeDistLeftPoint to array
      TimeDistLeftElement* tdp = new TimeDistLeftElement( 
         m_routeList->getTotalTime( false ) /* standstill true*/, 
         uint32( rint( distLeft ) ) );
      currentSpeed = m_routeList->routeElementAt( 1 )->getSpeedAt( 0 );
      tdp->setSpeed( currentSpeed );
      m_isabRouteVector.push_back( tdp );
      lastTimeDistLeftPointIndex = m_isabRouteVector.size() - 1;
      totSize += isabBoxNavMessageUtil::navigatorStructSize;
   }
   
   // Add first element that _ONLY CONTAINS ONE POSITION_
   // and a predfined angle to next point on route
   const RouteElement* re = m_routeList->routeElementAt(0);      
   float64 radLat, radLon, lastRadLat = 0, lastRadLon= 0, out3;
   CoordinateTransformer::transformFromMC2(re->getLastLat(),
                                           re->getLastLon(),
                                           CoordinateTransformer::
                                           sweref93_LLA,
                                           radLat,
                                           radLon,
                                           out3);
   // init the last variables   
   lastMc2Lat = re->getLastLat();
   lastMc2Lon = re->getLastLon();
   
   // calc scaleX
   scaleX = cos(radLat)*scaleY;
   origoX = radLon*scaleX;
   origoY = radLat*scaleY;         
   mc2dbg2 << "Adding OrigoX: " << origoX
           << " OrigoY: " << origoY << endl;
   
   //Create and Add OrigoElement to array
   OrigoElement* oe = new OrigoElement((int32)origoX,
                                       (int32)origoY,
                                       0);
   m_isabRouteVector.push_back(oe);
   totSize += isabBoxNavMessageUtil::navigatorStructSize;
   
   mc2dbg2 << "Adding scaleX: " << scaleX << endl;
   
   //Create and Add ScaleElement to array
   ScaleElement* se =
      new ScaleElement((int)scaleX,
                       uint32((scaleX - (int)scaleX)*65536),
                       0,
                       0);
   m_isabRouteVector.push_back(se);
   totSize += isabBoxNavMessageUtil::navigatorStructSize;

   m_truncated = false;
   m_dist2nextWPTFromTrunk = 0;
   int i = 1;
//    byte lastAttribute = 0;


   // Trunkated wpt used if protover >= 0x05
   WPTElement* trunkated_end_element = NULL;
   
   // Start at first coordinate
   int j = 0;
   // Start convering at second re
   convertRouteList( m_routeList, i, j,
                     maxSize, maxDist,
                     m_protoVer,
                     addTdp /* addTDP */, 
                     radLon, scaleX, origoX, 
                     radLat, scaleY, origoY,
                     xDiff, yDiff,
                     lastMc2Lat, lastMc2Lon,
                     lastRadLat, lastRadLon,
                     lastWptOtTpt,
                     distLastWptTptToCurr,
                     m_nbrStringsUsed,
                     m_truncated,
                     m_truncatedWPTNbr,
                     m_dist2nextWPTFromTrunk,
                     trunkated_end_element,
                     totSize, dist, distLeft, 
                     m_session, 
                     m_isabRouteVector, 
                     m_stringTable,
                     lastTimeDistLeftPointIndex, 
                     currentSpeed );

   // Must add EPT      
   // ok, just add EPT with last know xDiff and yDiff
   // (used in navigator as it is today)
   mc2dbg2 << "Adding EPT \tx: " << (int)xDiff << "\ty: "
           << (int)yDiff << "\tTC: "
           << (int)IsabRouteElement::EPT << "\tdist: "
           << 0 << "\tangle: " << 0 << endl;
   WPTElement* element =
      new WPTElement(IsabRouteElement::EPT,
                     0, // speedlimit
                     (int16)xDiff, // from origo
                     (int16)yDiff, // from origo
                     0, // flags
                     0, // meters
                     MAX_UINT16,
                     0,
                     RouteElement::NO_CROSSING);
   
   if (m_protoVer >= 3 && lastWptOtTpt) {
      mc2dbg2 << "Updating last WPT/TPT dist: " <<
                 distLastWptTptToCurr << endl;
      lastWptOtTpt->setDistance((uint16)distLastWptTptToCurr);
   }
   lastWptOtTpt = element;
   m_isabRouteVector.push_back(element);
   totSize += isabBoxNavMessageUtil::navigatorStructSize;

   // Loop and set nextOrigo in the origoElements
   int k = m_isabRouteVector.size();
   int16 nextOrigo = 0;

   mc2dbg2 << " TOTAL SENT DIST : " << dist << endl;
   m_truncatedDist = (int)dist;

   // TDP time counting 
   vector< TimeDistLeftElement* > tdpFromLastWpt;
   uint32 timeLeft = 0;
   // Get time for rest of route
   for ( int32 l = i ; l < m_routeList->getSize() ; ++l ) {
      timeLeft += m_routeList->routeElementAt( l )->getTime();
   }
   mc2dbg2 << "Time left after added route: " << timeLeft << endl;
   mc2dbg2 << "dist2nextWPTFromTrunk " << m_dist2nextWPTFromTrunk << endl;

   while (--k >= 0) {
      IsabRouteElement* ire =
         static_cast<IsabRouteElement*>(m_isabRouteVector[k]);
      
      if (dynamic_cast<OrigoElement*>(ire) != NULL) {
         OrigoElement* oe = dynamic_cast<OrigoElement*>(ire);
         oe->setNextOrigo(nextOrigo-k);
         nextOrigo = k;
      } else if ( dynamic_cast<TimeDistLeftElement*>( ire ) != NULL ) {
         tdpFromLastWpt.push_back( static_cast< TimeDistLeftElement* > ( 
                                      ire ) );
      } else if ( dynamic_cast<WPTElement*>( ire ) != NULL && addTdp ) {
         // Calculate times for tdps from last wpt
         if ( tdpFromLastWpt.size() > 1 ) { // Check for last (first time)
            // Calculate total dist / speed sum
            // Always TDP after WPT except last
            uint32 totalDist = 0;
            float64 totalFactor = 0;
            vector< TimeDistLeftElement* >::iterator it = 
               tdpFromLastWpt.begin();
            it++;
            for (  ; it != tdpFromLastWpt.end() ; ++it ) {
               totalFactor += ( float64( (*it)->getDistLeft() - 
                                         (*(it-1))->getDistLeft() ) / 
                                (*it)->getSpeed() );
            }
            totalDist = tdpFromLastWpt.back()->getDistLeft() - 
               (*tdpFromLastWpt.begin())->getDistLeft();
            
            // Total time to next WPT is stored in checkTimeDistLeft
            uint32 totalTime = tdpFromLastWpt.back()->getTimeLeft(); 

            it = tdpFromLastWpt.begin();
            it++;
            for ( ; 
                  it != tdpFromLastWpt.end() ; 
                  it++ )
            {
               dist = (*it)->getDistLeft() - (*(it-1))->getDistLeft();
               uint32 tdpTime = uint32( rint( float64(
                  dist ) / (*it)->getSpeed() / 
                     totalFactor *  totalTime ) );
               timeLeft += tdpTime;
               (*it)->setTimeLeft( timeLeft );
               mc2dbg4 << "Time TDP: " << tdpTime << " ETA " << timeLeft
                       << " dist " << dist << " speed " 
                       << int((*it)->getSpeed()) << endl;
            }


            // Keep last
            TimeDistLeftElement* last = tdpFromLastWpt.back(); 
            tdpFromLastWpt.clear();
            tdpFromLastWpt.push_back( last );
         } // End if tdpFromLastWpt not empty
         else { // First time (last TDP)
            if ( tdpFromLastWpt.size() == 1 ) {
               mc2dbg4 << "Last TDP (first in this loop)" << endl;
               mc2dbg4 << "WPT dist " 
                       << static_cast<WPTElement*>( ire )->getDistance() 
                       << " SpeedLimit " << int( static_cast<WPTElement*>(
                          ire )->getSpeedLimit() )
                       << endl;
               mc2dbg4 << "timeleft " 
                       << tdpFromLastWpt.back()->getTimeLeft()
                       << " tdp distleft " 
                       << tdpFromLastWpt.back()->getDistLeft() << endl;
               // Add to ETA
               timeLeft += tdpFromLastWpt.back()->getTimeLeft();
               
            } else {
               mc2dbg4 << "No tdp yet" << endl;
            }
         }
         
      } // End if is WPT and addTdp
   }

   // Add next wpt data if trunkated route
   if ( m_protoVer >= 0x05 && trunkated_end_element != NULL ) {
      mc2dbg2 << info << "Adding trunkated_end_element." << endl;
      m_isabRouteVector.push_back( trunkated_end_element );
   }

   m_additionalRouteData = false;

   // ---------------------------
   // Check if we should send additional route data.
   // ---------------------------
   uint8 downloadAdditionalRouteData =
         m_session->getDownloadAdditionalRouteData();
   if (downloadAdditionalRouteData) {

      // We'll need to send additional data on the full route
      // as binary data.
      mc2dbg2 << "Will create rest of route as binary data." << endl;
      mc2dbg2 << "data == " << int (downloadAdditionalRouteData) << endl;
      // Mark addtional data as valid.
      m_additionalRouteData = true;

      int dummyNbrStringsUsed = 0;
      convertRouteList( m_routeList, i, j,
                        MAX_UINT32/* No limit */, maxDist,
                        m_protoVer,
                        false /* addTDP */, 
                        radLon, scaleX, origoX, 
                        radLat, scaleY, origoY, 
                        xDiff, yDiff,
                        lastMc2Lat, lastMc2Lon,
                        lastRadLat, lastRadLon,
                        lastWptOtTpt,
                        distLastWptTptToCurr,
                        dummyNbrStringsUsed,
                        m_truncated,
                        m_truncatedWPTNbr,
                        m_dist2nextWPTFromTrunk,
                        trunkated_end_element,
                        totSize, dist, distLeft, 
                        m_session, 
                        m_isabAdditionalRouteVector, 
                        m_stringTable,
                        lastTimeDistLeftPointIndex, 
                        currentSpeed );

   } else {
      mc2dbg2 << "No binary data." << endl;
   }

   // Set truncated in RouteList
   m_routeList->setTruncated( m_truncated );
   m_routeList->setTruncatedDist( m_truncatedDist );
   m_routeList->setTruncatedWPTNbr( m_truncatedWPTNbr );
      
   return true;
}

int
IsabRouteList::getSize() const
{
   return m_isabRouteVector.size();
}

int
IsabRouteList::getSizeAdditional() const
{
   return m_isabAdditionalRouteVector.size();
}

bool
IsabRouteList::isTruncated() const
{
   return m_truncated;
}

bool
IsabRouteList::hasAdditionalRouteData() const
{
   return m_additionalRouteData;
}

int IsabRouteList::getNbrStringsUsed() const
{
   return m_nbrStringsUsed;
}


int IsabRouteList::getTruncatedDist() const
{
   return m_truncatedDist;
}

const IsabRouteElement*
IsabRouteList::isabRouteElementAt(int idx) const
{
   return (IsabRouteElement*)m_isabRouteVector[idx];
}

const IsabRouteElement*
IsabRouteList::isabAdditionalRouteElementAt(int idx) const
{
   return (IsabRouteElement*)m_isabAdditionalRouteVector[idx];
}


uint32 
IsabRouteList::getDist2nextWPTFromTrunk() const {
   return m_dist2nextWPTFromTrunk;
}


void 
IsabRouteList::checkTimeDistLeft( float64& distLeft, 
                                  IsabRouteElementVector& isabRouteVector, 
                                  int& i, int& j,
                                  const RouteElement* re, 
                                  const RouteElement* preRe, 
                                  int& lastTimeDistLeftPointIndex,
                                  uint32& totSize, uint8& currentSpeed )
{
   bool add = false;

   if ( currentSpeed != re->getSpeedAt( j ) ||
        dynamic_cast<WPTElement*>( isabRouteVector.back() ) != NULL ||
        lastTimeDistLeftPointIndex + 40 < int32(isabRouteVector.size()) )
   {
      add = true;
   }

   if ( add ) {
      mc2dbg2 << " Adding TDP: "
             << "\tat " << i << "\tcoord " << j << "\tdist " << distLeft
             << "\ttime " << re->getTime() << "\tspeed " 
             << int(re->getSpeedAt( j )) << endl;
      // // Get time that it is from last TimeDistLeftPoint
      // // Dist is substracted for each coordinate in loop in fillList
      // // Never more than a WPT (an ExpandStringItem/RouteElement) away

//      TimeDistLeftElement* lasttdp = static_cast<TimeDistLeftElement*> (
//         isabRouteVector.getElementAt( lastTimeDistLeftPointIndex ) );
      
      // timeLeft is calculated and set after main-creating loop in 
      // fillList
      TimeDistLeftElement* tdp = new TimeDistLeftElement( 
         re->getTime(), uint32( rint( distLeft ) ) );
      tdp->setSpeed( re->getSpeedAt( j ) );
      isabRouteVector.push_back( tdp );
      lastTimeDistLeftPointIndex = isabRouteVector.size() - 1;
      currentSpeed = re->getSpeedAt( j );
      totSize += isabBoxNavMessageUtil::navigatorStructSize;
   }

}


void 
IsabRouteList::convertRouteList( RouteList* routeList, int& i, int& j,
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
                                 uint8& currentSpeed )
{
   bool packetFull = false;
   // where are we in the NavStringTable, nbrStringsUsed set at end of loop
   int currTextPos = -1;
   // The distance between the last and the current coordinate
   float64 distLastToCurr;
   // Dummy Y-axis
   float64 out3;
   // TP-flag 
   uint8 flag = 0;
   // Used to pack TPT in miniPoints
   int16 history[2];
   uint8 speedHistory = 0;
   uint8 nbrHistory = 0;

   while ( i < routeList->getSize() && !packetFull ) {
      // Take all elements in RouteList _EXCEPT_ the first
      // that is already handled above
      const RouteElement* re = routeList->routeElementAt( i );
      const RouteElement* preRe = routeList->routeElementAt( i-1 );      
      
      // there are always atleast 2 coordinates i every one
      int j = 0;
      while (j < re->getNbrCoord() - 1 && !packetFull) {
         
         // transform into radians
//         float64 radLat, radLon, out3;
         CoordinateTransformer::transformFromMC2(
               re->getLatAt(j),
               re->getLonAt(j),
               CoordinateTransformer::sweref93_LLA,
               radLat,
               radLon,
               out3);

         
         // calculate distance between two coords and add to total dist
         distLastToCurr =
               sqrt(GfxUtility::squareP2Pdistance_linear(re->getLatAt(j),
                                                         re->getLonAt(j),
                                                         lastMc2Lat,
                                                         lastMc2Lon));
         dist += distLastToCurr;
         distLastWptTptToCurr += distLastToCurr;
         distLeft -= distLastToCurr; // Total dist left

         mc2dbg2 << "distLastToCurr: " << distLastToCurr << endl;

         // calc diff from current origo
         xDiff = radLon*scaleX - origoX;
         yDiff = radLat*scaleY - origoY;

         if (fabs(xDiff) < maxDist && fabs(yDiff) < maxDist) {
            // Ok _NO_ new origo and scale, add WPT or TPT
            if (j == 0) {
               // This is a WPT add...                        
               // Flags only reflect the first item in a group. FIXME.
               flag=isabBoxNavMessageUtil::attributeToFlags(re->getAttributeAt(j));
               mc2dbg2 << "Adding WPT \tx: " << (int)xDiff << "\ty: "
                       << (int)yDiff << "\tTC: "
                       <<  StringTable::getString(re->getTurnCode(),
                                                  StringTable::SWEDISH)
                       << "\tflag: " << (int)flag 
                       << "\ttextC: " << preRe->getText()
                       << "\texitCount: " << (int16)re->getExitCount()
                       << "\tspeed: " << (int16)re->getSpeedAt(j) 
                       << "\tCrossingKind: " 
                       << (int16)re->getCrossingKind() << endl;
               WPTElement* element =
                  new WPTElement(
                     isabBoxNavMessageUtil::turnCodeToIsabBox( 
                        re, protoVer ),
                     re->getSpeedAt(j), // speedlimit
                     (int16)xDiff, // from origo
                     (int16)yDiff, // from origo
                     flag, // flag
                     re->getDist(), // Distance is overwritten by the next WPT/TPT 
                                    // when protoVer>=3
                     preRe->getText(),
                     re->getExitCount(),
                     re->getCrossingKind());               
               if ( protoVer >= 3 && lastWptOtTpt ) {
                  mc2dbg2 << "Updating last WPT/TPT dist: " <<
                             distLastWptTptToCurr << endl;
                  lastWptOtTpt->setDistance((uint16)distLastWptTptToCurr);
               }
               lastWptOtTpt=element;
               distLastWptTptToCurr=0;
               isabRouteVector.push_back(element);
               totSize += isabBoxNavMessageUtil::navigatorStructSize;
               // Needs to know when the attributes changes
//                lastAttribute = re->getAttributeAt(j);
               
               if (preRe->getText() > currTextPos) {
                  currTextPos = preRe->getText();
                  // add the strlen to total 
                  totSize += strlen(stringTable->getStringAt(preRe->getText()) ) + 1;
               }
               if ( addTDP && protoVer >= 0x05 )
                  checkTimeDistLeft( 
                     distLeft, isabRouteVector, i, j, re, preRe, 
                     lastTimeDistLeftPointIndex, totSize, currentSpeed );
            } else {

               if (j+1 >= re->getNbrCoord() - 1 &&
                   nbrHistory == 0) {
                  // next will be a WPT, add what we have
                  // Add TPT
                  flag=isabBoxNavMessageUtil::attributeToFlags(re->getAttributeAt(j));
                  mc2dbg2 << "Adding TPT \tx: " 
                          << (int)xDiff << "\ty: "
                          << (int)yDiff << "\tflag: " << (int)flag
                          << "\tspeed: " << (int16)re->getSpeedAt(j) 
                          << endl;
                  TPTElement* tpt = new TPTElement(
                        (int16)xDiff,
                        (int16)yDiff,
                        flag,
                        0, // Distance is filled in by the next WPT/TPT when protoVer>=3
                        re->getSpeedAt(j));
                  if ( protoVer >= 3 && lastWptOtTpt ) {
                     mc2dbg2 << "Updating last WPT/TPT dist: " <<
                                distLastWptTptToCurr << endl;
                     lastWptOtTpt->setDistance((uint16)distLastWptTptToCurr);
                  }
                  lastWptOtTpt=tpt;
                  distLastWptTptToCurr=0;
                  isabRouteVector.push_back( tpt );
                  totSize += isabBoxNavMessageUtil::navigatorStructSize;
                  if ( addTDP && protoVer >= 0x05 )
                     checkTimeDistLeft( 
                        distLeft, isabRouteVector, i, j, re, preRe, 
                        lastTimeDistLeftPointIndex,totSize, currentSpeed );
               } else if (nbrHistory == 0) {
                  // no history, save current
                  history[0] = (int)xDiff;
                  history[1] = (int)yDiff;
                  speedHistory = re->getSpeedAt(j);
                  nbrHistory = 1;
               } else if (nbrHistory == 1) {
                  mc2dbg2 << "Adding MINI \tx1: " 
                          << (int)history[0] << "\ty1: "
                          << (int)history[1]
                          << "\tspeed1: " << (int) speedHistory 
                          << endl 
                          << "\t\tx2: " << (int)xDiff
                          << "\tx2: " << (int)yDiff 
                          << "\tspeed2: " << (int16)re->getSpeedAt(j) 
                          << endl;
                  MiniElement* mp
                     = new MiniElement((int16)history[0],
                                       (int16)history[1],
                                       (int16)xDiff,
                                       (int16)yDiff,
                                       speedHistory,
                                       re->getSpeedAt(j));
                  isabRouteVector.push_back( mp );
                  totSize += isabBoxNavMessageUtil::navigatorStructSize;
                  nbrHistory = 0;
                  if ( addTDP && protoVer >= 0x05 )
                     checkTimeDistLeft( 
                        distLeft, isabRouteVector, i, j, re, 
                        preRe, lastTimeDistLeftPointIndex, totSize, 
                        currentSpeed );
               }
            }
         } else { // Else need new origo and scale (minimap)

            if (nbrHistory == 1) {
               // Write history before new minimap
               flag=isabBoxNavMessageUtil::attributeToFlags(re->getAttributeAt(j));
               mc2dbg2 << "Adding TPT \tx: " 
                       << (int)history[0] << "\ty: "
                       << (int)history[1] << "\tflag: " << (int)flag
                       << "\tspeed: " << (int16)speedHistory 
                       << endl;
               TPTElement* tpt = new TPTElement(
                     (int16)history[0], 
                     (int16)history[1], 
                     flag,
                     0, // Distance is filled in by the next WPT/TPT when protoVer>=3
                     speedHistory);
               if ( protoVer >= 3 && lastWptOtTpt ) {
                  mc2dbg2 << "Updating last WPT/TPT dist: " <<
                             distLastWptTptToCurr << endl;
                  lastWptOtTpt->setDistance((uint16)distLastWptTptToCurr);
               }
               lastWptOtTpt=tpt;
               distLastWptTptToCurr=0;
               isabRouteVector.push_back( tpt );
               totSize += isabBoxNavMessageUtil::navigatorStructSize;
               nbrHistory = 0;
               if ( addTDP && protoVer >= 0x05 )
                  checkTimeDistLeft( 
                     distLeft, isabRouteVector, i, j, re, 
                     preRe, lastTimeDistLeftPointIndex, totSize, 
                     currentSpeed );
            }
            // Needs new scale and origo
            scaleX = cos(radLat)*scaleY;                        
            origoX = radLon*scaleX;
            origoY = radLat*scaleY;

            mc2dbg2 << "Adding OrigoX: " << origoX
                    << " OrigoY: " << origoY << endl;
            //Create and Add OrigoElement to array
            OrigoElement* oe = new OrigoElement((int32)origoX,
                                                (int32)origoY,
                                                0);
            isabRouteVector.push_back( oe );
            totSize += isabBoxNavMessageUtil::navigatorStructSize;
            
            mc2dbg2 << "Adding scaleX: " << scaleX << " refX: "
                    << (int)(lastRadLon*scaleX - origoX) <<
                       " refy: " << (int)(lastRadLat*scaleY - origoY)  
                    << endl;
            
            ScaleElement* se =
               new ScaleElement((int)scaleX,
                                uint32((scaleX - (int)scaleX)*65536),
                                uint16(lastRadLon*scaleX - origoX),
                                uint16(lastRadLat*scaleY - origoY));
            isabRouteVector.push_back( se );
            totSize += isabBoxNavMessageUtil::navigatorStructSize;
            
            if (j == 0) {
               // This is a WPT in new origo
               flag=isabBoxNavMessageUtil::attributeToFlags(re->getAttributeAt(j));
               mc2dbg2 << "Adding WPT\tx: "<< 0 << "\ty: " << 0 << "\tTC: "
                       << StringTable::getString(re->getTurnCode(),
                                                 StringTable::SWEDISH)
                       << "\tflag: " << (int)flag
                       << "\ttextC: " << preRe->getText()
                       << "\texitCount: " << (int16)re->getExitCount() 
                       << "\tspeed: " << (int16)re->getSpeedAt(j)
                       << "\tCrossingKind: " 
                       << (int16)re->getCrossingKind() << endl;
               //Creata and Add WPTElement            
               WPTElement* element =
                  new WPTElement(
                     isabBoxNavMessageUtil::turnCodeToIsabBox(
                        re, protoVer ),
                     re->getSpeedAt(j), // speedlimit
                     0, // on origoX
                     0, // on origoY
                     flag, // flag
                     re->getDist(), // Distance is overwritten by the next WPT/TPT 
                                    // when protoVer>=3
                     preRe->getText(),
                     re->getExitCount(),
                     re->getCrossingKind());
               if ( protoVer >= 3 && lastWptOtTpt ) {
                  mc2dbg2 << "Updating last WPT/TPT dist: " <<
                             distLastWptTptToCurr << endl;
                  lastWptOtTpt->setDistance((uint16)distLastWptTptToCurr);
               }
               lastWptOtTpt=element;
               distLastWptTptToCurr=0;
               isabRouteVector.push_back(element);
               totSize += isabBoxNavMessageUtil::navigatorStructSize;
               // Needs to know when the attributes changes
//                lastAttribute = re->getAttributeAt(j);
               
               if (preRe->getText() > currTextPos) {
                  currTextPos = preRe->getText();
                  // add the strlen to total 
                  totSize += strlen( stringTable->getStringAt(
                                        preRe->getText()) ) + 1;
                  if ( addTDP && protoVer >= 0x05 )
                     checkTimeDistLeft( distLeft, 
                                        isabRouteVector, i, j, re, preRe,
                                        lastTimeDistLeftPointIndex,
                                        totSize, currentSpeed );
               }
            } else {
               if (j+1 >= re->getNbrCoord() - 1 &&
                   nbrHistory == 0) {
                  // next will be a WPT, add what we have
                  // Add TPT
                  flag=isabBoxNavMessageUtil::attributeToFlags(re->getAttributeAt(j));
                  mc2dbg2 << "Adding TPT \tx: " 
                          << (int)xDiff << "\ty: "
                          << (int)yDiff << "\tflag: " << (int)flag
                          << "\tspeed: " << (int16)re->getSpeedAt(j) 
                          << endl;
                  TPTElement* tpt = new TPTElement(
                        (int16)xDiff, 
                        (int16)yDiff, 
                        flag,
                        0, // Distance is filled in by the next WPT/TPT when protoVer>=3
                        re->getSpeedAt(j));
                  if ( protoVer >= 3 && lastWptOtTpt ) {
                     mc2dbg2 << "Updating last WPT/TPT dist: " <<
                                distLastWptTptToCurr << endl;
                     lastWptOtTpt->setDistance((uint16)distLastWptTptToCurr);
                  }
                  lastWptOtTpt=tpt;
                  distLastWptTptToCurr=0;
                  isabRouteVector.push_back( tpt );
                  totSize += isabBoxNavMessageUtil::navigatorStructSize;
                  if ( addTDP && protoVer >= 0x05 )
                     checkTimeDistLeft( 
                        distLeft, isabRouteVector, i, j, re, 
                        preRe, lastTimeDistLeftPointIndex, totSize, 
                        currentSpeed );
               } else if (nbrHistory == 0) {
                  // no history, save current
                  history[0] = 0;
                  history[1] = 0;
                  speedHistory = re->getSpeedAt(j);
                  nbrHistory = 1;
               } else if (nbrHistory == 1) {
                  mc2log << error << here << "ERROROORRE !!\n"
                            " history not empty after minimap!" << endl;
               }                
            }
         } // End else need new origo and scale (minimap)
         // Save these so that we can fill in correct informaton in scaleElement
         lastRadLat = radLat;
         lastRadLon = radLon;
         lastMc2Lat = re->getLatAt(j);         
         lastMc2Lon = re->getLonAt(j);

         // Calculate trunkated end string size here and use in check
         int trunkated_end_string_size = 0;
         if ( protoVer >= 0x05 ) { // Not used in < 0x05
            // Find next string that will be used and check stringindex
            if ( re->getText() > currTextPos) {
               trunkated_end_string_size = strlen( 
                  stringTable->getStringAt( re->getText() ) ) + 1;
            }
         }

         if ( totSize + trunkated_end_string_size > maxSize ) {
            packetFull = true;                  
            mc2dbg2 << "TRUNCATING! totsize:" <<  totSize
                    << " currTextPos: " << currTextPos <<endl;
            truncated = true;
            truncatedWPTNbr = i;

            // Check if add next waypoint
            if ( protoVer >= 0x05 && (i + 1) < routeList->getSize() ) {
               // Find next waypoint
               const RouteElement* next = 
                  routeList->routeElementAt( i + 1 );

               // Get distance to next waypoint
               // Loop all remaining coordinates
               int32 currMC2Lat = lastMc2Lat;
               int32 currMC2Lon = lastMc2Lon;
               float64 dist2nextWPTFromTrunkFloat = 0;
               for ( int jSafe = j; jSafe < re->getNbrCoord() -1 ;jSafe++ )
               {
                  dist2nextWPTFromTrunkFloat += sqrt( 
                     GfxUtility::squareP2Pdistance_linear(
                        re->getLatAt( jSafe ), re->getLonAt( jSafe ),
                        currMC2Lat, currMC2Lon ) );
                  currMC2Lat = re->getLatAt( jSafe );
                  currMC2Lon = re->getLonAt( jSafe );
               }

               // Set dist2nextWPTFromTrunk
               dist2nextWPTFromTrunk = 
                  uint32( rint( dist2nextWPTFromTrunkFloat ) );

               // Next way point
               trunkated_end_element =
                  new WPTElement(
                     isabBoxNavMessageUtil::turnCodeToIsabBox(
                        next, protoVer ),
                     next->getSpeedAt( 0 ), // speedlimit
                     0, // Not used, not possible, may need new origo.
                     0, // Not used, not possible, may need new origo.
                     isabBoxNavMessageUtil::attributeToFlags( 
                        next->getAttributeAt( 0 ) ), // flag
                     0, // Not used, not possible, may need new origo.
                     re->getText(),
                     next->getExitCount(),
                     next->getCrossingKind() ); 

               // We are allready at packetfull, but for right size
               totSize += isabBoxNavMessageUtil::navigatorStructSize;
               totSize += trunkated_end_string_size;
               
               if ( re->getText() > currTextPos ) {
                  currTextPos = re->getText();
               }

            } // End If add trunkated_end wpt element

         } // End if total used size > max size


         nbrStringsUsed = currTextPos;
         j++;
      } // End while j < re->getNbrCoord() && !packetFull
      
      i++;      
   } // End while i < routeList.size() && !packetFull

      
}


uint8 mc2Side2Nav( SearchTypes::side_t side ) {
   switch ( side ) {
      case SearchTypes::left_side :
         return 0;
      case SearchTypes::right_side :
         return 1;
      default:
         return 2;
   }
}


uint8 mc2Landmark_t2Nav( ItemTypes::landmark_t type ) {
   switch( type ) {
      case ItemTypes::builtUpAreaLM :
         return 0;
      case ItemTypes::railwayLM :
         return 1;
      case ItemTypes::areaLM :
         return 2;
      case ItemTypes::poiLM :
         return 3;
      case ItemTypes::signPostLM :
         return 4;
      case ItemTypes::countryLM :
         return 5;
      case ItemTypes::countryAndBuiltUpAreaLM :
         return 6;
      case ItemTypes::passedStreetLM :
         return 7;
      case ItemTypes::accidentLM :
         return 8;
      case ItemTypes::roadWorkLM :
         return 9;
      case ItemTypes::cameraLM :
         return 10;
      case ItemTypes::speedTrapLM :
         return 11;
      case ItemTypes::policeLM :
         return 12;
      case ItemTypes::weatherLM :
         return 13;
      case ItemTypes::trafficOtherLM :
         return 14;
      case ItemTypes::userDefinedCameraLM :
         return 15;

      default :
         return 0;
   }
}


uint8 mc2Landmark_location_t2Nav( ItemTypes::landmarklocation_t location )
{
   switch( location ) {
      case ItemTypes::after :
         return 0;
      case ItemTypes::before :
         return 1;
      case ItemTypes::in :
         return 2;
      case ItemTypes::at :
         return 3;
      case ItemTypes::pass :
         return 4;
      case ItemTypes::into :
         return 5;
      case ItemTypes::arrive :
         return 6;
      default :
         return 0;
   }
}

class landmarkSortLess {
public:
   bool operator()( const ExpandedRouteLandmarkItem* a, 
                    const ExpandedRouteLandmarkItem* b ) const {
         return a->getLandmarkID() < b->getLandmarkID();
   }
};

class landmarkSortLessAndStart {
public:
   bool operator()( const ExpandedRouteLandmarkItem* a, 
                    const ExpandedRouteLandmarkItem* b ) const {
      if ( a->getLandmarkID() != b->getLandmarkID() ) {
         return a->getLandmarkID() < b->getLandmarkID();
      } else {
         return a->isStart() && !b->isStart();
      }
   }
};


typedef multiset< const ExpandedRouteLandmarkItem*, 
                  landmarkSortLess > lmset;

typedef multiset< const ExpandedRouteLandmarkItem*, 
                  landmarkSortLessAndStart > lmsetOrdered;

bool 
IsabRouteList::fillList7p() {
   if ( m_routeList->getSize() < 2 ) {
      return false;
   }

   // These variables will keep track of our current position
   float32 scaleX = 0;   
   // Fix scale, earth is considerd to be round in this direction
   float32 scaleY = GfxConstants::EARTH_RADIUS;
   float32 origoX = 0;
   float32 origoY = 0;
   float32 xDiff = 0, yDiff = 0;
   // Max distance between two Origo,Scale points (2^15 and some buffer)
   const int maxDist = 14000;
   // The total length of the sent route
   float64 dist = 0;   
   // The accumulated distance from the last WPT/(full)TPT to the next
   float64 distLastWptTptToCurr = 0;

   // Used to calc total length of sent route
   int32 lastMc2Lat = 0;
   int32 lastMc2Lon = 0;

   // totalSize of the route added
   uint32 totSize = 0;
   // how much data can the Navigator handle.
   // It can handle 1500 route datums and unlimited stringtable.
   int maxRouteSize = m_session->getMaxBufferLength();
   mc2dbg2 << "IsabRouteList::fillList7p  maxBuffer size " 
           << maxRouteSize << endl;
   // If to add coordiantes
   bool addCoordinates = ( m_routeList->getReq()->getRouteType() == 
                           RouteReq::full );
   // How much route, in seconds, to add before trunkating
   uint32 timeBeforeTrunk = m_routeList->getReq()->getTimeBeforeTrunk();
   
   // Calc the max size of route for ordinary part
   int32 maxSize;
   // Used to update the field "meters" in the last WPT/TPT when the
   // next one is found. Needed since there may be several minipoints 
   // between them.
   WPTorTPTElement* lastWptOtTpt;
   // Left time and dist
   float64 distLeft = 0; // Calculated below
   uint32 currentTime = 0;
   uint8 currentSpeed = 0;
   // Coords vars
   float64 radLat = 0.0;
   float64 radLon = 0.0;
   float64 lastRadLat = 0.0;
   float64 lastRadLon= 0.0;
   float64 out3 = 0.0;
   // Route different because of disturbance
   bool differentDisturbanceRoute = false;
   const char* differentDisturbanceRouteStr = "";
   int differentDisturbanceRouteStrIndex = 0;
   StringTable::languageCode lang = 
      m_routeList->getRouteRequest()->getLanguage();
//   LangTypes::language_t language = 
//      ItemTypes::getLanguageCodeAsLanguageType( lang );
   // To handle nameChange trackpoints.
   bool nameChangeTrackpoint = false;
   WPTElement* lastWP = NULL;
   // Last added coord Used for Micro points
   int16 lastAddedXDiff = 0;
   int16 lastAddedYDiff = 0;
   uint8 lastAddedSpeed = 0;
   // The last coordinate used to check if coordinates are same
   float32 lastXDiff = 0;
   float32 lastYDiff = 0;

   // landmark fields
   uint16 landmarkID = 0;
   activeLMMap activeLM;
   map<uint32, LandmarkPTElement*>::iterator lmi;

   if ( m_routeList->getRouteRequest() != NULL &&
        m_routeList->getReq()->getRouteCost() == 
        RouteTypes::TIME_WITH_DISTURBANCES &&
        m_routeList->getRouteRequest()->compareRoutes() ) 
   {
      differentDisturbanceRoute = true;
      // "There is a disturbance in the force I haven't felt since...";
      differentDisturbanceRouteStr = StringTable::getString(
         StringTable::DISTURED_ROUTE, lang );
      // Add differentDisturbanceRouteStr to string table 
      differentDisturbanceRouteStrIndex = 
         m_stringTable->addString( differentDisturbanceRouteStr );
   }

   // Calculate real dist for route
   lastMc2Lat = m_routeList->routeElementAt( 0 )->getLastLat();
   lastMc2Lon = m_routeList->routeElementAt( 0 )->getLastLon();
   uint32 routeTotalTime = m_routeList->routeElementAt( 0 )->getTime();
   uint32 allStringsSize = 0;
   uint32 nbrLandmarks = 0;
   map< uint32, MC2String > landmarkStrings;
   m_nbrStringsUsed = 
      m_routeList->getStringTable().getSize()-1; // All strings (index)
   vector< bool > needNewOrigoScale;
   uint32 nbrLandmarkRemindersAtScale = 0;
   needNewOrigoScale.push_back( false ); // STP is follwed by Origo,Scale
   uint32 nbrNameChangeWPTs = 0;

   for ( int32 i = 0 ; i < m_routeList->getStringTable().getSize() ; ++i )
   {
      // Add to string size. Resuse of identical strings, like "E4" 
      allStringsSize += strlen( m_stringTable->getStringAt( i ) ) + 1;
   }

   // Init Origo, Scale
   {
      const RouteElement* re = m_routeList->routeElementAt( 0 );      
      CoordinateTransformer::transformFromMC2( re->getLastLat(),
                                               re->getLastLon(),
                                               CoordinateTransformer::
                                               sweref93_LLA,
                                               radLat,
                                               radLon,
                                               out3 );

      // calc scaleX
      scaleX = cos( radLat ) * scaleY;
      origoX = radLon*scaleX;
      origoY = radLat*scaleY;         
   }
   
   for ( int i = 1 ; i < m_routeList->getSize() ; ++i ) {
      const RouteElement* re = m_routeList->routeElementAt( i );
      routeTotalTime += re->getTime();

      if ( nameChangeTrackpoint ) {
         nbrNameChangeWPTs++;
      }

      if ( re->getTurnCode() == StringTable::LMLOCATION_ROADNAMECHANGE ) {
         nameChangeTrackpoint = true;
      } else {
         nameChangeTrackpoint = false;
      }

      bool addedScale = false;
      
      // There are always at least 2 coordinates i every one
      int j = 0;
      for ( j = 0 ; j < re->getNbrCoord() - 1 ; ++j ) {
         // transform into radians
         CoordinateTransformer::transformFromMC2(
            re->getLatAt( j ), re->getLonAt( j ),
            CoordinateTransformer::sweref93_LLA,
            radLat, radLon, out3 );
         // calculate distance between two coords and add to total dist
         distLeft += sqrt( GfxUtility::squareP2Pdistance_linear(
                              re->getLatAt( j ), re->getLonAt( j ),
                              lastMc2Lat, lastMc2Lon ) );
         // calc diff from current origo
         xDiff = radLon*scaleX - origoX;
         yDiff = radLat*scaleY - origoY;

         // If needs new origo, scale
         if ( addCoordinates && 
              (fabs( xDiff ) >= maxDist || fabs( yDiff ) >= maxDist) ) 
         {
            scaleX = cos(radLat)*scaleY;                        
            origoX = radLon*scaleX;
            origoY = radLat*scaleY;
            nbrLandmarkRemindersAtScale += activeLM.size();

            // Recalculate xDiff and YDiff using new origo
            xDiff = radLon*scaleX - origoX;
            yDiff = radLat*scaleY - origoY;
            addedScale = true;
         }


         lastMc2Lat = re->getLatAt( j );         
         lastMc2Lon = re->getLonAt( j );
      } // End for all coords

      // If scale,origos in this re
      needNewOrigoScale.push_back( addedScale );

      // Add landmark strings, if any
      char description[4096];
      nbrLandmarks += re->getNbrLandmarks();
      // Active lms
      lmsetOrdered lands;
      for ( uint32 l = 0 ; l < re->getNbrLandmarks() ; ++l ) {
         lands.insert( re->getLandmark( l ) );
      }
      for ( lmsetOrdered::const_iterator it = lands.begin() ; 
            it != lands.end() ; ++it ) {
         const ExpandedRouteLandmarkItem* land = *it;
         bool ok = ExpandedRoute::makeRouteLandmarkDescription( 
            description, 4096, land, 
            ExpandedRouteItem::routeTurnFromStringCode( re->getTurnCode()),
            lang, StringTableUtility::NORMAL, StringTableUtility::METERS,
            re->getTransportationType(),
            "/", false/*showDist*/, false/*wapFormat*/ );
         
         if ( !ok ) {
            mc2log << error << "IsabRouteList::fillList7p "
                   << "landmarks text failed at WPT " << i << " lm ";
            land->dump( mc2log );
            mc2log << endl;
            strcpy( description, "" );
         }

         landmarkStrings.insert( make_pair( land->getLandmarkID(), 
                                            MC2String( description ) ) );
         allStringsSize += strlen( description ) + 1;
         lmi = activeLM.find(land->getLandmarkID());
         if ( (land->isStart() && !land->isStop()) &&
              (lmi == activeLM.end()) ) {
            // Save this landmark.
            activeLM.insert( make_pair( land->getLandmarkID(),
                                        (LandmarkPTElement*)NULL/*lme*/ ) );
         } else if ( land->isStop() && lmi != activeLM.end() ) {
            // Landmark passed, remove it
            activeLM.erase( lmi );
         }
//          mc2dbg2 << "Landm: name " << *land->getName() << " id " 
//                 << land->getLandmarkID() << " start " << land->isStart()
//                 << " stop " << land->isStop() 
//                 << "\tTC: "
//                 << StringTable::getString( re->getTurnCode(),
//                                            StringTable::SWEDISH )
//                 << "\ttextC: " << m_routeList->routeElementAt( i-1 )->getText()
//                 << " i " << i << endl;
      }


      if ( m_reqVer >= 2 ) {
         // SignPost strings
         for ( uint32 s = 0 ; s < re->getSignPosts().size() ; ++s ) {
            allStringsSize += re->getSignPosts()[ s ].getText().size() + 1;
         }
      }
   }

   // Zero here
   radLat = 0.0;
   radLon = 0.0;
   out3 = 0.0;
   xDiff = 0.0;
   yDiff = 0.0;
   nameChangeTrackpoint = false;
   activeLM.clear();
   
   mc2dbg2 << "Total length of route: " << distLeft << " module says: "
           << m_routeList->getTotalDist() << endl;
   // Send the actual distance
   m_routeList->setTotalDist( uint32( rint( distLeft ) ) );
   mc2dbg2 << "Total time of route: " << routeTotalTime << " module says: "
           << m_routeList->getTotalTime( false ) << " w.stand.t. " 
           << m_routeList->getTotalTime( true ) << endl;
   m_routeList->setTotalTime( routeTotalTime );

   int32 estNbrRouteDatumSize = 
      isabBoxNavMessageUtil::navigatorStructSize*nbrLandmarks + 
      isabBoxNavMessageUtil::navigatorStructSize*differentDisturbanceRoute+
      isabBoxNavMessageUtil::navigatorStructSize * 2 * (  
         (m_routeList->getSize()-nbrNameChangeWPTs-1)/*NamechangeWpts not*/
         /* in route and STP is always in route */
         + std::count( needNewOrigoScale.begin(), needNewOrigoScale.end(),
                       true )*2 /* The needed nbr truncated Origo,Scale */
         + 1/*EPT EPT*/ + 1/*Origo,Scale*/ + 
         1/*One WPT may be in addElem*/ ) +
      isabBoxNavMessageUtil::navigatorStructSize * (
         nbrLandmarkRemindersAtScale/*Landmark reminders at scales*/ );
   maxSize = maxRouteSize - 22 /* Some in "route-header" in reply pak */
      - isabBoxTypeMessage::getHeaderSize( 
         m_routeList->getReq()->getProtoVer() ) - allStringsSize -
      estNbrRouteDatumSize;

   const int32 minCoordBytes = 8192;
   if ( maxSize < minCoordBytes ) {
      mc2log << warn << "IsabRouteList::fillList7p not enough space for "
             << "coords (" << maxSize << " required is " << minCoordBytes 
             << ") " << "client max size " << maxRouteSize 
             << " nbr elements " << m_routeList->getSize() 
             << " string size " << allStringsSize << " estRouteDatumSize "
             << estNbrRouteDatumSize << endl;
      // Client has 1500 routedatums 1500*12 bytes
      maxSize = MIN( minCoordBytes, 
                     int32(1499*12) - estNbrRouteDatumSize );
      if ( maxSize < isabBoxNavMessageUtil::navigatorStructSize*50 ) {
         mc2log << warn << "IsabRouteList::fillList7p route too big "
                << "size for coords " << maxSize << endl;
         return false;
      } else {
         mc2log << info << "IsabRouteList::fillList7p new size for coords "
                << maxSize << endl;
      }
   }

   // Find the start element to determine if the route starts with a 
   // u-turn or not
   bool start_with_uturn = (m_routeList->routeElementAt( 0 )->getTurnCode()
                            == StringTable::DRIVE_START_WITH_UTURN);
   mc2dbg2 << "Adding SPT " << (start_with_uturn?"with":"without") 
           << " uturn" << endl;
   WPTElement* element12 = new WPTElement(
      (int)IsabRouteElement::SPT,
      0, // speedlimit
      0, // on origoX
      0, // on origoY
      start_with_uturn ? 0x01 : 0x00, // flags
      0, // meters
      0, // nameIndex
      0, // crossingKind
      RouteElement::NO_CROSSING );
   m_isabRouteVector.push_back( element12 );
   totSize += isabBoxNavMessageUtil::navigatorStructSize;

   if ( differentDisturbanceRoute ) {
      // Add metapoint with differentDisturbanceRouteStr text
      MetaPTElement* me = new MetaPTElement( 
         MetaPTElement::additional_text, 0x0 /*Flags*/,
         differentDisturbanceRouteStrIndex, 
         0/*high dist*/, 0/*low dist*/ );
      m_isabRouteVector.push_back( me );
      mc2log << info << "differentDisturbanceRoute Added Additional Text "
             << MC2CITE( differentDisturbanceRouteStr ) << endl;
   }

   // Add first element that _ONLY CONTAINS ONE POSITION_
   // and a predfined angle to next point on route
   const RouteElement* re = m_routeList->routeElementAt( 0 );      
   CoordinateTransformer::transformFromMC2( re->getLastLat(),
                                            re->getLastLon(),
                                            CoordinateTransformer::
                                            sweref93_LLA,
                                            radLat,
                                            radLon,
                                            out3 );

   // calc scaleX
   scaleX = cos( radLat ) * scaleY;
   origoX = radLon*scaleX;
   origoY = radLat*scaleY;         
   mc2dbg2 << "Adding OrigoX: " << origoX
           << " OrigoY: " << origoY << endl;
   
   // Create and Add OrigoElement to array
   OrigoElement* oe = new OrigoElement( int32( rint( origoX ) ),
                                        int32( rint( origoY ) ),
                                        0 );
   m_isabRouteVector.push_back(oe);
   totSize += isabBoxNavMessageUtil::navigatorStructSize;
   
   mc2dbg2 << "Adding scaleX: " << scaleX << endl;
   
   // Create and Add ScaleElement to array
   ScaleElement* se =
      new ScaleElement((int)scaleX,
                       uint32((scaleX - (int)scaleX)*65536),
                       0,
                       0);
   m_isabRouteVector.push_back(se);
   totSize += isabBoxNavMessageUtil::navigatorStructSize;


   // init the last variables   
   lastMc2Lat = re->getLastLat();
   lastMc2Lon = re->getLastLon();
   lastWptOtTpt = NULL;
   // The distance between the last and the current coordinate
   float64 distLastToCurr;
   currentSpeed = m_routeList->routeElementAt( 1 )->getSpeedAt( 0 );
   uint8 lastAttribute = isabBoxNavMessageUtil::attributeToFlags(
      m_routeList->routeElementAt( 1 )->getAttributeAt( 0 ) );
   m_truncated = false;
   m_dist2nextWPTFromTrunk = 0;

   // Used to pack TPT in miniPoints or micropoints
   vector< historyPoint > history;
   
   // Attribute-flag 
   uint8 flag = 0;

   // For all routeelements (except first that is STP:ed above)
   for ( int i = 1 ; i < m_routeList->getSize() ; ++i ) {
      const RouteElement* re = m_routeList->routeElementAt( i );
      const RouteElement* preRe = m_routeList->routeElementAt( i-1 );

      // There are always at least 2 coordinates in every one
      for ( int j = 0 ; j < re->getNbrCoord() - 1 ; ++j ) {
         // transform into radians
         CoordinateTransformer::transformFromMC2(
            re->getLatAt( j ), re->getLonAt( j ),
               CoordinateTransformer::sweref93_LLA,
               radLat, radLon, out3 );
         // calculate distance between two coords and add to total dist
         distLastToCurr =
            sqrt( GfxUtility::squareP2Pdistance_linear( re->getLatAt( j ),
                                                        re->getLonAt( j ),
                                                        lastMc2Lat,
                                                        lastMc2Lon ) );
         dist += distLastToCurr;
         distLastWptTptToCurr += distLastToCurr;
         distLeft -= distLastToCurr; // Total dist left

         // calc diff from current origo
         lastXDiff = xDiff;
         lastYDiff = yDiff;
         xDiff = radLon*scaleX - origoX;
         yDiff = radLat*scaleY - origoY;

         if ( int16( lastXDiff ) == int16( xDiff ) &&
              int16( lastYDiff ) == int16( yDiff ) ) {
            // Snap to route implementation in Java client divides by zero 
            // if two coordinates in a row have zero distance.
            // Perturd it by 1m like in perturbLastWptToAvoidDivideByZero...
            mc2dbg2 << "[iRL]: Nudged the coordinate "
                   << "(" <<yDiff<< "," <<xDiff<< ") MC2: ("
                   << re->getLatAt( j ) << ","  << re->getLonAt( j )
                   << ") Last MC2: (" << lastMc2Lat << "," << lastMc2Lon << ")"
                   << " to avoid divide by zero in Java clients." << endl;
            xDiff += 2;
            yDiff += 2;
            mc2dbg2 << "[iRL]: New coord: (" <<yDiff<< "," <<xDiff<< ")" 
                    << endl;
         } // End if same x and y diff as last coord, then nudge it a bit

         vector< IsabRouteElement* > addElem;
         // If a WPT/TPT has used the coordinate
         bool addedCoordinate = false;

         // If needs new origo, scale
         if ( addCoordinates && 
              (fabs( xDiff ) >= maxDist || fabs( yDiff ) >= maxDist) ) 
         {
            // Write coord cache before changing origo
            coordCache( history,
                        lastAddedXDiff, lastAddedYDiff, lastAddedSpeed,
                        totSize, m_isabRouteVector,
                        MAX_INT16, MAX_INT16, MAX_UINT8, 
                        isabBoxNavMessageUtil::attributeToFlags(
                           re->getAttributeAt( j ) ), lastWptOtTpt, 
                        distLastWptTptToCurr, preRe->getText(), true  );

            addNewOrigoScale( radLat, radLon, lastRadLat, lastRadLon,
                              xDiff, yDiff, scaleX, scaleY, 
                              origoX, origoY, totSize, addElem );
            // lastAddedXDiff and lastAddedYDiff Not on new MiniMap
            lastAddedXDiff = MAX_INT16;
            lastAddedYDiff = MAX_INT16;
            lastAddedSpeed = MAX_UINT8;
            if ( fabs( xDiff ) >= maxDist || fabs( yDiff ) >= maxDist ) {
               // Make a while of the top if and add a TPT with an edge
               // coordinate here
               mc2log << error << "IsabRouteList::fillList7p needs to "
                      << "add more than one OrigoScale not supported." 
                      << "RouteID " 
                      << m_routeList->getRouteRequest()->getRouteID() 
                      << " Route CreateTime " 
                      << m_routeList->getRouteRequest()
                  ->getRouteCreateTime() << endl;
            }

            // Add active landmarks here with start = stop = 0
            for ( activeLMMap::const_iterator it = activeLM.begin() ;
                  it != activeLM.end() ; ++it ) {
               LandmarkPTElement* lme = new LandmarkPTElement( *it->second );
               lme->setisStart( false );
               lme->setisEnd( false );
               addElem.push_back( lme );
               mc2dbg2 << "Adding scale LM reminder Text "
                       << MC2CITE( (lme->getStrNameIdx() != MAX_UINT16 ? 
                                    m_stringTable->getStringAt( 
                                       lme->getStrNameIdx() ):"" ) ) 
                       << " dist " << lme->getDistance() << " detour " 
                       << BP( lme->getisDetour() )
                       << " stringNbr " << lme->getStrNameIdx() 
                       << " Start " << BP( lme->getisStart() ) 
                       << " Stop " << BP( lme->getisEnd() ) 
                       << " nextid " << lme->getLandmarkRouteID()
                       << " did " << lme->getLandmarkServerID() << endl;
            }
            // Add lanes and signposts here as reminders
            addLanesAndSignPosts( re, m_stringTable, totSize, 
                                  maxSize, m_nbrStringsUsed,
                                  addElem, m_colorTable, 
                                  true/*reminder*/, lang );

         } // End needs new origo scale

         // If add WPT
         if ( j == 0 && ! nameChangeTrackpoint)
         {
            
            addedCoordinate = true;
            // Clear coord cache
            coordCache( history,
                        lastAddedXDiff, lastAddedYDiff, lastAddedSpeed, 
                        totSize, m_isabRouteVector,
                        MAX_INT16, MAX_INT16, MAX_UINT8,
                        isabBoxNavMessageUtil::attributeToFlags(
                           re->getAttributeAt( j ) ), lastWptOtTpt, 
                        distLastWptTptToCurr, preRe->getText(), true  );
            if ( !addCoordinates && 
                 (fabs( xDiff ) >= maxDist || fabs( yDiff ) >= maxDist) ) 
            {
               // New hack 20030908 Makes the coordinates in WPT valid
               // even if in no coordinates mode
               // needed by client to zoom to turn
               // Needs new scale and origo
               addNewOrigoScale( radLat, radLon, lastRadLat, lastRadLon,
                                 xDiff, yDiff, scaleX, scaleY, 
                                 origoX, origoY, totSize, addElem );
            }
            flag = isabBoxNavMessageUtil::attributeToFlags(
               re->getAttributeAt( j ) );
            // Needs to know when the attributes changes
            lastAttribute = flag;
            currentSpeed = re->getSpeedAt( j );
            mc2dbg2 << "Adding WPT TDP\tx: " << (int)xDiff << "\ty: "
                    << (int)yDiff << "\tTC: "
                    << StringTable::getString( re->getTurnCode(),
                                               StringTable::SWEDISH )
                    << "\tflag: " << (int)flag 
                    << "\ttextC: " << preRe->getText() << " \""
                    << m_routeList->getStringTable()[preRe->getText()]
                    << "\"" 
                    << "\texitCount: " << (int16)re->getExitCount()
                    << "\tspeed: " << (int16)re->getSpeedAt( j ) 
                    << "\tCrossingKind: " 
                    << (int16)re->getCrossingKind() << "\ttime "
                    << re->getTime() << "\tdistLeft " 
                    << uint32( rint( distLeft ) ) << endl;
            WPTElement* element = new WPTElement(
               isabBoxNavMessageUtil::turnCodeToIsabBox( 
                  re, m_routeList->getReq()->getProtoVer() ),
               re->getSpeedAt( j ), // speedlimit
               (int16)xDiff, // from origo
               (int16)yDiff, // from origo
               flag, // flag
               re->getDist(),// Distance is overwritten by the next WPT/TPT
               preRe->getText(),
               re->getExitCount(),
               re->getCrossingKind());               
            // Update last added
            lastAddedXDiff = int16(xDiff);
            lastAddedYDiff = int16(yDiff);
            lastAddedSpeed = currentSpeed; 
            if ( lastWptOtTpt ) {
               mc2dbg2 << "Updating last WPT/TPT dist: " <<
                  distLastWptTptToCurr << endl;
               lastWptOtTpt->setDistance( (uint16)distLastWptTptToCurr );
            }
            lastWptOtTpt = element;
            distLastWptTptToCurr = 0;
            addElem.push_back( element );
            totSize += isabBoxNavMessageUtil::navigatorStructSize;
            // This WPT is no longer needed in trunkated WPT list
            maxSize += isabBoxNavMessageUtil::navigatorStructSize;
            // Nor any Origo,Scale in trunkated WPT list
            if ( needNewOrigoScale[ i ] ) {
               // Nor any Origo,Scale in trunkated WPT list
               maxSize += 2*isabBoxNavMessageUtil::navigatorStructSize;
            }
            // Add TDP
            TimeDistLeftElement* tdp = new TimeDistLeftElement( 
               routeTotalTime - /*re*/preRe->getTotalTimeSoFar(), 
               uint32( rint( distLeft ) ) );
            tdp->setSpeed( re->getSpeedAt( j ) );
            addElem.push_back( tdp );
            totSize += isabBoxNavMessageUtil::navigatorStructSize;
            // This TDP is no longer needed in trunkated WPT list
            maxSize += isabBoxNavMessageUtil::navigatorStructSize;

            
            // Check if next is a namechange Trackpoint.
            // This means that next wpt should be a namechange tpt
            if( re->getTurnCode() == StringTable::LMLOCATION_ROADNAMECHANGE ){
                  nameChangeTrackpoint = true;
                  lastWP = element;
            }

            if ( preRe->getTurnCode() != 
                 StringTable::LMLOCATION_ROADNAMECHANGE )
            {
               // Add landmarks metas here where it is safe
               addLandmark( re, m_stringTable, landmarkStrings,
                            landmarkID, activeLM, totSize, maxSize,
                            m_nbrStringsUsed, addElem );
               if ( addCoordinates ) {
                  // And lanes and signposts
                  addLanesAndSignPosts( re, m_stringTable, totSize, 
                                        maxSize, m_nbrStringsUsed,
                                        addElem, m_colorTable, 
                                        false/*reminder*/, lang );
               }
            } else {
               for ( uint32 l = 0 ; l < re->getNbrLandmarks() ; ++l ) {
                  const ExpandedRouteLandmarkItem* land = re->getLandmark( l );
                  mc2dbg2 << " WPT LMLOCATION_ROADNAMECHANGE skipped Landmark Text "
                          << MC2CITE( landmarkStrings[ land->getLandmarkID() ])
                          << " dist " << land->getDist() << " Side \""
                          << StringTable::getString(
                             ItemTypes::getLandmarkSideSC(
                                land->getRoadSide() ),StringTable::ENGLISH)
                          << "\" type " << int(land->getType() ) 
                          << " location " << StringTable::getString(
                             ItemTypes::getLandmarkLocationSC( 
                                land->getLocation() ),StringTable::ENGLISH)
                          << " Start " 
                          << BP( land->isStart() ) << " Stop "
                          << BP( land->isStop() )
                          << " id " << land->getLandmarkID() << endl;

                  maxSize += landmarkStrings[ land->getLandmarkID() ].size() +
                     1;
                  // This LMP is no used WPT list
                  maxSize += isabBoxNavMessageUtil::navigatorStructSize;
               }
            }
            
         } // End if add WPT
         // If not WPT perhaps needs a TPT?
         else if ( addCoordinates && 
                   (re->getSpeedAt( j ) != currentSpeed || lastAttribute !=
                    isabBoxNavMessageUtil::attributeToFlags(
                       re->getAttributeAt( j ) ) ||
                    (nameChangeTrackpoint && j == 0) ) )
         {
            // Add TPT and TDP
            addedCoordinate = true;

            // Check if this was the last nameChange for now.
            // (Next is not nameChange)
            if( nameChangeTrackpoint && lastWP != NULL &&
               re->getTurnCode() != StringTable::LMLOCATION_ROADNAMECHANGE ){
               nameChangeTrackpoint = false;
               lastWP->setNewType(isabBoxNavMessageUtil::turnCodeToIsabBox( 
                                     re, m_routeList->getReq()->getProtoVer()));
               lastWP->setNewExitCount(re->getExitCount());
               mc2dbg2 << "Changing turndescription of last WP to : "
                       << StringTable::getString( re->getTurnCode(),
                                                  StringTable::SWEDISH )
                       << endl;
            }

            
            // Clear coord cache
            coordCache( history,
                        lastAddedXDiff, lastAddedYDiff, lastAddedSpeed,
                        totSize, m_isabRouteVector,
                        MAX_INT16, MAX_INT16, MAX_UINT8,
                        isabBoxNavMessageUtil::attributeToFlags(
                           re->getAttributeAt( j ) ), lastWptOtTpt, 
                        distLastWptTptToCurr, preRe->getText(), true  );
            flag = isabBoxNavMessageUtil::attributeToFlags(
               re->getAttributeAt( j ) );
            lastAttribute = flag;
            currentSpeed = re->getSpeedAt( j );
            mc2dbg2 << "Adding TPT TDP \tx: " 
                    << (int)xDiff << "\ty: "
                    << (int)yDiff << "\tflag: " << (int)flag
                    << "\tspeed: " << (int16)currentSpeed << "\tname "
                    << preRe->getText() << " \"" 
                    << m_routeList->getStringTable()[ preRe->getText() ]
                    << "\"" << "\ttime "
                    << re->getTime() << "\tdistLeft " 
                    << uint32( rint( distLeft ) ) << endl;
            TPTElement* tpt = new TPTElement(
               (int16)xDiff, // from origo
               (int16)yDiff, // from origo
               flag,
               0, // Distance is filled in by the next WPT/TPT
               currentSpeed );
            tpt->setNameIndex( preRe->getText() );
            // Update last added
            lastAddedXDiff = int16(xDiff);
            lastAddedYDiff = int16(yDiff);
            lastAddedSpeed = currentSpeed;
            if ( lastWptOtTpt != NULL ) {
               mc2dbg2 << "Updating last WPT/TPT dist: " <<
                  distLastWptTptToCurr << endl;
               lastWptOtTpt->setDistance( (uint16)distLastWptTptToCurr );
            }
            lastWptOtTpt = tpt;
            distLastWptTptToCurr = 0;
            addElem.push_back( tpt );
            totSize += isabBoxNavMessageUtil::navigatorStructSize;
            // Add TDP
            TimeDistLeftElement* tdp = new TimeDistLeftElement( 
               routeTotalTime - /*re*/preRe->/*getTime*/getTotalTimeSoFar(), 
               uint32( rint( distLeft ) ) );
            tdp->setSpeed( currentSpeed );
            addElem.push_back( tdp );
            totSize += isabBoxNavMessageUtil::navigatorStructSize;

            // Add landmarks here even if it is not a wpt, we need the
            // tdp dists before we can move them to prevoius wpt.
            // This is done in the loop after this that goes through the
            // m_isabRouteVector backwards.
            if ( j == 0 ) {
               addLandmark( re, m_stringTable, landmarkStrings,
                            landmarkID, activeLM, totSize, maxSize,
                            m_nbrStringsUsed, addElem );
               addLanesAndSignPosts( re, m_stringTable, totSize, 
                                     maxSize, m_nbrStringsUsed,
                                     addElem, m_colorTable,
                                     false/*reminder*/, lang );
            }
               
         } else if ( !addCoordinates && nameChangeTrackpoint && j == 0 ) {
            // Not adding coordinates but still has nameChangeTrackpoint
            // Check if this was the last nameChange for now.
            if ( lastWP != NULL && re->getTurnCode() != 
                 StringTable::LMLOCATION_ROADNAMECHANGE )
            {
               nameChangeTrackpoint = false;
               lastWP->setNewType( 
                  isabBoxNavMessageUtil::turnCodeToIsabBox(
                     re, m_routeList->getReq()->getProtoVer() ) );
               mc2dbg2 << "When !addCoordinates changing turndescription "
                       << "of last WP to : "
                       << StringTable::getString( re->getTurnCode(),
                                                  StringTable::SWEDISH )
                       << endl;
            }
            // This namechange WPT is not in trunkated list
            for ( uint32 i = 0 ; i < re->getNbrLandmarks() ; ++i ) {
               const ExpandedRouteLandmarkItem* land = re->getLandmark( i );
               maxSize += landmarkStrings[ land->getLandmarkID() ].size() + 1;
               // This LMP is no used WPT list
               maxSize += isabBoxNavMessageUtil::navigatorStructSize;
            }
         }



         // Add coordinate and addEl
         if ( !addElem.empty() ) {
            for ( uint32 k = 0 ; k < addElem.size() ; ++k ) {
               m_isabRouteVector.push_back( addElem[ k ] );
            }
         }
         if ( addCoordinates && !addedCoordinate ) {
            coordCache( history,
                        lastAddedXDiff, lastAddedYDiff, lastAddedSpeed,
                        totSize, m_isabRouteVector, 
                        int16(xDiff), int16(yDiff), re->getSpeedAt( j ),
                        isabBoxNavMessageUtil::attributeToFlags(
                           re->getAttributeAt( j ) ), lastWptOtTpt, 
                        distLastWptTptToCurr, preRe->getText() );
         }


         // If stop adding coordinates and just add WPTs
         if ( addCoordinates && (int32(totSize) >= maxSize || 
                                 currentTime >= timeBeforeTrunk ) ) 
         {
            // New hack 20030908 Make the coordinates in WPT valid
            // even if in no coordinates mode by adding Origo/Scale 
            // not needed here as addCoordinates is true
            addCoordinates = false;
            history.clear();
            m_truncated = true;
            m_truncatedDist = int( rint( dist ) );
            m_truncatedWPTNbr = i;
            // Add first EPT with last know xDiff and yDiff
            // (used in navigator as it is today)
            mc2dbg2 << "Adding trunkating-EPT \tx: " << (int)xDiff 
                    << "\ty: " << (int)yDiff << "\tTC: "
                    << (int)IsabRouteElement::EPT << "\tdist: "
                    << 0 << "\tangle: " << 0 << endl;

            WPTElement* element =
               new WPTElement(IsabRouteElement::EPT,
                              0, // speedlimit
                              (int16)xDiff, // from origo
                              (int16)yDiff, // from origo
                              0xff, // flags 0xff Trunkated EPT
                              0, // meters
                              MAX_UINT16,
                              0,
                              RouteElement::NO_CROSSING);
            if ( lastWptOtTpt != NULL ) {
               mc2dbg2 << "Updating last WPT/TPT dist: " <<
                  distLastWptTptToCurr << endl;
               lastWptOtTpt->setDistance((uint16)distLastWptTptToCurr);
            }
            lastWptOtTpt = element;
            m_isabRouteVector.push_back( element );
            totSize += isabBoxNavMessageUtil::navigatorStructSize;
            // This EPT is now not in extra space
            maxSize += isabBoxNavMessageUtil::navigatorStructSize;
         }

         // Update last with this coordinate
         lastRadLat = radLat;
         lastRadLon = radLon;
         lastMc2Lat = re->getLatAt( j );
         lastMc2Lon = re->getLonAt( j );
      } // for all coordinates in re


      currentTime += re->getTime();

   } // for all routeelements


   if ( !addCoordinates && 
        (fabs( xDiff ) >= maxDist || fabs( yDiff ) >= maxDist) ) 
   {
      // New hack 20030908 Makes the coordinates in WPT valid
      // even if in no coordinates mode
      // needed by client to zoom to turn
      // Needs new scale and origo for EPT
      vector< IsabRouteElement* > addElem;
      addNewOrigoScale( radLat, radLon, lastRadLat, lastRadLon,
                        xDiff, yDiff, scaleX, scaleY, 
                        origoX, origoY, totSize, addElem );
      for ( uint32 k = 0 ; k < addElem.size() ; ++k ) {
         m_isabRouteVector.push_back( addElem[ k ] );
      }
   }

   // Add EPT
   if ( !m_truncated ) {
      // Add first EPT with last know xDiff and yDiff
      // (used in navigator as it is today)
      mc2dbg2 << "Adding trunkating-EPT at end \tx: " << (int)xDiff 
              << "\ty: " << (int)yDiff << "\tTC: "
              << (int)IsabRouteElement::EPT << "\tdist: "
              << 0 << "\tangle: " << 0 << endl;
      WPTElement* element =
         new WPTElement(IsabRouteElement::EPT,
                        0, // speedlimit
                        (int16)xDiff, // from origo
                        (int16)yDiff, // from origo
                        0xff, // flags 0xff Trunkated EPT
                        0, // meters
                        MAX_UINT16,
                        0,
                        RouteElement::NO_CROSSING); 
      if ( lastWptOtTpt != NULL ) {
         mc2dbg2 << "Updating last WPT/TPT dist: " <<
            distLastWptTptToCurr << endl;
         lastWptOtTpt->setDistance((uint16)distLastWptTptToCurr);
      }
      lastWptOtTpt = element;
      m_isabRouteVector.push_back( element );
      totSize += isabBoxNavMessageUtil::navigatorStructSize;
      // This EPT is now not in extra space
      maxSize += isabBoxNavMessageUtil::navigatorStructSize;
   }
   mc2dbg2 << "Adding EPT \tx: " << (int)xDiff << "\ty: "
           << (int)yDiff << "\tTC: "
           << (int)IsabRouteElement::EPT << "\tdist: "
           << 0 << "\tangle: " << 0 << endl;
   WPTElement* element =
      new WPTElement(IsabRouteElement::EPT,
                     0, // speedlimit
                     (int16)xDiff, // from origo
                     (int16)yDiff, // from origo
                     0, // flags
                     0, // meters
                     MAX_UINT16,
                     0,
                     RouteElement::NO_CROSSING);
   if ( lastWptOtTpt != NULL ) {
      mc2dbg2 << "Updating last WPT/TPT dist: " <<
         distLastWptTptToCurr << endl;
      lastWptOtTpt->setDistance((uint16)distLastWptTptToCurr);
   }
   lastWptOtTpt = element;
   m_isabRouteVector.push_back( element );
   totSize += isabBoxNavMessageUtil::navigatorStructSize;
   // This EPT is now not in extra space
   maxSize += isabBoxNavMessageUtil::navigatorStructSize;
   
   // Loop and set nextOrigo in the origoElements
   // Also set TDP times
   // Also try to fixup landmarks after TPT TDP by moving them to prevoius
   // WPT
   int k = m_isabRouteVector.size();
   int16 nextOrigo = 0;

   // TDP time counting 
   vector< TimeDistLeftElement* > tdpFromLastWpt;
   uint32 lastDTG = 0; // Begins at end
   uint32 lastETA = 0; // Begins at end

   while (--k >= 0) {
      IsabRouteElement* ire = m_isabRouteVector[ k ];
      
      if (dynamic_cast<OrigoElement*>(ire) != NULL) {
         OrigoElement* oe = dynamic_cast<OrigoElement*>(ire);
         oe->setNextOrigo(nextOrigo-k);
         nextOrigo = k;
      } else if ( dynamic_cast<TimeDistLeftElement*>( ire ) != NULL ) {
         tdpFromLastWpt.push_back( static_cast< TimeDistLeftElement* > ( 
                                      ire ) );
         if ( k + 1 < int32(m_isabRouteVector.size()) && /*Space for a lm*/
              dynamic_cast<TPTElement*>( m_isabRouteVector[ k - 1 ] ) != NULL&&
              dynamic_cast<LandmarkPTElement*>( m_isabRouteVector[ k + 1 ] )
              != NULL )
         {
            // Landmarks after TPT!
            // Move them to prevoius WPT (WPT with lower k, so ahead in this 
            // loop)

            int wp = k;
            WPTElement* wpt = NULL;
            while ( wp > 0 && (wpt = dynamic_cast<WPTElement*>( 
                       m_isabRouteVector[ wp ] )) == NULL )
            {
               --wp;
            }
            if ( wpt != NULL ) {
               // Try to move lms here and update their dist
               // No need to update dist as it is distance to next wpt and
               // it is already right 20070502 (Check expandrouteprocessor...)

               uint32 lmnbr = 0;
               // While LandmarkPTElement at k + 1 + lmnbr
               while ( k + 1 + lmnbr < m_isabRouteVector.size() &&
                       dynamic_cast<LandmarkPTElement*>( 
                          m_isabRouteVector[ k + 1 + lmnbr ] ) != NULL ) {
                  LandmarkPTElement* lm = static_cast<LandmarkPTElement*>(
                     m_isabRouteVector[ k + 1 + lmnbr ] );
                  m_isabRouteVector.erase( m_isabRouteVector.begin() + ( k + 1 + lmnbr ) );
                  m_isabRouteVector.insert( m_isabRouteVector.begin() + ( wp + 2 + lmnbr ) , lm );
                  ++lmnbr;
               }
               // While laneinfo at k + 1 + lmnbr
               while ( k + 1 + lmnbr < m_isabRouteVector.size() &&
                       dynamic_cast<LaneInfoPTElement*>( 
                          m_isabRouteVector[ k + 1 + lmnbr ] ) != NULL ) {
                  LaneInfoPTElement* li = static_cast<LaneInfoPTElement*>(
                     m_isabRouteVector[ k + 1 + lmnbr ] );
                  m_isabRouteVector.erase( m_isabRouteVector.begin() + ( k + 1 + lmnbr ) );
                  m_isabRouteVector.insert( m_isabRouteVector.begin() + ( wp + 2 + lmnbr ) , li );
                  ++lmnbr;
               }
               // While lanedata at k + 1 + lmnbr
               while ( k + 1 + lmnbr < m_isabRouteVector.size() &&
                       dynamic_cast<LaneDataPTElement*>( 
                          m_isabRouteVector[ k + 1 + lmnbr ] ) != NULL ) {
                  LaneDataPTElement* ld = static_cast<LaneDataPTElement*>(
                     m_isabRouteVector[ k + 1 + lmnbr ] );
                  m_isabRouteVector.erase( m_isabRouteVector.begin() + ( k + 1 + lmnbr ) );
                  m_isabRouteVector.insert( m_isabRouteVector.begin() + ( wp + 2 + lmnbr ) , ld );
                  ++lmnbr;
               }
               // While signpost at k + 1 + lmnbr
               while ( k + 1 + lmnbr < m_isabRouteVector.size() &&
                       dynamic_cast<SignPostPTElement*>( 
                          m_isabRouteVector[ k + 1 + lmnbr ] ) != NULL ) {
                  SignPostPTElement* sp = static_cast<SignPostPTElement*>(
                     m_isabRouteVector[ k + 1 + lmnbr ] );
                  m_isabRouteVector.erase( m_isabRouteVector.begin() + ( k + 1 + lmnbr ) );
                  m_isabRouteVector.insert( m_isabRouteVector.begin() + ( wp + 2 + lmnbr ) , sp );
                  ++lmnbr;
               }
            }
         }
      } else if ( dynamic_cast<WPTElement*>( ire ) != NULL ) {
         // Calculate times for tdps from last wpt
         if ( tdpFromLastWpt.size() > 1 ) { // More than one 
            // Calculate total dist / speed sum
            // Always TDP after WPT except last
            float64 totalFactor = 0;
            uint32 distSoFar = lastDTG;
            uint32 dist = 0;
            vector< TimeDistLeftElement* >::iterator it = 
               tdpFromLastWpt.begin();
            for (  ; it != tdpFromLastWpt.end() ; ++it ) {
               dist = (*it)->getDistLeft() - distSoFar;
               totalFactor += ( float64( dist ) /
                                (*it)->getSpeed() );
               distSoFar += dist;
            }
            uint32 totalTime = tdpFromLastWpt.back()->getTimeLeft() - 
               lastETA;

            uint32 timeUsed = 0;
            distSoFar = lastDTG;
            it = tdpFromLastWpt.begin();
            for ( ; 
                  it != tdpFromLastWpt.end() ; 
                  it++ )
            {
               dist = (*it)->getDistLeft() - distSoFar;
               distSoFar += dist;
               uint32 tdpTime = uint32( rint( float64(
                  dist ) / (*it)->getSpeed() / 
                     totalFactor *  totalTime ) );
               timeUsed += tdpTime;
               (*it)->setTimeLeft( lastETA + timeUsed );
               mc2dbg4 << "Time TDP: " << tdpTime << " ETA " 
                       << (*it)->getTimeLeft()
                       << " dist " << dist << " speed " 
                       << int((*it)->getSpeed()) << endl;
            }

            // Set last ETA, DTG
            lastETA = tdpFromLastWpt.back()->getTimeLeft();
            lastDTG = tdpFromLastWpt.back()->getDistLeft();
            
            // All done with these
            tdpFromLastWpt.clear();
         } else if ( tdpFromLastWpt.size() == 1 ) {
            mc2dbg4 << "Single TDP between WPTs"
                    << endl;
            mc2dbg4 << "WPT dist " 
                    << static_cast<WPTElement*>( ire )->getDistance() 
                    << " SpeedLimit " << int(
                       static_cast<WPTElement*>( ire )->getSpeedLimit())
                    << endl;
            mc2dbg4 << "timeleft " 
                    << tdpFromLastWpt.back()->getTimeLeft()
                    << " tdp distleft " 
                    << tdpFromLastWpt.back()->getDistLeft() << endl;
            lastETA = tdpFromLastWpt.back()->getTimeLeft();
            lastDTG = tdpFromLastWpt.back()->getDistLeft();
            // All done with this
            tdpFromLastWpt.clear();
         } // Else none, zero, tdp From Last Wpt! Truncated EPT and SPT
      } // End if is WPT and addTdp
   } // End while ( --k > 0 )
   
   mc2dbg2 << "WPT and TDP nodes:" << endl;
   for ( uint32 k = 0 ; k < m_isabRouteVector.size() ; ++k ) {
      if ( dynamic_cast<TimeDistLeftElement*>( m_isabRouteVector[ k ] ) 
           != NULL ) 
      {
         TimeDistLeftElement* tdp = static_cast< TimeDistLeftElement* > ( 
            m_isabRouteVector[ k ] );
         mc2dbg2 << "TDP: ETG " << tdp->getTimeLeft()
                 << " DTG " << tdp->getDistLeft() << " speed " 
                 << int(tdp->getSpeed()) << endl;
      } else if ( dynamic_cast<WPTElement*>( m_isabRouteVector[ k ] ) 
                  != NULL ) 
      {
         WPTElement* wpt = static_cast< WPTElement* > ( 
            m_isabRouteVector[ k ] );
         mc2dbg2 << "WPT: dist " << wpt->getDistance() << " flags "
                 << int(wpt->getFlags()) << " speed " 
                 << int(wpt->getSpeedLimit()) << " lon " << wpt->getLon()
                 << " lat " << wpt->getLat() << " text " 
                 << (wpt->getTextIndex() != MAX_UINT16 ? 
                     m_stringTable->getStringAt( wpt->getTextIndex() ):"" )
                 << " action " << StringTable::getString( 
                    isabBoxNavMessageUtil::isabBoxToTurnCode( wpt->getAction(),
                                       m_routeList->getReq()->getProtoVer() ),
                    StringTable::SWEDISH )
                 << " exitc " << int(wpt->getExitCount()) << endl;
      } else if ( dynamic_cast<TPTElement*>( 
                     m_isabRouteVector[ k ] ) != NULL )
      {
         TPTElement* wpt = static_cast< TPTElement* > ( 
            m_isabRouteVector[ k ] );
         mc2dbg2 << "TPT: dist " << wpt->getDistance() << " flags "
                 << int(wpt->getFlags()) << " speed " 
                 << int(wpt->getSpeedLimit()) << " lon " << wpt->getLon()
                 << " lat " << wpt->getLat() << " text " 
                 << (wpt->getTextIndex() != MAX_UINT16 ? 
                     m_stringTable->getStringAt( wpt->getTextIndex() ):"" )
                 << endl;
      } else if ( dynamic_cast<LandmarkPTElement*>( 
                     m_isabRouteVector[ k ] ) != NULL )
      {
         LandmarkPTElement* l = static_cast<LandmarkPTElement*>( 
            m_isabRouteVector[ k ] );
         mc2dbg2 << "Landmark: Text "
                 << MC2CITE( (l->getStrNameIdx() != MAX_UINT16 ? 
                     m_stringTable->getStringAt( l->getStrNameIdx() ):"" ) ) 
                 << " dist " << l->getDistance() << " detour " 
                 << BP( l->getisDetour() )
                 << " stringNbr " << l->getStrNameIdx() 
                 << " Start " << BP( l->getisStart() ) 
                 << " Stop " << BP( l->getisEnd() ) 
                 << " nextid " << l->getLandmarkRouteID()
                 << " did " << l->getLandmarkServerID() << endl;
      } else if ( dynamic_cast<ScaleElement*>( 
                     m_isabRouteVector[ k ] ) != NULL )
      {
         ScaleElement* s = static_cast<ScaleElement*>( 
            m_isabRouteVector[ k ] );
         mc2dbg2 << "Scale " << s->getRefLat() << ", " << s->getRefLon() 
                 << endl;
      }
   }

   m_additionalRouteData = false;

   mc2dbg2 << "Total size of route elements: " << totSize << endl;

   // downloadAdditionalRouteData
   // All WPTs are sent. If client wants all coordinates it must send
   // big buffer


   // Set truncated in RouteList
   m_routeList->setTruncated( m_truncated );
   m_routeList->setTruncatedDist( m_truncatedDist );
   m_routeList->setTruncatedWPTNbr( m_truncatedWPTNbr );

   // Perturb the coordinate of the last waypoint in case it is on the
   // same position as the end point. This is a workaround for old clients
   // that otherwise will divide by zero.
   perturbLastWptToAvoidDivideByZero();

   return true;
}


void 
IsabRouteList::coordCache( vector< historyPoint >& history,
                           int16& lastXDiff, int16& lastYDiff,
                           uint8& lastSpeed,
                           uint32& totSize, IsabRouteElementVector& isabRouteVector,
                           float32 xDiff, float32 yDiff, uint8 speed,
                           uint8 flag, WPTorTPTElement*& lastWptOtTpt,
                           float64& distLastWptTptToCurr,
                           uint16 nameIndex,
                           bool forceClean )
{
   if ( yDiff != MAX_INT16 ) {
      // Add new coord
      history.push_back( 
         historyPoint( xDiff, yDiff, speed, distLastWptTptToCurr ) );
   }


   if ( history.empty() ) {
      // Nothing to do
   } else  {
      // Check if to output something
      
      // How many fit inside Micro?
      uint32 nbrUFit = 0;
      if ( lastYDiff != MAX_INT16 ) { // On cur MiniMap
         float32 lastX     = lastXDiff;
         float32 lastY     = lastYDiff;
         uint8   lastspeed = lastSpeed; // Speed not relative
         for ( uint32 i = 0 ; i < history.size() ; ++i ) {
            if ( fabs( lastX - history[ i ].xDiff ) < MAX_INT8 &&
                 fabs( lastY - history[ i ].yDiff ) < MAX_INT8 &&
                 !(lastX == history[ i ].xDiff && // Not same coord
                   lastY == history[ i ].yDiff ) && // 0,0 invalid in Micro
                 lastspeed == history[ i ].speed )
            {
               lastX = history[ i ].xDiff;
               lastY = history[ i ].yDiff;
               lastspeed = history[ i ].speed; // Not needed,sameallthetime
            } else {
               // Not update nbrUFit and end loop
               break;
            }
            nbrUFit++;
         }
      }
      if ( m_protoVer < 0xa ) {
         // Turn off micropoints as Wayfinder had that out-defed
         nbrUFit = 0;
      }

      if ( nbrUFit > 2 && 
           (history.size() >= 5 || history.size() > nbrUFit || 
            forceClean) ) 
      {
         // Add Micro point
         vector< pair< int8, int8 > > coords;
         float32 lastX = lastXDiff;
         float32 lastY = lastYDiff;
         mc2dbg2 << "Adding MICRO ";
         for ( uint32  i = 0 ; i < nbrUFit ; ++i ) {
            mc2dbg2 << "\tx" << (i+1) << ": " << int(history[ i ].xDiff)
                    << "\ty" << (i+1) << ": " << int(history[ i ].yDiff);
//             mc2dbg << "\tdx " << (history[ i ].xDiff - lastX) 
//                    << "\tdy " << (history[ i ].yDiff - lastY);
            coords.push_back( 
               make_pair( int8(history[ i ].xDiff - lastX),
                          int8(history[ i ].yDiff - lastY) ) );
            lastX = history[ i ].xDiff;
            lastY = history[ i ].yDiff;
         }
         for ( uint32  i = nbrUFit ; i < 5 ; ++i ) {
            coords.push_back( make_pair( 0, 0 ) ); // 0,0 point is invalid
            mc2dbg2 << "\tx" << (i+1) << ": " << 0 
                    << "\ty" << (i+1) << ": " << 0;
         }
         mc2dbg2 << endl;

//         mc2dbg2 << "Adding MICRO(W.DIFFS) ";
//         for ( uint32  i = 0 ; i < 5 ; ++i ) {
//            mc2dbg2 << "\tx" << (i+1) << ": " << int(coords[ i ].first) 
//                    << "\ty" << (i+1) << ": " << int(coords[ i ].second);
//         }
//         mc2dbg2 << endl;
         MicroDeltaElement* mp = new MicroDeltaElement(
            coords[ 0 ].first, coords[ 0 ].second,
            coords[ 1 ].first, coords[ 1 ].second,
            coords[ 2 ].first, coords[ 2 ].second,
            coords[ 3 ].first, coords[ 3 ].second,
            coords[ 4 ].first, coords[ 4 ].second );
         isabRouteVector.push_back( mp );
         totSize += isabBoxNavMessageUtil::navigatorStructSize;         
         lastXDiff = int16(lastX);
         lastYDiff = int16(lastY);
         // Speed not updated
         history.erase( history.begin(), history.begin() + nbrUFit );
      }

      while ( ((nbrUFit < history.size()) || forceClean) && 
              history.size() >= 2 )
      {
         // Add Mini point
         mc2dbg2 << "Adding MINI \tx1: " << int(history[ 0 ].xDiff) 
                 << "\ty1: " << int(history[ 0 ].yDiff) << "\tspeed1: " 
                 << int(history[ 0 ].speed)
                 << endl 
                 << "\t\tx2: " << int(history[ 1 ].xDiff) << "\tx2: " 
                 << int(history[ 1 ].yDiff)
                 << "\tspeed2: " << int(history[ 1 ].speed) << endl;
         MiniElement* mp = new MiniElement( 
            int16(history[ 0 ].xDiff), int16(history[ 0 ].yDiff),
            int16(history[ 1 ].xDiff), int16(history[ 1 ].yDiff),
            history[ 0 ].speed, history[ 1 ].speed );
         isabRouteVector.push_back( mp );
         totSize += isabBoxNavMessageUtil::navigatorStructSize;         
         lastXDiff = int16(history[ 1 ].xDiff);
         lastYDiff = int16(history[ 1 ].yDiff);
         lastSpeed = history[ 1 ].speed;
         history.erase( history.begin(), history.begin() + 2 );
      }

      if ( forceClean && history.size() >= 1 ) {
         // Add TDP
         mc2dbg2 << "Adding TPT \tx: " << int(history[ 0 ].xDiff) 
                 << "\ty1: " << int(history[ 0 ].yDiff) << "\tspeed: " 
                 << int(history[ 0 ].speed) << "\tflag: " << int(flag)
                 << "\tname " << nameIndex << " \"" 
                 << m_routeList->getStringTable()[ nameIndex ] << "\"" 
                 << endl;
         TPTElement* tpt = new TPTElement(
            int16(history[ 0 ].xDiff), int16(history[ 0 ].yDiff), flag,
            0, // Distance is filled in by the next WPT/TPT
            history[ 0 ].speed );
         tpt->setNameIndex( nameIndex );
         if ( lastWptOtTpt != NULL ) {
            mc2dbg2 << "Updating last WPT/TPT dist: " <<
               history[ 0 ].distLastWptTptToCurr << endl;
            lastWptOtTpt->setDistance( 
               uint16( history[ 0 ].distLastWptTptToCurr ) );
         }
         lastWptOtTpt = tpt;
         distLastWptTptToCurr = distLastWptTptToCurr - 
            history[ 0 ].distLastWptTptToCurr;
         isabRouteVector.push_back( tpt );
         totSize += isabBoxNavMessageUtil::navigatorStructSize;
         lastXDiff = int16(history[ 0 ].xDiff);
         lastYDiff = int16(history[ 0 ].yDiff);
         lastSpeed = history[ 0 ].speed;
         history.erase( history.begin() );
      }

   }
}


void 
IsabRouteList::addNewOrigoScale( float64 radLat, float64 radLon,
                                 float64 lastRadLat, float64 lastRadLon,
                                 float32& xDiff, float32& yDiff,
                                 float32& scaleX, float32& scaleY,
                                 float32& origoX, float32& origoY,
                                 uint32& totSize,
                                 vector< IsabRouteElement* >& addElem )
{
   // Needs new scale and origo
   scaleX = cos(radLat)*scaleY;                        
   origoX = radLon*scaleX;
   origoY = radLat*scaleY;

   // Create and Add OrigoElement to array
   OrigoElement* oe = new OrigoElement( int32( rint( origoX ) ),
                                        int32( rint( origoY ) ),
                                        0 );
   mc2dbg2 << "Adding OrigoX: " << origoX
           << " OrigoY: " << origoY << endl;
   addElem.push_back( oe );
   totSize += isabBoxNavMessageUtil::navigatorStructSize;
   
   ScaleElement* se =
      new ScaleElement( (int)scaleX,
                        uint32((scaleX - (int)scaleX)*65536),
                        uint16(lastRadLon*scaleX - origoX),
                        uint16(lastRadLat*scaleY - origoY));
   mc2dbg2 << "Adding scaleX: " << scaleX << endl;
   addElem.push_back( se );
   totSize += isabBoxNavMessageUtil::navigatorStructSize;
   
   // Recalculate xDiff and YDiff using new origo
   xDiff = radLon*scaleX - origoX;
   yDiff = radLat*scaleY - origoY;
}

void 
IsabRouteList::perturbLastWptToAvoidDivideByZero()
{
   
   // First check the indeces of the last waypoint and the last endpoint.
   int lastWPTIdx = -1;
   int lastEPTIdx = -1;
   for ( int k = m_isabRouteVector.size() - 1; k >= 0; --k ) {
      if ( dynamic_cast<WPTElement*>( m_isabRouteVector[ k ] ) 
                  != NULL ) {
         WPTElement* wpt = static_cast<WPTElement*> ( 
               m_isabRouteVector[ k ] );
     
         if ( wpt->getAction() == IsabRouteElement::EPT ) {
            // The end point.
            lastEPTIdx = k;
         } else {
            // Not the end point, i.e. the last waypoint.
            lastWPTIdx = k;
            // Jump out.
            break;
         }
      }
   }

   if ( lastWPTIdx < 0 || lastEPTIdx < 0 ) {
      return;
   }
   
   OrigoElement* origo = NULL;
   ScaleElement* scale = NULL;

   double prevLat = 0;
   double prevLon = 0;
   double curLat = 0;
   double curLon = 0;
   
   WPTElement* lastWpt = NULL;
   
   for ( int k = 0 ; k < (int)m_isabRouteVector.size() ; ++k ) {
      if ( dynamic_cast<OrigoElement*>( m_isabRouteVector[ k ] ) 
                  != NULL ) 
      {
         origo = static_cast< OrigoElement* > ( m_isabRouteVector[ k ] );
      }
      else if ( dynamic_cast<ScaleElement*>( m_isabRouteVector[ k ] ) 
                  != NULL ) 
      {
         scale = static_cast< ScaleElement* > ( m_isabRouteVector[ k ] );
      } else if ( k == lastEPTIdx || k == lastWPTIdx ) {
         // Either the last waypoint or last endpoint.
         WPTElement* wpt = static_cast< WPTElement* > ( 
            m_isabRouteVector[ k ] );
   
         MC2_ASSERT( origo != NULL );
         MC2_ASSERT( scale != NULL );

         prevLat = curLat;
         prevLon = curLon;
         
         // Calculate the absolute coordinate.
         double scaleX = scale->getIntegerPart() + 
               (double)scale->getRestPart() / 65536;
         double scaleY = GfxConstants::EARTH_RADIUS;
         curLat = (wpt->getLat() + origo->getLat()) / scaleY * 180 / M_PI;
         curLon = (wpt->getLon() + origo->getLon()) / scaleX * 180 / M_PI;

         if ( k == lastEPTIdx ) {
            // Last endpoint.
            
            // Check if the last waypoint has the same coordinate as the
            // last endpoint.
            if ( (curLat - prevLat) == 0 &&
                 (curLon - prevLon) == 0 ) {
               // Perturb (move) the coordinate of the last waypoint
               // two meters. 
               mc2dbg << "[iRL]: Perturbed the coordinate "
                      << "of the last waypoint to avoid divide by zero "
                      << "for old clients." << endl;
               mc2dbg1 << "[iRL]: Old coord: (" << lastWpt->getLat() 
                       << "," << lastWpt->getLon() << ")" << endl;
               lastWpt->setLat( lastWpt->getLat() + 8 );
               lastWpt->setLon( lastWpt->getLon() + 8 );
               mc2dbg1 << "[iRL]: New coord: (" << lastWpt->getLat() 
                       << "," << lastWpt->getLon() << ")" << endl;
            }
         } else {
            // Store the last waypoint.
            lastWpt = wpt;
         }
      }
   }
}

void
IsabRouteList::addLandmark( const RouteElement* re, 
                            NavStringTable* stringTable,
                            const map< uint32, MC2String >& landmarkStrings, 
                            uint16& landmarkID,
                            activeLMMap& activeLM, uint32& totSize,
                            int32& maxSize, int& nbrStringsUsed,
                            vector< IsabRouteElement* >& addElem ) const
{
   lmsetOrdered lands;
   map<uint32, LandmarkPTElement*>::iterator lmi;

   for ( uint32 l = 0 ; l < re->getNbrLandmarks() ; ++l ) {
      lands.insert( re->getLandmark( l ) );
   }

   for ( lmsetOrdered::const_iterator it = lands.begin() ; it != lands.end() ;
         ++it ) {
      const ExpandedRouteLandmarkItem* land = *it;
                  
      // Add string to stringTable->addString
      int strNbr = stringTable->addString( 
         landmarkStrings.find( land->getLandmarkID() )->second.c_str() );
                  
      uint32 nextID;
      // check map ? (!(start && stop)
      lmi = activeLM.find(land->getLandmarkID());
      if ( !land->isStart() && land->isStop() && lmi != activeLM.end()  ) {
         nextID = lmi->second->getLandmarkRouteID();
      } else {
         nextID = landmarkID++;
      }
                  
      // Add landmarkpoint with text.
      LandmarkPTElement* lme =
         new LandmarkPTElement(land->isDetour(),
                               mc2Side2Nav(land->getRoadSide()),
                               mc2Landmark_t2Nav(
                                  land->getType()),
                               mc2Landmark_location_t2Nav(
                                  land->getLocation()),
                               land->getDist(),
                               strNbr,
                               land->isStart(),
                               land->isStop(),
                               nextID,
                               land->getLandmarkID());
                  
      addElem.push_back( lme );
                  
      // check map ? (!(start && stop)
      if ( land->isStart() && !land->isStop() && lmi == activeLM.end() ) {
         // Save this landmark.
         activeLM.insert( make_pair( land->getLandmarkID(), lme ) );
      } else if ( land->isStop() && lmi != activeLM.end() ) {
         // Landmark passed, remove it
         activeLM.erase( lmi );
      }
                 
      totSize += isabBoxNavMessageUtil::navigatorStructSize;
      // This LMP is no longer needed in trunkated WPT list
      maxSize += isabBoxNavMessageUtil::navigatorStructSize;
      mc2dbg2 << " Added Landmark Text "
              << MC2CITE( landmarkStrings.find( 
                             land->getLandmarkID() )->second ) 
              << " dist " << land->getDist() << " Side \""
              << StringTable::getString(
                 ItemTypes::getLandmarkSideSC(
                    land->getRoadSide() ),StringTable::ENGLISH)
              << "\" type " << int(land->getType() ) 
              << " location " << StringTable::getString(
                 ItemTypes::getLandmarkLocationSC( 
                    land->getLocation() ),StringTable::ENGLISH)
              << " stringNbr " << strNbr << " Start " 
              << BP( land->isStart() ) << " Stop "
              << BP( land->isStop() ) << " nextid " << nextID
              << " id " << land->getLandmarkID() << endl;
      nbrStringsUsed = int32(stringTable->getSize()) -1;
   }
}


uint8 laneByte( const ExpandStringLane& l, const RouteElement* re ) {
   uint8 b = 0;
   // Lane direction
   switch( l.getSingleDirectionUsingTurn( re->getTurnCode() ) ) {
      case ExpandStringLane::no_dir :
         b = 0;
         break;
      case ExpandStringLane::left :
      case ExpandStringLane::sharp_left :
      case ExpandStringLane::uturn_left :
         b = 2;
         break;
      case ExpandStringLane::half_left :
         b = 3;
         break;
      case ExpandStringLane::ahead :
         b = 4;
         break;
      case ExpandStringLane::right :
      case ExpandStringLane::sharp_right :
      case ExpandStringLane::uturn_right :
         b = 6;
         break;
      case ExpandStringLane::half_right :
         b = 5;
         break;

      case ExpandStringLane::preferred_lane :
      case ExpandStringLane::not_car_lane :
      case ExpandStringLane::opposite_direction :
         // Not a lane
         break;
   };
   b |= (l.getPreferred() << 7);
   b |= (l.getNotCar() << 6 );

   return b;
}

void colorTableIndex( IsabRouteList::colorTable_t* colortable, byte& color ) {
   GDUtils::Color::RGB rgb;
   GDUtils::Color::getRGB( GDUtils::Color::imageColor( color ), rgb );
   IsabRouteList::colorTable_t::const_iterator findIt = find( 
      colortable->begin(), colortable->end(), rgb );
   if ( findIt != colortable->end() ) {
      color = findIt - colortable->begin();
      mc2dbg8 << "   Using color index " << int(color) << " for " 
              << rgb << endl;
   } else {
      // Add
      color = colortable->size();
      colortable->push_back( rgb );
      mc2dbg8 << "  Adding color " << rgb << " to colortable" << endl;
   }
}

void
IsabRouteList::addLanesAndSignPosts( 
   const RouteElement* re, 
   NavStringTable* stringTable,
   uint32& totSize, int32& maxSize, 
   int& nbrStringsUsed,
   vector< IsabRouteElement* >& addElem,
   colorTable_t* colorTable,
   bool reminder,
   StringTable::languageCode lang ) const
{
   if ( m_reqVer >= 2 ) {
      // Lanes
      // Add only last
      uint32 l = re->getLanes().size() > 0 ? re->getLanes().size() - 1 : 0;
      if ( re->getTurnCode() == StringTable::DRIVE_FINALLY ) {
         // No need for lanes at goal, we don't know what lane the destination
         // is in.
         l = re->getLanes().size();
      }
      for ( ; l < re->getLanes().size() ; ++l ) {
         const ExpandStringLanes& el = re->getLanes()[ l ];
         mc2dbg2 << " Added Lanes Nbr lanes " << el.size() << " dist "
                 << el.getDist() << " stopOfLanes "
                 << StringUtility::booleanAsString( el.getStopOfLanes() )
                 << " reminder " << StringUtility::booleanAsString( reminder );
         // Four first lanes in Lane info
         uint8 liLanes[ 4 ] = { 0,0,0,0 };
         for ( uint32 ln = 0 ; ln < MIN( 4, el.size() ) ; ++ln ) {
            mc2dbg2 << " " << el[ ln ];
            liLanes[ ln ] = laneByte( el[ ln ], re );
         }
         LaneInfoPTElement* li = new LaneInfoPTElement( 
            el.getStopOfLanes(), reminder, el.size(), liLanes[ 0 ],
            liLanes[ 1 ], liLanes[ 2 ], liLanes[ 3 ], el.getDist() );
         addElem.push_back( li );
         totSize += isabBoxNavMessageUtil::navigatorStructSize;
         for ( uint32 ln = 4 ; ln < el.size() ; ln += 10 ) {
            // Add 10 lanes from ln 
            uint8 iLanes[ 10 ] = { 0,0,0,0,0, 0,0,0,0,0 };
            for ( uint32 i = 0 ; i < 10 && i + ln < el.size() ; ++i ) {
               mc2dbg2 << " " << el[ ln + i ];
               iLanes[ i ] = laneByte( el[ ln + i ], re );
            }
            LaneDataPTElement* ld = new LaneDataPTElement( 
               iLanes[ 0 ], iLanes[ 1 ], iLanes[ 2 ], iLanes[ 3 ], 
               iLanes[ 4 ], iLanes[ 5 ], iLanes[ 6 ], iLanes[ 7 ],
               iLanes[ 8 ], iLanes[ 9 ] );
            addElem.push_back( ld );
            totSize += isabBoxNavMessageUtil::navigatorStructSize;
         } // For all LaneData
         mc2dbg2 << endl;
      } // For all lane groups

      // SignPosts But not more than two for now (In importance order)
      for ( uint32 s = 0 ; s < re->getSignPosts().size() && s < 2; ++s ) {
         const ExpandStringSignPost& el = re->getSignPosts()[ s ];
         MC2String text;
         if ( el.getElementType() == ExpandStringSignPost::exit ) {
            text.append( StringTable::getString( StringTable::EXIT, lang ) );
            text.append( " " );
         }
         text.append( el.getText() );

         // Add string to stringTable->addString
         int strNbr = stringTable->addString( text.c_str() );
         nbrStringsUsed = int32(stringTable->getSize()) -1;
         byte textColor = el.getTextColor();
         byte frontColor = el.getFrontColor();
         byte backColor = el.getBackColor();
         if ( colorTable != NULL ) {
            colorTableIndex( colorTable, textColor );
            colorTableIndex( colorTable, frontColor );
            colorTableIndex( colorTable, backColor );
         }
         SignPostPTElement* sp = new SignPostPTElement( 
            strNbr, textColor, backColor, frontColor, el.getDist() );
         addElem.push_back( sp );
         totSize += isabBoxNavMessageUtil::navigatorStructSize;
         mc2dbg2 << " Added SignPost " << el << " with text " << text << endl;
      } // End for all signposts
   } // End if reqVer >= 2
}


/*
#ifdef undef_level_1
#   undef DEBUG_LEVEL_1
#endif
#ifdef undef_level_2
#   undef DEBUG_LEVEL_2
#endif
#ifdef undef_level_4
#   undef DEBUG_LEVEL_4
#endif
#ifdef undef_level_8
#   undef DEBUG_LEVEL_8
#endif
*/
