/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCH_MATCH_H
#define SEARCH_MATCH_H

#include "config.h"

#include <vector>

#include "MC2String.h"
#include "SearchTypes.h"
#include "SearchMatchPoints.h"
#include "IDPairVector.h"
#include "MC2Coordinate.h"
#include "MC2BoundingBox.h"
#include "NewStrDup.h"
#include "SearchSorting.h"
#include "StringUtility.h"
#include "MapRights.h"
#include "POIReview.h"

class VanillaSearchReplyPacket; // forward
class SearchRequestPacket;      // forward (extern)
class MatchLink;
class VanillaRegionMatch;
class Packet;
class ItemInfoEntry;



#define VANILLAMATCH  1
#define OVERVIEWMATCH 2

typedef vector< pair<VanillaRegionMatch*, bool> > RegionMatchVector;

/**
  *   Superclass to OverviewMatch and VanillaMatch.
  *   FIXME: Remove unused members.
  *   FIXME: Remove strange constructors.
  *   FIXME: Use createMatch in SearchModule too.
  *   FIXME: Un-inline the constructors so that they can be inlined
  *          in the .cpp-file instead.
  *   FIXME: Do something about the names and when to delete them.
  *   FIXME: Add flags to load and save so that the regions are saved in
  *          a more compact fashion.
  */
class SearchMatch {
public:

   /// Type of vector that contains the ItemInfos
   typedef vector<ItemInfoEntry> ItemInfoVector;
   
   /**
    *   @param   tpd 
    *   Don't forget to update the copy-constructor.
    */
   SearchMatch( int tpd,
                uint32 mapID,
                uint32 itemID,
                ItemTypes::itemType itemType,
                uint16 itemSubtype,
                const SearchMatchPoints& points,
                uint8 restrictions,
                uint16,
                uint32 type,
                const char* name,
                const char* alphaSortingName,
                const char* locationName,
                uint32 nameInfo,
                uint32 locationNameInfo,
                uint32 location);
   
   /**
    *   Creates a SearchMatch for e.g. routing.
    *   Strings will be copied.
    *   Don't forget to update the copy-constructor.
    */
   SearchMatch( int tpd,
                uint32 type,
                const IDPair_t& id,
                const char* name,
                const char* locationName);

   /**
    *    For conversion to itemid.
    */
   operator const IDPair_t&() const { return m_id; };

   /**
    *   Print the SearchMatch on an ostream. Will print a dec to the stream.
    *   @param stream The stream to print on.
    *   @param match  The SearchMatch to print.
    *   @return The stream.
    */
   friend ostream& operator<<( ostream& stream,
                               const SearchMatch& match);
   
   /**
    * Creates a SearchMatch from the supplied string.
    * Useful when sending matches back and forth from clients.
    *
    * @param str String to convert into match.
    * @return A new Match or NULL if problem with str.
    */
   static SearchMatch* createMatch( const char* str );

   /**
    *   @see createMatch( const char* str )
    */
   static SearchMatch* createMatch( const MC2String& str );

   /**
    *   Creates a SearchMatch with the correct dynamic type
    *   using the type in searchType.
    *   @param searchType Searchtype of the type of match
    *                     to create.
    *   @param id         Optional parameter for id.
    *   @param offset     Optional offset parameter.
    *   @return New match.
    */
   static SearchMatch* createMatch( uint32 searchType,
                                    const IDPair_t& id = IDPair_t(),
                                    uint16 offset = 0);

   /**
    *   Cretes a match from e.g. an Item.
    *   @param name         The name of the match.
    *   @param locationName Where the match is located.
    *   @param id           The map and item id of the match.
    *   @param itemType     The item type of the item.
    *   @param itemSubType  The item subtype of the ite,
    */
   static SearchMatch* createMatch( const char* name,
                                    const char* locationName,
                                    const IDPair_t& id,
                                    const MC2Coordinate& coords,
                                    ItemTypes::itemType itemType,
                                    uint32 itemSubType );

   /**
    *   Reads a match from a packet and creates one with
    *   the correct type.
    *   @param packet The Packet to read from.
    *   @param pos    Position to start at.
    */
   static SearchMatch* createMatch( const Packet* packet,
                                    int& pos,
                                    bool compactForm=false);
   
   /**
    *   Saves the match into the packet.
    *   Note that save saves the type to the packet which is not
    *   read by load, but only by createMatch.
    *   FIXME: Add compact form of saving for regions of matches.
    *          (Points are not needed then, for instance).
    */
   virtual int save(Packet* packet, int& pos, bool compactForm = false) const;
   
   /**
    *   Sets a new name.
    */
   void setName(const char* name0) {
      const char* locationName = m_locationName;
      if ( m_deleteStrings ) {
         delete [] m_name;
         delete [] m_alphaSortingName;
      }
      m_name             = NewStrDup::newStrDup( name0 );
      m_alphaSortingName = NewStrDup::newStrDup( name0 );
      m_locationName     = NewStrDup::newStrDup( getLocationName() );
      if ( m_deleteStrings ) {
         delete [] locationName;
      }
      m_deleteStrings    = true;
   }

   /**
    * Prints the SearchMatch to a string.
    * NB! Data is lost, but you should still be able to search and route.
    *
    * @param match The SearchMatch to print as string.
    * @param str The string to print to.
    * @param len The length of str, should be searchMatchBaseStrLength 
    *            long.
    * @param extradata If to add extraData, not implemented.
    * @return True if printed ok, false if not.
    */
   static bool matchToString( const SearchMatch* match,
                              char* str, uint32 len, bool extradata );

   /**
    *   @see matchToString.
    */
   MC2String matchToString(bool extradata = false) const;


   /**
    * The base size of a match as a string.
    */
   static uint32 searchMatchBaseStrLength;

   
   /**
    *   Copy constructor. Copies stuff. I.e. regions.
    */
   SearchMatch(const SearchMatch& other);

   /**
    *   Destructor. Deletes the regions.
    */
   virtual ~SearchMatch();

   /**
    *   Returns the typeid. Can be OVERVIEWMATCH or VANILLAMATCH, I think.
    */
   int getTypeid() {return m_typeid;}

   /**
    *   Sets the coordinates for the match.
    *   @param coords New coordinates.
    */
   inline void setCoords(const MC2Coordinate& coords) {
      m_coordinates = coords;
   }

   /**
    *   Sets new angle.
    */
   inline void setAngle( uint16 angle ) {
      m_angle = angle;
   }

   /**
    *   Sets the external source and id of the match.
    *   Will affect the conversion to and from strings
    *   to send to clients.
    *   @param extSource External source @see ExtServices
    *   @param extID     ID to be able to map the match back
    *                    for e.g. getting more info.
    *                    @see ItemInfoRequest
    */
   inline void setExtSourceAndID( uint32 extSource,
                                  const MC2String& extID ) {
      m_extSource = extSource;
      m_extID     = extID;
   }

   /// Returns external source id @see ExtServices.
   inline uint32 getExtSource() const {
      return m_extSource;
   }

