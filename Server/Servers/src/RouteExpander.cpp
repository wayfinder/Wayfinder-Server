/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "RouteExpander.h"
#include "ExpandRoutePacket.h"
#include "RoutePacket.h"
#include "IDTranslationPacket.h"
#include "TopRegionPacket.h"
#include "ExpandStringItem.h"
#include "ExpandItemID.h"
#include "SubRoute.h"
#include "SubRouteVector.h"
#include "Vehicle.h"
#include "DriverPref.h"
#include "DisturbanceDescription.h"
#include "LandmarkLink.h"
#include "LandmarkHead.h"
#include "MapBits.h"

RouteExpander::RouteExpander( Request* request,
                              RouteReplyPacket* routeReply,
                              uint32 expandRouteType,
                              bool uTurn,
                              bool noConcatinate,
                              uint32 passedRoads,
                              StringTable::languageCode language,
                              bool abbreviate,
                              bool landmarks,
                              bool removeAhead,
                              bool nameChangeAsWP)
      : m_request( request ), m_done( false ), m_answer( NULL ),
        m_nbrExpandRoutePackets( 0 ), m_nbrReceivedExpandPackets( 0 ),
        m_expandRouteReplyPackets( NULL ), m_routeReply( routeReply ),
        m_srVect( NULL ), m_startDirectionHousenumber( 
           ItemTypes::unknown_nbr_t ), 
        m_startDirectionOddEven( ItemTypes::unknown_oddeven_t ),
        m_expandRouteType( expandRouteType ), m_uTurn( uTurn ),
        m_noConcatinate( noConcatinate ), m_passedRoads( passedRoads ),
        m_removeNameChangeAhead(removeAhead),m_nameChangeAsWP(nameChangeAsWP),
        m_language( language ), m_abbreviate( abbreviate ),
        m_landmarks( landmarks ), 
        m_startOffset( routeReply->getStartOffset() ),
        m_endOffset( routeReply->getEndOffset() ), 
        m_startDir( 0 ), m_endDir( 0 ),
        m_nbrIDTransPackets( 0 ), m_idtransReplyPackets( NULL ),
        m_nbrReceivedIDTransPackets( 0 ), m_topregionPacket( NULL )
{
   // Create initial packets
   m_state = createIDTransPackets( m_routeReply, m_packetsReadyToSend );
   if ( m_state == TOP_REGION ) {
      m_state = createTopRegionPackets( m_packetsReadyToSend );
   }

   if ( m_state == ERROR || m_packetsReadyToSend.getCardinal() == 0 ) {
      m_state = ERROR;
      m_done = true;
   }
   m_useAddCost = false;
}


RouteExpander::RouteExpander( Request* request,
                              const SubRouteVector* srVect,
                              uint32 expandRouteType,
                              bool uTurn,
                              bool noConcatinate,
                              uint32 passedRoads,
                              StringTable::languageCode language,
                              bool abbreviate,
                              bool landmarks,
                              bool removeAhead,
                              bool nameChangeAsWP)
      : m_request( request ), m_done( false ), m_answer( NULL ),
        m_nbrExpandRoutePackets( 0 ), m_nbrReceivedExpandPackets( 0 ),
        m_expandRouteReplyPackets( NULL ), m_routeReply( NULL ),
        m_srVect( srVect ), m_startDirectionHousenumber( 
           ItemTypes::unknown_nbr_t ), 
        m_startDirectionOddEven( ItemTypes::unknown_oddeven_t ),
        m_expandRouteType( expandRouteType ), m_uTurn( uTurn ),
        m_noConcatinate( noConcatinate ), m_passedRoads( passedRoads ),
        m_removeNameChangeAhead(removeAhead), m_nameChangeAsWP(nameChangeAsWP),
        m_language( language ), m_abbreviate( abbreviate ),
        m_landmarks( landmarks ), m_startOffset( 0 ),
        m_endOffset( 0 ), m_startDir( 0 ), m_endDir( 0 ),
        m_nbrIDTransPackets( 0 ), m_idtransReplyPackets( NULL ),
        m_nbrReceivedIDTransPackets( 0 ), m_topregionPacket( 0 )
{
   // Create initial packets
   m_state = createIDTransPackets( m_srVect, m_packetsReadyToSend );
   if ( m_state == TOP_REGION ) {
      m_state = createTopRegionPackets( m_packetsReadyToSend );
   }
   if ( m_state == ERROR || m_packetsReadyToSend.getCardinal() == 0 ) {
      m_state = ERROR;
      m_done = true;
   }
   m_useAddCost = false;
}


RouteExpander::~RouteExpander()
{
   // Delete unsent packets
   PacketContainer* pc = m_packetsReadyToSend.getMin();
   while ( pc != NULL ) {
      m_packetsReadyToSend.remove( pc );
      delete pc;
      pc = m_packetsReadyToSend.getMin();
   }
   // Delete answer packets
   if ( m_expandRouteReplyPackets != NULL ) {
      for ( uint32 i = 0 ; i < m_nbrExpandRoutePackets ; ++i ) {
         delete m_expandRouteReplyPackets[ i ];
      }
      delete [] m_expandRouteReplyPackets;
   }
   // The answer
   // Old problem: The answer had to be deleted by user //delete m_answer;
   // The reply IDTransPacktes
   if ( m_idtransReplyPackets != NULL ) {
      for ( uint32 i = 0 ; i < m_nbrReceivedIDTransPackets ; ++i ) {
         delete m_idtransReplyPackets[ i ];
      }
      delete [] m_idtransReplyPackets;
   }
   // The topregionPacket
   delete m_topregionPacket;
   // The subroutevector deleted here as routeobject can't do it
   delete m_srVect;
}


