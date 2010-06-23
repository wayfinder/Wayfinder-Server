/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_MAP_PARAMTYPES_H
#define TILE_MAP_PARAMTYPES_H

#include "config.h"

class TileMapParamTypes {
public:
   enum param_t {
      /// Data or text map
      TILE,
      /// Icon.
      BITMAP,
      /// TileMapFormatDesc
      FORMAT_DESC,
      /// CRC for the TileMapFormatDesc
      FORMAT_DESC_CRC,
      /// Holds several maps
      BUFFER_HOLDER,
      UNKNOWN,
   };

   static inline param_t getParamType( const char* str );
   
   static inline param_t getParamType( const MC2SimpleString& str );

   static inline bool isMapFormatDesc( const char* paramsStr );
   
   static inline bool isMap( const char* paramsStr );
   
   static inline bool isBitmap( const char* paramsStr );
   
   static inline bool isOldBitmap( const char* paramsStr );

   static inline bool isBufferHolder( const char* paramsStr );
   
   static inline bool isBufferHolder( const MC2SimpleString& str );

   /// @return Whether param string is image that wants a custom size
   static inline bool isCustomImageSize( const char* paramStr );
   /// @return Whether param string is image that wants a custom size and cropped
   static inline bool isCustomImageSizeCropped( const char* paramStr );
};

inline TileMapParamTypes::param_t
TileMapParamTypes::getParamType( const char* str )
{
   switch ( str[0] ) {
      case 'G': // Features
      case 'T': // Strings
         return TILE;
      case 'Q':
      case 'q':
      case 'B':
      case 'b':
         return BITMAP;
      case 'D':
      case 'd': // night mode
         return FORMAT_DESC;
      case 'C':
      case 'c': // night mode
         return FORMAT_DESC_CRC;
      case 'N':
         return BUFFER_HOLDER;
   }
   return UNKNOWN;
}

inline TileMapParamTypes::param_t
TileMapParamTypes::getParamType( const MC2SimpleString& str )
{
   return getParamType( str.c_str() );
}

inline bool 
TileMapParamTypes::isMapFormatDesc( const char* paramsStr )
{
   return TileMapParamTypes::getParamType( paramsStr ) ==
      TileMapParamTypes::FORMAT_DESC ||
      TileMapParamTypes::getParamType( paramsStr ) ==
      TileMapParamTypes::FORMAT_DESC_CRC;
}

inline bool 
TileMapParamTypes::isMap( const char* paramsStr )
{
   return TileMapParamTypes::getParamType( paramsStr ) ==
      TileMapParamTypes::TILE;
}

inline bool 
TileMapParamTypes::isBitmap( const char* paramsStr )
{
   return TileMapParamTypes::getParamType( paramsStr ) ==
      TileMapParamTypes::BITMAP;
}

inline bool 
TileMapParamTypes::isOldBitmap( const char* paramsStr )
{
   return paramsStr[ 0 ] == 'B';
}

inline bool
TileMapParamTypes::isBufferHolder( const char* paramStr )
{
   return getParamType( paramStr ) == TileMapParamTypes::BUFFER_HOLDER;
}

inline bool
TileMapParamTypes::isBufferHolder( const MC2SimpleString& str )
{
   return isBufferHolder( str.c_str() );
}

inline bool
TileMapParamTypes::isCustomImageSize( const char* paramsStr ) {
   return
      paramsStr[ 0 ] == 'Q' ||
      isCustomImageSizeCropped( paramsStr );
}

inline bool
TileMapParamTypes::isCustomImageSizeCropped( const char* paramsStr ) {
   return paramsStr[ 0 ] == 'q';
}

#endif
