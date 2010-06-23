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

#include "MapSafeVector.h"

#include "Processor.h"
#include "Packet.h"
#include "LeaderIPPacket.h"
#include "PacketDump.h"
#include "TestPacket.h"
#include "LogBuffer.h"

// AcknowledgePacket is in SystemPackets.
#include "SystemPackets.h" 

Processor::Processor( MapSafeVector* loadedMaps ):
   m_loadedMaps( loadedMaps ) {
   // initial state is "job ended"
   m_loadedMaps->setJobThreadEnd();
}


Processor::~Processor() {
}

Packet*
Processor::handleRequest( RequestPacket* p,
                          char* packetInfo ) {

   PacketUtils::dumpBinaryToFile( m_packetFilename, *p );

   //
   // First we try to handle the package ourself,
   // if we didnt handle it send it to our sub classes
   //
   Packet *reply = handleSpecialPacket( *p, packetInfo );
   if ( reply == NULL ) {
      reply = handleRequestPacket( *p, packetInfo );
   }

   delete p;

   return reply;
}

Packet* Processor::handleSpecialPacket( const RequestPacket& p,
                                        char* packetInfo ) {
   Packet* reply = NULL;
   uint16 subType = p.getSubType();
   mc2dbg8 << "[Proc]: Subtype is " << subType << endl;
   switch ( subType ) {
      case Packet::PACKETTYPE_TESTREQUEST:
         reply = new TestReplyPacket(
            static_cast<const TestRequestPacket&>( p ),0,0 );
         break;
         
      case Packet::PACKETTYPE_LEADERIP_REQUEST: {
         const LeaderIPRequestPacket& lrp =
            static_cast<const LeaderIPRequestPacket&>( p );
         return new LeaderIPReplyPacket( lrp,
                                         m_loadedMaps->getAddr().getIP(),
                                         m_loadedMaps->getAddr().getPort(),
                                         lrp.getModuleType(),
                                         m_loadedMaps->getLeaderAddr() );

      } break;

   }
   return reply;
}

void Processor::finishRequest( uint32 processTime ) {
   const float PROCESSTIMEFORGETTINGFACTOR = 0.25;

   uint32 time = m_loadedMaps->getProcessTime();
   uint32 newTime;
   if ( processTime > 5000000 ) {
      processTime = 50000000;
   }
   
   if(time > 0)
      newTime = uint32(PROCESSTIMEFORGETTINGFACTOR * processTime) +
         uint32((1.0 - PROCESSTIMEFORGETTINGFACTOR) * time);
   else
      newTime = processTime/2;
   m_loadedMaps->setProcessTime(newTime);
}

void Processor::beginWork() {
   m_loadedMaps->setJobThreadStart();
}

void Processor::endWork() {
   m_loadedMaps->setJobThreadEnd();
}
