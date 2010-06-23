/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFX_PROCESSOR_H
#define GFX_PROCESSOR_H

#include "config.h"

#include "Processor.h"

class GfxFeatureMapImageReplyPacket;
class GfxFeatureMapImageRequestPacket;
class ReplyPacket;
class Packet;
class RequestPacket;
class POIImageIdentificationTable;
class ServerTileMapFormatDesc;

/**
  *   A processor, the main task of which is to produce replies 
  *   containing map images (GIF). Handle questions such as:
  *   convert GfxFeatureMap to image.
  *   
  *   
  */
class GfxProcessor : public Processor {
   public:
      /**
        *   Creates one GfxProcessor
        *   XXX: The processor shouldn't have any maps.
        *   @param loadedMaps is a vector containing the currently 
        *                     loaded maps.
        *   @param packetFileName Filename for saving packets for
        *                         later profiling.
        */
      GfxProcessor( MapSafeVector* loadedMaps,
                    const char* packetFileName = NULL);

      /** 
       *    Destructor of the GfxProcessor.
       */
       virtual ~GfxProcessor();


      /**
        *   Implementation of the virtual method in the super class.
        *   @return  The status of this processor.
        *            {\it Currently not used, so not implemented.}
        */
      int getCurrentStatus();


protected:
      /**
        *   Implementation of the virtual method in the super class.
        *   @param   p  The requestpacket to process.
        *   @return  The answer to the question given as parameter.
        */
      Packet* handleRequestPacket( const RequestPacket& p,
                                   char* packetInfo );
private:

   
       /**
        *   Produces a reply to a GfxFeatureMapImageRequest, which
        *   contains a GfxFeatureMap. Returns a GfxFeatureMapImageReply,
        *   which contains an Image.
        *   @param  p  The GfxFeatureMapImageRequestPacket.
        *   @return   The GfxFeatureMapImageReplyPacket.
        */        
       GfxFeatureMapImageReplyPacket* processGfxFeatureMapImageRequest(
				       const GfxFeatureMapImageRequestPacket* p);

      /**
       *    Name of packet file. NULL if no packets should be saved.
       */
      MC2String m_packetFileName;
   ServerTileMapFormatDesc* m_stmfd;
   /// Translation table for custom poi images.
   POIImageIdentificationTable* m_poiImageTable;
};

#endif




