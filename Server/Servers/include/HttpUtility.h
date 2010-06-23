/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPUTILITY_H
#define HTTPUTILITY_H

#include "config.h"
#include "StringTable.h"
#include "ImageDrawConfig.h"
#include "MapSettings.h"
#include "Vector.h"
#include "MC2String.h"

class GfxFeatureMap;
class HttpHeader;

class SMSSendRequest;

class ExpandItemID;
class ExpandStringItem;
class ExpandRouteReplyPacket;

class RouteRequest;
class CellularPhoneModel;

/**
 * Utility class for generic Http help functions.
 * 
 */
class HttpUtility {
   public:
      /**
       * Check if the authentication is in the authentication list.
       * @param auth The string with the authentication.
       * @return True if the authentication if succesfull, false if
       *         not or error.
       */
      static bool checkAuth( const char* auth );
      
      
      /**
       * Remove all current data. Mostly used when shuting down.
       * Monitor procedure.
       */
      static void clear();

      
      /**
       * Parses str for user settings.
       * @param str The string to parse for user settings.
       * @param userLang Set to the language in user setting.
       * @return True if str contains valid user settings, false if not.
       */
      static bool parseUserSettings( const char* str, 
                                     StringTable::languageCode& userLang );


      /**
       * Makes a link to a stored route for later use.
       *
       * @param target The string to print the link into.
       * @param host The host to make the link to.
       * @param protocol The protocol to use in link. "http".
       * @param routeID The id of the route. 65.
       * @param routeCreateTime The creation time of the route. 1002099708.
       * @param lang The language that the route should be presented in.
       * @param routePage The page that serves stored routes, default
       *                  "route.wml".
       * return The target or NULL if something is wrong.
       */
      static char* makeStoredRouteLink( char* target, const char* host,
                                        const char* protocol,
                                        uint32 routeID, 
                                        uint32 routeCreateTime,
                                        StringTable::languageCode lang,
                                        const char* routePage = 
                                        "route.wml" );


      /**
       * Makes a SMSSendRequest with the comment text before the routeLink
       * text. The comment is trunkated if it where to increase the size
       * of the SMS beyond the MAX_SMS_SIZE.
       *
       * @param comment The comment text to add before link in SMS,
       *                is trunkated to fit into SMS.
       * @param host The host to make the link to.
       * @param protocol The protocol to use in link. "http".
       * @param routeID The id of the route. 65.
       * @param routeCreateTime The creation time of the route. 1002099708.
       * @param lang The language that the route should be presented in.
       * @param cellular The cellular model.
       * @param reqID The requestID to use.
       * @param service The SMS service to use, "WEBB".
       * @param receiver The receiver of the SMS.
       * @param routePage The page that serves stored routes, default
       *                  "route.wml".
       * return A new SMSSendRequest with the SMS or NULL if something is
       *        wrong.
       */
      static SMSSendRequest* makeRouteLinkSMS( const char* comment,
                                               const char* host,
                                               const char* protocol,
                                               uint32 routeID, 
                                               uint32 routeCreateTime,
                                               StringTable::languageCode 
                                               lang,
                                               const CellularPhoneModel* 
                                               cellular,
                                               uint16 reqID,
                                               const char* service,
                                               const char* receiver,
                                               const char* routePage = 
                                               "route.wml" );


      /**
       * Makes a link to a certain area with optional markers for later 
       * use.
       *
       * @param target The string to print the link into.
       * @param host The host to make the link to.
       * @param protocol The protocol to use in link. "http".
       * @param ula The northern latitude.
       * @param llo The western longitude.
       * @param lla The southern latitude.
       * @param ulo The eastern longitude.
       * @param symbolMap A GfxFeatureMap with GfxFeatureSymbols that
       *                  should be shown on the map image. May be NULL.
       * @param lang The language that the text should have.
       * @param localMapString A string describing the localmap. Default
       *                       NULL.
       * @param signature THe signature. Default NULL.
       * @param localMapPage The page that serves local maps, default
       *                  "lmap.wml".
       * @param clientType The client type to use, or NULL.
       * return The target or NULL if something is wrong.
       */
      static char* makeLocalMapLink( char* target, const char* host,
                                     const char* protocol,
                                     int32 ula, int32 llo,
                                     int32 lla, int32 ulo,
                                     GfxFeatureMap* symbolMap,
                                     StringTable::languageCode lang,
                                     const char* localMapString = NULL,
                                     const char* signature = NULL,
                                     const char* localMapPage = "lmap.wml",
                                     const char* clientType = NULL );


