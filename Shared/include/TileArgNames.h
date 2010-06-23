/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_FEATURE_IRGNUMES_H
#define TILE_FEATURE_IRGNUMES_H

/**
 *   Contains constants for the argument names.
 */
class TileArgNames {
public:
   enum tileArgName_t {
      coords       = 0,
      coord        = 1,
      name_type    = 2,
      level        = 3,
      level_1      = 4,
      image_name   = 5,
      radius       = 6,
      color        = 7,
      min_scale    = 8,
      max_scale    = 9,
      border_color = 10,
      font_type    = 11,
      font_size    = 12,
      width        = 13,
      border_width = 14,
      width_meters = 15,
      real_feature_type = 16, ///< the real feature type of the feature
      time         = 17, ///< time in UTC.
      ext_id       = 18, ///< unique external ID
      duration     = 19, ///< duration, ( resolution 5 min for events )
   };
};

#endif
