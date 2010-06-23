/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMTYPES_H
#define ITEMTYPES_H

#include "config.h"
#include "SearchTypes.h"
#include <map>
#include <set>

class LangType;
class StringCode;
class LanguageCode;

#define INVALID_ITEMID MAX_UINT32

/**
  *   Contains types used for and inside the items.
  *
  */
class ItemTypes {
   
  public:

      // ===============================================================
      //                                                     ItemTypes =
      // ===============================================================
            
      /**
       *    The possible types of the Items on the map. This type is 
       *    used to identify the items on the map.
       *    Keep in sync with c_itemTypeToSearchTypeTable
       */
      enum itemType {
         /**
          *    The type of the StreetSegmentItem. Indicates that the 
          *    item is a street segment.
          *    @see StreetSegmentItem
          */
         streetSegmentItem    = 0,

         /**      
          *    An item of type MunicipalItem, in sweden this is called 
          *    "kommun". Municipal in as administrative area, with 
          *    the requirement in MC2 that there must be municipal items
          *    covering all land areas of a country.
          *    @see MunicipalItem
          */
         municipalItem        = 1,

         /**
          *    The type of the WaterItems. Indicates that the item 
          *    describes any kind of water.
          *    @see WaterItem
          */
         waterItem            = 2,

         /**
          *    The type of the ParkItems. 
          *    @see ParkItem
          */
         parkItem             = 3,
        
         /**
          *    The type of the ForestItems. 
          *    @see ForestItem
          */
         forestItem           = 4,
        
         /**
          *    The type of the BuildingItems, typically industrial areas.
          *    @see BuildingItem
          */
         buildingItem         = 5,
        
         /**
          *    The type of the RailwayItems.
          *    @see RailwayItem
          */
         railwayItem          = 6,
        
         /**
          *    The type of the IslandItems.
          *    @see IslandItem
          */
         islandItem           = 7,
        
         /**
          *    The type of the StreetItems. Indicates that the item 
          *    describes a group of street segments that together make 
          *    a street = shared name.
          *    @see StreetItem
          */
         streetItem           = 8,
         
         /**
          *    The type of the NullItems. This item is an item that 
          *    doesn't exist ``for real'' on the map.
          *    @see NullItem
          */
         nullItem             = 9,
         
         /**
          *    The type of the ZipCodeItems, that is an item that 
          *    describes a zip code area (e.g. "22731").
          *    @see ZipCodeItem
          */
         zipCodeItem          = 10,

         /**      
          *    The type of the BuiltUpAreaItems. A built up area is the
          *    same thing as a city of any size, from large metorpolitan 
          *    cities to the smallest rural villages.
          *    @see BuiltUpAreaItem
          */
         builtUpAreaItem      = 11,

         /**
          *    The type of the CityPartItems.
          *    Larger cities are often sub-divided into city parts, 
          *    for better specification of where an address in a city
          *    is located.
          *    @see CityPartItem
          */
         cityPartItem         = 12,

         /**
          *    The type of the zip areas (postal areas), that is an 
          *    item that describes a zip area (e.g. "LUND").
          *    @see ZipAreaItem
          */
         zipAreaItem          = 13,



         // --------------------------------------------------------------
         // These types must have these numbers, since they are used as 
         // index into the itemsWithZoom vector
         
         /**
          *    The type of the PointOfInterestItems. The point of interest
          *    have different POI types depending on the real-world object
          *    they describe. E.g. company, golfCourse or hospital.
          *    @see PointOfInterestItem
          */
         pointOfInterestItem  = 14,

         /**
          *    The type of the CategoryItems. A category is a collection
          *    of companies (PointOfInterests). Compare with the YelloPages.
          *    
          *    The category item is deprecated, no longer used for describing
          *    a collection of POIs.
          *    POI categories are instead defined in 
          *    the poi_category_tree.xml file, and the category(ies) of each 
          *    POI is stored in the mcm map (OldGenericMap) and 
          *    m3 map (GenericMap).
          */
         categoryItem         = 15,

         /**
          *    The type of the RouteableItems. 
          *    Super class to street segments and ferry items, providing 
          *    support for nodes and connections.
          *    @see RouteableItem
          */
         routeableItem        = 16,

         /**
          *    The type of the BusRouteItems. Deprecated - not used.
          *    @see BusRouteItem
          */
         busRouteItem        = 17,

         /**
          *    The type of the FerryItems, ferry lines.
          *    @see FerryItem
          */
         ferryItem        = 18,

         /**
          *    The type of the AirportItems. The airport ground.
          *    @see AirportItem
          */
         airportItem        = 19,

         /**
          *    The type of the AircraftRoadItems.
          *    The runways on airport grounds.
          *    @see AircraftRoadItem
          */
         aircraftRoadItem        = 20,

         /**
          *    The type of the PedestrianAreaItems. Deprecated - not used.
          *    @see PedestrianAreaItem
          */
         pedestrianAreaItem        = 21,

         /**
          *    The type of the MilitaryBaseItems. Deprecated - not used.
          *    @see MilitaryBaseItem
          */
         militaryBaseItem        = 22,

         /**
           *    The type of the IndividualBuildingItems.
           *    Building footprints, outlines.
           *    @see IndividualBuildingItem
           */
         individualBuildingItem   = 23,

          /**
           *    The type of the SubwayLineItems. Deprecated - not used.
           *    @see SubwayLineItem
           */
         subwayLineItem      = 24,

         /// A not used item type, kept in the enum not to break format.
         notUsedItemType = 25,
         
         /// The type of borderItems, item of class Item.
         borderItem = 26,
         
         /**
          *    The type of CartographicItem.
          *    Misc ground items, such as cemeteries, schools, shopping 
          *    centres, golf courses. See ItemSubTypes cartographicType_t
          *    for all possible cartographic item sub types.
          */
         cartographicItem = 27,

         /// The number of item types. Keep last.
         numberOfItemTypes = 28
      };            
      
      /**
       *    Get a item type in printable form.
       *    @param   t  The type that the name should be returned for.
       *    @return StringCode of the itemType in the parameter.
       */
      static StringCode getItemTypeSC(itemType t);

      /**
       *    Returns a pointer into the stringtable to the
       *    itemtype in English ( for debug ).
       *    @param type The itemtype to look for.
       *    @return Pointer to a string in English describing the item.
       */
      static const char* getItemTypeAsString(itemType type);
      
      /**
        *   Used to get the value of the first stringItemType.
        *   To be used in combine with getIncItemTypeSC() as:
        *   @verbatim
            int position = ItemTypes::getFirstItemTypePosition();
            while (position >= 0) {
               cout << position " : ";    // Must be written in own cout
               cout << StringTable::getString(
                           ItemTypes::getIncItemTypeSC(position),
                           StringTable::ENGLISH) 
                    << endl;
            }
            @endverbatim
        *   @return  The position of the first item type.
        */
      static inline int getFirstItemTypePosition();

