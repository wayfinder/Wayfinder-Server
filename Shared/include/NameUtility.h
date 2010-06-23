/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAME_UTILITY_H
#define NAME_UTILITY_H

#include "config.h"
#include "ItemTypes.h"
#include "LangTypes.h"
#include "StringTable.h"

#include <vector>
#include <set>
#include <map>
#include <stdlib.h>

/**
 *   Class consisting of static functions to be used
 *   for selecting the best name of e.g. an Item or Searchable.
 */ 
class NameUtility {
public:
  /**
   *    Returns the best name for an item.
   * 
   *    @param nbrNames    The number of names in the array.
   *    @param stringIdx   The stringIndices.
   *    @param reqlang     The requested language to use.
   *    @param usedStrings Raw stringIdx for already used strings that
   *                       should not be used again. Selected index will
   *                       be added.
   *    @param acceptUniqueName Wether unique names should be returned,
   *                            @see ItemTypes::uniqueName.
   *    @return Index in array to the best name. -1 if no allowed names.
   */ 
  static inline int getBestName(int nbrNames,
                                const uint32* stringIdx,
                                LangTypes::language_t reqlang,
                                set<uint32>* usedStrings = NULL,
                                bool acceptUniqueName = true);

  
  /**
   *    Returns the best name for an item. This function will always
   *    return a name unless the number of names is 0.
   * 
   *    @param nbrNames  The number of names in the array.
   *    @param stringIdx The stringIndices.
   *    @param reqlangs  The requested languages to use.
   *    @param usedStrings Raw stringIdx for already used strings that
   *                       should not be used again. Selected index will
   *                       be added.
   *    @param acceptUniqueName Wether unique names should be returned,
   *                            @see ItemTypes::uniqueName. 
   *    @return Index in array to the best name. -1 if no allowed names.
   */ 
  static inline int getBestName(int nbrNames,
                                const uint32* stringIdx,
                                const vector<LangTypes::language_t>& reqLangs,
                                set<uint32>* usedStrings = NULL,
                                bool acceptUniqueName = true);

  /**
   *    Compare two languages and return large value
   *    if the difference is small. The order of the
   *    inparameters may be important in the future.
   *    <br />
   *    FIXME: Use better method inside.
   *    @return The number of points, maximum of maxStringPoints.
   */
  static inline uint32 getLangPoints(LangTypes::language_t itemLang,
                                     LangTypes::language_t reqLang);
                                                                              

  /**
   * Get the best languageCode for a language_t.
   *
   * @param lang The language_t to get best language for.
   * @param dist Set to the distance between the languages, 0 if same
   *             language in languageCode as in language_t.
   */
  static StringTable::languageCode getBestLanguage( 
     LangTypes::language_t lang, uint32& dist );


  /**
   *    Returns the best name of a certain type or -1 if there
   *    was no name of that type.
   *    @param nbrNames  The number of names in the array.
   *    @param stringIdx The stringIndices.
   *    @param reqlang   The requested language to use.
   *    @param reqType   The requeted type.
   *    @param usedStrings Raw stringIdx for already used strings that
   *                       should not be used again. Selected index will
   *                       be added.
   *    @return Index to best name or -1 if no name of the
   *            requested type.
   */
  static inline int getBestNameOfType(int nbrNames,
                                      const uint32* stringIdx,
                                      LangTypes::language_t reqlang,
                                      ItemTypes::name_t reqType,
                                      set<uint32>* usedStrings = NULL);  

  /**
   *    Returns the best name of a certain type.
   *    @param nbrNames  The number of names in the array.
   *    @param stringIdx The stringIndices.
   *    @param reqlang   The requested language to use.
   *    @param reqType   The requeted type.
   *    @param usedStrings Raw stringIdx for already used strings that
   *                       should not be used again. Selected index will
   *                       be added.
   *    @return Index to best name or -1 if no name of the
   *            requested type.
   */
  static inline int
     getBestNameOfType(int nbrNames,
                       const uint32* stringIdx,
                       const vector<LangTypes::language_t>& reqlangs,
                       ItemTypes::name_t reqType,
                       set<uint32>* usedStrings = NULL);

