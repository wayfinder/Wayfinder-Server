/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WINDOWSPOINT_H
#define WINDOWSPOINT_H

#ifndef _WIN32
// Define POINT
/**
 *    Planear point, name POINT is choosen as it allready exists in WIN32.
 *    Please use MC2Point instead.
 */
struct POINT{
   int32 x;
   int32 y;
};

inline ostream& operator<<( ostream& o, const POINT& point )
{
   return o << "{" << point.x << "," << point.y << "}";
}

#endif

inline bool operator==( const POINT& a, const POINT& b )
{
   return ( a.x == b.x ) && ( a.y == b.y );
}

inline bool operator!=( const POINT& a, const POINT& b )
{
   return !( a == b );
}

inline POINT makePoint( int32 x, int32 y )
{
   POINT tmp;
   tmp.x = x;
   tmp.y = y;
   return tmp;
}

namespace GfxUtil {
// For POINT
inline int32 getCoordX( const POINT& point ) {
   return point.x;
}

inline int32 getCoordY( const POINT& point ) {
   return point.y;
}

// For POINT*
inline int32 getCoordX( const POINT* point ) {
   return getCoordX( *point );
}

inline int32 getCoordY( const POINT* point ) {
   return getCoordY( *point );
}

}


#endif

