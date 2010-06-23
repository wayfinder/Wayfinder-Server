/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLPARSERTHREAD_H
#define XMLPARSERTHREAD_H


#include "config.h"
#include "ISABThread.h"
#include "HttpParserThread.h"
#include "ImageDrawConfig.h"
#include "MapSettingsTypes.h"
#include "RouteMessageRequestType.h"
#include "ExpandedRouteItem.h"
#include "ParserUserHandler.h"

#ifdef USE_XML

# include <dom/DOM.hpp>
# include <parsers/XercesDOMParser.hpp>
  
# include <util/PlatformUtils.hpp>
# include <util/XMLString.hpp>
# include <util/XMLUniDefs.hpp>
# include <framework/MemBufInputSource.hpp> 
# include <framework/XMLFormatter.hpp>
# include <iostream>
  
# include <dom/deprecated/DOMString.hpp>
  
# include "XMLUtility.h"
# include "XMLCommonEntities.h"
# include "XMLCommonElements.h"
# include "XMLParserErrorReporter.h"
# include "XMLTreeFormatter.h"
# include "XMLParserThreadConstants.h"

# include <vector>
# include "MC2String.h"
# include "SearchRequestParameters.h"
# include "ParserThreadGroup.h"
# include "UserData.h"
# include "CompactSearch.h"

#include "ItemInfoEnums.h"
#include "ItemInfoEntry.h"
#include "StringTableUtility.h"

class SearchResultRequest;
class InfoTypeConverter;
class VanillaMatch;

typedef vector<VanillaMatch*> VanillaVector;
typedef VanillaVector::const_iterator VanillaVectorIt;

struct equalVanillaMatch :
   public binary_function<const VanillaMatch*, 
                          const VanillaMatch*, bool> {
   bool operator()(const VanillaMatch* x, const VanillaMatch* y) const;
};
#endif



// Forward declaration
class UserFavorite;
class ItemInfoRequest;
class UserTrackElementsList;
class UserTrackElement;
class CellularPhoneModel;
class ExpandedRoute;
class DebitElement;
class XMLLastRouteData;
class TopRegionRequest;
class CellularPhoneModelsElement;
class OverviewMatch;
class VanillaRegionMatch;
class GfxFeatureMap;
class SearchMatch;
class XMLExtServiceHelper;
class XMLAuthData;
class SearchParserHandler;


/**
 * Handles XML Requests.
 *
 */
class XMLParserThread : public HttpParserThread {
   public:
      /**
       * Creates a new XMLParserThread.
       * @param group The HttpParserThreadGroup that this XMLParserThread
       *              is part of.
       */
      XMLParserThread( HttpParserThreadGroup* group );

      
      /**
       * Destructor deletes all local data.
       * Don't call this use terminate.
       */
      virtual ~XMLParserThread();


  protected:
      /**
       * This function is called when a new HTTP request has been received.
       * The possible parameters in inBody is not added to paramsMap,
       * make a parseParameters( inBody->getBody(), paramsMap, inHead );
       * call if there are parameters in the body to add.
       * The Content-Type in the outHead is set to the mime-type of the
       * fileexternsion of the pagename.
       *
       * @param inHead The HTTP header of the request.
       * @param inBody The body content, may be empty, of the HTTP request.
       * @param paramsMap The startline's parameters.
       * @param outHead The HTTP header of the reply.
       * @param outBody The content of the reply.
       * @param now The time of the request.
       * @param dateStr The time of the request as a string.
       * @return True if the request was handled and outBody was filled 
       *         with the reply.
       */
      virtual bool handleHttpRequest( HttpHeader* inHead, 
                                      HttpBody* inBody,
                                      stringMap* paramsMap,
                                      HttpHeader* outHead, 
                                      HttpBody* outBody,
                                      uint32 now,
                                      const char* dateStr );


   private:
#ifdef USE_XML

      /** Handle zoom_settings_request
       */
       bool xmlParseZoomSettingsRequest( DOMNode* cur, 
                                         DOMNode* out,
                                         DOMDocument* reply,
                                         bool indent );
      /**
       * Handle a xml request.
       */
      bool xmlParseRequest( DOMDocument* request,
                            DOMDocument* reply,
                            bool indent,
                            const HttpHeader& inHeaders );
      /**
       * Handle simple_poi_desc_request
       */
      bool xmlParseSimplePOIDescRequest( DOMNode* cur, DOMNode* out,
                                         DOMDocument* reply,
                                         bool indent );

      /**
       * Parses and handles an isab-mc2 element.
       */
      bool xmlParseIsabmc2( DOMNode* cur, 
                            DOMNode* out,
                            DOMDocument* reply,
                            bool indent,
                            const HttpHeader& inHeaders );


      /**
       * Parses and handles an request element.
       */
      bool xmlParsePublicRequest( DOMNode* cur, 
                                  DOMNode* out,
                                  DOMDocument* reply,
                                  bool indent );


      /**
       *    @name Handle isab-mc2.
       *    @memo Method for handling nodes in an isab-mc2 element.
       *    @doc  Method for handling nodes in an isab-mc2 element.
       */
      //@{
      
         /**
          * Parse and handle an auth-user element.
          */
         bool xmlParseAuth( DOMNode* cur, 
                            DOMNode* out,
                            DOMDocument* reply,
                            bool indent );
      
      
         /**
          * Parse and handle an user_request element.
          */
         bool xmlParseUserRequest( DOMNode* cur, 
                                   DOMNode* out,
                                   DOMDocument* reply,
                                   bool indent );
         
         /**
          * Parse and handle a route_request element.
          */
         bool xmlParseRouteRequest( DOMNode* cur, 
                                    DOMNode* out,
                                    DOMDocument* reply,
                                    bool indent );


         /**
          * Parse and handle a search_request element.
          */
         bool xmlParseSearchRequest( DOMNode* cur, 
                                     DOMNode* out,
                                     DOMDocument* reply,
                                     bool indent );

   /**
    * Parse and handle search_desc_request element
    */
   bool xmlParseSearchDescRequest( DOMNode* cur, 
                                   DOMNode* out,
                                   DOMDocument* reply,
                                   bool indent );
   /**
    * Parse and handle search_position_desc_request element
    */
   bool xmlParseSearchPositionDescRequest( DOMNode* cur, 
                                           DOMNode* out,
                                           DOMDocument* reply,
                                           bool indent );

   /**
    * Parse and handle compact_search_request element
    */
   bool xmlParseCompactSearchRequest( DOMNode* cur,
                                      DOMNode* out,
                                      DOMDocument* reply,
                                      bool indent ); 

   /**
    * Parse and handle one_search_request element
    */
   bool xmlParseOneSearchRequest( DOMNode* cur,
                                      DOMNode* out,
                                      DOMDocument* reply,
                                      bool indent ); 

   /**
    * Parse and handle category_list_request element
    */
   bool xmlParseCategoryListRequest( DOMNode* cur,
                                     DOMNode* out,
                                     DOMDocument* reply,
                                     bool indent ); 
   /**
    * Parse and handle category_tree_request element 
    */
   bool xmlParseCategoryTreeRequest( DOMNode* cur,
                                     DOMNode* out,
                                     DOMDocument* reply,
                                     bool indent );