      /**
       * Makes a SMSSendRequest with the comment text before the 
       * localMapLink text. The comment is trunkated if it where to 
       * increase the size of the SMS beyond the MAX_SMS_SIZE.
       *
       * @param comment The comment text to add before link in SMS,
       *                is trunkated to fit into SMS.
       * @param host The host to make the link to.
       * @param protocol The protocol to use in link. "http".
       * @param ula The northern latitude.
       * @param llo The western longitude.
       * @param lla The southern latitude.
       * @param ulo The eastern longitude.
       * @param symbolMap A GfxFeatureMap with GfxFeatureSymbols that
       *                  should be shown on the map image. May be NULL.
       * @param lang The language that the text should have.
       * @param lang The language that the route should be presented in.
       * @param cellular The cellular model.
       * @param reqID The requestID to use.
       * @param service The SMS service to use, "WEBB".
       * @param receiver The receiver of the SMS.
       * @param routePage The page that serves stored routes, default
       *                  "lmap.wml".
       * return A new SMSSendRequest with the SMS or NULL if something is
       *        wrong.
       */
      static SMSSendRequest* makeLocalMapLinkSMS( 
         const char* comment,
         const char* host,
         const char* protocol,
         int32 ula, int32 llo,
         int32 lla, int32 ulo,
         GfxFeatureMap* symbolMap,
         StringTable::languageCode lang,
         CellularPhoneModel* cellular,
         uint16 reqID,
         const char* service,
         const char* receiver,
         const char* routePage = "lmap.wml" );


      /**
       * Makes an SMS with as much as possible of comment before link.
       *
       * @param comment The comment text to add before link in SMS,
       *                is trunkated to fit into SMS.
       * @param link    The link to have last in SMS after comment.
       * @param cellular The cellular model.
       * @param reqID The requestID to use.
       * @param service The SMS service to use.
       * @param receiver The receiver of the SMS.
       * @return  A new SMSSendRequest with the SMS.
       */
      static SMSSendRequest* makeLinkSMS( const char* comment,
                                          const char* link,
                                          const CellularPhoneModel* cellular,
                                          uint16 reqID, 
                                          const char* service,
                                          const char* receiver );

      
      /**
       * Add map symbols to an URL.
       *
       * @param target The string to add map symbols URL data to.
       * @param symbolMap The map with the symbols to add.
       * @param maxURLSize The maximun nbr of chars to write to the target.
       * @return The number of chars written to target.
       */
      static uint32 addMapSymbols( char* target, 
                                   const GfxFeatureMap* symbolMap,
                                   uint32 maxURLSize = 512 );


      /**
       * Reads map symbols from an HttpHeader parameterlist.
       *
       * @param str The parameterlist.
       * @param symbolMap The GfxFeatureMap to add the GfxSymbolFeatures
       *                  to.
       */
      static void readMapSymbols( const MC2String* str, 
                                  GfxFeatureMap* symbolMap );


