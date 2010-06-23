/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEM_H
#define ITEM_H

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include "ItemTypes.h"
#include "NameUtility.h"

class GenericMap;
class ItemNames;
class DataBuffer;
class GfxData;
class RouteableItem;
class GroupItem;

// Cast functions which are needed when we remove the virtualness of the
// items.

class Item;

#define ITEMCAST(X, Y) class X; \
 inline X* item_cast_helper(Item* i, \
                            const X*) \
{ \
   return (X *) Item::itemOfType( i, ItemTypes::Y ); \
}

template<typename DEST>
const DEST* item_cast_helper_const( const Item* i, const DEST* d )
{
   return item_cast_helper( const_cast<Item*>(i), const_cast<DEST*>(d));
}

template<typename T>
T item_cast(const Item* i)
{
   return item_cast_helper_const( i, static_cast<T>(i) );
}

template<typename T>
T item_cast(Item* i)
{
   return item_cast_helper( i, static_cast<T>(i) );
}


/**
  *   Contain all the data about an item on the map. An item is
  *   e.g. a piece of an street (like street_rec in MapCentral), 
  *   a forest, a lake.
  *
  */
class Item {
      
   /**
    *   Macro to get the zoomlevel of an item id.
    */
#define GET_ZOOMLEVEL(id) ( ((id) >> 27) & 0x0f)

#define GET_INDEXINZOOM(id) ( (id) & 0x07ffffff)

#define CREATE_ITEMID(z, i) ( ((uint32(z) & 0x0f) << 27) |\
                                     (uint32(i) & 0x07ffffff))
   
public:
   friend class M3Creator;
   typedef uint32 group_t;
   typedef group_t* groupIndex_t;
   typedef uint32 name_t;
   typedef name_t* nameIndex_t;

   /**
    * Creates a new item from specified type.
    * @param type the item type
    * @param theMap The Generic map 
    * @param itemID Specific id for the new item
    * @return allocated item
    */
   static Item* createItemFromType( ItemTypes::itemType type,
                                    GenericMap& theMap, 
                                    uint32 itemID);
   /**
    *    Create a new item with the correct dynamic type from a
    *    databuffer.
    *    @param   dataBuffer  The buffer that contains data for
    *                         the item.
    *    @return  The new item.
    */
   static Item* createNewItem(DataBuffer& dataBuffer, GenericMap& theMap);
   /**
    *   Mask to get the item id from a nodeID
    *   usage: itemID = nodeID & ITEMID_MASK
    */
