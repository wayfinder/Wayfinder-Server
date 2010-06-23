/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCH_HEADING_DESC_H
#define SEARCH_HEADING_DESC_H

#include "config.h"
#include "CompactSearch.h"
#include "ImageTable.h"

#include <map>
#include <stdexcept>

class UserUser;
class ExternalSearchDescGenerator;

/**
 * Creates and handles client specific header list.
 */
class SearchHeadingDesc {
public:

   /** Special headings such as favorites and phonebook.
    * Do not change these numbers.
    */
   enum SpecialHeadings {
      FAVORITES_HEADING = 100, ///< favorites
      PHONEBOOK_HEADING,        ///< phonebook in the client
      MERGED_RESULTS_HEADING,  ///< merged results for "one list search"
      ADVERTISEMENT_HEADING = 1000 ///< advertisement heading
   };

   typedef uint32 HeadingID;
   typedef uint32 ServiceID;

   /// Invalid heading ID number
   static const HeadingID INVALID_HEADING_ID = MAX_UINT32;

   /// @param imageSet The image set to use for the search heading icons.
   explicit SearchHeadingDesc( ImageTable::ImageSet imageSet,
                               const CompactSearchHitTypeVector& headings );

   /**
    * Tries to find a heading ID for a specific service ID
    * @param serviceID the service ID to find a heading for
    * @return heading id for the specified heading or MAX_UINT32 on failure
    */
   HeadingID findServiceHeading( ServiceID service ) const;

   /**
    * Tries to find a heading type for a specific service ID.
    *
    * @param serviceID The service ID to find a heading for.
    * @return heading Type for the specified heading or invalid heading on 
    *                 failure.
    */
   CompactSearchHitType findServiceHeadingType( ServiceID serviceID ) const;

   /**
    * @return true if the user has the rights to search this heading. (Only for
    *         round 0 atm)
    */
   static bool canSearchHeading( const CompactSearchHitType& heading,
                                 const UserUser& user );

   /**
    * Tries to find a search hit description from a heading
    * @param heading the heading id to search for
    * @return compact search hit type, the name of the hit type will be empty
    *         if it failed to find a heading
    */
   CompactSearchHitType getHitTypeFromHeading( HeadingID heading ) const;

   /**
    * Finds and returns a \c heading. Will throw exception if the \c heading id
    * is invalid.
    * @param id A heading ID.
    * @return reference to heading.
    */
   const CompactSearchHitType&
   getHeading( HeadingID id ) const throw ( std::out_of_range );

   /**
    * @return true if heading ID is \c special
    */
   static bool isSpecialHeading( HeadingID heading );

   /**
    * Translate a compact search heading.
    * @param hitType The heading to translate.
    * @param language Language to translate to.
    * @param descGen Generator for external services.
    */ 
   void translateHitType( CompactSearchHitType& hitType,
                          LangType::language_t language) const;

   /**
    * Get headings that are valid for a \c user.
    * @param user The user that has rights for some headings.
    * @param heading Will be filled with the headings that the user can search
    *                in.
    */
   void getHeadings( const UserUser& user,
                     CompactSearchHitTypeVector& headings ) const;

   /**
    * Translate all the headings
    * @param user The current user, used to fetch rights for.
    * @param language Language to translate to.
    * @param includeSpecialHeadings Whether or not to include special headings.
    * @param hitTypes Will contain the translated headings.

    */
   void getTranslatedHeadings( const UserUser& user,
                               LangType::language_t language,
                               bool includeSpecialHeadings,
                               CompactSearchHitTypeVector& hitTypes ) const;

   /**
    * Tries to find a heading ID for a specific searchType in round 0.
    *
    * @param searchType The type that is in a round 0 search.
    * @return Heading id for the specified heading or MAX_UINT32 on failure.
    */
   HeadingID findSearchTypeHeading( uint32 searchType ) const;

   /**
    * Tries to find a heading for a specific searchType in round 0.
    *
    * @param searchType The type that is in a round 0 search.
    * @return Heading for the specified heading or MAX_UINT32 on failure.
    */
   CompactSearchHitType findSearchTypeHeadingType( uint32 searchType ) const;

private:
   typedef uint32 HeadingIndex;
   typedef map< HeadingID, HeadingIndex > HeadingIDToHeadingIndex;

   /// maps heading id to heading index in the heading vector
   HeadingIDToHeadingIndex m_headingIDToHeadingIndex;

   /// contains all search headings
   CompactSearchHitTypeVector m_headings;
   
   /// Map to find heading from ExtService.
   map< ServiceID, HeadingIndex > m_externalToHeadingIndex;
};

#endif // SEARCH_HEADING_DESC_H
