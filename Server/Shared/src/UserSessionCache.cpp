/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "UserSessionCache.h"

#include "TimeUtility.h"
#include <algorithm>


UserSessionCache::UserSessionCache( uint32 maxSize ) 
      : m_maxSize( maxSize )
{
   
}


UserSessionCache::~UserSessionCache() {
   // Nothing newed in constructor or in userSessionMap
}


void
UserSessionCache::addSession( const char* sessionID, 
                              const char* sessionKey,
                              uint32 UIN, uint32 lastAccessTime )
{
   m_userSessionMap.insert( 
      make_pair( sessionID, 
                 SessionData( sessionKey, UIN, lastAccessTime ) ) );
   if ( m_userSessionMap.size() > m_maxSize )
   {
      uint32 rstartTime = TimeUtility::getCurrentMicroTime();
      uint32 startTime = rstartTime;
      // Cleanup, remove oldest half
      mc2dbg << "UserSessionCache cleanup starts size " 
             << m_userSessionMap.size() << endl;

      vector< pair< uint32, userSessionMap::iterator > > sortedVect;

      for ( userSessionMap::iterator it = m_userSessionMap.begin() ;
            it != m_userSessionMap.end() ; ++it )
      {
         sortedVect.push_back( make_pair( it->second.getLastAccessTime(),
                                          it ) );
      }

      uint32 endTime = TimeUtility::getCurrentMicroTime();
      mc2dbg << "Time for sortedVect creation is " \
             << (endTime-startTime) << " us"  << endl;
      startTime = TimeUtility::getCurrentMicroTime();
      std::sort( sortedVect.begin(), sortedVect.end(), sortedVectLess() );
      endTime = TimeUtility::getCurrentMicroTime();
      mc2dbg << "Time to sort sortedVect is " \
             << (endTime-startTime) << " us"  << endl;
      startTime = TimeUtility::getCurrentMicroTime();
      for ( uint32 i = 0 ; i <= m_maxSize / 2 ; ++i ) {
         m_userSessionMap.erase( sortedVect[ i ].second );
      }
      endTime = TimeUtility::getCurrentMicroTime();
      mc2dbg << "Time to remove session is " \
             << (endTime-startTime) << " us"  << endl;
      
      endTime = TimeUtility::getCurrentMicroTime();
      mc2dbg << "Time for UserSessionCache cleanup is " \
             << (endTime-rstartTime) << " us size " 
             << m_userSessionMap.size() << endl;
   }
}


void 
UserSessionCache::addSession( const MC2String& sessionID, 
                              const MC2String& sessionKey,
                              uint32 UIN, uint32 lastAccessTime )
{
   addSession( sessionID.c_str(), sessionKey.c_str(), UIN, lastAccessTime);
}


bool
UserSessionCache::getSession( const char* sessionID,
                              const char*& sessionKey,
                              uint32& UIN, uint32& lastAccessTime ) const
{
   userSessionMap::const_iterator findIt = m_userSessionMap.find( 
      sessionID );
   if ( findIt != m_userSessionMap.end() ) {
      sessionKey = findIt->second.getSessionKey();
      UIN = findIt->second.getUIN();
      lastAccessTime = findIt->second.getLastAccessTime();
      return true;
   } else {
      return false;
   }
}


void
UserSessionCache::removeSession( const char* sessionID ) {
   userSessionMap::iterator findIt = m_userSessionMap.find( 
      sessionID );
   if ( findIt != m_userSessionMap.end() ) {
      m_userSessionMap.erase( findIt );
   }
}

void
UserSessionCache::removeSession( const MC2String& sessionID ) {
   removeSession( sessionID.c_str() );
}

uint32
UserSessionCache::size() const {
   return m_userSessionMap.size();
}