   /**
    * Parse and handle local_category_tree_request element 
    */
   bool xmlParseLocalCategoryTreeRequest( DOMNode* cur,
                                          DOMNode* out,
                                          DOMDocument* reply,
                                          bool indent );

   /**
    * Parse and handle copyright_strings_request element
    */
   bool xmlParseCopyrightStringsRequest( DOMNode* cur,
                                         DOMNode* out,
                                         DOMDocument* reply,
                                         bool indent );

   /**
    * Parse and handle server_info_request element 
    */
   bool xmlParseServerInfoRequest( DOMNode* cur,
                                   DOMNode* out,
                                   DOMDocument* reply,
                                   bool indent );
   

   /**
    * Parse and handle ad_debit_request element
    */
   bool
   xmlParseAdDebitRequest( DOMNode* cur,
                           DOMNode* out,
                           DOMDocument* reply,
                           bool indent );

   /**
    * Parse and handle a expand_request element.
    */
   bool xmlParseExpandRequest( DOMNode* cur, 
                               DOMNode* out,
                               DOMDocument* reply,
                               bool indent );
   
   
   /**
    * Parse and handle a send_sms_request element.
    */
   bool xmlParseSendSMSRequest( DOMNode* cur, 
                                DOMNode* out,
                                DOMDocument* reply,
                                bool indent );
   
   
   /**
    * Parse and handle a user_login_request element.
    */
   bool xmlParseUserLoginRequest( DOMNode* cur, 
                                  DOMNode* out,
                                  DOMDocument* reply,
                                  bool indent );
   
   
   /**
    * Parse and handle a user_verify_request element.
    */
   bool xmlParseUserVerifyRequest( DOMNode* cur, 
                                   DOMNode* out,
                                   DOMDocument* reply,
                                   bool indent );
   
   
   /**
    * Parse and handle a user_logout_request element.
    */
   bool xmlParseUserLogoutRequest( DOMNode* cur, 
                                   DOMNode* out,
                                   DOMDocument* reply,
                                   bool indent );

   
   /**
    * Parse and handle a map_request element.
    */
   bool xmlParseMapRequest( DOMNode* cur, 
                            DOMNode* out,
                            DOMDocument* reply,
                            bool indent );

   
   /**
    * Parse and handle a poi_info_request element.
    */
   bool xmlParsePOIInfoRequest( DOMNode* cur, 
                                DOMNode* out,
                                DOMDocument* reply,
                                bool indent );
   
   
   /**
    * Parse and handle a poi_detail_request element.
    */
   bool xmlParsePOIDetailRequest( DOMNode* cur,
                                  DOMNode* out,
                                  DOMDocument* reply,
                                  bool indent );
   
   /**
    * Parse and handle a email_request element.
    */
   bool xmlParseEmailRequest( DOMNode* cur, 
                              DOMNode* out,
                              DOMDocument* reply,
                              bool indent );
   
   
   /**
    * Parse and handle a sms_format_request element.
    */
   bool xmlParseSMSFormatRequest( DOMNode* cur, 
                                  DOMNode* out,
                                  DOMDocument* reply,
                                  bool indent );
   
   
   /**
    * Parse and handle a user_cap_request element
    */
   bool xmlParseUserCapRequest( DOMNode* cur,
                                DOMNode* out,
                                DOMDocument* reply,
                                bool indent );
   /**
    * Parse and handle a user_show_request element.
    */
   bool xmlParseUserShowRequest( DOMNode* cur, 
                                 DOMNode* out,
                                 DOMDocument* reply,
                                 bool indent );
   

   /**
    * Parse and handle a user_favorites_request element.
    */
   bool xmlParseUserFavoritesRequest( DOMNode* cur, 
                                      DOMNode* out,
                                      DOMDocument* reply,
                                      bool indent );
   
   /**
    * Parse and handle an user_favorites_crc_request element.
    */
   bool xmlParseUserFavoritesCRCRequest( DOMNode* cur, 
                                         DOMNode* out,
                                         DOMDocument* reply,
                                         bool indent );
   
         /**
          * Parse and handle a sort_dist_request element.
          */
         bool xmlParseSortDistRequest( DOMNode* cur, 
                                       DOMNode* out,
                                       DOMDocument* reply,
                                       bool indent );


         /**
          * Parse and handle a top_region_request element.
          */
         bool xmlParseTopRegionRequest( DOMNode* cur, 
                                        DOMNode* out,
                                        DOMDocument* reply,
                                        bool indent );



         /**
          * Parse and handle a phone_manufacturer_request element.
          */
         bool xmlParsePhoneManufacturerRequest( DOMNode* cur, 
                                                DOMNode* out,
                                                DOMDocument* reply,
                                                bool indent );


         /**
          * Parse and handle a phone_model_request element.
          */
         bool xmlParsePhoneModelRequest( DOMNode* cur, 
                                         DOMNode* out,
                                         DOMDocument* reply,
                                         bool indent );


         /**
          * Parse and handle a user_track_request element.
          */
         bool xmlParseUserTrackRequest( DOMNode* cur, 
                                        DOMNode* out,
                                        DOMDocument* reply,
                                        bool indent );


         /**
          * Parse and handle a user_track_add_request element.
          */
         bool xmlParseUserTrackAddRequest( DOMNode* cur, 
                                          DOMNode* out,
                                          DOMDocument* reply,
                                          bool indent );


         /**
          * Parse and handle a user_debit_log_request element.
          */
         bool xmlParseUserDebitLogRequest( DOMNode* cur, 
                                           DOMNode* out,
                                           DOMDocument* reply,
                                           bool indent );


         /**
          * Parse and handle a user_find_request element.
          */
         bool xmlParseUserFindRequest( DOMNode* cur, 
                                       DOMNode* out,
                                       DOMDocument* reply,
                                       bool indent );


         /**
          * Parse and handle a transactions_request element.
          */
         bool xmlParseTransactionsRequest( DOMNode* cur, 
                                           DOMNode* out,
                                           DOMDocument* reply,
                                           bool indent );


         /**
          * Parse and handle a transaction_days_request element.
          */
         bool xmlParseTransactionDaysRequest( DOMNode* cur, 
                                              DOMNode* out,
                                              DOMDocument* reply,
                                              bool indent );


         /**
          * Parse and handle a activate_request element.
          */
         bool xmlParseActivateRequest( DOMNode* cur, DOMNode* out,
                                       DOMDocument* reply, bool indent,
                                       const HttpHeader& inHeaders );

         /**
          * Parse and handle a tunnel_request element.
          */
         bool xmlParseTunnelRequest( DOMNode* cur, 
                                     DOMNode* out,
                                     DOMDocument* reply,
                                     bool indent );
   
