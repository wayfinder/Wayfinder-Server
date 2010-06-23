/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandStringItem.h"
#include "LandmarkHead.h"
#include "Utility.h"
#include "StringTableUtility.h"

DescProps::DescProps(
   StringTable::languageCode lang, byte maxTurnNumber,
   StringTableUtility::distanceFormat distFormat,
   StringTableUtility::distanceUnit distUnit, bool printSignPostText,
   bool wapFormat, bool compassLangShort, byte houseNbrStart)
{
   m_lang                  = lang;
   m_maxTurnNumber         = maxTurnNumber;
   m_distFormat            = distFormat;
   m_distUnit              = distUnit;
   m_printSignPostText     = printSignPostText; 
   m_wapFormat             = wapFormat;
   m_compassLangShort      = compassLangShort;
   m_houseNbrStart         = houseNbrStart;
}

   
   
StringTable::languageCode
DescProps::getLanguage() const 
{
   return m_lang;
}

   
byte
DescProps::getMaxTurnNumber() const
{
   return m_maxTurnNumber;
}


StringTableUtility::distanceFormat
DescProps::getDistFormat() const
{
   return m_distFormat; 
}

StringTableUtility::distanceUnit
DescProps::getDistUnit() const
{
   return m_distUnit; 
}

bool
DescProps::getPrintSignPostText() const
{
   return m_printSignPostText;
}

bool
DescProps::getWapFormat() const{
   return m_wapFormat;
}

bool
DescProps::getCompassLangShort() const {
   return m_compassLangShort;
}

byte
DescProps::getHouseNbrStart() const{
   return m_houseNbrStart;
}


ExpandStringItem::ExpandStringItem( 
               uint32 dist,
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
               ItemTypes::crossingkind_t crossingKind,
               LandmarkHead* landmarks,
               uint32 nbrPossTurns,
               uint32* possTurns)
      : m_dist(dist), m_time(time), m_stringCode(stringCode),
        m_transportationType(transport), m_latitude(lat), 
        m_longitude(lon), m_nameType(nameType), m_turnNumber(turnNumber),
        m_crossingKind( crossingKind ), m_nbrPossTurns( nbrPossTurns ),
        m_lanes( lanes ), m_signPosts( signPosts )
        
{
   m_str = new char[strlen(str)+1];
   strcpy(m_str, str);
   m_landmarks = new LandmarkHead();
   if (landmarks != NULL) {
      LandmarkLink* lmLink = static_cast<LandmarkLink*>(landmarks->first());
      while (lmLink != NULL) {
         LandmarkLink* insertLink = lmLink;
         lmLink = static_cast<LandmarkLink*>(lmLink->suc());
         insertLink->into(m_landmarks);
      }
   }
   MC2_ASSERT(m_nbrPossTurns < 7);
   for(uint32 i = 0 ; i < m_nbrPossTurns ; i++){
      m_possTurns[i] = *possTurns++;
   }
   m_originLocation = NULL;
   m_destinationLocation = NULL;
   m_originRoadNumber = NULL;
   m_destinationRoadNumber = NULL;
   m_originCompany = NULL;
   m_destinationCompany = NULL;
   m_preCompanyText = NULL;
   m_preRoadName = NULL;
   m_postRoadName = NULL;
} 


ExpandStringItem::~ExpandStringItem() {
   delete [] m_str;
   delete m_landmarks;

   delete [] m_originLocation;
   delete [] m_destinationLocation;
   delete [] m_originRoadNumber;
   delete [] m_destinationRoadNumber;
   delete [] m_originCompany;
   delete [] m_destinationCompany;
   delete [] m_preCompanyText;
   delete [] m_preRoadName;
   delete [] m_postRoadName;
}

bool
ExpandStringItem::hasLandmarks() const
{
   if (m_landmarks == NULL)
      return false;
   else 
      return(!m_landmarks->empty());
}

uint32
ExpandStringItem::getNbrLandmarks() const
{
   if (m_landmarks == NULL)
      return 0;
   else 
      return(m_landmarks->cardinal());
}

LandmarkHead*
ExpandStringItem::getLandmarks() const
{
   return m_landmarks;
}