   /// Returns external id
   inline const MC2String& getExtID() const {
      return m_extID;
   }

   /// Returns the angle or MAX_UINT16
   inline uint16 getAngle( ) const {
      return m_angle;
   }

   /// Returns the synonym name if any
   inline const char* getSynonymName() const {
      return m_synonymName ? m_synonymName : "";
   }

   /// Sets the synonym name
   void setSynonymName( const char* synonymName );
   
   /**
    *   Sets the coordinates for the match.
    *   @param lat New latitude.
    *   @param lon New longitude.
    */
   inline void setCoords(int32 lat, int32 lon) {
      m_coordinates = MC2Coordinate(lat, lon);
   }

   /**
    *   Returns a reference to the coordinates of the match.
    */
   inline const MC2Coordinate& getCoords() const {
      return m_coordinates;
   }

   /**
    *   Sets the bounding box of the match.
    */
   inline void setBBox(const MC2BoundingBox& bbox) {
      m_bbox = bbox;
   }
   
   /**
    *   Returns a reference to the bounding box of the match.
    */
   inline const MC2BoundingBox& getBBox() const {
      return m_bbox;
   }
   
   /**
    *   Returns the mapid of the match.
    */
   inline uint32 getMapID() const {
      return m_id.getMapID();
   }

   /**
    *   Sets the mapid of the match.
    */
   inline void setMapID(uint32 mapID) {
      m_id.first = mapID;
   }

   /**
    *   Returns the item id of the match.
    */
   inline uint32 getItemID() const {
      return m_id.getItemID();
   }

   /**
    *   @param itemID the itemID
    */
   void setItemID( uint32 itemID ) {
      m_id.second = itemID;
   }

   /**
    *   Returns the complete ID of the match.
    */
   const IDPair_t& getID() const {
      return m_id;
   }
   
   /**
    *   Returns the itemtype of the match.
    */
   inline ItemTypes::itemType getItemType() const {
      return ItemTypes::itemType(m_itemType);
   }

   /**
    *   Returns the itemsubtype of the match.
    */
   inline uint16 getItemSubtype() const {
      return uint16(m_itemSubtype);
   }

   /**
    *   Returns the offset. Only applicable for streetmatches.
    *   @return 0.
    */
   virtual uint16 getOffset() const {
      return 0;
   }
   
   /**
    *    Get the number of regions for this match.
    *    @return The number of regions for this match.
    */
   inline uint32 getNbrRegions() const {
      return m_regions.size();
   }

   /**
    *    Get the index'th region.
    *    @param index The index of the region to get.
    *    @return The region at index index.
    */
   VanillaRegionMatch* getRegion( uint32 index ) const {
      if ( index < m_regions.size() ) {
         return m_regions[ index ].first;
      } else {
         return NULL;
      }
   }

   /**
    *    Returns pointers to the regions of the match that
    *    are suitable for printing in the result.
    *    <br />
    *    Will only work for city parts, bua:s and municipals.
    *    <br />
    *    @param printRegions   Vector to <b>add</b> the matches to.
    */
   void getRegionsToPrint( vector<const SearchMatch*>& printRegions ) const;
   
   /**
    *    Add a region to this match.
    *    @param region The region to add.
    *    @param deleteOnDestruction If the region should be deleted in 
    *                               descructor, default false.
    */
   inline void addRegion( VanillaRegionMatch* region, 
                          bool deleteOnDestruction = false );

   /**
    *    Removes region number <code>idx</code>. The indeces of
    *    the internal structure will change after this.
    *    @param idx Region number to remove.
    */
   inline void removeRegion(uint32 idx);
   
   /**
    *    Sets new regions for the match.
    *    @param regions The regions to add.
    *    @param copy    True if the regions should be copied.
    *                   (Default is false).
    */
   void setRegions( const RegionMatchVector& regions, bool copy = false );

   /**
    *    Copies the regions from the other match into this one.
    */
   void setRegions( const SearchMatch& otherMatch) {
      setRegions( otherMatch.m_regions, true);
   }
   
   /**
    *    Returns the distance from a point supplied to the SearchRequest.
    *    The distance is in meters.
    */
   uint32 getDistance() const {
      return m_distance;
   }

   /**
    *    Sets the distance to a point supplied to the SearchRequest.
    *    Unit is  meters.
    */
   void setDistance(uint32 distance) {
      m_distance = distance;
   }
   
   /** 
    *    Get the search restrictions that were used to get this match.
    *    <table>
    *    <tr><td>Bit 1</td><td>Is set if we were doing close matching 
    *       instead of exact matching.</td></tr>
    *    <tr><td>Bit 2</td><td>Is set if we were searching in BUAs 
    *       instead of city parts.</td></tr>
    *    <tr><td>Bit 3</td><td>Is set if we were searching in Municipals 
    *       instead of BUAs.</td></tr>
    *    <tr><td>Bit 4</td><td>Is set if we were searching anywhere in 
    *       the string instead of in the beginning of the string.</td></tr>
    *    <tr><td>Bit 5</td><td>Is set if we were searching in the whole 
    *       map without location masking.</td></tr>
    *    <tr><td>Bit 6</td><td>Is set if we were ignoring any category 
    *       masking.</td></tr> 
    *    <tr><td>Bit 7</td><td>Is set if we were trying phonetic search.
    *       </td></tr>
    *    <tr><td>Bit 8</td><td>Is set if we were trying to change the 
    *       search string.</td></tr>
    *    </table>
    *    @return the restrictions from the search. It contains 8 bits.
    */
   inline uint8 getRestrictions() const {
      return m_restrictions;
   }

   /// Set the restrictions
   void setRestrictions( uint8 res ) { m_restrictions = res; }

   /// Get the confidence points
   const SearchMatchPoints& getPoints() const { return m_points; }

   SearchMatchPoints& getPointsForWriting() { return m_points; }
   SearchMatchPoints& getPointInfo() { return m_points; }
   
   
   /// Set the points
   void setPoints( const SearchMatchPoints& points ) { m_points = points; }
   
   /**
    *    Get the type of this object.
    *    @return  The type of the object.
    */
   uint32 getType() const {
      return m_type;
   }

   /** 
    *   @param the type
    */
   void setType( uint32 type ) {
      m_type = type;
      m_points.setTypes( m_type, m_itemType, m_itemSubtype );
   }
   
   /**
    *    Returns a name of the object.
    */
   const char* getName() const { return m_name; }

   /**
    *    Returns the name for alphaSorting.
    */
   const char* getAlphaSortingName() const { return m_alphaSortingName; }

   /**
    *    Returns a nice location name.
    */
   const char* getLocationName() const;

   /**
    *    Sets a new location name.
    */
   void setLocationName(const char* newLocationName);

   /**
    *    Uses the regions of the match to update the
    *    location name.
    */
   void updateLocationName(uint32 requestedTypes);

   /**
    *   Sets the itemtype of the match.
    */
   inline void setItemType( ItemTypes::itemType type ) {
      m_itemType = type;
      m_points.setTypes( m_type, m_itemType, m_itemSubtype );
   }