void
RouteExpander::packetReceived( const PacketContainer* cont )
{
   mc2dbg8   << "RouteExpander::packetReceived  in state : " << int(m_state) 
          << ", type " << (int)cont->getPacket()->getSubType() << endl;
   if ( cont != NULL ) {
      switch( m_state ) {
         case IDTRANS:
            m_state = handleIDTransPack( cont );
            if ( m_state == ERROR ) {
               mc2log << warn << "RouteExpander::packetReceived "
                         "handleIDTransPack returned ERROR, aborting." << endl;
               m_done = true;
            }
            break;
         case TOP_REGION:
            m_state = handleTopRegionPack( cont );
            if ( m_state == ERROR ) {
               mc2log << warn << "RouteExpander::packetReceived "
                         "handleTopRegionPack returned ERROR, aborting." 
                      << endl;
               m_done = true;
            }
            break;
         case EXPAND:
            if ( cont->getPacket()->getSubType() ==
                 Packet::PACKETTYPE_EXPANDROUTEREPLY ) 
            {
               ExpandRouteReplyPacket* erp = static_cast<
                  ExpandRouteReplyPacket*> ( cont->getPacket() );
               uint32 ordinal = erp->getOrdinal();
               if ( ordinal < m_nbrExpandRoutePackets ) {
                  if ( m_expandRouteReplyPackets[ ordinal ] == NULL ) {
                     m_nbrReceivedExpandPackets++;
                     m_expandRouteReplyPackets[ ordinal ] = static_cast<
                        ExpandRouteReplyPacket*> ( erp->getClone() );
                     if ( m_nbrReceivedExpandPackets >= 
                          m_nbrExpandRoutePackets ) 
                     {
                        if ( concatenateExpandRoute() != NULL &&
                             static_cast<ReplyPacket*>( 
                                m_answer->getPacket() )->getStatus() == 
                             StringTable::OK )
                        {
                           m_state = DONE;
                           m_done = true;
                        } else {
                           m_state = ERROR;
                           m_done = true;
                           mc2log << warn << "RouteExpander::packetReceived "
                                  << " concatenateExpandRoute returned "
                                  << "NULL or not OK" 
                                  << " ordinal " << ordinal 
                                  << "  nbrExpandRoutePackets " 
                                  << m_nbrExpandRoutePackets
                                  << " in state EXPAND, aborting." << endl;
                        }
                     }
                  } else {
                     m_state = ERROR;
                     m_done = true;
                     mc2log << warn << "RouteExpander::packetReceived got "
                            << " duplicate reply for ordinal in expandreply " 
                            << ordinal 
                            << "  nbrExpandRoutePackets " 
                            << m_nbrExpandRoutePackets
                            << " in state EXPAND, aborting." << endl;
                  }
               } else {
                  m_state = ERROR;
                  m_done = true;
                  mc2log << warn << "RouteExpander::packetReceived got odd "
                         << "ordinal in expandreply " << ordinal 
                         << "  nbrExpandRoutePackets " << m_nbrExpandRoutePackets
                         << " in state EXPAND, aborting." << endl;
               }

            } else {
               m_state = ERROR;
               m_done = true;
               mc2log << warn << "RouteExpander::packetReceived got non "
                      << "EXPANDROUTEREPLY in state EXPAND, aborting." << endl;
            }
            break;
         case DONE:

            break;
         case ERROR:
            break;
      }
   }
}

   
PacketContainer*
RouteExpander::getNextRequest()
{
   // The packet to return
   PacketContainer* retPacketCont = m_packetsReadyToSend.getMin();
   if ( retPacketCont != NULL ) {
      m_packetsReadyToSend.remove( retPacketCont );
      mc2dbg8 << "[RE]::getNextRequest - "
           << retPacketCont->getPacket()->getSubType() << endl;
   } 
   
   return retPacketCont;
}


bool
RouteExpander::getDone()
{
   return m_done;
}


PacketContainer*
RouteExpander::createAnswer()
{
   return m_answer;
}

void
RouteExpander::addDisturbanceInfo(const vector<DisturbanceDescription>& dists)
{
   m_disturbanceInfo = dists;
   
}

void
RouteExpander::setUseAddCost(bool useAddCost)
{
   m_useAddCost = useAddCost;
}


