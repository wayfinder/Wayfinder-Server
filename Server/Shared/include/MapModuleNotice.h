/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPMODULENOTICE_H
#define MAPMODULENOTICE_H

#include "config.h"

#include "DataBuffer.h"
#include "MapNotice.h"
#include "GfxDataFull.h"

#include <set>

class CountryCode;
/**
  *   Objects of this class is used by the MapModule to keep track of
  *   all existing maps without having to have all of them loaded.
  *
  */
class MapModuleNotice : public MapNotice {
   public :
      /**
        *   Creates a MapModuleNotice according to the specifications.
        *
        *   @param   mapID        The map id this MapModuleNotice describes.
        *   @param   mapGfxData   The graphical description of the boundry
        *                         of the map. Will not be copied.
        *   @param   creationTime The time when the map was created.
        *                         Nice to know when cacheing.
        */
      MapModuleNotice(  uint32 mapID        = MAX_UINT32, 
                        GfxData* mapGfxData = NULL,
                        uint32 creationTime = MAX_UINT32);

      /**
        *   Creates a MapModuleNotice from a DataBuffer. Useful when
        *   loading from disk.
        *   @param   dataBuffer  Where to get the rawdata of this 
        *                        MapModuleNotice.
        *   @param version Version of index.db. If version > 0
        *                  the MapModuleNotice will also have a version.
        *   @param mapSet The Map set ID to use, MAX_UINT32 is default and
        *                 means ignore
        */
      MapModuleNotice(DataBuffer* dataBuffer, 
                      int version = 0,
                      uint32 mapSet = MAX_UINT32);

      /**
        *   Deletes this MapModuleNotice and returns the allocated memory.
        */      
      virtual ~MapModuleNotice();

  public:
      
      /**
        *   Saves this MapModuleNotice to disk.
        *   @param   dataBuffer  Where the MapNotice is to be stored.
        *   @return  True if this MapModuleNotice saved ok, false 
        *            otherwise.
        */
      bool save(DataBuffer* dataBuffer, int version = 0);

      /**
        *    NB! This method adds 4 in order to compensate for alignment,
        *        i.e. it does not return the extact size.
        *
        *    Tells how large the notice is in when saved in a databuffer.
        *    @return The size of the notice in a data buffer.
        */
      uint32 getSizeInDataBuffer(int version = 0) const;

      /**
        *   This function is inlined.
        *   @return  The graphical representation of this map.
        */
      inline const GfxData* getGfxData() const;

      /**
       *    Returns the bounding box of the gfxData.
       */
      inline const MC2BoundingBox& getBBox() const;

      /**
       *    Copies the internal bounding box to the outbbox and returns
       *    a reference to it.
       */
      inline const MC2BoundingBox& getBBox(MC2BoundingBox& outbbox) const;
      
      /**
        *   This function is inlined.
        *   @param   gfx   The graphical representation of this map.
        */
      inline void setGfxData( const GfxData* gfx );

      /**
       *    Update the countryCode of this map.
       *    @param country The new country of this map.
       */
      void setCountryCode( const CountryCode& country );

      /**
       *    Get the country where this map is located.
       *    @return The country where this map is located.
       */
      CountryCode getCountryCode() const;

      /**
       *    Update the creation time of the map (this must be the
       *    same as the creation time in the real map), in UNIX-time.
       *    @param country The new creation time of this map.
       */
      inline void setCreationTime(uint32 creationTime);

      /**
       *    Get the time when the map was created (in UNIX-time).
       *    @return The time when this map was created.
       */
      inline uint32 getCreationTime() const;

      /**
       *    Returns the name of the map. Used when creating maps
       *    and top regions. Can also be used for nicer debugging
       *    messages.
       *    @return The name of the map.
       */
      inline const char* getMapName() const;

      /**
       *    Sets the name of the map.
       *    @param name The name.
       */
      void setMapName(const char* name);
            
      /**
       *    Sets the name of the country map.
       *    @param name The name.
       */
      void setCountryMapName(const char* name);
      
   private:
      
      /**
        *   GfxData of the map, usefull when looking for what map
        *   to ask for when looking for the closest item to a coordinate.
        */
      GfxData* m_gfxData;

      /**
       *    Bounding box for the GfxData. The getBoundingBox-method
       *    of GfxData is slow so we create the bbox when loading.
       */
      MC2BoundingBox m_bbox;
      
      /**
        *   mapIDs of the neighbour maps. Never set. Can be loaded though.
        */
      set<uint32> m_neighbourMaps;

      /**
       *    The country where this map is located.
       */
      uint32 m_countryCode;

      /**
       *    The creation time of the map. Used when detecting
       *    if a map has changed.
       */
      uint32 m_creationTime;

      /**
       *    Name of the map. Used when creating maps and regions.
       */
      char* m_mapName;

      /**
       *    Country map name prefix.
       */
      static const char* countryNamePrefix;

};


// =======================================================================
//                                 Implementation of the inlined methods =


inline const GfxData* 
MapModuleNotice::getGfxData() const
{
   return m_gfxData;
}

inline const MC2BoundingBox&
MapModuleNotice::getBBox() const
{
   return m_bbox;
}

inline const MC2BoundingBox&
MapModuleNotice::getBBox(MC2BoundingBox& bbox) const
{
   bbox = m_bbox;
   return bbox;
}

inline void 
MapModuleNotice::setGfxData( const GfxData* gfx )
{
   delete m_gfxData;
   //m_gfxData = new GfxDataFull( *dynamic_cast<const GfxDataFull*>(gfx) );
   m_gfxData = GfxData::createNewGfxData(NULL, gfx);
   // Also update the bounding box.
   m_gfxData->getMC2BoundingBox( m_bbox );
}


inline void 
MapModuleNotice::setCreationTime(uint32 creationTime)
{
   m_creationTime = creationTime;
}

inline uint32 
MapModuleNotice::getCreationTime() const
{
   return m_creationTime;
}

inline const char*
MapModuleNotice::getMapName() const
{
   return m_mapName;
}


#endif

