/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OLDGENERICMAPHEADER_H
#define OLDGENERICMAPHEADER_H

#include "config.h"

#include "Vector.h"
#include "MC2String.h"
#include "LangTypes.h"
#include "MapGenUtil.h"

#define CLOCK_MAPLOAD(a) a
//#define CLOCK_MAPLOAD(a) 

class DataBuffer;

/**
 *   Class representing the header part of the mcm-maps.
 *   <br />
 *   FIXME: Remove unnecessary inlined functions.
 */
class OldGenericMapHeader : public VectorElement {
public:
   /**
    *   Creates empty header with given map id
    *   @param id The map id to set.
    */
   OldGenericMapHeader(uint32 mapID = MAX_UINT32);

   /**
    *   Create a header with mapID and path. The header is @b not filled
    *   with data from the file. The load-method must be called after
    *   creation!
    *
    *   @param   mapID Id of the map to be loaded.
    *   @param   path The file-path to the map.
    */
   OldGenericMapHeader(uint32 mapID, const char *path);


   /**
    *    @param   mcmName  The name of the map on the filesystem that 
    *                      contains the map.
    *    @return  A new map, created with data from the file at the
    *             given path. NULL is returned upon error.
    */
   static OldGenericMapHeader* createMap(const char* mcmName);


   /**
    *   Deletes the OldGenericMapHeader.
    */
   virtual ~OldGenericMapHeader();

   /**
    *   Loads a map with id, name and path specified by the 
    *   attributes. The format of the file is described in the
    *   "grammars.tex"-file (.mcm).
    */
   virtual bool load();

   /**
    *   Save the file to disk in .mcm-format.
    *   Opens the file and the calls the internalSave-method.
    *   @return  True if the map is saved alright, false otherwise.
    */
   bool save();

   /**
    *   Get the ID of this map.
    *   @return  The 32b map id of this map.
    */
   inline uint32 getMapID() const;

   /**
    *   @name Operators
    *   These operators are used when sorting the maps and searching 
    *   among them. The actual sorting- and searchalgorithms are 
    *   implemented in ObjVector.
    */
   //@{
   /// equal
   inline bool operator == (const VectorElement& elm) const;
   
   /// not equal
   inline bool operator != (const VectorElement& elm) const;

   /// greater
   inline bool operator > (const VectorElement& elm) const;

   /// less
   inline bool operator < (const VectorElement& elm) const;
   //@}

   /**
    *   Get the name of this map.
    *   @return  The name of this map. This is currently equal to
    *            the name of the municipals in the map.
    */
   inline const char* getMapName() const;

   /**
    *   Set the map ID of this map to a new value. This method
    *   is private because wrong usage is leads to very serious
    *   errors.
    */
   void setMapID(uint32 newMapID);
   
   /**
    *   Set the name of this map.
    *   @param name The name of this map.
    */
   void setMapName( const char* name );

   /**
    *    Get the string with copyright information for this map.
    *    @return A pointer to a string with copyright information
    *            about this map.
    */
   inline const char* getCopyrightString() const;
   
   /**
    *    Set the string with copyright information for this map.
    *    @param  A pointer to a string with copyright information
    *            for this map.
    */
   void setCopyrightString(const char* copyright);

   /**
    *   Get the origin of this map.
    *   @return  The origin of this map.
    */
   inline const char* getMapOrigin() const;
   
   /// Get the map data supplier of this map.
   inline const MapGenEnums::mapSupplier getMapSupplier() const;

   /// Get the map data version of this map.
   inline const MapGenEnums::mapVersion  getMapVersion() const;
   

   /**
    *   Set the origin of this map.
    *   @param name The origin of this map.
    */
   void setMapOrigin( const char* origin );
   
   /**
    *   Get the Country code for the country where this map is
    *   located. To get a printable name, use the method
    *   StringTable::getCountryStringCode.
    *   @return  StringCode for the name of the country for this 
    *            map.
    */
   inline StringTable::countryCode getCountryCode() const;
   
   /**
    *   Set the countrycode for this map.
    *   @param   countryName CountryCode for the country of this map.
    */
   void setCountryCode(StringTable::countryCode countryName);

   /** 
    *   Driving on the right side of the road?
    *   @return  True if you should drive on the right side of the
    *            road, false otherwise.
    */
   inline bool driveOnRightSide() const;
   
   /** 
    *   Set the driving side.
    *   @param   driveRight  True if you should drive on the right 
    *                        side of the road, false otherwise.
    */
   void setDrivingSide(bool driveRight);
   
