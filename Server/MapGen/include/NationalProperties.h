/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NATIONALPROPERTIES_H
#define NATIONALPROPERTIES_H

#include "StringTable.h"
#include "ItemTypes.h"
#include "CharEncoding.h"
#include "MapGenEnums.h"
#include "LangTypes.h"

/**
 * This class contains information of how different countries and
 * map suppliers are treated differently when generating maps.
 *
 */
class NationalProperties {
   public:
      /**
       * Different types of zip codes
       */
      enum zipCodeType_t {
         /** These zip codes can be merged by removing an arbritary number
          * of numbers from the end and check for equality. I.e. the zip
          * are hierarchical when removeing one digit from the end.
          * Example zipcode 22457, 
          * with the property that 22457+22456+22455 etc 
          */
         symmetricNumberZipCodeType,
         
         /** The type of zip codes used in United Kingdom.
          *  The digits and characters in the zip code name are grouped
          *  in a special pattern, not hierarchical in the sense that you 
          *  cannot remove one digit/character from the end to get the 
          *  parental zip code.
          *  UK complete zips (7 digits) have 4 levels:
          *  - postal area
          *  - postal district
          *  - sector
          *  - full zip location
          *  But hierarchical when removing all the digits/chars that are 
          *  part of one level.
          */
         ukZipCodeType,
         
         /** Zip code type with a name followed by a number, as
          *  used in, for example, Ireland. E.g. "Dublin12".
          *  Not hierarchical: cannot remove one digit/character from the end
          *  to get the parental zip code.
          */
         numberNameZipCodeType
      };


      /**
       * Returns what type of zip codes to use depending on country code.
       *
       * @param countryCode The country code of the country to get the 
       *                    zip code type for.
       */
      static zipCodeType_t getZipCodeType(StringTable::countryCode 
                                          countryCode);


      /**
       * This method makes it possbile to use different merging distances
       * in different countries. Call it to get the mergin distance for the
       * country to merge.
       *
       * @param countryCode The country code of the country to get the 
       *                    administrative area merging distance for.
       */
      static uint32 getAdminAreasMergeDist( StringTable::countryCode    
                                            countryCode);
      

      /** This mehtod returns the distance used for merging streets,
       *  depending on the road class. The distance returned has been 
       *  squared.
       *
       * @param countryCode The country code of the country to get the 
       *                    street merging distance for.
       * @param roadClass   The road class to get the street merging 
       *                    distance for.
       * @return The distance to use when merging streets of the road class
       *         of the specified country. If the method failed to find
       *         a merge distance it returns MAX_FLOAT64.
       */
      static float64 getStreetMergeSqDist( StringTable::countryCode    
                                           countryCode, uint32 roadClass);


      /**
       * Tells whether to extend abbreviations of names for a specific
       * item type and country. The reason for extending abbreviations
       * is e.g. that the names of street segments in Tele Atlas US maps
       * all are very abbreviated, making it hard to search with any 
       * word as the extended version.
       */
      static bool extendAbbrevStreetNames( StringTable::countryCode
                                           countryCode,
                                           ItemTypes::itemType itemType );

      /**
       * Returns country specific speed approximatoins for speed levels
       * 0 - 3.
       *
       * @param  countryCode The country code of the country to get speed
       *         approximations for.
       * $return Country specific speed approximation for different speed 
       *         levels.
       */
      static uint16 getSpeedApproximation(StringTable::countryCode
                                          countryCode,
                                          int32 level);

      

      /**
       *    Returns a char encoding object to use when reading names
       *    from the map data of the country of countryCode from the 
       *    map supplier mapOrigin.
       *
       *    @param countryCode Country code of the country to get the 
       *                       char encoding object for.
       *    @param mapOrigin   The map origin of the map data to get the
       *                       char encoding object for.
       *
       *    @return Returns a char encoding object to use when converting 
       *            names in map data of countryCode and mapOrigin to the
       *            char encoding system of MC2. Ownership is left to the
       *            calling method, which should take responsiblility for
       *            its deletion. Returns NULL if no conversion is needed.
       */
      static CharEncoding* getMapToMC2ChEnc( StringTable::countryCode
                                             countryCode,
                                             const char* mapOrigin );