         /** Functions that handle POI reviews. */
         //@{
            /**
             * Parse and handle a poi_reveiw_requests element.
             */
            bool xmlParsePoiReviewRequests( DOMNode* cur, 
                                            DOMNode* out,
                                            DOMDocument* reply,
                                            bool indent );
            bool xmlParsePoiReviewAddRequest( DOMNode* cur, 
                                              DOMNode* out,
                                              DOMDocument* reply,
                                              int indentLevel,
                                              bool indent );
            bool xmlParsePoiReviewDeleteRequest( DOMNode* cur, 
                                                 DOMNode* out,
                                                 DOMDocument* reply,
                                                 int indentLevel,
                                                 bool indent );
            bool xmlParsePoiReviewListRequest( DOMNode* cur, 
                                               DOMNode* out,
                                               DOMDocument* reply,
                                               int indentLevel,
                                               bool indent );

            UserItem* findUser( uint32 uin, const MC2String& userID, 
                                const MC2String& userSessionID, 
                                const MC2String& userSessionKey );

            UserItem* findUser( DOMNode* cur );
            //@}

         /**
          * Parse and handle a error_report element.
          */
         bool xmlParseErrorReport( DOMNode* cur, 
                                   DOMNode* out,
                                   DOMDocument* reply,
                                   bool indent );

         /**
          * Parse and handle a show_activationcode element.
          */
         bool xmlParseShowActivationcode(  DOMNode* cur, 
                                           DOMNode* out,
                                           DOMDocument* reply,
                                           bool indent );

         /**
          * Parse and handle a expand_top_region element.
          */
         bool xmlParseExpandTopRegion(  DOMNode* cur, 
                                        DOMNode* out,
                                        DOMDocument* reply,
                                        bool indent );

         /**
          * Parse and handle a client_type_info element.
          */
         bool xmlParseClientTypeInfoRequest( DOMNode* cur, 
                                             DOMNode* out,
                                             DOMDocument* reply,
                                             bool indent );

         /**
          * Parse and handle a server_list_for_client_type element.
          */
         bool xmlParseServerListForClientTypeRequest( DOMNode* cur, 
                                                      DOMNode* out,
                                                      DOMDocument* reply,
                                                      bool indent );

         /**
          * Parse and handle a create_wayfinder_user element.
          */
         bool xmlParseCreateWayfinderUserRequest( DOMNode* cur, 
                                                  DOMNode* out,
                                                  DOMDocument* reply,
                                                  bool indent );

         /**
          * Parse and handle a update_hardware_keys element.
          */
         bool xmlParseUpdateKeysRequest( DOMNode* cur, 
                                         DOMNode* out,
                                         DOMDocument* reply,
                                         bool indent );
         /**
          * Parse and handle a get stored user data element.
          */
         bool xmlParseGetStoredUserData( DOMNode* cur, 
                                         DOMNode* out,
                                         DOMDocument* reply,
                                         bool indent );
         
         /**
          * Parse and handle a set stored user data element.
          */
         bool xmlParseSetStoredUserData( DOMNode* cur, 
                                         DOMNode* out,
                                         DOMDocument* reply,
                                         bool indent );

         /**
          * Parse and handle a friend finder request.
          */
         bool xmlParseFriendFinder( DOMNode* cur, 
                                    DOMNode* out,
                                    DOMDocument* reply,
                                    bool indent );
         
         /**
          * Parse and handle a friend finder information request.
          */
         bool xmlParseFriendFinderInfo( DOMNode* cur, 
                                        DOMNode* out,
                                        DOMDocument* reply,
                                        bool indent );

         /**
          * Parse and handle a cell id request.
          */
         bool xmlParseCellIDRequest( DOMNode* cur,
                                     DOMNode* out,
                                     DOMDocument* reply,
                                     bool indent );
         /**
          * Parse and handle a get pois in an area request.
          */
         bool xmlParsePOISearchRequest( DOMNode* cur,
                                        DOMNode* out,
                                        DOMDocument* reply,
                                        bool indent );

      //@}


      /**
       *    @name Handle user.
       *    @memo Method for handling nodes in a user_request.
       *    @doc  Method for handling nodes in a user_request.
       */
      //@{
         /**
          *    Parse and handle an user element in a user_request.
          */
         bool xmlParseUserRequestUser( DOMNode* cur, bool hasNewUser,
                                       bool newUser,
                                       DOMNode* out,
                                       DOMDocument* reply,
                                       int indentLevel,
                                       bool indent );

         /**
          *    Parse and handle an phone element in an user element.
          */
         bool xmlParseUserPhone( DOMNode* cur, UserUser* user ) const;


         /**
          * Change a users password.
          *
          * @param user The user to change password for.
          * @param newPassword The new password.
          * @param oldPassword The old password is checked before changing.
          * @param errorCode If problem then this is set to error code.
          * @param errorMessage If problem then this is set to error 
          *                     message.
          * @return True if all 
          */
         bool changeUserPassword( const UserUser* user, 
                                  const char* newPassword, 
                                  const char* oldPassword, 
                                  bool checkPassword,
                                  MC2String& errorCode, 
                                  MC2String& errorMessage );


         /**
          * Parse and handle an binary_key element in an user element.
          *
          * @param cur The binary_key element.
          * @param user The UserUser object of the user.
          * @param statusCodeString If problem then this is set to 
          *                         error code.
          * @param statusMessageString If problem then this is set to 
          *                            error message.
          */
         bool xmlParseUserLicence( DOMNode* cur, UserUser* user,
                                   MC2String& statusCodeString,
                                   MC2String& statusMessageString ) const;


         /**
          * Parse and handle an user_licence_key element in an user element.
          *
          * @param cur The user_licence_key element.
          * @param user The UserUser object of the user.
          * @param statusCodeString If problem then this is set to 
          *                         error code.
          * @param statusMessageString If problem then this is set to 
          *                            error message.
          */
         bool xmlParseUserLicenceKey( DOMNode* cur, UserUser* user,
                                      MC2String& statusCodeString,
                                      MC2String& statusMessageString ) const;


         /**
          * Parse and handle a region_access element in an user element.
          *
          * @param cur The region_access element.
          * @param user The UserUser object of the user.
          * @param statusCodeString If problem then this is set to 
          *                         error code.
          * @param statusMessageString If problem then this is set to 
          *                            error message.
          */
         bool xmlParseUserRegionAccess( 
            DOMNode* cur, UserUser* user,
            MC2String& statusCodeString, MC2String& statusMessageString ) const;


         /**
          * Parse and handle a right element in an user element.
          *
          * @param cur The right element.
          * @param user The UserUser object of the user.
          * @param statusCodeString If problem then this is set to 
          *                         error code.
          * @param statusMessageString If problem then this is set to 
          *                            error message.
          */
         bool xmlParseUserRight( DOMNode* cur, UserUser* user,
                                 MC2String& statusCodeString,
                                 MC2String& statusMessageString ) const;

         /**
          * Parse and handle a wayfindersubscription element in an user 
          * element.
          *
          * @param cur The wayfinder_subscription element.
          * @param user The UserUser object of the user.
          * @param statusCodeString If problem then this is set to 
          *                         error code.
          * @param statusMessageString If problem then this is set to 
          *                            error message.
          */
         bool xmlParseUserWayfinderSubscription( 
            DOMNode* cur, UserUser* user,
            MC2String& statusCodeString, 
            MC2String& statusMessageString ) const;


