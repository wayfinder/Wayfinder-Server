/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapFontSettings.h"
#include "GfxFeature.h"
#include "MapSettings.h"

namespace MapFontSettings {

void getFontNameAndColor( const GfxFeature* feature,
                          const char*& fontName,
                          GDUtils::Color::CoolColor& color ) {
   switch( feature->getType() ) {
   case ( GfxFeature::STREET_MAIN ) :
   case ( GfxFeature::STREET_FIRST ) :
   case ( GfxFeature::STREET_SECOND ) :
   case ( GfxFeature::STREET_THIRD ) :
   case ( GfxFeature::STREET_FOURTH ) :
   case ( GfxFeature::FERRY ) :
      fontName = "Vera.ttf";
      color = GDUtils::Color::makeColor( GDUtils::Color::BLACK );
      break;
   case ( GfxFeature::LAND ) :
      fontName = "Vera.ttf";
      color = GDUtils::Color::makeColor( GDUtils::Color::BLACK );
      break;
   case ( GfxFeature::BUILTUP_AREA ) :
   case ( GfxFeature::BUILTUP_AREA_SQUARE ) :
   case ( GfxFeature::BUILTUP_AREA_SMALL ) :
   case ( GfxFeature::POI ) : {
      const GfxPOIFeature* poi =
         static_cast<const GfxPOIFeature*>(feature);
      if ( poi->getPOIType() == ItemTypes::cityCentre ) {
         byte extraPOIInfo = poi->getExtraInfo();
         if ( extraPOIInfo <= 2 ) {
            fontName = "VeraBd.ttf";
            color = GDUtils::Color::makeColor( GDUtils::Color::BLACK );
         } else if( extraPOIInfo <= 6 ) {
            fontName = "VeraBd.ttf";
            color = GDUtils::Color::makeColor( GDUtils::Color::BLACK );
         } else if( extraPOIInfo <= 8 ) {
            fontName = "VeraBd.ttf";
            color = GDUtils::Color::makeColor( GDUtils::Color::BLACK );
         } else if( extraPOIInfo <= 11 ) {
            fontName = "VeraBd.ttf";
            color = GDUtils::Color::makeColor( GDUtils::Color::BLACK );
         } else {
            fontName = "Vera.ttf";
            color = GDUtils::Color::makeColor( GDUtils::Color::BLACK );
         }
      } else {
         fontName = "VeraBd.ttf";
         color = GDUtils::Color::makeColor( GDUtils::Color::BLACK );
      }
      break;
   }

   case ( GfxFeature::PARK ) :
   case ( GfxFeature::NATIONALPARK ) :
   case ( GfxFeature::FOREST ) :
      fontName = "VeraIt.ttf";
      color = GDUtils::Color::makeColor( GDUtils::Color::NEWDARKGREEN );
      break;
   case ( GfxFeature::WATER ) :
   case ( GfxFeature::WATER_LINE ) :
      fontName = "VeraIt.ttf";
      color = GDUtils::Color::makeColor( GDUtils::Color::NEWDARKBLUE );
      break;
   case ( GfxFeature:: ISLAND ) :
      fontName = "VeraIt.ttf";
      color = GDUtils::Color::makeColor( GDUtils::Color::BLACK );
      break;

   case GfxFeature::CARTOGRAPHIC_GROUND:
   case GfxFeature::CARTOGRAPHIC_GREEN_AREA:
   case ( GfxFeature::BUILDING ) :
   case ( GfxFeature::INDIVIDUALBUILDING ) :
   case ( GfxFeature::PEDESTRIANAREA ) :
   case ( GfxFeature::AIRCRAFTROAD ) :
      fontName = "VeraBd.ttf";
      color = GDUtils::Color::makeColor( GDUtils::Color::BLACK );
      break;
   default:
      fontName = "Vera.ttf";
      color = GDUtils::Color::makeColor( GDUtils::Color::BLACK );
      break;
   }
}

/**
 * @param feature the feature to fetch font size for.
 * @param scaleLevel the scale level to get the font size for.
 * @param smallImage whether current image is a small image.
 */
int32 getFontSize( const GfxFeature* feature, int32 scaleLevel, bool smallImage ) {

   int32 fontSize = 8;
   
   // setup font size, NOTE: small image special case 
   // at end of function
   switch ( feature->getType() ) {
      // Areas with content (BUA, LAND)
   case GfxFeature::BUILTUP_AREA:
      fontSize = 12;
      if ( scaleLevel <= 1 ) { // continent and country
         fontSize = 11;
      }
      break;
   case GfxFeature::BUILTUP_AREA_SQUARE:
      fontSize = 12;
      break;
   case GfxFeature::BUILTUP_AREA_SMALL:
      fontSize = 10;
      break;
   case GfxFeature::LAND:
      fontSize = 12;
      if ( scaleLevel == 0 ) { // continent
         fontSize = 9;
      } else if (scaleLevel == 1 ) { // country
         fontSize = 11;
      }
      break;
   case GfxFeature::WATER_LINE:
      fontSize = 12;
      break;
   case GfxFeature::CARTOGRAPHIC_GREEN_AREA:
   case GfxFeature::CARTOGRAPHIC_GROUND:
   case GfxFeature::PARK:
   case GfxFeature::NATIONALPARK:
   case GfxFeature::FOREST:
   case GfxFeature::BUILDING:
   case GfxFeature::INDIVIDUALBUILDING:
      fontSize = 10;
      break;
   case GfxFeature::WATER:
   case GfxFeature::ISLAND:
   case GfxFeature::PEDESTRIANAREA:
   case GfxFeature::AIRCRAFTROAD:
      fontSize = 12;
      break;
   case GfxFeature::POI: {
      const GfxPOIFeature* poi = 
         static_cast< const GfxPOIFeature* >( feature );

      if ( poi->getPOIType() == ItemTypes::cityCentre ) {
         byte extraPOIInfo = poi->getExtraInfo();
         if ( extraPOIInfo <= 2 ) {
            fontSize = 12;
         } else if ( extraPOIInfo <= 6 ) {
            fontSize = 10;
         } else if ( extraPOIInfo <= 8 ) {
            fontSize = 9;
         } else if ( extraPOIInfo <= 11 ) {
            fontSize = 8;
         } else {
            fontSize = 8;
         }
      } else {
         fontSize = 10;
      }
   } break;
   default:
      break;
   }

   // Special case for small images
   if ( smallImage ) {
      switch (feature->getType()) {
      case GfxFeature::LAND:
         fontSize = 12;
         break;
      case GfxFeature::BUILTUP_AREA :
      case GfxFeature::BUILTUP_AREA_SQUARE :
      case GfxFeature::WATER_LINE:
      case GfxFeature::PARK:
      case GfxFeature::NATIONALPARK:
      case GfxFeature::FOREST:
      case GfxFeature::BUILDING:
      case GfxFeature::WATER:
      case GfxFeature::ISLAND:
      case GfxFeature::PEDESTRIANAREA:
      case GfxFeature::AIRCRAFTROAD:
         fontSize = 10;
         break;
      case GfxFeature::CARTOGRAPHIC_GROUND:
      case GfxFeature::CARTOGRAPHIC_GREEN_AREA:
      case GfxFeature::BUILTUP_AREA_SMALL :
      case GfxFeature::INDIVIDUALBUILDING :
         fontSize = 8;
         break;
      default:
         break;
      }
   }

   return fontSize;
}

void getFontSettings( const GfxFeature* feature, 
                      const MapSetting* settings,
                      const char*& fontName,
                      int32& fontSize,
                      GDUtils::Color::CoolColor& color,
                      int32 scaleLeve,
                      bool smallImage ) {
   if ( settings ) {      
      fontName = settings->m_fontName;
      color = settings->m_fontColor;
   } else {
      getFontNameAndColor( feature, fontName, color );
   }

   fontSize = getFontSize( feature, scaleLeve, smallImage );
}

}
