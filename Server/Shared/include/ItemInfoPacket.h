/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMINFOPACKET_H
#define ITEMINFOPACKET_H

#include "config.h"
#include "Packet.h"
#include "ItemTypes.h"
#include "ItemInfoEnums.h"
#include "ItemInfoEntry.h"

// Forwards
class UserRightsMapInfo;
class UserUser;
class IDPair_t;
class VanillaMatch;

/**
 *    Packet used to request additional information about one item with
 *    given ID or some items that are located on the same street. The
 *    information is returned in the ItemInfoReplyPacket in the format
 *    specified in this request packet.
 *    
 *    After the general ReplyPacket-header the format is 
 *    (X = REPLY_HEADER_SIZE):
 *    @packetdesc
 *       @row X    @sep 4 bytes @sep The ID of the item to get 
 *                                   information about.           @endrow
 *       @row X+4  @sep 4 bytes @sep Latitude for the item.       @endrow
 *       @row X+8  @sep 4 bytes @sep Longitude for the item.      @endrow
 *       @row X+12 @sep 4 bytes @sep The preferred language of the reply.
 *                                                                @endrow
 *       @row X+16 @sep 4 bytes @sep The item type to get information about.
 *                                                                @endrow
 *       @row X+20 @sep 4 bytes @sep The format that should be used in the 
 *                                   reply packet.                @endrow
 *       @row X+24 @sep Y bytes @sep The name of the poi to get on the 
 *                                   street.                      @endrow
 *       @row X+Y  @sep Z bytes @sep The User rights per map info @endrow
 *    @endpacketdesc
 *
 *    The following table explains what will be returned for different
 *    combinations of ID:s and coordinates.
 *    <table>
 *       <tr><td><b>Indata</b></td><td><b>Requested type</b></td>
 *           <td><b>Result</b></td></tr>
 *       <tr><td>X (mapID+itemID)</td><td>X</td>
 *           <td>Information about X.</td></tr>
 *       <tr><td>Street/SSI (mapID+itemID)</td><td>POI</td>
 *           <td>Information about all POI:s on the street or street 
 *               segment.</td></tr>
 *       <tr><td>Street/SSI (mapID+itemID+(lat,lon))</td><td>POI</td>
 *           <td>Information about the closest POI:s. More than one POI 
 *               will be returned if located on equal distance from 
 *               (lat,lon).</td></tr>
 *       <tr><td>POI (mapID+itemID)</td><td>Street/SSI</td>
 *           <td>Information about the street where the POI is located.
 *               </td></tr>
 *    </table>
 *       
 */
class ItemInfoRequestPacket : public RequestPacket {
public:

      /**
       *    @name Format of result.
       *    The possible formats of the result.
       */
      enum outformat_t {
         /// The reply of the request will be unformatted text.
         text = 0
      };
      
      /**
       *    Create a request packet with the item to get information about.
       *    To be able to get information about a Point Of Interest that
       *    only is known by its coordinates it is possible to add these
       *    coordinates. If any coordinates are added the itemID must 
       *    belong to the Street Segment Item where the POI is located.
       *
       *    @note See the table in the class documentation for usage of
       *          ID:s, coordinates and itemType.
       *
       *    @param id      The ID of the item.
       *    @param user    The current user.
       *    @param poiLat  Optional parameter that is the latitude part of
       *                   the coordinate of the POI.
       *    @param poiLon  Optional parameter that is the longitude part of
       *                   the coordinate of the POI.
       *    @param poiName Optional parameter that limits the result to 
       *                   pois with name matching poiName. Empty string
       *                   matches any name.
       */
      ItemInfoRequestPacket(const IDPair_t& id,
                            const UserUser* user,
                            int32 poiLat = MAX_INT32, 
                            int32 poiLon=MAX_INT32,
                            const char* poiName = "");

      /**
        *   Get the ID of the item in this packet.
        *   @return The ID of the requested item. 
        */
      uint32 getItemID() const;

      /**
       *    @name Preferred language.
       *    Get and set the preferred language in the reply.
       */
      //@{
         /**
          *    Set the language that is preferred in the reply (if this is
          *    available).
          *    @param lang The preferred language.
          */
         void setPreferredLanguage(LangTypes::language_t lang);

         /**
          *    Get the language that is preferred in the reply.
          *    @return The preferred language in the reply.
          */
         LangTypes::language_t getPreferredLanguage() const;
      //@}