   /**
    *   Sets the item subtype of the match.
    */
   inline void setItemSubType( uint16 subType ) {
      m_itemSubtype = subType;
      m_points.setTypes( m_type, m_itemType, m_itemSubtype );
   }

   /**
    *   Returns a reference to the ItemInfos.
    */
   inline const ItemInfoVector& getItemInfos() const {
      return m_itemInfos;
   }
   
   /**
    *   Sets new ItemInfos by swapping.
    */
   inline void swapItemInfos( ItemInfoVector& itemInfos ) {
      m_itemInfos.swap( itemInfos );
   }

   /**
    * Merges the item infos to a new list merging some of the
    * address fields etc.
    */
   void mergeToSaneItemInfos( LangTypes::language_t language, uint32 topRegion );

   /**
    * Set the map rights of this match
    * @param rights Map rights to set
    */
   void setMapRights( const MapRights& rights ) {
      m_mapRights = rights;
   }

   MapRights getMapRights() const {
      return m_mapRights;
   }

   bool getAdditionalInfoExists() const {
      return m_additionalInfoExists;
   }

   void setAdditionalInfoExists( bool moreInfo ) {
      m_additionalInfoExists = moreInfo;
   }

protected:

   /**
    *   Creates a match from a string created by a match
    *   from an external source.
    */
   static SearchMatch* createExtMatch( const char* str );
   
   /**
    *   Loads the match from a packet.
    */
   virtual int load(const Packet* packet, int& pos, bool compactForm = false);

   /**
    *    Uses the regions of the match to update the
    *    location name. Locationame should be the one
    *    that looks nice when printing.
    *    @return New string that contains location name. Should be deleted
    *            by the caller.
    */
   char* createLocationName(uint32 requestedTypes,
                            vector<const SearchMatch*>* printRegs) const;
   
   /**
    *   Follows the region upwards and sets the location name.
    *   Duplicate names (E.g. Lund, LUND) are removed.
    *   @param startReg The region to start at.
    *   @param allowedSearchTypes Allowed searchTypes for the regions.
    *   @return New string containing locationname.
    */
   char* getLocationNameFromRegionAndUp(const SearchMatch* startReg,
                                        uint32 allowedSearchTypes,
                                        vector<const SearchMatch*>* printRegs)
      const;
   
   /**
    *    Converts the typeChar into a SEARCH_*.
    *    @param typeChar Type as character.
    *    @return Type as defined in SearchTypes.h
    */
   static uint32 typeCharacterToSearchType( char typeChar );

   /**
    *    Converts the typeChar into a SEARCH_*.
    *    @param typeChar Type as character.
    *    @return Type as defined in SearchTypes.h
    */
   static char searchTypeToTypeCharacter( uint32 searchType );

   /**
    *   Deletes the regions and empties the region vector.
    */
   void deleteRegions();
   
   /**
    *   Type id.
    */
   int m_typeid;

   /**
    *   The map and item id of the match.
    */
   IDPair_t m_id;
   
   /**
    *   The itemtype of the match.
    */
   ItemTypes::itemType m_itemType;

   /**
    *   The itemsubtype of the match.
    */
   uint16 m_itemSubtype;

   /// The bounding box of the match. Must be looked up in the server.
   MC2BoundingBox m_bbox;

   /// The coordinate of the match. Must be looked up in the server
   MC2Coordinate m_coordinates;

   /// The regions of this match.
   RegionMatchVector m_regions;
   /// Vector of ItemInfo
   ItemInfoVector m_itemInfos;

   /// The distance in meters from an origin supplied in SearchRequest.
   uint32 m_distance;

   /// The confidence sort points for the match.
   SearchMatchPoints m_points;
   
   /// The restrictions used to get the match
   uint8 m_restrictions;

   /// The type of object
   uint32 m_type;

   /// A name of the object
   const char *m_name;

   /** 
    *    A name of the location.
    *    @deprecated Location is replaced by regions.
    */
   const char *m_locationName;

   /** 
    *    The name used for alphabetical sorting (normally in all 
    *    uppercase).
    */
   const char *m_alphaSortingName;

   /// Synonym name of the match. Used when getting extra info.
   char* m_synonymName;

   /**
    *   True if the strings that the m_names** points to is created
    *   in this object (and must be deleted here), false otherwise.
    */
   bool m_deleteStrings;

   /// The type and language of m_name
   uint32 m_nameInfo;
   
   /**
    *    The type and language of the locationname.
    *    @deprecated Location is replaced by regions.
    */
   uint32 m_locationNameInfo;

   /// Angle UINT16 is invalid
   uint16 m_angle;

   /// ID of the match if it is from an external source
   MC2String m_extID;
   /// ID of the external source as in ExtServices
   uint32 m_extSource;

   // Map rights of the match
   MapRights m_mapRights;

   /// Additional info about this match exists
   bool m_additionalInfoExists;
   
   /**
    *    Map mapping characters to types.
    */
   static map<char, uint32> c_typeCharacterMap;

   /**
    *    Initializes the type character map.
    */
   static map<char, uint32> initTypeCharMap();

   /**
    * Map mapping types to characters.
    */
   static map<uint32, char> c_characterTypeMap;

   /**
    * Initializes the character type map.
    */
   static map<uint32, char> initCharTypeMap();
   
  protected:
   /**
    *   Protected default constructor, to avoid usage!
    */
   SearchMatch();
};


class SearchMatchIDLessThan {
   public:
      bool operator()(const SearchMatch& a, const SearchMatch& b) const {
         return a.getID() < b.getID();
      }
      bool operator()(const SearchMatch* a, const SearchMatch* b) const {
         return a->getID() < b->getID();
      }
};

class SearchMatchIDAndNameLessThan {
   public:
      bool operator()(const SearchMatch& a, const SearchMatch& b) const {
         if ( a.getID() != b.getID() ) {
            return a.getID() < b.getID();
         } else {
            return StringUtility::strcmp( a.getName(), b.getName() ) < 0;
         }
      }
      bool operator()(const SearchMatch* a, const SearchMatch* b) const {
         if ( a->getID() != b->getID() ) {
            return a->getID() < b->getID();
         } else {
            return StringUtility::strcmp( a->getName(), b->getName() ) < 0;
         }
      }
};


/**
  *   Obsolete class, use the vector of matches in SearchRequest.
  *
  */
class SearchMatchLink {
  public:
   /**
    *   Class for holding SearchMatches in a linked list.
    *   @param   match The SearchMatch to hold.
    */
   SearchMatchLink( SearchMatch* match );
   
   /**
    *   Destructor, doesn't delete match.
    */
   virtual ~SearchMatchLink();
   
   /**
    *   The match of this link.
    */
   inline SearchMatch* getMatch() const;

   /**
    *   The next link, NULL if no more.
    */
   inline SearchMatchLink* getNext() const;
      
   /**
    *   Sets the next link to next.
    */
   inline void setNext( SearchMatchLink* next );

  protected:
   /// The SearchMatch this link is holding.
   SearchMatch* m_match;
      
   /// The next link.
   SearchMatchLink* m_next;
};

