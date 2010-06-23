/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERFAVORITES_H
#define USERFAVORITES_H

#include "config.h"
#include "ItemInfoEntry.h"
#include <list>
#include <vector>

class Packet;


/**
 *  This encapsulates a User Favorite that can be stored and retreived 
 *  via the User Module.
 *
 *
 */
class UserFavorite {
   public:
      typedef vector< ItemInfoEntry > InfoVect;


      /** 
       *   Constructor, version for use with synchronization.
       *   @param favID       The ID of this favorite
       */
      UserFavorite(const uint32 favID); 

      /** 
       *   Constructor, reads itself from a Packet
       *   @param packet  Pointer to the packet to use
       *   @param pos     Reference the position to use, will point to the
       *                  next pos after this favorite when the constructor
       *                  is done
       *   @param onlyID  Assume that only the ID has been stored in the
       *                  packet
       */
      UserFavorite(const Packet* packet, int& pos, bool onlyID = false); 

      /** 
       *   Constructor, full version. favID is ignored if this favorite is 
       *   going to be added. Any strings that are too long will be 
       *   truncated
       *
       *   @param favID       The ID of this favorite
       *   @param lat         The latitude of the position
       *   @param lon         The longitude of the position
       *   @param name        The name of this favorite, max 255 chars
       *   @param shortName   The short name for this favorite, max 30 
       *                      chars.
       *   @param description The description for this favorite, max 255 
       *                      chars.
       *   @param category    The category of this favorite, max 255 chars
       *   @param mapIconName The name of the map icon for this favorite,
       *                      max 255 chars
       *   @param infoStr     The id-key-values in a string.
       */
      UserFavorite( const uint32 favID, 
                    const int32 lat, const int32 lon, 
                    const char* name,  const char* shortName,
                    const char* description, const char* category,
                    const char* mapIconName,
                    const char* infoStr );


      /** 
       *   Constructor, full version. favID is ignored if this favorite is 
       *   going to be added. Any strings that are too long will be 
       *   truncated
       *
       *   @param favID       The ID of this favorite
       *   @param lat         The latitude of the position
       *   @param lon         The longitude of the position
       *   @param name        The name of this favorite, max 255 chars
       *   @param shortName   The short name for this favorite, max 30 
       *                      chars.
       *   @param description The description for this favorite, max 255 
       *                      chars.
       *   @param category    The category of this favorite, max 255 chars
       *   @param mapIconName The name of the map icon for this favorite,
       *                      max 255 chars
       *   @param infoStr     The id-key-values in an array of 
       *                      ItemInfoEntries.
       */
      UserFavorite( const uint32 favID, 
                    const int32 lat, const int32 lon, 
                    const char* name,  const char* shortName,
                    const char* description, const char* category,
                    const char* mapIconName,
                    const InfoVect& v );


      /** 
       *   Constructor, makes a copy of another UserFavorite
       *   @param fav     Pointer to another UserFavorite
       *   @param onlyID  Assume that only the ID has been stored in the
       *                  packet
       */
      UserFavorite(UserFavorite* fav, bool onlyID = false); 

      /**
       *   Destructor, cleans up
       */
      ~UserFavorite();
      
      /**
       *   Store this favorite in a packet. No check is made to
       *   see if there's space enough for it to fit.
       *   @param packet  The packet to store this favorite in
       *   @param pos     Reference the position to use, will point to the
       *                  next pos after this favorite when the constructor
       *                  is done
       *   @param onlyID  Store only the ID in the packet
       */
      void store(Packet* packet, int& pos, bool onlyID = false) const;

      /** 
       *   Get the ID of this favorite
       *   @return The ID of the favorite
       */
      inline const uint32 getID() const;

      /** 
       *   Set the ID of this favorite
       *   @param id The new ID of the favorite
       */
      inline void setID(const uint32 id);

      /** 
       *   Get the latitude of this favorite
       *   @return The latitude of the favorite
       */
      inline const int32 getLat() const;

      /** 
       *   Get the longitude of this favorite
       *   @return The longitude of the favorite
       */
      inline const int32 getLon() const;

      /** 
       *   Get the name of this favorite
       *   @return The name of the favorite
       */
      inline const char* getName() const;

      /** 
       *   Get the description of this favorite
       *   @return The description of the favorite
       */
      inline const char* getDescription() const;

      /** 
       *   Get the category of this favorite
       *   @return The category of the favorite
       */
      inline const char* getCategory() const;

      /** 
       *   Get the short name of this favorite
       *   @return The short name of the favorite
       */
      inline const char* getShortName() const;

      /** 
       *   Get the name of this favorite
       *   @return The name of the favorite
       */
      inline const char* getMapIconName() const;


      /** 
       * Get the info entries for the favorite.
       *
       * @return The info entries for the favorite.
       */
      const InfoVect& getInfos() const;


