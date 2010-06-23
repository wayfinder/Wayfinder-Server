/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2UnitTestMain.h"

#include "TrafficHandler.h"
#include "TrafficIPC.h"
#include "DisturbanceElement.h"
#include "TrafficSituation.h"
#include "IDPairVector.h"
//#include "TrafficDataTypes.h"

#include "boost/lexical_cast.hpp"

#include "DeleteHelpers.h"
#include "STLUtility.h"

#include <iterator>
#include <algorithm>
#include <memory>

typedef TrafficIPC::Disturbances DisturbanceElements;

struct Clone {
   DisturbanceElement* operator()( const DisturbanceElement* other ) {
      return new DisturbanceElement( *other );
   }
};

struct SortElements {
   bool operator() ( const DisturbanceElement* lhs, const DisturbanceElement* rhs ) {
      return lhs->getSituationReference() < rhs->getSituationReference();
   }
};

void clone( const TrafficIPC::Disturbances& cloneThese,
            TrafficIPC::Disturbances& toMe ) {
   std::transform( cloneThese.begin(), cloneThese.end(),
                   std::back_inserter( toMe ), Clone() );
}

class TestTrafficIPC: public TrafficIPC {
public:

   TestTrafficIPC():m_fakeResult( false ) { }

   virtual ~TestTrafficIPC() {
      reset();
   }

   bool sendChangeset( const Disturbances& newElements,
                       const Disturbances& removedElements ) {
      reset();
      // clone the events
      clone( newElements, m_newElements );
      clone( removedElements, m_removedElements );

      return m_fakeResult;
   }

   bool sendPacketContainers( PacketContainers& pcs ) {
      //TODO Not done!
      return m_fakeResult;
   }

   void getMapIDsNodeIDsCoords( const TrafficSituation& traffSit,
                                IDPairsToCoords& idPairsToCoords ) {
      idPairsToCoords.insert( 
            make_pair( IDPair_t( 15, 0 ), // mapID, nodeID
                       make_pair( MC2Coordinate( 100, 200 ),  3 ) ) ); 
   }

   bool getAllDisturbances( const MC2String& provider,
                            Disturbances& disturbances ) {
      return false;
   }

   void reset() {
      STLUtility::deleteValues( m_newElements );
      m_newElements.clear();
      STLUtility::deleteValues( m_removedElements );
      m_removedElements.clear();
   }
     
   const Disturbances& getNewElements() const {
      return m_newElements;
   }

   const Disturbances& getRemovedElements() const {
      return m_removedElements;
   }

   /// Sets the return value for \c sendChangeSet
   void setFakeResult( bool shouldWeFakeIt ) {
      m_fakeResult = shouldWeFakeIt;
   }

private:
   Disturbances m_newElements;
   Disturbances m_removedElements;
   bool m_fakeResult;
};

void createTrafficSituations( const uint32 startIDNbr, 
                              const uint32 nbrOfSits,
                              TrafficHandler::SitCont& parsedSits ) {
   STLUtility::deleteValues( parsedSits );
   parsedSits.clear();
   for ( size_t i = startIDNbr; i < startIDNbr + nbrOfSits; ++i ) {
      parsedSits.push_back( 
            new TrafficSituation( boost::lexical_cast< MC2String >(i),
                                  "foo" ) );
      vector<pair<int32, int32> > coords;
      auto_ptr< TrafficSituationElement > traffElement( 
            new TrafficSituationElement( boost::lexical_cast< MC2String >(i),
                                         1218454200, // start time
                                         1918454200, // expiry time
                                         1218024613,  // creation time
                                         TrafficDataTypes::Accident, // type
                                         TrafficDataTypes::ACI,      // phrase
                                         TrafficDataTypes::Closed,   // severity
                                         "Chuck Norris on a rampage!",
                                         "EE3316185", // first location
                                         "EE3316184", // second location
                                         MAX_UINT16, // event code
                                         0, // extent
                                         TrafficDataTypes::Positive,
                                         0, // queue length
                                         coords ) ); // coords
                                         
      parsedSits.back()->addSituationElement( traffElement.release() );
   }
}

void createElements( const uint32 startIDNbr,
                     const uint32 nbrOfElems,
                     TrafficIPC::Disturbances& elems ) {
   STLUtility::deleteValues( elems );
   elems.clear();
   for ( size_t i = startIDNbr; i < startIDNbr + nbrOfElems; ++i ) {

      // This is needed since Tpeg based parsers have a severity factor
      TrafficDataTypes::severity severity = TrafficDataTypes::Closed;
      uint32 costFactor = TrafficDataTypes::getCostFactorFromSeverity( severity );
      if( costFactor != MAX_UINT32 ) {
         TrafficDataTypes::severity_factor severityFactor = 
            TrafficDataTypes::Unspecified;
            //trafficElem->getSeverityFactor();
         costFactor = (uint32)( (float)costFactor * 
               TrafficDataTypes::
               getCostFactorFromSeverityFactor( severityFactor ) );
      }

      elems.push_back( new DisturbanceElement( 
               MAX_UINT32, // DisturbanceID
               boost::lexical_cast< MC2String >(i), // situation reference
               TrafficDataTypes::Accident, // type
               TrafficDataTypes::ACI, // phrase
               MAX_UINT16, // event code
               1218454200, // start time
               1918454200, // expiry time
               1218024613,  // creation time
               severity, // severity
               TrafficDataTypes::Positive, // direction
               "EE3316185",
               "EE3316184",
               0, // extent
               costFactor, // cost Factor
               "Chuck Norris on a rampage!", // text
               0 // queue length
               ) );
      // add some fake route indexes
      elems.back()->addCoordinate( 0, // node ID
                                   100, // latitude
                                   200, // longitude
                                   0, // angle
                                   3 ); // route index
      elems.back()->setMapID( 15 );

      /*
      elems.back()->addCoordinate( 1, // node ID
                                   300, // latitude
                                   400, // longitude
                                   90, // angle
                                   4 ); // route index
                                   */
      //elems.push_back( new DisturbanceElement() );
      //elems.back()->setSituationReference( boost::lexical_cast< MC2String >(i) );
   }
}