         /**
          * Parse and handle a token element in an user element.
          *
          * @param cur The token element.
          * @param user The UserUser object of the user.
          * @param statusCodeString If problem then this is set to 
          *                         error code.
          * @param statusMessageString If problem then this is set to 
          *                            error message.
          */
         bool xmlParseUserToken( DOMNode* cur, UserUser* user,
                                 MC2String& statusCodeString,
                                 MC2String& statusMessageString ) const;


         /**
          * Parse and handle a PIN element in an user element.
          *
          * @param cur The PIN element.
          * @param user The UserUser object of the user.
          * @param statusCodeString If problem then this is set to 
          *                         error code.
          * @param statusMessageString If problem then this is set to 
          *                            error message.
          */
         bool xmlParseUserPIN( DOMNode* cur, UserUser* user,
                               MC2String& statusCodeString,
                               MC2String& statusMessageString ) const;


         /**
          * Parse and handle a id_key element in an user element.
          *
          * @param cur The id_key element.
          * @param user The UserUser object of the user.
          * @param statusCodeString If problem then this is set to 
          *                         error code.
          * @param statusMessageString If problem then this is set to 
          *                            error message.
          */         
         bool xmlParseUserIDKey( DOMNode* cur, UserUser* user,
                                 MC2String& statusCodeString, 
                                 MC2String& statusMessageString ) const;
      //@}

      /**
       *    @name Handle search.
       *    @memo Methods for handling nodes in a search_request.
       *    @doc  Methods for handling nodes in a search_request.
       */
      //@{
         /**
          * Parse and handle a search_request_header.
          */
         bool xmlParseSearchRequestSearchRequestHeader( 
            DOMNode* cur, 
            DOMNode* out,
            DOMDocument* reply,
            int indentLevel,
            SearchRequestParameters& params,
            bool& latlonForSearchHits,
            XMLCommonEntities::coordinateType& positionSystem,
            bool& positionSearchItems,
            bool& positionSearchAreas,
            int32& searchAreaStartingIndex,
            int32& searchAreaEndingIndex,
            int32& searchItemStartingIndex,
            int32& searchItemEndingIndex,
            MC2String& errorCode, MC2String& errorMessage );


         /**
          * Parse and handle a search_preferences.
          */
         bool xmlParseSearchRequestSearchRequestHeaderPreferences( 
            DOMNode* cur, 
            DOMNode* out,
            DOMDocument* reply,
            int indentLevel,
            SearchRequestParameters& params,
            MC2String& errorCode, MC2String& errorMessage );


         /**
          * Parse and handle a search_query.
          */
         bool xmlParseSearchRequestSearchQuery(
            DOMNode* cur, 
            DOMNode* out,
            DOMDocument* reply,
            int indentLevel,
            bool indent,
            const SearchRequestParameters& params,
            bool latlonForSearchHits,
            XMLCommonEntities::coordinateType positionSystem,
            bool positionSearchItems,
            bool positionSearchAreas,
            int32 searchAreaStartingIndex,
            int32 searchAreaEndingIndex,
            int32 searchItemStartingIndex,
            int32 searchItemEndingIndex,
            MC2String& errorCode, MC2String& errorMessage );


         /**
          * Parse and handle a proximity_query.
          */
         bool xmlParseSearchRequestProximityQuery(
            DOMNode* cur, 
            DOMNode* out,
            DOMDocument* reply,
            int indentLevel,
            bool indent,
            const SearchRequestParameters& params,
            bool latlonForSearchHits,
            XMLCommonEntities::coordinateType positionSystem,
            bool positionSearchItems,
            bool positionSearchAreas,
            int32 searchAreaStartingIndex,
            int32 searchAreaEndingIndex,
            int32 searchItemStartingIndex,
            int32 searchItemEndingIndex,
            MC2String& errorCode, MC2String& errorMessage );

      //@}


      /**
       *    @name Handle route.
       *    @memo Methods for handling nodes in a route_request.
       *    @doc  Methods for handling nodes in a route_request.
       */
      //@{
         /**
          *    Parse and Handle a route_request_header.
          */
         bool xmlParseRouteRequestRouteRequestHeader( 
            DOMNode* cur, 
            DOMNode* out,
            DOMDocument* reply,
            int indentLevel,
            byte& routeCostA,
            byte& routeCostB,
            byte& routeCostC,
            byte& routeCostD,
            bool& avoidtollroad,
            bool& avoidhighway,
            ItemTypes::vehicle_t& routeVehicle,
            StringTableUtility::distanceFormat& distFormat,
            StringTableUtility::distanceUnit& routeMeasurement,
            StringTable::languageCode& language,
            bool& routeImageLinks,
            bool& routeTurnData,
            bool& routeRoadData,
            bool& routeItems,
            bool& abbreviateRouteNames,
            bool& routeLandmarks,
            uint32& routeOverviewImageWidth,
            uint32& routeOverviewImageHeight,
            uint32& routeTurnImageWidth,
            uint32& routeTurnImageHeight,
            ImageDrawConfig::imageFormat& routeImageFormat,
            MapSettingsTypes::defaultMapSetting& routeImageDisplayType,
            struct MapSettingsTypes::ImageSettings& imageSettings,
            XMLCommonEntities::coordinateType& bboxCoordinateSystem,
            bool& routeTurnBoundingbox,
            RouteID& previous_route_id,
            MC2String& reason );


         /**
          * Handles the two routeable_item_lists using the settings
          * provided and routes then prints result.
          */
         bool xmlHandleRouteRequest( 
            DOMNode* origin_list,
            DOMNode* destination_list,
            DOMElement* out,
            DOMDocument* reply,
            int indentLevel,
            byte routeCostA,
            byte routeCostB,
            byte routeCostC,
            byte routeCostD,
            bool& avoidtollroad,
            bool& avoidhighway,
            ItemTypes::vehicle_t routeVehicle,
            StringTableUtility::distanceFormat distFormat,
            StringTableUtility::distanceUnit routeMeasurement,
            StringTable::languageCode language,
            bool routeImageLinks,
            bool routeTurnData,
            bool routeRoadData,
            bool routeItems,
            bool abbreviateRouteNames,
            bool routeLandmarks,
            uint32 routeOverviewImageWidth,
            uint32 routeOverviewImageHeight,
            uint32 routeTurnImageWidth,
            uint32 routeTurnImageHeight,
            ImageDrawConfig::imageFormat routeImageFormat,
            MapSettingsTypes::defaultMapSetting routeImageDisplayType,
            struct MapSettingsTypes::ImageSettings& imageSettings,
            XMLCommonEntities::coordinateType bboxCoordinateSystem,
            bool routeTurnBoundingbox,
            RouteID previous_route_id,
            MC2String reason,
            bool indent );
         
         
         /**
          * Parse and Handle a route_preferences.
          */
         bool xmlParseRouteRequestRouteRequestHeaderRoutePreferences( 
            DOMNode* cur, 
            DOMNode* out,
            DOMDocument* reply,
            int indentLevel,
            byte& routeCostA,
            byte& routeCostB,
            byte& routeCostC,
            byte& routeCostD,
            bool& avoidtollroad,
            bool& avoidhighway,
            ItemTypes::vehicle_t& routeVehicle,
            StringTableUtility::distanceFormat& distFormat,
            StringTableUtility::distanceUnit& routeMeasurement,
            StringTable::languageCode& language,
            bool& routeImageLinks,
            bool& routeTurnData,
            bool& routeRoadData,
            bool& routeItems,
            bool& abbreviateRouteNames,
            bool& routeLandmarks,
            uint32& routeOverviewImageWidth,
            uint32& routeOverviewImageHeight,
            uint32& routeTurnImageWidth,
            uint32& routeTurnImageHeight,
            ImageDrawConfig::imageFormat& routeImageFormat,
            MapSettingsTypes::defaultMapSetting& routeImageDisplayType,
            struct MapSettingsTypes::ImageSettings& imageSettings,
            XMLCommonEntities::coordinateType& bboxCoordinateSystem,
            bool& routeTurnBoundingbox );


