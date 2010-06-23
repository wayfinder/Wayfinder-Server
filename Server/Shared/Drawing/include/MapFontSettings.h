/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAP_FONT_SETTINGS_H
#define MAP_FONT_SETTINGS_H

#include "config.h"

#include "GDColor.h"

class GfxFeature;
class MapSetting;

namespace MapFontSettings {
/**
 * Determine font name and text color for the feature.
 * @param feature The feature to fetch font name and text color for.
 * @param fontName will return font name.
 * @param color will return text color.
 */
void getFontNameAndColor( const GfxFeature* feature,
                          const char*& fontName,
                          GDUtils::Color::CoolColor& color );
/**
 * @param feature the feature to fetch font size for.
 * @param scaleLevel the scaleLevel to get the font size for.
 * @param smallImage whether current image is a small image.
 */
int32 getFontSize( const GfxFeature* feature, int32 scaleLevel, bool smallImage );

/**
 * Get font name, font size and text color for feature.
 * @param feature The feature to fetch settings for.
 * @param fontName Returns font name for feature.
 * @param fontSize Returns font size for feature.
 * @param color Returns text color for feature.
 * @param scaleLevel The scale level to get the settings for.
 * @param smallImage Whether this is for a small image or not.
 */
void getFontSettings( const GfxFeature* feature, 
                      const MapSetting* settings,
                      const char*& fontName,
                      int32& fontSize,
                      GDUtils::Color::CoolColor& color,
                      int32 scaleLevel,
                      bool smallImage = false );

} // MapFontSettings


#endif // MAP_FONT_SETTINGS_H