ExpandRouteReplyPacket* 
RouteExpander::concatenateExpandRoute() {
   // Check if all packets are OK!
   // Calculate the total size too. The concatenated packet should
   // probably be smaller than the sum of the nonconcatenated ones.
   uint32 totalSize = 512; // For the check in the packet.
   for ( uint32 i = 0 ; i < m_nbrReceivedExpandPackets ; ++i ) {
      totalSize += m_expandRouteReplyPackets[ i ]->getLength();
      if ( m_expandRouteReplyPackets[ i ]->getStatus() != StringTable::OK )
      {
         ExpandRouteReplyPacket* reply = new ExpandRouteReplyPacket;
         reply->setStatus( m_expandRouteReplyPackets[ i ]->getStatus() );
         
         // Don't do anything more.
         mc2dbg1 << "RouteExpander::concatenateExpandRoute One packet (" 
                 << i << ") had status != OK" 
                 << endl;
         delete m_answer;
         m_answer = new PacketContainer( 
            reply, 0, 0, MODULE_TYPE_INVALID );
         return reply;
      }
      
   }
   
   // Packet to put the result into.
   ExpandRouteReplyPacket* result = new ExpandRouteReplyPacket( totalSize );
   // Arrays to put the temporary results into.
   ExpandStringItem*** stringItems = 
      new ExpandStringItem**[ m_nbrReceivedExpandPackets ];
   ExpandItemID** itemIDs = new ExpandItemID*[ m_nbrReceivedExpandPackets ];
   uint32* numStrings = new uint32[ m_nbrReceivedExpandPackets ];
   bool* bMerged = new bool[ m_nbrReceivedExpandPackets ];

   for( uint32 i = 0 ; i < m_nbrReceivedExpandPackets ; ++i ) {
      bMerged[ i ] = false;
   }
   
   // Per packet data
   uint32 totalStandStill = 0;
   uint32 totalTime       = 0;
   uint32 totalDist       = 0;

   // Add some data to the result
   result->setRouteType( m_expandRouteType );
   
   if ( m_nbrReceivedExpandPackets > 0 ) {
      result->setLastItemPosition(
         m_expandRouteReplyPackets[ 
            m_nbrReceivedExpandPackets - 1 ]->getLastItemLat(),
         m_expandRouteReplyPackets[
            m_nbrReceivedExpandPackets - 1 ]->getLastItemLon() );
      result->setStartDirectionHousenumber(
         m_expandRouteReplyPackets[ 0 ]->getStartDirectionHousenumber() );
      mc2dbg4 << "ExpRouteRepPack: Setting start housenumber to " 
              << int( result->getStartDirectionHousenumber() ) << endl;
      result->setStartDirectionOddEven(
         m_expandRouteReplyPackets[ 0 ]->getStartDirectionOddEven() );
      mc2dbg4 << "ExpRouteRepPack: Setting start odd/even to " 
              << int( result->getStartDirectionOddEven() ) << endl;
      m_startDirectionHousenumber = m_expandRouteReplyPackets[ 0 ]
         ->getStartDirectionHousenumber();
      m_startDirectionOddEven = m_expandRouteReplyPackets[ 0 ]
         ->getStartDirectionOddEven();
   }

   for( uint32 i = 0 ; i < m_nbrReceivedExpandPackets ; ++i ) {
      ExpandRouteReplyPacket* currPack = m_expandRouteReplyPackets[ i ];
      numStrings[ i ]  = currPack->getNumStringData();
      stringItems[ i ] = currPack->getStringDataItem();
      itemIDs[ i ]     = currPack->getItemID();
      // Add times and distances
      totalStandStill += currPack->getStandStillTime();
      totalTime       += currPack->getTotalTime();
      totalDist       += currPack->getTotalDist();
   }

   bool merged = false;
   
   uint32 i = 0;
   while ( i < m_nbrReceivedExpandPackets ) {
      uint32 j = 0;
      uint32 first_i = 0; 
      if ( merged ) {
         first_i = 1; // We have already put the first description 
                      // in the route
      } else {
         first_i = 0; // We have not merged
      }
      merged = false;
      
      for ( j = first_i ; j < numStrings[ i ] ; ++j ) {
         ExpandStringItem* si = stringItems[ i ][ j ];

         // Check if this is the last part in this packet.
         if ( j == numStrings[ i ] - 1 ) {
            bool done = false;
            uint32 sumDist = si->getDist();
            uint32 sumTime = si->getTime();
            uint32 leftExitCount = 0;
            uint32 rightExitCount = 0;
            LandmarkHead* landmarks = new LandmarkHead();
            ExpandStringLanesCont lanes;
            ExpandStringSignPosts signPosts;
            
            // Go through all packets and merge the strings so that
            // we don't get: "forward to E6" for each map we're passing
            // through.
            ExpandStringItem* si_next = NULL;
            while ( (i + 1) < m_nbrReceivedExpandPackets && !done ) {
               // Delete the previous one.
               delete si_next;
               // Get the first string in the next packet.
               si_next = stringItems[ i + 1 ][ 0 ];
               
               // Transfer any landmarks from si_next.
               if ( si_next->hasLandmarks() ) {
                  LandmarkLink* lmLink = static_cast<LandmarkLink*>
                     ( si_next->getLandmarks()->first() );
                  while ( lmLink != NULL ) {
                     mc2dbg8 << "inserting LM " << lmLink->getLMName() 
                             << " last, dist=" << lmLink->m_lmDist 
                             << "+" << sumDist << endl;
                     LandmarkLink* insertLink = lmLink;
                     insertLink->m_lmDist += sumDist;
                     lmLink = static_cast< LandmarkLink* >( lmLink->suc() );
                     insertLink->into( landmarks );
                  }
               }

               // Transfer any lanes from si_next
               for ( uint32 l = 0 ; l < si_next->getLanes().size() ; ++l ) {
                  ExpandStringLanes lg( si_next->getLanes()[ l ] );
                  lg.setDist( lg.getDist() + sumDist );
                  lanes.addLaneGroup( lg );
               }
               // Transfer any signposts from si_next
               for ( uint32 s = 0 ; s < si_next->getSignPosts().size() ; ++s ){
                  ExpandStringSignPost sg( si_next->getSignPosts()[ s ] );
                  sg.setDist( sg.getDist() + sumDist );
                  signPosts.addSignPost( sg );
               }
               
               // Add the distance to the new distance and merge
               merged = true;
               bMerged[ i + 1 ] = true;
               sumDist += si_next->getDist();   
               sumTime += si_next->getTime();
               
               // Increase left and right exit count.
               leftExitCount += m_expandRouteReplyPackets[i]->
                  getLastSegmentsLeftStreetCount();
               rightExitCount += m_expandRouteReplyPackets[i]->
                  getLastSegmentsRightStreetCount();

               // If this is the one and only string in this packet
               // then we continue with the next one, otherwise stop.
               if ( numStrings[ i + 1 ] != 1 ) {
                  // Next packet isn't hasn't just one string, which means
                  // it we shouldn't merge that one as well. Signal we're
                  // done.
                  done = true;
               } else {
                  // Delete the stringItems for packet i, and move on to
                  // the next packet.
                  delete [] stringItems[ i ];
                  stringItems[ i ] = NULL;
                  i++;
               }
            }
            if ( merged ) {

               // Have to decrease the GroupID in the next packet
               // Check if there is a next packet
               if ( i + 1 < m_nbrReceivedExpandPackets ) {
                  bMerged[ i + 1 ]=true;
               }
               
               uint32 exitCount = 0;
               // Calculate exit count. Use left or right count
               // depending on if the last turn is to the left or right.
               if ( si_next->getStringCode() == StringTable::LEFT_TURN ) {
                  exitCount = leftExitCount + si_next->getExitCount();
               } 
               else if ( si_next->getStringCode() == StringTable::RIGHT_TURN )
               {
                  exitCount = rightExitCount + si_next->getExitCount();
               }
               else if(si_next->getStringCode() ==
                       StringTable::EXIT_ROUNDABOUT_TURN ){
                  exitCount =  si_next->getExitCount();
               }
               
               

               // Transfer landmarks from the prior item (si), 
               if ( si->hasLandmarks() ) {
                  LandmarkLink* lmLink = static_cast<LandmarkLink*>
                     ( si->getLandmarks()->last() );
                  while ( lmLink != NULL ) {
                     LandmarkLink* insertLink = lmLink;
                     lmLink = static_cast< LandmarkLink* >( lmLink->pred() );
                     insertLink->intoAsFirst( landmarks );
                     mc2dbg8 << "inserting LM " << insertLink->getLMName() 
                             << " as first." << endl;
                  }
               }

               // Transfer any lanes from the prior item (si)
               for ( uint32 l = 0 ; l < si->getLanes().size() ; ++l ) {
                  lanes.addLaneGroup( si->getLanes()[ l ] );
               }
               // Transfer any signposts from the prior item (si)
               for ( uint32 s = 0 ; s < si->getSignPosts().size() ; ++s ){
                  signPosts.addSignPost( si->getSignPosts()[ s ] );
               }

               // Select which landmarks to keep in this merged route-part.
               selectLandmarks(landmarks, sumDist);
               // Add stringdata.
               result->setStringData ( si_next->getText(), 
                                       si_next->getStringCode(),
                                       sumDist, sumTime,
                                       si->getTransportationType(),
                                       si->getLatitude(),
                                       si->getLongitude(),
                                       si->getNameType(),
                                       exitCount,
                                       si_next->getCrossingKind(),
                                       si_next->getNbrPossTurns(),
                                       si_next->getPossibleTurns(),
                                       lanes,
                                       signPosts,
                                       landmarks );
               delete si_next;
               si_next = NULL;
            }
            delete landmarks;
         }
         if ( merged == false ) {
            result->setStringData( si->getText(),     si->getStringCode(),
                                   si->getDist(),
                                   si->getTime(),
                                   si->getTransportationType(),
                                   si->getLatitude(), si->getLongitude(),
                                   si->getNameType(), si->getExitCount(),
                                   si->getCrossingKind(),
                                   si->getNbrPossTurns(),
                                   si->getPossibleTurns(),
                                   si->getLanes(),
                                   si->getSignPosts(),
                                   si->getLandmarks() );
         }
         delete si;
         si = NULL;
      }
      // Done with packet nbr i, delete stringItems.
      delete [] stringItems[ i ];
      stringItems[ i ] = NULL;
      // Increase counter so that the next packet is processed.
      i++;
   } // End while i < m_nbrReceivedExpandPackets
   
   for ( uint32 i = 0; i < m_nbrReceivedExpandPackets; i++ ) {
      uint32 j = 0;
      ExpandItemID* ii = itemIDs[ i ];
      Vector& items = ii->getItemID();
      Vector& maps  = ii->getMapID();
      uint32 number = ii->getNbrItems();
      if ( (m_expandRouteType & ROUTE_TYPE_GFX_COORD) ) {
         IntVector& lon = ii->getLon();
         IntVector& lat = ii->getLat();
         for ( j = 0 ; j < number ; j++ ) {
            result->setItemData( maps[ j ], items[ j ], lat[ j ], lon[ j ] );
         }
      } else if ( (m_expandRouteType & ROUTE_TYPE_NAVIGATOR) ||
                  (m_expandRouteType & ROUTE_TYPE_ALL_COORDINATES) ||
                  (m_expandRouteType & ROUTE_TYPE_GFX_TRISS) ) 
      {
         IntVector& lon = ii->getLon();
         IntVector& lat = ii->getLat();
         Vector& speedLimit = ii->getSpeedLimit();
         Vector& attributes = ii->getAttributes();
         
         uint32 startIndex = 0;
         for ( j = 0 ; j < number ; j++ ) {
            // Should not be necessary now since we created the result
            // with a size
            result->updateSize(4 + 4 + 4, MAX_PACKET_SIZE / 2);
            uint32 nbrCoordPos = 
               result->setNavItemData( maps[ j ], 
                                       items[ j ], 
                                       speedLimit[ j ],
                                       attributes[ j ] );
            uint32 endIndex = ii->nextCoordStart( j );

            // Should not be necessary now since we created the result
            // with a size
            result->updateSize( (4 * 4) * (endIndex - startIndex),
                                MAX_PACKET_SIZE / 2 );
            for ( uint32 k = startIndex ; k < endIndex ; k++ ) {
               result->addCoordToNavItemData( lat[ k ], lon[ k ], 
                                              nbrCoordPos );
            }
            // Update startindex
            startIndex = endIndex;
         } 
      } else {
         for ( j = 0 ; j < number ; j++ ) {
            result->setItemData( maps[ j ], items[ j ] );
         }
      }
   } // End for i < m_nbrReceivedExpandPackets
   
   // Adding table for nbr of items per string
   if( m_expandRouteType & ROUTE_TYPE_ITEM_STRING ){
      uint32 curr = 0;
      uint32 count = 0;
      for ( i = 0 ; i < m_nbrReceivedExpandPackets ; i++ ) {
         Vector& group = itemIDs[ i ]->getGroupID();
         curr = group[ 0 ];
         if( !bMerged[ i ] ) {
            count = 1;
         } else {
            count++;
         }
         for ( uint32 j = 1 ; j < group.getSize() ; j++ ) {
            if ( group[ j ] == curr ) {
               count++;
            } else {
               result->addItemsPerString( count );
               count = 1;
               curr = group[ j ];
            }
         }
         if ( i == (m_nbrReceivedExpandPackets - 1) || !bMerged[ i + 1 ] ) {
            result->addItemsPerString( count );
         }
      }
   }
      
   for ( uint32 i = 0 ; i < m_nbrReceivedExpandPackets ; i++ ) {
      delete itemIDs[ i ];
   }
   
   result->setStandStillTime( totalStandStill );
   result->setTotalDist( totalDist );
   result->setTotalTime( totalTime );
   result->setStatus( StringTable::OK ); // XXX: This should be checked
   result->setStartOffset( m_startOffset );
   result->setEndOffset( m_endOffset );
   result->setStartDir( m_startDir );
   result->setEndDir( m_endDir );
   mc2dbg2 << "  Resulting ExpandRouteReplyPacket uses "
           << result->getLength() << " of " << result->getBufSize()
           << " bytes" << endl;
   
   // Delete the rest of things here
   delete [] stringItems;
   delete [] itemIDs;
   delete [] numStrings;
   delete [] bMerged;

   delete m_answer;
   m_answer = new PacketContainer( result, 0, 0, MODULE_TYPE_INVALID );
   return result;
}

