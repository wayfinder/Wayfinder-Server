/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#ifndef TILEMAPCOORD_H
#define TILEMAPCOORD_H

#include <vector>

/**
 *   One coordinate of an item.
 */
class TileMapCoord : public MC2Coordinate {
public:
   inline TileMapCoord();
   
   inline TileMapCoord( const MC2Coordinate& other ) :
         MC2Coordinate( other ) {}
   
   inline TileMapCoord( int32 lat, int32 lon );
   
   inline void setCoord( int32 lat, int32 lon );
   
   inline int32 getLat() const; 
   
   inline int32 getLon() const;
   
   /// Equal operator.
   inline bool operator == ( const TileMapCoord& coord ) const;
   /// Not equal operator.
   inline bool operator != ( const TileMapCoord& coord ) const;
};

class TileMapCoords : public vector<TileMapCoord> {
};


// --- Inlined methods ---

inline 
TileMapCoord::TileMapCoord() : MC2Coordinate(bool(false)) {}

inline void 
TileMapCoord::setCoord( int32 plat, int32 plon ) 
{
   lat = plat;
   lon = plon;
}

inline
TileMapCoord::TileMapCoord( int32 lat, int32 lon ) 
{
   setCoord( lat, lon );
}
  

inline int32 
TileMapCoord::getLat() const 
{
   return lat;
}

inline int32 
TileMapCoord::getLon() const 
{
   return lon;
}

inline bool 
TileMapCoord::operator == ( const TileMapCoord& coord ) const
{
   return MC2Coordinate::operator==(coord);
}

inline bool 
TileMapCoord::operator != ( const TileMapCoord& coord ) const
{
   return MC2Coordinate::operator!=(coord);
}


#endif