  /**
   *    Returns the index of the first name that has the
   *    requested type and language or -1 if no such name.
   *    @param nbrNames  The number of names.
   *    @param stringIdx Raw string indices.
   *    @param reqType   Requested type.
   *    @param reqLang   Requested language.
   *    @param usedStrings Raw stringIdx for already used strings that
   *                       should not be used again. Selected index will
   *                       be added.
   */
  static inline int getExactName(int nbrNames,
                                 const uint32* stringIdx,
                                 LangTypes::language_t reqLang,
                                 ItemTypes::name_t reqType,
                                 set<uint32>* usedStrings = NULL);

  /**
   *    Returns the index of a name and prefers names of the
   *    requested name and type.
   *    Calls getExactName(), then getBestNameOfType() and last
   *    getBestName().
   *    @param nbrNames  The number of names.
   *    @param stringIdx The raw string idxes.
   *    @param reqType   The requested type.
   *    @param reqLang   The requested language.
   *    @param usedStrings Raw stringIdx for already used strings that
   *                       should not be used again. Selected index will
   *                       be added.
   *    @return Index of the best name or -1 if no names.
   */
  static inline int getName(int nbrNames,
                            const uint32* stringIdx,
                            LangTypes::language_t reqLang,
                            ItemTypes::name_t reqType,
                            set<uint32>* usedStrings = NULL);     
  
  /**
   *    The maximum number of points that can be returned
   *    in getLangPoints.
   */
  static const uint32 maxLangPoints;


  typedef map< LangTypes::language_t, 
     pair< StringTable::languageCode, uint32 > > bestLanguageCodeMap;

   typedef map< LangTypes::language_t, map< LangTypes::language_t, int > > 
      PreferedLanguageCont;


  /**
   *  Returns the name priority in language code for a number name.
   *  This is to be able to use the language as a value of the priority
   *  of that number name.
   *  (language of roadNumbers was always invalidLanguage.)
   * 
   *  @param name Name string
   *  @return Language type representing name priority.
   */
  static inline LangTypes::language_t getNumberNameValue(const char* name);
 
  private:
  /**
   *   Inner function that is the same as getBestName, but it does
   *   not add the selected index to usedStrings.
   */
  static inline int innerGetBestName(int nbrNames,
                                     const uint32* stringIdx,
                                     LangTypes::language_t reqlang,
                                     set<uint32>* usedStrings,
                                     bool acceptUniqueName = true);

  /**
   *   Inner function that is the same as getBestNameOfType but
   *   does not add the selected index to usedStrings.
   */
  static inline int innerGetBestNameOfType(int nbrNames,
                                           const uint32* stringIdx,
                                           LangTypes::language_t reqlang,
                                           ItemTypes::name_t reqType,
                                           set<uint32>* usedStrings);


  /**
   * The best StringTable::languageCode for a LangTypes::language_t and
   * the distance for it.
   */
  static bestLanguageCodeMap m_bestLanguageCodeMap;

   /**
    * The languages to select before defaulting to english or just any
    * language.
    */
   static PreferedLanguageCont m_preferedLanguage;
};


inline uint32
NameUtility::getLangPoints(LangTypes::language_t reqLang,
                           LangTypes::language_t itemLang)
{
   if ( reqLang == itemLang ) {
      // Best
      return NameUtility::maxLangPoints;
   } else if ( itemLang == LangTypes::invalidLanguage ) {
      // Worst
      return 0;
   } else if ( itemLang == LangTypes::english ) {
      // English is the best language if not the requested was found.
      return NameUtility::maxLangPoints - 1;
   } else {
      // FIXME: This works in Finland where English
      //        chooses Finnish names over Swedish ones
      //        and Norwegian chooses Swedish.
      //        Use a method like the one in StringSearchUtility
      //        instead.
      return abs((int)(reqLang) - (int)(itemLang)) + 1;
   }
}

// Check if the stringInfo can be used.
#define CAN_USE(x) ( ( usedStrings == NULL ) ||\
                     ( usedStrings->find(x) == usedStrings->end() ) )

// Insert the string into the usedStrings
#define I_HAVE_USED(x)  do { if ( usedStrings != NULL) {\
                         usedStrings->insert(x); }} while (0)