   /** 
    *   Get the number of native languages in the country for 
    *   this map.
    *   @see Item.h The supported languages.
    *   @return  The number of native languages for this map.
    */
   inline uint32 getNbrNativeLanguages() const;
   
   /**
    *   Get one of the native languages.
    *   @param   i  Index of the native language to return. Valid
    *               values are 0 <= i < getNbrNativeLanguages().
    *   @return  Laguage code for native language number i on
    *            this map.
    */
   inline LangTypes::language_t getNativeLanguage(uint32 i) const;
   
   /**
    *   Add a native language for this map.
    *   @param   lang  The native langugae to add to this map.
    */
   void addNativeLanguage(LangTypes::language_t lang);
   
   /**
    *   Reset the native languages.
    */
   void clearNativeLanguages();
   
   /**
    *   Get the number of currencies in this country.
    */
   inline uint32 getNbrCurrencies() const;
   
   /**
    *   Get one of the currencies in this country.
    *   @param   i  Index of the currency to return. Valid
    *               values are 0 <= i < getNbrCurrencies().
    *   @return  String code for currency  number i on this map.
    */
   inline StringTable::stringCode getCurrency(uint32 i) const;
   
   /**
    *   Add a currency to this map.
    *   @param   cur   The currency to add to this map.
    */
   void addCurrency(StringTable::stringCode cur );
   
   /**
    *   Reset the currencies.
    */
   void clearCurrencies();
   
   /**
    *    Get a pointer to the filename from where this GenericMap is
    *    created. Please notice that no-one is allowed to change or
    *    delete the returned string!
    *    @return  The name of the file from where this map is created.
    */
   inline const char* getFilename() const;

   /**
    *    @name Get and set different time-stamps.
    *    Methods for managing the different times that are stored in
    *    the map header.
    */
   //@{
   /**
    *    Get the creation time (last-saved-time) for this map.
    *    @return  The time when this map was created (saved)
    *             (in UNIX-time).
    */
   inline uint32 getCreationTime() const;

   /**
    *    Get the true creation time for this map, which is the time
    *    when it was saved for the first time.
    */
   inline uint32 getTrueCreationTime() const;
      
   /**
    *    Update the true creation time for this map.
    *    NB! Don't do this! The m_trueCreationTime is set in the 
    *    internalSave-method the first time this map is saved, 
    *    and should then never be changed.
    *    This method is here to enable that old maps can have a 
    *    trueCreationTime set to make dynamic extra data and wasp work.
    *    This method might be removed when it is not needed anymore.
    */
   void setTrueCreationTime(uint32 time = MAX_UINT32);
      
   /// Get the time for the last WASPing of this map, normal or dynamic.
   inline uint32 getWaspTime() const;
      
   /// Update the time for the last WASPing of this map, normal or dynamic.
   void setWaspTime( uint32 time = MAX_UINT32 );
      
   /// Get the time for the last dynamic extra data addition to this map.
   inline uint32 getDynamicExtradataTime() const;
      
   /// Update the time for last dynamic extra data addition to this map.
   void setDynamicExtradataTime( uint32 time = MAX_UINT32 );
   //@}
   
   /**   Tells whether the strings in this map are coded in UTF-8 or
    *    as 8 bit ASCII iso-8859-1.
    *
    *    @return True if the strings in the map are coded in UTF-8.
    */
   inline bool stringsCodedInUTF8() const;

   /**
    *    Tells if this map has been filtered in levels or not.
    *    @return  True if the map is filtered in levels, false if unfiltered.
    */
   inline bool mapIsFiltered() const;
   
   /// Update the mapFiltered member.
   inline void setMapFiltered( bool isFiltered );
   
   /**
    *    Tells if the map gfx data of this map has been filtered in levels 
    *    or not.
    *    @return  True if the map gfx data is filtered in levels,
    *             false if unfiltered.
    */
   inline bool mapGfxDataIsFiltered() const;
   
   /// Update the mapGfxDataFiltered member.
   inline void setMapGfxDataFiltered( bool isFiltered );
   
   /**   Returns the directory where the maps of this country are 
    *    stored. 
    *
    *    @return Absolute path to country map store directory. Returns
    *            "" when not set.
    */
   inline const char* getMapCountryDir() const;

   /**
    *    @return Returns one variable per string. Each string includes
    *            both the name of the variable and its value.
    */
   vector<MC2String> getHeaderAsStrings() const;

   /**
    * @return total map size
    */
   inline uint32 getTotalMapSize() const;

   /**
    * @return number of allocators
    */
   inline uint32 getNbrAllocators() const;

   /**
    * @return version number of the loaded map
    */
   inline bool getLoadedVersion() const;

   inline bool groupsAreInLocationNameOrder() const;

