/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavLatestNewsData.h"
#include "StringUtility.h"
#include "MC2CRC32.h"
#include "ParserThreadGroup.h"
#include "FOBC.h"


const uint32 NavLatestNewsData::m_nbrDaysLeftValues = 4;

const uint16 NavLatestNewsData::m_daysLeftValues[ m_nbrDaysLeftValues ] = 
{0, 7, 14, 30};


NavLatestNewsData::NavLatestNewsData( 
   const char* latestNews,
   const char* clientType,
   LangTypes::language_t lang,
   WFSubscriptionConstants::subscriptionsTypes wfSubscriptionType,
   uint16 daysLeft,
   const char* imageExtension,
   ParserThreadGroup* group,
   uint32 recheckTime )
      : m_lastNews( NULL ), m_lastNewsLength( 0 ), m_crc( 0 ),
        m_lastNewsFile( StringUtility::newStrDup( latestNews ) ),
        m_clientType( StringUtility::newStrDup( clientType ) ),
        m_lang( lang ), m_wfSubscriptionType( wfSubscriptionType ),
        m_daysLeft( daysLeft > 30 ? MAX_UINT16 : daysLeft ), 
        m_imageExtension( StringUtility::newStrDup( imageExtension ) ),
        m_recheckTime( recheckTime ),
        m_lastNewsLoadTime( 0 ), m_group( group )
{
}


NavLatestNewsData::~NavLatestNewsData() {
   delete [] m_lastNewsFile;
   delete [] m_clientType;
   delete [] m_imageExtension;
}


bool
NavLatestNewsData::loadFile() {
   bool ok = false;

   aFOB f = m_group->getFOBC()->getFile( m_lastNewsFile );

   if ( f.get() != NULL ) {
      m_file = f;
      m_lastNews = m_file->getBytes();
      m_lastNewsLength = m_file->size();
      m_crc = MC2CRC32::crc32( m_lastNews, m_lastNewsLength );
      ok = true;
   }

   // Set here to avoid checking all the time if file is corrupt
   m_lastNewsLoadTime = TimeUtility::getRealTime();

   return ok;
}


bool
NavLatestNewsData::recheckFile() {
   uint32 now = TimeUtility::getRealTime();
   if ( now - m_lastNewsLoadTime >= m_recheckTime ) {
      return loadFile();
   } else {
      return true;
   }
}


const byte*
NavLatestNewsData::getLastNews() const {
   return m_lastNews;
}


uint32
NavLatestNewsData::getLastNewsLength() const {
   return m_lastNewsLength;
}


const char* 
NavLatestNewsData::getFilePatch() const {
   return m_lastNewsFile;
}


uint32 
NavLatestNewsData::getNbrDaysLeftValues() {
   return m_nbrDaysLeftValues;
}


uint16 
NavLatestNewsData::getDaysLeftValue( uint32 index ) {
   MC2_ASSERT( index < m_nbrDaysLeftValues );
   return m_daysLeftValues[ index ];
}


uint16 
NavLatestNewsData::getClosestDaysLeftValue( uint16 value ) {
   uint32 i = 0;
   uint16 res = 0;
   do {
      res = m_daysLeftValues[ i ];
   } while ( res < value && i++ < getNbrDaysLeftValues() );
   if ( i >= getNbrDaysLeftValues() ) { // Larger than any
      res = MAX_UINT16;
   }

   return res;
}


// --------- SearchNavLatestNewsData -----------------


SearchNavLatestNewsData::SearchNavLatestNewsData()
      : NavLatestNewsData( "", "", LangTypes::english, 
                           WFSubscriptionConstants::TRIAL,
                           MAX_UINT16, "", NULL )
{
   delete [] m_clientType;
   m_clientType = NULL;
   delete [] m_imageExtension;
   m_imageExtension = NULL;
}


void
SearchNavLatestNewsData::setData( 
   const char* clientType,
   LangTypes::language_t lang,
   WFSubscriptionConstants::subscriptionsTypes wfSubscriptionType,
   uint16 daysLeft,
   const char* imageExtension )
{
   m_clientType = const_cast< char* > ( clientType );
   m_lang = lang;
   m_wfSubscriptionType = wfSubscriptionType;
   m_daysLeft = daysLeft > 30 ? MAX_UINT16 : daysLeft;
   m_imageExtension = const_cast< char* > ( imageExtension );
}


void
SearchNavLatestNewsData::unSetData() {
   m_clientType = NULL;
   m_imageExtension = NULL;
}
