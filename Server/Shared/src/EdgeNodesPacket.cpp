/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "EdgeNodesPacket.h"

#include "OrigDestInfo.h"
#include "StringTable.h"

//-----------------------------------------------------------------
// EdgeNodesRequestPacket
//-----------------------------------------------------------------

#define DEST_SIZE 16

int
EdgeNodesRequestPacket::writeOrigDestInfo(int& pos,
                                          const OrigDestInfo& info)
{
   incWriteLong(pos, info.getMapID());
   incWriteLong(pos, info.getNodeID());
   incWriteLong(pos, info.getLat());
   incWriteLong(pos, info.getLon());
   return 4*4;
}

EdgeNodesRequestPacket::EdgeNodesRequestPacket(uint32 packetID,
                                               uint32 requestID,
                                               uint32 mapID,
                                               uint32 level,
                                               const OrigDestInfoList& dests,
                                               uint32 userDef,
                                               uint32 vehicles) :
      RequestPacket( EDGENODES_REQUEST_MAX_LENGTH + dests.size()*DEST_SIZE,
                     EDGENODES_REQUEST_PRIO,
                     Packet::PACKETTYPE_EDGENODESREQUEST,
                     packetID,
                     requestID,
                     mapID)
{
   int pos = LEVEL_POS;
   incWriteLong(pos, level);
   incWriteLong(pos, userDef);
   incWriteLong(pos, vehicles);
   incWriteLong(pos, dests.size());
   for ( OrigDestInfoList::const_iterator it = dests.begin();
         it != dests.end();
         ++it ) {
      writeOrigDestInfo(pos, *it);
   }
   setLength(pos);
}

OrigDestInfo*
EdgeNodesRequestPacket::readOrigDestInfo(int& pos) const
{
   uint32 mapID  = incReadLong(pos);
   uint32 itemID = incReadLong(pos);
   uint32 lat    = incReadLong(pos);
   uint32 lon    = incReadLong(pos);
   return new OrigDestInfo(NULL,
                           mapID,
                           itemID,
                           MAX_UINT32, // prev route
                           MAX_UINT32, // cost,
                           MAX_UINT32, // estimated cost
                           lat,
                           lon);
}

int
EdgeNodesRequestPacket::getDestinations(OrigDestInfoList& dests) const
{
   int pos = DEST_NBR_POS;
   int nbrDest = incReadLong(pos);
   pos = DEST_START_POS;
   for ( int i = 0; i < nbrDest; ++i ) {
      OrigDestInfo* dest = readOrigDestInfo(pos);
      dests.addOrigDestInfo(*dest);
      delete dest; // Ok. 
   }
   return nbrDest;
}

//-----------------------------------------------------------------
// EdgeNodesReplyPacket
//-----------------------------------------------------------------

inline void
EdgeNodesReplyPacket::writeBorderMaps(int& pos,
                                      const set<uint32>& borderMaps)
{
   set<uint32>::const_iterator it(borderMaps.begin());
   while ( it != borderMaps.end() ) {
      incWriteLong(pos, *it++);
   }
}

inline void
EdgeNodesReplyPacket::writeExternalNodes(int& pos,
                                         const vector<uint32>& externalNodes,
                                         const vector<int32>& lats,
                                         const vector<int32>& lons)
{
   int nbrExternal = externalNodes.size();
   for(int i=0; i < nbrExternal; ++i ) {
      incWriteLong(pos, externalNodes[i]);
      incWriteLong(pos, lats[i]);
      incWriteLong(pos, lons[i]);
   }
}

inline void
EdgeNodesReplyPacket::writePacketStart(int& pos,
                                       const EdgeNodesRequestPacket* req)
{
   pos = MAP_ID_POS; // Must set pos here.
   // Map id
   incWriteLong( pos, req->getMapID() );
   // Copy level
   incWriteLong( pos, req->getLevel() );
   // Copy user defined data
   incWriteLong( pos, req->getUserDefinedData() );
   incWriteLong( pos, 0x0000 ); // For future use
   incWriteLong( pos, 0xffff ); // For future use
}