      /**
       *    Converts the itemType to searchtype for use in
       *    the SearchModule. Should be fast, since it is used
       *    for each item that matches the string when searching.
       *    Items with types that map to zero using this function
       *    will not be added to the SearchMap.
       *    @param theType ItemType.
       *    @return Search type as defined in SearchTypes.h
       */
      static inline uint32 itemTypeToSearchType(ItemTypes::itemType theType);

      /**
       *    Puts the itemTypes that are covered by the searchType.
       *    @param typ        ItemTypes are put here.
       *    @param searchType The searchType.
       */
      static void searchTypeToItemTypes(set<ItemTypes::itemType>& typ,
                                        uint32 searchType);
      
      /**
        *   To be used together with getFirstItemTypePosition() to
        *   print all possible itemTypes.
        *   @param   pos   The position in the array, updated within 
        *                  this method.
        *   @return  The stringcode for item type number pos.
        */
      static StringCode getIncItemTypeSC(int &pos);

      /**
       *    Use the given string to find the item type, ignoring case. 
       *    Will return the first type that is found in any substring 
       *    of typeStr. E.g. "buildingItem" will be returned if typeStr 
       *    is "smallBuildingItem.txt".
       *    @param   typeStr  The string containing the itemtype.
       *    @return  The matching itemtype, if no matching type is found 
       *             numberOfItemTypes is returned.
       */
      static itemType getItemTypeFromString(const char* typeStr);

      // ===============================================================
      //                                                   StringMasks =
      // ===============================================================
      
      /**
        *   @name String-masks
        *   Masks and shifts to get information about the strings.
        */
      //@{
         /**
           *   Use this define as mask to get the language of a string from 
           *   the combinded language, type and stringindex.
           */
         #define STRING_LANGUAGE_MASK 0xFE000000

         /**
          *  Use this define if the string is invalid.
          *  (should be invalidName | invalidLanguage)
          */
         #define INVALID_STRING_INFO  (STRING_LANGUAGE_MASK+STRING_TYPE_MASK)

         /**
           *   The number of steps to shift the combined language, type and
           *   atringindex to get the language. NB! This must be done after
           *   using the mask!
           */
         #define STRING_LANGUAGE_SHIFT 25

