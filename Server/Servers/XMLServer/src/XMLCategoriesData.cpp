/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLCategoriesData.h"
#include "StringUtility.h"
#include "XMLUtility.h"
#include "MC2CRC32.h"
#include "STLStringUtility.h"

XMLCategory::XMLCategory( const MC2String& name, const MC2String& image, 
                          SearchParserHandler::Category::CategoryID id ) 
      : m_name( name.size() + 1 ), 
        m_image( image.size() + 1 ), m_id( id ) {
   XMLUtility::transcodeToXML( m_name, name.c_str() );
   XMLUtility::transcodeToXML( m_image, image.c_str() );
}

XMLCategory::XMLCategory( const XMLCategory& o ) 
      : m_name( XMLString::stringLen( o.m_name ) + 1 ),
        m_image( XMLString::stringLen( o.m_image ) + 1 ), m_id( o.m_id ) {
   XMLString::copyString( m_name, o.m_name );
   XMLString::copyString( m_image, o.m_image );
}


XMLCategoriesData::XMLCategoriesData( bool clientUsesLatin1 ): 
      CategoriesData( clientUsesLatin1 )
{
}


XMLCategoriesData::~XMLCategoriesData() {
}

void
XMLCategoriesData::innerMakeList( const char* clientType,
                                  LangTypes::language_t language,
                                  const CategoriesDataHolder::CatData& data ) {
   // TODO: Make the utf-8/latin1 only for Nav where it is needed.
   if ( m_clientUsesLatin1 ) {
      return;
   }
   MC2String all;
   m_categories.reserve( data.stringIDs.size() );
   for ( uint32 i = 0 ; i < data.stringIDs.size() ; ++i ) {
      // Add category
      // ids and names are already in right utf8/iso
      //mc2dbg << "Cat " << clientType << " " << data.names[ i ] 
      //       << " " << data.filenames[ i ] << endl;
      m_categories.push_back( XMLCategory( data.names[ i ],
                                           data.filenames[ i ],
                                           data.categoryIDs[ i ] ) );
      all.append( data.names[ i ] );
      all.append( data.filenames[ i ] );
      STLStringUtility::uint2str( data.categoryIDs[ i ], all );
      m_nbrCategories++;
   }

   // do crc on all the data
   m_categoriesCRC = MC2CRC32::crc32( 
      reinterpret_cast< const byte* > ( all.data() ), all.size() );
}

CategoriesData*
XMLCategoriesDataHolder::createCategoriesData( bool latin1 ) {
   return new XMLCategoriesData( latin1 );
}
