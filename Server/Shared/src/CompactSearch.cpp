/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CompactSearch.h"
#include "StringUtility.h"
#include "CategoryTree.h"
#include "StringSearchUtility.h"



bool 
CompactSearchHitType::operator==( const CompactSearchHitType& other ) const {
#define CSCMP(x) other.x == x
   return
      CSCMP( m_name ) &&
      CSCMP( m_nameStringCode ) &&
      CSCMP( m_type ) &&
      CSCMP( m_typeStringCode ) &&
      CSCMP( m_imageName ) &&
      CSCMP( m_round ) &&
      CSCMP( m_serviceID ) &&
      CSCMP( m_heading ) &&
      CSCMP( m_searchTypeMask ) &&
      CSCMP( m_topRegionID ) &&
      CSCMP( m_language ) &&
      CSCMP( m_mapRights ) &&
      CSCMP( m_invertRights ) &&
      CSCMP( m_providerType );
#undef CSCMP
         
}

CompactSearchHitType& 
CompactSearchHitType::operator+=( const CompactSearchHitType& other ) {
   bool isEmptyHitType = !isValid();
#define CSADD(x) x = isEmptyHitType ? other.x : x
   m_name = isEmptyHitType ? other.m_name : m_name + ":" + other.m_name;
   CSADD( m_nameStringCode );
   m_type = isEmptyHitType ? other.m_type : m_type + ":" + other.m_type;
   CSADD( m_typeStringCode );
   CSADD( m_imageName );
   CSADD( m_round );
   CSADD( m_serviceID );
   CSADD( m_heading );
   m_searchTypeMask = isEmptyHitType ? other.m_searchTypeMask : 
      ( other.m_searchTypeMask | m_searchTypeMask );
   CSADD( m_topRegionID );
   CSADD( m_language );
   if ( isEmptyHitType ) { // Empty type
      m_mapRights = other.m_mapRights;
      m_invertRights = other.m_invertRights;
   } else if ( m_invertRights == other.m_invertRights  ) {
      // Invert rights equal
      m_mapRights = ( m_mapRights | other.m_mapRights );
   } else if ( m_invertRights != other.m_invertRights ) {
      // Invert rights not equal. Do not merge this with
      // the if above. Did not get that to work.
      m_mapRights = ( m_mapRights ^ other.m_mapRights );
      m_invertRights = true;
   }
   
   CSADD( m_providerType );
#undef CSADD
   return *this;
}

void CompactSearch::cleanInput() {
   m_what = StringUtility::trimStartEnd( m_what );
   m_where = StringUtility::trimStartEnd( m_where );
   m_categoryName = StringUtility::trimStartEnd( m_categoryName );
}

bool CompactSearch::validInput() const {
   bool invalidWhat = 
      ! m_what.empty() &&
      StringSearchUtility::convertIdentToClose( m_what ).empty();

   bool invalidWhere = 
      ! m_where.empty() && 
      StringSearchUtility::convertIdentToClose( m_where ).empty();

   bool invalidCategory =
      ! m_categoryName.empty() &&
      StringSearchUtility::convertIdentToClose( m_categoryName ).empty();
      
   if ( invalidWhat || invalidWhere || invalidCategory ) {
      return false;
   }

   return true;
}