/**
  *   VanillaMatch, superclass to the match objects.
  *
  */
class VanillaMatch : public SearchMatch {
  public:
   /**
    *   Constructor.
    *   @param names      A name.
    *   @param mapID      The mapID of the object.
    *   @param itemID     The itemID of the object.
    *   @param type       The object type.
    *   @param points     The number of points (used for sorting).
    *   @param alphaSortingName 
    *                     The name to use for sorting.
    *   @param location   The location of the object (or MAX_UINT32).
    */
   VanillaMatch( const char *name,
                 uint32 nameInfo,
                 const char *locationName,
                 uint32 locationNameInfo,
                 uint32 mapID,
                 uint32 itemID,
                 uint32 type,
                 const SearchMatchPoints& points,
                 uint16,
                 const char *alphaSortingName,
                 uint32 location,
                 uint8 restrictions,
                 ItemTypes::itemType itemType,
                 uint16 itemSubtype)
      :  
      SearchMatch( VANILLAMATCH, mapID, itemID, itemType, itemSubtype,
                   points, restrictions, 0, type, name, alphaSortingName,
                   locationName, nameInfo, locationNameInfo, location)
      {
         // Nothing more to do...
      }

   /**
    *    Constructor for e.g. servers.
    *    Strings will be copied.
    */
   VanillaMatch ( uint32 type,
                  const IDPair_t& id,
                  const char* name,
                  const char* locationName )
         : SearchMatch( VANILLAMATCH, type, id, name, locationName )
      {}
   
   /**
    *   Copy constructor that create new strings and these into
    *   this object.
    */
   VanillaMatch( const VanillaMatch& vm );

   /**
    *   Destructor.
    *   NB! The array of streetSegmentIDs are deleted here even if 
    *   it is created outside this class! If the member variable 
    *   m_deleteStrings is true also the strings are deleted, 
    *   otherwise they are not (since they in this case belongs to 
    *   the datafield in the packet).
    */
   virtual ~VanillaMatch();

   /**
    *    Get a copy of this VanillaMatch.
    *
    *    @return A copy of <this>.
    */
   virtual VanillaMatch* clone() const = 0;
         
   /**
    *   Adds <this> to the packet p
    *   @param   p  The packet to be added to.
    */
   virtual void addToPacket( VanillaSearchReplyPacket *p, 
                             bool increaseCount = true ) const;

   /**
    *    Get the ID of the item that this VanillaMatch represents.
    *    @deprecated 
    *    Kept for backwards compatibility.
    *    @return  The itemID.
    */
   uint32 getMainItemID() const {
      return getItemID();
   }


  protected:
 
};

class VanillaMatchWithSSI : public VanillaMatch {
public:
   /**
    * Constructor
    * @param nbrNames the number of names
    * @param names the names as a vector of char*
    * @param mapID the mapID of the street
    * @param itemID the itemID of the street
    * @param location the location
    * @param nbrStreetSegmentIDs the number of segments
    * @param streetSegmentsIDs a vector of street segment itemIDs
    * @param streetSegmentItemID a particular segment
    * @param streetSegmentOffset the offset on the segment
    * @param streetNumber the matched street number
    * @param side The side of the street.
    */
   VanillaMatchWithSSI( const char *name,
                        uint32 nameInfo,
                        const char *locationName,
                        uint32 locationNameInfo,
                        uint32 mapID,
                        uint32 itemID,
                        const SearchMatchPoints& points,
                        uint16,
                        const char *alphaSortingName,
                        uint32 location,
                        uint32 streetSegmentItemID,
                        uint16 streetSegmentOffset,
                        uint16 streetNumber,
                        SearchTypes::side_t side,
                        uint8 restrictions,
                        ItemTypes::itemType itemType,
                        uint16 itemSubtype);
   
};

/**
  *   A street match.
  *
  */
class VanillaStreetMatch : public VanillaMatch {
  public:

   /**
    * Constructor
    * @param nbrNames the number of names
    * @param names the names as a vector of char*
    * @param mapID the mapID of the street
    * @param itemID the itemID of the street
    * @param location the location
    * @param nbrStreetSegmentIDs the number of segments
    * @param streetSegmentsIDs a vector of street segment itemIDs
    * @param streetSegmentItemID a particular segment
    * @param streetSegmentOffset the offset on the segment
    * @param streetNumber the matched street number
    * @param side The side of the street.
    */
protected:
   VanillaStreetMatch( const char *name,
                       uint32 nameInfo,
                       const char *locationName,
                       uint32 locationNameInfo,
                       uint32 mapID,
                       uint32 itemID,
                       const SearchMatchPoints& points,
                       uint16,
                       const char *alphaSortingName,
                       uint32 location,
                       uint32 streetSegmentItemID,
                       uint16 streetSegmentOffset,
                       uint16 streetNumber,
                       SearchTypes::side_t side,
                       uint8 restrictions,
                       ItemTypes::itemType itemType,
                       uint16 itemSubtype);
   friend class VanillaSearchReplyPacket;
public:
   /**
    *    Constructor for e.g. servers.
    * 
    */
   VanillaStreetMatch( const IDPair_t& id,
                       const char* name,
                       const char* locationName,
                       uint16 offset,
                       uint32 streetNumber,
                       bool streetNumberFirst = false,
                       bool streetNumberComma = false,
                       uint32 ssiid = MAX_UINT32 ) :
         VanillaMatch ( SEARCH_STREETS,
                        id, name, locationName),
         m_streetSegmentItemID( ssiid ),
         m_streetSegmentOffset( offset ),
         m_streetNumber( streetNumber ),
         m_side( SearchTypes::unknown_side ),
         m_streetNumberFirst( streetNumberFirst ),
         m_streetNumberComma( streetNumberComma )
      {
         if ( ssiid != MAX_UINT32 ) {
            setItemID( ssiid );
            setItemType( ItemTypes::streetSegmentItem );
         } else {
            setItemType( ItemTypes::streetItem );
         }
      }

   /**
    *   Copy constructor that create new strings and these into
    *   this object.
    */
   VanillaStreetMatch(const VanillaStreetMatch& vm);

   /**
    *   Saves the match into the packet.
    *   Note that save saves the type to the packet which is not
    *   read by load, but only by createMatch.
    */
   virtual int save(Packet* packet, int& pos, bool compactForm = false ) const;
   
   /**
    *   Destructor. NB! The array of streetSegmentIDs are deleted
    *   here even if it is created outside this class!
    */
   virtual ~VanillaStreetMatch();

   /**
    * Returns a copy of this.
    *
    * @return A copy of this.
    */
   virtual VanillaMatch* clone() const {
      return new VanillaStreetMatch( *this );
   }


   /**
    * Gets the street segment itemid to route to.
    * @return The street segment itemid to route to.
    */
   virtual uint32 getStreetSegmentID() const {
      return m_streetSegmentItemID;
   }

   /**
    *
    */
   void setStreetSegmentID(uint32 id) {
      m_streetSegmentItemID = id;
   }

   /**
    * @return the offset of the particular segment
    */
   uint16 getOffset() const {
      return m_streetSegmentOffset;
   }