      /**
       *    Returns how much road classes may differ between two forks 
       *    in a bifurcating 3-way crossing for the turn description to the
       *    forks to be keep left and keep right.
       *    @param   countryCode Country code of the country for which
       *                         turn descriptions are calculated.
       */
      static int getMaxRoadClassDiffForBifurc( 
                        StringTable::countryCode countryCode );



      /**
       *     Used for removing short zip codes, which will be doubled by
       *     the zip code overviews in the ovierview map.
       *     @param countryCode The country to check the shortest zip code
       *                        names length to keep in the map 
       */
      static uint32 getZipMinLength(StringTable::countryCode 
                                    countryCode);

      /**
       *     Tells whether to remove non numeric zip code names  or not.
       *     @param countryCode The country to find out whether to remove non
       *            numeric country names or not.
       */
      static bool rmNonNumZipNames(StringTable::countryCode 
                                   countryCode);

      /**
       *     Whether to merge items of one item type, when the items are
       *     close to each other and have the same name
       */
      static bool mergeSameNameCloseItems(
            StringTable::countryCode countryCode,
            const char* mapOrigin,
            ItemTypes::itemType itemType,
            uint32 mapID );


      /**
       *   @name Properties for index areas.
       */
      //@{
      /**
       *     Tells if to use index areas for this country and map provider.
       */
      static bool useIndexAreas(StringTable::countryCode countryCode,
                                const char* mapOrigin);
      
      /**
       * @return Returns true if this is an index area order we want to use
       *         for this country.
       */
      static bool indexAreaToUse(StringTable::countryCode countryCode,
                                 const char* mapOrigin,
                                 uint32 indexAreaOrder);
      //@}

      /**
       *    Whether to use the name of the country to set name of 
       *    buas with no names. For GMSMap::setNamesOnNoNameBuas
       *    Will result in poor names on buas, but if you really 
       *    want to keep no-name buas for nice map display, this is 
       *    one way fo doing it.
       */
      static bool useCountryNameForNoNameBuas(
            StringTable::countryCode countryCode,
            const char* mapOrigin );

      /**
       *    If to remove no-name buas that are located in no-name
       *    municipals.
       */
      static bool removeNoNameBuasInNoNameMunicipals(
            StringTable::countryCode countryCode,
            const char* mapOrigin );

      /**
       *    Whether to remove holes from the item type, with
       *    the eliminateHoles (falukorv method) and the 
       *    eliminateSelfTouch methods.
       *    Be careful which item types you do this for. The elim-methods
       *    are slow for large items with many holes. There will be small
       *    gaps between the small resulting poly parts.
       */
      static bool eliminateHolesAndSelftouch(
            StringTable::countryCode countryCode,
            const char* mapOrigin,
            ItemTypes::itemType itemType );

      /**
       *    Returns the maximum size of waters that are considered when 
       *    computing area feature draw display classes (AFDCC).
       */
      static uint32 AFDDCInteriorItemMaxLength(
            StringTable::countryCode countryCode,
            ItemTypes::areaFeatureDrawDisplayClass_t afddc );

      /**
       *    Returns the minimum size of parks that are considered when 
       *    computing area feature draw display classes (AFDCC).
       */
      static uint32 AFDDCExteriorItemMinLength(
            StringTable::countryCode countryCode,
            ItemTypes::areaFeatureDrawDisplayClass_t afddc );


      /**
       *    Returns a priority for a given AFDDC. Used when setting 
       *    the AFDDC for an item; if the item falls under several AFDDC, use
       *    the one with the highest priority. 
       *
       *    Note that the priorities are chosen with regards to the 
       *    current drawing order. As en example, currently city parks are
       *    drawn after islands. This means, if a water is both on an 
       *    island and inside a city park, we want it to be classified as
       *    as a water-in-city-park, as it would otherwise be overdrawn
       *    by the city park.

       *    @param  type   The AFDDC.
       */
      static int32 AFDDCPriority( 
         ItemTypes::areaFeatureDrawDisplayClass_t type);


 private:
      /**
       * Contains country specific speed limits for different speed levels.
       * Used by getSpeedApproximation.
       */
      static uint16 countrySpeedMatrix[][4];


}; // NationalProperties

#endif