#define ITEMID_MASK 0x7fffffff

   /**
    *    Default construcor, implemented to be as fast as possible.
    */
   inline Item() { MINFO("Item()"); };
      
   /**
    *   Create an item and fill it with data. 
    *   @param   type The type of item (streetSegmentItem, companyItem
    *                  etc).
    */
   explicit Item(ItemTypes::itemType type, uint32 id = MAX_UINT32);

   /**
    *   Deletes the Item.
    */
   ~Item() {}

   /**
    *   Returns a group item if the item is one.
    *   Else NULL.
    */
   static GroupItem* groupItem( Item* maybeGroup );

   /**
    *   Returns a routeable item if the item is one.
    *   Else NULL.
    */
   static RouteableItem* routeableItem( Item* maybeRoutaeble );

   /**
    *   Returns the item if the item is of the wanted type.
    *   Else NULL.
    */
   static Item* itemOfType( Item* in_item,
                            ItemTypes::itemType wantedType );
   
   /** 
    *   @name Get names.
    *   Methods to get the names of this item.
    */
   //@{
   /**
    *   Get the language for name number offset.
    *   @param   offset   Which of the types to return.
    *   @return  The language for name number offset.
    */
   inline LangTypes::language_t getNameLanguage(byte offset) const;

   /**
    *   Get the type of name number offset.
    *   @param   offset   Which of the types to return.
    *   @return  The type of name number offset.
    */
   inline ItemTypes::name_t getNameType(byte offset) const;

   /**
    *   Get the offset:th name of this item.
    *   @param   offset   Which of the names of the item to return.
    *   @return  The name of this item, in terms of index in
    *            the stringTable. MAX_UINT32 is returned if no
    *            name with that index.
    */
   inline uint32 getStringIndex(byte offset) const;
         
   /**
    *   Get the offset:th name, type and language of this item.
    *   @param   offset   Which of the names of the item to return.
    *   @return  The name of this item, including type and
    *            language bits, in terms of index in
    *            the stringTable. MAX_UINT32 is returned if no
    *            name with that index.
    */
   inline uint32 getRawStringIndex(byte offset) const;

   /**
    *   Get the name and the type for one name.
    *   @param   offset   Which of the names of the item to return.
    *   @param   strLang  Outparameter that is set to the language
    *                     of name number offset.
    *   @param   strType  Outparameter that is set to the type
    *                     of name number offset.
    *   @param   strIndex Outparameter that is set to the stringindex
    *                     of name number offset.
    *   @return  True is returned if the outparameters are set 
    *            correctly, false otherwise.
    */
   inline bool getNameAndType(byte offset, 
                              LangTypes::language_t& strLang, 
                              ItemTypes::name_t& strType, 
                              uint32& strIndex) const;

   /**
    *   Get a name with a specified type and/or language.
    *
    *   @param   strType  The type of the string. If
    *                     "invalidName" is send as parameter
    *                     the type of the returned string is 
    *                     undefined.
    *   @param   strLang  The language of the string. If
    *                     "invalidLanguage" is send as parameter
    *                     the language of the returned string is 
    *                     undefined.
    *   @return  Index of the FIRST string with specified type and 
    *            language. MAX_UINT32 is returned if no such string.
    */
   inline uint32 getNameWithType(ItemTypes::name_t strType, 
                                 LangTypes::language_t strLang) const;

   /**
    *    Uses NameUtility::getBestName to get the best name
    *    in the item.
    *    @param  reqLang Requested language.
    *    @return The stringindex of the best name or MAX_UINT32
    *            if there are no names in the item.
    */
   inline uint32 getBestRawName( LangTypes::language_t reqLang,
                                 bool acceptUniqueName = true ) const;

   /**
    *    Uses NameUtility::getName to get the best name
    *    in the item.
    *    @param  reqLang Requested language.
    *    @param  reqType Requested type.
    *    @return The stringindex of the best name or MAX_UINT32
    *            if there are no names in the item.
    */
   inline uint32 getBestRawName( LangTypes::language_t reqLang,
                                 ItemTypes::name_t reqType,
                                 set<uint32>* usedStrings = NULL) const;

   /**
    *    Uses NameUtility::getBestName to get the best name
    *    in the item.
    *    @param  reqLangs Requested languages.
    *    @return The stringindex of the best name or MAX_UINT32
    *            if there are no names in the item.
    */
   inline uint32 getBestRawName(
                                const vector<LangTypes::language_t>& reqLangs,
                                bool acceptUniqueName = true ) const;

   /**
    *    Uses NameUtility::getBestNameOfType to get the best name
    *    in the item.
    *    @param  nameType The requested name type.
    *    @param  reqLang  Requested language.
    *    @return The stringindex of the best name or MAX_UINT32
    *            if there are no names of the requested type
    *            in the item.
    */
   inline uint32
   getBestRawNameOfType( ItemTypes::name_t    nameType,
                         LangTypes::language_t reqLang,
                         set<uint32>* usedStrings = NULL) const;

   /**
    *    Uses NameUtility::getBestNameOfType to get the best name
    *    in the item.
    *    @param  nameType Requested type.
    *    @param  reqLangs Requested languages.
    *    @return The stringindex of the best name or MAX_UINT32
    *            if there are no names of the requested type
    *            in the item.
    */
   inline uint32 getBestRawNameOfType( ItemTypes::name_t nameType,
                       const vector<LangTypes::language_t>& reqLangs ) const;
         
   /**
    *   Get the number of names for this item.
    *   @return The number of names (stringIndeces) for this item
    */
   inline byte getNbrNames() const;

   /**
    *   Get the number of names for this item with the specified
    *   type.
    *
    *   @param   types The type for the names that should be 
    *                  included in the returned count.
    *   @return  The number of names for this item with the specified
    *            type.
    */
   inline byte getNbrNamesWithType(ItemTypes::name_t types) const;

   //@}
     
   /**
    *    @name Change name type and language
    *    Methods to change the type and the language of one of the 
    *    names.
    */
   //@{
   /**
    *    Set the type of the given name to a new value. The
    *    language and string index for that name will no be
    *    affected.
    *    @param offset  The offset of the name to change type for.
    *    @param type    The new type of the name.
    *    @return  True if the type is changed, false otherwise.
    */
   bool setNameType(byte offset, ItemTypes::name_t type);

   /**
    *    Set the language of the given name to a new value. The
    *    type and string index for that name will no be affected.
    *    @param offset  The offset of the name to change type for.
    *    @param lang    The new language of the name.
    *    @return  True if the type is changed, false otherwise.
    */
   bool setNameLanguage(byte offset, LangTypes::language_t lang);
   //@}
     

   /**
    * @name Name comparing methods. Checks similarity of names 
    *       between items.
    */
   //@{

   /**
    *    Checks if this item has exact the same names as the supplied
    *    item. Note that only duplicates of a name differing between
    *    the items still makes the method return true.
    *    
    *    @param   item  The item to check names against.
    *    @return  True if this item has exact the same names as the 
    *             supplied item, however, duplicates of a name in only
    *             one of the items does not indicate a difference in 
    *             names.
    */
   bool hasSameNames(const Item* item) const;


   /**
    *    Checks if both items have a name with the same string index,
    *    i.e. not paying respect to whether the name type or language
    *    are also the same for the common name.
    *
    *    @param   item  The item to check names against.
    *    $return  True if a common name is found.
    *
    */
   bool hasCommonName(const Item* item) const;

   /**
    *    Checks if both items have a name with the same string index
    *    and language, i.e. not paying respect to differences in name
    *    type.
    *
    *    @param   item  The item to check names against.
    *    $return  True if a common name is found.
    */
   bool hasCommonLangName(const Item* item) const;

   /**
    *    Checks if all names of item are present in this item, i.e. the
    *    names of item is a subset of the names in this item. Pays
    *    respect to language but not to name type
    *
    *    @param   item  The item to check names against.
    *    @return True if this item have all names of item.
    */
   bool hasAllLangNames(const Item* item) const;

   /**
    *    Check what string indexes that are used by both items and
    *    returns them in a vector.
    *    @param item The item to compare to.
    *    @return A vector with all string indexes present in both items.
    */
   vector<uint32> getCommonStrIdxes(const Item* item) const;
   //@}

   /**
    *    Get the identification number of this Item.
    *    @return  ID of this item.
    */
   inline uint32 getID () const ;

   /**
    *     Set the ID of this item.
    *     @param ID The ID of the item.
    */
   inline void setID(uint32 ID);
      
   /**
    *   Get the geographical representation of this item.
    *   @return  The geographical representation of this item.
    *            This might return NULL. 
    */
   inline GfxData* getGfxData() const;
   inline const nameIndex_t getNames() const;
   inline const groupIndex_t getGroups() const;

   /**
    *   Set the geographical representation to a new value. <B>NB!</B>
    *   The given gfxData is not copied, and must therefore not be 
    *   deleted by the caller.
    *   @warning The old GfxData will <B>not</B> be deleted since
    *            it might be allocated by an allocator in the map. Sa
    *            if the old GfxData is allocated elsewhere this might
    *            introduce a memory leak.
    *
    *   @param   gfx   The new gfxData that have the georaphical
    *                  information about this item.
    */
   void setGfxData(GfxData* gfx, bool deleteOld = false);

   /**
    *   Get the length of the geographical representation of this
    *   Item. The length is returned in meters.
    *   @todo The returned length is of the _first_ polygon only!
    *   @return  The length (in meters) of this Item. MAX_UINT32 
    *            is returned if an error occurred.
    */
   uint32 getLength() const;

   /**
    *   Get the type of this item. The type is stored on disc in 8 
    *   bits and as a byte in the memory (the member that holds the
    *   type) to make sure that not more than 8 bits are occupied.
    *
    *   @return  The type of this item.
    */
   inline ItemTypes::itemType getItemType() const;
      


   inline bool memberOfGroup(uint32 groupID) const;

   /**
    *   Get the number of groups that this item is member of.
    *   @return  The number of groups this item is member of.
    */
   inline uint32 getNbrGroups() const;

   /**
    *   Get the ID of one of the groups for this item.
    *   @return  The id of the i:th groups this item is member of,
    *            MAX_UINT32 if i > nbrGroups.
    */
   inline group_t getGroup( uint32 i ) const;


   inline uint32 getGroupIndex() const;

   /**
    *    Get group number i, without masking the MSB.
    */
   inline uint32 getUnmaskedGroup( uint32 i ) const;


   /**
    *   Sets the group index
    *   @param   index index into group
    */
   inline void setGroup( groupIndex_t index ) { m_groups = index; }
   /**
    * Sets the names index
    * @param index into group
    */
   inline void setNames( nameIndex_t index ) { m_names = index; }

   /**
    *    Get the source bitmask, with information about the origin of 
    *    the information described by this item.
    *    @return The source bitmask.
    */
   inline uint8 getSource() const;

   /**
    *  Sets a new source of the information of this item.
    *  @param newSource The new source bitmask.
    */
   inline void setSource(uint8 newSource);

   /**
    *   Get the itemtype of this item as a string.
    *
    *   @param   lc The desired language of the text. Default english.
    *   @return  The itemtype of this item as a string.
    */
   inline const char* getItemTypeAsString( StringTable::languageCode lc = 
                                           StringTable::ENGLISH ) const;
         
   /**
    *   Get the memory usage for this item.
    *   @return The number of bytes used by this Item.
    */
   uint32 getMemoryUsage() const;


   /**
    *   Saves the common parts of all items
    *   @param   dataBuffer Where to store the data.
    */
   void save( DataBuffer& dataBuffer, const GenericMap& map ) const;

   /**
    *    Create this item from a databuffer.
    */
   void load( DataBuffer& dataBuffer, GenericMap& theMap );

   void setItemType( ItemTypes::itemType type ) { m_type = type; }