DescProps
ExpandStringItem::createRouteDescProps(
                     StringTable::languageCode lang,
                     bool compassLangShort,
                     byte maxTurnNumbers,
                     bool printSignPostText,
                     StringTableUtility::distanceFormat distFormat,
                     StringTableUtility::distanceUnit distUnit,
                     bool wapFormat,
                     ItemTypes::routedir_oddeven_t oddevenstart,
                     ItemTypes::routedir_nbr_t housenbrstart)
{
   byte houseNbrByte = 0;
   if ( oddevenstart != ItemTypes::unknown_oddeven_t ) {
      houseNbrByte = oddevenstart;
   } else {
      houseNbrByte = ItemTypes::rightOddLeftEven + 1 + housenbrstart;
   }
   
   return DescProps( lang, maxTurnNumbers, distFormat, distUnit, 
                     printSignPostText,
                     wapFormat, compassLangShort, houseNbrByte );

   
   
   
   /*
   uint32 bitmask = 0;

   bitmask += (uint32(lang) << LANGUAGE_CODE_SHIFT) &  
      LANGUAGE_CODE_MASK; 
      bitmask += (maxTurnNumbers << MAX_TURN_NUMBER_SHIFT) &
      MAX_TURN_NUMBER_MASK;
      bitmask += (printSignPostText << SIGNPOST_TEXT_SHIFT) &
      SIGNPOST_TEXT_MASK; 
      bitmask += (printSignPostExitNbr << SIGNPOST_EXIT_NUMBER_SHIFT) &
      SIGNPOST_EXIT_NUMBER_MASK;
      bitmask += (printSignPostRouteNbr << SIGNPOST_ROUTE_NUMBER_SHIFT) &
   SIGNPOST_ROUTE_NUMBER_MASK;
   bitmask += (uint32(distFormat) << DISTANCE_FORMAT_SHIFT) &
      DISTANCE_FORMAT_MASK; 
      bitmask += (wapFormat << WAP_FORMAT_SHIFT) &
      WAP_FORMAT_MASK;
      bitmask += (compassLangShort << COMPASS_LANGUAGE_SHORT_SHIFT) &
      COMPASS_LANGUAGE_SHORT_MASK;
   uint32 houseNbrByte = 0; 
   if ( oddevenstart != ItemTypes::unknown_oddeven_t ) { 
      houseNbrByte = oddevenstart; 
   } else {
      houseNbrByte = ItemTypes::rightOddLeftEven + 1 + housenbrstart; 
   } 
   bitmask += (houseNbrByte << START_HOUSENUMBER_DIR_SHIFT ) &  
      START_HOUSENUMBER_DIR_MASK; 
               
      return (bitmask);
   */
}

void
ExpandStringItem::setOriginLocation(const char* str)
{
   delete [] m_originLocation;
   m_originLocation = StringUtility::newStrDup(str);
}

void
ExpandStringItem::setDestinationLocation(const char* str)
{
   delete [] m_destinationLocation;
   m_destinationLocation = StringUtility::newStrDup(str);
}

void
ExpandStringItem::setOriginCompany(const char* str)
{
   delete [] m_originCompany;
   m_originCompany = StringUtility::newStrDup(str);
}

void
ExpandStringItem::setDestinationCompany(const char* str)
{
   delete [] m_destinationCompany;
   m_destinationCompany = StringUtility::newStrDup(str);
}

void
ExpandStringItem::setOriginRoadNumber(const char* str)
{
   delete [] m_originRoadNumber;
   m_originRoadNumber = StringUtility::newStrDup(str);
}

void
ExpandStringItem::setDestinationRoadNumber(const char* str)
{
   delete [] m_destinationRoadNumber;
   m_destinationRoadNumber = StringUtility::newStrDup(str);
}

void
ExpandStringItem::setPreCompanyText(const char* str)
{
   delete [] m_preCompanyText;
   m_preCompanyText = StringUtility::newStrDup(str);
}

void
ExpandStringItem::setPreRoadName(const char* str)
{
   delete [] m_preRoadName;
   m_preRoadName = StringUtility::newStrDup(str);
}

void
ExpandStringItem::setPostRoadName(const char* str)
{
   delete [] m_postRoadName;
   m_postRoadName = StringUtility::newStrDup(str);
}