      /**
       * If favorite has any info field of a type.
       */
      bool hasInfoType( ItemInfoEnums::InfoType type ) const;


      /**
       * Get the size of the favorite packed into a Packet.
       */
      uint32 getSize() const;


      /**
       * Make ItemInfoEntry vector from string.
       *
       * @param str The string to parse.
       * @param v The InfoVect to add infos to.
       * @return True if string is valid, false if not.
       */
      static bool makeInfoVect( const char* str, InfoVect& v );


      /**
       * Make string from ItemInfoEntry vector.
       *
       * @param v The InfoVect to turn into string.
       * @param str The string to append to.
       */
      static void makeInfoStr( const InfoVect& v, MC2String& str );

   /**
    * Calculates and returns crc of this favorite
    * @return crc 
    */
   uint32 getCRC() const;

   private:
      /**
       * Inits all but m_infos.
       */
      void init( const uint32 favID, 
                 const int32 lat, const int32 lon, 
                 const char* name,  const char* shortName,
                 const char* description, const char* category,
                 const char* mapIconName );

      /** 
       *   The empty constructor, should not be used.
       */
      UserFavorite(); 

      // the ID of this favorite
      uint32 m_favID;
 
      // position of the faorite (latitude)
      int32 m_lat;
 
      // position of the faorite (longitude)
      int32 m_lon;
 
      // the name of this favorite
      char* m_name;
 
      // the description for this favorite
      char* m_description;
 
      // the category for this favorite
      char* m_category;
 
      // the short name (if any) for this favorite
      char* m_shortName;

      // the icon name (if any) for this favorite
      char* m_mapIconName;

      /// The ItemInfoEntries (if any) for this favorite
      InfoVect m_infos;

      // true if the strings were allocated in the constructor
      bool m_selfAlloc;
};

/**
 *   Container for a list of UserFavorites, can store and restore itself
 *   in/from a Packet.
 *
 **/

class UserFavoritesList : public list<UserFavorite*>
{
   public:

      /**
       *   Destructor, will also delete all the favorites in the list
       */
      ~UserFavoritesList();

      /**
       *   Store the list in a packet.
       *   @param packet  The packet to store this list in
       *   @param pos     Reference the position to use, will point to the
       *                  next pos after this favorite when the constructor
       *                  is done
       *   @param onlyID  Store only the ID in the packet
       */
      void store(Packet* packet, int& pos, bool onlyID = false);

      /**
       *   Restore a list from a packet.
       *   @param packet  The packet to restore this list from
       *   @param pos     Reference the position to use, will point to the
       *                  next pos after this favorite when the constructor
       *                  is done
       *   @param onlyID  Assume that only the ID has been stored in the
       *                  packet
       */
      void restore(const Packet* packet, int& pos, bool onlyID = false);

      /**
       *   Add an already created favorite. It will be deleted when
       *   the list is deleted.
       *   @param favID       Pointer to a UserFavorite
       */
      void addFavorite(UserFavorite* userFav);

      /**
       *   Add a favorite with only ID, used for sync list.
       *   @param favID       The ID of the favorite
       */
      void addFavorite(const uint32 favID); 

      /** 
       *   Add a favorite, full version. favID is ignored if this favorite
       *   is going to be added.
       *
       *   @param favID       The ID of this favorite
       *   @param lat         The latitude of the position
       *   @param lon         The longitude of the position
       *   @param name        The name of this favorite
       *   @param shortName   The short name for this favorite
       *   @param description The description for this favorite
       *   @param category    The category of this favorite
       *   @param mapIconName The name of the map icon for this favorite
       */
      void addFavorite( const uint32 favID, 
                        const int32 lat, const int32 lon, 
                        const char* name,  const char* shortName,
                        const char* description, const char* category,
                        const char* mapIconName,
                        const char* infoStr );

      /**
       *   Sort the list contents based on the ID of the favorites
       */
      void sortByID();

};

// ========================================================================
//                                  Implementation of the inlined methods =

inline const uint32
UserFavorite::getID() const
{
   return m_favID; 
}

inline void
UserFavorite::setID(uint32 id)
{
   m_favID = id; 
}

inline const int32
UserFavorite::getLat() const
{
   return m_lat; 
}

inline const int32
UserFavorite::getLon() const
{
   return m_lon; 
}

inline const char*
UserFavorite::getName() const
{
   return m_name; 
}

inline const char*
UserFavorite::getDescription() const
{
   return m_description; 
   
}

inline const char*
UserFavorite::getCategory() const
{
   return m_category; 
}

inline const char*
UserFavorite::getShortName() const
{
   return m_shortName; 
}

inline const char*
UserFavorite::getMapIconName() const
{
   return m_mapIconName; 
}


inline const UserFavorite::InfoVect&
UserFavorite::getInfos() const {
   return m_infos;
}


#endif // USERFAVORITES_H