inline int
NameUtility::innerGetBestNameOfType(int nbrNames,
                                    const uint32* stringIdx,
                                    LangTypes::language_t reqlang,
                                    ItemTypes::name_t reqType,
                                    set<uint32>* usedStrings)
{
   int bestPoints = -1;
   int bestIndex = -1;
   int i;
   PreferedLanguageCont::const_iterator prefLangs = m_preferedLanguage.find(
      reqlang );
   for( i = 0; i < nbrNames; i++) {
      uint32 stringInfo = stringIdx[i];
      if ( ! CAN_USE( stringInfo ) ) {
         continue;
      }
      ItemTypes::name_t stringType = GET_STRING_TYPE(stringInfo);
      LangTypes::language_t language = GET_STRING_LANGUAGE(stringInfo);
      if( stringType == reqType ) {
         if(reqType == ItemTypes::roadNumber){
            // Handle different as language means name priority.
            // If this is not set all roadNumber still have same value.
            int points = (int)language;
            if ( points > bestPoints ) {
               bestPoints = points;
               bestIndex  = i;
            }
         }
         else if ( language == reqlang ) {
            // Correct language - we can return now
            return i;
         } else {
            // Get the language points.
            int points = getLangPoints(reqlang, language);
            if ( prefLangs != m_preferedLanguage.end() ) {
               PreferedLanguageCont::mapped_type::const_iterator findIt =
                  (*prefLangs).second.find( language );
               if ( findIt != (*prefLangs).second.end() ) {
                  mc2dbg8 << "[NameU]: found pref lang "
                         << LangTypes::getLanguageAsString( language, true )
                         << " points " << MC2HEX( findIt->second ) << endl;
                  points = findIt->second;
               }
            }
            if ( points > bestPoints ) {
               bestPoints = points;
               bestIndex  = i;
            }
         }
      }
   }
   return bestIndex;
}

inline int
NameUtility::getBestNameOfType(int nbrNames,
                               const uint32* stringIdx,
                               LangTypes::language_t reqlang,
                               ItemTypes::name_t reqType,
                               set<uint32>* usedStrings)
{
   int idx = innerGetBestNameOfType(nbrNames, stringIdx, reqlang,
                                    reqType, usedStrings );
   if ( idx >= 0 ) {
      I_HAVE_USED( stringIdx[idx] );
   }
   
   return idx;
}

inline int
NameUtility::getBestNameOfType(int nbrNames,
                               const uint32* stringIdx,
                               const vector<LangTypes::language_t>& reqlangs,
                               ItemTypes::name_t reqType,
                               set<uint32>* usedStrings)
{
   int bestIdx = -1;
   // Initialized later if bestIdx < 0.
   uint32 bestPoints = 0;
   for ( uint32 i = 0; i < reqlangs.size(); ++i ) {
      int idx = innerGetBestNameOfType(nbrNames, stringIdx, reqlangs[i],
                                       reqType, usedStrings);
      // Check if ok.
      if ( idx < 0 ) {
         // No names of that type.
         return idx;
      }
      
      uint32 nbrPoints = getLangPoints(GET_STRING_LANGUAGE(stringIdx[idx]),
                                       reqlangs[i]);
      if ( (bestIdx < 0) ||
           (nbrPoints > bestPoints) ) {
         bestIdx = i;
         bestPoints = nbrPoints;
      }      
   }
   if ( bestIdx >= 0 ) {
      I_HAVE_USED( stringIdx[bestIdx] );
   }
   return bestIdx;
}