   inline const char* getPathName() const { return m_pathname; }

   /**
    *    Number of initial item allocators. These are the allocators for
    *    special item types (from StreetSegmentItem to FerryItem) that are
    *    stored before the special allocators. Variable needed to be able 
    *    to add more item types and more item allocators.
    */
   static const uint32 numberInitialItemTypeAllocators;


protected:
   /**
    *   Prints the filename into the membervariable filename.
    *   Filename is absolute path + mapid printed in 9 positions 
    *   (zeroes in the begining) + ".mcm" at the end.
    */
   void setFilename(const char* path);

   /**
    *   Sets variables to default values.
    */
   void initEmptyHeader(uint32 id);
   
   /**
    *   Save this map into outfile.
    *   @param   outfile  The filedescriptor of the file where the
    *                     map will be saved.
    *   @return  True if the map is saved correctly, false otherwise.
    */
   virtual bool internalSave(int outfile);
   
   /**
    *    Create the map from a databuffer (preferrably with a 
    *    memory-mapped file).
    *    @param   A data buffer with all raw data for this map.
    */
   virtual bool internalLoad(DataBuffer& dataBuffer);

   /**
    *    Runs the program specified by command and tries to
    *    fill the DataBuffer from it.
    *    @return A databuffer if the loading was successful,
    *            NULL if failure.
    */
   DataBuffer* getDataBufferFromProgram(const char* command);
   
   /**
    *   Total size of the map on disk in bytes. Exclusive the
    *   first 8 bytes containing the mapID and this size.
    *   @remark This is not used and not implemented!!!
    */
   uint32 m_totalMapSize;

   /**
    *  True if the groups are either completely inside each other
    *  in a nice hierarchy or sorted in location name order.
    */
   bool m_groupsInLocationNameOrder;
 
   byte m_loadedVersion;

   /** 
    *   The id of this map
    */
   uint32 m_mapID;

   /** 
    *   The file name of this map <b>without any extension</b>.
    *   Is that true?
    */
   char* m_filename;
   
   /** 
    *   The path of this map. Needed when changeing the filename.
    */
   char* m_pathname;
   
   /** 
    *   The name of this map.
    */
   char* m_name;

   /** 
    *   The origin of this map.
    *   Typically the map supplier and release, e.g. TeleAtlas_2009_09
    */
   char* m_origin;

   /// The country that this map a part of
   StringTable::countryCode m_country;
   
   /// Driving on the right side of the road?
   bool m_driveOnRightSide;

   /** 
    *   Native languages in m_country. To be counted as a native langugae
    *   in an mcm map, there needs to be at least a certain nbr of
    *   names in the map that have that language.
    *   Calculated in OldGenericMap::updateNativeLanguages
    *   @see Item.h The supported languages.
    */
   Vector* m_nativeLanguages;
   
   /**
    *   The currencies in this country.
    */
   Vector* m_currency;
   
   /**
    *    String with copyright information for the images that are
    *    generated from this map.
    */
   char* m_copyrightString;

   /**
    *   The time this map was created (last saved), in UNIX-time 
    *   (seconds since 1970-01-01).
    */
   uint32 m_creationTime;

   /**
    *   The number of allocators.
    */
   uint32 m_nbrAllocators;

   /**
    *    The true time this map was created = the time when it was saved
    *    for the first time, in UNIX-time (seconds since 1970-01-01).
    */
   uint32 m_trueCreationTime;

   /**
    *    The last time when this map was WASPed, normal or dynamic
    *    (in UNIX-time). This time is valid only for underview maps.
    */
   uint32 m_waspTime;
   
   /**
    *    The last time when this map had extra data added dynamic
    *    (in UNIX-time). Tims time is valid only for underview maps.
    */
   uint32 m_dynamicExtradataTime;

   /**
    *    If this one is true, the strings in the map are coded as UTF-8.
    *    Otherwise the strings are coded as 8 bit ASCII iso-8859-1.
    */
   bool m_utf8Strings;

   /**
    *    Tells if the map has been filtered in levels or not. If filtered the
    *    levels are stored in the last 2 bits of lat and lon, else the
    *    coordinates have undefined filter level with the true (=original)
    *    values on lat and lon.
    */
   bool m_mapFiltered;

   /**
    *    Tells if the map gfx data of this map has been filtered in levels
    *    or not.
    *    If filtered the levels are stored in the last 2 bits of lat and 
    *    lon, else the coordinates have undefined filter level with the 
    *    true (=original) values on lat and lon.
    */
   bool m_mapGfxDataFiltered;