   /**
    *   Sets new offset.
    *   @param offset The new offset.
    */
   void setOffset(uint32 offset) {
      m_streetSegmentOffset = offset;
   }
   
   /**
    *   @return The matched street number.
    */
   uint32 getStreetNbr() const {
      return m_streetNumber;
   }
   
   /**
    *    Sets the housenumber for the match.
    */
   void setStreetNbr(uint32 nbr) {
      m_streetNumber = nbr;
   }

   /**
    * Gets the side of the street.
    * @return The side.
    */
   SearchTypes::side_t getSide() const {
      return m_side;
   }

   /**
    * Sets the side of the street.
    * @return The side.
    */
   void   setSide(SearchTypes::side_t side) {
      m_side = side;
   }

   bool getStreetNumberFirst() const {
      return m_streetNumberFirst;
   }

   void setStreetNumberFirst(bool first) {
      m_streetNumberFirst = first;
   }
   
   bool getStreetNumberComma() const {
      return m_streetNumberComma;
   }

   void setStreetNumberComma(bool comma) {
      m_streetNumberComma = comma;
   }
         
  protected:
   /**
    *   Loads the match from a packet.
    */
   virtual int load(const Packet* packet, int& pos, bool compactForm = false);


   uint32 m_streetSegmentItemID;
   uint16 m_streetSegmentOffset;
   uint16 m_streetNumber;
   SearchTypes::side_t m_side;
   bool m_streetNumberFirst;
   bool m_streetNumberComma;
};

/** 
 *    A region match.
 *
 */
class VanillaRegionMatch : public VanillaMatch {
  public:
   /**
    * Constructor
    * @param name The name as a char*.
    * @param mapID the mapID of the Region
    * @param itemID the itemID of the Region
    */
   VanillaRegionMatch( const char *name,
                       uint32 nameInfo,
                       uint32 mapID,
                       uint32 itemID,
                       uint32 type,
                       const SearchMatchPoints& points,
                       uint16,
                       const char *alphaSortingName,
                       uint32 location,
                       uint8 restrictions,
                       ItemTypes::itemType itemType,
                       uint16 itemSubtype);
   
   /**
    *   Destructor.
    */
   virtual ~VanillaRegionMatch() {}

   /**
    *   Saves the match into the packet.
    *   Note that save saves the type to the packet which is not
    *   read by load, but only by createMatch.
    */
   virtual int save(Packet* packet, int& pos, bool compactForm = false) const;

   
   /**
    *  Returns a copy of this.
    *
    *  @return A copy of this.
    */
   virtual VanillaMatch* clone() const {
      return new VanillaRegionMatch(*this);
   }
   
protected:
   /**
    *   Loads the match from a packet.
    */
   virtual int load(const Packet* packet, int& pos, bool compactForm = false);
};

/**
 *    VanillaBuiltUpAreaMatch.
 *
 */
class VanillaBuiltUpAreaMatch : public VanillaRegionMatch {
  public:
   VanillaBuiltUpAreaMatch( const char *name,
                            uint32 nameInfo,
                            uint32 mapID,
                            uint32 itemID,
                            const SearchMatchPoints& points,
                            uint16,
                            const char *alphaSortingName,
                            uint32 location,
                            uint8 restrictions,
                            ItemTypes::itemType itemType,
                            uint16 itemSubtype);

   virtual ~VanillaBuiltUpAreaMatch() {}
   
   /**
    * Returns a copy of this.
    *
    * @return A copy of this.
    */
   virtual VanillaMatch* clone() const {
      return new VanillaBuiltUpAreaMatch( *this );
   }

};


/**
 * VanillaCountryMatch, holds a simple match name and top region id for country.
 * (It's actually top region, which also includes the states in USA and regions in
 * India for example.)
 * This is kind of special, it does not have any valid item id.
 * The only thing valid about this match is the name and top region.
 */
class VanillaCountryMatch : public VanillaRegionMatch {
public:
   VanillaCountryMatch( const char *name,
                        uint32 topRegionID );

   virtual ~VanillaCountryMatch() {}

   /**
    * Returns a copy of this.
    *
    * @return A copy of this.
    */
   virtual VanillaMatch* clone() const {
      return new VanillaCountryMatch( *this );
   }

   /// @return top region ID
   uint32 getTopRegionID() const {
      return m_topRegionID;
   }

   /**
    *   Loads the match from a packet.
    */
   virtual int load(const Packet* packet, int& pos, bool compactForm = false);
   /**
    *   Saves the match into the packet.
    *   Note that save saves the type to the packet which is not
    *   read by load, but only by createMatch.
    */
   virtual int save(Packet* packet, int& pos, bool compactForm = false ) const;

private:
   uint32 m_topRegionID;
};

/**
 *    VanillaMunicipalMatch.
 *
 */
class VanillaMunicipalMatch : public VanillaRegionMatch {
  public:
   VanillaMunicipalMatch( const char *name,
                          uint32 nameInfo,
                          uint32 mapID,
                          uint32 itemID,
                          const SearchMatchPoints& points,
                          uint16,
                          const char *alphaSortingName,
                          uint32 location,
                          uint8 restrictions,
                          ItemTypes::itemType itemType,
                          uint16 itemSubtype);
   
   virtual ~VanillaMunicipalMatch() {}

   /**
    * Returns a copy of this.
    *
    * @return A copy of this.
    */
   virtual VanillaMatch* clone() const {
      return new VanillaMunicipalMatch( *this );
   }

};


/**
 *    VanillaCityPartMatch.
 *
 */
class VanillaCityPartMatch : public VanillaRegionMatch {
  public:
   VanillaCityPartMatch( const char *name,
                         uint32 nameInfo,
                         uint32 mapID,
                         uint32 itemID,
                         const SearchMatchPoints& points,
                         uint16,
                         const char *alphaSortingName,
                         uint32 location,
                         uint8 restrictions,
                         ItemTypes::itemType itemType,
                         uint16 itemSubtype);
   
   virtual ~VanillaCityPartMatch() {}

   /**
    * Returns a copy of this.
    *
    * @return A copy of this.
    */
   virtual VanillaMatch* clone() const {
      return new VanillaCityPartMatch( *this );
   }

};

/**
 *    VanillaCityPartMatch.
 *
 */
class VanillaZipCodeMatch : public VanillaRegionMatch {
  public:
   VanillaZipCodeMatch( const char *name,
                        uint32 nameInfo,
                        uint32 mapID,
                        uint32 itemID,
                        const SearchMatchPoints& points,
                        uint16,
                        const char *alphaSortingName,
                        uint32 location,
                        uint8 restrictions,
                        ItemTypes::itemType itemType,
                        uint16 itemSubtype);
   
   virtual ~VanillaZipCodeMatch() {}

   /**
    * Returns a copy of this.
    *
    * @return A copy of this.
    */
   virtual VanillaMatch* clone() const {
      return new VanillaZipCodeMatch( *this );
   }

};

/**
 *    VanillaCityPartMatch.
 *
 */