inline int
NameUtility::innerGetBestName(int nbrNames,
                              const uint32* stringIdx,
                              LangTypes::language_t reqlang,
                              set<uint32>* usedStrings,
                              bool acceptUniqueName)
{
   int i = 0;
   mc2dbg8 << "[NameU]: Requested language is "
           << LangTypes::getLanguageAsString(reqlang, true)
           << endl;

   if ( nbrNames == 1 ) {
      if ( CAN_USE( stringIdx[0] ) ) {
         mc2dbg8 << "[NameU]: Taking only name " << endl;
         return i;
      } else {
         return -1;
      }
   } else if ( nbrNames == 0 ) {
      return -1;
   }
   
   // Unique names are probably the best.
   if (acceptUniqueName) {
      for( i = 0; i < nbrNames; i++) {
         uint32 stringInfo = stringIdx[i];
         if ( ! CAN_USE ( stringInfo ) ) {
            continue;
         }
         uint32 stringType = GET_STRING_TYPE(stringInfo);
         if( stringType == ItemTypes::uniqueName ) {
            mc2dbg8 << "[NameU]: Found unique name" << endl;
            return i;
         }
      }
   }

   for( i = 0; i < nbrNames; i++) {
      uint32 stringInfo = stringIdx[i];
      if ( ! CAN_USE ( stringInfo ) ) {
         continue;
      }
      LangTypes::language_t language = GET_STRING_LANGUAGE(stringInfo);
      uint32 nameType = GET_STRING_TYPE(stringInfo);
      if( (nameType == ItemTypes::officialName) &&
          (language == reqlang) ) {
         mc2dbg8 << "[NameU]: Found official name correct lang" << endl;
         return i;
      }
   }     

   for( i = 0; i < nbrNames; i++) {
      uint32 stringInfo = stringIdx[i];
      if ( ! CAN_USE ( stringInfo ) ) {
         continue;
      }
      LangTypes::language_t language = GET_STRING_LANGUAGE(stringInfo);
      uint32 nameType = GET_STRING_TYPE(stringInfo);
      if( (nameType == ItemTypes::alternativeName) &&
          (language == reqlang) ) {
         mc2dbg8 << "[NameU]: Found alternative name correct lang" << endl;
         return i;
      }
   }     

   int tmpIdx = getBestNameOfType(nbrNames, stringIdx, reqlang,
                                  ItemTypes::officialName, usedStrings );
   if ( tmpIdx >= 0 ) {
      mc2dbg8 << "[NameU]: Found nice official name in "
             << LangTypes::getLanguageAsString(
                GET_STRING_LANGUAGE(stringIdx[tmpIdx]), true)
             << endl;
      return tmpIdx;
   }

   // We will not get here.
   for( i = 0; i < nbrNames; i++) {
      uint32 stringInfo = stringIdx[i];
      if ( ! CAN_USE ( stringInfo ) ) {
         continue;
      }
      uint32 language = GET_STRING_LANGUAGE(stringInfo);
      uint32 nameType = GET_STRING_TYPE(stringInfo);
      if( (nameType == ItemTypes::officialName) &&
          (language == LangTypes::english) ) {
         mc2dbg8 << "[NameU]: Found official name in english" << endl;
         return i;
      }
   }

   for( i = 0; i < nbrNames; i++) {
      uint32 stringInfo = stringIdx[i];
      if ( ! CAN_USE ( stringInfo ) ) {
         continue;
      }
      uint32 nameType = GET_STRING_TYPE(stringInfo);
      if( nameType == ItemTypes::officialName ) {
         mc2dbg8 << "[NameU]: Found official name" << endl;
         return i;
      }
   }

   for( i = 0; i < nbrNames; i++) {
      uint32 stringInfo = stringIdx[i];
      if ( ! CAN_USE ( stringInfo ) ) {
         continue;
      }
      uint32 language = GET_STRING_LANGUAGE(stringInfo);
      uint32 nameType = GET_STRING_TYPE(stringInfo);
      if( (nameType == ItemTypes::alternativeName) &&
          (language == LangTypes::english) ) {
         mc2dbg8 << "[NameU]: Found alternative name in english" << endl;
         return i;
      }
   }

   for( i = 0; i < nbrNames; i++) {
      uint32 stringInfo = stringIdx[i];
      if ( ! CAN_USE ( stringInfo ) ) {
         continue;
      }
      uint32 nameType = GET_STRING_TYPE(stringInfo);
      if( nameType == ItemTypes::alternativeName ) {
         mc2dbg8 << "[NameU]: Found alternative name" << endl;
         return i;
      }
   }
   
   int bestPoints = -1;
   int bestIndex  = -1;
   for( i = 0; i < nbrNames; i++) {
      uint32 stringInfo = stringIdx[i];
      if ( ! CAN_USE ( stringInfo ) ) {
         continue;
      }
      uint32 nameType = GET_STRING_TYPE(stringInfo);
      if( nameType == ItemTypes::roadNumber ) {
         mc2dbg8 << "[NameU]: Found roadnumber" << endl;
         int points = (int)(GET_STRING_LANGUAGE(stringInfo));
         if(points > bestPoints){
            bestPoints = points;
            bestIndex  = i;
         }
      }
      if(bestIndex != -1) // There might be no roadnumber names.
         return bestIndex;
   }
   
   // return the first allowed name
   for ( i = 0; i < nbrNames; ++i ) {
      if ( CAN_USE( stringIdx[i] ) ) {
         mc2dbg8 << "[NameU]: Taking first allowed name" << endl;
         return i;
      }
   }
   // No allowed name found.
   return -1;
}

