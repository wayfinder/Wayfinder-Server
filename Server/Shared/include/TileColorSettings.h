/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILECOLORSETTINGS_H
#define TILECOLORSETTINGS_H

#include "config.h"

#include "ServerTileMapFormatDesc.h"
#include <map>

namespace TileColorSettings {
/**
 * Some common settings. For example text color.
 */
struct BasicSetting {
   BasicSetting():
      m_textColor( MAX_UINT32 ),
      m_topHorizonColor( MAX_UINT32 ),
      m_bottomHorizonColor( MAX_UINT32 ) {
   }

   uint32 m_textColor;
   uint32 m_topHorizonColor;
   uint32 m_bottomHorizonColor;
};

/**
 * Save color settings to a XML file.
 * @param filename The name of the XML file.
 * @param settings
 * @return true on success.
 */
bool saveSettings( const MC2String& filename,
                   const map< int, TileDrawSettings >& settingsByType,
                   const BasicSetting& setting );

/**
 * Load settings from a XML file.
 * @param filename The name of the XML file.
 * @param settingByType
 * @param setting Basic settings such as text color.
 * @return true on success, else false and the input values remains the same.
 */
bool loadSettings( const MC2String& filename,
                   map< int, TileDrawSettings >& settingsByType,
                   BasicSetting& setting );

} // TileColorSettings

#endif // TILECOLORSETTINGS_H