class VanillaZipAreaMatch : public VanillaRegionMatch {
  public:
   VanillaZipAreaMatch( const char *name,
                        uint32 nameInfo,
                        uint32 mapID,
                        uint32 itemID,
                        const SearchMatchPoints& points,
                        uint16,
                        const char *alphaSortingName,
                        uint32 location,
                        uint8 restrictions,
                        ItemTypes::itemType itemType,
                        uint16 itemSubtype);
   
   virtual ~VanillaZipAreaMatch() {}

   /**
    * Returns a copy of this.
    *
    * @return A copy of this.
    */
   virtual VanillaMatch* clone() const {
      return new VanillaZipAreaMatch( *this );
   }
};

/**
  *   A company match.
  *
  */
class VanillaCompanyMatch : public VanillaMatch {
  public:
   /**
    * Constructor
    * @param nbrNames the number of names
    * @param the names as a vector of char*
    * @param mapID the mapId of the company
    * @param itemID the itemID of the company 
    * @param location the location
    * @param streetSegmentID the itemID of the street segment of the
    *                        company
    * @param streetOffset the offset on the street segment
    */
protected:
   VanillaCompanyMatch( const char *name,
                        uint32 nameInfo,
                        const char *locationName,
                        uint32 locationNameInfo,
                        uint32 mapID,
                        uint32 itemID,
                        const SearchMatchPoints& points,
                        uint16,
                        const char *alphaSortingName,
                        uint32 location,
                        uint32 streetSegmentID,
                        uint16 streetOffset,
                        uint16 streetNumber,
                        SearchTypes::side_t side,
                        uint8 restrictions,
                        ItemTypes::itemType itemType,
                        uint16 itemSubtype)
      : VanillaMatch(name, nameInfo, locationName, locationNameInfo,
                     mapID, itemID, SEARCH_COMPANIES,
                     points, 0, alphaSortingName, location,
                     restrictions, itemType, itemSubtype),
      m_streetSegmentID(streetSegmentID),
      m_streetOffset(streetOffset),
      m_streetNumber( streetNumber ),
      m_side(side)
      {
      }
   friend class VanillaSearchReplyPacket;
   friend class SearchableLink;
public:
   /**
    *    Constructor for e.g. servers.
    *    Strings will be copied.
    */
   VanillaCompanyMatch( const IDPair_t& id,
                        const char* name,
                        const char* locationName,
                        uint16 offset,
                        uint32 streetNumber,
                        uint32 streetSegmentID = MAX_UINT32 ) :
         VanillaMatch ( SEARCH_COMPANIES,
                        id, name, locationName),
         m_streetSegmentID( streetSegmentID ),
         m_streetOffset( offset ),
         m_streetNumber( streetNumber ),
         m_side( SearchTypes::unknown_side )
      {
         setItemType( ItemTypes::pointOfInterestItem );
      }
   
   /**
    *   Copy constructor that creates new strings and copies these into
    *   this object.
    */
   VanillaCompanyMatch(const VanillaCompanyMatch& vm):
      VanillaMatch( vm ),
      m_streetSegmentID( vm.m_streetSegmentID ),
      m_streetOffset( vm.m_streetOffset ),
      m_streetNumber( vm.m_streetNumber ),
      m_side( vm.m_side ),
      m_specialImage( vm.m_specialImage ),
      m_categories( vm.m_categories ),
      m_companyName( vm.m_companyName ),
      m_reviews( vm.m_reviews ),
      m_imageUrls( vm.m_imageUrls )
   { }

   /**
    *  
    */
   virtual ~VanillaCompanyMatch()
      {
      }

   /**
    * Returns a copy of this.
    *
    * @return A copy of this.
    */
   virtual VanillaMatch* clone() const {
      return new VanillaCompanyMatch( *this );
   }

   /**
    *   Saves the match into the packet.
    *   Note that save saves the type to the packet which is not
    *   read by load, but only by createMatch.
    */
   virtual int save(Packet* packet, int& pos, bool compactForm = false) const;

   
   /**
    *   @return The street segment item ID.
    */
   uint32 getStreetSegmentID() const {
      return m_streetSegmentID;
   }

   /**
    *   @return The offset on the street segment.
    */
   uint16 getOffset() const {
      return m_streetOffset;
   }

   /**
    *   @return The street number of the company on the street.
    */
   uint32 getStreetNbr() const {
      return m_streetNumber;
   }

   /**
    *    Sets the housenumber for the match.
    */
   void setStreetNbr(uint32 nbr) {
      m_streetNumber = nbr;
   }

   /**
    * Gets the side of the street.
    * @return The side.
    */
   SearchTypes::side_t getSide() const {
      return m_side;
   }

   /**
    * Sets the side of the street.
    * @return The side.
    */
   void setSide(SearchTypes::side_t side) {
      m_side = side;
   }

   /// Sets the unique image to be used with this search hit.
   void setSpecialImage( const MC2String& image ) {
      m_specialImage = image;
   }

   /// @return Unique image to be used with this search hit.
   const MC2String& getSpecialImage() const {
      return m_specialImage;
   }

   typedef vector< uint16 > Categories;
   typedef vector< POIReview > Reviews;
   typedef vector< MC2String > ImageURLs;

   /**
    * Sets the categories this match belongs to.
    * @param categories
    */
   template <typename Container>
   void setCategories( const Container& categories ) {
      m_categories.clear();
      m_categories.insert( m_categories.begin(),
                           categories.begin(), categories.end() );
   }

   /**
    * @return categories this match belongs to.
    */
   const Categories& getCategories() const {
      return m_categories;
   }

   /**
    * Get the name of the company without street address, is empty if no
    * street address is in name.
    */
   const MC2String& getCleanCompanyName() const;

   /**
    * Set the clean company name without street address.
    */
   void setCleanCompanyName( const MC2String& str );

   /**
    * Get the image URLs for this match.
    */
   const ImageURLs& getImageURLs() const;

   /**
    * Get the reviews for this match
    */
   const Reviews& getReviews() const;

   /**
    * Set image URLs by swapping.
    */
   void swapImageURLs( ImageURLs& urls);

   /**
    * Set reviews by swapping
    */
   void swapReviews( Reviews& reviews);

   

protected:
   /**
    *   Loads the match from a packet.
    */
   virtual int load(const Packet* packet, int& pos, bool compactForm = false);


   uint32 m_streetSegmentID;
   uint16 m_streetOffset;
   uint16 m_streetNumber;
   SearchTypes::side_t m_side;

private:
   /// A special image for this company, such as posten or unique hotel
   MC2String m_specialImage;
   Categories m_categories;

   /**
    * The name of the company without street address, is empty if no
    * street address is in name.
    */
   MC2String m_companyName;

   /// List with reviews for this match
   Reviews m_reviews;

   /// List with image URLs for this match
   ImageURLs m_imageUrls;
};

/**
  *   A category match.
  *
  */
