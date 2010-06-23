/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTEMESSAGEREQUEST_H
#define ROUTEMESSAGEREQUEST_H

#include "config.h"

#include "StringTable.h"
#include "Request.h"
#include "UserConstants.h"
#include "MimePart.h"
#include "RouteMessageRequestType.h"
#include "ImageDrawConfig.h"
#include "MapSettings.h"
#include "RouteArrowType.h"

#include "NotCopyable.h"

#include <list>

class MimeMessage;
class RouteReplyPacket;
class ExpandRouteReplyPacket;
class ExpandStringItem;
class ExpandItemID;
class GfxFeatureMapImageRequest;
class TopRegionRequest;
class CopyrightHandler;
class UserItem;

/** 
 * A Request that makes a message of a route that optinally can be sent
 * as an email.
 *
 */
class RouteMessageRequest : public Request, private NotCopyable {
   public:
      /**
        *   The timout-time before resending a EMailRequestPacket to 
        *   the EmailModule (in ms).
        */
      static const uint32 EMAILREQUESTPACKET_TIMEOUT;

      /**
       * Create a RouteMessageRequest.
       *
       * @param reqID unique request ID.
       * @param routeID The ID of the route, used to update the valid time 
       *                of the route in the database, set to 0 if not to
       *                update.
       * @param routeCreateTime The creation time of the route, used to 
       *                        update the valid time of the route in the
       *                        database, set to 0 if not to update.
       * @param lang The desired language of the message.
       * @param routePack The RouteReplyPacket with the route.
       * @param expand The ExpandRouteReplyPacket with the route.
       * @param exp The ExpandItemID with the RouteItems stored, may be
       *            NULL then a new one is created from expand.
       *
       * @param routeOverviewWidth The width of the overview image.
       * @param routeOverviewHeight The height of the overview image.
       * @param routeTurnWidth The width of the turn images.
       * @param routeTurnHeight The height of the turn images.
       * @param originString String describing the origin, like 
       *                     STOCKHOLM-BROMMA AIRPORT
       * @param originLocationString String describing the origin's 
       *                             location, like Alvik.
       * @param destinationString String describing the destination, like 
       *                          Gustavslundsvägen Alvik.
       * @param destinationLocationString String describing the 
       *                                  destination's location, 
       *                                  like Alvik.
       * @param routeVehicle The type of vehicle at the start of the route.
       * @param topReq Pointer to valid TopRegionRequest with data
       * @param makeImages If the message should contain the images
       *                   refered to in the route.
       * @param makeLink If a link to a webpage containing the html version
       *                 of the message.
       * @param realMapURLS If map image urls should be real ones that
       *                    can be sent to a [XML|HTTP]Server.
       * @param includeLandmarks If landmarks should be show in route 
       *                         description, default true.
       * @param useNavTurnBoundingBox If use alternative 
       *        getNavCrossingMC2BoundingBox method for making turn bboxes,
       *        default false, requires coordinates in expanded route.
       * @param exportMap If return ExportGfxFeatureMap data and not image
       *                  map data, default false.
       * @param onlyOverview If make a route message with only overview 
       *                     image.
       * @param contentType The type of message wanted.
       * @param turnImageType The type of route turn images to use.
       * @param arrowType The type of turn arrow to draw, default NOT_USED.
       * @param maxMessageSize The maximum size of a message, default 
       *                       MAX_UINT32.
       * @param sendEmail If the message should be sent as an email.
       * @param toMailAdr The receipitent of the email, used if sendEmail.
       * @param fromMailAdr The sender of the email, used if sendEmail.
       * @param subject The subject of the email, used if sendEmail.
       * @param signature The signuture, text last, of the email.
       * @param defaultFormat The default format of the images. 
       *        Default PNG.
       * @param user The user doing the request, default NULL.
       * @param contentLocation If to add Content-Location or Content-ID,
       *                        default Content-ID.
       * @param mapSetting The defaultMapSetting to use. Default STD.
       * @param imageSettings The ImageSettings to use, default NULL.
       * @param messageStartFmt Format string where:
       *                     "%DIRECTIONS%", "%FROM%", "%TO%",
       *                     "%ORIGINSTRING%","%DESTINATIONSTRING%", 
       *                     "%ORIGINLOCATIONSTRING%",
       *                     "%DESTINATIONLOCATIONSTRING%",
       *                     "%ROUTEOVERVIEW_URI%","%ROUTEOVERVIEW_WIDTH%",
       *                     "%ROUTEOVERVIEW_HEIGHT%", 
       *                     "%ROUTEOVERVIEW_ALT%", "%ROUTE_INFO%",
       *                     "%VEHICLE%, "%ROUTE_VEHICLE%", 
       *                     "%TOTAL_DISTANCE%","%ROUTE_TOTAL_DISTANCE%",
       *                     "%TOTAL_TIME%", "%ROUTE_TOTAL_TIME%" 
       *                     are replaced by actual strings
       *                     (use without citations).
       * @param messageRestartFmt Format string where:
       *                     "%DIRECTIONS%", "%FROM%", "%TO%",
       *                     "%ORIGINSTRING%","%DESTINATIONSTRING%", 
       *                     "%ORIGINLOCATIONSTRING%",
       *                     "%DESTINATIONLOCATIONSTRING%",
       *                     "%ROUTEOVERVIEW_URI%","%ROUTEOVERVIEW_WIDTH%",
       *                     "%ROUTEOVERVIEW_HEIGHT%", 
       *                     "%ROUTEOVERVIEW_ALT%", "%ROUTE_INFO%",
       *                     "%VEHICLE%, "%ROUTE_VEHICLE%", 
       *                     "%TOTAL_DISTANCE%","%ROUTE_TOTAL_DISTANCE%",
       *                     "%TOTAL_TIME%", "%ROUTE_TOTAL_TIME%" 
       *                     are replaced by actual strings
       *                     (use without citations).
       * @param messageRouteTurnFmt Formart string where:
       *                     "%ROUTE_DESCRIPTION%", "%ROUTE_TURN_URI%",
       *                     "%ROUTE_TURN_WIDTH%", "%ROUTE_TURN_HEIGHT%"
       *                     "%ROUTE_TURN_ALT%", "%ROUTE_PRE_LANDMARKS%",
       *                     "%ROUTE_POST_LANDMARKS%"
       *                     are replaced by actual strings
       *                     (use without citations).
       * @param messageRoutePreTurnLandmarkFmt Format string used for pre
       *        turn landmarks in messageRouteTurnFmt.
       *        "%ROUTE_LANDMARK_DESCRIPTION%".
       * @param messageRoutePostTurnLandmarkFmt Format string used for post
       *        turn landmarks in messageRouteTurnFmt.
       *        "%ROUTE_LANDMARK_DESCRIPTION%".
       * @param messageEndFmt Formart string where:
       *                      "%SIGNATURE%" is replaced by actual string
       *                      (use without citations).
       * @param nbrMessageResources The number of resources that are
       *                            static in the message.
       * @param messageResourcesURI The URI of the messageresources.
       * @param messageResourcesBuffLength The length of the resources
       *        in messageResourcesBuff.
       * @param messageResourcesBuff The raw bytes the the 
       *                             messageResources.
       * @param navigatorCrossingMaps  If this request is only used for 
       *                               getting crossingmaps for the
       *                               navigator 
       *                               (exportscaleable gfxfeaturemaps).
       *                               This means that the result is not
       *                               valid for anything else than just
       *                               to use as crossing maps.
       * @param mapSettings            By submitting this parameter,
       *                               the information in mapSetting
       *                               and imageSettings is overridden.
       *                               Ie. if you want to specify more
       *                               detailed mapsettings then you can
       *                               do by those two parameters.
       *                               Note however, if realMapURLs is 
       *                               true, then the mapURLs will reflect
       *                               the information in mapSetting and
       *                               imageSettings, and not the
       *                               information from this parameter.
       */
      RouteMessageRequest( const RequestData& reqData,
                           uint32 routeID, uint32 routeCreateTime,
                           StringTable::languageCode lang,
                           const RouteReplyPacket* p,
                           ExpandRouteReplyPacket* expand,
                           ExpandItemID* exp,
                           uint16 routeOverviewWidth,
                           uint16 routeOverviewHeight,
                           uint16 routeTurnWidth,
                           uint16 routeTurnHeight,
                           const char* originString,
                           const char* originLocationString,
                           const char* destinationString,
                           const char* destinationLocationString,
                           ItemTypes::vehicle_t routeVehicle,
                           const TopRegionRequest* topReq,
                           bool makeImages = true,
                           bool makeLink = false,
                           bool realMapURLs = true,
                           bool includeLandmarks = true,
                           bool useNavTurnBoundingBox = false,
                           bool exportMap = false,
                           bool onlyOverview = false,
                           RouteMessageRequestType::MessageContent 
                           contentType = 
                           RouteMessageRequestType::HTML_CONTENT,
                           UserConstants::RouteTurnImageType turnImageType
                           =UserConstants::ROUTE_TURN_IMAGE_TYPE_MAP_IMAGE,
                           RouteArrowType::arrowType arrowType = 
                           RouteArrowType::NOT_USED,
                           uint32 maxMessageSize = MAX_UINT32,
                           bool sendEmail = false,
                           const char* toMailAddr = NULL,
                           const char* fromMailAddr = NULL,
                           const char* subject = NULL,
                           const char* signature = NULL,
                           ImageDrawConfig::imageFormat defaultFormat = 
                           ImageDrawConfig::PNG,
                           const CopyrightHandler* copyrights = NULL,
                           const UserItem* user = NULL,
                           bool contentLocation = false,
                           MapSettingsTypes::defaultMapSetting mapSetting =
                           MapSettingsTypes::MAP_SETTING_STD,
                           struct MapSettingsTypes::ImageSettings* 
                           imageSettings = NULL,
                           const char* messageStartFmt = NULL,
                           const char* messageRestartFmt = NULL,
                           const char* messageStartTextFmt = NULL,
                           const char* messageRouteTurnFmt = NULL,
                           const char* messageRouteTurnTextFmt = NULL,
                           const char* messageRoutePreTurnLandmarkFmt 
                           = NULL,
                           const char* messageRoutePostTurnLandmarkFmt 
                           = NULL,
                           const char* messageEndFmt = NULL,
                           const char* messageEndTextFmt = NULL,
                           uint32 nbrMessageResources = MAX_UINT32,
                           char** messageResourcesURI = NULL,
                           uint32* messageResourcesBuffLength = NULL,
                           byte** messageResourcesBuff = NULL,
                           MapSettings* mapSettings = NULL
                           );


