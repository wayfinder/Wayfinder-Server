/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAP_DRAWER_EXPORT_H
#define MAP_DRAWER_EXPORT_H

#include "config.h"

#include "DrawSettings.h"

class GfxFeatureMap;
class MapSettings;
class DrawingProjection;
class DataBuffer;

namespace MapDrawerExport {

/**
 * Creates a Export version of a GfxFeatureMap.
 * @param map The GfxFeatureMap to export.
 * @param mapSettings The MapSettings if any, may be NULL.
 * @param bool singleDraw If the export map should contain duplicates
 *        for polygons that should be drawn twice.
 * @param bool initText If text should be initialized. Default true.
 * @return A new DataBuffer with the export map, 
 *         or NULL if bad input.
 */
DataBuffer* makeExportGfxFeatureMap( GfxFeatureMap* map,
                                     MapSettings* mapSettings,
                                     bool singleDraw,
                                     const DrawingProjection* projection,
                                     bool initText = true );

/**
 * Creates a Export version of a GfxFeatureMap keeping all scalelevels.
 * @param map The GfxFeatureMap to export.
 * @param mapSettings The MapSettings if any, may be NULL.
 * @param bool singleDraw If the export map should contain duplicates
 *        for polygons that should be drawn twice.
 * @param bool initText If text should be initialized. Default true.
 * @return A new DataBuffer with the export map, 
 *         or NULL if bad input.
 */
DataBuffer* makeExportScalableGfxFeatureMap( GfxFeatureMap* map,
                                             MapSettings* mapSettings,
                                             bool singleDraw,
                                             bool initText = true );


} // MapDrawerExport

#endif // MAP_DRAWER_EXPORT_H
