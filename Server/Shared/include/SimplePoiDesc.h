/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SIMPLE_POI_DESC_H
#define SIMPLE_POI_DESC_H

#include "config.h"

#include "MC2String.h"

class BitBuffer;
class ServerTileMapFormatDesc;
class SharedBuffer;
class POIImageIdentificationTable;

/**
 *    Objects of this class describes the properties of each
 *    type of poi.
 */ 
class SimplePoiDescTypeEntry {
public:
   /**
    *   There is one of these entries for each type of poi.
    *   @param type                 The integer type of the entry.
    *   @param bitmap_name          Name of bitmap with no extension.
    *   @param centered_bitmap_name Name of centered version of bitmap.
    */
   SimplePoiDescTypeEntry( uint32 type,
                           const MC2String& bitmap_name,
                           const MC2String& centered_bitmap_name );

   const MC2String& getBitmapName() const {
      return m_bitmapName;
   }

   const MC2String& getCenteredBitmapName() const {
      return m_centeredBitmapName;
   }

   uint32 getType() const {
      return m_type;
   }
   
private:
   /// TileMapFeature type
   uint32 m_type;
   /// Name of bitmap
   MC2String m_bitmapName;
   /// Name of centered bitmap (m_bitMapName + "_old", probably )
   MC2String m_centeredBitmapName;
};

class SimplePoiDesc {
public:
   /// Offset to special custom feature index
   static const uint16 SPECIAL_FEATURE_OFFSET;
   /**
    *    Creates a new SimplePoiDesc from the ServerTileMapFormatDesc.
    *    Is self-contained in opposit of the SimplePoiMaps, i.e. you
    *    can still use the objects when the ServerTileMapFormatDesc
    *    has been deleted.
    *    @param tmfd ServerTileMapFormatDesc
    *    @param urlbase The base url to be added before all the images when
    *                   fetching via HTTP, e.g. "/TMap/"
    *    @param imageTable Special POI image table for custom pois.
    */
   SimplePoiDesc( const ServerTileMapFormatDesc& tmfd,
                  const MC2String& urlbase,
                  const POIImageIdentificationTable& imageTable );

   /**
    *    Could very well be a destructor.
    */
   ~SimplePoiDesc();

   /// Returns a new SharedBuffer containing the saved desc.
   SharedBuffer* getAsNewBytes();
   
private:

   /// Saves the data to the bitbuffer.
   void save( BitBuffer& buf ) const;

   /// URL base
   MC2String m_urlBase;

   /// Type of the vector of SimplePoiDescTypeEntry
   typedef vector<SimplePoiDescTypeEntry> typeEntryVector_t;
   
   /// Vector of type entries.
   typeEntryVector_t m_typeEntries;
   
};

#endif