      /**
       * Destructor deletes all resources.
       */
      virtual ~RouteMessageRequest();

      
      /**
       * Gets the next packet to send.
       * @return The next packet to send.
       */
      virtual PacketContainer* getNextPacket();

       
      /**
       * Process the answer from module.
       * @param ansCont The answer to process might be NULL.
       */
      virtual void processPacket( PacketContainer* ansCont );
       
       
      /**
       * @return The answer if sendEmail then the answer is the 
       *         SendEmailReplyPacket.
       */
      virtual PacketContainer* getAnswer();
      
      
      /**
       * The resulting MimeMessage.
       * @param index The index of the MimeMessage to get. Default 0.
       * @return The MimeMessage at index index or NULL if index is out of
       *         range.
       */
      inline const MimeMessage* getMimeMessage( uint32 index = 0 ) const;

      
      /**
       * The number of resulting MimeMessages.
       * @return The number of MimeMessages.
       */
      inline uint32 getNbrMimeMessages() const;


      /**
       * Loads the file.
       * @param buff Set to the buffer with the file, user must delete 
       *             this.
       * @param fileName The path of the file.
       * @param binary If not binary then an extra byte is allocated.
       * @return Negative value if error else size of buff.
       */
      static int loadFile( byte*& buff, const char* fileName, 
                           bool binary = false );