class VanillaCategoryMatch : public VanillaMatch {
  public:
   /**
    *   Constructor.
    *   @param   nbrNames The number of names.
    *   @param   names    A vector of char* of names.
    *   @param   mapID    The mapID of the category.
    *   @param   itemID   The itemID of the category.
    */
   VanillaCategoryMatch( const char *name,
                         uint32 nameInfo,
                         const char *locationName,
                         uint32 locationNameInfo,
                         uint32 mapID,
                         uint32 itemID,
                         const SearchMatchPoints& points,
                         uint16,
                         const char *alphaSortingName,
                         uint8 restrictions,
                         ItemTypes::itemType itemType,
                         uint16 itemSubtype)
      : VanillaMatch(name, nameInfo, locationName, locationNameInfo,
                     mapID, itemID, SEARCH_CATEGORIES,
                     points, 0, alphaSortingName,
                     MAX_UINT32 /*location is invalid for categories*/,
                     restrictions, itemType, itemSubtype) 
      {}


   /**
    *    Constructor for e.g. servers.
    * 
    */
   VanillaCategoryMatch ( const IDPair_t& id,
                          const char* name,
                          const char* locationName) :
         VanillaMatch ( SEARCH_CATEGORIES,
                        id, name, locationName)
      {}
   
   /**
    *   Copy constructor.
    */
   VanillaCategoryMatch(const VanillaCategoryMatch& vm)
      : VanillaMatch( vm )
      {
      }

   /**
    *   Destructor (does nothing).
    */
   virtual ~VanillaCategoryMatch()
      {
         // nothing to delete here...
      }

   /**
    * Returns a copy of this.
    *
    * @return A copy of this.
    */
   virtual VanillaMatch* clone() const {
      return new VanillaCategoryMatch( *this );
   }

   /**
    *   Saves the match into the packet.
    *   Note that save saves the type to the packet which is not
    *   read by load, but only by createMatch.
    */
   virtual int save(Packet* packet, int& pos, bool compactForm = false) const;
   
  protected:
   /**
    *   Loads the match from a packet.
    */
   virtual int load(const Packet* packet, int& pos, bool compactForm = false);

};

/**
 *   A misc item match.
 *
 */
class VanillaMiscMatch : public VanillaMatch {
  public:
   /**
    *   Constructor.
    *   @param   nbrNames The number of names.
    *   @param   names    A vector of char* of names.
    *   @param   mapID    The mapID of the category.
    *   @param   itemID   The itemID of the category.
    */
   VanillaMiscMatch( const char *name,
                     uint32 nameInfo,
                     const char *locationName,
                     uint32 locationNameInfo,
                     uint32 mapID,
                     uint32 itemID,
                     const SearchMatchPoints& points,
                     uint16,
                     const char *alphaSortingName,
                     uint32 location,
                     uint32 miscType,
                     uint8 restrictions,
                     ItemTypes::itemType itemType,
                     uint16 itemSubtype)
      : VanillaMatch(name, nameInfo, locationName, locationNameInfo,
                     mapID, itemID, SEARCH_MISC,
                     points, 0, alphaSortingName, location,
                     restrictions, itemType, itemSubtype)
      {
         m_miscType = miscType;
      }

   /**
    *    Constructor for e.g. servers.
    * 
    */
   VanillaMiscMatch ( const IDPair_t& id,
                          const char* name,
                          const char* locationName) :
         VanillaMatch ( SEARCH_MISC,
                        id, name, locationName)
      {}

   /**
    *   Copy constructor.
    */
   VanillaMiscMatch(const VanillaMiscMatch& vm)
      : VanillaMatch( vm )
      {
         m_miscType = vm.m_miscType;
      }

   /**
    *   Destructor (does nothing).
    */
   virtual ~VanillaMiscMatch()
      {
         // nothing to delete here...
      }

   /**
    * Returns a copy of this.
    *
    * @return A copy of this.
    */
   virtual VanillaMatch* clone() const {
      return new VanillaMiscMatch( *this );
   }


   /**
    *   Saves the match into the packet.
    *   Note that save saves the type to the packet which is not
    *   read by load, but only by createMatch.
    */
   virtual int save(Packet* packet, int& pos, bool compactForm = false) const;

   /**
    * Gets the misc type.
    * @return The misc type.
    */
   uint32 getMiscType() const {
      return m_miscType;
   }

   /**
    * Sets the misc type.
    * @return The misc type.
    */
   void setMiscType(uint32 miscType) {
      m_miscType = miscType;
   }

  protected:
   /**
    *   Loads the match from a packet.
    */
   virtual int load(const Packet* packet, int& pos, bool compactForm = false);

   uint32 m_miscType;
};

/**
 *    Link for the matches extracted from a searchreplypacket.
 *
 */
class MatchLink {
  public:
   /**
    * Constructor
    */
   MatchLink( VanillaMatch *vm /*, SortingT *st*/ ) 
      {
         next = NULL;
         myVanillaMatch = vm;
//      sorting = st;
      }

   virtual ~MatchLink() 
      {
         // nothing to delete here. (The SortingType object is the same
         // for all MatchLinks, and is deleted elsewhere.
      }

   /** @return The VanillaMatch associated with this object */
   VanillaMatch *getMatch() const {return myVanillaMatch;}

   /** guess what 
    * @return the next MatchLink of the list.
    */
   MatchLink *getNext() const {return next;}
   
   void setNext( MatchLink *newNext ) {next = newNext;}

   /**
    *   Returns true if this MatchLink is less than the other
    *   one. Should be removed and the merge function in SearchHandler
    *   moved to SearchSorting.
    */
   inline bool isLessThan( const MatchLink *rhs,
                           uint8 sorting ) const;
   
  private:
   MatchLink();
   VanillaMatch *myVanillaMatch;
   MatchLink *next;
};

inline bool MatchLink::isLessThan( const MatchLink *rhs,
                                   uint8 sorting ) const
{
   return SearchSorting::isLessThan(this, rhs,
                                    SearchTypes::SearchSorting(sorting));
}

/// Forw decl.
class OverviewSearchReplyPacket;

/**
 *    Describes one matching map in the overview map.
 *    Contains all matching items for that map with mask and itemID.
 *
 */
class OverviewMatch : public SearchMatch {
  public:
   /// constructor @param mapID the mapID
   OverviewMatch( uint32 mapID,
                  ItemTypes::itemType itemType,
                  uint16 itemSubtype,
                  uint16 source = MAX_UINT16) // For HtmlFunctions
      : SearchMatch( OVERVIEWMATCH,
                     mapID,
                     MAX_UINT32,
                     itemType,
                     itemSubtype,
                     SearchMatchPoints(), // points
                     0, // restrictions
                     source, // Unused
                     MAX_UINT32, // Type set later
                     NULL, // name - set later
                     NULL, // alphaSortingName - set to empty
                     NULL,  // location name - set to empty
                     MAX_UINT32, // Nameinfo not set
                     MAX_UINT32, // Locationnameinfo
                     MAX_UINT32 // Location
                     ), 
      m_overviewID(MAX_UINT32, MAX_UINT32)
      {
         m_id.second      = MAX_UINT32;
         m_name           = NewStrDup::newStrDup("");
         m_locationName   = NewStrDup::newStrDup("");
         m_alphaSortingName = NewStrDup::newStrDup("");
         // Deletestrings is always true here.
         m_deleteStrings  = true;
         m_radiusMeters   = 0;
      }

