/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVMESSHELP_H
#define NAVMESSHELP_H


#include "config.h"
#include "InterfaceParserThreadConfig.h"
#include "NavHandler.h"
#include "RouteID.h"
#include "ImageDrawConfig.h"
#include "RouteMessageRequestType.h"


class UserUser;
class UserItem;
class SearchMatch;


/**
 * Class with help functions for message handling.
 *
 */
class NavMessHelp : public NavHandler {
   public:
      /**
       * The types of messages.
       */
      enum MessageType {
         invalid        = 0,
         HTML_email     = 1,
         non_HTML_email = 2,
         SMS            = 3,
         MMS            = 4,
         FAX            = 6,
         InstantMessage = 5,
      };


      /**
       * The type of object data.
       */
      enum ObjectType {
         invalidObjectType = 0,
         Destination    = 1,
         SearchItem     = 2,
         FullSearchItem = 3,
         Route          = 4,
         Itinerary      = 5,
         Map            = 6,
         Position       = 7,
      };


      /**
       * Get the MessageType as string.
       */
      static const char* getMessageTypeAsString( MessageType type );


      /**
       * Get the ObjectType as string.
       */
      static const char* getObjectTypeAsString( ObjectType type );


      /**
       * Constructor.
       */
      NavMessHelp( InterfaceParserThread* thread,
                   NavParserThreadGroup* group );


      /**
       * Handles a route/itinerary part of a send message request.
       *
       * @param routeID The id of the route.
       * @param messType The type of message to send.
       * @param language The language to use.
       * @param sendMessage If to send message or return content in data.
       * @param imageFormat The type of images whanted.
       * @param messageType The type of message whanted.
       * @param overviewImageWidth Overview image width.
       * @param overviewImageHeight Overview image height.
       * @param routeTurnImageWidth Route turn image width.
       * @param routeTurnImageHeight Route turn image height.
       * @param userItem. The user to use when making maps.
       * @param sender The sender of the message.
       * @param signature The signature of the message.
       * @param receiver The receiver of the message.
       * @param data Value parameter set to result.
       * @param dataLen Value parameter set to len of data.
       * @return The status, is OK, NOT_OK or TIMEOUT.
       */
      uint8 handleRouteNavSend( RouteID routeID,
                                MessageType messType,
                                StringTable::languageCode language,
                                bool sendMessage,
                                ImageDrawConfig::imageFormat imageFormat,
                                RouteMessageRequestType::MessageContent 
                                messageType,
                                uint32 maxMessageSize,
                                uint32 overviewImageWidth,
                                uint32 overviewImageHeight,
                                uint32 routeTurnImageWidth,
                                uint32 routeTurnImageHeight,
                                const UserItem* userItem,
                                const char* sender,
                                const char* signature,
                                const char* receiver,
                                bool contentLocation,
                                byte*& data, uint32& dataLen );


      /**
       * Handles a Destination/SearchItem/FullSearchItem part of a
       * send message request.
       *
       * @param objectType The type of message to send.
       * @param favoriteID The id of the favorite to send.
       * @param position The coordinate to send.
       * @param positionName The name of the position.
       * @param messType The type of message to send.
       * @param language The language to use.
       * @param sendMessage If to send message or return content in data.
       * @param imageFormat The type of images whanted.
       * @param messageType The type of message whanted.
       * @param overviewImageWidth Overview image width.
       * @param overviewImageHeight Overview image height.
       * @param user The user to get favorites for
       * @param userItem. The user to use when making maps.
       * @param sender The sender of the message.
       * @param signature The signature of the message.
       * @param receiver The receiver of the message.
       * @param contentLocation If client uses Content-Location or Content-ID.
       * @param data Value parameter set to result.
       * @param dataLen Value parameter set to len of data.
       * @return The status, is OK, NOT_OK or TIMEOUT.
       */
      uint8 handleItemNavSend( ObjectType objectType,
                               uint32 favoriteID,
                               MC2Coordinate position,
                               const char* positionName,
                               const SearchMatch* searchMatch,
                               MessageType messType,
                               StringTable::languageCode language,
                               bool sendMessage,
                               ImageDrawConfig::imageFormat imageFormat,
                               RouteMessageRequestType::MessageContent 
                                messageType,
                               uint32 maxMessageSize,
                               uint32 overviewImageWidth,
                               uint32 overviewImageHeight,
                               const UserUser* user,
                               const UserItem* userItem,
                               const char* sender,
                               const char* signature,
                               const char* receiver,
                               bool contentLocation,
                               byte*& data, uint32& dataLen );


      /**
       * Prints the position into a string.
       *
       * @param coord The coordinate.
       * @param str The string to print to.
       * @param language The language to use.
       * @return The string with the position nicely printed.
       */
      void makePositionString( MC2Coordinate coord, char* str,
                               StringTable::languageCode language ) const;
};


#endif // NAVMESSHELP_H