      /**
       * Saves the file.
       * @param buff The buffer with the file.
       * @param size The length of buff in bytes.
       * @param fileName The path of the file.
       * @return Negative value if error else size of buff.
       */
      static int saveFile( byte*& buff, uint32 size,
                           const char* fileName );


      /**
       * Replaces all occurrances of findStr with replaceStr in str.
       * @param str The string to work with.
       * @param findStr The string to find.
       * @param replaceStr The string to replace with.
       * @param wapFormat If the replaceStr should be wap formated,
       *                  default false.
       * @return The number of substrings replaced in str.
       */
      static uint32 replaceString( MC2String& str, const char* findStr, 
                                   const char* replaceStr, 
                                   bool wapFormat = false );


      /**
       * Checks a string for MessageContentType.
       * @param str The string to check.
       * @param defaultType The type to return if str doesn't match any
       *        MessageContent, default HTML_CONTENT.
       */
      static RouteMessageRequestType::MessageContent
         MessageContentFromString( 
            const char* str,
            RouteMessageRequestType::MessageContent defaultType = 
            RouteMessageRequestType::HTML_CONTENT );


      /**
       * Makes a mimepart from indata.
       * @param content The content.
       * @param mainContentType The main content type, text or application.
       * @param messageTextMarkupType The sub text type, html.
       * @param messageApplicationMarkupType The sub application type,
       *                                     smil.
       * @param characterSet The characterSet of the content.
       * @param pageName The name of the content.
       */
      static MimePart* makeMimePart( 
         const MC2String& content, 
         MimePart::mainContentType mainContentType,
         MimePartText::contentType messageTextMarkupType,
         MimePartApplication::contentType messageApplicationMarkupType,
         MimePart::characterSet characterSet, 
         const char* pageName );