         /**
           *   Use this macro to get the language of a given language, type
           *   and stringindex.
           *   @param   a  The combined data.
           *   @return     The language for a.
           */
         #define GET_STRING_LANGUAGE(a) LangTypes::language_t( \
               (STRING_LANGUAGE_MASK & a) >> STRING_LANGUAGE_SHIFT)

         /**
           *   Use this define as mask to get the type of a string from the
           *   combinded language, type and stringindex.
           */
         #define STRING_TYPE_MASK 0x01C00000

         /**
           *   The number of steps to shift the combined language, type and
           *   atringindex to get the type. NB! This must be done after
           *   using the mask!
           */
         #define STRING_TYPE_SHIFT 22

         /**
           *   Use this macro to get the type of a given language, type
           *   and stringindex.
           *   @param   a  The combined data.
           *   @return     The type part of the a.
           */
         #define GET_STRING_TYPE(a) ItemTypes::name_t( \
            (STRING_TYPE_MASK & a) >> STRING_TYPE_SHIFT)

         /**
           *   Use this define as mask to get the string index from the
           *   combinded language, type and stringindex.
           */
         #define STRING_INDEX_MASK 0x003FFFFF
      
         /**
           *   The number of steps to shift the combined language, type and
           *   atringindex to get the sting index. NB! This must be done after
           *   using the mask!
           */
         #define STRING_INDEX_SHIFT 0

         /**
           *   Use this macro to get the string index of a given language, 
           *   type and stringindex.
           *   @param   a  The combined data.
           *   @return     The stringindex part of a.
           */
         #define GET_STRING_INDEX(a) ( (STRING_INDEX_MASK & a) \
                     >> STRING_INDEX_SHIFT)

         /**
           *   Use this macro to get the string info (without the 
           *   string index).
           *   @param   a  The combined data.
           *   @return     The non stringindex part of a.
           */
         #define GET_STRING_INFO(a)  ( ((!STRING_INDEX_MASK) & a) \
            >> STRING_INDEX_SHIFT)
      
         /**
           *   Create a new combined data for the string (language, type and
           *   string index).
           */
         #define CREATE_NEW_NAME(lang, typ, idx) ( (lang << \
            STRING_LANGUAGE_SHIFT) | (typ << STRING_TYPE_SHIFT) | (idx) )
      //@}

      // ===============================================================
      //                                                     NameTypes =
      // ===============================================================
      
      /**
        *   The possible types of the names. Since this is stored in only
        *   tree bits in the items, there must not be more than 8 different
        *   types. Also note that since these are the values that are 
        *   written to disc, new name types must be added at the end of
        *   the list.
        *
        *   @see language_t   The language is specified by the language_t.
        */
      enum name_t {
         /** Indicate that the name is the official name */
         officialName = 0,

         /** Indicate that the name is the alternative name. */
         alternativeName,

         /** Indicate that the name is a roadnumber. */
         roadNumber,

         /** Indicate that the name is invalid. */
         invalidName,

         /** 
           *   Indicate that this name is an abbreviation of the true 
           *   name (probably the official or alternative name). 
           */
         abbreviationName,

         /**
           *   A name of this type is unique within the current location.
           *   E.g. if there is two McDonalds in the same city (located
           *   on the streets A and B) they will have the same location
           *   and the same officialName ("McDonalds"), but different
           *   uniqueName ("McDonals, A" and "McDonalds B").
           */
         uniqueName,
         
         /**
           *   Indicate that this name is an exit number on a road sign that 
           *   can be seen on certain highways.
           */
         exitNumber,

         /**
           *   Indicate that this name is a synonym that probably has 
           *   been manually added to certain item types, to facilitate for
           *   finding the items when searching.
           *   This name should not displayed in case there exist 
           *   any other names with other name types.
           */
         synonymName,
         
         /**
          *  Indicates the end of this list.
          */
         maxDefinedName,
      };

      /**
       *    Get a string that describes this name-type.
       *    @param nameType   The name type to return a string for.
       *    @param shortName  Optional parameter, that if set will make
       *                      this method return a short string.
       */
      static const char* getNameTypeAsString(name_t nameType, 
                                             bool shortName = false);

      /**
       *   Get the type of the name described with a string. 
       *   @param nameType The string describing the name type.
       *   @return         The name type, if no match is found invalidName 
       *                   is returned.
       */
      static ItemTypes::name_t getStringAsNameType(const char* nameType);
   
      /**
       * Convert a StringTable::languageCode into LangTypes::language_t.
       * Short languages are converted into thier long counterparts.
       * Not moved to LangTypes to avoid inclusion of StringTable.
       *
       * @param language The languageCode to convert.
       * @return The language_t matching the languageCode.
			 */
      static LangType getLanguageCodeAsLanguageType( const LanguageCode& language );


      /**
       * Converts a language_t into a StringTable::languageCode.
       * 
       * @param lang The language_t to convert.
       * @return The languageCode for lang, SMSISH_ENG if not supported.
       */
      static LanguageCode getLanguageTypeAsLanguageCode( const LangType& lang );


      // ===============================================================
      //                                             StreetNumberTypes =
      // ===============================================================
      
      /**
        *   Types of enumerating of the houses on a street.
        */
      enum streetNumberType {
         /// There are no numbers on this segment.
         noStreetNumbers         = 0,

         /// The numbers are mixed odd and even on each side.
         mixedStreetNumbers      = 1,

         /**
          *    The numbers on the left side are even. This implicates 
          *    that the numbers on the right side are odd.
          */
         leftEvenStreetNumbers   = 2,

         /**
          *    The numbers on the left side are odd. This implicates 
          *    that the numbers on the right side are even.
          */
         leftOddStreetNumbers    = 3,

         /**
          *    The house numbering is irregular.
          */
         irregularStreetNumbers  = 4
      };

      /**
       *    Get a printable form of one of the street numbering types.
       *    @param   t  The interesting street number type.
       *    @return StringCode of the streetNumberType given as parameter.
       */
      static StringCode getStreetNumberTypeSC(streetNumberType t);

      /**
        *   Used to get the value of the first streetNumberType.
        *   To be used in combine with getIncStreetNumberTypeSC() as
        *   described at the getFirstItemTypePosition()-function.
        *   @return  The position of the first street number type.
        */
      static inline int getFirstStreetNumberTypePosition();

      /**
        *   To be used together with getFirstStreetNumberTypePosition() to
        *   print all possible streetNumberTypes.
        *   @param   pos   In/out parameter that is the position of the
        *                  street number type to return. Updated inside
        *                  this method.
        *   @return  The name of the street number type at the position
        *            given as parameter.
        */
      static StringCode getIncStreetNumberTypeSC(int &pos);


      /**
       *   Calculates the streetNumberType for a street segment item 
       *   given the start and end house numbers on the left and right 
       *   side of the street segment.
       *   @param leftSideStart  The first house number on the left side.
       *   @param leftSideEnd    The last house number on the left side.
       *   @param rightSideStart The first house number on the right side.
       *   @param rightSideEnd   The last house number on the right side.
       *   @return The streetNumberType matching this houseNumbering.
       */
      static streetNumberType getStreetNumberTypeFromHouseNumbering(
         uint16 leftSideStart, uint16 leftSideEnd,
         uint16 rightSideStart, uint16 rightSideEnd );


      // ===============================================================
      //                                          RouteStartDirections =
      // ===============================================================
      /**
       *    The start direction of the route in terms of driving towards
       *    increasing or decreasing housenumbers.
       */
      enum routedir_nbr_t{
         /**
          *    The direction in terms of increasing or decreasing 
          *    housenumbers are unknown (e.g. housenumbers unknown or non
          *    excisting).
          */
         unknown_nbr_t = 0,

         /// The startdirecion goes towards increasing housenumbers.
         increasing = 1,

         /// The startdirecion goes towards decreasing housenumbers.
         decreasing = 2
      };

      /**
       *    Get a printable form of the startdirection.
       *    @param t The start direction of the route.
       *    @return  A string code that will give a printable version of
       *             the start direction of the route.
       */
      static StringCode getStartDirectionHousenumberSC( routedir_nbr_t t);

      /**
       *    The startdirection ot the route in terms of driving with
       *    odd numbers to the left or right.
       */
      enum routedir_oddeven_t{
         /**
          *    The direction can not be explained with odd/even numbers
          *    on one side.
          */
         unknown_oddeven_t = 0,

         /**
          *    The startdirection mean that drive with the odd numbers on
          *    the left side and even numbers on the right.
          */
         leftOddRightEven = 1,

         /**
          *    The startdirection mean that drive with the odd numbers on
          *    the right side and even numbers on the left.
          */
         rightOddLeftEven = 2
      };

      /**
       *    Get a printable form of the startdirection.
       *    @param t The start direction of the route.
       *    @return  A string code that will give a printable version of
       *             the start direction of the route.
       */
      static StringCode getStartDirectionOddEvenSC( routedir_oddeven_t t);
      

      // ===============================================================
      //                                              StreetConditions =
      // ===============================================================

      /**   
        *   The different kinds of condition the street can be in.
        */
      enum streetCondition {
         /// This streetsegment is paved.
         pavedStreet          = 0,

         /// This streetsegment is not paved.   
         unpavedStreet        = 1,

         /// This streetsegment is in poor condition.
         poorConditionStreet  = 2
      };

      /**
       *    Get a printable form of one of the street condition types.
       *    @param   t  The interesting street condition type.
       *    @return StringCode of the streetCondition in the parameter.
       */
      static StringCode getStreetConditionSC(streetCondition t);

      /**
        *   Used to get the value of the first streetCondition.
        *   To be used in combine with getIncStreetConditionTypeSC() as
        *   described at the getFirstItemTypePosition()-function.
        *   @return  The position of the first street condition type.
        */
      static inline int getFirstStreetConditionPosition();

      /**
        *   To be used together with getFirstStreetNumberTypePosition() to
        *   print all possible streetNumberTypes.
        *   @param   pos   In/out parameter that is the position of the
        *                  street condition type to return. Updated inside
        *                  this method.
        *   @return  The name of the street condition at the position
        *            given as parameter.
        */
      static StringCode getIncStreetConditionSC(int &pos);


      // ===============================================================
      //                                             EntryRestrictions =
      // ===============================================================
      
      /**
        *   Enumeration type used to administrate the different 
        *   restrictions that are possible at one node.
        *
        *   @remark This is stored in only two bits in the Nodes,
        *           so if more restrictions should be added, then
        *           the member variable in Node must be changed!
        */
      enum entryrestriction_t {
         /// There are no restrictions when passing this node.
         noRestrictions = 0,

         /** 
          *    Throughfare is not allowed when enter street segment at 
          *    this node.
          */
         noThroughfare  = 1,

         /// It is not allowed to enter the steet segment at this node.
         noEntry        = 2,

         /**
          *    This node is on a one-way street segment, that not is 
          *    allowed to be entered from this node.
          */
         noWay          = 3 
      };

      /**
       *    Get a printable form of one of the entry restriction types.
       *    @param   t  The interesting entry restriction type.
       *    @return StringCode of the entryRestriction in the parameter.
       */
      static StringCode getEntryRestrictionSC( entryrestriction_t t);

      /**
        *   Used to get the value of the first entryRestriction.
        *   To be used in combine with getIncStreetConditionTypeSC() as
        *   described at the getFirstItemTypePosition()-function.
        */
      static inline int getFirstEntryRestrictionPosition();

      /**
        *   To be used together with getFirstEntryRestrictionPosition() to
        *   print all possible streetNumberTypes.
        */
      static StringCode getIncEntryRestrictionSC(int &pos);

      /**
       *    Get the entryrestriction_t that matches a given string. The
       *    string are compared with the text-representation of the 
       *    entry restrictions in the given language and an exact match,
       *    except from the case is required.
       *    @param str  The text representation of the entry restriction
       *                to return.
       *    @param lc   The language of str.
       *    @return  An integer that, if >= 0, is the value of the entry
       *             restriction. A negative value is returned if no
       *             entry restriction matches the given string in the
       *             given language.
       */
      static int getEntryRestriction(const char* str, 
                                      const LanguageCode& lc);

      // ===============================================================
      //                                                   RoadClasses =
      // ===============================================================
      
      /**
        *   The class of the road (streetsegment).
        */
      enum roadClass {
         /** 
          *    This is a main road. 
          */
         mainRoad          = 0,

         /// This is a first class road (similar to NAVTEQ's classification).   
         firstClassRoad    = 1,

         /// This is a second class road (similar to NAVTEQ's classification).
         secondClassRoad   = 2,

         /// This is a third class road (similar to NAVTEQ's classification).
         thirdClassRoad    = 3,

         /// This is a fourth class road (similar to NAVTEQ's classification).
         fourthClassRoad   = 4

      };

      /**
       *    Get a printable form of one of the road class types.
       *    @param   t  The interesting road class type.
       *    @return StringCode of the roadClass in the parameter.
       */
      static StringCode getRoadClassSC(roadClass t);

      /**
        *   Used to get the value of the first roadClass.
        *   To be used in combine with getIncRoadClassSC() as
        *   described at the getFirstItemTypePosition()-function.
        */
      static inline int getFirstRoadClassPosition();

      /**
        *   To be used together with getFirstRoadClassPosition() to
        *   print all possible streetNumberTypes.
        */
      static StringCode getIncRoadClassSC(int &pos);


      // ===============================================================
      //                                                   Speedlimits =
      // ===============================================================
      
      /**
        *   The speed limits of the road (streetsegment). This type is
        *   only used to give the user the speed limit alternatives, 
        *   it is @b not used to store the speed limit in the nodes.
        */
      enum speedLimit {
         /// Used to describe that the speed limit is 10 km/h.
         speed_10 = 0,

         /// Used to describe that the speed limit is 20 km/h.
         speed_20 = 1,

         /// Used to describe that the speed limit is 30 km/h.
         speed_30 = 2,

         /// Used to describe that the speed limit is 50 km/h.
         speed_50 = 3,

         /// Used to describe that the speed limit is 70 km/h.
         speed_70 = 4,

         /// Used to describe that the speed limit is 90 km/h.
         speed_90 = 5,

         /// Used to describe that the speed limit is 110 km/h.
         speed_110= 6,
      };

      /**
       *    Get a printable form of one of the speed limit types.
       *    @param   t  The interesting speed limit type.
       *    @return StringCode of the roadClass in the parameter.
       */
      static StringCode getSpeedLimitSC(speedLimit t);

      /**
        *   Used to get the value of the first roadClass.
        *   To be used in combine with getIncRoadClassSC() as
        *   described at the getFirstItemTypePosition()-function.
        */
      static inline int getFirstSpeedLimitPosition();

      /**
        *   To be used together with getFirstRoadClassPosition() to
        *   print all possible streetNumberTypes.
        */
      static StringCode getIncSpeedLimitSC(int &pos);


      // ===============================================================
      //                                                  JunctionType =
      // ===============================================================
      
      /**
        *   The junction type at the nodes.
        */
      enum junction_t {
         /// This is a normal crossing
         normalCrossing         = 0,

         /**
          *    Indicates that the crossing not is a real crossing, but 
          *    that the street splits up in two. Probably used in combine 
          *    with the "multiple digitalized"-attribute in the street 
          *    segments.
          */
         bifurcation            = 1,

         /// This is a crossing between a street segment and a railway.
         railwayCrossing        = 2,

         /// Border crossing
         borderCrossing         = 3
      };
   
      
      // ===============================================================
      //                                                  VehicleTypes =
      // ===============================================================
      
      /**
        *   The supported vehicletypes. E.g. used to tell what kind of
        *   vehicles that are llowed when entering the street segment.
        *   The values of the types are set so that they could be used 
        *   as masks in bitfileds.
        */
      enum vehicle_t {
         /// Passenger car.
         passengerCar           = 0x00000001,

         // Pedestrian.
         pedestrian             = 0x00000080,

         /// Emergency vehicle.
         emergencyVehicle       = 0x00000020,

         /// Taxi.
         taxi                   = 0x00000010,

         /// Public bus.
         publicBus              = 0x00000004,

         // Delivery truck.
         deliveryTruck          = 0x00001000,

         /// Transport truck.
         transportTruck         = 0x00000002,

         /// Car with more than 2 passengers.
         highOccupancyVehicle   = 0x00000040,

         /// Bicycle.
         bicycle                = 0x00000008,
         
         /// Public transportation.
         publicTransportation   = 0x00100000,
        
         /**
          * Passenger car during closed season.
          * This vehicle type will not be allowed to drive on roads
          * that are closed during winter or only open from dusk till dawn.
          */
         passCarClosedSeason    = 0x00200000,
         
         /**
          * Avoid road toll.
          */
         avoidTollRoad          = 0x00400000,
         
         /**
          * Avoid highway.
          */
         avoidHighway           = 0x00800000, // Last supported vehicle
         
         /// Motor cycle.
         motorcycle             = 0x00002000,

         /// Moped.
         moped                  = 0x00004000,

         /// Private bus.
         privateBus             = 0x00000400,

         /// Military vehicle.
         militaryVehicle        = 0x00000800,
         
         /// A residential vehicle (in swedish "behörig trafik").
         residentialVehicle     = 0x00000100,

         /// Car with trailer.
         carWithTrailer         = 0x00000200,
         
         /// Farm vehicle.
         farmVehicle            = 0x00008000,

         /// private vehicle.
         privateVehicle         = 0x00010000,

         /// Vehicle with water polluting load.
         waterPollutingLoad     = 0x00020000,

         /// Vehicle with explosive load.
         explosiveLoad          = 0x00040000,

         /// Vehicle with dangerous load.
         dangerousLoad          = 0x00080000
      };
      
      /**
       *    Constant indicating the first non supported vehicle type.
       *    By supported means that we actually have maps with
       *    restrictions for these vehicle types. Note that all
       *    supported vehicle types must be listed BEFORE the
       *    unsupported vehicle types.
       */
      static const vehicle_t firstNonSupportedVehicle;

      /**
       *    Returns true if the vechicle is equvalent to a
       *    passengercar. E.g. passengerCar or the car that
       *    will be used for winter roads.
       */
      static inline bool isPassengerCar( vehicle_t t );

      /**
       *    Returns true if the vechicle is equvalent to a
       *    passengercar. E.g. passengerCar or the car that
       *    will be used for winter roads.
       */
      static inline bool isPassengerCar( uint32 t );
      
      /**
       *    Get a printable form of one of the vehicle types.
       *    @param   t  The interesting vehicle type.
       *    @return StringCode of the vehicle given as parameter.
       */
      static StringCode getVehicleSC( vehicle_t t );

      /**
        *   Used to get the value of the first vehicle.
        *   To be used in combine with getIncVehicleSC() as
        *   described at the getFirstItemTypePosition()-function.
        */
      static inline int getFirstVehiclePosition();

      /**
       *    Get the index for one vehicle_t.
       *    @param   t  The interesting vehicle type.
       *    @return  The internal position of the vehicle type given
       *             as parameter.
       */
      static int getVehicleIndex(vehicle_t t);

      /**
        *   To be used together with getFirstRoadClassPosition() to
        *   print all possible Vehicle types.
        *   @param   pos   In/out parameter for the position among the
        *                  vehicle types. Updated insice this method.
        *   @param   allTypes If set to true, all vehicle types will be
        *                     listed, otherwise only the supported types
        *                     will be listed. Is default set to false.
        *   @return  The stringcode for the vehicle type at the given 
        *            position.
        */
      static StringCode getIncVehicleSC( int &pos, 
													  bool allTypes = false );
      

      /**
       *    Tries to match a string with a vehicle type, if no match
       *    defaultType is returned.
       *    @param vehicleType The vehice type string.
       *    @param defaultType The type of vehicle to return if no
       *                       vehicle type matches vehicleType string.
       *    @return A vehicle_t as described above.
       */
      static vehicle_t getVehicleFromString( 
         const char* const vehicleType,
         vehicle_t defaultType = passengerCar );

      /**
       *    State mask to mask out state-node with.
       */
      #define STATE_MASK 0xff000000
      
      /**
       *    Transportation state mask. Used to form a node id together 
       *    (AND) with transportation_t in order to represent a change
       *    of transportation type.
       */
      #define TRANSPORTATION_STATE_MASK 0xf0000000

      /**
       *    Mask to use when checking for additional costs in
       *    route.
       */
      #define ADD_COST_STATE_MASK 0xf1000000
      
      /**
       *    Macro to get the transportation type from a node id
       *    representing a change of transportation type.
       */
      #define GET_TRANSPORTATION_STATE(a) ItemTypes::transportation_t( \
            ( ((a) & STATE_MASK) == TRANSPORTATION_STATE_MASK )\
            ? ((a)&(~(STATE_MASK))) : 0 )  

      /**
       *    Macro to get additional delay in routereplies.
       */
      #define GET_ADDITIONAL_COST_STATE(a) ( \
              (((a) & STATE_MASK) == ADD_COST_STATE_MASK ) ? \
              ( (a) & (~(STATE_MASK))) : 0 )
      
      /**
        *   The possible ways of transportation currently supported
        *   when routing.
        */
      enum transportation_t {
         undefined   = 0,
         drive       = 1,
         walk        = 2,
         bike        = 3,
         bus         = 4,
      };

      /**
       *   Transforms a transportation_t to a vehicle_t.
       *   @param trans The transportation_t to convert.
       *   @return The vehicle_t corresponding trans.
       */
      static vehicle_t transportation2Vehicle( transportation_t trans );


      /**
       *   Transforms a vehicle_t to a transportation_t.
       *   @param vehicleT The vehicle_t to transform.
       *   @return The transportation_t corresponding to the vehicle_t.
       */
      static transportation_t vehicle2Transportation(vehicle_t vehicleT);

      /**
       *   Transforms a uint32 containing the vehicle_t
       *   to a transportation_t.
       *   @param vehicleT The vehicle_t to transform.
       *   @return The transportation_t corresponding to the vehicle_t.
       */
      static transportation_t vehicle2Transportation( uint32 vehicleMask );
      
      // ===============================================================
      //                                 T u r n d e s c r i p t i o n =
      // ===============================================================

      /**
        *   @name    The possible directions of one connection. 
        *   The possible directions of the turn described by this 
        *   connection. This is stored in one byte on disc and 8 bits in 
        *   the memory, so there might not be more than 256 different 
        *   turndirections.
        */
      enum turndirection_t {
         /*
          *    Undefined turndescription. This might be used for turns 
          *    that not is possible to make, not even for emergency 
          *    vehicles (e.g. driv in the wrong way in a roundabout).
          */
         UNDEFINED   = 0,

         /**   
          *    This connection results in a turn to the left.
          */
         LEFT = 1,

         /**   
          *    This connection results in driving straight ahead.
          */
         AHEAD = 2,

         /**   
          *    This connection results in a turn to the left.
          */
         RIGHT = 3,

         /**   
          *    This connection results in a u-turn.
          */
         UTURN = 4,

         /**   
          *    This connection results in follow the same road.
          */
         FOLLOWROAD = 5,

         /**   
          *    This connection results in driving into a roundabout.
          */
         ENTER_ROUNDABOUT = 6,

         /**   
          *    This connection results in driving out from a roundabout.
          */
         EXIT_ROUNDABOUT = 7,

         /**
           *   This connection results in driving right at the roundabout.
           *   It is not possible to do this if driving on the left side
           *   of the road.
           */
         RIGHT_ROUNDABOUT = 8,

         /**
           *   This connection results in driving left at the roundabout.
           *   It is not possible to do this if driving on the right side
           *   of the road.
           */
         LEFT_ROUNDABOUT = 9,

         /**
           *   Enter the highway at this ramp.
           */
         ON_RAMP = 10,

         /**
           *   Exit the highway at this ramp.
           */
         OFF_RAMP = 11,

         /**
           *   Enter bus.
           */
         ENTER_BUS = 12,

         /**
           *   Exit bus.
           */
         EXIT_BUS = 13,

         /**
           *   Change bus.
           */
         CHANGE_BUS = 14,

         /**
           *   Hold to the right. (Less than turn right.)
           */
         KEEP_RIGHT = 15,

         /**
           *   Hold to the left. (Less than turn left.)
           */
         KEEP_LEFT = 16,

         /**
          * Drive onto a ferry.
          */ 
         ENTER_FERRY = 17,

         /**
          * Leave a ferry.
          */
         EXIT_FERRY = 18,

         /**
          * Leave one ferry and enter an other one.
          */
         CHANGE_FERRY = 19,

         /**
          * Use an exit ramp on the left side of the road.
          */
         OFF_RAMP_LEFT = 20,

         /**
          * Use an exit ramp on the left side of the road.
          */
         OFF_RAMP_RIGHT = 21,

         /**
          * Multi connection turn.
          */
         MULTI_CONNECTION_TURN = 22,

         /**
          *  Turndescription for mapfixes. To use when a roundabout is
          *  a normal crossing in the map.
          */
         AHEAD_ROUNDABOUT = 23, 
         
         // Possible new directions to add:
         // FOLLOW_RIGHT
         // FOLLOW_LEFT
         // KEEP_MIDDLE
         // AHEAD_KEEP_LEFT
         // AHEAD_KEEP_RIGHT
         // AHEAD_KEEP_MIDDLE
      };

      /**
       *    Get the string code for one turndirection_t.
       *    @param  t The turndescription to get the string code for.
       *    @return The string code for the given turndescription.
       */
      static StringCode getTurndirectionSC(turndirection_t t);

      /**
        *   Used to get the value of the turning direction.
        *   To be used in combine with getIncTurnDirectionSC() as
        *   described at the getFirstItemTypePosition()-function.
        *   @return  The initial value of the position.
        */
      static inline int getFirstTurnDirectionPosition();

      /**
        *   To be used together with getFirstTurnDirectionPosition() to
        *   print all possible turn directions.
        *   @param   pos   In/out parameter for the position among the
        *                  turn direction types. Updated inside this 
        *                  method. Set to a negative value when all
        *                  turn-directions are returned.
        *   @return  The stringcode for the turn direction type at the 
        *            given position.
        */
      static StringCode getIncTurnDirectionSC(int &pos);

      /**
       *    Get the turndirection_t that matches a given string. The
       *    string are compared with the text-representation of the 
       *    entry restrictions in the given language and an exact match,
       *    except from the case, is required.
       *    @param str  The text representation of the turn direction
       *                to return.
       *    @param lc   The language of str.
       *    @return  An integer that, if >= 0, is the value of the turn
       *             direction. A negative value is returned if no
       *             turn direction matches the given string in the
       *             given language.
       */
      static int getTurnDirection(const char* str, 
                                  const LanguageCode& lc);

      
      // ===============================================================
      //                                       C r o s s i n g k i n d =
      // ===============================================================

      /**
        *   The possible kinds of crossing. Stored in one byte on disc
        *   and in memory.
        */
      enum crossingkind_t {
         /** 
          *    This crossing is not defined. 
          */
         UNDEFINED_CROSSING = 0,

         /** 
          *    There is no crossing at this connection "Follow the road". 
          */
         NO_CROSSING = 1,

         /** 
          *    Three street crossing that looks lika a "T".
          */
         CROSSING_3WAYS_T = 2,

         /** 
          *    Three street crossing that looks lika a "Y".
          */
         CROSSING_3WAYS_Y = 3,

         /** 
          *    4 street crossing.
          */
         CROSSING_4WAYS = 4,

         /** 
          *    5 street crossing.
          */
         CROSSING_5WAYS = 5,
 
         /** 
          *    6 street crossing.
          */
         CROSSING_6WAYS = 6,

         /** 
          *    7 street crossing.
          */
         CROSSING_7WAYS = 7,

         /** 
          *    8 street crossing.
          */
         CROSSING_8WAYS = 8,

         /** 
          *    Roundabout with two exits.
          */
         CROSSING_2ROUNDABOUT =  9,

         /** 
          *    Roundabout with three exits.
          */
         CROSSING_3ROUNDABOUT = 10,

         /** 
          *    Symmetric roundabout with four exits.
          */
         CROSSING_4ROUNDABOUT = 11,

         /** 
          *    Asymmetric roundabout with four exits.
          */
         CROSSING_4ROUNDABOUT_ASYMMETRIC = 12,

         /** 
          *    Roundabout with five exits.
          */
         CROSSING_5ROUNDABOUT = 13,

         /** 
          *    Roundabout with six exits.
          */
         CROSSING_6ROUNDABOUT = 14,

         /** 
          *    Roundabout with seven or more exits.
          */
         CROSSING_7ROUNDABOUT = 15,

      };

      /**
       *    Get a printable form of one of the crossing kind types.
       *    @param  t   The interesting crossing kind type.
       *    @return StringCode of the crossing kind in the parameter.
       */
      static StringCode getCrossingKindSC(crossingkind_t t);

      /**
       *   Used to get the value of the turning direction.
       *   To be used in combine with getIncTurnDirectionSC() as
       *   described at the getFirstItemTypePosition()-function.
       *   @return   The position of the first crossing kind type.
       */
      static inline int getFirstCrossingKindPosition();

      /**
        *   To be used together with getFirstTurnDirection() to
        *   print all possible kinds of crossings.
        *   @param   pos   In/out parameter for the position among the
        *                  crossing kind types. Updated inside this 
        *                  method. pos will be set to a negative value 
        *                  when all crossingkinds are returned.
        *   @return  The stringcode for the crossing kind type at the 
        *            given position.
        */
      static StringCode getIncCrossingKindSC(int &pos);
    
      // ===============================================================
      //                                                     waterType =
      // ===============================================================
      
      /**
        *   The water type for the WaterItems.
        */
      enum waterType {
         /**
           *   Ocean.
           *   Waters with this type are not kept/used in the mcm map.
           */
         ocean = 0,

         /// Lake.
         lake,

         /// River.
         river,

         /// Canal.
         canal,

         /// Harbour / Port.
         harbour,

         /**
           *   Other water element type - TA. 
           *   Waters with this type are not kept/used in the mcm map.
           */
         otherWaterElement,

         /// No type or unknown
         unknownWaterElement
      
      };

      /**
        *   Get the waterType from a string, case is ignored.
        *   @param   waterTypeStr   The string describing the water type.
        *   @return  An integer that, if >= 0, is the value of the water 
        *            type. A negative value is returned if no matching 
        *            type was found for the string.
        */
      static int getWaterTypeFromString(const char* waterTypeStr);
      
      /**
        *   Get a string from the waterType.
        *   @param   waterType   The waterType.
        *   @return  A string for the water type.
        */
      static const char* getWaterTypeAsString(waterType type);

      // ===============================================================
      //                                                      parkType =
      // ===============================================================
      
      /**
        *   The type of parks.
        */
      enum parkType {
         /// City park.
         cityPark = 0,

         /// Region or national park.
         regionOrNationalPark
      
      };

      /**
        *   Get the parkType from a string, case is ignored.
        *   @param   parkTypeStr The string describing the parktype.
        *   @return  An integer that, if >= 0, is the value of the 
        *            park type. A negative value is returned if no 
        *            matching type was found from the string.
        */
      static int getParkTypeFromString(const char* parkTypeStr);
      
      // ===============================================================
      //                                                     ferryType =
      // ===============================================================
      
      /**
        *   The type of ferries.
        */
      enum ferryType {
         /// Operated by ship.
         operatedByShip = 0,

         /// Operated by train.
         operatedByTrain
      
      };

      /**
        *   Get the ferryType from a string, case is ignored.
        *   @param   ferryTypeStr The string describing the ferrytype.
        *   @return  An integer that, if >= 0, is the value of the 
        *            ferry type. A negative value is returned if no 
        *            matching type was found from the string.
        */
      static int getFerryTypeFromString(const char* ferryTypeStr);
      
      // ===============================================================
      //                                             pointOfInterest_t =
      // ===============================================================
      
      /**
        *   The type of a point of interest. This is stored in a 
        *   singel byte on disc (so there must not be more than 255 
        *   alternatives in this enumeration).
        */
      enum pointOfInterest_t {
         /// This point of interest is a company
         company = 0,

         airport = 1,
         amusementPark = 2,
         atm = 3,
         automobileDealership = 4,
         bank = 5,
         bowlingCentre = 6,
         busStation = 7, // bus terminal
         businessFacility = 8,
         casino = 9,
         cinema = 10,
         cityCentre = 11,
         cityHall = 12,
         communityCentre = 13,
         commuterRailStation = 14, // (sub) urban rail
         courtHouse = 15,
         exhibitionOrConferenceCentre = 16,
         ferryTerminal = 17,
         frontierCrossing = 18,
         golfCourse = 19,
         groceryStore = 20,
         historicalMonument = 21,
         hospital = 22,
         hotel = 23,
         iceSkatingRink = 24,
         library = 25,
         marina = 26,
         motoringOrganisationOffice = 27,
         museum = 28,
         nightlife = 29,
         openParkingArea = 30,
         parkAndRide = 31,
         parkingGarage = 32,
         petrolStation = 33,
         policeStation = 34,
         publicSportAirport = 35,
         railwayStation = 36, // international, national railway stations
                              // compare with commuterRailStation, subwayStation,
                              // tramStation
         recreationFacility = 37,
         rentACarFacility = 38,
         restArea = 39,
         restaurant = 40,
         school = 41,
         shoppingCentre = 42,
         skiResort = 43,
         sportsActivity = 44,
         sportsCentre = 45,
         theatre = 46,
         touristAttraction = 47,
         touristOffice = 48,
         university = 49,
         vehicleRepairFacility = 50,
         winery = 51,
         postOffice = 52,
         tramStation = 53, // tram-stops
         multi = 54, // deprecated
         // Unused 55
         shop = 56,
         cemetery = 57,
         industrialComplex = 58,
         publicIndividualBuilding = 59,   // not used
         otherIndividualBuilding = 60,    // not used
         notCategorised = 61, // deprecated
         unknownType = 62, // type of the POI is unknown
         airlineAccess = 63,
         beach = 64,
         campingGround = 65,
         carDealer = 66,
         concertHall = 67,
         tollRoad = 68,
         culturalCentre = 69,
         dentist = 70,
         doctor = 71,
         driveThroughBottleShop = 72,
         embassy = 73,
         entryPoint = 74,
         governmentOffice = 75,
         mountainPass = 76,
         mountainPeak = 77,
         musicCentre = 78,
         opera = 79,
         parkAndRecreationArea = 80,
         pharmacy = 81,
         placeOfWorship = 82, // Generic place of worship, 
                              // other than: church, mosque, synagogue, 
                              //             hinduTemple and buddhistSite
         rentACarParking = 83,
         restaurantArea = 84,
         scenicView = 85,
         stadium = 86,
         swimmingPool = 87,
         tennisCourt = 88,
         vetrinarian = 89,
         waterSports = 90,
         yachtBasin = 91,
         zoo = 92,
         wlan = 93,
         noType = 94,   // you know what the POI is, but there is no other
                        // appropriate type to use.
         invalidPOIType = 95, // marker for error type
         church = 96,
         mosque = 97,
         synagogue = 98,
         subwayStation = 99,  // subway, metro, underground
         cafe = 100,
         hinduTemple = 101,
         buddhistSite = 102,
         busStop = 103, // bus stop, not bus station/terminal
         taxiStop = 104, // where you can expect to find taxis waiting for clients

         nbr_pointOfInterest
      };

      /**
       *
       */
      static StringCode getPOIStringCode(pointOfInterest_t t);

      
      // ===============================================================
      //                                        B u i l t U p A r e a =
      // ===============================================================
      
      /**
       *
       */
      enum city_t {
         CAPITAL_CITY,
         BIG_CITY,
         MEDIUM_CITY,
         SMALL_CITY,
         VILLAGE_CITY
      };
      
    
      // ===============================================================
      //                                             L a n d m a r k s =
      // ===============================================================
      
      /**
        *   The different landmark types we can have in the map.
        */
      enum landmark_t {
         builtUpAreaLM,
         railwayLM,
         areaLM,
         poiLM,
         signPostLM,
         countryLM,
         countryAndBuiltUpAreaLM,
         passedStreetLM,
         accidentLM,
         roadWorkLM,
         cameraLM,
         speedTrapLM,
         policeLM,
         weatherLM,
         trafficOtherLM,
         blackSpotLM,
         userDefinedCameraLM,
      };

      /**
        *   The location of the landmark relative to the crossing 
        *   or routeable item it is connected to.
        */
      enum landmarklocation_t {
         /** Indicates that the crossing is after the landmark (you 
           * should turn after the landmark). */
         after,

         /** Indicates that the crossing is before the landmark. */
         before,

         /** Indicates that the crossing is in the landmark 
           * (e.g. a park). */
         in,
         
         /** Indicates that the crossing is at the landmark (if 
           * after, before or in does not fit the location). */
         at,

         /** Indicates that you should pass the landmark (when 
           * you drive ahead in a crossing or when the landmark 
           * is located just beside a road). */
         pass,

         /** Indicates that the crossing is in the landmark, 
          *  and you want description "after xx meters go into..". 
          *  The landmark culd be e.g. a built-up area or a country. */
         into,
         
         /** Indicates that the crossing is where you arrive 
          *  at the landmark (from e.g. ferryitem). */
         arrive,
         
         /** Initial value.*/
         undefinedlocation
      };
   
      /**
        *   Structure to hold the information about landmarks when used 
        *   in the map's landmark table and when expanding the route.
        */
      struct lmdescription_t {
            // The id of the item representing the landmark,
            // e.g.  a built-up area, a railway or an individual building
            uint32 itemID;
            // is a measure of how important the landmark is. The scale is
            // from 0-4, where 0 is most important and 4 is least important
            uint32 importance;
            // describes on which side of a road/crossing the landmark is 
            // located, to the right or left. A special case is when the 
            // road/crossing is located in the landmark, when driving 
            // straight ahead, the description will be to pass through.
            SearchTypes::side_t side;
            // describes how the crossing/road is located in relation to 
            // the landmark,
            // e.g. that you should turn after, before, in or at the landmark.
            // Another alternative is to pass the landmark.
            landmarklocation_t location;
            // gives what kind of landmark is it.
            landmark_t type;
      };
     
      /**
        *   Get the landmark location from a string, case is ignored.
        *   @param   str   The string describing the landmark location.
        *   @return  An integer that if >=0 is the value of the 
        *            landmark location. A negative value us returned
        *            if no matching type was found from the string.
        */
      static int getLandmarkLocationFromString(const char* str);
      
      /**
        *   Get the stringCodes for the landmark locations.
        */
      static StringCode getLandmarkLocationSC( landmarklocation_t t);

      /**
        *   Get the landmark side from a string, case is ignored.
        *   @param   str   The string describing the landmark side.
        *   @return  An integer that if >=0 is the value of the
        *            landmark side. A negative value us returned if
        *            no matching type was found from the string.
        */
      static int getLandmarkSideFromString(const char*);
      
      /**
        *   Get the stringCodes for the landmark side.
        */
      static StringCode getLandmarkSideSC(SearchTypes::side_t t);
      
      // ===============================================================
      //                                                       M I S C =
      // ===============================================================
      

      /**
       *    Zoomlevels. These can be used as masks aswell.
       */
      static const uint32 zoomlevelVect[];
   
      /**
       *    Static constant for CityPartItem zoomlevel.
       */
      static const uint32 zoomConstCPI;
      
      /**
       *    PointsOfInterest zoomlevel.
       */
      static const uint32 poiiZoomLevel;

      /**
       * The different types of traffic disturbances.
       */
      enum disturbance_t {
         accident = 0,
         roadWork,
         trafficMessage,
         levelOfService,
         obstructionHazards,
         restrictions,
         fogSmokeDust,
         wind,
         exhaustPollution,
         incident,
         movingHazards,
         skidHazards,
         snowOnTheRoad,
         operatorActivities,
         activities,
         other,
         unknown
      };

      /**
       *  The severiry at a disturbance.
       */
      enum severity_t {
		   blocked = 0,
         extremelySevere,
         verySevere,
         severe,
         lowSeverity,
         lowestSeverity,
         notProvided,
         NBR_SEVERITY_T
      };
         

      /**
       * Used to create a byte with the days of the week
       * a disturbance is active,
       */
      enum weekDays_t {
         monday = 0x1,
         tuesday = 0x2,
         wednesday = 0x4,
         thursday = 0x8,
         friday = 0x10,
         saturday = 0x20,
         sunday = 0x40,
         allweek = 0x80
      };

      /**
       *    Display class for roads.
       *    Support for drawing a street segment different from 
       *    the default setting depeding on road class of the street segment.
       */
      enum roadDisplayClass_t {
         partOfWalkway = 0,
         partOfPedestrianZone,
         roadForAuthorities,
         entranceExitCarPark,
         etaParkingGarage,
         etaParkingPlace,
         etaUnstructTrafficSquare,
         partOfServiceRoad,
         roadUnderContruction,
         
         // If adding more disp clases, tell server team so they can
         // define drawsettings
         
         // keep last
         nbrRoadDisplayClasses
      };

      static const char* 
         getStringForRoadDisplayClass( roadDisplayClass_t dpClass );
   
      /**
       * Display class for area features. It is used to deviate from
       * the normal drawing order, to display items that disappear within
       * holes of other items.
       */
      enum areaFeatureDrawDisplayClass_t {
         waterInCityPark = 0,
         waterInCartographic,
         waterInBuilding,  // BuldingItem i.e. Industrial areas
         waterOnIsland,
         islandInBua,
         cartographicInCityPark,
         cartographicInForest,
         buaOnIsland,
         IIWIPOutsideParkOutsideBua, //IIWIP = Island in water in park
         IIWIPOutsideParkInsideBua,
         IIWIPInsidePark,

         // If adding more disp clases, tell server team so they can
         // define drawsettings. If new itemType is affected make sure
         // that it is added properly to m3 maps. (see buaOnIsland)
         
         // keep last
         nbrAreaFeatureDrawDisplayClasses
      };
      
      static const char* 
         getStringForAreaFeatureDrawDisplayClass( 
               areaFeatureDrawDisplayClass_t dpClass );


   private:

      /**
       *  Table containing translation from itemType to
       *  SearchType (mask).
       */
      static const uint32
         c_itemTypeToSearchTypeTable[];


};