protected:
   /**
    *    Initialize/reset members.
    *    @param type The type of this Item.
    */
   void init(ItemTypes::itemType type);
      
   /**
    *   Array with names in form of indices in the stringTable 
    *   in the Map where this item is located. Each element also 
    *   contain information about the language and type of that 
    *   name. This array contains m_nbrNames items.
    */
   nameIndex_t m_names;

   /**
    *   Array with groups in form of itemID to GroupItem. Contains
    *   m_nbrGroups elements.
    */
   groupIndex_t m_groups;

   /**
    *   The local id of this item. 
    *   The first bit (bit 0) is used for identify node0 or node1
    *   (this means that the first bit is unused for every item
    *   except for the subclass "StreetSegmentItem"). Bit 1 to 4
    *   shows on what zoomlevel this item is significant.
    */
   uint32 m_localID;

   /**   
    *   The geographical representation of this item.
    */
   GfxData* m_gfxData;

   /**
    *    The number of elements in m_names.
    */
   byte m_nbrNames;

   /**
    *    The number of elements in m_groups.
    */
   uint32 m_nbrGroups;

   /**   
    *   Type of object, defined in the ItemTypes-class. Stored as
    *   an byte to make sure that no more than 8 bits are used.
    *   @see ItemTypes.h
    */
   byte m_type;

   /**
    *    The source of the object, see SearchTypes::DBsource.
    */
   byte m_source;

};