      /**
       * Transforms a string from internal ISO charset to another charset.
       *       
       * @param str The string in internal ISO charset.
       * @param outCharSet The characterset to change to.
       */
      static void changeCharacterSet( MC2String& str,
                                      MimePart::characterSet outCharSet );

      /**
       * Get number gfxfeaturemapimage requests.
       * @return  Number of gfxfeaturemapimage requests.
       */
      inline uint32 getNbrGfxFeatureMapImageRequests() const;

      /**
       * Get a gfxfeaturemapimage request.
       * @param   i  Index of the gfxfeaturemapimage request. 
       *             0 <= i < getNbrGfxFeatureMapImageRequests().
       * @return  The request.
       */
      inline GfxFeatureMapImageRequest* 
         getGfxFeatureMapImageRequest( uint32 i ) const;

      /**
       * Get the rotation angle of a certain crossing map.
       * The angles are defined as positive clockwise and 0 degrees
       * corresponds to north.
       * 
       * @param   i  The index of the crossing map. 
       *             Valid range is [0,nbrStringItems[.
       * @return  The rotation angle of the specified crossing map.
       */
      inline uint16 getRotationForMap( uint32 i ) const;

   private:
      /**
       * The states of the request.
       */
      enum state {
         INITIAL,
         IMAGE_REQUESTS,
         EMAIL_REQUEST,
         UPDATE_ROUTE,
         DONE,
         ERROR
      } m_state;


      /**
       * Gets the default message.
       * @param contentType The type of message wanted.
       * @param messageStartFmt Set to new string with the default
       *                        startformat.
       * @param messageRestartFmt Set to new string with the default
       *                          restartformat.
       * @param messageRouteTurnFmt Set to new string with the
       *                            default routeturnformat.
       * @param messageRoutePreTurnLandmarkFmt Set to new string with the
       *                                       default prelandmarkformat.
       * @param messageRoutePostTurnLandmarkFmt Set to new string with the
       *                                        default postlandmarkformat.
       * @param messageEndFmt Set to new string with the default end 
       *                      format.
       * @param nbrMessageResources Set to the number of messageResources.
       * @param messageResourcesBuffLength Set to new vector with the
       *                                   lengths of messageResourcesBuff.
       * @param messageResourcesBuff Set to new vector with new buffers
       *                             with the messageResources.
       */
      void getDefaultMessage( RouteMessageRequestType::MessageContent
                              contentType,
                              char*& messageStartFmt,
                              char*& messageRestartFmt,
                              char*& messageStartTextFmt,
                              char*& messageRouteTurnFmt,
                              char*& messageRouteTurnTextFmt,
                              char*& messageRoutePreTurnLandmarkFmt,
                              char*& messageRoutePostTurnLandmarkFmt,
                              char*& messageEndFmt,
                              char*& messageEndTextFmt,
                              uint32& nbrMessageResources,
                              char**& messageResourcesURI,
                              uint32*& messageResourcesBuffLength,
                              byte**& messageResourcesBuff );


