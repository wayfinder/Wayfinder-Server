/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DRAWSETTINGS_H
#define DRAWSETTINGS_H

#include "config.h"
#include "GDColor.h"
#include "GfxFeature.h"

/**
 *    Objects of this class contains data for how to draw an item on the
 *    screen. Works almost like a struct, the only differance is that the
 *    members are resetted when created.
 *
 */
class DrawSettings {
   public:
      enum drawstyle_t {
         LINE,
         CLOSED,
         FILLED,
         SYMBOL
      };

      enum symbol_t {
         ROUTE_ORIGIN_SYMBOL,
         ROUTE_DESTINATION_SYMBOL,
         ROUTE_PARK_SYMBOL,

         /// A 3-d square, e.g. used for big cities in less detailed levels 
         SQUARE_3D_SYMBOL,

         // A circle, e.g. used for medium cities in medium detailed levels 
         SMALL_CITY_SYMBOL,
         
         /// POI symbols 
         POI_SYMBOL,

         /// PIN
         PIN,
         
         /// USER_DEFINED, check symbolImage for precise image
         USER_DEFINED,

         /// DANGER
         DANGER,
         
         /// ROADWORK
         ROADWORK,

         /// Speed camera
         SPEED_CAMERA,

         /// User defined speed camera
         USER_DEFINED_SPEED_CAMERA,
      };

      enum poiStatus_t {
         /// Just a single POI
         singlePOI = 0,
         /// Several POIs of the same type
         multiSamePOI = 1,
         /// Several POIs of different type
         multiDifferentPOI = 2
      };
   
      /**
       *    Create a new draw settings, resets all members.
       */
      DrawSettings() { 
         reset(); 
      };
      
      /**
       *    Delete this draw settings. Nothing is allocated inside this
       *    class, so nothing will be deleted.
       */
      ~DrawSettings() { };
      
      /**
       *    Reset the value of all member variables.
       */
      inline void reset();

      /**
       *    Print all the members to standard out.
       */
      inline void dump();

      /**
       *    The color of the feature to draw on the screen.
       */
      GDUtils::Color::CoolColor m_color;

      /**
       *    True if the polygon should be filled, false otherwise.
       */
      drawstyle_t m_style;

      /**
       *    The width, in pixels, of the line.
       */
      int m_lineWidth;

      /**
       *    This is currently only used when the draw-style is SYMBOL
       */
      symbol_t m_symbol;
      
      /**
       * This is currently only used when the symbol type is POI.
       */
      poiStatus_t m_poiStatus;
      
      /**
       *    This is currently only used when the symbol-type is POI.
       */
      GfxFeature::gfxFeatureType m_featureType;

      /**
       * This is currently only used when the symbol-type is ROUTE_ORIGIN. 
       */
      ItemTypes::transportation_t m_transportationType;

      /**
       * This is currently only used when the symbol-type is ROUTE_ORIGIN. 
       */
      uint8 m_startingAngle;

      /**
       * This is currently only used when the symbol-type is ROUTE_ORIGIN. 
       */
      bool m_drivingOnRightSide;

      /**
       * This is currently only used when the symbol-type is ROUTE_ORIGIN. 
       */
      bool m_initialUTurn;

      /**
       * This is currently only used when the symbol-type is POI
       */
      ItemTypes::pointOfInterest_t m_poiType;

      /**
       * This is currently only used when the symbol-type is POI
       */
      bool m_multiplePOI;

      /**
       * This is currently only used when the symbol-type is USER_DEFINED.
       */
      MC2String m_symbolImage;
};    

// =======================================================================
//                                     Implementation of inlined methods =

inline void 
DrawSettings::reset() 
{
   m_color = static_cast<GDUtils::Color::CoolColor>( 0x000000 );
   m_lineWidth = 1;
   m_style = LINE;
};

inline void 
DrawSettings::dump()
{
   cerr << "color: " << int(m_color) << ", lineWidth: " << m_lineWidth
        << ", style: " << int(m_style) << endl;
}


#endif

