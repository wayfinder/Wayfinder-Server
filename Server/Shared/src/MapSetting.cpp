/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapSetting.h"
#include "StringUtility.h"
#include "Packet.h"


MapSetting::MapSetting( GfxFeature::gfxFeatureType featureType,
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
                        bool copyString )
      : m_featureType( featureType ), 
   m_highScaleLevel( highScaleLevel ), 
   m_lowScaleLevel( lowScaleLevel ), 
   m_onMap( onMap ), m_borderOnMap( borderOnMap ),
   m_textOnMap( textOnMap ),
   m_drawStyle( drawStyle ), m_drawSymbol( drawSymbol ),
   m_lineWidth( lineWidth ), m_drawColor( drawColor ),
   m_borderLineWidth( borderLineWidth ), 
   m_borderColor( borderColor ),
   m_fontSize( fontSize ), m_fontColor( fontColor )
{
   m_copyString = copyString;
   if ( m_copyString ) {
      m_fontName = StringUtility::newStrDup( fontName );
   } else {
      m_fontName = fontName;
   }
}


MapSetting::MapSetting() 
      : m_featureType( GfxFeature::NBR_GFXFEATURES ), 
   m_highScaleLevel( 0 ), 
   m_lowScaleLevel( 0 ), 
   m_onMap( false ), m_borderOnMap( false ),
   m_textOnMap( false ),
   m_drawStyle( DrawSettings::LINE ), 
   m_drawSymbol( DrawSettings::SQUARE_3D_SYMBOL ),
   m_lineWidth( 0 ),
   m_drawColor( static_cast<GDUtils::Color::CoolColor>( 0x000000 ) ),
   m_borderLineWidth( 0 ), 
   m_borderColor( static_cast<GDUtils::Color::CoolColor>( 0x000000 ) ),
   m_fontSize( 0 ),
   m_fontColor( static_cast<GDUtils::Color::CoolColor>( 0x000000 ) )
{
   m_copyString = false; 
   m_fontName = NULL;
}

MapSetting::MapSetting( const MapSetting& other, bool copyString )
      : m_featureType( other.m_featureType ), 
   m_highScaleLevel( other.m_highScaleLevel ), 
   m_lowScaleLevel( other.m_lowScaleLevel ), 
   m_onMap( other.m_onMap ), m_borderOnMap( other.m_borderOnMap ),
   m_textOnMap( other.m_textOnMap ),
   m_drawStyle( other.m_drawStyle ), m_drawSymbol( other.m_drawSymbol ),
   m_lineWidth( other.m_lineWidth ), m_drawColor( other.m_drawColor ),
   m_borderLineWidth( other.m_borderLineWidth ), 
   m_borderColor( other.m_borderColor ),
   m_fontSize( other.m_fontSize ), m_fontColor( other.m_fontColor )
{
   m_copyString = copyString;
   if ( m_copyString ) {
      m_fontName = StringUtility::newStrDup( other.m_fontName );
   } else {
      m_fontName = other.m_fontName;
   }
}



MapSetting::~MapSetting() {
   if ( m_copyString ) {
      delete [] m_fontName;
   }
}


void
MapSetting::saveToPacket( Packet* p, int& pos ) const {
   // featureType
   p->incWriteShort( pos, m_featureType );
   // highScaleLevel
   p->incWriteByte( pos, m_highScaleLevel );
   // lowScaleLevel
   p->incWriteByte( pos, m_lowScaleLevel );
   // onMap
   p->incWriteByte( pos, m_onMap );
   // borderOnMap
   p->incWriteByte( pos, m_borderOnMap );
   // textOnMap
   p->incWriteByte( pos, m_textOnMap );
   // drawStyle
   p->incWriteByte( pos, m_drawStyle );
   // drawSymbol
   p->incWriteByte( pos, m_drawSymbol );
   // lineWidth
   p->incWriteByte( pos, m_lineWidth );
   // drawColor
   p->incWriteLong( pos, m_drawColor );
   // borderColor
   p->incWriteLong( pos, m_borderColor );
   // borderLineWidth
   p->incWriteByte( pos, m_borderLineWidth );
   // fontSize
   p->incWriteByte( pos, m_fontSize );
   // fontColor
   p->incWriteLong( pos, m_fontColor );
   // fontName
   p->incWriteString( pos, m_fontName );
}


void
MapSetting::loadFromPacket( const Packet* p, int& pos, bool copyString ) {
   if ( m_copyString ) {
      delete m_fontName;
   }
   m_copyString = copyString;

   // featureType
   m_featureType = GfxFeature::gfxFeatureType( p->incReadShort( pos ) );
   // highScaleLevel
   m_highScaleLevel = p->incReadByte( pos );
   // lowScaleLevel
   m_lowScaleLevel = p->incReadByte( pos );
   // onMap
   m_onMap = p->incReadByte( pos );
   // borderOnMap
   m_borderOnMap = p->incReadByte( pos );
   // textOnMap
   m_textOnMap = p->incReadByte( pos );
   // drawStyle
   m_drawStyle = DrawSettings::drawstyle_t( p->incReadByte( pos ) );
   // drawSymbol
   m_drawSymbol = DrawSettings::symbol_t( p->incReadByte( pos ) );
   // lineWidth
   m_lineWidth = p->incReadByte( pos );
   // drawColor
   m_drawColor = static_cast<GDUtils::Color::CoolColor>
      ( p->incReadLong( pos ) );
   // borderColor
   m_borderColor = static_cast<GDUtils::Color::CoolColor>
      ( p->incReadLong( pos ) );
   // borderLineWidth
   m_borderLineWidth = p->incReadByte( pos );
   // fontSize
   m_fontSize = p->incReadByte( pos );
   // fontColor
   m_fontColor = static_cast<GDUtils::Color::CoolColor>
      ( p->incReadLong( pos ) );
   // fontName
   char* fontName = NULL;
   p->incReadString( pos, fontName );
   if ( m_copyString ) {
      m_fontName = StringUtility::newStrDup( fontName );
   } else {
      m_fontName = fontName;
   }
}
