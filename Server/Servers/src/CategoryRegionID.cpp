/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CategoryRegionID.h"
#include "ItemTypes.h"
#include <boost/lexical_cast.hpp>

CategoryRegionID::CategoryRegionID() 
: m_country(StringTable::ENGLAND_CC), m_valid( false ) {
}

CategoryRegionID::CategoryRegionID( StringTable::countryCode countryCode )
: m_country( countryCode ), m_valid( true ) {
}

MC2String CategoryRegionID::toString() const {
   if ( m_valid ) {
      return boost::lexical_cast<MC2String>( m_country );
   } else {
      return "invalid_region";
   }
}

bool CategoryRegionID::operator<( const CategoryRegionID& other ) const {
   if ( m_valid != other.m_valid ) {
      return m_valid < other.m_valid;
   }

   return m_country < other.m_country;
}

const CategoryRegionID CategoryRegionID::NO_REGION;
