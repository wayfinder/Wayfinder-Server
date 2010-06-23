/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavCategoriesData.h"
#include "StringUtility.h"
#include "isabBoxNavMessage.h"
#include "MC2CRC32.h"
#include "NParam.h"
#include "FOBC.h"

NavCategoriesData::NavCategoriesData( bool clientUsesLatin1 ): 
   CategoriesData( clientUsesLatin1 ), 
   m_categories( NULL ), m_categoriesLength( 0 ),
   m_filenamesData( NULL ), m_filenamesDataLength( 0 )
{
}


NavCategoriesData::~NavCategoriesData() {
   delete [] m_categories;
   delete [] m_filenamesData;
}

void
NavCategoriesData::innerMakeList( const char* clientType,
                                  LangTypes::language_t language,
                                  const CategoriesDataHolder::CatData& data ) {
   NParam all( 0 ); // contains all data
   NParam fileParam( 0 ); // holds filename data
   NParam catIDParam( 0 ); // holds category data

   NParam p( 0 ); // holds category strings and category translation names
   
   for ( uint32 i = 0 ; i < data.stringIDs.size() ; ++i ) {
      // Add category
      // ids and names are already in right utf8/iso
      p.addByteArray( data.stringIDs[ i ].c_str(), data.stringIDs[ i ].size() +1 );
      all.addByteArray( data.stringIDs[ i ].c_str(), data.stringIDs[ i ].size() +1 );
      p.addByteArray( data.names[ i ].c_str(), data.names[ i ].size() +1 );
      all.addByteArray( data.names[ i ].c_str(), data.names[ i ].size() +1 );

      fileParam.addByteArray( data.filenames[ i ].c_str(), data.filenames[ i ].size() + 1 );
      all.addByteArray( data.filenames[ i ].c_str(), data.filenames[ i ].size() + 1 );

      catIDParam.addUint16( data.categoryIDs[ i ] );
      all.addUint16( data.categoryIDs[ i ] );

      m_nbrCategories++;
   }
   // copy category data from param
   delete [] m_categories;
   m_categoriesLength = p.getLength();
   m_categories = new byte[ m_categoriesLength ];
   memcpy( m_categories, p.getBuff(), m_categoriesLength );
   // copy filenames from param
   delete [] m_filenamesData;
   m_filenamesData = new byte[ fileParam.getLength() ];
   m_filenamesDataLength = fileParam.getLength();
   memcpy( m_filenamesData, fileParam.getBuff(), fileParam.getLength() );

   // copy category ids from param
   m_categoryIDs.resize( catIDParam.getLength() );
   memcpy( &m_categoryIDs[ 0 ], catIDParam.getBuff(), catIDParam.getLength() );

   // do crc on all the data
   m_categoriesCRC = MC2CRC32::crc32( all.getBuff(), all.getLength() );
}


const byte*
NavCategoriesData::getCategories() const {
   return m_categories;
}


uint32
NavCategoriesData::getCategoriesLength() const {
   return m_categoriesLength;
}

void 
NavCategoriesData::getCategories( const byte*& categories, 
                                  uint32& length, uint32& nbr ) const
{
   categories = m_categories;
   length = m_categoriesLength;
   nbr = m_nbrCategories;
}

void 
NavCategoriesData::getCategoriesFilenames( const byte*& filenames,
                                           uint32& length ) const {
   filenames = m_filenamesData;
   length = m_filenamesDataLength;
}

void
NavCategoriesData::getCategoryIDs( const byte*& categoryIds,
                                   uint32& length ) const {
   categoryIds = &m_categoryIDs[ 0 ];
   length = m_categoryIDs.size();
}

CategoriesData*
NavCategoriesDataHolder::createCategoriesData( bool latin1 ) {
   return new NavCategoriesData( latin1 );
}
