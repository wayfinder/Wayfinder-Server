/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "IDTranslationPacket.h"

//-----------------------------------------------------------------
// IDTranslationRequestPacket
//-----------------------------------------------------------------
IDTranslationRequestPacket::
IDTranslationRequestPacket(uint32 packetID,
                              uint32 requestID,
                              uint32 mapID,
                              bool translateToLower,
                              const OrigDestInfoList& nodeList,
                              uint32 userDef)
      : RequestPacket( IDTRANSLATION_REQUEST_MAX_LENGTH,
                       IDTRANSLATION_REQUEST_PRIO,
                       Packet::PACKETTYPE_IDTRANSLATIONREQUEST,
                       packetID,
                       requestID,
                       mapID)
{
   // Move the stuff to a format that init recognizes.
   // Optimize later.
   IDPairVector_t* nodeVectors[1];   
   nodeVectors[0] = new IDPairVector_t;

   
   for( OrigDestInfoList::const_iterator it(nodeList.begin());
        it != nodeList.end();
        ++it) {
      nodeVectors[0]->push_back(IDPair_t((*it).getMapID(),
                                           (*it).getNodeID()));
   }
   
   init(translateToLower,
        nodeVectors,
        1,
        userDef);
   delete nodeVectors[0];
}

void
IDTranslationRequestPacket::init(bool translateToLower,
                                    IDPairVector_t** nodeVectors,
                                    int nbrNodeVectors,
                                    uint32 userDef)
{
   int pos = USER_DEF_POS;
   incWriteLong(pos, userDef);
   writeBit(FLAGS_POS, LOWER_LEVEL_BIT_POS, translateToLower);
   // Skip
   pos = NBR_NODE_VECT_POS;
   incWriteLong(pos, nbrNodeVectors);

   // Write the sizes of the nodevectors
   for(int i=0; i < nbrNodeVectors; ++i ) {
      incWriteLong(pos, nodeVectors[i]->size() );
   }

   // Write all the nodes
   for(int i = 0; i < nbrNodeVectors; ++i ) {
      const IDPairVector_t* curVect = nodeVectors[i];
      int vectSize = curVect->size();
      for(int j=0; j < vectSize; ++j) {
         incWriteLong(pos, (*curVect)[j].first);  // mapID
         incWriteLong(pos, (*curVect)[j].second); // nodeID
      }
   }
   // Don't forget to set the length
   setLength(pos);
}
   
IDTranslationRequestPacket::
IDTranslationRequestPacket(uint32 packetID,
                              uint32 requestID,
                              uint32 mapID,
                              bool translateToLower,
                              IDPairVector_t** nodeVectors,
                              int nbrNodeVectors,
                              uint32 userDef)
      : RequestPacket( IDTRANSLATION_REQUEST_MAX_LENGTH,
                       IDTRANSLATION_REQUEST_PRIO,
                       Packet::PACKETTYPE_IDTRANSLATIONREQUEST,
                       packetID,
                       requestID,
                       mapID)
{
   init(translateToLower,
        nodeVectors,
        nbrNodeVectors,
        userDef);
}

IDTranslationRequestPacket::
IDTranslationRequestPacket(uint32 packetID,
                           uint32 requestID,
                           uint32 mapID,
                           bool translateToLower,
                           IDPairVector_t& nodeVector,
                           uint32 userDef) 
      : RequestPacket( IDTRANSLATION_REQUEST_MAX_LENGTH,
                       IDTRANSLATION_REQUEST_PRIO,
                       Packet::PACKETTYPE_IDTRANSLATIONREQUEST,
                       packetID,
                       requestID,
                       mapID)
{
   IDPairVector_t* nodeVector_p = &nodeVector;
   init(translateToLower,
        &nodeVector_p,
        1,
        userDef);
}

   


int
IDTranslationRequestPacket::getNbrNodeVectors() const
{
   return readLong(NBR_NODE_VECT_POS);
}

int
IDTranslationRequestPacket::getNbrNodesForVector(int i) const
{
   return readLong(NODE_VECT_SIZES_START_POS + 4*i);
}

int
IDTranslationRequestPacket::getNodesStartPos() const
{
   return NODE_VECT_SIZES_START_POS + getNbrNodeVectors() * 4;
}

int
IDTranslationRequestPacket::getAllNodes(IDPairVector_t& allNodes) const
{
   int totalNodes = 0;
   for(int i = 0; i < getNbrNodeVectors(); ++i ) {
      totalNodes += getNbrNodesForVector(i);
   }

   allNodes.reserve(totalNodes);

   int pos = getNodesStartPos();

   for(int i=0; i < totalNodes; ++i ) {
      uint32 mapID  = incReadLong(pos);
      uint32 nodeID = incReadLong(pos);
      allNodes.push_back(IDPair_t(mapID, nodeID));
   }
   return totalNodes;
}

