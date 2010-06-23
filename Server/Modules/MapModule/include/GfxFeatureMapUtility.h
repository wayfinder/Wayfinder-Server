/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXFEATUREMAPUTILITY_H
#define GFXFEATUREMAPUTILITY_H

#include "config.h"
#include "MC2String.h"

class GenericMap;
class PointOfInterestItem;
class MapSettings;

namespace GfxFeatureMapUtility {

/**
 * Get extra information about a point of interest item, such as image
 * name and some generic extra poi info.
 * @param poi the point of interest item to fetch extra data for.
 * @param map the map in which the poi exist.
 * @param extraPOIInfo additional poi information to be returned.
 * @param imageName the image name to be returned
 */
void getExtraInfoForPOI( const PointOfInterestItem& poi,
                         const GenericMap& map,
                         byte& extraPOIInfo,
                         MC2String& imageName );

/**
 * Extra check to remove pois before checking the coordinates.
 * @param poi the item to check
 * @param theMap the map in which the item exists.
 * @param mapSettings settings for the gfx map to be created.
 * @param includeFreePOIs whether or not to include "free" pois even if map
 *                        right is set.
 */
bool checkPOIExtra( const PointOfInterestItem& poi,
                    const GenericMap& theMap,
                    const MapSettings& mapSettings,
                    bool includeFreePOIs );
}
#endif //  GFXFEATUREMAPUTILITY_H
