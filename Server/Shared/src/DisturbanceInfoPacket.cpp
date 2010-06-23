/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DisturbanceInfoPacket.h"
#include "Packet.h"
#include "UserRightsMapInfo.h"
#include "DisturbanceDescription.h"
#include "StringTable.h"

//
// ====================== Disturbance Ifot =====================
//

namespace {
class DisturbanceInfo {
   public :

      DisturbanceInfo(TrafficDataTypes::disturbanceType type,
                      MC2String comment);
   ~DisturbanceInfo();

   
   TrafficDataTypes::disturbanceType getType() const
      {return m_type; }
   

   MC2String getComment() const
      { return m_comment; }
   

   // Fields
   TrafficDataTypes::disturbanceType m_type;
   MC2String m_comment;
  private:
   DisturbanceInfo() { }
   
};




DisturbanceInfo::DisturbanceInfo(TrafficDataTypes::disturbanceType type,
                                 MC2String comment)
      : m_type(type), m_comment(comment)
{}

DisturbanceInfo::~DisturbanceInfo()
{}
}

//
// ====================== Request Packet =====================
// 

DisturbanceInfoRequestPacket::DisturbanceInfoRequestPacket(
   const UserUser* user, uint32 mapID, uint32 originIP, uint16 originPort)
      : RequestPacket( MAX_PACKET_SIZE,
                       DISTURBANCE_REQUEST_PRIO,
                       Packet::PACKETTYPE_DISTURBANCEINFOREQUEST,
                       0,
                       0,
                       MAX_UINT32 )
{
   setOriginIP(originIP);
   setOriginPort(originPort);
   setMapID( mapID );
   
   // Rights should maybe be sent in from the beginning.
   MapRights mask( MapRights::TRAFFIC_AND_SPEEDCAM );
   UserRightsMapInfo rights( mapID, user, mask ); 

   int pos = USER_RIGHTS_POS;
   rights.save( this, pos );

   // UserRights done lets write where the nodes begin.
   writeLong( START_OF_NODES_POS, pos );
   
   // FIXME: Write enough data for UserUser
   setLength( pos );
}

DisturbanceInfoRequestPacket::~DisturbanceInfoRequestPacket()
{
   
}

uint32
DisturbanceInfoRequestPacket::getFirstNodePosition() const {
   return readLong(START_OF_NODES_POS);
}

int
DisturbanceInfoRequestPacket::getRights( UserRightsMapInfo& rights ) const
{
   int pos = USER_RIGHTS_POS;
   return rights.load( this, pos );
}


void
DisturbanceInfoRequestPacket::addNode(uint32 mapID, uint32 nodeID)
{
   int nbrNodes  = getNumberOfNodes();
   int pos       = nbrNodes*8 + getFirstNodePosition();
   incWriteLong( pos, mapID);
   incWriteLong( pos, nodeID);
   writeShort(NUMBER_OF_NODES, nbrNodes+1);
   setLength(pos);
}

void
DisturbanceInfoRequestPacket::addNodes(vector<IDPair_t> nodes)
{
   int nbrNodes  = getNumberOfNodes();
   int pos       = nbrNodes*8 + getFirstNodePosition();
   vector<IDPair_t>::iterator vi;
   for(vi = nodes.begin(); vi != nodes.end(); vi++){
      incWriteLong( pos, (*vi).getMapID());
      incWriteLong( pos, (*vi).getItemID());
      nbrNodes++;
   }
   setLength(pos);
}

void
DisturbanceInfoRequestPacket::addDisturbanceDescription(
   const DisturbanceDescription& dist)
{
   int nbrNodes  = getNumberOfNodes();
   int pos       = nbrNodes*8 + getFirstNodePosition();
   incWriteLong( pos, dist.m_distPoint.getMapID());
   incWriteLong( pos, dist.m_distPoint.getItemID());
   writeShort(NUMBER_OF_NODES, nbrNodes+1);
   setLength(pos);
}

void
DisturbanceInfoRequestPacket::addDisturbanceVector(
   const vector<DisturbanceDescription>& distVect)
{
   int nbrNodes  = getNumberOfNodes();
   int pos       = nbrNodes*8 + getFirstNodePosition();
   vector<DisturbanceDescription>::const_iterator vi;
   for(vi = distVect.begin(); vi != distVect.end(); vi++){
      incWriteLong( pos, (*vi).m_distPoint.getMapID());
      incWriteLong( pos, (*vi).m_distPoint.getItemID());
      nbrNodes++;
      mc2dbg << " XXXXXXXXXXXXXXXxx add node : "
           << (*vi).m_distPoint.getMapID() << "." 
           <<(*vi).m_distPoint.getItemID() << endl;
   }
   writeShort(NUMBER_OF_NODES, nbrNodes);
   setLength(pos);
}