         /**
          * Parse and handle a route_settings
          */
         bool xmlParseRouteRequestRouteRequestHeaderRouteSettings(
            DOMNode* cur, 
            DOMNode* out,
            DOMDocument* reply,
            int indentLevel,
            byte& routeCostA,
            byte& routeCostB,
            byte& routeCostC,
            byte& routeCostD,
            bool& avoidtollroad,
            bool& avoidhighway,
            ItemTypes::vehicle_t& routeVehicle,
            StringTableUtility::distanceFormat& distFormat,
            StringTable::languageCode& language );
      //@}
   

      /**
       *    @name Handle expand.
       *    @memo Methods for handling nodes in a expand_request.
       *    @doc  Methods for handling nodes in a expand_request.
       */
      //@{
         /**
          *    Parse and Handle a expand_request_header.
          */
         bool xmlParseExpandRequestExpandRequestHeader( 
            DOMNode* cur, 
            DOMNode* out,
            DOMDocument* reply,
            int indentLevel,
            SearchRequestParameters& params,
            XMLCommonEntities::coordinateType& positionSystem,
            bool& locationNameOnlyCountryCity,
            MC2String& errorCode, MC2String& errorMessage );
         
         
         /**
          *    Parse and handle a expand_query.
          */         
         bool xmlParseExpandRequestExpandQuery(
            DOMNode* cur, 
            DOMNode* out,
            DOMDocument* reply,
            int indentLevel,
            bool indent,
            const SearchRequestParameters& params,
            XMLCommonEntities::coordinateType positionSystem,
            bool locationNameOnlyCountryCity );
      //@}


      /**
       *    @name Handle map requests.
       *    @memo Methods for handling nodes in a map_request.
       *    @doc  Methods for handling nodes in a map_request.
       */
      //@{
         /**
          *    Parse and Handle a map_request_header.
          */
         bool xmlParseMapRequestHeader( 
            DOMNode* cur, 
            MC2BoundingBox& bbox,
            uint16& width,
            uint16& height,
            ImageDrawConfig::imageFormat& imageType,
            const char*& routeIDstr,
            uint32& beforeTurn,
            uint32& afterTurn,
            MapSettingsTypes::defaultMapSetting& displayType,
            bool& showMap,
            bool& showTopographMap,
            bool& showPOI,
            bool& showRoute,
            bool& showScale,
            bool& showTraffic,
            struct MapSettingsTypes::ImageSettings& imageSettings,
            MC2String& errorCode, MC2String& errorMessage );


         /**
          *    Parse and Handle a route_data.
          */
         bool xmlParseMapRequestHeaderRouteData( 
            DOMNode* cur, 
            const char*& routeIDstr,
            uint32& beforeTurn,
            uint32& afterTurn,
            MC2String& errorCode, MC2String& errorMessage );
      //@}


      /**
       *    @name Handle send sms requests.
       *    @memo Methods for handling nodes in a send_sms_request.
       *    @doc  Methods for handling nodes in a send_sms_request.
       */
      //@{
         /**
          *    Parse a route_sms_message.
          */
         bool xmlParseSendSMSRequestRouteSMSMessage( 
            DOMNode* cur,
            CellularPhoneModel*& model,
            bool& wapLink );

         /**
          *   Parse a local_map_sms_settings.
          */
         bool xmlParseLocalMapSMSSettings( DOMNode* cur,
                                           CellularPhoneModel*& model );
                  


         
         /**
          * Parse a WAP Push Service Indication SMS
          */
         bool xmlParseWapPushServiceIndication( DOMNode* cur,
                                                MC2String& message,
                                                MC2String& href,
                                                MC2String& errorCode, 
                                                MC2String& errorMessage );
      //@}


      /**
       *    @name Handle email requests.
       *    @memo Methods for handling nodes in a email_request.
       *    @doc  Methods for handling nodes in a email_request.
       */
      //@{

         /**
          * Parse a local_map_data.
          */
         bool xmlParseLocalMapData( DOMNode* cur,
                                    StringTable::languageCode& language,
                                    char*& signature,
                                    MC2BoundingBox& lmBoundingbox,
                                    char*& localMapString,
                                    GfxFeatureMap* mapSymbolMap );
      //@}


      /**
       *    @name Handle user favorites request.
       *    @memo Methods for handling nodes in a 
       *          user_favorites_request.
       *    @doc  Methods for handling nodes in a 
       *          user_favorites_request.
       */
      //@{
         /**
          * Reads a favorite_id_list.
          * @param cur Must be a favorite_id_list element.
          * @param favIDs All favorite IDs are added to this vector.
          */
         void readFavoriteIDList( DOMNode* cur, 
                                  vector< uint32 >& favIDs );


         /**
          * Appends a favorite_id_list as child to cur.
          * @param cur Where to put the favorite_id_list node as 
          *            last child.
          * @param reply The document to make the nodes in.
          * @param indentLevel The indent to use.
          * @param indent If to use indent.
          * @param favoriteIDListName The name of the favorite ID list element.
          * @param favIDs The favorite IDs to add.
          */
         void appendFavoriteIDList( DOMNode* cur, DOMDocument* reply,
                                    int indentLevel, bool indent,
                                    const char* favoriteIDListName,
                                    const vector< uint32 >& favIDs );



         /**
          * Reads a favorite_list.
          * @param cur Must be a favorite_list element.
          * @param favorites All favorites are added to this vector.
          */
         bool readFavoriteList( DOMNode* cur, 
                                vector< UserFavorite* >& favorites,
                                MC2String& errorCode, 
                                MC2String& errorMessage );

         typedef vector< ItemInfoEntry > InfoVect;
         /**
          * Reads a fav_info element and adds it to v.
          */
         bool readFavInfo( DOMNode* cur, InfoVect& v, 
                           MC2String& errorCode, 
                           MC2String& errorMessage );
      //@}


