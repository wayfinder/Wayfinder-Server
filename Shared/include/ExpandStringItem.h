/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXPAND_STRINH_ITEM
#define EXPAND_STRINH_ITEM

#include "config.h"
#include "StringUtility.h"
#include "StringTableUtility.h"
#include "SMSStringUtility.h"
#include "StringTable.h"
#include "ExpandStringLane.h"
#include "ExpandStringSignPost.h"

#include "ItemTypes.h"

#include "NotCopyable.h"

class LandmarkHead;

class DescProps 
{
  public:
   
   DescProps(StringTable::languageCode m_lang,
             byte maxTurnNumber,
             StringTableUtility::distanceFormat distFormat,
             StringTableUtility::distanceUnit distUnit,
             bool printSignPostText,
             bool wapFormat,
             bool compassLangShort,
             byte houseNbrStart);
   
   
   StringTable::languageCode getLanguage() const;
   
   byte getMaxTurnNumber() const;

   StringTableUtility::distanceFormat getDistFormat() const;
   StringTableUtility::distanceUnit getDistUnit() const;

   bool getPrintSignPostText() const;

   bool getWapFormat() const;
   bool getCompassLangShort() const;
   byte getHouseNbrStart() const;
  
  protected:   
   StringTable::languageCode m_lang;

   byte m_maxTurnNumber;
   StringTableUtility::distanceFormat m_distFormat;
   StringTableUtility::distanceUnit m_distUnit;
   bool m_printSignPostText; 
   bool m_wapFormat;
   bool m_compassLangShort;
   byte m_houseNbrStart;
   
};



/**
  *   The route in text-form is inserted into a list of 
  *   ExpandStringItems that is returned to the caller
  *   of the getStringDataItem()-method.
  *
  *   @see ExpandRouteReplyPacket For packet format.
  *
  */
class ExpandStringItem: private NotCopyable {
   public: 
      
   
   
      /**
        *   @name Stringcode masks
        *   Masks used with the stringcode-field
        */
      //@{
         /** 
           *   Get the StringCode from the  stringcode field in the 
           *   packet.
           */
         #define STRINGCODE_MASK 0x00FFFFFF

         /** 
           *   Get the StringCode from the  stringcode field in the 
           *   packet.
           */
         #define EXITCOUNT_MASK 0xFF000000
      //@}

      /**
        *   @name Masks, shifts and macros 
        *   Masks, shifts and macros that are used for extracting the 
        *   bitfield that describes the properties of the route 
        *   descriptions.
        */
      //@{
         /** 
           *   Language code mask. 4 bits.
           */
         #define LANGUAGE_CODE_MASK 0x0000000F

         /**
           *   Language code shift.
           */
         #define LANGUAGE_CODE_SHIFT 0

