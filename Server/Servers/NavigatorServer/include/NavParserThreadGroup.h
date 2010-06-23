/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVPARSERTHREADGROUP_H
#define NAVPARSERTHREADGROUP_H

#include "config.h"
#include "InterfaceParserThreadGroup.h"
#include "NavLatestNewsDataCmp.h"


class RouteRequest; // A mc2-request
class RouteReply;  // A Nav-Packet
class NavClientVersionStorage;
class NavClientVersion;
class NParam;
class NParamBlock;
class NavReplyPacket;
class NavLatestNewsData;
class SearchNavLatestNewsData;
class NavCategoriesDataHolder;
class ClientSetting;
/**
 * Class that handles a group of Nav threads.
 *
 */
class NavParserThreadGroup : public InterfaceParserThreadGroup {
   public:
      /** 
       * Creates a new NavParserThreadGroup.
       *
       * @param threadGroupName The name of the group.
       * @param minNbrThreads The lowest number of parserthreads.
       * @param maxNbrThreads The highest number of parserthreads.
       * @param queueFactor The number of irequests, calculated as
       *                    queueFactor*nbrThreads, there can be in
       *                    waiting queue before it is concidered full.
       * @param queueOverFullFactor The number of irequests, calculated as
       *        queueFactor*queueOverFullFactor*nbrThreads, there can be in
       *        waiting queue before it is concidered over full.
       */
      NavParserThreadGroup(
         const char* threadGroupName = "NavParserThreadGroup",
         uint32 minNbrThreads = 5, uint32 maxNbrThreads = 20,
         uint32 queueFullFactor = 3, uint32 queueOverFullFactor = 3 );

      
      /**
       * Destructor.
       */
      virtual ~NavParserThreadGroup();


      /**
       * Handle a done InterfaceRequest, default implementation here
       * is to send it to the first InterfaceFactory.
       *
       * @param ireply The InterfaceRequest that is done.
       */
      virtual void handleDoneInterfaceRequest( InterfaceRequest* ireply );


      /**
       *   Return whether we should save simulation routes.
       *   @return True if routes should be saved, false otherwise.
       */
      bool simulatedRouteFileName();


      /**
       *   Set the filename for simulator routes.
       *   If the filename is already set, the old data will
       *   be deleted and a new copy made.
       */
      void setSimulatedRouteFileName( const char* filename );


      /**
       *   Save the simulation route based on the specified route.
       *   @param routeReq The route request with reply data.
       *   @param reply The raw route reply nav packet to save if not NULL.
       *                Default NULL.
       */
      bool saveRouteForSimulator( RouteRequest *rr, 
                                  RouteReply* reply = NULL );


      /**
       * Save the simulation route based on the specified route.
       *
       * @param reply The route reply nav packet to save if not NULL.
       * @param rr The RouteRequest to save if not NULL.
       */
      bool saveRouteForSimulator( NavReplyPacket* reply, 
                                  RouteRequest *rr );


      /**
       * Set the latest news files for client types.
       * Syncronized function.
       *
       * @param latestNews The list of client types and latest news files.
       */
      void setLatestNews( const char* latestNews );


      /**
       * Get a copy of the latest news file for a client type.
       * Syncronized function.
       *
       * @param clientType The type of client to get latest news for.
       *                   for the clientType.
       * @param lang The language of the latest news image.
       * @param wfSubscriptionType The Wayfinder subscription type of the
       *                           lastest news.
       * @param daysLeft The number of days left for subscription.
       * @param imageExtension The image extension.
       * @param latestNewsLength Value parameter set to the length of
       *                         latestNews.
       * @return True if lastest news is found for clientType, 
       *         false if not.
       */
      bool getLatestNews( const char* clientType, 
                          LangTypes::language_t lang,
                          WFSubscriptionConstants::subscriptionsTypes
                          wfSubscriptionType,
                          uint16 daysLeft,
                          const char* imageExtension,
                          byte*& latestNews,
                          uint32& latestNewsLength );


      /**
       * Get the crc for the latest news file for a client type.
       * Syncronized function.
       *
       * @param clientType The type of client to get latest news for.
       * @param lang The language of the latest news image.
       * @param wfSubscriptionType The Wayfinder subscription type of the
       *                           lastest news.
       * @param daysLeft The number of days left for subscription.
       * @param imageExtension The image extension.
       * @return True if lastest news is found for clientType, 
       *         false if not.
       */
      bool getLatestNewsCRC( const char* clientType, 
                             LangTypes::language_t lang,
                             WFSubscriptionConstants::subscriptionsTypes
                             wfSubscriptionType,
                             uint16 daysLeft,
                             const char* imageExtension,
                             uint32& crc );


      /**
       * Set alternativeServersList. A list of servers that clients 
       * should use.
       */
      void setAlternativeServersList( const char* alternativeServersList );


      /**
       * Get alternativeServersList.
       */
      const char* getAlternativeServersList() const;


      /**
       * Get alternativeServersListCRC.
       */
      uint32 getAlternativeServersListCRC() const;


      /**
       * Set longAlternativeServersList. A list of servers that clients 
       * should use.
       */
      void setLongAlternativeServersList( 
         const char* longAlternativeServersList );


      /**
       * Get longAlternativeServersList.
       */
      const char* getLongAlternativeServersList() const;


      /**
       * Get longAlternativeServersListCRC.
       */
      uint32 getLongAlternativeServersListCRC() const;


      /**
       * Set httpAlternativeServersList. A list of servers that http 
       * clients should use.
       */
      void setHttpAlternativeServersList( 
         const char* httpAlternativeServersList );


      /**
       * Get httpAlternativeServersList.
       */
      const char* getHttpAlternativeServersList() const;