uint32
DisturbanceInfoRequestPacket::getNumberOfNodes() const
{
   return readShort(NUMBER_OF_NODES);
}

IDPair_t
DisturbanceInfoRequestPacket::getMapAndNode(uint32 index) const
{
   mc2dbg4 << " getMapAndNode" << endl;	 
   uint32 mapID  = readLong(getFirstNodePosition() +index *8);
   uint32 nodeID = readLong(getFirstNodePosition() +index *8 + 4);
   return IDPair_t(mapID,nodeID);
}

uint32
DisturbanceInfoRequestPacket::getNumberOfNodesOnMap(uint32 mapID) const
{
   // Check all nodes and count the ones on map mapID
   uint32 nbr        = 0;
   uint32 nbrOfNodes = getNumberOfNodes();
   for(uint32 i = 0; i < nbrOfNodes; i++){
      if(readLong(getFirstNodePosition()+ 8*i) == mapID)
         nbr++;
   }
   
   return nbr;
}

uint32
DisturbanceInfoRequestPacket::getNodeOnMap(uint32 mapID, uint32 index) const
{
   uint32 nbr = 0;
   uint32 nbrOfNodes = getNumberOfNodes();
   for(uint32 i = 0; i < nbrOfNodes; i++){
      if(readLong(getFirstNodePosition()+ 8*i) == mapID)
         if(nbr == index)
            return readLong(getFirstNodePosition()+ 8*i+4);
         nbr++;
   }
   return MAX_UINT32;
}


   
//
// ====================== Reply Packet =====================
// 

DisturbanceInfoReplyPacket::DisturbanceInfoReplyPacket(
                                const DisturbanceInfoRequestPacket* p)
      : ReplyPacket(
         MAX_PACKET_SIZE,
         PACKETTYPE_DISTURBANCEINFOREPLY,
         (RequestPacket*) p,
         StringTable::OK)
{
   setNumberOfDist(0);
   setLength(START_OF_DIST);
}

DisturbanceInfoReplyPacket::~DisturbanceInfoReplyPacket()
{

}

void
DisturbanceInfoReplyPacket::addDisturbance(
   uint32 mapID, uint32 nodeID,
   TrafficDataTypes::disturbanceType type,
   uint32 distID, MC2String comment)
{
   mc2dbg4 << "Adding Disturbance to Info Packet";
   int pos = getLength();
   set<uint32>::iterator si =  m_storedDisturbanceIDs.find(distID);
   if(si == m_storedDisturbanceIDs.end()){
      // new disturbance stor all info.
      mc2dbg4 << " new info " << endl;
      if(pos + 3*4+2+comment.size()+1 >= getBufSize()){
         resize(getBufSize()*2);
      }
      incWriteLong(pos, mapID);
      incWriteLong(pos, nodeID);
      incWriteLong(pos, distID);
      incWriteShort(pos, int(type));
      
      incWriteString(pos, comment.data());
      m_storedDisturbanceIDs.insert(distID);
   } else {
      // This is already in the packet.
      mc2dbg4 << " old info " << endl;
      if(uint32(pos + 3*4) >= getBufSize()){
         resize(getBufSize()*2);
      }
      incWriteLong(pos, mapID);
      incWriteLong(pos, nodeID);
      incWriteLong(pos, distID);
   }
   
   
   setNumberOfDist(getNumberOfDist()+1);
   setLength(pos);
}

uint32
DisturbanceInfoReplyPacket::getNumberOfDist() const
{
   return readShort(NUMBER_OF_DIST);
}

void
DisturbanceInfoReplyPacket::setNumberOfDist(uint32 nbr)
{
   writeShort(NUMBER_OF_DIST, nbr);
}

int
DisturbanceInfoReplyPacket::getNextDistID(
   uint32 &mapID, uint32 &nodeID, uint32 &distID, int pos )
{
   
   if(pos < START_OF_DIST)
      pos = START_OF_DIST;
   if(uint32(pos+14) > getLength())
      return -1;
   
   mapID  = incReadLong(pos);
   nodeID = incReadLong(pos);
   distID = incReadLong(pos);
   return pos;
}

