/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXFEATUREMAPPROCBASE_H
#define GFXFEATUREMAPPROCBASE_H

#include "config.h"
#include <map>

class GenericMap;

/**
 *   Class containing methods common to the GfxFeatureMapProcessor
 *   and the GfxTileFeatureMapProcessor.
 */
class GfxFeatureMapProcBase {
protected:
   /// Creates a new GfxFeatureMapProcBase for the specified map
   GfxFeatureMapProcBase(
      GenericMap& m,
      GfxFeatureMapProcBase* theOneToShareCacheWith = NULL);

   /// Cleans up
   virtual ~GfxFeatureMapProcBase();

   /**
    *    Get the citycentre display class for the specified sql id.
    *    Caches the found display classes.
    *    
    *    @param   sqlID        The sql id.
    *    @param   displayClass [OUT] Will be set to display class if
    *                          successful.
    *    @return  True if the outparameter was succesfully set, i.e.
    *             a citycentre display class was found for the
    *             specified sql id.
    *                          
    */
   bool getDisplayClassForCC( uint32 poiID, uint8& displayClass );

   /**
    *   The map that has the information that is to be extracted
    *   to the feature map.
    */
   GenericMap* m_map;

private:
   /**
    *    Reads all the poi data from the database.
    */
   void precacheDisplayClasses();

   /// True if the citycentres have been cached
   bool m_poiInfoCached;
   
   ///   Citycentre display class by sql id.
   map<uint32, uint8> m_ccDisplayClassByPOIID;
   /// If this variable is non-null the cached data will be taken from there
   GfxFeatureMapProcBase* m_theOneWithTheData;
   
};

#endif