void stripUnNeededElementData( DisturbanceElements& elements ) {
   for ( DisturbanceElements::iterator it = elements.begin();
         it != elements.end(); ++it ) {
      (*it)->stripData();
   }
}


void compareElements( TrafficIPC::Disturbances& newElems,
                      TrafficIPC::Disturbances& expectedElems ) {
   sort( newElems.begin(), newElems.end(), SortElements() );
   sort( expectedElems.begin(), expectedElems.end(), SortElements() );
   MC2_TEST_REQUIRED( newElems.size() == expectedElems.size() );
   for ( DisturbanceElements::size_type i = 0; i < newElems.size(); ++i ) {
      MC2_TEST_CHECK_EXT( *newElems[ i ] == *expectedElems[ i ], i );
   }
}

MC2_UNIT_TEST_FUNCTION( testTrafficHandler ) {
   TestTrafficIPC* ttipc = new TestTrafficIPC();
   ttipc->setFakeResult( true );

   TrafficHandler handler( "dummy", ttipc );

   TrafficHandler::SitCont parsedSits;
   createTrafficSituations( 0, 10, parsedSits );
   // processes situations with id 0,...,9
   // 0,...,9 are stored now
   handler.processSituations( parsedSits );

   DisturbanceElements newElems = ttipc->getNewElements();
   DisturbanceElements removedElems = ttipc->getRemovedElements();
   DisturbanceElements newElemsExpected;
   createElements( 0, 10, newElemsExpected );

   DisturbanceElements removedElemsExpected;
   compareElements( newElems, newElemsExpected );
   stripUnNeededElementData( removedElemsExpected );
   compareElements( removedElems, removedElemsExpected );
   
   createTrafficSituations( 4, 13, parsedSits );
   // processes situations with id from 4,...,16.
   // this will give us 0,1,2,3 need to be removed. 4 in total.
   // 10,11,...,15,16 will become new elements. 7 in total.
   // stored elements after this will be from 4,...,16
   handler.processSituations( parsedSits );

   createElements( 10, 7, newElemsExpected ); // 10,...,16
   createElements( 0, 4, removedElemsExpected ); // 0,...,3
   newElems = ttipc->getNewElements();
   removedElems = ttipc->getRemovedElements();

   compareElements( newElems, newElemsExpected );
   stripUnNeededElementData( removedElemsExpected );
   compareElements( removedElems, removedElemsExpected );

   
   createTrafficSituations( 7, 4, parsedSits );
   // processes situations with id from 7,8,9,10
   // this will give us 4,5,6,11,12,13,14,15,16 to be removed. 9 in total
   // 0 new elements
   // stored elements will after this be 7,8,9,10
   handler.processSituations( parsedSits );

   createElements( 0, 0, newElemsExpected ); // 0 
   createElements( 4, 3, removedElemsExpected ); // 4,5,6
   DisturbanceElements tmp;
   createElements( 11, 6, tmp );
   removedElemsExpected.insert( removedElemsExpected.end(), tmp.begin(), tmp.end() );
   tmp.clear();
   newElems = ttipc->getNewElements();
   removedElems = ttipc->getRemovedElements();
   
   compareElements( newElems, newElemsExpected );
   stripUnNeededElementData( removedElemsExpected );
   compareElements( removedElems, removedElemsExpected );

   createTrafficSituations( 3, 10, parsedSits );
   // processes situations with id from 3,4,5,6,7,8,9,10,11,12
   // this will give us 0 to remove
   // 3,4,5,6,11,12 will be new elements, 6 in total
   // stored elements will after this be 3,4,5,6,7,8,9,10,11,12
   handler.processSituations( parsedSits );

   createElements( 3, 4, newElemsExpected ); // 3,4,5,6
   createElements( 11, 2, tmp );
   newElemsExpected.insert( newElemsExpected.end(), tmp.begin(), tmp.end() );
   tmp.clear();
   createElements( 0, 0, removedElemsExpected ); // 0
   newElems = ttipc->getNewElements();
   removedElems = ttipc->getRemovedElements();
   
   compareElements( newElems, newElemsExpected );
   stripUnNeededElementData( removedElemsExpected );
   compareElements( removedElems, removedElemsExpected );

   STLUtility::deleteValues( parsedSits );
   STLUtility::deleteValues( newElemsExpected );
}
