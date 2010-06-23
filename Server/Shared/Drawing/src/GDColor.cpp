/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GDColor.h"
namespace GDUtils {
namespace Color {

const int s_colors[ NBR_COLORS ][3] = {
   { 255, 250, 250 },  // SNOW
   { 248, 248, 255 },  // GHOSTWHITE
   { 245, 245, 245 },  // WHITESMOKE
   { 220, 220, 220 },  // GAINSBORO
   { 255, 250, 240 },  // FLORALWHITE
   { 253, 245, 230 },  // OLDLACE
   { 250, 240, 230 },  // LINEN
   { 250, 235, 215 },  // ANTIQUEWHITE
   { 255, 239, 213 },  // PAPAYAWHIP
   { 255, 235, 205 },  // BLANCHEDALMOND
   { 255, 228, 196 },  // BISQUE
   { 255, 218, 185 },  // PEACHPUFF
   { 255, 222, 173 },  // NAVAJOWHITE
   { 255, 228, 181 },  // MOCCASIN
   { 255, 248, 220 },  // CORNSILK
   { 255, 255, 240 },  // IVORY
   { 255, 250, 205 },  // LEMONCHIFFON
   { 255, 245, 238 },  // SEASHELL
   { 240, 255, 240 },  // HONEYDEW
   { 245, 255, 250 },  // MINTCREAM
   { 240, 255, 255 },  // AZURE
   { 240, 248, 255 },  // ALICEBLUE
   { 230, 230, 250 },  // LAVENDER
   { 255, 240, 245 },  // LAVENDERBLUSH
   { 255, 228, 225 },  // MISTYROSE
   { 255, 255, 255 },  // WHITE
   {   0,   0,   0 },  // BLACK
   {  47,  79,  79 },  // DARKSLATEGREY
   { 105, 105, 105 },  // DIMGREY
   { 112, 128, 144 },  // SLATEGREY
   { 119, 136, 153 },  // LIGHTSLATEGREY
   { 190, 190, 190 },  // GREY
   { 211, 211, 211 },  // LIGHTGREY
   {  25,  25, 112 },  // MIDNIGHTBLUE
   {   0,   0, 128 },  // NAVYBLUE
   { 100, 149, 237 },  // CORNFLOWERBLUE
   {  72,  61, 139 },  // DARKSLATEBLUE
   { 106, 100, 205 },  // SLATEBLUE
   { 123, 104, 238 },  // MEDIUMSLATEBLUE
   { 132, 112, 255 },  // LIGHTSLATEBLUE
   {   0,   0, 205 },  // MEDIUMBLUE
   {  65, 105, 225 },  // ROYALBLUE
   {   0,   0, 255 },  // BLUE
   {  30, 144, 255 },  // DODGERBLUE
   {   0, 191, 255 },  // DEEPSKYBLUE
   { 135, 206, 235 },  // SKYBLUE
   { 135, 206, 250 },  // LIGHTSKYBLUE
   {  70, 130, 180 },  // STEELBLUE
   { 176, 196, 222 },  // LIGHTSTEELBLUE
   { 173, 216, 230 },  // LIGHTBLUE
   { 176, 224, 230 },  // POWDERBLUE
   { 175, 238, 238 },  // PALETURQUOISE
   {   0, 206, 209 },  // DARKTURQUOISE
   {  72, 209, 204 },  // MEDIUMTURQUOISE
   {  64, 224, 208 },  // TURQUOISE
   {   0, 255, 255 },  // CYAN
   { 224, 255, 255 },  // LIGHTCYAN
   {  95, 158, 160 },  // CADETBLUE
   { 102, 205, 170 },  // MEDIUMAQUAMARINE
   { 127, 255, 212 },  // AQUAMARINE
   {   0,  90,   0 },  // DARKGREEN
   {  85, 107,  47 },  // DARKOLIVEGREEN
   { 143, 188, 143 },  // DARKSEAGREEN
   {  46, 139,  87 },  // SEAGREEN
   {  60, 179, 113 },  // MEDIUMSEAGREEN
   {  32, 178, 170 },  // LIGHTSEAGREEN
   { 152, 251, 152 },  // PALEGREEN
   {   0, 255, 127 },  // SPRINGGREEN
   { 124, 252,   0 },  // LAWNGREEN
   {   0, 255,   0 },  // GREEN
   { 127, 255,   0 },  // CHARTREUSE
   {   0, 250, 154 },  // MEDIUMSPRINGGREEN
   { 173, 255,  47 },  // GREENYELLOW
   {  50, 205,  50 },  // LIMEGREEN
   { 154, 205,  50 },  // YELLOWGREEN
   {  34, 139,  34 },  // FORESTGREEN
   { 107, 142,  35 },  // OLIVEDRAB
   { 189, 183, 107 },  // DARKKHAKI
   { 240, 230, 140 },  // KHAKI
   { 238, 232, 170 },  // PALEGOLDENROD
   { 250, 250, 210 },  // LIGHTGOLDENRODYELLOW
   { 255, 255, 224 },  // LIGHTYELLOW
   { 255, 255,   0 },  // YELLOW
   { 255, 215,   0 },  // GOLD
   { 238, 221, 130 },  // LIGHTGOLDENROD
   { 218, 165,  32 },  // GOLDENROD
   { 184, 134,  11 },  // DARKGOLDENROD
   { 188, 143, 143 },  // ROSYBROWN
   { 205,  92,  92 },  // INDIANRED
   { 139,  69,  19 },  // SADDLEBROWN
   { 160,  82,  45 },  // SIENNA
   { 205, 133,  63 },  // PERU
   { 222, 184, 135 },  // BURLYWOOD
   { 245, 245, 220 },  // BEIGE
   { 245, 222, 179 },  // WHEAT
   { 244, 164,  96 },  // SANDYBROWN
   { 210, 180, 140 },  // TAN
   { 210, 105,  30 },  // CHOCOLATE
   { 178,  34,  34 },  // FIREBRICK
   { 165,  42,  42 },  // BROWN
   { 233, 150, 122 },  // DARKSALMON
   { 250, 128, 114 },  // SALMON
   { 255, 160, 122 },  // LIGHTSALMON
   { 255, 165,   0 },  // ORANGE
   { 255, 140,   0 },  // DARKORANGE
   { 255, 127,  80 },  // CORAL
   { 240, 128, 128 },  // LIGHTCORAL
   { 255,  99,  71 },  // TOMATO
   { 255,  69,   0 },  // ORANGERED
   { 255,   0,   0 },  // RED
   { 255, 105, 180 },  // HOTPINK
   { 255,  20, 147 },  // DEEPPINK
   { 255, 192, 203 },  // PINK
   { 255, 182, 193 },  // LIGHTPINK
   { 219, 112, 147 },  // PALEVIOLETRED
   { 176,  48,  96 },  // MAROON
   { 199,  21, 133 },  // MEDIUMVIOLETRED
   { 208,  32, 144 },  // VIOLETRED
   { 255,   0, 255 },  // MAGENTA
   { 238, 130, 238 },  // VIOLET
   { 221, 160, 221 },  // PLUM
   { 218, 112, 214 },  // ORCHID
   { 186,  85, 211 },  // MEDIUMORCHID
   { 153,  50, 204 },  // DARKORCHID
   { 148,   0, 211 },  // DARKVIOLET
   { 138,  43, 226 },  // BLUEVIOLET
   { 160,  32, 240 },  // PURPLE
   { 147, 112, 219 },  // MEDIUMPURPLE
   { 216, 191, 216 },  // THISTLE
   { 169, 169, 169 },  // DARKGREY
   {   0,   0, 139 },  // DARKBLUE
   {   0, 139, 139 },  // DARKCYAN
   { 139,   0, 139 },  // DARKMAGENTA
   { 139,   0,   0 },  // DARKRED
   { 144, 238, 144 },  // LIGHTGREEN
   { 252, 239, 170 },  // YELLOWISH
   { 255, 198, 186 },  // SKINTONED
   {  54,  73,  54 },  // NEWDARKGREEN
   {  35,  32,  53 },  // NEWDARKBLUE
   { 225, 225, 225 },  // LIGHTGREY_1
   { 235, 235, 235 },  // LIGHTGREY_2
   { 245, 245, 245 },  // LIGHTGREY_3

   { 255, 174,   0 },  // ORANGEGOLD
   { 255, 120,   0 },  // FIREORANGE

   { 255, 249, 195 },  // PALEWHITE
   { 160, 160, 160 },  // GREY_1

   { 250, 250, 250 },  // ALMOSTWHITE
   { 110, 178, 220 },  // FINEBLUE

   { 239, 239, 230 },  // GREYSNOW
   { 224, 214, 198 },  // GREYCLOUD
   { 188, 219, 239 },  // BLUESEA
   { 162, 198, 156 },  // FOREST
   { 189, 184, 182 },  // BUILDINGGREY
   { 109,  92,  92 },  // DARKGREY_2
   { 203, 203, 203 },  // LIGHTGREY_4

   { 213, 153, 103 }, // STREET_1_BORDER_A
   { 225, 163, 103 }, // STREET_1_BORDER_B
   { 255, 177,   79 }, // ORANGEBROWN

   { 23, 23, 23 },     // ALMOSTBLACK
   { 196, 1, 27 },     // ALMOSTRED_2
   { 178, 173, 244 },     // LIGHTBLUE_2
   { 110, 110, 110 },     // DIMGREY_2
   { 238, 0, 0 },     // ALMOSTRED_1
   { 0xFF, 0xC2, 0x00 },  // XL ROAD
   { 0xF3, 0xB1, 0x00 },  // XL ROAD BORDER
   { 0xFF, 0xC2, 0x00 },  // L ROAD
   { 0xF3, 0xB1, 0x00 },  // L ROAD BORDER
   { 0xF0, 0xDA, 0x00 },  // M ROAD
   { 0xd6, 0xba, 0x00 },  // M ROAD BORDER
   { 0xFF, 0xFF, 0xFF },  // S ROAD
   { 0xcc, 0xcc, 0xcc },  // S ROAD BORDER
   { 0xFF, 0xFF, 0xFF },  // XS ROAD
   { 0xcc, 0xcc, 0xcc },  // XS ROAD BORDER
   { 0x33, 0xAF, 0xE1 }, // WATER
   { 0xF2, 0xF2, 0xF2 }, // LAND
   { 0xcc, 0xcc, 0xcc }, // BUILDING AREA
   { 0xAd, 0xad, 0xad }, // BUILDING OUTLINE
   { 0xE2, 0xE2, 0xE2 }, // BUA
   { 0x3f, 0x3f, 0x3f }, // RAILWAY
   { 0xF3, 0xF2, 0xF2 }, // ISLAND
   { 0xbc, 0xbc, 0xbc }, // BORDER
   { 0xcc, 0xcc, 0xcc }, // AIRPORT GROUND
   { 0x58, 0x9e, 0x60 }, // NEW_FOREST
   { 0x9C, 0x96, 0x97 }, // AIRCRAFT_ROAD
   { 0xd5, 0xd9, 0x9c }, // WALKWAY
   { 0xCC, 0xCC, 0xCC }, // WALKWAY BORDER
   { 0xd6, 0xd6, 0xd6 }, // BUA_OUT 
   
};

}
}
