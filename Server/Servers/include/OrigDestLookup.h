/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ORIGDESTLOOKUP_H
#define ORIGDESTLOOKUP_H

#include <map>
#include "PacketContainerTree.h"
#include "CoordinateObject.h"

class Request; // forward decl
class PacketContainer; // forward decl
class OrigDestInfo; // forward decl
class OrigDestInfoList; // forward decl
class OrigDestInfoListList; // forward decl
class HandleOrigDestInfo; // forward decl, for friend
class RouteExpandItemReplyPacket; // forward decl
class TopRegionRequest;

/**
 *    Class for completing info about each OrigDestInfo in a request.
 *    Info should be ItemID, MapID and MC2 lat and long
 *
 */
class OrigDestLookup {
public:
   
   OrigDestLookup(Request*               request,
                  OrigDestInfoList*      origInfoList,
                  const TopRegionRequest* topReq,
                  OrigDestInfoListList*  viaInfoList  = NULL,
                  OrigDestInfoList*      destInfoList = NULL);

   void processPacket( PacketContainer* p );

   PacketContainer* getNextPacket( );
   
   inline bool getDone( ) const;

private:
   
   inline int addCoordinate(int32        lat,
                            int32        lon, 
                            uint32       outDataTypes,
                            byte         nbrAllowedItemTypes,
                            const byte*  itemTypes,
                            uint16       angle);
   
   void updateOutgoingPackets();
   
   void handleIDReplyPacket(PacketContainer* pc);

   inline void addCoordRequestInfo( uint32        key,
                                    OrigDestInfo* info );

   inline void addIDRequestInfo( int           key,
                                 OrigDestInfo* info );

   inline void addOutgoingPacket( PacketContainer* p );
   
   Request* m_request;
   
   /**
    *    Container for keeping packets on the way to the RouteRequest
    */
   PacketContainerTree m_outgoingQueue;

   CoordinateObject m_coordinateObject;
   
   multimap<uint32, OrigDestInfo*> m_requestCoordList;

   map<int, OrigDestInfo*> m_requestIDList;

   bool m_done;

   StringTable::stringCode m_status;
   
   friend class HandleOrigDestInfo;
   
};

// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -

inline bool
OrigDestLookup::getDone() const {
   return m_done;
}

#endif



