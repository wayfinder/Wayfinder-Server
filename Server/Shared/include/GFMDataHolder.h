/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFMDATAHOLDER_H
#define GFMDATAHOLDER_H 

#include "config.h"
#include "MC2BoundingBox.h"
#include <vector>
#include "MC2SimpleString.h"

class DataBuffer;
class PacketContainer;
class Packet;
class GfxFeatureMap;

/**
 *
 *    GfxFeatureMapDataHolder.
 *
 *    Is used to merge the GfxFeatureMaps from the MapModule.
 *    Can be saved and loaded to packet so that the merging can be
 *    done by a module and not necessarily by a server.
 *
 */
class GFMDataHolder {
   public:

      /**
       *   Constructor.
       */
      GFMDataHolder();
      
      /**
       *   Constructor with params.
       */
      GFMDataHolder( const MC2BoundingBox& bbox, 
                     int screenSizeX, 
                     int screenSizeY );
      
      /**
       *   Deletes all contents, including the supplied PacketContainers.
       */
      ~GFMDataHolder();

      /**
       *   Adds GfxFeatureMap related data.
       *   
       *   @param pc          PacketContainer. Will be deleted in the
       *                      destructor.
       *   @param ziped       If the GfxFeatureMap is zipped.
       *   @param data        The GfxFeatureMap data.
       *   @param requestTag  The request tag for the GFM.
       *   @param copyright   The copyright string.
       */
      void addGFMData( PacketContainer* pc,
                       bool zipped,
                       DataBuffer* data,
                       uint32 requestTag = MAX_UINT32,
                       const char* copyright = "" );
      
      /**
       *   Save to packet.
       */
      int save( Packet* packet, int& pos ) const;

      /**
       *   Load from packet.
       */
      int load( const Packet* packet, int& pos );
      
      /**
       *   Get the resulting GfxFeatureMap object. 
       */
      const GfxFeatureMap* getGfxFeatureMap();
      
      /**
       *   Get the resulting GfxFeatureMap buffer.
       */
      DataBuffer* getBuffer();

      /**
       *   Get the resulting copyright string.
       */
      const MC2SimpleString& getCopyright();
      
   private:
      /**
       *   Merge the GfxFeatureMap if not already done.
       */
      void mergeIfNecessary();

      /// The bbox.
      MC2BoundingBox m_bbox;

      /// Screen size x.
      int m_screenSizeX;

      /// Screen size y.
      int m_screenSizeY;
      
      /// The merged GfxFeatureMap.
      GfxFeatureMap* m_gfxMap;

      /// The merged GfxFeatureMap buffer.
      DataBuffer* m_gfxMapBuffer;

      /// The copyright string.
      MC2SimpleString m_copyright;
     
      /// Internal object, used to keep track of GFM data.
      struct GFMData {
         /// Constructor.
         GFMData( PacketContainer* pc,
                  bool zipped,
                  DataBuffer* data,
                  uint32 requestTag,
                  const MC2SimpleString& copyright ) : m_garbage( pc ),
                                            m_data( data ),
                                            m_requestTag( requestTag ),
                                            m_copyright( copyright ), 
                                            m_zipped( zipped ) {}
         /// Constructor.
         GFMData() {}

         /// Packet container.
         PacketContainer* m_garbage;

         /// Data buffer.
         DataBuffer* m_data;

         /// Request tag.
         uint32 m_requestTag;

         /// Copyright string.
         MC2SimpleString m_copyright;

         /// GfxFeatureMap is zipped?
         bool m_zipped;
      };
      
      /// The GfxFeatureMap related data.
      vector<GFMData> m_data;

};


// ------ Implementation of inlined methods -----------


#endif // GFMDATAHOLDER_H 



