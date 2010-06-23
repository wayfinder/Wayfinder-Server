/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVREQUESTHANDLER_H
#define NAVREQUESTHANDLER_H

#include "config.h"
#include "InterfaceParserThreadConfig.h"
#include <memory>

// Forward declarations
class NavInterfaceRequest;
class NavAuthHandler;
class NavParserThreadGroup;
class NavUserHelp;
class NavSearchHandler;
class NavInfoHandler;
class NavRouteHandler;
class NavMapHandler;
class NavFavHandler;
class NavServerInfoHandler;
class NavMessHandler;
class NavCellHandler;
class NavTrackHandler;
class NavTunnelHandler;
class NavCombinedSearchHandler;
class NavCellIDHandler;
class NavKeyedDataHandler;
class NavRevGeocodingHandler;
class NavVerifyThirdPartyTransactionHandler;
class NavLocalCategoryTreeHandler;
class NavOneSearchHandler;


/**
 * Class handling NavServerProt, v10+, requests.
 *
 */
class NavRequestHandler {
   public:
      /**
       * Constructor.
       */
      NavRequestHandler( InterfaceParserThread* thread,
                         NavParserThreadGroup* group );


      /**
       * Destructor,
       */
      ~NavRequestHandler();


      /**
       * Handles a NavInterfaceRequest.
       */
      void handleRequest( NavInterfaceRequest* req );


   private:
      /// The thread
      InterfaceParserThread* m_thread;

      /// The group.
      NavParserThreadGroup* m_group;

      /// The NavUserHelp.
      NavUserHelp* m_userHelp;


      /// The auth handler
      NavAuthHandler* m_auth;


      /// The search handler
      NavSearchHandler* m_search;


      /// The info handler
      NavInfoHandler* m_info;


      /// The route handler
      NavRouteHandler* m_route;


      /// The map handler
      NavMapHandler* m_map;


      /// The fav handler
      NavFavHandler* m_fav;


      /// The Server info handler
      NavServerInfoHandler* m_serverInfo;


      /// The Message handler
      NavMessHandler* m_mess;


      /// The Cell handler
      NavCellHandler* m_cell;


      /// The Tracking handler.
      NavTrackHandler* m_track;


      /// The Tunnel handler.
      NavTunnelHandler* m_tunnel;

      /// The combined search handler.
      NavCombinedSearchHandler* m_csearch;

      /// The cell id handler.
      NavCellIDHandler* m_cellID;

      /// The keyed data handler.
      NavKeyedDataHandler* m_keyedData;

      /// The reverse geocoding handler.
      NavRevGeocodingHandler* m_revGeocoding;

      /// The verify a third party transaction handler.
      auto_ptr<NavVerifyThirdPartyTransactionHandler> m_verifyThirdParty;

      /// The local category tree handler.
      auto_ptr<NavLocalCategoryTreeHandler> m_localCategoryTree;

      /// The One Search handler
      auto_ptr<NavOneSearchHandler> m_oneSearch;
};


#endif // NAVREQUESTHANDLER_H