int
DisturbanceInfoReplyPacket::getNextDistInfo(
   TrafficDataTypes::disturbanceType &type, MC2String &comment, int pos)
{
   if((pos < START_OF_DIST+12) || uint32(pos+2) > getLength()){
      mc2dbg8 << "DisturbanceInfoRepPacket::getNextDistInfo - "
           <<"wrong pos in packet, " << pos << " START_OF_DIST :"
           << START_OF_DIST << endl;
      return -1;
   }
   
   type   = TrafficDataTypes::disturbanceType(incReadShort(pos));
   char* str;
   incReadString(pos, str);
   comment = MC2String(str);
   return pos;
}

int
DisturbanceInfoReplyPacket::fillDisturbanceVector(
   vector<DisturbanceDescription> &distVector)
{
   mc2dbg4 << "distVector " <<  distVector.size() << endl;
   
   vector<DisturbanceDescription>::iterator vi;
   map<uint32, DisturbanceInfo*> foundDistInfo;
   
   uint32 mapID;
   uint32 nodeID;
   TrafficDataTypes::disturbanceType type;
   uint32 distID;
   MC2String comment;
   //int pos    = getNextDisturbance(mapID, nodeID, type, distID, comment);
   int nbrSet = 0;
   
   int pos    = getNextDistID(mapID, nodeID, distID);
   map<uint32, DisturbanceInfo*>::iterator di = foundDistInfo.find(distID);
   mc2dbg8 << "Next dist,";
   if(di != foundDistInfo.end()){
      // Use old info
      mc2dbg8 << " old info" << endl;
      type    = di->second->getType();
      comment = di->second->getComment();
      
   } else {
      // Read the info from the packet.
      pos = getNextDistInfo(type, comment, pos);
      mc2dbg8 << " new info" << endl;
      // Create new DistInfo and add to set.
      DisturbanceInfo*  newInfo = new DisturbanceInfo(type, comment);
      foundDistInfo.insert(make_pair(distID, newInfo));
   }
   
   
   
   
   while(pos != -1){
      mc2dbg4 << "Disturbance from packet";
      bool found      = false;
      DisturbanceDescription* alreadySet = NULL;
      vi = distVector.begin();

      // Find this disturbance in the vector.
      while((vi != distVector.end()) && !found){
         uint32 vMapID  = (*vi).getMapID();
         uint32 vNodeID = (*vi).getNodeID();

         if((vMapID == mapID) && (vNodeID == nodeID)){
            if(!(*vi).isDescriptionSet()){
               found = true;
               (*vi).setDescription(type, distID, comment.data());
                mc2dbg4<< ", filled with " << (int)type << ", "
                    << distID << ", " << comment.data() << endl;
               nbrSet++;
            } else if (alreadySet == NULL){
               mc2dbg4 << ", distID = " << (*vi).m_distID;
               alreadySet = &(*vi++);
            } else {
               vi++;
            }
         } else {
            vi++;
         }
      }
      if(alreadySet != NULL){
         mc2dbg4 << ",was allready set "<< endl;
         DisturbanceDescription newDist(alreadySet->m_splitPoint,
                                        alreadySet->m_distPoint,
                                        alreadySet->m_joinPoint,
                                        type, distID, comment.data());
         distVector.push_back(newDist);
         nbrSet++;
         
      } else if(!found){
         mc2dbg8 << " Disturbance not found in vector" << endl;
         
      }
      
      // Try next disturbance in packet.
      //pos = getNextDisturbance(mapID, nodeID, type, distID, comment, pos);
      pos    = getNextDistID(mapID, nodeID, distID, pos);
      di = foundDistInfo.find(distID);
      //mc2dbg8 << "Next dist,";
      if(di != foundDistInfo.end()){
         // Use old info
         //mc2dbg8 << " old info" << endl;
         type    = di->second->getType();
         comment = di->second->getComment();
         
      } else {
         // Read the info from the packet.
         pos = getNextDistInfo(type, comment, pos);
         //mc2dbg8 << " new info" << endl;
         // Create new DistInfo and add to set.
         DisturbanceInfo*  newInfo = new DisturbanceInfo(type, comment);
         foundDistInfo.insert(make_pair(distID, newInfo));
      }
   }

   // Delete the temp DisturbanceInfo
   for ( map<uint32, DisturbanceInfo*>::iterator it = foundDistInfo.begin()
            ; it != foundDistInfo.end() ; ++it )
   {
      delete it->second;
   }
   
   return nbrSet;
   
}