      /**
       * Makes the message and sets up all the image requests.
       *
       * @return The next state of the request.
       */
      state makeInitialSetup(  );

      
      /**
       * Makes a SendEmailRequestPacket with data from //current m_message.
       */
      state makeEmailRequest();

      
      /**
       * @param turn The stringcode for the turn.
       * @return URI for the turn.
       */
      static const char* getPictogramForTurn( 
         StringTable::stringCode turn );


      /**
       * Returns the size that the mimepart takes in a MimeMessage.
       */
      uint32 getMimePartSize( const MimePart* part ) const;


      /**
       * Get the image of a GfxFeatureMapImageRequest as a MimePart.
       * @param i The index of the GfxFeatureMapImageRequest.
       * @param uri The URI for the image. If NULL then m_imageURIs is 
       *            used. Default value is NULL.
       * @return A new MimePartImage. 
       */
      MimePartImage* getMapImageRequestImage( uint32 i, 
                                              const char* uri = NULL );


      /**
       * Ends the current message if the message will become too large and
       * starts a new message.
       */
      bool makeNewMessageIfMaxSize( 
         MC2String& message, uint32& messageSize, uint32& nbrMMSObjects,
         uint32 headerSize,
         const MC2String& fmtStr,
         bool useTurnTextFile, const MimePartText* textFilePart,
         bool makeImages, const MimePartImage* turnImagePart,
         bool reusedPictogram,
         vector< MC2String >& usedPictogramImages,
         vector< MimePartImage* >& usedPictogramImageParts,
         vector<MimePartText*>& textPartVector,
         vector< MimePartImage* >& usedMapImageParts,
         vector< MimePart* >& resourcesParts,
         uint32& resourcesSize,
         const MC2String& contEndFmtStr,
         vector< MimePart* >& contEndResourcesParts, uint32 contEndSize,
         uint32 contEndNbrMMSObjects,
         const MC2String& restartFmtStr,
         vector< MimePart* >& restartResourcesParts, 
         vector< MimePartText* >& restartTextParts,
         uint32 restartSize, uint32 restartNbrMMSObjects,
         MimePart::mainContentType mainContentType,
         MimePartText::contentType messageTextMarkupType,
         MimePartApplication::contentType messageApplicationMarkupType,
         MimePart::characterSet characterSet, 
         const char* pageName );


      /**
       * Add URI and MimePartImage if it isn't already added.
       *
       * @param uris The URIs of the already added images.
       * @param imageParts The vector of added images.
       * @param uri The uri to add.
       * @return The added MimePartImage or NULL if not added.
       */
      MimePartImage* addImage( vector< MC2String >& uris,
                               vector< MimePartImage* >& imageParts,
                               const char* uri ) const;


      /**
       * Looks for use of resources in str and add mimeparts for them in
       * the vector.
       * @param The document with the references to the resources.
       * @param resourcesParts The resources to check if used in str.
       * @param checkUnique If to check if resource already is added.
       */
      void getUsedResources( const MC2String& str, 
                             vector< MimePart* >& resourcesParts,
                             bool checkUnique = true );


      /**
       * The default message start format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageStartFmtFile[];


      /**
       * The default message restart format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageRestartFmtFile[];


      /**
       * The default message start text format file, used if START_FILE is
       * present in start format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageStartTextFmtFile[];


      /**
       * The default message route turn format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageRouteTurnFmtFile[];


      /**
       * The default message route turn text format file, used if 
       * ROUTE_TURN_FILE is present in route turn format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageRouteTurnTextFmtFile[];


      /**
       * The default message route turn pre landmark format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageRoutePreTurnLandmarkFmtFile[];


      /**
       * The default message route turn post landmark format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageRoutePostTurnLandmarkFmtFile[];
 

      /**
       * The default message end format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageEndFmtFile[];


      /**
       * The default message end text format file,  used if 
       * END_FILE is present in end format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageEndTextFmtFile[];


      /**
       * The default overview only message start format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageOverviewOnlyStartFmtFile[];


      /**
       * The default overview only message restart format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageOverviewOnlyRestartFmtFile[];


      /**
       * The default overview only message start text format file, 
       * used if START_FILE is present in start format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageOverviewOnlyStartTextFmtFile[];


      /**
       * The default overview only message route turn text format file, 
       * used if ROUTE_TURN_FILE is present in route turn format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageOverviewOnlyRouteTurnTextFmtFile[];


      /**
       * The default overview only message route turn format file, 
       * used if ROUTE_TURN_FILE is present in route turn format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageOverviewOnlyRouteTurnFmtFile[];


      /**
       * The default overview only message route turn pre landmark format
       * file.
       * One for each type of MessageContent. 
       */
      static const char* const 
         m_messageOverviewOnlyRoutePreTurnLandmarkFmtFile[];