void
RouteExpander::selectLandmarks(LandmarkHead* landmarks, uint32 routeLinkDist)
{
   mc2dbg4 << "selectLandmarks, nbr LMs " << landmarks->cardinal() << endl;

   // How many pass-LMs wanted
   // Rules from selectLandmarks in the ExpandRouteProcessor, overviewmap
   uint32 nbrWantedPassLMs;
   if (routeLinkDist < 50000)       // 50 km
      nbrWantedPassLMs = 1;
   else if (routeLinkDist < 200000) // 200 km
      nbrWantedPassLMs = 2;
   else if (routeLinkDist < 500000) // 500 km
      nbrWantedPassLMs = 3;
   else
      nbrWantedPassLMs = 4;
   
   // how many pass-LMs along this part of the route, per importance
   uint32 nbrPassLMs = 0;
   uint32 nbrPassLMsPerImportance[5];
   for (uint32 i = 0; i < 5; i++) {
      nbrPassLMsPerImportance[i] = 0;
   }
   LandmarkLink* lmLink = static_cast<LandmarkLink*>(landmarks->first());
   while (lmLink != NULL) {
      if( (lmLink->m_lmdesc.location == ItemTypes::pass) &&
          ! lmLink->isTraffic())
      {
         nbrPassLMs++;
         nbrPassLMsPerImportance[lmLink->m_lmdesc.importance]++;
      }
      lmLink = static_cast<LandmarkLink*>(lmLink->suc());
   }

   if (nbrPassLMs > nbrWantedPassLMs) {
      // which importance to keep
      uint32 nbr = 0;
      uint32 maxImportance = 0;
      bool cont = true;
      while (cont && (maxImportance < 5)) {
         nbr += nbrPassLMsPerImportance[maxImportance];
         if (nbr > nbrWantedPassLMs)
            cont = false;
         else 
            maxImportance++;
      }
      uint32 removeNbrFirstOfMaxImportance = 0;
      if (!cont) {
         removeNbrFirstOfMaxImportance = nbr - nbrWantedPassLMs;
      }

      mc2dbg4 << "Of " << nbrPassLMs << " pass-LMs, " << endl
              << " keeping all LMs of max importance " << maxImportance-1 
              << endl
              << " and the "
              << (nbrPassLMsPerImportance[maxImportance] - 
                                 removeNbrFirstOfMaxImportance)
              << " last LMs of importance " << maxImportance << endl;
      
      // remove all LMs that are not wanted
      LandmarkLink* lmLink = static_cast<LandmarkLink*>(landmarks->first());
      while (lmLink != NULL) {
         bool remove = false;
         if ((lmLink->m_lmdesc.location == ItemTypes::pass) &&
             ! lmLink->isTraffic())
         {
            uint32 importance = lmLink->m_lmdesc.importance;
            if ( importance > maxImportance )
               remove = true;
            else if ((importance == maxImportance) &&
                     (removeNbrFirstOfMaxImportance > 0)) {
               remove = true;
               removeNbrFirstOfMaxImportance--;
            }
            // else keep
         }

         if (remove) {
            mc2dbg4 << "Removing " << lmLink->getLMName() << " importance="
                    << lmLink->m_lmdesc.importance << endl;
            LandmarkLink* outLink = lmLink;
            lmLink = static_cast<LandmarkLink*>(outLink->suc());
            outLink->out();
            delete outLink; // Don't litter
         } else {
            lmLink = static_cast<LandmarkLink*>(lmLink->suc());
         }
      }
   }

   mc2dbg4 << "Remains " << landmarks->cardinal() << " landmarks" << endl;
}