      /**
       * Get httpAlternativeServersListCRC.
       */
      uint32 getHttpAlternativeServersListCRC() const;
   
      /**
       * Set forceRedirect. If force client-redirect, 
       * alternativeServersList should be set.
       */
      void setForceRedirect( bool forceRedirect );


      /**
       * Get forceRedirect.
       */
      bool getForceRedirect() const;


      /**
       * Set currentVersion. A list of client versions.
       */
      void setCurrentVersion( const char* currentVersion );


      /**
       * Get the client version for a client type.
       */
      const NavClientVersion* getClientVersion( 
         const char* clientType ) const;


      /**
       * Set noUserTrack. If no user tracking should be made.
       */
      void setNoUserTrack( bool noUserTrack );


      /**
       * Get noUserTrack.
       */
      bool getNoUserTrack() const;

      //Otherwise hidden by getSetting(const NParamblock&... below.
      using ParserThreadGroup::getSetting;

      /**
       * Get the clientsetting for a clientType using a parameterblock.
       */
      const ClientSetting* getSetting( 
         const NParamBlock& params, ParserThreadHandle thread ) const;

      /**
       * Get the top region data for a user.
       *
       * @param thread Handle to the thread calling this function.
       * @param user The user to make top regions for.
       * @param lang The language to make top regions in.
       * @param urmask The user right type mask to match right with.
       * @param data Top region data is added to this.
       * @return 0 if ok, 1 if error and 2 if timeout.
       */
      int getTopRegionData( ParserThreadHandle thread, 
                            const UserUser* user,
                            StringTable::languageCode lang,
                            UserEnums::URType urmask,
                            NParam& data );


      /**
       * Get the top region crc for a user.
       *
       * @param thread Handle to the thread calling this function.
       * @param user The user to make top regions for.
       * @param lang The language to make top regions in.
       * @param urmask The user right type mask to match right with.
       * @param crc Set to the Top region crc.
       * @return 0 if ok, 1 if error and 2 if timeout.
       */
      int getTopRegionCRC( ParserThreadHandle thread, 
                           const UserUser* user,
                           StringTable::languageCode lang,
                           UserEnums::URType urmask,
                           uint32& crc );


   protected:
      /**
       * Create a new Nav thread for processing.
       *
       * @return A new ParserThread.
       */
      virtual ParserThreadHandle spawnThread();

  private:
      /**
       * Saves an MapViewerRoute to a file.
       */
      bool saveMapRoute( const char* fileName, RouteRequest* rr );


      /**
       *   Where to save routes for simulation in MapViewer.
       *   Null if no route data should be saved.
       */
      char* m_simulatedRouteFileName;


      /**
       * The mutex used to lock latest news.
       */
      ISABMutex m_latestNewsMutex;


      typedef set< NavLatestNewsData*, NavLatestNewsLessComp > 
         latestNewsMap;


      /**
       * The latest news.
       */
      latestNewsMap m_latestNews;


      /**
       * The current thread id.
       */
      uint32 m_threadID;


      /**
       * The alternativeServersList
       */
      const char* m_alternativeServersList;


      /**
       * The checksum for m_alternativeServersList.
       */
      uint32 m_alternativeServersListCRC;


      /**
       * The longAlternativeServersList
       */
      const char* m_longAlternativeServersList;


      /**
       * The checksum for m_longAlternativeServersList.
       */
      uint32 m_longAlternativeServersListCRC;


      /**
       * The httpAlternativeServersList
       */
      const char* m_httpAlternativeServersList;


      /**
       * The checksum for m_httpAlternativeServersList.
       */
      uint32 m_httpAlternativeServersListCRC;


      /**
       * If force redirection of clients.
       */
      bool m_forceRedirect;


      /**
       * The current client versions storage.
       */
      NavClientVersionStorage* m_clientVersions;


      /**
       * The mutex used to lock client versions storage.
       */
      ISABMutex m_clientVersionMutex;


      /**
       * if no UserTrack.
       */
      bool m_noUserTrack;

      /**
       * The search NavLatestNewsData.
       */
      mutable SearchNavLatestNewsData* m_latestNewsSearch;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline void 
NavParserThreadGroup::setForceRedirect( bool forceRedirect ) {
   m_forceRedirect = forceRedirect;
}


inline bool
NavParserThreadGroup::getForceRedirect() const {
   return m_forceRedirect;
}


inline void 
NavParserThreadGroup::setNoUserTrack( bool noUserTrack ) {
   m_noUserTrack = noUserTrack;
}


inline bool
NavParserThreadGroup::getNoUserTrack() const {
   return m_noUserTrack;
}


inline const char* 
NavParserThreadGroup::getAlternativeServersList() const {
   return m_alternativeServersList;
}


inline uint32 
NavParserThreadGroup::getAlternativeServersListCRC() const {
   return m_alternativeServersListCRC;
}


inline const char* 
NavParserThreadGroup::getLongAlternativeServersList() const {
   if ( m_longAlternativeServersList != NULL ) {
      return m_longAlternativeServersList;
   } else {
      return m_alternativeServersList;
   }
}


inline uint32 
NavParserThreadGroup::getLongAlternativeServersListCRC() const {
   if ( m_longAlternativeServersList != NULL ) {
      return m_longAlternativeServersListCRC;
   } else {
      return m_alternativeServersListCRC;
   }
}


inline const char* 
NavParserThreadGroup::getHttpAlternativeServersList() const {
   return m_httpAlternativeServersList;
}


inline uint32 
NavParserThreadGroup::getHttpAlternativeServersListCRC() const {
   return m_httpAlternativeServersListCRC;
}


#endif // NAVPARSERTHREADGROUP_H