      /**
       * The default overview only message route turn post landmark format
       * file.
       * One for each type of MessageContent. 
       */
      static const char* const 
         m_messageOverviewOnlyRoutePostTurnLandmarkFmtFile[];


      /**
       * The default overview only message end format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageOverviewOnlyEndFmtFile[];


      /**
       * The default overview only message end text format file,  used if 
       * END_FILE is present in end format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageOverviewOnlyEndTextFmtFile[];


      /**
       * The default message resources file with list of uris and files.
       * One for each type of MessageContent.
       */
      static const char* const m_messageResourcesFile[];


      /**
       * "%DIRECTIONS%" string.
       */
      static const char* directionsStr;

      
      /**
       * "%FROM%" string.
       */
      static const char* fromStr;


      /**
       * "%TO%" string.
       */
      static const char* toStr;

      
      /**
       * "%ORIGINSTRING%" string
       */
      static const char* originStr;


      /**
       * "%ORIGINLOCATIONSTRING%" string
       */
      static const char* originLocationStr;


      /**
       * "%DESTINATIONSTRING%" string
       */
      static const char* destinationStr;


      /**
       * "%DESTINATIONLOCATIONSTRING%" string
       */
      static const char* destinationLocationStr;


      /**
       * "%ROUTEOVERVIEW_URI%" string
       */
      static const char* routeOverviewURIStr;


      /**
       * "%ROUTEOVERVIEW_WIDTH%" string
       */
      static const char* routeOverviewWidthStr;


      /**
       * "%ROUTEOVERVIEW_HEIGHT%" string
       */
      static const char* routeOverviewHeightStr;


      /**
       * "%ROUTEOVERVIEW_ALT%" string
       */
      static const char* routeOverviewAltStr;


      /**
       * "%ROUTE_INFO%" string
       */
      static const char* routeInfoStr;


      /**
       * "%VEHICLE%" string
       */
      static const char* vehicleStr;


      /**
       * "%ROUTE_VEHICLE%" string
       */
      static const char* routeVehicleStr;


      /**
       * "%TOTAL_DISTANCE%" string
       */
      static const char* totalDistanceStr;


      /**
       * "%ROUTE_TOTAL_DISTANCE%" string
       */
      static const char* routeTotalDistanceStr;


      /**
       * "%TOTAL_TIME%" string
       */
      static const char* totalTimeStr;


      /**
       * "%ROUTE_TOTAL_TIME%" string
       */
      static const char* routeTotalTimeStr;


      /**
       * "%ROUTE_DESCRIPTION%" string
       */
      static const char* routeDescriptionStr;


      /**
       * "%ROUTE_TURN_URI%" string
       */
      static const char* routeTurnURIStr;


      /**
       * "%ROUTE_TURN_WIDTH%" string
       */
      static const char* routeTurnWidthStr;


      /**
       * "%ROUTE_TURN_HEIGHT%" string
       */
      static const char* routeTurnHeightStr;


      /**
       * "%ROUTE_TURN_ALT%" string
       */
      static const char* routeTurnAltStr;


      /**
       * "%ROUTE_PRE_LANDMARKS%" string
       */
      static const char* routeTurnPreLandmarkStr;


      /**
       * "%ROUTE_POST_LANDMARKS%" string
       */
      static const char* routeTurnPostLandmarkStr;


      /**
       * "%ROUTE_LANDMARK_DESCRIPTION%" string
       */
      static const char* routeTurnLandmarkDescriptionStr;

      
      /**
       * "%SIGNATURE%" string.
       */
      static const char* signatureStr;


      /**
       * "%START_FILE%" string.
       */
      static const char* startFileStr;


      /**
       * "%ROUTE_TURN_FILE%" string.
       */
      static const char* routeTurnFileStr;


      /**
       * "%END_FILE%" string.
       */
      static const char* endFileStr;


      /**
       * The MessageContents as strings.
       */
      static const char * const m_messageContentsStr[];


