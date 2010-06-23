/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPSETTING_H
#define MAPSETTING_H

#include "config.h"
#include "GfxFeature.h"
#include "DrawSettings.h"

class Packet;


/**
 * Class for holding a mapsetting, not intended for any use but very
 * close to MapSettings.
 *
 */
class MapSetting {
   public:
      /**
       * Constructs new MapSetting.
       */
      MapSetting( GfxFeature::gfxFeatureType featureType,
                  uint32 highScaleLevel,
                  uint32 lowScaleLevel,
                  bool onMap,
                  bool borderOnMap,
                  bool textOnMap,
                  DrawSettings::drawstyle_t drawStyle,
                  DrawSettings::symbol_t drawSymbol,
                  int lineWidth,
                  GDUtils::Color::CoolColor drawColor,
                  int borderLineWidth,
                  GDUtils::Color::CoolColor borderColor,
                  int fontSize,
                  GDUtils::Color::CoolColor fontColor,
                  const char* fontName,
                  bool copyString = false );

      
      /**
       * Constructs new empty MapSetting.
       */
      MapSetting();


      /**
       * Copyconstructor with optional copy of string.
       */
      MapSetting( const MapSetting& other, bool copyString = false );

      /**
       * Deletes the fontName string if copyString is true.
       */
      ~MapSetting();

      
      /**
       * The maximum size of the MapSetting in a Packet.
       */
      inline uint32 getSize() const;

      
      /**
       * Saves the MapSetting to a packet.
       */
      void saveToPacket( Packet* p, int& pos ) const;

      
      /**
       * Loads the content of the packet.
       * @param p The packet to read from.
       * @param pos The position in p to start at.
       * @param copyString If strings from packet should be copied, default
       *        false.
       */
      void loadFromPacket( const Packet* p, int& pos, bool copyString = false );
      
      /**
       * Dumps the contents of the MapSetting to cerr, parameter not
       * standard error.
       */
      void dump( ostream& cerr ) const {
         cerr << "   featureType " << int(m_featureType) << endl
              << "   highScaleLevel " << m_highScaleLevel << endl
              << "   lowScaleLevel " << m_lowScaleLevel << endl
              << "   onMap " << m_onMap << endl
              << "   borderOnMap " << m_borderOnMap << endl
              << "   textOnMap " << m_textOnMap << endl
              << "   drawStyle " << int(m_drawStyle) << endl
              << "   drawSymbol " << int(m_drawSymbol) << endl
              << "   lineWidth " << m_lineWidth << endl
              << "   drawColor " << int( m_drawColor ) << endl
              << "   borderLineWidth " << m_borderLineWidth << endl
              << "   borderColor " << int( m_borderColor ) << endl
              << "   fontSize " << m_fontSize << endl
              << "   fontColor " << int(m_fontColor) << endl
              << "   fontName " << m_fontName << endl;
      }
            
      // private: no! not private public so we can use . operator.
      GfxFeature::gfxFeatureType m_featureType;
      uint32 m_highScaleLevel;
      uint32 m_lowScaleLevel;
      bool m_onMap;
      bool m_borderOnMap;
      bool m_textOnMap;
      DrawSettings::drawstyle_t m_drawStyle;
      DrawSettings::symbol_t m_drawSymbol;
      int m_lineWidth;
      GDUtils::Color::CoolColor m_drawColor;
      int m_borderLineWidth;
      GDUtils::Color::CoolColor m_borderColor;
      int m_fontSize;
      GDUtils::Color::CoolColor m_fontColor;
      const char* m_fontName;
      bool m_copyString;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline uint32 
MapSetting::getSize() const {
   // Comment lines and return lines match
   // featureType,highScaleLevel,lowScaleLevel,onMap,borderOnMap,
   // textOnMap,drawStyle,drawSymbol,lineWidth,drawColor,borderColor,
   // borderLineWidth,fontSize,fontColor,fontName + possible padding
   return 2+1+1+1+1+
      1+1+1+1+2+2+
      1+1+2+strlen( m_fontName ) + 1 + 3;
}


#endif // MAPSETTING_H