   explicit OverviewMatch( const IDPair_t& idpair ) 
      : SearchMatch( OVERVIEWMATCH,
                     idpair.getMapID(),
                     MAX_UINT32,
                     ItemTypes::numberOfItemTypes,
                     0, // Subtype
                     SearchMatchPoints(), // points
                     0, // restrictions
                     0, // dbsource?
                     MAX_UINT32, // Type set later
                     NULL, // name - set later
                     NULL, // alphaSortingName - set to empty
                     NULL,  // location name - set to empty
                     MAX_UINT32, // Nameinfo not set
                     MAX_UINT32, // Locationnameinfo
                     MAX_UINT32 // Location
                     ), 
      m_overviewID(MAX_UINT32, MAX_UINT32)
      {
         m_id.second        = idpair.getItemID();
         m_name             = NewStrDup::newStrDup("");
         m_locationName     = NewStrDup::newStrDup("");
         m_alphaSortingName = NewStrDup::newStrDup("");
         // Deletestrings is always true here.
         m_deleteStrings    = true;
         m_radiusMeters     = 0;
      }

   /// Copy contructor
   OverviewMatch( const OverviewMatch& original );

   /**
    *   Constructor to use when creating an OverviewMatch
    *   from another match. (SearchModule).
    *   Some data has to be filled in afterwards, e.g. the
    *   overview id.
    */
   OverviewMatch( const SearchMatch& other );
   
   /// Destructor - everything is done in SearchMatch.
   virtual ~OverviewMatch() {};

   /**
    *   Saves the match into the packet.
    *   Note that save saves the type to the packet which is not
    *   read by load, but only by createMatch.
    */
   virtual int save(Packet* packet, int& pos, bool compactForm = false) const;
      
   /**
    *   Sets the overview map and itemid.
    */
   inline void setOverviewID( const IDPair_t& itemID);

   /**
    *   Returns the overview map id of the match
    *   to be used when looking up coordinates.
    */
   inline uint32 getOverviewMapID() const;

   /**
    *   Returns the overview item id of the match.
    *   To be used when looking up coordinates.
    */
   inline uint32 getOverviewItemID() const;
   
   /**
    *   @return the printable name
    */
   const char *getName0() const { return getName(); }
   
   /**
    *   @param name0 the printable name
    */
   void setName0( const char *name0 ) {
      setName(name0);
   }
   
   /**
    *   Adds <this> to the packet p
    *   @param   p  The packet to be added to.
    */
   virtual void addToPacket( OverviewSearchReplyPacket *p );   

   /**
    *   Returns a new overviewmatch with only the index'th item in it.
    *   Crashes if idx != 0.
    */
   OverviewMatch* getSingleMatch( uint32 index = 0);

   /**
    *   Returns true if the match came from a packet containg
    *   only full matches.
    */
   inline bool fromFullMatchPacket() const;

   /**
    *   Returns true if the match came from a packet containg
    *   only full matches.
    */
   inline bool fromUniqueOrFullMatchPacket() const;

   /**
    *   Set the fullmatchpacket flag. Should be set by the packet.
    */
   inline void setFromFullMatchPacket(bool fullMatchPacket);

   /**
    *   Set unique or full flag. Should be set by the packet.
    */
   inline void setFromUniqueOrFullMatchPacket(bool uniqueOrFullPacket);

   /**
    *   Returns the radius from the center to search in addition
    *   to the items that are logically inside this overview match.
    */
   inline uint32 getRadiusMeters() const;

   /**
    *   Sets the radius.
    */
   inline void setRadiusMeters(uint32 newRadius);

   /**
    *   Returns the number of characters that were removed from the
    *   search string to get this match.
    */
   inline uint8 getNbrRemovedCharacters() const;

   /**
    *   Sets the number of removed characters.
    */
   inline void setNbrRemovedCharacters(uint8 nbrCharacters);
   
protected:
   /**
    *   Loads the match from a packet.
    */
   virtual int load(const Packet* packet, int& pos, bool compactForm = false);

private:
   /// default constructor
   OverviewMatch();
   
   /**
    *   True if the match came from a packet containing only full
    *   matches
   */
   bool m_fullMatchPacket;
   
   /**
    *   True if the match came from a packet containing only
    *   full or unique matches,
    */
   bool m_fullOrUniqueMatchPacket;

   /// ID to be used when looking up coordinates
   IDPair_t m_overviewID;

   /// Radius of some items.
   uint32 m_radiusMeters;

   /// Number of removed characters to get this search match
   uint8 m_nbrRemovedCharacters;
   
};

// ---------- SearchMatch

inline void
SearchMatch::removeRegion( uint32 idx )
{
   RegionMatchVector::iterator it = m_regions.begin();
   it += idx;
   if ( m_regions[idx].second ) {
      delete m_regions[idx].first;
   }
   m_regions.erase(it);
}

inline void
SearchMatch::addRegion( VanillaRegionMatch* region,
                        bool deleteOnDestruction ) 
{
   m_regions.push_back( make_pair( region, deleteOnDestruction ) );
}

// ----------- OverviewMatch
inline void
OverviewMatch::setOverviewID( const IDPair_t& itemID)
{
   m_overviewID = itemID;
}

inline uint32
OverviewMatch::getOverviewMapID() const
{
   return m_overviewID.getMapID();
}

inline uint32
OverviewMatch::getOverviewItemID() const
{
   return m_overviewID.getItemID();
}

inline bool
OverviewMatch::fromFullMatchPacket() const
{
   return m_fullMatchPacket;
}

inline bool
OverviewMatch::fromUniqueOrFullMatchPacket() const
{
   return m_fullOrUniqueMatchPacket;
}

inline void
OverviewMatch::setFromFullMatchPacket( bool fromFullMatchPacket )
{
   m_fullMatchPacket = fromFullMatchPacket;
}

inline void
OverviewMatch::setFromUniqueOrFullMatchPacket(bool fromFullOrUniqueMatchPacket)
{
   m_fullOrUniqueMatchPacket = fromFullOrUniqueMatchPacket;
}

inline uint32
OverviewMatch::getRadiusMeters() const
{
   return m_radiusMeters;
}

inline void
OverviewMatch::setRadiusMeters(uint32 newRadius)
{
   m_radiusMeters = newRadius;
}

inline uint8
OverviewMatch::getNbrRemovedCharacters() const
{
   return m_nbrRemovedCharacters;
}

inline void
OverviewMatch::setNbrRemovedCharacters(uint8 nbrCharacters)
{
   m_nbrRemovedCharacters = nbrCharacters;
}

// ---------- SearchMatchLink

SearchMatch*
SearchMatchLink::getMatch() const {
   return m_match;
}


SearchMatchLink*
SearchMatchLink::getNext() const {
   return m_next;
}


void
SearchMatchLink::setNext( SearchMatchLink* next ) {
   m_next = next;
}

#endif
