/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LOCALMAPMESSAGEREQUEST_H
#define LOCALMAPMESSAGEREQUEST_H

#include "config.h"
#include "StringTable.h"
#include "Request.h"
#include "PacketContainer.h"
#include "MimeMessage.h"
#include "ImageDrawConfig.h"
#include "MC2String.h"
#include "RouteMessageRequestType.h"
#include "MapSettingsTypes.h"
#include "NotCopyable.h"

class TopRegionRequest;
class MapSettings;
class GfxFeatureMap;
class GfxFeatureMapImageRequest;
class UserItem;
class ClientSetting;

/** 
 * A Request that makes a message over a local aream that optinally can 
 * be sent as an email.
 *
 */
class LocalMapMessageRequest : public Request, private NotCopyable {
   public:
      /**
       * Create a LocalMapMessageRequest.
       *
       * @param reqID unique request ID.
       * @param northLat The northern latitude of the local map.
       * @param westLon The western longitude of the local map.
       * @param southLat The southern latitude of the local map.
       * @param eastLon The eastern longitude of the local map.
       * @param lang The desired language of the message.
       * @param localMapWidth The width of the local map image.
       * @param localMapHeight The height of the local map image.
       * @param localMapString String describing the contents of the 
       *                       local Map.
       * @param topReq Pointer to valid TopRegionRequest with data
       * @param clientSetting The client settings to use for the map,
       *                      or NULL.
       * @param markItems A GfxFeatureMap with GfxFeatureSymbols to
       *                  add to the map.
       * @param makeImages If the message should contain the images
       *                   refered to in the message.
       * @param makeLink If a link to a webpage containing the html version
       *                 of the message.
       * @param realMapURLS If map image urls should be real ones that
       *                    can be sent to a [XML|HTTP]Server.
       * @param contentType The type of message wanted.
       * @param maxMessageSize NB Not yet supported!
       *                       The maximum size of a message, default 
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
       * @param messageFmt Formart string where:
       *                   "%LOCAL_MAP%", "%LOCAL_MAP_URI%", 
       *                   "%LOCAL_MAP_WIDTH%", "%LOCAL_MAP_HEIGHT%",
       *                   "%LOCAL_MAP_ALT%", "%LOCAL_MAP_NAME%",
       *                   "%SIGNATURE%"
       *                   are replaced by actual strings
       *                   (use without citations).
       * @param nbrMessageResources The number of resources that are
       *                            static in the message.
       * @param messageResourcesURI The URI of the messageresources.
       * @param messageResourcesBuffLength The length of the resources
       *        in messageResourcesBuff.
       * @param messageResourcesBuff The raw bytes the the 
       *                             messageResources.
       */
      LocalMapMessageRequest( 
         const RequestData& data,
         int32 northLat, int32 westLon,
         int32 southLat, int32 eastLon,
         StringTable::languageCode lang,
         uint16 localMapWidth,
         uint16 localMapHeight,
         const char* localMapString,
         const TopRegionRequest* topReq,
         const ClientSetting* clientSetting,
         GfxFeatureMap* markItems = NULL,
         bool makeImages = true,
         bool makeLink = false,
         bool realMapURLs = true,
         RouteMessageRequestType::MessageContent contentType = 
         RouteMessageRequestType::HTML_CONTENT,
         uint32 maxMessageSize = MAX_UINT32,
         bool sendEmail = false,
         const char* toMailAddr = NULL,
         const char* fromMailAddr = NULL,
         const char* subject = NULL,
         const char* signature = NULL,
         ImageDrawConfig::imageFormat defaultFormat = ImageDrawConfig::PNG,
         const MC2String& copyright = "",
         const UserItem* user = NULL,
         bool contentLocation = false,
         MapSettingsTypes::defaultMapSetting mapSetting = 
         MapSettingsTypes::MAP_SETTING_STD,
         struct MapSettingsTypes::ImageSettings* 
         imageSettings = NULL,
         const char* messageFmt = NULL,
         const char* messageStartTextFmt = NULL,
         const char* messageEndTextFmt = NULL,
         uint32 nbrMessageResources = MAX_UINT32,
         char** messageResourcesURI = NULL,
         uint32* messageResourcesBuffLength = NULL,
         byte** messageResourcesBuff = NULL );


      /**
       * Destructor deletes all resources.
       */
      virtual ~LocalMapMessageRequest();

      
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
       */
      inline const MimeMessage* getMimeMessage() const;

      
   private:
      /**
       * The states of the request.
       */
      enum state {
         INITIAL,
         IMAGE_REQUESTS,
         EMAIL_REQUEST,
         DONE,
         ERROR
      } m_state;


      /**
       * Gets the default message.
       * @param messageFmt Set to new string with the default format.
       * @param nbrMessageResources Set to the number of messageResources.
       * @param messageResourcesBuffLength Set to new vector with the
       *                                   lengths of messageResourcesBuff.
       * @param messageResourcesBuff Set to new vector with new buffers
       *                             with the messageResources.
       */
      void getDefaultMessage( char*& messageFmt,
                              char*& messageStartTextFmt,
                              char*& messageEndTextFmt,
                              uint32& nbrMessageResources,
                              char**& messageResourcesURI,
                              uint32*& messageResourcesBuffLength,
                              byte**& messageResourcesBuff );