      /**
       *    @name Item type.
       *    Get and set the type of item to expand.
       */
      //@{
         /**
          *    Set the type of the item that information will be returned 
          *    for.
          *
          *    @note See the table in the class documentation for usage of
          *          ID:s, coordinates and itemType.
          *
          *    @param   t  The type of the item to get information about.
          */
         void setItemType(ItemTypes::itemType t);

         /**
          *    Get the type of the item that information will be returned 
          *    for.
          *    @return The type of the item to get information about.
          */
         ItemTypes::itemType getItemType() const;
      //@}

      /**
       *    @name Out data format.
       *    Get and set the requested format in the reply packet.
       */
      //@{
         /**
          *    Set the format that will be used in the reply packet.
          *    @param t The requested format in the reply packet.
          */
         void setOutdataFormat(outformat_t t);

         /**
          *    Get the format that will be used in the reply packet.
          *    @return  The requested format in the reply packet.
          */
         outformat_t getOutdataFormat() const;
      //@}

      /**
       *    @name POI-Coordinate.
       *    Get and set the coordinate for the POI.
       */
      //@{
         /**
          *    Set POI coordinates. If information about a POI is requested 
          *    it is necessary to first lookup the street segment and after
          *    that the coordinate of the point of interests.
          *
          *    @note See the table in the class documentation for usage of
          *          ID:s, coordinates and itemType.
          *
          *    @param lat  The latitude part of the coordinate for the POI.
          *    @param lon  The longitude part of the coordinate for the POI.
          */
         void setPOICoordinate(int32 lat, int32 lon);
         
         /**
          *    Get the latitude part of the coordinate for the POI. If this
          *    is != MAX_INT32, the coordinate information will not be used.
          *    @return  The latitude part of the coordinate for the POI.
          */
         int32 getPOILat() const;

         /**
          *    Get the longitude part of the coordinate for the POI.
          *    @return  The longitude part of the coordinate for the POI.
          */
         int32 getPOILon() const;
      //@}

      /**
       * Get the poi name. Rights were added later.
       */
      const char* getPOINameAndRights( UserRightsMapInfo& rights ) const;
  private:
      /// Well, calculates the packet size.
      static uint32 calcPackSize( const IDPair_t& id,
                                  const UserUser* user,
                                  int32 poiLat, int32 poiLon,
                                  const char* poiName );
};


/**
 * Packet to get additional information about several items in the same
 * mapID at once.
 * The reply is an ItemInfoReplyPacket with as many items as in this request.
 */
class GetItemInfoRequestPacket : public RequestPacket {
public:
   /**
    * 
    * @param mapID The mapID for all the items.
    * @param language The prefered language.
    * @param user The user making the request.
    * @param items The items to get information for.
    * @param infoTypeFilter Filter for item infos
    */
   GetItemInfoRequestPacket( uint32 mapID,
                             LangTypes::language_t language,
                             ItemInfoEnums::InfoTypeFilter infoTypeFilter,
                             const UserUser* user,
                             const vector< VanillaMatch* >& items );

   /**
    * Get the language.
    */
   LangTypes::language_t getLanguage() const;

   /**
    * Returns the item info filter for set in the request package.
    * @return The filter enum
    */
   ItemInfoEnums::InfoTypeFilter getItemInfoFilter() const;

   /**
    * Get the UserRightsMapInfo.
    */
   void getUserRightsMapInfo( UserRightsMapInfo& rights ) const;

   /// A vector of item ids.
   typedef vector< uint32 > itemIDVector;

   /**
    * Get the itemIDs.
    */
   void getItemIDs( itemIDVector& itemIDs ) const;

private:
   /**
    * The number of item IDs.
    */
   uint32 getNbrItems() const;

   /**
    * The positions of the things with static locations in the packet.
    */
   enum positions {
      language_POS             = REQUEST_HEADER_SIZE,
      infotypefilter_POS       = language_POS + 4,
      nbr_items_POS            = infotypefilter_POS + 4,
      map_Info_size_POS        = nbr_items_POS + 4,
      items_start_POS          = map_Info_size_POS + 4
   };
};


/**
 * Class for holding ItemInfo data.
 */
class ItemInfoData {
public:
   /**
    * Creates an empty ItemInfoData.
    */
   ItemInfoData();

   /// The category ids in a vector.
   typedef vector< uint16 > Categories;

   /// The container for ItemInfoEntries.
   typedef vector< ItemInfoEntry > ItemInfoEntryCont;