bool
ExpandStringItem::getRouteDescription( DescProps descProps,
                                       char* buf,
                                       uint32 maxLength,
                                       uint32& nbrBytesWritten ) const
{
   // Extract information from the description properties bitfield.
   StringTable::languageCode lang = descProps.getLanguage();
   byte maxTurnNumber = descProps.getMaxTurnNumber();
   StringTableUtility::distanceFormat distFormat = descProps.getDistFormat();
   StringTableUtility::distanceUnit distUnit = descProps.getDistUnit();
   bool printSignPostText = descProps.getPrintSignPostText(); 
   bool wapFormat = descProps.getWapFormat();
   bool compassLangShort = descProps.getCompassLangShort();
   byte houseNbrStart = descProps.getHouseNbrStart();
   
   buf[0] = '\0';
  
   enum routeState {
      NORMAL, // Normal
      START,  // Start of a route. ie. "Start NW at Baravägen"
      END     // End of a route.
   };
 
   routeState currRouteState;
   
   switch (m_stringCode) {
      case (StringTable::DRIVE_START):  
      case (StringTable::DRIVE_START_WITH_UTURN):
         currRouteState = START;
         break;
      case (StringTable::DRIVE_FINALLY):
         currRouteState = END;
         break;
      default:
         currRouteState = NORMAL;
         break;
   }
   DEBUG4(
      mc2dbg << "Made turn = "
      << StringTable::getString(StringTable::stringCode(m_stringCode),
                                lang) 
             << "  Possible turns = " << m_nbrPossTurns << endl;
      for(uint32 i = 0 ; i < m_nbrPossTurns ; i++){
         mc2dbg << "  Turn " << i << " : "
                << StringTable::getString(StringTable::stringCode(
                                           m_possTurns[i]),lang) << endl;   
      }
      );
      


      
   
   // ----------------------- Drive / Drive finally / Walk / Walk finally -
   // ---------------------------------------------------------------------
   
   // Don't print out "Drive 0 meters..."
   if (m_dist > 0) {
      switch (m_transportationType) {
         case (ItemTypes::walk) :
            if (m_stringCode == StringTable::DRIVE_FINALLY) {
               if (! addString(  StringTable::getString(
                                    StringTable::FINALLY_WALK, lang),
                                 buf, 
                                 maxLength,
                                 nbrBytesWritten,
                                 wapFormat)  ) { 
                  return (false);
               } 
            } else if (currRouteState == NORMAL) {
               if((m_stringCode != uint32(StringTable::EXIT_FERRY_TURN)) &&
                  (m_stringCode != uint32(StringTable::CHANGE_FERRY_TURN))){
                  if (! addString(  StringTable::getString(
                     StringTable::WALK, lang),
                                    buf, 
                                    maxLength, 
                                    nbrBytesWritten,
                                    wapFormat) ) {
                     return (false);
                  }
               } else {
                  if (! addString(  StringTable::getString(
                     StringTable::RIDE, lang),
                                    buf, 
                                    maxLength, 
                                    nbrBytesWritten,
                                    wapFormat) ) {
                     return (false);
                  } 
               }   
            }
            break;
         
         case (ItemTypes::drive) :
         default :
            if (m_stringCode == StringTable::DRIVE_FINALLY) {
               if (! addString(  StringTable::getString(
                                    StringTable::FINALY, lang),
                                 buf, 
                                 maxLength, 
                                 nbrBytesWritten,
                                 wapFormat) ) {
                  return (false);
               } 
            } else if (currRouteState == NORMAL) {
               if((m_stringCode != uint32(StringTable::EXIT_FERRY_TURN)) &&
                  (m_stringCode != uint32(StringTable::CHANGE_FERRY_TURN))){
                  
                  if (! addString(  StringTable::getString(
                     StringTable::DRIVE, lang),
                                    buf, 
                                    maxLength, 
                                    nbrBytesWritten,
                                    wapFormat) ) {
                     return (false);
                  } 
               } else {
                  if (! addString(  StringTable::getString(
                     StringTable::RIDE, lang),
                                    buf, 
                                    maxLength, 
                                    nbrBytesWritten,
                                    wapFormat) ) {
                     return (false);
                  } 
               }
               
            }
            break;
      }
   }
   
   if (currRouteState == START) {
      if (! addString(  StringTable::getString(
                        StringTable::DRIVE_START, lang),
                        buf, 
                        maxLength, 
                        nbrBytesWritten,
                        wapFormat) ) {
         return (false);
      }
   }
      
   // Add space if anything was written into buffer
   if ((buf[0] != '\0') && 
       (! addString(" ", buf, maxLength, nbrBytesWritten))) {
      return (false);
   }
   
   
   if (currRouteState == START) {
      // ----------------------------------- Housenumber start direction -
      // -----------------------------------------------------------------
      /*
       *   0 - unknown_oddeven_t
       *   1 - leftOddRightEven
       *   2 - rightOddLeftEven
       *   3 - unknown_nbr_t
       *   4 - increasing
       *   5 - decreasing
       */
      bool addParenthesisLast = false;
      if ( houseNbrStart > ItemTypes::unknown_oddeven_t &&
           houseNbrStart <= (ItemTypes::rightOddLeftEven + 1 +
                             ItemTypes::decreasing) &&
           houseNbrStart != (ItemTypes::rightOddLeftEven + 1 +
                             ItemTypes::unknown_nbr_t) )
      {
         StringTable::stringCode houseNbrSC = StringTable::NOSTRING;
         if ( houseNbrStart <= ItemTypes::rightOddLeftEven ) { 
            // Valid housenbrstart
            houseNbrSC = ItemTypes::getStartDirectionOddEvenSC(
               ItemTypes::routedir_oddeven_t( houseNbrStart) );
         } else {
            // Valid housenbroddevenstart
            houseNbrSC = ItemTypes::getStartDirectionHousenumberSC(
               ItemTypes::routedir_nbr_t( houseNbrStart - 1 -
                                          ItemTypes::rightOddLeftEven ) );
         }
         if (! addString( StringTable::getString( houseNbrSC, lang),
                          buf, 
                          maxLength, 
                          nbrBytesWritten,
                          wapFormat) ) 
         {
            return (false);
         }
         // Add space and parenthesis
         if (! addString(" (", buf, maxLength, nbrBytesWritten, wapFormat))
         {
            return (false);
         } 
         
         addParenthesisLast= true;
      } 

      // --------------------------------------------- Compass direction -
      // -----------------------------------------------------------------
      // Add compass direction instead of distance
      StringTable::languageCode compassLang = lang;
      if ( compassLangShort && 
           !StringTable::isShortLangugage( compassLang ) ) 
      {
         compassLang = StringTable::getShortLanguageCode( compassLang );
      } else if ( !compassLangShort && 
                  StringTable::isShortLangugage( compassLang ) )
      {
         compassLang = StringTable::getNormalLanguageCode( compassLang );
      }

      if (! addString(
                  StringTable::getString(
                     Utility::getCompass(m_dist, 2), //2 as in 8 directions
                     compassLang),
                  buf, maxLength, nbrBytesWritten, wapFormat ) ) 
      {
         return (false);
      }
      
      if ( addParenthesisLast ) {
         if (! addString(")", buf, maxLength, nbrBytesWritten, wapFormat)) 
         {
            return (false);
         } 
      }
      // Add space
      if (! addString(" ", buf, maxLength, nbrBytesWritten, wapFormat)) {
         return (false);
      }
   } else {
   
      // ------------------------------------------------------ Distance -
      // -----------------------------------------------------------------
      // Don't print out "Drive 0 meters ...."
      if (m_dist > 0) {
         // Create a temporary buffer         
         char* distBuf = new char[1024];
         StringTableUtility::printDistance(distBuf, m_dist, lang, distFormat, distUnit);

         if (! addString(  distBuf, buf, maxLength, nbrBytesWritten  )  ) {
            return (false);
         }

         // Delete the temporary buffer
         delete [] distBuf;

         // Add space moved to next step - StringTable::getRouteDescription()
      }
   }

   // ------------------------------ then turn left into the 3:rd street -
   // --------------------------------------------------------------------
   if((m_dist > 0) || (m_stringCode != StringTable::DRIVE_FINALLY)){
      
   
      
   // Create a temporary buffer
   const int turnBufMaxLength = 128;
   char turnBuf[ turnBufMaxLength ];
   memset( turnBuf, 0, 128 );

   // If this is 0 it means that the turn number is too high 
   // to be displayed.
   byte turnNbrToDisplay;
   
   if (m_turnNumber <= maxTurnNumber) {
      turnNbrToDisplay = m_turnNumber;
   } else {
      turnNbrToDisplay = 0;
   }
   
   // Check if this is a turn at the end of the road
   // t-crossings, no crossing with turns.
   bool endOfRoad = false;
   
   if((m_dist > 0) && ((m_stringCode == StringTable::LEFT_TURN) ||
                        (m_stringCode == StringTable::RIGHT_TURN))){ 
      if(isEndOfRoadTurn()){
         // End of the road T-crossing.
         endOfRoad = true;
         turnNbrToDisplay = 0;// lose the turn number 
      } else {
         //Add space
         if(! addString(" ", buf, maxLength, nbrBytesWritten, wapFormat)) {
            return (false); 
         }
      }
      
   } else if ((m_dist > 0) && (currRouteState != START)) {
      //Add space
      if(! addString(" ", buf, maxLength, nbrBytesWritten, wapFormat)) {
         return (false);
      }
   }
   
      
   StringTable::getRouteDescription(StringTable::stringCode(m_stringCode),
                                    lang, 
                                    turnNbrToDisplay,
                                    turnBuf, turnBufMaxLength,
                                    hasName()!=0,
                                    endOfRoad);

   // If the distance was 0, then "Drive 0 meters" is skipped and therefore
   // the first character here should be uppercase.
   if (m_dist == 0) {
      MC2String tmp = StringUtility::makeFirstCapital(MC2String(turnBuf));
      for(uint32 i = 0; i < tmp.size(); i++) {
         if( i < (uint32) turnBufMaxLength ) 
            turnBuf[i] = tmp[i];
      }
   }
   
   // Now add this to the rest of our buffer.
   if (! addString(turnBuf, buf, maxLength, nbrBytesWritten, wapFormat)) {
      return (false);
   }

   
   // -------------------------------------------------------- Road name -
   // --------------------------------------------------------------------
   
   if (hasName()!=0) {
      
      // Add space
      if (! addString(" ", buf, maxLength, nbrBytesWritten)) {
         return (false);
      }
   

      // Add the pre roadname string if existing.
      if ( m_preRoadName != NULL) {
         if (! addString(  m_preRoadName, 
                           buf, 
                           maxLength, 
                           nbrBytesWritten   )) {
            return (false);
         }
      }

      // Check if origin/destination location should be added.
      char* locationStr = NULL;
      if (currRouteState == START) {
         locationStr = m_originLocation;
      } else if (currRouteState == END) {
         locationStr = m_destinationLocation;
      }
      if (locationStr != NULL) {
         // Add location 
         if (! addString(locationStr, buf, maxLength, nbrBytesWritten, 
                         wapFormat)) {
            return (false);
         }
         // Add ":"
         if (! addString(":", buf, maxLength, nbrBytesWritten, 
                         wapFormat)) {
            return (false);
         }
      }
   
      // Road name exists. Add!
      if (! addString(m_str, buf, maxLength, nbrBytesWritten, wapFormat)) {
         return (false);
      }

      // Check if roadnumber should be added
      char* roadNumberStr = NULL;
      if (currRouteState == START) {
         roadNumberStr = m_originRoadNumber;
      } else if (currRouteState == END) {
         roadNumberStr = m_destinationRoadNumber;
      }
      if (roadNumberStr != NULL) {
         // Add roadnumber
         if (! addString(roadNumberStr, buf, maxLength, nbrBytesWritten, 
                         wapFormat)) {
            return (false);
         }
      }
      
      // Add the post roadname string if existing.
      if ( m_postRoadName != NULL) {
         if (! addString(  m_postRoadName, 
                           buf, 
                           maxLength, 
                           nbrBytesWritten)) {
            return (false);
         }
      }

   }
   
   
   // -------------------------------------------------------- Signposts -
   // --------------------------------------------------------------------

   if (printSignPostText && hasSignPost()) {
      // Add: "at sign reading"
      if ( !addString(  StringTable::getString(
                           StringTable::AT_SIGN_READING, lang),
                        buf, maxLength, nbrBytesWritten, wapFormat) ) {
         return false;
      }
      // Add space
      if ( !addString( " ", buf, maxLength, nbrBytesWritten ) ) {
         return false;
      }
      // For all signs
      for ( uint32 i = 0 ; i < m_signPosts.size() ; ++i ) {
         if ( i != 0 ) {
            // Add comma and space
            if ( !addString( ", ", buf, maxLength, nbrBytesWritten) ) {
               return false;
            }
         }
         if ( m_signPosts[ i ].getElementType() == ExpandStringSignPost::exit )
         {
            // Exit
            // Add: "exit"
            if ( !addString( StringTable::getString( StringTable::EXIT, lang ),
                             buf, maxLength, nbrBytesWritten, wapFormat) ) {
               return false;
            }

            // Add space
            if ( !addString( " ", buf, maxLength, nbrBytesWritten ) ) {
               return false;
            }

            // Add exitnumber
            if ( !addString( m_signPosts[ i ].getText().c_str(), buf, 
                             maxLength, nbrBytesWritten, wapFormat ) ) {
               return false;
            }
         } else {
            if ( !addString( m_signPosts[ i ].getText().c_str(), buf, 
                             maxLength, nbrBytesWritten, wapFormat ) )  {
               return false;
            }
         }
      }
   } // End if printSignposts and has signposts

   } // End if have dist != 0 and is not DRIVE_FINALLY

   // ------------------------------------------------- Add company name? -
   // ---------------------------------------------------------------------
   const char* companyStr = NULL;
   if (currRouteState == START) {
      companyStr = m_originCompany;
   } else if (currRouteState == END) {
      companyStr = m_destinationCompany;
   } // else: no company should be added so let companyStr == NULL

   if (companyStr != NULL) {
      if (m_preCompanyText != NULL) {
         // Add text before company, (for instance linebreak)
         if (! addString(m_preCompanyText, buf, maxLength,
                         nbrBytesWritten)) {
            return (false);
         }
      }
      // Add company
      if (! addString(companyStr, buf, maxLength, nbrBytesWritten,
                      wapFormat)) {
         return (false);
      }
   }
   
   // Everything did fit in the buffer.
   return (true);
}