// =======================================================================
//                                     Implementation of inlined methods =

inline int 
ItemTypes::getFirstItemTypePosition() 
{
   return (0);
}

inline int 
ItemTypes::getFirstStreetNumberTypePosition() 
{
   return (0);
}


inline int 
ItemTypes::getFirstStreetConditionPosition() 
{
   return (0);
}

inline int 
ItemTypes::getFirstEntryRestrictionPosition() 
{
   return (0);
}

inline int 
ItemTypes::getFirstRoadClassPosition() 
{
   return (0);
}

inline int 
ItemTypes::getFirstSpeedLimitPosition() 
{
   return (0);
}

inline int 
ItemTypes::getFirstVehiclePosition() 
{
   return (0x00000001);
}

inline int
ItemTypes::getFirstTurnDirectionPosition() 
{
   return turndirection_t(0);
}

inline int
ItemTypes::getFirstCrossingKindPosition() 
{
   return crossingkind_t(0);
}

//inline int 
//ItemTypes::getFirstMapUpdatePosition()
//{
//   return 0;
//}


inline bool
ItemTypes::isPassengerCar(ItemTypes::vehicle_t t )
{
   switch ( t ) {
      case ( ItemTypes::passengerCar ) :
      case ( ItemTypes::passCarClosedSeason ) :
         return true;
         break;
      default:
         return false;
         break;
   }
}

inline bool
ItemTypes::isPassengerCar(uint32 t)
{
   return isPassengerCar( ItemTypes::vehicle_t(t) );
}

inline uint32
ItemTypes::itemTypeToSearchType(ItemTypes::itemType theType)
{
   return c_itemTypeToSearchTypeTable[theType];
}

#endif