RouteExpander::state 
RouteExpander::createIDTransPackets( 
   RouteReplyPacket* routeReply,
   PacketContainerTree& packetsReadyToSend )
{
   DriverPref driverPref;
   SubRouteVector* srVect = 
      routeReply->createSubRouteVector( &driverPref );
   state resState = createIDTransPackets( srVect, packetsReadyToSend );
   delete srVect;
   return resState;
}

RouteExpander::state 
RouteExpander::createIDTransPackets( 
   const SubRouteVector* srVect,
   PacketContainerTree& packetsReadyToSend )
{
   // Make vectors of IDPairs with overview items to lookup.
   uint32 mapID = MAX_UINT32;
   uint32 lastOverviewMapID  = MAX_UINT32;
   vector< IDPairVector_t* > nodeVectors;

   for( uint32 i = 0; i < srVect->getSize() ; ++i ) {
      const SubRoute* subRoute = srVect->getSubRouteAt(i);
      mapID = subRoute->getThisMapID();
      if ( MapBits::isOverviewMap( mapID ) ) {
         if ( mapID != lastOverviewMapID ) {
            // New IDPairVector
            IDPairVector_t* nodeVector = new IDPairVector_t;
            nodeVectors.push_back( nodeVector );

            // New overview, set last overview mapid
            lastOverviewMapID = mapID;
         }

         // Add first and last node
         if ( subRoute->size() > 0 ) { 
            nodeVectors.back()->push_back( 
               IDPair_t( mapID, subRoute->getNodeID( 0 ) ) );
            nodeVectors.back()->push_back( 
               IDPair_t( mapID, 
                         subRoute->getNodeID( subRoute->size() - 1 ) ) );

         } else {
            nodeVectors.back()->push_back( IDPair_t( 
               mapID, subRoute->getOrigNodeID() ) );
            nodeVectors.back()->push_back( IDPair_t( 
               mapID, subRoute->getDestNodeID() ) );
         }
      }
   }

   for ( uint32 i = 0 ; i < nodeVectors.size() ; ++i ) {
      IDTranslationRequestPacket* IDTransPack = 
         new IDTranslationRequestPacket(
            m_request->getNextPacketID(), m_request->getID(), 
            (*nodeVectors[ i ])[ 0 ].getMapID(),
            true,/*to lower*/
            &nodeVectors[ i ], 1,
            (*nodeVectors[ i ])[ 0 ].getMapID() );
      m_nbrIDTransPackets++;
      packetsReadyToSend.add( 
         new PacketContainer( IDTransPack, 0, 0, MODULE_TYPE_MAP ) );
      delete nodeVectors[ i ];
   }

   if ( m_nbrIDTransPackets > 0 ) {
      // Make reply vector 
      m_idtransReplyPackets = new IDTranslationReplyPacket*[ 
         m_nbrIDTransPackets ];
      return IDTRANS;
   } else {
      // Nothing to look up
      return TOP_REGION;
   }
}


