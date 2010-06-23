/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SortDistRequest.h"
#include "STLUtility.h"
#include "GfxUtility.h"

SortDistRequest::SortDistRequest(uint16 reqID, 
                                 SortDistanceRequestPacket* sortPacket )
   : Request( reqID ), m_mapIDVect(16,16)
{
   // first add all maIDs.
   for( int i=0;i<sortPacket->getNbrItems(); i++ ){
      uint32 mapID, itemID;
      uint16 offset;
      sortPacket->getData( i, mapID, itemID, offset);      
      int index = m_mapIDVect.linearSearch(mapID);
      if( index == -1 ){
         m_mapIDVect.addLast(mapID);
      }
   }
   m_nbrPackets = m_mapIDVect.getSize();
   m_coordinatesRequestPackets = 
      new CoordinateOnItemRequestPacket*[m_nbrPackets];
   m_coordinatesReplyPackets = 
      new CoordinateOnItemReplyPacket*[m_nbrPackets];

   for( int i=0;i<m_nbrPackets;i++){
      m_coordinatesRequestPackets[i] = 
         new CoordinateOnItemRequestPacket(getNextPacketID(), reqID );
      m_coordinatesRequestPackets[i]->setMapID(m_mapIDVect.getElementAt(i));
      m_coordinatesReplyPackets[i] = NULL;
   }

   for( int  i=0;i<sortPacket->getNbrItems();i++){
      uint32 mapID, itemID;
      uint16 offset;
      sortPacket->getData( i, mapID, itemID, offset);      
      int index = m_mapIDVect.linearSearch( mapID );
      if( index >= 0 ) {
         m_coordinatesRequestPackets[index]->add( itemID, offset );
      }
   }
   m_sentPackets = 0;
   m_receivedPackets = 0;
}

SortDistRequest::~SortDistRequest()
{
   // The other ones is removed by the systems.
   delete m_coordinatesRequestPackets;
   for( int i=0;i<m_nbrPackets;i++){
      delete m_coordinatesReplyPackets[i];
   }

   delete [] m_coordinatesReplyPackets;
   m_mapIDVect.deleteAllObjs();
}

PacketContainer* SortDistRequest::getNextPacket()
{
   if( m_sentPackets < m_nbrPackets ){
      return new PacketContainer( m_coordinatesRequestPackets[m_sentPackets++], 0, 0, MODULE_TYPE_MAP );
   }
   else
      return NULL;
}

void SortDistRequest::processPacket( PacketContainer *packetContainer )
{
   if(packetContainer != NULL ){
      m_coordinatesReplyPackets[m_receivedPackets] = 
         (CoordinateOnItemReplyPacket*)packetContainer->getPacket();
      m_receivedPackets++;
   }
   if( m_receivedPackets == m_nbrPackets ) {
      m_done = true;
   }

}

PacketContainer* SortDistRequest::getAnswer()
{
   int originLat, originLon, destLat, destLon;
   uint32 itemID;
   // First coordinate is origin.
   m_coordinatesReplyPackets[0]->getLatLong( 0, itemID, originLat, originLon );
   SortDistanceReplyPacket* sortReply = new SortDistanceReplyPacket();
   int start = 1;
   vector < DistObj* > distVector;
   int k=0;
//Not origin   distVector.addLast( new DistObj( k++, m_mapIDVect[0], itemID, 0 ) );
   for( int i=0;i<m_nbrPackets;i++){
      for( uint32 j=start;j<m_coordinatesReplyPackets[i]->getNbrItems();j++){
         m_coordinatesReplyPackets[i]->getLatLong( j, itemID, destLat, destLon );
         double dist = GfxUtility::squareP2Pdistance_linear( originLat, originLon, destLat, destLon );
         int distance = (int)sqrt(dist);
         distVector.push_back( new DistObj( k++, m_mapIDVect[i], itemID, distance ) );
      }
      start = 0;
   }
   // Add to reply packet
   sort( distVector.begin(), distVector.end(), STLUtility::RefLess() );

   sortReply->setNbrItems( distVector.size() );
   for( uint32 i=0;i<distVector.size();i++){
      DistObj* distObj = distVector[i];
      sortReply->add( distObj->getIndex(), distObj->getMapID(), distObj->getItemID(), distObj->getDistance(), i );
   }

   STLUtility::deleteValues( distVector );

   return new PacketContainer( sortReply, 0, 0, MODULE_TYPE_INVALID );
}

////////////////////////////////////////////
// The temporary object to sort after distance
////////////////////////////////////////////

DistObj::DistObj( int index, uint32 mapID, uint32 itemID, uint32 distance )
{
   m_index = index;
   m_mapID = mapID;
   m_itemID = itemID;
   m_distance = distance;
}

