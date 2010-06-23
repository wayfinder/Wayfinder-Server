/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDITEM_H
#define OLDITEM_H

#include "config.h"

#include "ArrayTools.h"
#include "ItemTypes.h"
#include "NameUtility.h"
#include "GMSGfxData.h"

#include <stdio.h>
#include <stdlib.h>

// Foreward declarations
class OldItem;
class OldGenericMap;
class OldItemNames;
class DataBuffer;
class GfxData;
class GMSGfxData;
class CoordinateTransformer;

/**
  *   Contain all the data about an item on the map. An item is
  *   e.g. a piece of an street (like street_rec in OldMapCentral), 
  *   a forest, a lake.
  *
  */
class OldItem {
      
      /**
        *   Macro to get the zoomlevel of an item id.
        */
      #define GET_ZOOMLEVEL(id) ( ((id) >> 27) & 0x0f)

      #define GET_INDEXINZOOM(id) ( (id) & 0x07ffffff)

      #define CREATE_ITEMID(z, i) ( ((uint32(z) & 0x0f) << 27) |\
                                     (uint32(i) & 0x07ffffff))
   
   public:
      /**
        *   Write generic mif-header for all types of items to
        *   mif-file.
        *   @param nbrCol  Number of columns in mid-file.
        *   @param mifFile mif-file to write header to.
        *   @param coordsys The coordinate system that is used
        *                   in the mif file. Default mc2.
        *   @param coordOrderLatLon In which order are the coordinates
        *               expresses. Default = true = latlon. If the
        *               order is lonlat set false.
        */
      static void writeGenericMifHeader(uint32 nbrCol, ofstream& mifFile,
                        CoordinateTransformer::format_t coordsys 
                           = CoordinateTransformer::mc2,
                        bool coordOrderLatLon = true);
      
      /**
       *    Create a new item with the correct dynamic type from a
       *    databuffer.
       *    @param   dataBuffer  The buffer that contains data for
       *                         the item.
       *    @return  The new item.
       */
      static OldItem* createNewItem(DataBuffer* dataBuffer, OldGenericMap* theMap);

      /**
        *   Mask to get the item id from a nodeID
        *   usage: itemID = nodeID & ITEMID_MASK
        */
      #define ITEMID_MASK 0x7fffffff

      /**
        *   The size (in bytes) of the static allocated string used to print 
        *   the data about one item.
        */
      #define ITEM_AS_STRING_LENGTH 4096

      /**
       *    Default construcor, implemented to be as fast as possible.
       */
      inline OldItem() { MINFO("OldItem()"); };
      
      /**
        *   Create an item and fill it with data. 
        *   @param   type The type of item (streetSegmentItem, companyItem
        *                  etc).
        */
      OldItem(ItemTypes::itemType type, uint32 id = MAX_UINT32);

      /**
        *   Deletes the OldItem.
        */
      virtual ~OldItem();
      
      /**
        *   Write common parameters and gfx-data from all OldItems to a
        *   Mid/mif file.
        *   @param   midFile     
        *   @param   mifFile
        *   @param   namePointer Pointer to stringtables with all names
        *                        on items.
        */
      virtual void printMidMif(ofstream& midFile, ofstream& mifFile,
		               OldItemNames* namePointer);

      /**
       *    Virtual function for creating items from a mid and mif file.
       *    @param midFile
       *    @param readRestOfLine   If the rest of the mid line (eol) should 
       *                            be read or not, default true.
       */
      virtual bool createFromMidMif(ifstream& midFile,
                                    bool readRestOfLine = true);

      /**
       *    Virtual method that updates all attributes of item and,
       *    if any, its nodes and connections to the values of the 
       *    other item. This includes e.g. names, groups, entry 
       *    restrictions, sign posts, speed limit and gfxdata.
       *
       *    If the items originates from different maps, typically
       *    one underview and one overview or country overview map,
       *    the sameMap param should be set to false.
       *    In this case some attributes can not be updated by the
       *    item itself; names and sign post names (depending on 
       *    string index), connections (depending on underview- and 
       *    overview fromNodeIds). Update of these attributes is then 
       *    taken care of in the overview- and country overview maps 
       *    where id lookup tables and the map string tables are known.
       *    Note that group ids are updated here even if the item 
       *    originates from different maps; updating to e.g. overview 
       *    ids must then be handled separately.
       *
       *    @param otherItem  The item from which to get update values.
       *    @param sameMap    Default true meaning that the items are
       *                      one and the same (same map), set to false if
       *                      the items originates from different maps (e.g.
       *                      underview and overview map)
       *    @return  True if some attribute was updated for the item,
       *             false if not.
       */
      virtual bool updateAttributesFromItem(OldItem* otherItem,
                                            bool sameMap = true);

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
         inline uint32 getBestRawNameOfType(
            ItemTypes::name_t nameType,
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
      bool hasSameNames(const OldItem* item) const;


      /**
       *    Checks if both items have a name with the same string index,
       *    i.e. not paying respect to whether the name type or language
       *    are also the same for the common name.
       *
       *    @param   item  The item to check names against.
       *    $return  True if a common name is found.
       *
       */
      bool hasCommonName(const OldItem* item) const;

      /**
       *    Checks if both items have a name with the same string index
       *    and language, i.e. not paying respect to differences in name
       *    type.
       *
       *    @param   item  The item to check names against.
       *    $return  True if a common name is found.
       */
      bool hasCommonLangName(const OldItem* item) const;

      /**
       *    Checks if all names of item are present in this item, i.e. the
       *    names of item is a subset of the names in this item. Pays
       *    respect to language but not to name type
       *
       *    @param   item  The item to check names against.
       *    @return True if this item have all names of item.
       */
      bool hasAllLangNames(const OldItem* item) const;

      /**
       *    Check what string indexes that are used by both items and
       *    returns them in a vector.
       *    @param item The item to compare to.
       *    @return A vector with all string indexes present in both items.
       */
      vector<uint32> getCommonStrIdxes(const OldItem* item) const;
      //@}

      /**
       *    Get the identification number of this OldItem.
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
      inline GMSGfxData* getGfxData() const;

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
      void setGfxData(GfxDataFull* gfx, bool deleteOld = false);

      /**
        *   Get the length of the geographical representation of this
        *   OldItem. The length is returned in meters.
        *   @todo The returned length is of the _first_ polygon only!
        *   @return  The length (in meters) of this OldItem. MAX_UINT32 
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
      
      /**
        *   Adds a name to the item.
        *   @param   lang     The language of the string.
        *   @param   type     The type of the string.
        *   @param   nameID   The identification number of the name
        *   @return  The index of the name vector, negative value
        *            if unsuccessful.
        */
      int addName(LangTypes::language_t lang, ItemTypes::name_t type, 
                  uint32 nameID);

      /**
        *   @name Remove names
        */
      //@{
         /**
           *   Remove name with given offset.
           *   @param   i  Offset of the name to delete (valid values
           *               are between 0 and getNbrNames()).
           *   @return  True if the name is removed, false upon error.
           */
         bool removeNameWithOffset(uint32 i);

         /**
           *   Remove all names for this item.
           *   @return  True if the names are removed, false upon error.
           */
         bool removeAllNames();
      //@}

      /**
       *   Writes data about the item into a class variable
       *   and returns a pointer to it. Not thread-safe
       *   @return The item in string form
       */
      virtual char *toString(); 

      /**
        *   Saves the common parts of all items
        *   @param   dataBuffer Where to store the data.
        */
      virtual bool save(DataBuffer* dataBuffer) const ;


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
      inline uint32 getGroup(uint32 i) const;

      /**
       *    Get group number i, without masking the MSB.
       */
      inline uint32 getUnmaskedGroup(uint32 i) const;

      /**
        *   Replace an existing group id at a certain index.
        *   @param   i  Index of (existing) group to set. Valid values are
        *               0 <= i < getNbrGroups().
        *   @param   groupID  The new group id to set.
        *   @return  True if group number i is set to a new value, 
        *            false otherwise.
        */
      inline bool setGroup(uint32 i, uint32 groupID);
      
      /**
        *   Remove one of the groups that this item is member of.
        *   @param   i  Index of the group to remove. Valid values are
        *               0 <= i < getNbrGroups().
        *   @return  True if group number i is removed, false otherwise.
        */
      inline bool removeGroup(uint32 i);
            
      /**
        *   Remove one of the groups with a soceified ID that this item 
        *   is member of.
        *   @param   id The ID of the group to remove.
        *   @return  True if group with ID = id is removed, false otherwise.
        */
      inline bool removeGroupWithID(uint32 id);


      /**
       *    Remove all groups of this item.
       *
       *    @return  True if all removal went well, otherwise false.
       */
      bool removeAllGroups();


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
      inline const char* getItemTypeAsString( 
                                       StringTable::languageCode lc = 
                                          StringTable::ENGLISH ) const;
         
      /**
        *   Get the memory usage for this item.
        *   @return The number of bytes used by this OldItem.
        */
      virtual uint32 getMemoryUsage() const;



   /**
    * @return number of names
    */
   const uint32 *getNames() const { return m_names; }

   /**
    * @return Returns the goups of this item.
    */
   const uint32 *getGroups() const { return m_groups; }

   /**
    * @param theMap The map of this item (official codes are stored in the map)
    * @return Returns the official code of this item or MAX_UINT32 if no
    *         official code exists.
    */
   uint32 getOfficialCode(const OldGenericMap& theMap) const;

   /**
    * @param theMap The map of this item (official codes are stored in the map)
    * @param officialCode The official code to set. Usually from NAVTEQ 
    *                     attribute OC.
    */
   void setOfficialCode(OldGenericMap& theMap, uint32 officialCode);

   /**
     *   @name POI categories
     *
     *   The categories are stored in the mcm map, but the methods to get,
     *   add and remove categries for one POI is located here in 
     *   the OldItem class.
     */
   //@{
   /**
    * @param theMap The map of this item (categories are stored in the map)
    * @return Returns the categories of this item.
    */
   set<uint16> getCategories(const OldGenericMap& theMap) const;

   /**
    * @param theMap The map of this item (categories are stored in the map)
    * @param categoryID The category ID of the category to add.
    */
   void addCategory(OldGenericMap& theMap, uint16 categoryID);

   /**
    * @param theMap The map of this item (categories are stored in the map)
    * @param poiCategories The categories to add to this item.
    */
   void addCategories(OldGenericMap& theMap, 
                      const set<uint16>& poiCategories );

   /**
    * Remove all poi categories for this item.
    * @param theMap The map of this item (categories are stored in the map)
    */
   void removeCategories(OldGenericMap& theMap);
   //@}



  protected:
      /**
       *    Create this item from a databuffer.
       */
      virtual bool createFromDataBuffer(DataBuffer* dataBuffer, 
                                        OldGenericMap* theMap);

      /**
        *   Static buffer to hold stringdata about an object. Since
        *   this is allocated static, it is <B>not</B> threadsafe.
        */
      static char itemAsString[ITEM_AS_STRING_LENGTH];
      
      /**
       *    Initialize/reset members.
       *    @param type The type of this OldItem.
       */
      virtual void init(ItemTypes::itemType type);
      
      /**
        *   This function adds the item to a group.
        *
        *   NB! Don't use this method directly to set groups of an item,
        *       instead use OldGenericMap::addRegionToItem.
        *
        *   @param   groupID The group in which this item should be added.
        *   @return  true if the item was added to the group successfully,
        *            false otherwise.
        */
      bool addGroup(uint32 groupID);

      /**
        *   Array with names in form of indices in the stringTable 
        *   in the OldMap where this item is located. Each element also 
        *   contain information about the language and type of that 
        *   name. This array contains m_nbrNames items.
        */
      uint32* m_names;

      /**
        *   Array with groups in form of itemID to OldGroupItem. Contains
        *   m_nbrGroups elements.
        */
      uint32* m_groups;

      /**
        *   The local id of this item. 
        *   The first bit (bit 0) is used for identify node0 or node1
        *   (this means that the first bit is unused for every item
        *   except for the subclass "OldStreetSegmentItem"). Bit 1 to 4
        *   shows on what zoomlevel this item is significant.
        */
      uint32 m_localID;

      /**   
        *   The geographical representation of this item.
        */
      GfxDataFull* m_gfxData;

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

      friend class GMSItem;
      friend class OldGenericMap;
};