         /**
           *   Macro used to get the language code used in the route
           *   description from the combined data.
           *   @param   a  The combined data.
           *   @return  The language code.
           */
         #define GET_LANGUAGE_CODE(a) StringTable::languageCode( \
            (LANGUAGE_CODE_MASK & a) >> LANGUAGE_CODE_SHIFT)

         /** 
           *   Maximum number of turn number mask. 4 bits.
           */
         #define MAX_TURN_NUMBER_MASK 0x000000F0

         /**
           *   Maximum number of turn number shift.
           */
         #define MAX_TURN_NUMBER_SHIFT 4

         /**
           *   Macro used to get the maximum number of turn numbers
           *   that should be displayed in the route description.
           *   @param   a  The combined data.
           *   @return  The maximum number of turn numbers that should be
           *            displayed.
           */
         #define GET_MAX_TURN_NUMBER(a) (MAX_TURN_NUMBER_MASK & a) \
            >> MAX_TURN_NUMBER_SHIFT
         
         /** 
           *   Signpost text mask. 1 bit.
           */
         #define SIGNPOST_TEXT_MASK 0x00000100

         /** 
           *   Signpost text shift.
           */
         #define SIGNPOST_TEXT_SHIFT 8

         /**
           *   Macro used to know if the signpost text should be displayed
           *   or not.
           *   @param   a  The combined data.
           *   @return  True if the signposttext should be displayed.
           */
         #define PRINT_SIGNPOST_TEXT(a) \
            (SIGNPOST_TEXT_MASK & a) >> SIGNPOST_TEXT_SHIFT

         /** 
           *   Distance format mask. 2 bit.
           */
         #define DISTANCE_FORMAT_MASK 0x00001800
         
         /** 
           *   Distance format shift.
           */
         #define DISTANCE_FORMAT_SHIFT 11
         
         /**
           *   Macro used to get the distance format
           *   @param   a  The combined data.
           *   @return  The distance format
           */
         #define GET_DISTANCE_FORMAT(a) StringTableUtility::distanceFormat( \
            (DISTANCE_FORMAT_MASK & a) >> DISTANCE_FORMAT_SHIFT )
         
         /** 
           *   Wap format mask. 1 bit.
           */
         #define WAP_FORMAT_MASK 0x00002000

         /**
           *   Wap format shift.
           */
         #define WAP_FORMAT_SHIFT 13
         
         /**
           *   Macro used to know if the route description should be
           *   wap formatted or not.
           *   @param   a  The combined data.
           *   @return  True if the route description should be
           *            wap formatted.
           */
         #define WAP_FORMAT(a) \
            ((WAP_FORMAT_MASK & a) >> WAP_FORMAT_SHIFT) != 0
         
         /** 
           *   Compass Short Language code mask. 1 bit.
           */
         #define COMPASS_LANGUAGE_SHORT_MASK 0x00004000

         /**
           *   Language code shift.
           */
         #define COMPASS_LANGUAGE_SHORT_SHIFT 14

         /**
           *   Macro used to know if the language code used in the compass
           *   direction should be shortformated.
           *   @param   a  The combined data.
           *   @return  True if the languagecode should be short in the 
           *            compass direction.
           */
         #define COMPASS_LANGUAGE_SHORT(a) \
            ((COMPASS_LANGUAGE_SHORT_MASK & a) \
             >> COMPASS_LANGUAGE_SHORT_SHIFT)

         /** 
           *   Start housenumber direction code mask. 3 bit.
           *   The eight values are:
           *   0 - unknown_oddeven_t
           *   1 - leftOddRightEven
           *   2 - rightOddLeftEven
           *   3 - unknown_nbr_t
           *   4 - increasing
           *   5 - decreasing
           *   6 - reserved
           *   7 - reserved
           */
         #define START_HOUSENUMBER_DIR_MASK 0x00038000

         /**
           *   Start housenumber direction code shift.
           */
         #define START_HOUSENUMBER_DIR_SHIFT 15

         /**
           *   Macro used to know if the language code used in the compass
           *   direction should be shortformated.
           *   @param   a  The combined data.
           *   @return  Value see START_HOUSENUMBER_DIR_MASK for
           *            interpretation of values.
           */
         #define START_HOUSENUMBER_DIR(a) \
            ((START_HOUSENUMBER_DIR_MASK & a) \
             >> START_HOUSENUMBER_DIR_SHIFT)

      //@}
      

      /**
       *   Constructor that sets all membervariables.
       */
      ExpandStringItem( uint32 dist,
                        uint32 time,
                        uint32 stringCode,
                        const char* str,
                        ItemTypes::transportation_t transport,
                        int32 lat,
                        int32 lon,
                        byte nameType,
                        byte turnNumber,
                        const ExpandStringLanesCont& lanes,
                        const ExpandStringSignPosts& signPosts,
                        ItemTypes::crossingkind_t crossingKind =
                            ItemTypes::UNDEFINED_CROSSING,
                        LandmarkHead* landmarks = NULL,
                        uint32 nbrPossTurns = 0,
                        uint32* possTurns = NULL );
      
      /**
        *   Destructor, returns allocated memory to the OS.
        */
      virtual ~ExpandStringItem();
   
      /**
        *   Used to set the bitfield that describes the 
        *   route description properties according to the specified
        *   parameters.
        *
        *   @param   lang  Language code of the route description.
        *   @param   compassLangShort If compass direction should be short.
        *   @param   maxTurnNumbers Maximum number of turns that should
        *                           be printed out. Eg "turn left at
        *                           the 9:th road".
        *   @param   printSignPostText Set this to true if signpost
        *                              texts should be printed out.
        *   @param   distFormat     Distance format used when printing
        *                           the distance.
        *   @param   distUnit       Measurment unit  used when printing
        *                           the distance. (Meters, feet or yards)
        *   @param   wapFormat      Set this to true if the text
        *                           should be wap formatted. All text that
        *                           displayed on a wap phone must be
        *                           formatted (for instance swedish
        *                           characters must be replaced with
        *                           escape-codes and so on).
        *   @param   oddevenstart The start direction for odd even house
        *                         numbers.
        *   @param   housenbrstart The start direction for house numbers.
        *   @return  The resulting uint32 describing the route
        *            description properties.
        */
      static DescProps createRouteDescProps(
                              StringTable::languageCode lang,
                              bool compassLangShort,
                              byte maxTurnNumbers,
                              bool printSignPostText,
                              StringTableUtility::distanceFormat distFormat,
                              StringTableUtility::distanceUnit distUnit,
                              bool wapFormat,
                              ItemTypes::routedir_oddeven_t oddevenstart,
                              ItemTypes::routedir_nbr_t housenbrstart);

      /**
        *   Creates a route description, text formatted according to
        *   the specified route description properties bitfield.
        *   For instance "Drive 660m then turn left into the
        *   third street Baravägen".
        *   @param descProps  Route description properties (bitfield) that
        *                     that indicates which information
        *                     that should be included in the route
        *                     decription.
        *   @param buf        Preallocated buffer which will be filled
        *                     with the routedescription.
        *   @param maxLength  The length of buf.
        *   @param nbrBytesWritten  The number of bytes written into buf.
        *   @return True if the route description did fit in the
        *           buffer, false otherwise.
        */
      bool getRouteDescription( DescProps descProps, 
                                char* buf,
                                uint32 maxLength,
                                uint32& nbrBytesWritten ) const;
      
      /**
       *    Get this ExpandStringItem as text that should be inserted
       *    into an SMS.
       *
       *    @param  language        The prefered language of the route-
       *                            description. Must be an SMS-language
       *                            (e.g. SMSISH_ENG or SMSISH_SWE).
       *    @param  eol             The type of end-o-line that should be
       *                            used.
       *    @param  maxLineLength   The maximum length of one row in the
       *                            SMS.
       *    @param  dest            A preallocated string where the result
       *                            will be written.
       *    @param  maxLength       The maximum number of characters to
       *                            write to dest.
       *    @return The number of bytes written to dest, a negative value 
       *            is returned upon error.
       */
      int getRouteDescriptionSMS(
                     StringTable::languageCode language,
                     UserConstants::EOLType eol, 
                     int maxLineLength,
                     char* dest, 
                     uint32 maxLength,
                     StringTableUtility::distanceUnit distUnit
                     = StringTableUtility::METERS);
      
      /**
       *    @name Set optional text.
       *    Methods for setting text that is optional and should be added 
       *    to the route description somewhere.
       */
      //@{
         /**
           *   Sets the location of the origin.
           *   @param str  String with the above information.
           */
         void setOriginLocation(const char* str);
         
         /**
           *   Sets the location of the destination.
           *   @param str  String with the above information.
           */
         void setDestinationLocation(const char* str);

         /**
           *   Sets the road number (as in Baravägen 1.) of the origin.
           *   @param str  String with the above information.
           */
         void setOriginRoadNumber(const char* str);
         /**
           *   Sets the road number (as in Baravägen 1.) of the destination.
           *   @param str  String with the above information.
           */
         void setDestinationRoadNumber(const char* str);
         
         /**
           *   Sets the origin company.
           *   @param str  String with the above information.
           */
         void setOriginCompany(const char* str);
         
         /**
           *   Sets the destination company.
           *   @param str  String with the above information.
           */
         void setDestinationCompany(const char* str);

         /**
           *   Sets text that will be inserted before the company if
           *   specified. For instance linebreak information. Note that
           *   this text will not be wap formatted (so wml tags can be
           *   used).
           *   @param str  String with the above information.
           */
         void setPreCompanyText(const char* str);
         
         /**
           *   Sets text that will be inserted before the roadname if
           *   present. For instance linebreak information. Note that
           *   this text will not be wap formatted (so wml tags can be
           *   used).
           *   @param str  String with the above information.
           */
         void setPreRoadName(const char* str);
         
         /**
           *   Sets text that will be inserted before the roadname if
           *   present. For instance linebreak information. Note that
           *   this text will not be wap formatted (so wml tags can be
           *   used).
           *   @param str  String with the above information.
           */
         void setPostRoadName(const char* str);
      //@}
      
      /**
        *   Get the distance of this part of the route description.
        *   @return  The distance of this part of the routedescription.
        */
      uint32 getDist() const {
         return m_dist;
      }
      
      /**
        *   Get the travell time of this part of the route description.
        *   @return  The time of this part of the routedescription.
        */
      uint32 getTime() const {
         return m_time;
      }
      
      /**
        *   Get the turdescription for this part of the route description.
        *   @return  The stringcode describing the turn direction at
        *            this part of the route description.
        */
      StringTable::stringCode getStringCode() const {
         return StringTable::stringCode(m_stringCode);
      }
      
      /**
        *   Get the exit count for this part of the route description. The
        *   exit count describes the serial number of the street to turn
        *   into, e.g "2" here and "RIGHT" as getStringCode means
        *   "turn into the secon street on the right side".
        *
        *   @return  Exit count at the side described by getStringCode()
        */
      byte getExitCount() const {
         return m_turnNumber;
      }

      ItemTypes::crossingkind_t getCrossingKind() const {
         return m_crossingKind;
      }

      /**
        *   Get the name of the street or company.
        *   @return  The name of the street (or company).
        */
      const char* getText() const {
         return m_str;
      }

      /**   
       *    Set the name of the street or company.
       *    @param   text  The new name.
       */
      void setText(const char* text) {
         delete [] m_str;
         m_str = StringUtility::newStrDup(text);
      }
      
      /**
       * Get the lanes.
       *
       * @return The laness.
       */
      const ExpandStringLanesCont& getLanes() const;

      /**
       * Get the signposts.
       *
       * @return The signposts.
       */
      const ExpandStringSignPosts& getSignPosts() const;

      /**
        *   Get the type of the name.
        *   @return  The type of name in this item. {missing, ... }. The 
        *            type will be changed when more info is available.
        */
      byte getNameType() const {
         return m_nameType;
      }

      /** 
        *   Find out if the item has a name or not. Currently returns 0 
        *   if name is "MISSING" (or "Missing" or some other case 
        *   combination).
        *   @return  The stringindex of the name of the item.
        */
      uint32 hasName() const {
         if ( getText() != NULL ) 
            return (uint32) StringUtility::strcasecmp( "MISSING", getText() );
         else
            return 1;
      }

      /**
       *    Find out if the item has a signpost or not.
       *    @return  True if a signpost is associated with the item,
       *             false otherwise.
       */
      bool hasSignPost() const;

      /**
        *   Get the longitude part of the coordinate for this part of
        *   the route description.
        *   @return  The longitude at this part of the route description.
        */
      int32 getLongitude() const {
         return m_longitude;
      }

      /**
        *   Get the latitude part of the coordinate for this part of
        *   the route description.
        *   @return  The latitude at this part of the route description.
        */
      int32 getLatitude() const {
         return m_latitude;
      }
    
      /**
        *   Get the type of transportation.
        *   @return  The transportatiointype for this pice of the route.
        */
      ItemTypes::transportation_t getTransportationType() const {
         return m_transportationType;
      }

      /**
        *   Find out if the item has any landmarks.
        *   @return True if there are ant landmarks, false otherwise.
        */
      bool hasLandmarks() const;

      /**
        *   Get the number of landmarks of this item.
        *   @return Number of landmarks.
        */
      uint32 getNbrLandmarks() const;

      /**
        *   Returns a pointer to the landmark list of this item.
        */
      LandmarkHead* getLandmarks() const;

      /**
        *   Returns the number of possible turns not taken here..
        */
      uint32 getNbrPossTurns() const
         {return m_nbrPossTurns; }

      
      /**
       *   Returns a pointer to the possibleTurns array..
       */
      uint32* getPossibleTurns() 
         {return m_possTurns;}
      

      /**
       * Return s string representing the transportation type in parameter.
       * Why this is here? Because this class uses StringUtility in 
       * the declatation!
       * @param transportationType The type of transportation.
       * @return A string representing the transportation type.
       */
      static const char* transportationTypeToString( 
         ItemTypes::transportation_t transportationType );

 
      /**
        *   Appends a string to the specified buffer. Wap formats the 
        *   appended string as well if that is requested.
        *   @param   appendString   The string to be appended.
        *   @param   buf            The buffer where to append 
        *                           appendString.
        *   @param   maxLength      The maximum length allowed for buf.
        *   @param   nbrBytesWritten   Outparameter that indicates 
        *                              the number of bytes that the
        *                              totally contains of including the
        *                              appended string.
        *   @param   wapFormat      If this is set to true, the
        *                           appendString is first wap formatted.
        *                           Is false default.
        *   @return  True if the result did fit in buf, false otherwise.
        *                              
        */
      static bool addString(const char* appendString, 
                     char* buf,
                     uint32 maxLength,
                     uint32& nbrBytesWritten,
                     bool wapFormat = false);
      
      bool isEndOfRoadTurn() const;
      
   private:
         
      /** 
        *   The name of the street.
        */
      char* m_str;
      
      /** 
        *   The distance.
        */
      uint32 m_dist;
      
      /** 
        *   The traverse time.
        */
      uint32 m_time;
      
      /**   
        *   The turndirection in terms of the StringTable. This also
        *   contains the exitcount (in the most significant byte).
        */
      uint32 m_stringCode;
      
      /**
        *   The type of transportation (e.g wlak or drive).
        */
      ItemTypes::transportation_t m_transportationType;

       /**
        *   @name The geographical position.
        */
      //@{
         /** 
           *   Latitude part of the position.
           */
         int32 m_latitude;
         
         /** 
           *   Longitude part of the position.
           */
         int32 m_longitude;
      //@}
     
      /**
        *   The type of name.
        */
      byte m_nameType;

      /** 
        *   Turnnumber, for example 2nd street on the left (stringCode).
        */
      byte m_turnNumber;

      /** 
        *   The type of crossing for this turn. Might be 3-way, 4-way etc.
        */
      ItemTypes::crossingkind_t m_crossingKind;

      /**
       *  The number of possible turns NOT made at this turn.
       */ 
      uint32 m_nbrPossTurns;

      /**
       *  String codes of the turns that was possible but not made.
       */
      uint32 m_possTurns[7];

      /**
       * The lanes.
       */
      ExpandStringLanesCont m_lanes;

      /**
       * The signposts.
       */
      ExpandStringSignPosts m_signPosts;

      /**
        *   The landmarks of this item.
        */
      LandmarkHead* m_landmarks;
      
      /**
        *   @name Optional text
        *   Member variables containing optional text that should be 
        *   added to the route description.
        */
      //@{
         /** 
           *   Location of the origin.
           */
         char* m_originLocation;
         
         /** 
           *   Location of the destination.
           */
         char* m_destinationLocation;
         
         /** 
           *   Roadnumber (for instance "Baravägen 1") of the origin.
           */
         char* m_originRoadNumber;
         
         /** 
           *   Roadnumber (for instance "Baravägen 1") of the destination.
           */
         char* m_destinationRoadNumber;
         
         /**
           *   Origin company.
           */
         char* m_originCompany;
         
         /**
           *   Destination company.
           */
         char* m_destinationCompany;

         /**
           *   Text before companies. For instance html or wml code.
           */
         char* m_preCompanyText;
         
         /**
           *   Text before roadname. For instance html or wml code.
           */
         char* m_preRoadName;
         
         /**
           *   Text after roadname. For instance html or wml code.
           */
         char* m_postRoadName;
      //@}
};


// =======================================================================
//                                     Implementation of inlined methods =

inline const ExpandStringLanesCont& 
ExpandStringItem::getLanes() const {
   return m_lanes;
}

inline const ExpandStringSignPosts&
ExpandStringItem::getSignPosts() const {
   return m_signPosts;
}

inline bool
ExpandStringItem::hasSignPost() const {
   return m_signPosts.size() > 0;
}

#endif