inline int
NameUtility::getBestName(int nbrNames,
                         const uint32* stringIdx,
                         LangTypes::language_t reqlang,
                         set<uint32>* usedStrings,
                         bool acceptUniqueName)
{
   int idx = innerGetBestName(nbrNames, stringIdx,
                              reqlang, usedStrings,
                              acceptUniqueName);
   if ( idx >= 0 ) {
      I_HAVE_USED( stringIdx[idx] );
   }
   return idx;
}

inline int
NameUtility::getBestName(int nbrNames,
                         const uint32* stringIdx,
                         const vector<LangTypes::language_t>& reqLangs,
                         set<uint32>* usedStrings,
                         bool acceptUniqueName)
{
   // Check if there are zero names
   if ( nbrNames == 0 ) {
      return nbrNames - 1;
   }
   
   int bestIdx = -1;
   // Initialized later if bestIdx < 0.
   uint32 bestPoints = 0;
   for( uint32 i = 0; i < reqLangs.size(); ++i ) {
      int idx = innerGetBestName(nbrNames, stringIdx,
                                 reqLangs[i], usedStrings,
                                 acceptUniqueName);
      if ( idx < 0 ) {
         continue;
      }                        
      uint32 nbrPoints = getLangPoints(GET_STRING_LANGUAGE(stringIdx[idx]),
                                       reqLangs[i]);
      if ( (bestIdx < 0) ||
           (nbrPoints > bestPoints) ) {
         bestIdx = i;
         bestPoints = nbrPoints;
      }
   }
   if ( bestIdx >= 0 ) {
      I_HAVE_USED( stringIdx[bestIdx] );
   }
   return bestIdx;
}

inline int
NameUtility::getExactName(int nbrNames,
                          const uint32* stringIdx,
                          LangTypes::language_t reqLang,
                          ItemTypes::name_t reqType,
                          set<uint32>* usedStrings)
{
   for ( int i = 0; i < nbrNames; ++i ) {
      if ( ! CAN_USE ( stringIdx[i] ) ) {
         continue;
      }
      LangTypes::language_t language = GET_STRING_LANGUAGE(stringIdx[i]);
      ItemTypes::name_t nameType     = GET_STRING_TYPE(stringIdx[i]);
      if ( language == reqLang ) {
         if ( nameType == reqType ) {
            I_HAVE_USED ( stringIdx[i] );
            return i;
         }
      }
   }
   // Not found.
   return -1;
}

inline int
NameUtility::getName(int nbrNames,
                     const uint32* stringIdx,
                     LangTypes::language_t reqLang,
                     ItemTypes::name_t reqType,
                     set<uint32>* usedStrings)
{
   // N.B! Adds the index to usedStrings if used.
   int idx = getExactName(nbrNames, stringIdx, reqLang, reqType, usedStrings);
   // Return if ok
   if ( idx >= 0 ) {
      return idx;
   }
   // N.B! Adds the index to usedStrings if used.
   idx = getBestNameOfType(nbrNames, stringIdx, reqLang, reqType, usedStrings);
   // Return if ok
   if ( idx >= 0 ) {
      return idx;
   } else {
      // N.B! Adds the index to usedStrings if used.
      return getBestName(nbrNames, stringIdx, reqLang, usedStrings);
   }   
}

inline LangTypes::language_t
NameUtility::getNumberNameValue(const char* name){
      // if number name change language to indicate name priority.
      // 0 - Number roads without a character.
      // 1.- Number roads without a new character.
      // 2.- K roads
      // 3.- L roads
      // 4.- U roads
      // 5.- E roads
      // 6.- B & N roads
      // 7.- A roads
      // 8.- M roads
   if(name == NULL)
      return LangTypes::invalidLanguage;
   switch (name[0]){
      case 'M' :
         return LangTypes::language_t(8);
         break;
      case 'A' :
         return LangTypes::language_t(7);
            break;
      case 'B' :
         return LangTypes::language_t(6);
         break;
      case 'N' :
         return LangTypes::language_t(6);
         break;
      case 'E' :
         return LangTypes::language_t(5);
         break;
      case 'U' :
         return LangTypes::language_t(4);
         break;
      case 'L' :
         return LangTypes::language_t(3);
         break;
      case 'K' :
         return LangTypes::language_t(2);
         break;
      default :
         return LangTypes::language_t(0);
         break;
   }
}

#endif