      /**
       *    @name Handle sort dist request.
       *    @memo Methods for handling nodes in a 
       *          sort_dist_request.
       *    @doc  Methods for handling nodes in a 
       *          sort_dist_request.
       */
      //@{
         /**
          * Reads a all_favorites.
          * @param cur Must be a all_favorites element.
          * @param userID Set to the user_id of the all_favorites.
          * @param userSessionID Set to the user_session_id of the 
          *                      all_favorites.
          * @param userSessionKey Set to the user_session_key of the 
          *                       all_favorites.
          */
         void readAllFavorites( DOMNode* cur, 
                                MC2String& userID,
                                MC2String& userSessionID,
                                MC2String& userSessionKey );

 
         /**
          * Appends a favorite_list as child to cur.
          * @param cur Where to put the favorite_list node as 
          *            last child.
          * @param reply The document to make the nodes in.
          * @param indentLevel The indent to use.
          * @param indent If to use indent.
          * @param favoriteListName The name of the favorite list element.
          * @param positionSystem The position system to use.
          * @param fav_info_in_desc If to add fav_info elemens as text to 
          *                         description.
          * @param favorites The favorites to add.
          */
         void appendFavoriteList( 
            DOMNode* cur, DOMDocument* reply,
            int indentLevel, bool indent,
            const char* favoriteListName,
            XMLCommonEntities::coordinateType positionSystem,
            bool fav_info_in_desc,
            const vector< const UserFavorite* >& favorites );
 

      //@}


      /**
       *    @name Handle top region request.
       *    @memo Methods for handling nodes in a 
       *          top_region_request.
       *    @doc  Methods for handling nodes in a 
       *          top_region_request.
       */
      //@{
         /**
          * Parse a top_region_request_header.
          */
         bool xmlParseTopRegionRequestHeader( 
            DOMNode* cur,
            LangTypes::language_t& language,
            XMLCommonEntities::coordinateType& positionSystem,
            bool& addCountry, bool& addState, bool& addInternationalRegion,
            bool& addMetaregion,
            MC2String& errorCode, MC2String& errorMessage );


         /**
          * Appends a top_region_list as child to cur.
          * @param cur Where to put the top_region_list node as 
          *            last child.
          * @param reply The document to make the nodes in.
          * @param indentLevel The indent to use.
          * @param indent If to use indent.
          * @param language The prefered language.
          * @param positionSystem The position system to use.
          * @param addCountry If countries should be added to list.
          * @param addState If states shoukd be added to list.
          * @param addInternationalRegion If international regions should
          *        be added to list.
          * @param topRequest The TopRegionRequest to make list of.
          * @param clientTopRegionCRC The clients crc.
          * @param sortMatches If to sort the matches according to name or
          *                    add them in current order.
          * @return The CRC for the top region.
          */
         MC2String appendTopRegionList( 
            DOMNode* cur, DOMDocument* reply,
            int indentLevel, bool indent,
            LangTypes::language_t language,
            XMLCommonEntities::coordinateType positionSystem,
            bool& addCountry, bool& addState, bool& addInternationalRegion,
            bool& addMetaregion,
            const TopRegionRequest* topRequest,
            MC2String clientTopRegionCRC,
            bool sortMatches = true );
      //@}

      /**
       * Checks if the auth element has a valid mc2 user.
       * @param doc The document to check authorization in.
       * @param reply The document to make error status nodes in.
       * @param indent Set to true if reply should be indented.
       * @param development If client is under development.
       * @param inHead      Header.
       */
      bool checkAuthorization( const DOMDocument* doc,
                               DOMDocument* reply,
                               bool& indent,
                               bool& development,
                               const HttpHeader& inHead );


      /**
       * Make
       *
       * @param pos The position to make VanillaMatch for.
       * @param angle The angle at pos.
       * @param errorCode If problem then this is set to error code.
       * @param errorMessage If problem then this is set to error message.
       * @param language The requested language.
       * @param locationNameOnlyCountryCity If only to print country and city
       *                                    in location_name.
       * @return A VanillaMatch or NULL if positioning falied.
       */
      VanillaMatch* getVanillaMatchFromPosition( 
         const MC2Coordinate& pos,
         uint16 angle,
         MC2String& errorCode,
         MC2String& errorMessage,
         LangTypes::language_t language = LangTypes::english,
         bool locationNameOnlyCountryCity = false );
   
      /**
       * Extracts data from a position_item and puts it into a VanillaMatch.
       *
       * @param positionItem The position_item node to get the data from.
       * @param errorCode If problem then this is set to error code.
       * @param errorMessage If problem then this is set to error message.
       * @param language The requested language.
       * @param locationNameOnlyCountryCity If only to print country and city
       *                                    in location_name.
       * @return A VanillaMatch or NULL if positioning falied.
       */
      VanillaMatch* getVanillaMatchFromPositionItem( 
         DOMNode* positionItem, 
         MC2String& errorCode, MC2String& errorMessage,
         LangTypes::language_t language = LangTypes::english,
         bool locationNameOnlyCountryCity = false );


      /**
       * Extracts data from a valid routerequest match and puts it into a 
       * VanillaMatch.
       *
       * @param routeReq The RouteRequest to get the data from.
       * @param index The index of the valid match.
       * @param origin If it is a origin of destination match to extract
       *               from the RouteRequest.
       * @param matches The original matches in the route.
       * @return A VanillaMatch or NULL if something was really wrong.
       */
      VanillaMatch* getVanillaMatchFromRouteRequestValidMatch(
         RouteRequest* routeReq,
         uint32 index,
         bool origin,
         VanillaVector matches ) const;
      
      /**
       * Reads a routeable_item_list and puts the items into the 
       * vanillaVector as VanillaMatches. Position items are expanded
       * and inserted as VanillaStreetMatches.
       *
       * @param searchItemList The list of items to extract.
       * @param vanillaVector The vector to put the items into.
       * @param errorCode If problem then this is set to error code.
       * @param errorMessage If problem then this is set to error message.
       * @return True if all went ok, false if error.
       */
      bool readAndPositionRouteableItemList( 
         DOMNode* searchItemList,
         VanillaVector& vanillaVector,
         MC2String& errorCode, MC2String& errorMessage );

      /**
       * Expands a category.
       *
       * @param matches Hits are added to this vector.
       * @param itemMapID The MapID of the category to expand.
       * @param itemItemID The ItemID of the category to expand.
       * @param expandUnique expand unique categories.
       */
      bool expandCategory( VanillaVector& matches,
                           uint32 itemMapID,
                           uint32 itemItemID,
                           const SearchRequestParameters& params,
                           bool expandUnique = false );

      /**
       * Gets the coordinates for a vector of vanillamatches.
       * 
       * @param matches The VanillaMatches to get coordinates for.
       * @param latitudes Filled with latitudes of the matches.
       * @param longitudes Filled with the longitudes of the matches.
       * @param hasLatLon Set to true if latitudes and longitudes has
       *                  valid coordinates in the same index.
       */
      bool getCoordinatesForMatches( VanillaVector& matches,
                                     Vector& latitudes,
                                     Vector& longitudes,
                                     bool*& hasLatLon );