RouteExpander::state 
RouteExpander::createExpandRoutePackets( 
   RouteReplyPacket* routeReply, 
   IDTranslationReplyPacket** idtransReplyPackets, 
   uint32 nbrIDTransPackets, TopRegionReplyPacket* topregionPacket,
   byte expandRouteType, uint16& startOffset, 
   uint16& endOffset, bool& startDir, bool& endDir, 
   PacketContainerTree& addTree )
{
   uint32 mapID      = MAX_UINT32;
   uint32 nodeID     = MAX_UINT32;
   uint32 currMapID  = MAX_UINT32;
   uint32 lastMapID  = MAX_UINT32;
   uint32 lastNodeID = MAX_UINT32;
   uint32 lastCountryID = MAX_UINT32;
   ExpandRouteRequestPacket* currRequest = NULL;
   ExpandRouteRequestPacket* firstRequest = NULL;
   // To keep track of the state changes
   uint32 lastStateElement = 0;

   // Top region data
   TopRegionMatchesVector topRegions;

   // ID Trans map. OverviewMap and start and end IDPair(real mapID and nodeid)
   map<uint32, IDPairVector_t> IDTrans;
   // Packets added, just local copy to use in method.
   typedef vector< ExpandRouteRequestPacket* > expandRoutePacketsVector;
   expandRoutePacketsVector expandRoutePackets;
   
   // Get Top Region data
   topregionPacket->getTopRegions( topRegions );

   // Fill in ID Trans map.
   for ( uint32 i = 0 ; i < nbrIDTransPackets ; ++i ) {
      IDPairVector_t outVector;
      idtransReplyPackets[ i ]->getTranslatedNodes( 0, outVector );
      IDTrans.insert( make_pair( 
         idtransReplyPackets[ i ]->getUserDefinedData(),/*mapID*/
         outVector ) );
   }

   uint32 rrp_nbrItems = routeReply->getNbrItems();
   
   for( uint32 i = 0 ; i < rrp_nbrItems ; i++ ) {
      lastNodeID = nodeID;
      routeReply->getRouteItem(i, mapID, nodeID);

      // Check if the node was a state element.
      if ( GET_TRANSPORTATION_STATE(nodeID) != 0 )
         lastStateElement = nodeID;
      
      if ( currMapID != mapID || currRequest == NULL ) {
         currRequest = new ExpandRouteRequestPacket();

         if(m_noConcatinate){
            currRequest->setNoConcatinate(true);
         }

         currRequest->setNbrPassStreets( m_passedRoads );
         currRequest->setRemoveAheadIfNameDiffer( m_removeNameChangeAhead );
         currRequest->nameChangeAsTurn( m_nameChangeAsWP );
            
         if ( firstRequest == NULL ) {
            firstRequest = currRequest;
         }
         currRequest->setUturn( m_uTurn );
         currRequest->setAbbreviate( m_abbreviate );
         currRequest->setLanguage( m_language );
         currRequest->setIncludeLandmarks( m_landmarks );
         currRequest->setUseAddCost( m_useAddCost );
         
         // Set MapID
         currRequest->setMapID( mapID );
         currRequest->setOrdinal( m_nbrExpandRoutePackets );
         currRequest->setType( expandRouteType );
         // Zero the parameters
         currRequest->setIgnoreEndOffset( true );
         currRequest->setIgnoreStartOffset( true );
         // Set previous country
         currRequest->setLastCountry( lastCountryID );
         // Get last country for this 
         uint32 countryMapID = mapID;
         if ( MapBits::isOverviewMap( countryMapID ) ) {
            // Use IDTrans map
            map<uint32, IDPairVector_t>::const_iterator it = IDTrans.find( 
               countryMapID );
            if ( it != IDTrans.end() ) {
               countryMapID = it->second[ 1 ].first;
            } else {
               mc2log << error << "RouteExpander::createExpandRoutePackets "
                      << "IDTranslation map doesn't contain map: 0x" << hex
                      << countryMapID << dec << endl;
            }
         }
         uint32 tmpLastCountryID = getCountryID( countryMapID, topRegions );
         if ( tmpLastCountryID != StringTable::NBR_COUNTRY_CODES ) {
            lastCountryID = tmpLastCountryID;
         } else {
            mc2log << error << "RouteExpander-createExpandRoutePackets "
                   << "getCountryID failed not updating lastCountryID!" 
                   << endl;
         }


         // Add state element if there is one and the first
         // node isn't this element already.
         if ( lastStateElement != 0 &&
              lastStateElement != nodeID ) {
            currRequest->addNodeID( lastStateElement );
         }

         // Save stuff for next round
         lastMapID = currMapID;        
         currMapID = mapID;
         currRequest->setLastID( lastNodeID );
         currRequest->setLastMapID( lastMapID );
         m_nbrExpandRoutePackets++;
         currRequest->setPacketID( m_request->getNextPacketID() );
         currRequest->setRequestID( m_request->getID() );
         addTree.add( new PacketContainer( currRequest, 0, 0, 
                                           MODULE_TYPE_MAP ) );
         expandRoutePackets.push_back( currRequest );
      }
      // Add node id.
      currRequest->addNodeID( nodeID );
   }

   for ( TopRegionMatchesVector::iterator it = topRegions.begin() ;
         it != topRegions.end() ; ++it )
   {
      delete *it;
   }

   // Save landmarks/disturbances last so they don't get overwritten
   if ( m_landmarks ) {
      for ( expandRoutePacketsVector::iterator it = 
               expandRoutePackets.begin() ; it != expandRoutePackets.end()
               ; ++it ) {
            (*it)->addTrafficInfo( m_disturbanceInfo );
      }
   }

   if ( m_nbrExpandRoutePackets > 0 ) {
      ExpandRouteRequestPacket* lastRequest = currRequest;
      // Fix the first packet
      currRequest = firstRequest;
      currRequest->setStartOffset( startOffset );
      currRequest->setUturn( m_uTurn );
      currRequest->setAbbreviate( m_abbreviate );
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
/* Test for GCC > 3.2.0 */
#if GCC_VERSION > 30200
      currRequest->setStartDir( routeReply->getStartDir() );
      currRequest->setIgnoreStartOffset( false );
#else
      {int pos = REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE;
      byte b = currRequest->incReadByte( pos );
      if ( routeReply->getStartDir() ) {
         b |= 0x80;
      } else {
         b &= 0x7f;
      }
      b &= 0xef;
      pos = REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE;
      currRequest->incWriteByte( pos, b );}
#endif

      // Fix the last packet
      currRequest = lastRequest;
      currRequest->setEndOffset( endOffset );
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
/* Test for GCC > 3.2.0 */
#if GCC_VERSION > 30200
      currRequest->setEndDir( routeReply->getEndDir() );
      currRequest->setIgnoreEndOffset( false );
#else 
      {int pos = REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE;
      byte b = currRequest->incReadByte( pos );
      if ( routeReply->getEndDir() ) {
         b |= 0x40;
      } else {
         b &= 0xbf;
      }
      b &= 0xdf;
      pos = REPLY_HEADER_SIZE + EXPAND_REQ_IGNORE;
      currRequest->incWriteByte( pos, b );}
#endif
      // Create a vector to store the replies in
      m_expandRouteReplyPackets = new ExpandRouteReplyPacket*[
         m_nbrExpandRoutePackets ];
      for ( uint32 i = 0 ; i < m_nbrExpandRoutePackets ; ++i ) {
         m_expandRouteReplyPackets[ i ] = NULL;
      }
      
      return EXPAND;
   } else {
      mc2log << warn << "RouteExpander::createExpandRoutePackets generated"
             << " 0 packets" << endl;
      return ERROR;
   }
}