inline void
EdgeNodesReplyPacket::writeDistances(int& pos,
                                     const map<uint32,uint32>& distances)
{
   incWriteLong(pos, distances.size());
   for( map<uint32,uint32>::const_iterator it = distances.begin();
        it != distances.end();
        ++it ) {
      incWriteLong(pos, it->first);
      incWriteLong(pos, it->second);
   }
}

EdgeNodesReplyPacket::
EdgeNodesReplyPacket(const EdgeNodesRequestPacket* req,
                     uint32 status,
                     uint32 overviewMapID,
                     const set<uint32>& borderMaps,
                     const vector<uint32>& externalNodes,
                     const vector<int32>& lats,
                     const vector<int32>& lons,
                     const map<uint32,uint32>& distances,
                     int totalNbrExt) :
      ReplyPacket( EDGENODES_REPLY_MAX_LENGTH,
                   Packet::PACKETTYPE_EDGENODESREPLY,
                   req,
                   status)
{
   int pos;
   writePacketStart(pos, req); // pos is set here
   if ( status == StringTable::OK ) {
      // Overview map ID
      incWriteLong( pos, overviewMapID );
      // Number of border maps
      incWriteLong( pos, borderMaps.size() );
      // Number of external nodes
      incWriteLong( pos, externalNodes.size() );
      // Total number of external nodes
      incWriteLong( pos, totalNbrExt );
      // Write the values of the bordermap id:s
      writeBorderMaps(pos, borderMaps);
      // Write the external nodes and lat/lon.
      writeExternalNodes(pos, externalNodes, lats, lons);
      // Write the minimum distances
      writeDistances(pos, distances);
   }
   setLength(pos);
}

EdgeNodesReplyPacket::
EdgeNodesReplyPacket(const EdgeNodesRequestPacket* req,
                     uint32 status) :
      ReplyPacket( EDGENODES_REPLY_MAX_LENGTH,
                   Packet::PACKETTYPE_EDGENODESREPLY,
                   req,
                   status)
{
   int pos;
   writePacketStart(pos, req); // pos is set there.
   setLength(pos);
}

int
EdgeNodesReplyPacket::getEdgeNodes(OrigDestInfoList& nodeList,
                                   map<uint32,uint32>* distances,
                                   bool flipIDs) const
{
   uint32 mapID = getMapID();
   int pos = getNodesStartPos();
   int nbrNodes = getNbrEdgeNodes();
   for( int i=0; i < nbrNodes; ++i ) {
      uint32 nodeID = incReadLong(pos);
      if ( flipIDs ) {
         nodeID = nodeID ^ 0x80000000;
      }
      int32 lat = incReadLong(pos);
      int32 lon = incReadLong(pos);
      nodeList.addOrigDestInfo(OrigDestInfo(NULL, // Vehicle
                                            mapID,
                                            nodeID,
                                            MAX_UINT32, // PrevSubRoute
                                            0,          // Cost
                                            0,          // Estimated cost
                                            lat,
                                            lon));
   }
   if ( distances == NULL ) {
      return nbrNodes;
   }

   // The user wants the distances too.
   int nbrDistances = incReadLong(pos);
   for ( int i = 0; i < nbrDistances; ++i  ) {
      uint32 mapID = incReadLong(pos);
      uint32 dist  = incReadLong(pos);
      distances->insert(make_pair(mapID, dist));
   }
   return nbrNodes;
}

int
EdgeNodesReplyPacket::getBorderMaps(set<uint32>& borderMaps) const
{
   int nbrBorderMaps = getNbrBorderMaps();
   
   int pos = BORDER_MAPS_START_POS;
   
   for(int i=0; i < nbrBorderMaps; ++i ) {
      borderMaps.insert(incReadLong(pos));
   }
   return nbrBorderMaps;
}