// ========================================================================
//                                       Implementation ofinlined methods =

inline LangTypes::language_t 
OldItem::getNameLanguage(byte offset) const
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
OldItem::getNameType(byte offset) const
{
   if ((offset < m_nbrNames) && (m_nbrNames > 0)) {
      return (GET_STRING_TYPE(m_names[offset]));
   } else {
      return (ItemTypes::invalidName);
   }
}

inline uint32
OldItem::getRawStringIndex(byte offset) const
{
   if (m_nbrNames == 0)
      return (0);
   else if (offset < m_nbrNames)
      return (m_names[offset]);
   else
      return (MAX_UINT32);
}

inline uint32 
OldItem::getStringIndex(byte offset) const
{
   uint32 res = getRawStringIndex(offset);
   if (res != MAX_UINT32) {
      res = GET_STRING_INDEX(res);
   }
   return res;
}

inline bool 
OldItem::getNameAndType(byte offset, LangTypes::language_t& strLang, 
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
OldItem::getNameWithType(ItemTypes::name_t strType, 
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
OldItem::getBestRawName(LangTypes::language_t reqLang,
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
OldItem::getBestRawName(LangTypes::language_t reqLang,
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
OldItem::getBestRawName(const vector<LangTypes::language_t>& reqLangs,
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
OldItem::getBestRawNameOfType(ItemTypes::name_t     nameType,
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
OldItem::getBestRawNameOfType(ItemTypes::name_t nameType,
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
OldItem::getNbrNames() const
{
   return byte(m_nbrNames);
}

inline byte 
OldItem::getNbrNamesWithType(ItemTypes::name_t type) const
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

inline uint32 OldItem::getID () const
{
   return m_localID;
}

inline void OldItem::setID(uint32 ID)
{
   m_localID = ID;
}

inline GMSGfxData* 
OldItem::getGfxData() const
{
   return static_cast<GMSGfxData*>( m_gfxData );
}



inline ItemTypes::itemType
OldItem::getItemType() const
{
   return (ItemTypes::itemType(m_type));
}

inline uint32
OldItem::getNbrGroups() const
{
   return m_nbrGroups;
}

inline uint32
OldItem::getGroup(uint32 i) const
{
   if (i < getNbrGroups()) {
      return m_groups[i] & 0x7fffffff;
   }
   return MAX_UINT32;
}

inline uint32 
OldItem::getUnmaskedGroup(uint32 i) const
{
   if (i < getNbrGroups()) {
      return m_groups[i];
   }
   return MAX_UINT32;
}

inline bool 
OldItem::memberOfGroup(uint32 groupID) const
{
   for (uint32 i=0; i<getNbrGroups(); ++i) {
      if (getGroup(i) == groupID)
         return true;
   }
   return false;
}

inline bool 
OldItem::setGroup(uint32 i, uint32 groupID)
{
   if ( i < getNbrGroups() ) {
      m_groups[ i ] = groupID;
      return true;
   } else {
      return false;
   }
}

inline bool 
OldItem::removeGroup(uint32 i)
{
   if ( i < getNbrGroups()){
      ArrayTool::removeElement(m_groups, i, m_nbrGroups);
      return true;
   }
   else{
      return false;
   }
}
      
inline bool 
OldItem::removeGroupWithID(uint32 id)
{
   id &= 0x7fffffff;
   uint32 i = 0;
   while ( (i < getNbrGroups()) && ( (m_groups[i] & 0x7fffffff) != id)) {
      ++i;
   }
   if (i < getNbrGroups()) {
      return removeGroup(i);
   }
   return false;
}

inline uint8 
OldItem::getSource() const 
{
   return m_source;
}

inline void 
OldItem::setSource(uint8 newSource) 
{
   m_source = newSource;
}

inline const char* 
OldItem::getItemTypeAsString( StringTable::languageCode lc ) const 
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
