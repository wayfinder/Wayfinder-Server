/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTENDEDTILESTRING_H
#define EXTENDEDTILESTRING_H

#include "config.h"
#include "MC2String.h"

/**
 * Extended string type for tile maps.
 *
 */
class ExtendedTileString {
public:
   typedef uint16 FeatureIndex;
   /// Type of the extended string data.
   enum Type {
      PERFORMER      = 1, ///< group/artist/performer.
      INVALID_TYPE        ///< last name type, must be...last.
   };

   ExtendedTileString():
      m_featureIdx( 0 ),
      m_type( INVALID_TYPE ) {
   }

   ExtendedTileString( FeatureIndex featureIdx,
                       Type type,
                       const MC2String& data ):
      m_featureIdx( featureIdx ),
      m_type( type ),
      m_data( data ) {
   }

   /// @return Feature index that uses this string.
   FeatureIndex getFeatureIndex() const {
      return m_featureIdx;
   }

   /// @return Type of this string.
   Type getType() const {
      return m_type;
   }
   /// @return The string.
   const MC2String& getData() const {
      return m_data;
   }

private:
   FeatureIndex m_featureIdx; ///< feature that this string holds
   Type m_type; ///< type of the string
   MC2String m_data; ///< the string data
};

#endif // EXTENDEDTILESTRING_H
