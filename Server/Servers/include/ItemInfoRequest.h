/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMINFOREQUEST_H
#define ITEMINFOREQUEST_H

#include "config.h"
#include "Request.h"
#include "PacketContainerTree.h"
#include "CoordinateObject.h"
#include "ItemInfoEnums.h"
#include "ItemTypes.h"


class ItemInfoReplyPacket;
class PacketContainer;
class SearchMatch;
class SearchReplyData;
class TopRegionRequest;
class VanillaMatch;

/** 
  *   A request that returns some information about one item that is 
  *   known either by coorindate or ID.
  */
class ItemInfoRequest : public Request {
   public:
      /**
       *    Create a ItemInfoRequest for (mapID, itemID)
       *    @param   reqID unique request ID.
       *    @param topReq       Pointer to valid TopRegionRequest with data
       */
      ItemInfoRequest(const RequestData& reqData,
                      const TopRegionRequest* topReq);

      /**
       *    Returns a vector of matches containing coordinates,
       *    name, item info and synonym name.
       */
      const vector<VanillaMatch*>& getMatches() const;

      /**
       *   Sets a search match as original item. Can be created from
       *   an itemID string.
       *   @return false if nothing could be used in the match.
       */
      bool setItem( const SearchMatch& searchMatch,
                    LangTypes::language_t lang );

      // -- Deprecated from this point --
      
      /**
       *    @name Set expand item.
       *    @memo Set the item to expand.
       *    @doc  Set the item to expand.
       */
      //@{
         /**
          *    
          */
         void setItem(uint32 mapID, uint32 itemID, 
                      LangTypes::language_t lang,
                      ItemTypes::itemType itemType);

         /**
          * 
          *@param poiName Optional parameter that limits the result to 
          *               pois with name matching poiName. Empty string
          *               matches any name.
          */
         void setItem(int32 lat, int32 lon,
                      ItemTypes::itemType itemType,
                      LangTypes::language_t lang,
                      const char* poiName = "");

         
         
      //@}

      /**
       *    Delete memory that is allocated here.
       */
      virtual ~ItemInfoRequest();

      /** 
       *    Return the next packet that should be sent to the modules. This
       *    might be a CoordinateRequestPacket or an ItemInfoRequestPacket.
       *    @return The next packet that should be sent to the modules.
       */
      PacketContainer* getNextPacket();

      /** 
       *    Handle a reply from the modules. Will either result in that the
       *    request is ready or more packets to send to the modules.
       *    @param   pack  The packet that is returned from the modules.
       */
      void processPacket(PacketContainer* pack);

      /**
       *    Get the answer to this request. {\it {\bf NB!} This will always
       *    return NULL!} Use other methods to get the result!
       *    @return  The answer to the request (NULL!!).
       */
      PacketContainer* getAnswer();

      /**
       *    If request was successfull and reply data is set.
       *    @return True if request's reply data is set.
       */
      bool replyDataOk() const;


      bool virtual requestDone();
      
      /**
       *    Get the number of items with information.
       *    @return The number of items with information that are available.
       */
      uint32 getNbrReplyItems() const;

      /**
       *    @name Get reply.
       *    @memo Methods to get the information about the items.
       *    @doc  Methods to get the information about the items. All 
       *          these methods returns pointers into the packet, this 
       *          means that the pointer will be invalid after this
       *          request is deleted!
       */
      //@{
         /**
          *    Get the number of fields for item number i. Valid values
          *    are 0 <= i < getNbrReplyItems().
          *    @param  i   The index of the item to get the number of
          *                fields for.
          *    @return The number of fields for item number i.
          */
         uint32 getReplyNbrFields(uint32 i) const;

         uint32 getReplySubType(uint32 i) const;
         int32 getReplyLatitude(uint32 i) const;
         int32 getReplyLongitude(uint32 i) const;
      
         /**
          *    Get a pointer to the type of item number i. Valid values
          *    are 0 <= i < getNbrReplyItems().
          *    @param  i   The index of the item to get the type for.
          *    @return A pointer to a char* that contains the name of
          *            item type number i.
          */
         const char* getReplyType(uint32 i) const;
         const char* getReplyName(uint32 i) const;