      /**
       * Gets the coordinates for a vector of overviewmatches.
       * 
       * @param matches The OverviewMatches to get coordinates for.
       * @param latitudes Filled with latitudes of the matches.
       * @param longitudes Filled with the longitudes of the matches.
       * @param hasLatLon Set to true if latitudes and longitudes has
       *                  valid coordinates in the same index.
       */
      bool getCoordinatesForMatches( vector<OverviewMatch*>& matches,
                                     Vector& latitudes,
                                     Vector& longitudes,
                                     bool*& hasLatLon );

      
      /**
       * Reads image_settings node at curr.
       * @param curr The image_settings node.
       * @param bbox The  to fill the data into.
       * @return True if curr really is a image_settings node, 
       *         false if not.
       */
      bool readImageSettings( 
         DOMNode* cur, 
         struct MapSettingsTypes::ImageSettings& imageSettings );


      /**
       * Appends a info_item as child to curr.
       * 
       * @param curr Where to put the info_item node as last child.
       * @param reply The document to make the nodes in.
       * @param i The index of the info item in r.
       * @param r The ItemInforequest to get data from.
       * @para, format The coordinate format to use.
       * @param indentLevel The indent to use.
       * @param indent If indent.
       * @return The added InfoItem node.
       */
      DOMElement* appendInfoItem( DOMNode* cur, DOMDocument* reply,
                                  uint32 i, ItemInfoRequest* r,
                                  XMLCommonEntities::coordinateType 
                                  format,
                                  bool includeCategoryID,
                                  int indentLevel, bool indent );

      /**
       * Reads a map_symbol_list and adds the features to map.
       * @param cur The map_symbol_list element.
       * @param map The GfxFeatureMap to add the map_symbols to.
       * @return True if content was ok, false if not.
       */
      bool readMapSymbolList( DOMNode* cur, GfxFeatureMap* map );


      /**
       * Reads a map_symbol as a GfxSymbolFeature and adds it to the map.
       * @param cur The map_symbol element.
       * @param map The GfxFeatureMap to add the map_symbols to.
       * @return True if content was ok, false if not.
       */
      bool readAndAddMapSymbolItem( DOMNode* cur, GfxFeatureMap* map );

      /**
       * Check if a user has acces to a service.
       *
       * @param user The user to check access for.
       * @param service The service to check for.
       * @param checkTime If to check if user has access now, default true.
       * @param levelmask The needed level, default any.
       */
      bool checkUserAccess( UserUser* user, const char* service,
                            bool checkTime = true, 
                            UserEnums::userRightLevel levelmask =
                            UserEnums::ALL_LEVEL_MASK );

      /**
       * Appends a UserItem as a user element tree to out.
       *
       * @param showAll If to show all or just the needfull.
       */
      void appendUser( UserItem* userItem, 
                       DOMNode* cur, DOMDocument* reply,
                       int indentLevel, bool indent, 
                       bool showAll = true );

      /**
       * Handles a xml request.
       *
       * @param inHead The HTTP header of the request.
       * @param inBody The body content, may be empty, of the HTTP request.
       * @param paramsMap The startline's parameters.
       * @param outHead The HTTP header of the reply.
       * @param outBody The content of the reply.
       * @param now The time of the request.
       * @param dateStr The time of the request as a string.
       * @return True if the request was handled and outBody was filled 
       *         with the reply.       
       */
      bool handleXMLHttpRequest( HttpHeader* inHead, 
                                 HttpBody* inBody,
                                 stringMap* paramsMap,
                                 HttpHeader* outHead, 
                                 HttpBody* outBody,
                                 uint32 now,
                                 const char* dateStr );

       /**
        * Makes a favorite element.
        * @param favorite The UserFavorite to make an element for.
        * @param positionSystem The positionSystem to use.
        * @param fav_info_in_desc If to add fav_info elemens as text to 
        *                         description.
        * @param reply The document to make the element in.
        * @return The element with the favorite.
        * @param indentLevel The indent to use.
        * @param indent If indent.
        */
       DOMElement* makeUserFavoriteElement( 
          const UserFavorite* favorite,
          XMLCommonEntities::coordinateType positionSystem,
          bool fav_info_in_desc,
          DOMDocument* reply,
          int indentLevel, bool indent );

      /**
       * Extracts data from a top_region and puts it into a TopRegionMatch.
       *
       * @param topRegion The top_region node to get data from.
       * @param errorCode If problem then this is set to error code.
       * @param errorMessage If problem then this is set to error message.
       * @return A TopRegionMatch or NULL if problem occured.
       */
      TopRegionMatch* getTopRegionMatchFromTopRegion( 
         DOMNode* topregion, 
         MC2String& errorCode, MC2String& errorMessage ) const;

      /**
       * Checks if the auth element has a valid mc2 user.
       *
       * @param doc The document to check authorization in.
       * @param reply The document to make error status nodes in.
       * @param indent Set to true if reply should be indented.
       * @param development If client is under development.
       */
      bool checkPublicAuth( const DOMDocument* doc, DOMDocument* reply,
                            bool& indent, bool& development );

      /**
       * Reads a user-node and sets the changed values in the supplied
       * user object.
       *
       * @param userNode The user node to parse.
       * @param user The user to set data in.
       * @param newPassword Set the the new password in node.
       * @param oldPassword Set the the old password in node.
       * @param haveNewPassword Set to true if user-node contains a
       *                        new password node.
       * @param haveOldPassword Set to true if user-node contains a
       *                        old password node.
       * @param errorCode If problem then this is set to error code.
       * @param errorMessage If problem then this is set to error message.
       * @return True if parsed ok, false if an error.
       */
      bool readUser( DOMNode* userNode, UserUser* user, 
                     MC2String& newPassword, MC2String& oldPassword,
                     bool& haveNewPassword, bool& haveOldPassword,
                     MC2String& errorCode, MC2String& errorMessage ) const;


      /**
       * Method for finding users that matches the UserElement.
       *
       * @param elem The element to search with.
       * @param nbrUsers Set to the numbers of matching users.
       * @param uins Set to the vector with the matching UINs, caller must 
       *             delete it.
       * @param logonIDs Set to the vector with the matching logonIDs,
       *                 caller must delete it.
       * @param ansCont Set to the reply PacketContainer, caller must 
       *                 delete it.
       * @param errorCode If problem then this is set to error code.
       * @param errorMessage If problem then this is set to error message.
       * @return True if all communication with UserModule was ok,
       *         false if communication/database error.
       */
      bool getUsersFromUserElement( const UserElement* elem, 
                                    uint32& nbrUsers,
                                    uint32*& uins,
                                    const_char_p*& logonIDs,
                                    PacketContainer*& ansCont,
                                    MC2String& errorCode, 
                                    MC2String& errorMessage );




      /**
       * Checks the body for xml document header and adds if none and
       * adds dtd to doctype if not present.
       *
       * @param body The body to check and fix. May be changed.
       * @param errStr The error if method returns false.
       * @param linesAdded Set to the number of lines that was added to 
       *                   request.
       * @param requestName The name of the request.
       * @return True if body is ready for parsing, false if not.
       */
      bool checkAndFixHeader( HttpBody* body, MC2String& errStr, 
                              uint32& linesAdded, MC2String& requestName );