      /**
       *    @name Map support functions.
       *    @memo Map support functions.
       *    @doc  Map support functions.
       */
      //@{
         /**
          * Makes an URL to an imagemap.
          * The boundingbox is adjusted to fill the map.
          *
          * @param target The string to print the URL into. Must be
          *               4096 bytes or more.
          * @param widthStr The string to print the width into, may be
          *                 NULL.
          * @param heightStr The string to print the height into,
          *                  may be NULL.
          * @param beforeTurn The index of the feature to start the 
          *        routeturn at.
          * @param afterTurn The index of the feature to stop the 
          *        routeturn at.
          * @param turn The index of the turn, used as an alternative to 
          *             beforeTurn and afterTurn.
          * @param inHead The HttpHeader of the HTTPrequest with the 
          *        accept headers. See also defaultFormat.
          * @param routeID The routeID of the route.
          * @param routeCreateTime The routes create time.
          * @param width The width of the output.
          * @param height The height of the output.
          * @param minLat Minimum latitude of requested map.
          * @param minLon Minimum longitude of requested map.
          * @param maxLat Maximum latitude of requested map.
          * @param maxLon Maximum longitude of requested map.
          * @param defaultFormat The default format of the link.
          *        Default PNG.
          * @param mapSetting The defaultMapSetting to use. Default STD.
          * @param showMap If map contents should be on map.
          * @param showTopographMap If topographical map contents should 
          *                         be on map.
          * @param showPOI If POI contents should be on map.
          * @param showRoute If route contents should be in map, requires
          *                  valid routeID.
          * @param showScale If scale should be on map.
          * @param showTraffic If trafficinformation should be in map.
          * @param imageSettings The ImageSettings to use, default NULL.
          * @param symbolMap The map symbols to put on map, default NULL.
          * @param clientType The client type to use, or NULL.
          */
         static void makeMapURL( 
            char* target, 
            char* widthStr,
            char* heightStr,
            const HttpHeader* inHead,
            uint32 routeID,
            uint32 routeCreateTime,
            uint32 beforeTurn, uint32 afterTurn,
            uint32 turn,
            uint16 width,
            uint16 height,
            int32 minLat,
            int32 minLon,
            int32 maxLat,
            int32 maxLon,
            ImageDrawConfig::imageFormat defaultFormat = 
            ImageDrawConfig::PNG,
            MapSettingsTypes::defaultMapSetting mapSetting = 
            MapSettingsTypes::MAP_SETTING_STD,
            bool showMap = true, bool showTopographMap = true, 
            bool showPOI = true, bool showRoute = true,
            bool showScale = false, bool showTraffic = false,
            struct MapSettingsTypes::ImageSettings* imageSettings = NULL,
            const GfxFeatureMap* symbolMap = NULL,
            const char* clientType = NULL );


         /**
          * Calculates the boudingbox for a route.
          * 
          * @param expand The ExpandRouteReplyPacket with the route.
          * @param exp The ExpandItemID with the RouteItems stored.
          * @param startIndex The index to start counting boundingbox in 
          *        the route.
          * @param stopIndex The index to stop counting boundingbox in the
          *        route.
          * @param minLat Set to minimum latitude of requested bb.
          * @param minLon Set to minimum longitude of requested bb.
          * @param maxLat Set to maximum latitude of requested bb.
          * @param maxLon Set to maximum longitude of requested bb.
          */
         static void getRouteMC2BoundingBox( 
            ExpandRouteReplyPacket* expand,
            ExpandItemID* exp,
            uint32 startIndex, 
            uint32 stopIndex,
            int32& minLat,
            int32& maxLat,
            int32& minLon,
            int32& maxLon );


         /**
          * Calculates the boundingbox for a certain turn of a route.
          * The boundingbox is intended to be used for extracting
          * crossingmaps used in a navigation application.
          * 
          * @param expand The ExpandRouteReplyPacket with the route.
          * @param exp The ExpandItemID with the RouteItems stored.
          * @param stringItems The string items of the route.
          * @param index The index of the turn in the route.
          * @param minLat Set to minimum latitude of requested bb.
          * @param minLon Set to minimum longitude of requested bb.
          * @param maxLat Set to maximum latitude of requested bb.
          * @param maxLon Set to maximum longitude of requested bb.
          * @param angle  Set to the angle of the map.
          */
         static void getNavCrossingMC2BoundingBox( 
            ExpandRouteReplyPacket* expand,
            ExpandItemID* exp,
            ExpandStringItem** stringItems,
            uint32 index, 
            int32& minLat,
            int32& maxLat,
            int32& minLon,
            int32& maxLon,
            uint16& angle );