uint32
IDTranslationRequestPacket::getUserDefData() const
{
   return readLong(USER_DEF_POS);
}

bool
IDTranslationRequestPacket::getTranslateToLower() const
{
   return readBit(FLAGS_POS, LOWER_LEVEL_BIT_POS);
}

//-----------------------------------------------------------------
// IDTranslationReplyPacket
//-----------------------------------------------------------------

IDTranslationReplyPacket::
IDTranslationReplyPacket( const IDTranslationRequestPacket* p,
                          uint32 status) : 
   ReplyPacket( IDTRANSLATION_REPLY_MAX_LENGTH,
                Packet::PACKETTYPE_IDTRANSLATIONREPLY,
                p,
                status)

{
   int pos = MAP_ID_POS;
   incWriteLong(pos, p->getMapID());
   incWriteLong(pos, p->getUserDefData());
   // Skip the flags
   incWriteLong(pos, MAX_UINT32);
   // Skip the unused position
   incWriteLong(pos, 0);
   // Write number of nodevectors
   incWriteLong(pos, 0);
   // Write the flags
   writeBit(FLAGS_POS, LOWER_LEVEL_BIT_POS, p->getTranslateToLower());
   // Set the length
   setLength(pos);
}

      

IDTranslationReplyPacket::
IDTranslationReplyPacket(const IDTranslationRequestPacket* p,
                         uint32 status,
                         const IDPairVector_t& allTranslatedNodes)
      : ReplyPacket( IDTRANSLATION_REPLY_MAX_LENGTH,
                     Packet::PACKETTYPE_IDTRANSLATIONREPLY,
                     p,
                     status)
{
   int pos = MAP_ID_POS;
   incWriteLong(pos, p->getMapID());
   incWriteLong(pos, p->getUserDefData());
   // Skip the flags
   incWriteLong(pos, MAX_UINT32);
   // Write the flags
   writeBit(FLAGS_POS, LOWER_LEVEL_BIT_POS, p->getTranslateToLower());
   // Skip the unused position
   incWriteLong(pos, 0);
   int nbrNodeVectors = p->getNbrNodeVectors();
   // Write the number of node vectors and their sizes.
   incWriteLong(pos, nbrNodeVectors);
   for(int i=0; i < nbrNodeVectors; ++i ) {
      incWriteLong(pos, p->getNbrNodesForVector(i));      
   }

   // Write all the nodes
   for(uint32 i=0; i < allTranslatedNodes.size(); ++i) {
      incWriteLong(pos, allTranslatedNodes[i].first);  // mapID
      incWriteLong(pos, allTranslatedNodes[i].second); // nodeID
   }
   setLength(pos);
}

uint32
IDTranslationReplyPacket::getUserDefinedData() const
{
   return readLong(USER_DEF_POS);
}

int
IDTranslationReplyPacket::getNbrNodeVectors() const
{
   return readLong(NBR_NODE_VECT_POS);
}


int
IDTranslationReplyPacket::getNbrNodesForVector(int index) const
{
   return readLong(NODE_VECT_SIZES_START_POS + 4*index);
}

int
IDTranslationReplyPacket::getNodesStartPos() const
{
   return NODE_VECT_SIZES_START_POS + getNbrNodeVectors() * 4;
}

int
IDTranslationReplyPacket::getNodesStartForVector(int index) const
{
   int pos = getNodesStartPos();
   for(int i=0; i < index; ++i ) {
      pos += getNbrNodesForVector(index) * 8;
   }
   return pos;
}


int
IDTranslationReplyPacket::
getTranslatedNodes(int index,
                   OrigDestInfoList& outList,
                   const OrigDestInfoList& origList) const
{
   int nbrNodes = getNbrNodesForVector(index);
   int nodesStartPos = getNodesStartForVector(index);
   int pos = nodesStartPos;
   OrigDestInfoList::const_iterator it(origList.begin());
   for(int i=0; i < nbrNodes; ++i ) {
      uint32 mapID = incReadLong(pos);
      uint32 nodeID = incReadLong(pos);
      // Copy the origdestinfo and set new mapId and nodeid
      OrigDestInfo origDestInfo(*it);
      origDestInfo.setMapID(mapID);
      origDestInfo.setNodeID(nodeID);
      outList.addOrigDestInfo(origDestInfo);
      ++it;
   }
   return nbrNodes;
}


int
IDTranslationReplyPacket::
getTranslatedNodes(int index,
                   IDPairVector_t& outVector)
{
   int nbrNodes = getNbrNodesForVector(index);
   int nodesStartPos = getNodesStartForVector(index);
   int pos = nodesStartPos;
   for(int i=0; i < nbrNodes; ++i ) {
      uint32 mapID = incReadLong(pos);
      uint32 nodeID = incReadLong(pos);
      outVector.push_back(IDPair_t(mapID, nodeID));
   }
   return nbrNodes;
}