/*
int 
ExpandStringItem::getLengthAsSMS(const char* eol, int maxLineLength)
{

}
*/

int  
ExpandStringItem::getRouteDescriptionSMS(
                              StringTable::languageCode language,
                              UserConstants::EOLType eol, 
                              int maxLineLength,
                              char* dest, 
                              uint32 maxLength,
                              StringTableUtility::distanceUnit distUnit )
{
   // Find out if this is the first or last item
   bool isOrig = false;
   bool isDest = false;
   if (m_stringCode == StringTable::DRIVE_START) {
      isOrig = true;
   } else if (( m_stringCode == StringTable::FINALY) ||
              ( m_stringCode == StringTable::DRIVE_FINALLY)){
      isDest = true;
   }

   // Constants!?
   const uint32 nbrRows = 2;  // The maximum number of rows for the name
   
   // Some variables
   int destPos = 0;// The current position in dest
   const uint32 maxTurnDescriptionLength = 128;
   char turnDescription[maxTurnDescriptionLength];
   
   // Is this the origin?
   if (isOrig) {
      destPos = sprintf( dest + destPos, "%s %s %s",
                          StringTable::getString(
                             StringTable::stringCode( getStringCode()),
                                                      language),
                          StringTable::getString(
                             Utility::getCompass(getDist(), 2),
                             language),
                          StringTable::getString( StringTable::DIR_AT,
                                                  language)
                       );
      destPos += SMSStringUtility::writeEol( dest, destPos, eol, 100);
      dest[destPos] = '\0';

      // Add origin location??
      if ( (m_originLocation != NULL) && 
           (strlen(m_originLocation) > 0)) {
         // Got a description (city or district) of the origin
         char* tmpStr=new char[strlen(m_originLocation)+2];
         strcpy(tmpStr, m_originLocation);
         strcat(tmpStr, ":");
         destPos += SMSStringUtility::printSMSString( 
                                    dest + destPos, tmpStr,
                                    language, eol,
                                    maxLineLength, 0, 4, false );
         dest[destPos] = '\0';
         delete [] tmpStr;
      }

      // Add origin company??
      if ( (m_originCompany != NULL) && 
           (strlen(m_originCompany) > 0)) {
         // Got a description (city or district) of the origin
         char* tmpStr=new char[strlen(m_originCompany)+2];
         strcpy(tmpStr, m_originCompany);
         strcat(tmpStr, ":");
         destPos += SMSStringUtility::printSMSString( 
                                    dest + destPos, tmpStr,
                                    language, eol,
                                    maxLineLength, 0, 4, false );
         dest[destPos] = '\0';
         delete [] tmpStr;
      }

      // Add origin street-name and number
      destPos += SMSStringUtility::printSMSString( 
                                 dest + destPos, getText(),
                                 language, eol,
                                 maxLineLength, 0, 4, false);
      DEBUG8(
         mc2dbg << "m_originRoadNumber=" << m_originRoadNumber << endl;
      );
      if ( (m_originRoadNumber != NULL) &&
           (strlen(m_originRoadNumber) > 0)) {
         char* tmpStr=new char[strlen(m_originRoadNumber)+2];
         tmpStr[0] = ' ';
         tmpStr[1] = '\0';
         strcat(tmpStr,m_originRoadNumber); 
         destPos += SMSStringUtility::printSMSString( 
                                    dest + destPos, tmpStr,
                                    language, eol,
                                    maxLineLength, 0, 4, true );
         delete [] tmpStr;
      } else {
         destPos += SMSStringUtility::writeEol( dest, destPos, eol, 3 );
      }

   } else if (isDest) {
      // Print the destination

      // Print distance and stringcode (stop)
      StringTable::getRouteDescription(
         getStringCode(),
         language,
         0,
         turnDescription, 
         maxTurnDescriptionLength);
      destPos = StringTableUtility::printDistance(dest,
                                             getDist(),
                                             language,
                                             StringTableUtility::COMPACT,
                                             distUnit);
      DEBUG8(
         mc2dbg << "   dest=\"" << dest << "\"" << endl;
      );
      destPos += sprintf( dest + destPos, " %s", turnDescription);
      destPos += SMSStringUtility::writeEol( dest, destPos, eol, 3 );

      DEBUG8(
         mc2dbg << "m_destinationRoadNumber=" << m_destinationRoadNumber
         << endl
         << "m_destinationLocation=" << m_destinationLocation << endl;
         << "m_destinationCompany=" << m_destinationCompany << endl;
      );

      // Add destination location??
      if ( (m_destinationLocation != NULL) && 
           (strlen(m_destinationLocation) > 0)) {
         // Got a description (city or district) of the destination
         char* tmpStr=new char[strlen(m_destinationLocation)+2];
         strcpy(tmpStr, m_destinationLocation);
         strcat(tmpStr, ":");
         destPos += SMSStringUtility::printSMSString( 
                                    dest + destPos, tmpStr,
                                    language, eol,
                                    maxLineLength, 0, 4, false );
         dest[destPos] = '\0';
         delete [] tmpStr;
      }

      // Add destination company??
      if ( (m_destinationCompany != NULL) && 
           (strlen(m_destinationCompany) > 0)) {
         char* tmpStr=new char[strlen(m_destinationCompany)+2];
         strcpy(tmpStr, m_destinationCompany);
         strcat(tmpStr, ":");
         destPos += SMSStringUtility::printSMSString( 
                                    dest + destPos, tmpStr,
                                    language, eol,
                                    maxLineLength, 0, 4, false );
         dest[destPos] = '\0';
         delete [] tmpStr;
      }

      // Add destination street-name
      destPos += SMSStringUtility::printSMSString( 
                                 dest + destPos, getText(),
                                 language, eol,
                                 maxLineLength, 0, 4, false);
      if ( (m_destinationRoadNumber != NULL) &&
           (strlen(m_destinationRoadNumber) > 0)) {
         char* tmpStr=new char[strlen(m_destinationRoadNumber)+2];
         tmpStr[0] = ' ';
         tmpStr[1] = '\0';
         strcat(tmpStr,m_destinationRoadNumber); 
         destPos += SMSStringUtility::printSMSString( 
                                    dest + destPos, tmpStr,
                                    language, eol,
                                    maxLineLength, 0, 4, true );
         delete [] tmpStr;
      } else {
         destPos += SMSStringUtility::writeEol( dest, destPos, eol, 3 );
      }
      
   } else {
      
      // Print distance, exitCount, turndescription and eol to dest
      uint32 ec = getExitCount();
      if (ec > 9) { // don't print exitcount if > 9.
         ec = 0; 
      } 
      
      StringTable::getRouteDescription(
         getStringCode(),
         language,
         ec,
         turnDescription, 
         maxTurnDescriptionLength);
      destPos = StringTableUtility::printDistance(dest,
                                             getDist(),
                                             language,
                                             StringTableUtility::COMPACT,
                                             distUnit);
      DEBUG8(
         mc2dbg << "   dest=\"" << dest << "\"" << endl;
      );
      destPos += sprintf( dest + destPos, " %s", turnDescription );
      DEBUG8(
         mc2dbg << "   dest=\"" << dest << "\"" << endl;
      );
      destPos += SMSStringUtility::writeEol( dest, destPos, eol, 3 );
      DEBUG8(
         mc2dbg << "   dest=\"" << dest << "\"" << endl;
      );

      // Write the name
      if (hasName()) {
         destPos += SMSStringUtility::printSMSString(
                                       dest + destPos, getText(), 
                                       language, eol,
                                       maxLineLength, 0, 
                                       nbrRows, true);
      }
   }

   dest[destPos] = '\0';
   DEBUG4(
      mc2dbg << "   returning " << destPos << ", dest=\"" << dest 
             << "\""<< endl;
   );

   // Return the length of dest
   return (destPos);
}



