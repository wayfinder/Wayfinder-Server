/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GDCOLOR_H
#define GDCOLOR_H

#include "config.h"
#include "GDRGB.h"

namespace GDUtils {

/**
 * Color utilities for GDUtils
 */
namespace Color {

/**
 * The colors.
 */
enum imageColor {
   SNOW,
   GHOSTWHITE,
   WHITESMOKE,
   GAINSBORO,
   FLORALWHITE,
   OLDLACE,
   LINEN,
   ANTIQUEWHITE,
   PAPAYAWHIP,
   BLANCHEDALMOND,
   BISQUE,
   PEACHPUFF,
   NAVAJOWHITE,
   MOCCASIN,
   CORNSILK,
   IVORY,
   LEMONCHIFFON,
   SEASHELL,
   HONEYDEW,
   MINTCREAM,
   AZURE,
   ALICEBLUE,
   LAVENDER,
   LAVENDERBLUSH,
   MISTYROSE,
   WHITE,
   BLACK,
   DARKSLATEGREY,
   DIMGREY,
   SLATEGREY,
   LIGHTSLATEGREY,
   GREY,
   LIGHTGREY,
   MIDNIGHTBLUE,
   NAVYBLUE,
   CORNFLOWERBLUE,
   DARKSLATEBLUE,
   SLATEBLUE,
   MEDIUMSLATEBLUE,
   LIGHTSLATEBLUE,
   MEDIUMBLUE,
   ROYALBLUE,
   BLUE,
   DODGERBLUE,
   DEEPSKYBLUE,
   SKYBLUE,
   LIGHTSKYBLUE,
   STEELBLUE,
   LIGHTSTEELBLUE,
   LIGHTBLUE,
   POWDERBLUE,
   PALETURQUOISE,
   DARKTURQUOISE,
   MEDIUMTURQUOISE,
   TURQUOISE,
   CYAN,
   LIGHTCYAN,
   CADETBLUE,
   MEDIUMAQUAMARINE,
   AQUAMARINE,
   DARKGREEN,
   DARKOLIVEGREEN,
   DARKSEAGREEN,
   SEAGREEN,
   MEDIUMSEAGREEN,
   LIGHTSEAGREEN,
   PALEGREEN,
   SPRINGGREEN,
   LAWNGREEN,
   GREEN,
   CHARTREUSE,
   MEDIUMSPRINGGREEN,
   GREENYELLOW,
   LIMEGREEN,
   YELLOWGREEN,
   FORESTGREEN,
   OLIVEDRAB,
   DARKKHAKI,
   KHAKI,
   PALEGOLDENROD,
   LIGHTGOLDENRODYELLOW,
   LIGHTYELLOW,
   YELLOW,
   GOLD,
   LIGHTGOLDENROD,
   GOLDENROD,
   DARKGOLDENROD,
   ROSYBROWN,
   INDIANRED,
   SADDLEBROWN,
   SIENNA,
   PERU,
   BURLYWOOD,
   BEIGE,
   WHEAT,
   SANDYBROWN,
   TAN,
   CHOCOLATE,
   FIREBRICK,
   BROWN,
   DARKSALMON,
   SALMON,
   LIGHTSALMON,
   ORANGE,
   DARKORANGE,
   CORAL,
   LIGHTCORAL,
   TOMATO,
   ORANGERED,
   RED,
   HOTPINK,
   DEEPPINK,
   PINK,
   LIGHTPINK,
   PALEVIOLETRED,
   MAROON,
   MEDIUMVIOLETRED,
   VIOLETRED,
   MAGENTA,
   VIOLET,
   PLUM,
   ORCHID,
   MEDIUMORCHID,
   DARKORCHID,
   DARKVIOLET,
   BLUEVIOLET,
   PURPLE,
   MEDIUMPURPLE,
   THISTLE,
   DARKGREY,
   DARKBLUE,
   DARKCYAN,
   DARKMAGENTA,
   DARKRED,
   LIGHTGREEN,
   YELLOWISH,
   SKINTONED,
   NEWDARKGREEN,
   NEWDARKBLUE,

   LIGHTGREY_1,
   LIGHTGREY_2,
   LIGHTGREY_3,

   ORANGEGOLD,
   FIREORANGE,
   PALEWHITE,
   GREY_1,
   ALMOSTWHITE,
   FINEBLUE,
   GREYSNOW,
   GREYCLOUD,
   BLUESEA,
   FOREST,
   BUILDINGGREY,
   DARKGREY_2,
   LIGHTGREY_4,

   STREET_1_BORDER_A,
   STREET_1_BORDER_B,
   ORANGEBROWN,

   ALMOSTBLACK,
   ALMOSTRED_2,
   LIGHTBLUE_2,
   DIMGREY_2,
   ALMOSTRED_1,

   XL_ROAD,
   XL_ROAD_BORDER,
   L_ROAD,
   L_ROAD_BORDER,
   M_ROAD,
   M_ROAD_BORDER,
   S_ROAD,
   S_ROAD_BORDER,
   XS_ROAD,
   XS_ROAD_BORDER,
   WATER,
   LAND,
   BUILDING_AREA,
   BUILDING_OUTLINE,
   BUA,
   RAILWAY,
   ISLAND,
   BORDER,
   AIRPORT_GROUND,
   NEW_FOREST,

   AIRCRAFT_ROAD,
   WALKWAY,
   WALKWAY_BORDER,
   BUA_OUT,

   NBR_COLORS
};

// do not use these directly!
extern const int s_colors[][3];

/**
 * @param color to get the red value from
 * @return red value of the color
 */
inline int getRed( imageColor color ) {
   return s_colors[ color ][ 0 ];
}

/**
 * @param color to get the blue value from
 * @return green value of the color
 */
inline int getGreen( imageColor color ) {
   return s_colors[ color ][ 1 ];
}

/**
 * @param color the color to get the blue value from
 * @return blue value of the color
 */
inline int getBlue( imageColor color ) {
   return s_colors[ color ][ 2 ];
}

/**
 * Returns rgb of the color
 * @param color
 * @param rgb return value
 */
inline void getRGB( imageColor color, RGB& rgb ) {
   rgb.red = getRed( color );
   rgb.green = getGreen( color );
   rgb.blue = getBlue( color );
}

/**
 * Returns rgb of the color
 * @param color
 * @return rgb
 */
inline RGB getRGB( imageColor color ) {
   return RGB( getRed( color ), getGreen( color ), getBlue( color ) );
}

/// Temporary hack until we converted all color enums to int
enum CoolColor {
   VALUE = 0
};

/**
 * Convert image color to the hack with CoolColor enum
 */
inline CoolColor makeColor( imageColor color ) {
   return
      static_cast< CoolColor >( ( ( getRed( color ) << 16 ) & 0xFF0000 ) |
                                ( ( getGreen( color ) << 8 ) & 0x00FF00 ) |
                                ( ( getBlue( color ) ) & 0x0000FF ) );
}

} // Color

} // GDUtils

#endif // GDCOLOR_H