      /**
       * The TurnImageTypes as strings.
       */
      static const char * const m_turnImageTypesStr[];


      /**
       * The MimeMessage.
       */
      vector<MimeMessage*> m_message;


      /**
       * The number of image requests.
       */
      uint32 m_nbrImageRequests;

      
      /**
       * The current image request.
       */
      uint32 m_currentImageRequest;

      
      /**
       * The image requests. The overview image is last and any turn
       * images is before it.
       */
      GfxFeatureMapImageRequest** m_imageRequests;


      /**
       * The URI of the images. The overview image is last and any turn
       * images is before it.
       */
      char** m_imageURIs;


      /** 
       * The MapSettings to use.
       */
      MapSettings* m_mapSettings;

      
      /**
       * The answer packet if any.
       */
      PacketContainer* m_answer;

      
      /**
       * The format for the images.
       */
      ImageDrawConfig::imageFormat m_format;

      
      /**
       * The SendEmailRequestPackets if any.
       */
      list< PacketContainer* > m_emailRequest;


      /**
       * The SendEmailRequestPackets to send.
       */
      PacketContainer* m_sendEmailRequest;

      
      /**
       * If email should be sent.
       */
      bool m_sendEmail;

      
      /**
       * The toMailAddr.
       */
      const char* m_toMailAddr;

      
      /**
       * The fromMailAddr.
       */
      const char* m_fromMailAddr;

      
      /**
       * The subject.
       */ 
      const char* m_subject;

      
      /**
       * The signature.
       */
      const char* m_signature;
      

      /**
       * The MMS message.
       */
      vector<void*> m_mmsMessage;

      
      /**
       * The number of slides in the MMS message.
       */
      vector<int> m_nbrSlides;


      /**
       * If send MMS message.
       */
      bool m_sendMMS;


      /**
       * The maximun number of objects (MimeParts) in an MMS.
       */
      uint32 m_maxNbrMMSObjects;

      
      /**
       * The current number of MMS objects (MimeParts).
       */
      uint32 m_nbrMMSObjects;


      /**
       * The start format.
       */
      char* m_messageStartFmt;


      /**
       * The restart format.
       */
      char* m_messageRestartFmt;

      
      /**
       * The start text format.
       */
      char* m_messageStartTextFmt;

      
      /**
       * The route turn format.
       */
      char* m_messageRouteTurnFmt;


      /**
       * The route turn text format.
       */
      char* m_messageRouteTurnTextFmt;


      /**
       * The route pre turn landmark format.
       */
      char* m_messageRoutePreTurnLandmarkFmt;


      /**
       * The route post turn landmark format.
       */
      char* m_messageRoutePostTurnLandmarkFmt;


      /**
       * The end format.
       */
      char* m_messageEndFmt;


      /**
       * The end text format.
       */
      char* m_messageEndTextFmt;


      /**
       * The number of resources.
       */
      uint32 m_nbrMessageResources;


      /**
       * The URI for the resources.
       */
      char** m_messageResourcesURI;


      /**
       * The length of the resources buffers.
       */
      uint32* m_messageResourcesBuffLength;


      /**
       * The resources as byte buffers.
       */
      byte** m_messageResourcesBuff;


      /**
       * If the message format and resources should be deleted.
       */
      bool m_deleteMessageFmt;


      /**
       * The string describing the origin.
       */
      const char* m_originString;

      
      /**
       * String describing the origin's location.
       */
      const char* m_originLocationString;

      
      /**
       * String describing the destination.
       */
      const char* m_destinationString;


      /**
       * String describing the destination's location.
       */
      const char* m_destinationLocationString;


      
      /**
       * The ID of the route.
       */
      uint32 m_routeID;


      /**
       * The creation time of the route.
       */
      uint32 m_routeCreateTime;


      /**
       * The desired language of the message.
       */
      StringTable::languageCode m_language;

      
      /**
       * The RouteReplyPacket with the route.
       */
      const RouteReplyPacket* m_routeReplyPacket;


      /**
       * The ExpandRouteReplyPacket with the route.
       */
      ExpandRouteReplyPacket* m_expand;


      /**
       * The ExpandItemID with the RouteItems stored.
       */
      ExpandItemID* m_exp;