// ========================================================================
//                                       Implementation ofinlined methods =

inline LangTypes::language_t 
Item::getNameLanguage(byte offset) const
{
   if ((offset < m_nbrNames) && (m_nbrNames > 0)) {
      if(getNameType(offset) == ItemTypes::roadNumber)
         return (LangTypes::invalidLanguage);
      return (GET_STRING_LANGUAGE(m_names[offset]));
   } else {
      return (LangTypes::invalidLanguage);
   }
}

inline ItemTypes::name_t 
Item::getNameType(byte offset) const
{
   if ((offset < m_nbrNames) && (m_nbrNames > 0)) {
      return (GET_STRING_TYPE(m_names[offset]));
   } else {
      return (ItemTypes::invalidName);
   }
}

inline uint32
Item::getRawStringIndex(byte offset) const
{
   if (m_nbrNames == 0)
      return (0);
   else if (offset < m_nbrNames)
      return (m_names[offset]);
   else
      return (MAX_UINT32);
}

inline uint32 
Item::getStringIndex(byte offset) const
{
   uint32 res = getRawStringIndex(offset);
   if (res != MAX_UINT32) {
      res = GET_STRING_INDEX(res);
   }
   return res;
}

inline bool 
Item::getNameAndType(byte offset, LangTypes::language_t& strLang, 
                     ItemTypes::name_t& strType, uint32& strIndex) const
{
   if (m_nbrNames == 0) {
      strLang = LangTypes::invalidLanguage;
      strType = ItemTypes::invalidName;
      strIndex = 0;
      return true;
   } else if (offset < m_nbrNames) {
      strType = GET_STRING_TYPE(m_names[offset]);
      if(strType == ItemTypes::roadNumber)
         strLang = LangTypes::invalidLanguage;
      else
         strLang = GET_STRING_LANGUAGE(m_names[offset]);
      strIndex = GET_STRING_INDEX(m_names[offset]);
      return true;
   } else {
      return false;
   }
}

inline uint32 
Item::getNameWithType(ItemTypes::name_t strType, 
                      LangTypes::language_t strLang) const
{
   uint32 i=0;
   uint32 retValue = MAX_UINT32;
   bool found = false;
   while ((i<getNbrNames()) && (!found)) {
      ItemTypes::name_t curType;
      LangTypes::language_t curLang;
      uint32 curIndex;

      if (getNameAndType(i, curLang, curType, curIndex)) {
         
         if ( ((strType == curType) || 
              (strType == ItemTypes::invalidName)) &&
              ((strLang == curLang) || 
              (strLang == LangTypes::invalidLanguage)) ) {
            found = true;
            retValue = curIndex;
         } else {
            i++;
         }
      } else {
         found = true;
         retValue = MAX_UINT32;
      }

   }

   return (retValue);
}

