/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEFEATURESTRINGS_H
#define TILEFEATURESTRINGS_H

#include "config.h"
#include "TileFeature.h"

namespace TileFeatureStrings {
/**
 * Convert a tile feature type to a string.
 * @param type
 * @return tile feature type as string or "unknown" if the type
 *         was not recognized.
 */
const char* feat2str( uint32 type );

/**
 * Convert a string to a tile feature feature.
 * @param str
 * @return tile feature type or nbr_tile_features if the strings was not
 *         recognized as a valid tile feature.
 */
TilePrimitiveFeature::tileFeature_t str2feat( const char* str );

/**
 * Convert a string to a primitive.
 * @param str
 * @return primitive.
 */
TilePrimitiveFeature::tileFeature_t str2prim( const char* str );

/**
 * Convert a primitive type to a string.
 * @param prim
 * @return primitive as string.
 */
const char* prim2str( int prim );

} // TileFeatureStrings

#endif // TILEFEATURESTRINGS_H

