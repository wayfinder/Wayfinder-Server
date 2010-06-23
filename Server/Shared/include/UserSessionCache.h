/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERSESSIONCACHE_H
#define USERSESSIONCACHE_H


#include "config.h"
#include "MC2String.h"
#include "STLStrComp.h"
#include <map>


/**
 * Holds user sessions cached.
 *
 */
class UserSessionCache {
   public:
      /**
       * Creates a new UserSessionCache.
       *
       * @param maxSize The maximum number of sessions to cache.
       */
      UserSessionCache( uint32 maxSize = 100000 );


      /**
       * Destructor deletes all local data.
       */
      virtual ~UserSessionCache();


      /**
       * Add a session to the cache.
       * 
       * @param sessionID The sessionID to cache.
       * @param sessionKey The sessionKey to cache.
       * @param UIN The UIN for the session.
       * @param lastAccessTime The time of the last access.
       */
      void addSession( const char* sessionID, const char* sessionKey,
                       uint32 UIN, uint32 lastAccessTime );


      /**
       * Add a session to the cache.
       * 
       * @param sessionID The sessionID to cache.
       * @param sessionKey The sessionKey to cache.
       * @param UIN The UIN for the session.
       * @param lastAccessTime The time of the last access.
       */
      void addSession( const MC2String& sessionID, 
                       const MC2String& sessionKey,
                       uint32 UIN, uint32 lastAccessTime );


      /**
       * Get the sessionKey for a sessionID.
       *
       * @param sessionID The sessionID to look for.
       * @param sessionKey Set to the session key or NULL if not in cache.
       * @param UIN Set to the sessions UIN if in cache.
       * @param UIN Set to the sessions lastAccessTime if in cache.
       */
      bool getSession( const char* sessionID,
                       const char*& sessionKey,
                       uint32& UIN, uint32& lastAccessTime ) const;


      /**
       * Removes a session from the cache.
       */
      void removeSession( const char* sessionID );


      /**
       * Removes a session from the cache.
       */
      void removeSession( const MC2String& sessionID );

      /**
       * The size of the map.
       */
      uint32 size() const;

   private:
      /**
       * Class that holds session data
       */
      class SessionData {
         public:
            /**
             * Creates a new SessionData with data.
             */
            SessionData( const char* sessionKey, uint32 UIN, 
                         uint32 lastAccessTime )
               : m_sessionKey( sessionKey ), m_UIN( UIN ), 
                 m_lastAccessTime( lastAccessTime )
            {}


            /**
             * Get the sessionKey.
             */
            const char* getSessionKey() const {
               return m_sessionKey.c_str();
            }


            /**
             * Get the UIN.
             */
            uint32 getUIN() const {
               return m_UIN;
            }


            /**
             * Get the lastAccessTime.
             */
            uint32 getLastAccessTime() const {
               return m_lastAccessTime;
            }

            /**
             * Set the lastAccessTime.
             */
            void setLastAccessTime( uint32 newTime ) {
               m_lastAccessTime = newTime;
            }


         private:
            /// The sessionKey
            MC2String m_sessionKey;


            /// The UIN
            uint32 m_UIN;


            /// The lastAccessTime
            uint32 m_lastAccessTime;
      };


      /// The cached sessions.
      typedef map< MC2String, SessionData, strCompareLess > userSessionMap;
      userSessionMap m_userSessionMap;


      /**
       * Class for sorting pairs of uint32 and userSessionMap::iterator.
       */
      class sortedVectLess {
         public:
         inline bool operator() ( 
               const pair< uint32, userSessionMap::iterator >& a,
               const pair< uint32, userSessionMap::iterator >& b ) const 
               {
                  return a.first < b.first;
               }
      };


      /// The maximum size of the cache.
      uint32 m_maxSize;
};


#endif // USERSESSIONCACHE_H