   /**
    * Constructor that sets all values.
    */
   ItemInfoData( const MC2String& type,
                 const MC2String& itemName,
                 ItemTypes::itemType itemType,
                 uint32 subType,
                 const MC2Coordinate& coord,
                 const Categories& categories = Categories(),
                 const ItemInfoEntryCont& fields = ItemInfoEntryCont() );

   /**
    * Returns the size of this ItemInfoEntry in a Packet.
    */
   uint32 getSizeInPacket() const;

   /**
    * Saves the info in a packet.
    */
   int save( Packet* p, int& pos ) const;

   /**
    * Loads the info from a packet.
    */
   int load( const Packet* p, int& pos );

   /**
    * Add an information entry to the item.
    *
    * @param entry The information to add.
    */
   void addInfoEntry( const ItemInfoEntry& entry );

   /**
    * Add an information entry to the item.
    *
    * @param key      The key.
    * @param val      The value.
    * @param infotype The type of info that the key represents.
    */
   void addInfoEntry( const MC2String& key,
                      const MC2String& val,
                      ItemInfoEnums::InfoType infoType = ItemInfoEnums::text );

   /**
    * Get the type name.
    */
   const MC2String& getType() const;

   /**
    * Get the item name.
    */
   const MC2String& getItemName() const;

   /**
    * Get the item type.
    */
   ItemTypes::itemType getItemType() const;

   /**
    * Get the item sub type, poi type.
    */
   uint32 getSubType() const;

   /**
    * Get the coordinate.
    */
   const MC2Coordinate& getCoord() const;

   /**
    * Get reference to the Categories.
    */
   const Categories& getCategories() const;

   /**
    * Get reference to the Item Infos.
    */
   const ItemInfoEntryCont& getFields() const;

   /**
    * Set the item sub type, poi type.
    */
   void setSubType( uint32 type );

   /**
    * Set the Categories.
    */
   void setCategories( const Categories& categories );

   /**
    * Set the Item Infos.
    */
   void setFields( const ItemInfoEntryCont& fields );

   /**
    * Set the Item name.
    */
   void setItemName( const MC2String& name );

   /**
    * Set more info available
    */
   void setMoreInfoAvailable( bool moreInfo );

   /**
    * Get more info available
    */
   bool getMoreInfoAvailable() const;

private:
   /// The type name.
   MC2String m_type;

   /// The item name.
   MC2String m_itemName;

   /// The item type.
   ItemTypes::itemType m_itemType;

   /// The item sub type, poi type.
   uint32 m_subType;

   /// The coordinate.
   MC2Coordinate m_coord;

   /// The Categories.
   Categories m_categories;

   /// The Item Infos.
   ItemInfoEntryCont m_fields;

   /// Indicates if more information is available
   bool m_moreInfo;
};

/// A container of ItemInfoData
typedef vector< ItemInfoData > ItemInfoDataCont;


/**
 * Packet that is sent as a reply to the Item Info Request packet.
 * Contains pairs with the type of information and the actual 
 * information, e.g. ("Address", "Baravägen 1"). Also contain the
 * name etc. of the item. All strings will be written in the language
 * that is specified in the request packet, if this language is 
 * available.
 */
class ItemInfoReplyPacket : public ReplyPacket {
public:
   /**
    * Create an empty item info reply packet.
    */
   ItemInfoReplyPacket();

   /**
    * Create an item info reply packet as a reply to the given
    * request packet.
    *
    * @param req The Item Info Request Packet that this should be
    *            a reply to.
    */
   ItemInfoReplyPacket( const ItemInfoRequestPacket* req );

   /**
    * Create an item info reply packet as a reply to the given
    * request packet.
    *
    * @param req The Item Info Request Packet that this should be
    *            a reply to.
    */
   ItemInfoReplyPacket( const GetItemInfoRequestPacket* req );      

   /**
    * Returns the info as a vector of search matches.
    *
    * @param matches New matches are created for the item info in this
    *                packet.
    */
   void getAsMatches( vector<VanillaMatch*>& matches ) const;

   /**
    * Set the information.
    *
    * @param info The information to fill this packet with.
    */
   void setInfo( const ItemInfoDataCont& info );

   /**
    * Get the information.
    *
    * @param info The information is added to this.
    */
   void getInfo( ItemInfoDataCont& info ) const;

   /**
    * Get the information and put it into matches.
    * It is required that the number of matches is the same as the
    * number of ItemInfoData.
    *
    * @param matches The macthes to add information to.
    * @return True if information added ok, false if not.
    */
   bool getInfo( vector<VanillaMatch*>& matches ) const;
};


#endif