         /**
          *    Get a pointer to one of the fields for item number itemNbr. 
          *    Valid values are {\tt 0 <= itemNbr < getNbrReplyItems()} 
          *    and {\tt 0 <= fieldNbr < getReplyNbrFields(itemNbr)}.
          *    @param  itemNbr   The index of the item to get field name 
          *                      for.
          *    @param  fieldNbr  The index of the field to get the name 
          *                      for.
          *    @return A pointer to a char* that contains the name of
          *            field number fieldNbr for item nbr itemNbr.
          */
         const char* getReplyItemFieldname(uint32 itemNbr, 
                                           uint32 fieldNbr) const;

         /**
          *    Get a pointer to the value of one of the fields for item 
          *    number itemNbr. Valid values are 
          *    {\tt 0 <= itemNbr < getNbrReplyItems()} and 
          *    {\tt 0 <= fieldNbr < getReplyNbrFields(itemNbr)}.
          *    @param  itemNbr   The index of the item to get value for.
          *    @param  fieldNbr  The index of the field to get the value
          *                      for.
          *    @return A pointer to a char* that contains the value of
          *            field number fieldNbr for item nbr itemNbr.
          */
         const char* getReplyItemValue(uint32 itemNbr, uint32 fieldNbr) const;

         /**
          * Get the type of information for a field.
          * 
          * @param itemNbr The index of the item.
          * @param fieldNbr The index of the tuple for the item.
          * @return The type of information that the tuple is.
          */
         ItemInfoEnums::InfoType getReplyItemType( uint32 itemNbr, 
                                                   uint32 fieldNbr ) const;
      //@}

   LangTypes::language_t getLanguage() const { 
      return m_language;
   }

private:
      /**
       *    The possible states of this object.
       */
      enum state_t { 
         /// Waiting for indata
         AWAITING_INDATA,
         
         /// Sending coordinate request to the map module.
         SENDING_COORDINATE_REQUEST,

         /** 
          *    Waiting for the coordinate reply packet that contains 
          *    information about the item that is close to the given 
          *    coordinates.
          */
         AWAITING_COORDINATE_REPLY,

         ///   Sending item info request to the map module.
         SENDING_ITEMINFO,

         ///   Waiting for the item info reply packet.
         AWAITING_ITEMINFO,

         /// This request is done.
         DONE,

         /// Something has gone wrong.
         ERROR
      };

      ItemInfoRequest::state_t handleItemInfoReply(ItemInfoReplyPacket* p);
      state_t handleExternalSearchReply( const Packet* p );
      ItemInfoRequest::state_t handleCoordinateReply(PacketContainer* pc);

      PacketContainer* createItemInfoRequestPacket(
               uint32 mapID, 
               uint32 itemID, 
               LangTypes::language_t lang,
               ItemTypes::itemType itemType = ItemTypes::streetSegmentItem,
               int32 lat = MAX_INT32,
               int32 lon = MAX_INT32,
               const char* poiName = "");


      /**
       *    Containers with the packets that are ready to be processed
       *    by the modules.
       */
      PacketContainerTree m_packetsReadyToSend;

      /**
       *    The coordinate-request handler that is used for creating the
       *    coordinate request and reply packets.
       */
      CoordinateObject m_coordinateObject;

      /**
       *    The current state of this object.
       */
      state_t m_state;

      /**
       *   Saved packets that are unsafe to delete until
       *   the request is deleted.
       */
      vector<PacketContainer*> m_savedPackets;

      /**
       *   VanillaMatches are stored here.
       */
      SearchReplyData* m_searchReplyData;
      
      /**
       *
       */
      LangTypes::language_t m_language;

      struct {
         ItemTypes::itemType itemType;
         int32 lat;
         int32 lon;
         const char* poiName;
      } m_coordinateData;

      /// Clone of original search match if any.
      VanillaMatch* m_origMatch;
};

#endif




