/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "config.h"
#include "MC2String.h"

class Packet;
class RequestPacket;
class MapSafeVector;

/**
  *   Superclass of all RequestPacket processors.
  *
  */
class Processor {
public:
   
   /** Maximum size of the packetInfo string in handleRequest */
   static const int c_maxPackInfo = 1024;
   
   /**
    *   Constructor that takes the pointer to the loadedMap's
    *   in the Reader.
    *   @param p_loadedMaps The MapSafeVector for Reader-Processor
    *                        communication.
    */
   explicit Processor( MapSafeVector* loadedMaps );

   /**
    * Destructor.
    */
   virtual ~Processor();

   
   /**
    *   NB! This function created the reply packet (with "new" or
    *       by transforming the incomming RequestRacket) and {\bf 
    *       must delete} the request (if the ReplyPacket is created 
    *       with "new")!
    *
    *   @param   p          The RequestPacket, that the ReplyPacket should
    *                       answer to.
    *   @param   packetInfo Information about the packet that should
    *                       be printed by the JobThread after the packet
    *                       is finished. JobThread already prints the
    *                       packettype, so you won't have to add that.
    *   @return  The answer to the requestPacket in the parameter.
    */
   Packet* handleRequest( RequestPacket* p, char* packetInfo );
   
   /**
    *   This is currently not used...
    */
   virtual int getCurrentStatus() = 0;
   
   /**
    *  Updates processorTime in the MapSafeVector.
    *  @param processTime The time it tock to process the request.
    */
   virtual void finishRequest( uint32 processTime );
   
   virtual void beginWork();
   
   virtual void endWork();

   void setPacketFilename( const MC2String& filename ) { 
      m_packetFilename = filename; 
   }

protected:
   /**
    *   Virtual method that produces the reply to the incomming
    *   packet.
    *   NB! This function created the reply packet (with "new" or
    *       by transforming the incomming RequestRacket) and
    *       the request (if the ReplyPacket is created 
    *       with "new")!
    *
    *   @param   p          The RequestPacket, that the ReplyPacket should
    *                       answer to.
    *   @param   packetInfo Information about the packet that should
    *                       be printed by the JobThread after the packet
    *                       is finished. JobThread already prints the
    *                       packettype, so you won't have to add that.
    *   @return  The answer to the requestPacket in the parameter.
    */
   virtual Packet* handleRequestPacket( const RequestPacket& p,
                                        char* packetInfo ) = 0;

   virtual Packet* handleSpecialPacket( const RequestPacket& p,
                                        char* packetInfo );

private:


   /// the logging prefix
   char m_logPrefix[32];

   /// File to write packets to for debugging
   MC2String m_packetFilename;
   MapSafeVector* m_loadedMaps;
};

#endif