      /**
       * Method for doing external http authentication in
       * checkAuthorization and xmlParseActivateRequest.
       */
      bool doHttpAuth( UserItem*& user, MC2String& unauthorizedStr, 
                       MC2String& errorCode, const MC2String& extType, 
                       LangTypes::language_t clientLang,
                       const MC2String& currentID, 
                       const MC2String& newID,
                       bool checkTime = true );

      /**
       * Checks if current user needs a new ServerList.
       */
      bool needsNewServerList() const;


      /**
       * Get the level of the client set in m_clientType.
       */
      UserEnums::userRightLevel getClientTypeLevel() const;

   const NamedServerList* 
   getServerList( const MC2String& fixedServerListName ) const;

      /**
       * Get the language_t for a language string.
       */
      LangTypes::language_t getStringAsLanguage( const char* langStr ) const;

      /**
       * Get the languageCode for a language string.
       */
      StringTable::languageCode getLanguageCode( const char* langStr ) const;

      /**
       * Get the right for a service string.
       */
      UserEnums::userRightService getRightService( const char* service ) const;

      /**
       * The xml parser.
       */
      XercesDOMParser* m_parser;

      /**
       * Get the user for a set of hardware keys.
       */
      bool getUserFromLicenceKeys( 
         const UserLicenceKeyVect& hwKeys,
         UserItem*& hardwareUser, 
         const MC2String& noSuchLicenceKeyErrorCode,
         MC2String& errorCode,
         MC2String& errorMessage,
         bool wipeFromCache = false );

      /**
       * Get the user for a hardware key.
       */
      bool getUserFromLicenceKey( 
         const UserLicenceKey& licenceKey, 
         UserItem*& hardwareUser, 
         const MC2String& noSuchLicenceKeyErrorCode,
         MC2String& errorCode,
         MC2String& errorMessage,
         bool wipeFromCache = false );

      /**
       * Add a Wayfinder (trial) user for a licence key.
       * Checks for hardware key activation code, IMEI:00440000113342.
       *
       * @param errorCode If problem then this is set to error code.
       * @param errorMessage If problem then this is set to error 
       *                     message.
       * @param passwd Set to the password of the user.
       * @param server Set to the server name if redirect, if errorCode
       *               is -211 that is.
       * @param userKey The licence to add user for, NULL if none.
       * @param userItem Set to the the new user.
       * @param startTime The start time of initial access.
       * @param endTime The end time of initial access. If same as 
       *                startTime then no start acces right is added.
       * @param regionID The region of initial access.
       * @param lang The language of the client.
       * @param clientType The clientType, NULL if none.
       * @param clientTypeOptions The clientTypeOptions, NULL if none.
       * @param programV Vector of three uint32.
       * @param activationCode The activation code used to create user,
       *                       empty if not so.
       * @param activationRegionID The selected region, if activation code 
       *                           is choose right.
       * @param extraTypes Some extra types of UserRights to add.
       * @param createWFST The WFST to use, default TRIAL.
       * @param transactionDays The number of transaction days, default 0.
       * @param brand The brand of the user, defualt empty.
       * @param fixedUserName The fixed user name to use, default NULL.
       * @param extraComment Extra text added to operatorcomment.
       * @param extraElements Elements to add to user, elements are
       *                      owned by userItem after call.
       * @param userNamePrefix The explicitly set user name prefix, 
       *                       uses default if NULL. Overrides 
       *                       fixedUserName if not NULL.
       * @param email The email address to set in user, NULL if none.
       * @param name The name to set in user, split into first and last 
       *             name, NULL if none.
       * @param opt_in_name  The name of customer contact info to set.
       * @param opt_in_value The value to set for opt_in_name.
       * @param extraRights additional rights for user
       * @param autoKey The licence to check for auto activation, NULL if none.
       * @param checkAndUseAC If the AC should be checked and used by this
       *                      method or not, default not.
       * @return True if ok else error in errorCode and errorMessage.
       */
      bool createWayfinderUserForXML( 
         MC2String& errorCode, 
         MC2String& errorMessage,
         MC2String& passwd,
         MC2String& server,
         const UserLicenceKey* userKey, 
         UserItem*& userItem,
         uint32 startTime,
         uint32 endTime,
         uint32 regionID,
         LangTypes::language_t lang, 
         const char* clientType,
         const char* clientTypeOptions,
         const uint32* programV,
         const MC2String& activationCode,
         uint32 activationRegionID = MAX_INT32,
         const ParserUserHandler::ExtraTypes& extraTypes = 
         ParserUserHandler::ExtraTypes(),
         WFSubscriptionConstants::subscriptionsTypes createWFST = 
         WFSubscriptionConstants::TRIAL,
         int32 transactionDays = 0,
         const MC2String& brand = "",
         const char* fixedUserName = NULL,
         const char* extraComment = NULL,
         const ParserUserHandler::UserElementVector& extraElements = 
         ParserUserHandler::UserElementVector(),
         const char* userNamePrefix = NULL,
         const char* email = NULL,
         const char* name = NULL, 
         const MC2String& opt_in_name = "",
         const MC2String& opt_in_value = "",
         const MC2String& extraRights = MC2String(""),
         const UserLicenceKey* autoKey = NULL,
         bool checkAndUseAC = false );

   /**
    * Check with external authority if user is allowed to search.
    *
    * @param root Node to add error messages below
    * @param reply The reply document
    * @param params CompactSearch settings used
    * @param indent If the XML should be indented
    * @return True if allowed to search otherwise false.
    *         When false is returned error message has also 
    *         been added to the XML document.
    */
   bool checkAllowedToSearch( DOMNode* root,
                              DOMDocument* reply,
                              CompactSearch& params,
                              bool indent );
      
public:
      /**
       * "Content-Type" string.
       */
      static const MC2String ContentTypeStr;

      /**
       * Get the InfoTypeConverter.
       */
      const InfoTypeConverter& getInfoTypeConverter() const;

private:
#endif

      /**
       *    Get the name of this server to be used in the debiting.
       *    @return The name of this server, will always return the 
       *             MC2String "XML".
       */
      virtual const char* getServerType();

   /**
    *    Prints log information about the user that was authorized.
    *    The information includes info about the user and its last
    *    used client. The output can also be customized with a prefix.
    *
    *    @param prefix   A string which will be printed before
    *                    " User accepted...", this can be used to
    *                    show for instance where logUserAccepted is called
    *                    from or additional information about the user.
    */
   void logUserAccepted( const MC2String& prefix );

      /**
       * The name of this server to use when debiting.
       */
      char* m_serverName;


      /**
       * The incomming HttpHeader of the HttpRequest.
       */
      HttpHeader* m_inHead;


      /**
       * The allowed requests tag names, all if empty.
       */
      set<MC2String> m_allowedRequests;

      /// Helper for stuff and etc.
      XMLExtServiceHelper* m_extServiceHelper;
      friend class XMLExtServiceHelper;

   /// converts from info type to string and vice versa
   auto_ptr<InfoTypeConverter> m_infoTypes;


      /**
       * The holder of the authetication data.
       */
      XMLAuthData* m_authData;
};


#endif // XMLPARSERTHREAD_H