         /**
          * Checks the headers of a request and makes the best choice of
          * image format for the requester.
          * @param inHead The http header with the possible Accept-line.
          * @param defaultFormat The format to return if no format was
          *        found in inHead, default PNG.
          */
         static ImageDrawConfig::imageFormat getImageFormatForRequest( 
            const HttpHeader* inHead,
            ImageDrawConfig::imageFormat defaultFormat = 
            ImageDrawConfig::PNG );

         
         /**
          * Calculates the beforeturnitemindex and the afterturnitemindex
          * for a pair of stringitemindexes.
          * @param exp The ExpandItemID with the groups.
          * @param startIndex The starting stringitemindex.
          * @param stopIndex The ending stringitemindex.
          * @param beforeTurn Set to the first coordinate item index.
          * @param afterTurn Set to the last coordinate item index.
          */
         static void turnItemIndexFromStringItemIndex( ExpandItemID* exp,
                                                       uint32 startIndex,
                                                       uint32 stopIndex,
                                                       uint32& beforeTurn,
                                                       uint32& afterTurn );
      //@}


      /**
       * Makes a html link to a stored route.
       *
       * @param target The string to print the link into.
       * @param host The host to make the link to.
       * @param protocol The protocol to use in link. "http".
       * @param routeID The id of the route. 65.
       * @param routeCreateTime The creation time of the route. 1002099708.
       * @param lang The language that the route should be presented in.
       * @param originString String describing the origin.
       * @param originLocationString String describing the origin's 
       *                             location.
       * @param destinationString String describing the destination.
       * @param destinationLocationString String describing the 
       *                                  destination's location.
       * @param signature The signuture, text last, on the page.
       * @param routePage The page that serves stored routes, default
       *                  "route.html".
       * return The target or NULL if something is wrong.
       */
      static char* makeRouteLink( char* target, const char* host,
                                  const char* protocol,
                                  uint32 routeID, 
                                  uint32 routeCreateTime,
                                  StringTable::languageCode lang,
                                  const char* originString,
                                  const char* originLocationString,
                                  const char* destinationString,
                                  const char* destinationLocationString,
                                  const char* signature,
                                  const char* routePage = "route.html" );


      /**
       * Makes sure that the boundingbox isn't too small.
       * 
       * @param minLat minimum latitude.
       * @param minLon minimum longitude.
       * @param maxLat maximum latitude.
       * @param maxLon maximum longitude.
       * @param frame How much frame to add around bbox. Default 0.0.
       * @param minWidth  Minimum width in mc2-coordinates of the 
       *                  boundingbox. Default value 40000.
       * @param minHeight Minimum height in mc2-coordinates of the 
       *                  boundingbox, not taking coslat into 
       *                  consideration. Default value 40000.
       */
      static void checkMinimumBoundingbox( int32& minLat, int32& maxLat,
                                           int32& minLon, int32& maxLon,
                                           float64 frame = 0.0,
                                           int32 minWidth = 40000,
                                           int32 minHeight = 40000 );


   private:
      /**
       * Private constructor to avoid creation of instances.
       * NB! Not implemented.
       */
      HttpUtility();

      
      /**
       * Initializes the checkAuth authentication.
       * Monitor procedure.
       */
      static void initializeCheckAuth();

      
      /**
       * If m_authHosts and m_authUsers is initialized.
       */
      static bool m_checkAuthInitialized;


      /**
       * The the authenticated hosts.
       */
      static StringVector m_authHosts;


      /**
       * The the authenticated users for the hosts.
       */
      static StringVector m_authUsers;
};

#endif // HTTPUTILITY_H