RouteExpander::state 
RouteExpander::createExpandRoutePackets( 
   const SubRouteVector* srVect, 
   IDTranslationReplyPacket** idtransReplyPackets, 
   uint32 nbrIDTransPackets, TopRegionReplyPacket* topregionPacket,
   byte expandRouteType, uint16& startOffset, 
   uint16& endOffset, bool& startDir, bool& endDir, 
   PacketContainerTree& addTree )
{
   // Create a vector to store the replies in
   m_nbrExpandRoutePackets = srVect->getSize();
   m_expandRouteReplyPackets = new ExpandRouteReplyPacket*[
      m_nbrExpandRoutePackets ];
   for ( uint32 i = 0 ; i < m_nbrExpandRoutePackets ; ++i ) {
      m_expandRouteReplyPackets[ i ] = NULL;
   }

   // Local variables
   ExpandRouteRequestPacket* currRequest = NULL;
   ExpandRouteRequestPacket* firstRequest = NULL;
   uint32 prevMapID  = MAX_UINT32;
   uint32 prevNodeID = MAX_UINT32;
   uint32 lastCountryID = MAX_UINT32;
   // Top region data
   TopRegionMatchesVector topRegions;
   // ID Trans map. OverviewMap and start and end IDPair(real mapID and nodeid)
   map<uint32, IDPairVector_t> IDTrans;
   

   startOffset = srVect->getStartOffsetUint16();
   endOffset = srVect->getEndOffsetUint16();

   // Q: Why must we invert the startdir? Can not be sane!!!!
   // A: Difference FromZero vs. Towards0 confusing? YES!
   startDir = !srVect->getStartDirFromZero();
   endDir = srVect->getEndDirFromZero();
   
   
   // Get Top Region data and whole id tree.
   ItemIDTree wholeTree;
   topregionPacket->getTopRegionsAndIDTree( topRegions, wholeTree );

   // Fill in ID Trans map.
   for ( uint32 i = 0 ; i < nbrIDTransPackets ; ++i ) {
      IDPairVector_t outVector;
      idtransReplyPackets[ i ]->getTranslatedNodes( 0, outVector );
      IDTrans.insert( make_pair( 
         idtransReplyPackets[ i ]->getUserDefinedData(),/*mapID*/
         outVector ) );
   }

   for ( uint32 i = 0 ; i < m_nbrExpandRoutePackets ; i++ ) {
      const SubRoute* subRoute = srVect->getSubRouteAt( i );
      currRequest = new ExpandRouteRequestPacket;
      if(m_noConcatinate){
         currRequest->setNoConcatinate(true);
      }
      currRequest->setNbrPassStreets( m_passedRoads );
      currRequest->setRemoveAheadIfNameDiffer( m_removeNameChangeAhead );
      currRequest->nameChangeAsTurn( m_nameChangeAsWP );
      
      if ( firstRequest == NULL ) {
         firstRequest = currRequest;
      }
      currRequest->setUturn( m_uTurn );
      currRequest->setAbbreviate( m_abbreviate );
      currRequest->setLanguage( m_language );
      currRequest->setIncludeLandmarks( m_landmarks );
      currRequest->setUseAddCost( m_useAddCost );
      currRequest->setMapID( subRoute->getThisMapID() );
      currRequest->setOrdinal( i );
      currRequest->setType( expandRouteType );
      // Ignore end offset on all but the last.
      currRequest->setIgnoreEndOffset( i != m_nbrExpandRoutePackets - 1 );
      // Ignore start offset on all but the first.
      currRequest->setIgnoreStartOffset( i != 0 );
      currRequest->setLastID( prevNodeID );
      currRequest->setLastMapID( prevMapID );

      // Set previous country
      currRequest->setLastCountry( lastCountryID );
      // Get last country for this 
      uint32 countryMapID = subRoute->getThisMapID();
      if ( MapBits::isOverviewMap( countryMapID ) ) {
         // Use IDTrans map
         map<uint32, IDPairVector_t>::const_iterator it = IDTrans.find( 
            countryMapID );
         if ( it != IDTrans.end() ) {
            countryMapID = it->second[ 1 ].first;
         } else {
            mc2log << error << "RouteExpander::createExpandRoutePackets "
                   << "IDTranslation map doesn't contain map: 0x" << hex
                   << countryMapID << dec << endl;
         }
         // Make country lookup table 
         map<uint32,uint32> mapID2CountryID;
         
         vector<uint32> lowerMaps;
         wholeTree.getLowerMapsFor( subRoute->getThisMapID(), lowerMaps );
         if ( lowerMaps.empty() ) {
            mc2log << warn << "Couldn't find lower maps for "
                   << hex << "0x" << subRoute->getThisMapID() << dec 
                   << endl;
         }
         for ( TopRegionMatchesVector::const_iterator it = 
                  topRegions.begin() ; it != topRegions.end() ; ++it )
         {
            if ( (*it)->getType() == TopRegionMatch::country ) {
               const ItemIDTree& itemIDTree = (*it)->getItemIDTree();
               for ( vector<uint32>::const_iterator mapIt = 
                        lowerMaps.begin() ; mapIt != lowerMaps.end() ;
                     ++mapIt )
               {
                  if ( itemIDTree.containsMap( *mapIt ) ) {
                     mapID2CountryID.insert( 
                        make_pair(*mapIt, (*it)->getID() ) );
                  }
               }
            }
         }
         currRequest->setMapIDToCountryID( mapID2CountryID );
      }
      uint32 tmpLastCountryID = getCountryID( countryMapID, topRegions );
      if ( tmpLastCountryID != StringTable::NBR_COUNTRY_CODES ) {
         lastCountryID = tmpLastCountryID;
      } else {
         mc2log << error << "RouteExpander::createExpandRoutePackets "
                << "getCountryID failed not updating lastCountryID!" 
                << endl;
      }

      // Update last mapID
      prevMapID = subRoute->getThisMapID();

      if ( subRoute->size() > 0 ) { 
         prevNodeID = subRoute->getNodeID( subRoute->size() - 1 );
      } else {
         // This can happen if there are only two nodes on the route
         // i.e. orig and dest.
         prevNodeID = subRoute->getOrigNodeID();
      }

      // Insert state element if the vehicle is something else
      // than a car.
      // This could be removed later if we send the SubRoutes
      // to the MapModule instead of doing all these tricks.
      switch ( subRoute->getOrigVehicle()->getVehicleMask() ) {
         case ItemTypes::pedestrian:
            currRequest->addNodeID( 0xf0000002 );
            break;
         case ItemTypes::bicycle:
            currRequest->addNodeID( 0xf0000003 );
            break;
      }
      
      currRequest->addNodeID( subRoute->getOrigNodeID() );
      for( uint32 j = 0 ; j < subRoute->size() ; ++j ) {
         // Add node id.
         currRequest->addNodeID( subRoute->getNodeID( j ) );
      }
      
      // Add destination too if it is on the same map.
      if (subRoute->hasOneMap() ) {
         currRequest->addNodeID( subRoute->getDestNodeID() );
      }
      
      if(m_landmarks){
            currRequest->addTrafficInfo(m_disturbanceInfo);
      }

      currRequest->setPacketID( m_request->getNextPacketID() );
      currRequest->setRequestID( m_request->getID() );
      addTree.add( new PacketContainer( currRequest, 0, 0, 
                                        MODULE_TYPE_MAP ) );
   }

   for ( TopRegionMatchesVector::iterator it = topRegions.begin() ;
         it != topRegions.end() ; ++it )
   {
      delete *it;
   }
   
   if ( m_nbrExpandRoutePackets > 0 ) {
      ExpandRouteRequestPacket* lastRequest = currRequest;
      // Fix the first packet
      currRequest = firstRequest;
      currRequest->setStartOffset( startOffset );
      currRequest->setStartDir( startDir );
      currRequest->setUturn( m_uTurn );
      currRequest->setAbbreviate( m_abbreviate );
      currRequest->setIncludeLandmarks( m_landmarks );
      currRequest->setUseAddCost( m_useAddCost );
      currRequest->setIgnoreStartOffset( false );
      // Fix the last packet
      currRequest = lastRequest;
      currRequest->setEndOffset( endOffset );
      currRequest->setEndDir(  endDir );
      currRequest->setIgnoreEndOffset( false );

      return EXPAND;
   } else {
      mc2log << warn << "RouteExpander::createExpandRoutePackets generated"
             << " 0 packets" << endl;
      return ERROR;
   }
}