      /**
       * Makes the message and sets up all the image requests.
       *
       * @param northLat The northern latitude of the local map.
       * @param westLon The western longitude of the local map.
       * @param southLat The southern latitude of the local map.
       * @param eastLon The eastern longitude of the local map.
       * @param lang The desired language of the message.
       * @param localMapWidth The width of the local map image.
       * @param localMapHeight The height of the local map image.
       * @param localMapString String describing the contents of the 
       *                       local Map.
       * @param markItems A GfxFeatureMap with GfxFeatureSymbols to
       *                  add to the map.
       * @param makeImages If the message should contain the images
       *                   refered to in the message.
       * @param makeLink If a link to a webpage containing the html version
       *                 of the message.
       * @param realMapURLS If map image urls should be real ones that
       *                    can be sent to a [XML|HTTP]Server.
       * @param contentType The type of message wanted.
       * @param maxMessageSize NB Not yet supported!
       *                       The maximum size of a message.
       * @param signature The signuture, text last, of the email.
       * @param defaultFormat The default format of the images. 
       * @param mapSetting The defaultMapSetting to use.
       * @param imageSettings The ImageSettings to use.
       * @param messageFmt Formart string for the message.
       * @param nbrMessageResources The number of resources that are
       *                            static in the message.
       * @param messageResourcesURI The URI of the messageresources.
       * @param messageResourcesBuffLength The length of the resources
       *        in messageResourcesBuff.
       * @param messageResourcesBuff The raw bytes the the 
       *                             messageResources.
       * @param topReq Pointer to valid TopRegionRequest with data
       */
      state makeInitialSetup( int32 northLat, int32 westLon,
                              int32 southLat, int32 eastLon,
                              StringTable::languageCode lang,
                              uint16 localMapWidth,
                              uint16 localMapHeight,
                              const char* localMapString,
                              GfxFeatureMap* markItems,
                              bool makeImages,
                              bool makeLink,
                              bool realMapURLs,
                              RouteMessageRequestType::MessageContent 
                              contentType,
                              uint32 maxMessageSize,
                              const char* signature,
                              ImageDrawConfig::imageFormat defaultFormat,
                              MapSettingsTypes::defaultMapSetting
                              mapSetting,
                              struct MapSettingsTypes::ImageSettings* 
                              imageSettings,
                              const char* messageFmt,
                              const char* messageStartTextFmt,
                              const char* messageEndTextFmt,
                              uint32 nbrMessageResources,
                              char** messageResourcesURI,
                              uint32* messageResourcesBuffLength,
                              byte** messageResourcesBuff, 
                              const TopRegionRequest* topReq);

      
      /**
       * Makes a SendEmailRequestPacket with data from current m_message.
       */
      state makeEmailRequest();


      /**
       * The default message format file.
       * One for each type of MessageContent.
       */
      static const char* m_messageFmtFile[];
      

      /**
       * The default message text format file, used if START_FILE is
       * present in format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageStartTextFmtFile[];


      /**
       * The default message text format file, used if END_FILE is
       * present in format file.
       * One for each type of MessageContent.
       */
      static const char* const m_messageEndTextFmtFile[];

      
      /**
       * The default message resources file with list of uri file.
       * One for each type of MessageContent.
       */
      static const char* m_messageResourcesFile[];


      /**
       * "%LOCAL_MAP%" string.
       */
      static const char* localMapStr;

      
      /**
       * "%LOCAL_MAP_URI%" string.
       */
      static const char* localMapURIStr;


      /**
       * "%LOCAL_MAP_WIDTH%" string.
       */
      static const char* localMapWidthStr;

      
      /**
       * "%LOCAL_MAP_HEIGHT%" string
       */
      static const char* localMapHeightStr;


      /**
       * "%LOCAL_MAP_ALT%" string
       */
      static const char* localMapAltStr;


      /**
       * "%LOCAL_MAP_NAME%" string
       */
      static const char* localMapNameStr;

      
      /**
       * "%SIGNATURE%" string.
       */
      static const char* signatureStr;


      /**
       * "%START_FILE%" string.
       */
      static const char* startFileStr;
      

      /**
       * "%END_FILE%" string.
       */
      static const char* endFileStr;


      /**
       * The MimeMessage.
       */
      MimeMessage m_message;

      
      /**
       * The number of image requests.
       */
      uint32 m_nbrImageRequests;

      
      /**
       * The current image request.
       */
      uint32 m_currentImageRequest;

      
      /**
       * The image requests.
       */
      GfxFeatureMapImageRequest** m_imageRequests;


      /**
       * The URI of the images.
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
       * The SendEmailRequestPacket if any.
       */
      PacketContainer* m_emailRequest;

      
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
       * The type of message wanted.
       */
      RouteMessageRequestType::MessageContent m_contentType;


      /**
       * If the message should contain the images refered to in the route.
       */
      bool m_makeImages;

      /**
       * If using Content-Location if not then Content-ID.
       */
      bool m_contentLocation;

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
   /// copyright for images
   MC2String m_copyright;

      /// The user doing the request, may be NULL.
      const UserItem* m_user;

   /// The client settings which should be used for the map.
   const ClientSetting* m_clientSetting;
};


// ========================================================================
//                                  Implementation of the inlined methods =


inline const MimeMessage* 
LocalMapMessageRequest::getMimeMessage() const {
   return &m_message;
}



#endif // LOCALMAPMESSAGEREQUEST_H

