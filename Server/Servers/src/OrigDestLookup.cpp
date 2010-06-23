/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <algorithm>
#include "OrigDestLookup.h"
#include "OrigDestInfo.h"
#include "Request.h"
#include "GfxConstants.h"
#include "RouteExpandItemPacket.h"

/**
 *    This class takes care of an OrigDestInfo and separates those that need
 *    more information about
 *
 *    * Coordinates from Item ID and Map ID
 *    * Item ID and Map ID from coordinates
 *    * Neither
 *
 *    Used by HandleOrigDestInfoList
 *
 */
class HandleOrigDestInfo{
public:
   HandleOrigDestInfo(  OrigDestLookup* thisOrigDestLookup ) {
      m_origDestLookup = thisOrigDestLookup;
   }
   void operator() ( OrigDestInfo& info ) {
      if ( info.getNodeID() == MAX_UINT32 ||
           info.getMapID()  == MAX_UINT32 ) {
         const byte itemTypes = byte(ItemTypes::streetSegmentItem);
         int key =
            m_origDestLookup->addCoordinate( info.getLat(),
                                             info.getLon(),
                                             0,    // All outdata types
                                             1,    // Only streetSegmentItem
                                             &itemTypes,
                                             info.getAngle() );
         m_origDestLookup->addIDRequestInfo( key, &info );
      } else if ( info.getLat() == GfxConstants::IMPOSSIBLE ) {
         m_origDestLookup->addCoordRequestInfo( info.getMapID(), &info );
      }
   }
private:
   OrigDestLookup* m_origDestLookup;
};

/**
 *    This class takes care of a list of OrigDestInfo as they enter
 *    OrigDestLookup. For each OrigDestInfo, HandleOrigDestInfo is called.
 *
 *    Used by Constructor of OrigDestLookup
 *
 */
class HandleOrigDestInfoList{
public:
   HandleOrigDestInfoList( OrigDestLookup* thisOrigDestLookup ) {
      m_origDestLookup = thisOrigDestLookup;
   }


   void operator() ( OrigDestInfoList* infoList ) {
      for_each( infoList->begin(),
                infoList->end(),
                HandleOrigDestInfo( m_origDestLookup ) );
   }
private:
   OrigDestLookup* m_origDestLookup;
};

void
OrigDestLookup::updateOutgoingPackets()
{
   PacketContainer* pc = m_coordinateObject.getNextPacket();
   while (pc != NULL) {
      addOutgoingPacket(pc);
      pc = m_coordinateObject.getNextPacket();
   }

   multimap<uint32, OrigDestInfo*>::iterator it =
      m_requestCoordList.begin();
   uint32 curMapID = MAX_UINT32;
   RouteExpandItemRequestPacket* pack = NULL;
   while ( it != m_requestCoordList.end() )
   {
      if  ( it->first != curMapID ) {
         if ( pack != NULL ) {
            addOutgoingPacket(
               new PacketContainer(pack, 0, 0, MODULE_TYPE_MAP ) );
         }
         curMapID = it->first;
         uint16 curPacketID = m_request->getNextPacketID();
         pack = new RouteExpandItemRequestPacket(curPacketID,
                                                 m_request->getID() );
      }
      pack->add( it->second->getNodeID(),
                 uint16(it->second->getOffset()) );
      it++;
   }
   if ( pack != NULL ) {
      addOutgoingPacket(
         new PacketContainer( pack, 0, 0, MODULE_TYPE_MAP ) );
   }
}
   
OrigDestLookup::OrigDestLookup(Request*              request,
                               OrigDestInfoList*     origInfoList,
                               const TopRegionRequest* topReq,
                               OrigDestInfoListList* viaInfoList,
                               OrigDestInfoList*     destInfoList) :
      m_request(request),
      m_outgoingQueue(),
      m_coordinateObject( request, topReq ),
      m_requestCoordList(),
      m_requestIDList()
{
   HandleOrigDestInfoList handleList( this );
   handleList(origInfoList);
   if ( destInfoList != NULL )
      handleList(destInfoList);
   if ( viaInfoList != NULL ) {
      for_each( viaInfoList->begin(),
                viaInfoList->end(),
                handleList );
   }
   updateOutgoingPackets();
   m_done = m_coordinateObject.getDone();
}
   
inline int
OrigDestLookup::addCoordinate(int32        lat,
                              int32        lon, 
                              uint32       outDataTypes,
                              byte         nbrAllowedItemTypes,
                              const byte*  itemTypes,
                              uint16       angle)
{
   return m_coordinateObject.addCoordinate(lat,
                                           lon,
                                           outDataTypes,
                                           nbrAllowedItemTypes,
                                           itemTypes,
                                           angle);
}

void
OrigDestLookup::handleIDReplyPacket(PacketContainer* pc)
{
   m_coordinateObject.processPacket(pc);
   PacketContainer* packCont = m_coordinateObject.getNextPacket();
   while (packCont != NULL) {
      addOutgoingPacket(pc);
      packCont = m_coordinateObject.getNextPacket();
   }

   if ( m_coordinateObject.getDone() == true )
   {
      CoordinateReplyPacket* getCoordinateReply(uint32 i = 0);
   }      
}

void
OrigDestLookup::processPacket( PacketContainer* p )
{
   if ( p == NULL ) {
      // PacketContainer == NULL indicates a timeout.
      m_status = StringTable::TIMEOUT_ERROR;
      return;
   }
   
   Packet* packet = p->getPacket();

   switch ( packet->getSubType() ) {
      case Packet::PACKETTYPE_COORDINATEREPLY:
         handleIDReplyPacket( p );
         break;
      default:
         mc2log << error << "OrigDestLookup got packet of type "
                << packet->getSubTypeAsString() << endl;
         m_status = StringTable::NOTOK;
         break;     
   }
   delete p;
}

PacketContainer*
OrigDestLookup::getNextPacket( )
{
   if (m_outgoingQueue.getMin() == NULL) {
      return NULL;
   } else {
      return m_outgoingQueue.extractMin();
   }
}
   
inline void
OrigDestLookup::addCoordRequestInfo( uint32        key,
                                     OrigDestInfo* info) {
   m_requestCoordList.insert( pair<uint32, OrigDestInfo*>( key, info ) );
}

inline void
OrigDestLookup::addIDRequestInfo( int           key,
                                  OrigDestInfo* info ) {
   m_requestIDList.insert( pair<int, OrigDestInfo*>( key, info ) );
}

inline void
OrigDestLookup::addOutgoingPacket( PacketContainer* p ) {
   m_outgoingQueue.add( p );
}