      /**
       * If the ExpandItemID should be deleted.
       */
      bool m_deleteExp;


      /**
       * The ExpandStringItems for the route.
       */
      ExpandStringItem** m_stringItems;


      /**
       * The number of ExpandStringItems.
       */
      uint16 m_nbrStringItems;

      
      /**
       * The rotation angles of the crossing maps.
       * The angles are defined as positive clockwise and 0 degrees
       * corresponds to north.
       * There will be m_nbrStringItems elements in the vector, 
       * one for each crossing map, in case all angles are not 0.
       * Use getRotationForMap() in order to retrieve the rotation
       * angle of a map.
       */
      vector<uint16> m_mapRotationAngles;


      /**
       * The width of the overview image.
       */
      uint16 m_routeOverviewWidth;


      /**
       * The height of the overview image.
       */
      uint16 m_routeOverviewHeight;

      
      /**
       * The width of the turn images.
       */
      uint16 m_routeTurnWidth;


      /**
       * The height of the turn images.
       */
      uint16 m_routeTurnHeight;


      /**
       * The type of vehicle at the start of the route.
       */
      ItemTypes::vehicle_t m_routeVehicle;


      /**
       * If the message should contain the images refered to in the route.
       */
      bool m_makeImages;


      /**
       * If a link to a webpage containing the html version
       * of the message.
       */
      bool m_makeLink;


      /**
       * If map image urls should be real ones that
       * can be sent to a [XML|HTTP]Server.
       */
      bool m_realMapURLs;

      /**
       * If using Content-Location if not then Content-ID.
       */
      bool m_contentLocation;

      /**
       * If landmarks should be included in route description.
       */
      bool m_includeLandmarks;


      /**
       * If use getNavTurnBoundingBox.
       */
      bool m_useNavTurnBoundingBox;


      /**
       * If return ExportGfxFeatureMap data and not image data.
       */
      bool m_exportMap;


      /**
       * If make only overview image route.
       */
      bool m_onlyOverview;


      /**
       * The type of message wanted.
       */
      RouteMessageRequestType::MessageContent m_contentType;


      /**
       * The type of route turn images to use.
       */
      UserConstants::RouteTurnImageType m_turnImageType;


      /**
       * The type of turn arrow to draw.
       */
      RouteArrowType::arrowType m_arrowType;


      /**
       * The maximum size of a message.
       */
      uint32 m_maxMessageSize;


      /**
       * The defaultMapSetting to use.
       */
      MapSettingsTypes::defaultMapSetting m_mapSetting;


      /**
       * The ImageSettings to use.
       */
      struct MapSettingsTypes::ImageSettings* m_imageSettings;

      bool m_navigatorCrossingMaps;

      bool m_deleteMapSettings;


      /**
       * The type of text files.
       */
      MimePartText::contentType m_textFileType;


      /**
       * The default charset.
       */
      MimePart::characterSet m_defaultCharacterSet;

      /// pointer to valid topregionrequest *with* data
      const TopRegionRequest* m_topReq;

   const CopyrightHandler* m_copyrights;

      /// The user doing the request, may be NULL.
      const UserItem* m_user;
};


// ========================================================================
//                                  Implementation of the inlined methods =


inline const MimeMessage* 
RouteMessageRequest::getMimeMessage( uint32 index ) const {
   if ( index < getNbrMimeMessages() ) {
      return m_message[ index ];
   } else {
      return NULL;
   }
}


inline uint32 
RouteMessageRequest::getNbrMimeMessages() const {
   return m_message.size();
}


inline uint32
RouteMessageRequest::getNbrGfxFeatureMapImageRequests() const
{
   return (m_nbrImageRequests);
}


inline GfxFeatureMapImageRequest*
RouteMessageRequest::getGfxFeatureMapImageRequest( uint32 i ) const
{
   if ( i < getNbrGfxFeatureMapImageRequests() ) {
      return (m_imageRequests[ i ]);
   } else {
      return (NULL);
   }
}
      

inline uint16 
RouteMessageRequest::getRotationForMap( uint32 i ) const
{
   uint16 retVal = 0;
   if ( i < m_mapRotationAngles.size() ) {
      retVal = m_mapRotationAngles[ i ];
   }

   return (retVal);
   
}

#endif // ROUTEMESSAGEREQUEST_H