inline uint32
Item::getBestRawName(LangTypes::language_t reqLang,
                     bool acceptUniqueName) const
{
   int idx = NameUtility::getBestName(m_nbrNames,
                                      m_names,
                                      reqLang,
                                      NULL,
                                      acceptUniqueName);
   if ( idx < 0 ) {
      return MAX_UINT32;
   } else {
      return m_names[idx];
   }
}

inline uint32
Item::getBestRawName(LangTypes::language_t reqLang,
                     ItemTypes::name_t reqType,
                     set<uint32>* usedStrings) const
{
   int idx = NameUtility::getName(m_nbrNames,
                                  m_names,
                                  reqLang,
                                  reqType,
                                  usedStrings);
   if ( idx < 0 ) {
      return MAX_UINT32;
   } else {
      return m_names[idx];
   }
}

inline uint32
Item::getBestRawName(const vector<LangTypes::language_t>& reqLangs,
                     bool acceptUniqueName) const
{
   int idx = NameUtility::getBestName(m_nbrNames,
                                      m_names,
                                      reqLangs,
                                      NULL,
                                      acceptUniqueName);
   if ( idx < 0 ) {
      return MAX_UINT32;
   } else {
      return m_names[idx];
   }
}


inline uint32
Item::getBestRawNameOfType(ItemTypes::name_t     nameType,
                           LangTypes::language_t reqLang ,
                           set<uint32>* usedStrings) const
{
   int idx = NameUtility::getBestNameOfType(m_nbrNames,
                                            m_names,
                                            reqLang,
                                            nameType,
                                            usedStrings);
   if ( idx < 0 ) {
      return MAX_UINT32;
   } else {
      return m_names[idx];
   } 
}

inline uint32
Item::getBestRawNameOfType(ItemTypes::name_t nameType,
                           const vector<LangTypes::language_t>& reqLangs) const
{
   int idx = NameUtility::getBestNameOfType(m_nbrNames,
                                            m_names,
                                            reqLangs,
                                            nameType);
   if ( idx < 0 ) {
      return MAX_UINT32;
   } else {
      return m_names[idx];
   }
}

inline byte 
Item::getNbrNames() const
{
   return byte(m_nbrNames);
}

inline byte 
Item::getNbrNamesWithType(ItemTypes::name_t type) const
{
   // The value to return.
   byte retVal = 0;

   // Loop over all the names and find the names that match the given types
   for (uint32 i=0; i<getNbrNames(); i++) {
      ItemTypes::name_t strType = GET_STRING_TYPE(m_names[i]);
      if ( strType == type) {
         retVal++;
      }
   }

   // Return the number of found companies
   return (retVal);
}

inline uint32 Item::getID () const
{
   return m_localID;
}

inline void Item::setID(uint32 ID)
{
   m_localID = ID;
}

inline GfxData* 
Item::getGfxData() const
{
   return m_gfxData;
}

inline const Item::nameIndex_t
Item::getNames() const
{
   return m_names;
}

inline const Item::groupIndex_t
Item::getGroups() const
{
   return m_groups;
}

inline ItemTypes::itemType
Item::getItemType() const
{
   return (ItemTypes::itemType(m_type));
}

inline uint32 
Item::getNbrGroups() const
{
   return m_nbrGroups;
}

inline Item::group_t
Item::getGroup(uint32 i) const
{
   if (i < getNbrGroups()) {
      return m_groups[i] & 0x7fffffff;
   }
   return MAX_UINT32;
}

inline uint32 
Item::getUnmaskedGroup(uint32 i) const
{
   if (i < getNbrGroups()) {
      return m_groups[i];
   }
   return MAX_UINT32;
}

inline bool 
Item::memberOfGroup(uint32 groupID) const
{
   for (uint32 i=0; i<getNbrGroups(); ++i) {
      if (getGroup(i) == groupID)
         return true;
   }
   return false;
}

inline uint8 
Item::getSource() const 
{
   return m_source;
}

inline void 
Item::setSource(uint8 newSource) 
{
   m_source = newSource;
}

inline const char* 
Item::getItemTypeAsString( StringTable::languageCode lc ) const 
{
   const char* str = StringTable::getString( 
         ItemTypes::getItemTypeSC( getItemType() ), lc );
   if (str != NULL) {
      return (str);
   } else {
      return ("Unknown itemtype");
   }
      
}


#endif