   /**
    *    This is the directory where the maps of this country are stored.
    *    NB! Not set by any method!
    */
   const char* m_mapCountryDir;
};

// ========================================================================
//                                      Implementation of inlined methods =
inline bool 
OldGenericMapHeader::operator == (const VectorElement& elm) const 
{
   return ( m_mapID == ((OldGenericMapHeader*) (&elm))->m_mapID);
}

inline bool 
OldGenericMapHeader::operator != (const VectorElement& elm) const 
{
   return ( m_mapID != ((OldGenericMapHeader*) (&elm) )->m_mapID);
}

inline bool 
OldGenericMapHeader::operator > (const VectorElement& elm) const 
{
   return ( m_mapID > ((OldGenericMapHeader*) (&elm) )->m_mapID);
}

inline bool 
OldGenericMapHeader::operator < (const VectorElement& elm) const 
{
   return ( m_mapID < ((OldGenericMapHeader*) (&elm) )->m_mapID);
}

inline uint32 
OldGenericMapHeader::getMapID() const
{
   return (m_mapID);
}


inline const char* 
OldGenericMapHeader::getMapName() const
{
   return m_name;
}

inline const char* 
OldGenericMapHeader::getMapOrigin() const
{
   return m_origin;
}


inline const MapGenEnums::mapSupplier 
OldGenericMapHeader::getMapSupplier() const
{
   return MapGenUtil::getMapSupAndVer(m_origin).first;
}

inline const MapGenEnums::mapVersion  
OldGenericMapHeader::getMapVersion() const
{
   return MapGenUtil::getMapSupAndVer(m_origin).second;
}


inline const char* 
OldGenericMapHeader::getCopyrightString() const
{
   return m_copyrightString;
}

inline StringTable::countryCode 
OldGenericMapHeader::getCountryCode() const
{
   return m_country;
}

inline const char* 
OldGenericMapHeader::getFilename() const
{ 
   return m_filename; 
}

inline uint32 
OldGenericMapHeader::getNbrNativeLanguages() const
{
   if (m_nativeLanguages != NULL)
      return (m_nativeLanguages->getSize());
   else
      return (0);
}

inline LangTypes::language_t
OldGenericMapHeader::getNativeLanguage(uint32 i) const
{
   if (i < getNbrNativeLanguages())
      return (LangTypes::language_t(m_nativeLanguages->getElementAt(i)));
   else
      return (LangTypes::invalidLanguage);
}

inline uint32 
OldGenericMapHeader::getNbrCurrencies() const
{
   if (m_currency != NULL)
      return (m_currency->getSize());
   else
      return (0);

}

inline StringTable::stringCode 
OldGenericMapHeader::getCurrency(uint32 i) const
{
   if (i < getNbrCurrencies())
      return (StringTable::stringCode(m_currency->getElementAt(i)));
   else
      return (StringTable::NOSTRING);

}

inline uint32 
OldGenericMapHeader::getCreationTime() const
{
   return (m_creationTime);
}

inline bool 
OldGenericMapHeader::driveOnRightSide() const
{
   return (m_driveOnRightSide);
}

inline uint32
OldGenericMapHeader::getTrueCreationTime() const
{
   return m_trueCreationTime;
}

inline uint32
OldGenericMapHeader::getWaspTime() const
{
   return m_waspTime;
}

inline uint32
OldGenericMapHeader::getDynamicExtradataTime() const
{
   return m_dynamicExtradataTime;
}

inline bool
OldGenericMapHeader::stringsCodedInUTF8() const
{
   return m_utf8Strings;
}

inline bool
OldGenericMapHeader::mapIsFiltered() const
{
   return m_mapFiltered;
}

inline void
OldGenericMapHeader::setMapFiltered( bool isFiltered )
{
   m_mapFiltered = isFiltered;
}

inline bool
OldGenericMapHeader::mapGfxDataIsFiltered() const
{
   return m_mapGfxDataFiltered;
}

inline void
OldGenericMapHeader::setMapGfxDataFiltered( bool isFiltered )
{
   m_mapGfxDataFiltered = isFiltered;
}

inline const char* 
OldGenericMapHeader::getMapCountryDir() const
{
   return m_mapCountryDir;
}

inline uint32 
OldGenericMapHeader::getTotalMapSize() const
{
   return m_totalMapSize;
}

inline bool
OldGenericMapHeader::getLoadedVersion() const
{
   return m_loadedVersion;
}

inline bool
OldGenericMapHeader::groupsAreInLocationNameOrder() const 
{
   return m_groupsInLocationNameOrder;
}

inline uint32 
OldGenericMapHeader::getNbrAllocators() const
{
   return m_nbrAllocators;
}

#endif
