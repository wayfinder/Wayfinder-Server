/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FILTERCOORDINATE_H
#define FILTERCOORDINATE_H


/**
 *    Utility methods for managing e.g. filter levels in mc2 coordinates.
 *    Static cast MC2Coordinate to FilteredCoord and call the get/set
 *    methods.
 */
class FilteredCoord : public MC2Coordinate {

   public:
      
      /**
       *    Get the filter level for a coordinate. The level is parsed
       *    from the last bits of latitude + the last bits of longitude.
       *    Currently the last 2 bits from each of lat and long is used,
       *    thus level is 0-15.
       */
      inline uint8 getFilterLevel( ) const;

      /**
       *    Static version to get the filter level of a coordinate.
       */
      static inline uint8 getFilterLevel( const MC2Coordinate& coord );
      
      /**
       *    Set filter level in a coordinate. The level is stored 
       *    combining the 2 last bits of each latitude and longitude,
       *    thus level 0-15 is acceptable.
       *    @param   level    The filter level to set in this mc2 coord, value
       *                      between 0-15.
       *    @return  True if level was set, false if not (e.g. if level
       *             was another value than 0-15).
       */
      inline bool setFilterLevel( uint8 level );
      
      
   private:
   
      /**
       *   Empty constructor sets lat and lon to MAX_INT32
       */
      FilteredCoord()
            : MC2Coordinate( bool(false) ) {
      }

};

// ========================================================================
//                                      Implementation of inlined methods =

inline uint8
FilteredCoord::getFilterLevel( const MC2Coordinate& coord )
{
   // Last 2 bits of latitude and longitude are used for storing filter level
   
   //uint32 nrOflsbsToBeUsed = 2;
   //int lsbMask = ( 0xffffffff >> nrOflsbsToBeUsed ) << nrOflsbsToBeUsed;
   //int lsbInvertMask = 0xffffffff - lsbMask;
   
   uint8 filterLevel = uint8( coord.lat & 0x00000003 );
   filterLevel = (filterLevel << 2);
   filterLevel += uint8( coord.lon & 0x00000003 );
   
   return filterLevel;
}

inline uint8 FilteredCoord::getFilterLevel() const
{
   return getFilterLevel(*this);
}


inline bool
FilteredCoord::setFilterLevel( uint8 level )
{
   // Last 2 bits of latitude and longitude are used for storing filter level
   // This makes level > 15 impossible
   if ( level > 15 )
      return false;
   
   //uint32 nrOflsbsToBeUsed = 2;
   //int lsbMask = ( 0xffffffff >> nrOflsbsToBeUsed ) << nrOflsbsToBeUsed;
   //int setLevelMask = 0xffffffff - lsbMask + 1;
   //// level 3 = 0011, level 4 = 0100, level 15 = 1111
   
   int lsbMask = 0xfffffffc;
   int setLevelMask = 0x00000004;
   
   lat = ( lat & lsbMask ) + ( level / setLevelMask );
   lon = ( lon & lsbMask ) + ( level % setLevelMask );
   
   return true;
}

#endif