bool
ExpandStringItem::addString(  const char* appendString,
                              char* buf,
                              uint32 maxLength,
                              uint32& totalNbrBytesWritten,
                              bool wapFormat )
{
   
   // Check if it is an empty appendstring.
   if (*appendString == '\0') {
      totalNbrBytesWritten = 0;
      return (true);
   }
   
   // Whether the appended string will fit in the buffer.
   bool willFit = true;
   
   // The number of bytes to write this time.
   uint32 nbrBytesToWrite;

   uint32 sizeOfBuf = strlen(buf);
   
   // This is the string to append, wap formatted if requested.
   char* fAppStr; // f(ormatted)App(end)Str(ing)
   if (wapFormat) {
      fAppStr = new char[MIN(maxLength, strlen(appendString)*7)];
      // Wap format the string.
      StringUtility::wapStr(fAppStr, appendString);
   } else {
      fAppStr = (char*) appendString;
   }

   uint32 sizeOfFAppStr = strlen(fAppStr);

   if (sizeOfBuf + sizeOfFAppStr >= maxLength) {
      // This won't fit.
      nbrBytesToWrite = maxLength - sizeOfBuf - 1;
      willFit = false;
   } else {
      nbrBytesToWrite = sizeOfFAppStr;
   }

   // Copy the string
   strncat(buf, fAppStr, nbrBytesToWrite);

   totalNbrBytesWritten = sizeOfBuf + nbrBytesToWrite;
   
   // Terminate the buffer
   buf[totalNbrBytesWritten + 1] = '\0';
   
   // Delete fAppStr if it was allocated here.
   if (wapFormat) {
      delete [] fAppStr;
   }
   
   // Return if it did fit in the buffer.
   almostAssert(willFit);
   return(willFit);

}

bool
ExpandStringItem::isEndOfRoadTurn() const
{
   if((m_dist > 0) && ((m_stringCode == StringTable::LEFT_TURN) ||
                       (m_stringCode == StringTable::RIGHT_TURN))){ 
      if((m_crossingKind == ItemTypes::CROSSING_3WAYS_T) &&
         (m_nbrPossTurns == 1) &&
         (m_possTurns[0] != StringTable::AHEAD_TURN)){
         return true;
      }
      
   }
   return false;
}

   
const char* 
ExpandStringItem::transportationTypeToString( 
   ItemTypes::transportation_t transportationType )
{
   switch ( transportationType ) {
      case ItemTypes::undefined :
         return "";
         break;
      case ItemTypes::drive :
         return "drive";
         break;
      case ItemTypes::walk :
         return "walk";
         break;
      case ItemTypes::bus :
         return "bus";
         break;
      case ItemTypes::bike:
         return "bike";
         break;
   }
   return "";
}