RouteExpander::state 
RouteExpander::createTopRegionPackets( 
   PacketContainerTree& packetsReadyToSend ) const
{
   packetsReadyToSend.add( 
      new PacketContainer( new TopRegionRequestPacket( 
         m_request->getID(), m_request->getNextPacketID() ), 0, 0, 
         MODULE_TYPE_MAP ) );
   return TOP_REGION;
}


RouteExpander::state 
RouteExpander::handleIDTransPack( const PacketContainer* cont ) {
   if ( cont->getPacket()->getSubType() ==
        Packet::PACKETTYPE_IDTRANSLATIONREPLY )
   {
      if ( static_cast< ReplyPacket* > ( cont->getPacket() )->getStatus() ==
           StringTable::OK )
      {
         m_idtransReplyPackets[ m_nbrReceivedIDTransPackets++ ] = static_cast<
            IDTranslationReplyPacket*> ( cont->getPacket()->getClone() );
         if ( m_nbrReceivedIDTransPackets >= m_nbrIDTransPackets ) {
            return createTopRegionPackets( m_packetsReadyToSend );
         } else {
            // More to do here
            return IDTRANS;
         }
      } else {
         mc2log << warn << "RouteExpander::handleIDTransPack "
                << "IDTranslationReplyPacket not ok: " 
                << StringTable::getString( 
                   StringTable::stringCode( 
                      static_cast< ReplyPacket* > ( 
                         cont->getPacket() )->getStatus() ),
                   StringTable::ENGLISH ) << endl;
         return ERROR;
      }
   } else {
      mc2log << warn << "RouteExpander::handleIDTransPack not a "
                "IDTranslationReplyPacket!" << endl;
      return ERROR;
   }
}


RouteExpander::state 
RouteExpander::handleTopRegionPack( const PacketContainer* cont ) {
   if ( cont->getPacket()->getSubType() ==
        Packet::PACKETTYPE_TOPREGIONREPLY )
   {
      if ( static_cast< ReplyPacket* > ( cont->getPacket() )->getStatus() ==
           StringTable::OK )
      {
         m_topregionPacket = static_cast<
            TopRegionReplyPacket*> ( cont->getPacket()->getClone() );
         // Make expand route packets.
         if ( m_routeReply != NULL ) {
            return createExpandRoutePackets( 
               m_routeReply, m_idtransReplyPackets, m_nbrIDTransPackets,
               m_topregionPacket, m_expandRouteType, m_startOffset, 
               m_endOffset, m_startDir, m_endDir, m_packetsReadyToSend );
         } else {
            return createExpandRoutePackets( 
               m_srVect, m_idtransReplyPackets, m_nbrIDTransPackets,
               m_topregionPacket, m_expandRouteType, m_startOffset, 
               m_endOffset, m_startDir, m_endDir, m_packetsReadyToSend );
         }
      } else {
         mc2log << warn << "RouteExpander::handleTopRegionPack "
                << "TopRegionReplyPacket not ok: " 
                << StringTable::getString( 
                   StringTable::stringCode( 
                      static_cast< ReplyPacket* > ( 
                         cont->getPacket() )->getStatus() ),
                   StringTable::ENGLISH ) << endl;
         return ERROR;
      }
   } else {
      mc2log << warn << "RouteExpander::handleTopRegionPack not a "
                "TopRegionReplyPacket!" << endl;
      return ERROR; 
   }
}


uint32 
RouteExpander::getCountryID( uint32 countryMapID, 
                             TopRegionMatchesVector& topRegions ) const
{
   // Check all top region for a country containing countryMapID
   uint32 countryID = StringTable::NBR_COUNTRY_CODES;

   TopRegionMatchesVector::const_iterator it = topRegions.begin();
   
   while ( it != topRegions.end() && 
           countryID == StringTable::NBR_COUNTRY_CODES )
   {
      // Look for countryMapID in this Top region.
      if ( (*it)->getType() == TopRegionMatch::country ) {
         const ItemIDTree& idTree = (*it)->getItemIDTree();
         set<uint32> mapIDs;
         idTree.getLowestLevelMapIDs( mapIDs );
         // Look for countryMapID in mapIDs
         set<uint32>::const_iterator mapIt = mapIDs.find( countryMapID );
         if ( mapIt != mapIDs.end() ) {
            countryID = (*it)->getID(); 
         }
      } // Not a country
      ++it;
   }

   return countryID;
}
