/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVLATESTNEWSDATA_H
#define NAVLATESTNEWSDATA_H

#include "config.h"
#include "ItemTypes.h"
#include "WFSubscriptionConstants.h"
#include "FOB.h"
#include "StringUtility.h"
#include "LangTypes.h"

class ParserThreadGroup;


/**
 * Class for holding a latest news image/file.
 *
 */
class NavLatestNewsData {
   public:
      /**
       * Constructor.
       *
       * @param latestNewsFile The filepatch to the latest news file.
       *                       Is copied.
       * @param clientType The client type of the latest news image.
       * @param lang The language of the latest news image.
       * @param wfSubscriptionType The Wayfinder subscription type of the
       *                           lastest news.
       * @param daysLeft The number of days left for subscription.
       * @param imageExtension The image extension.
       * @param recheckTime The time before a recheck of the file on disk
       *                    is needed, default 3600. In seconds.
       */
      NavLatestNewsData( const char* latestNews,
                         const char* clientType,
                         LangTypes::language_t lang,
                         WFSubscriptionConstants::subscriptionsTypes
                         wfSubscriptionType,
                         uint16 daysLeft,
                         const char* imageExtension,
                         ParserThreadGroup* group,
                         uint32 recheckTime = 3600 );


      /**
       * Destructor.
       */
      virtual ~NavLatestNewsData();


      /**
       * Loads the latest news.
       * Keeps the old file if load fails.
       *
       * @return If loaded successfull, false if not.
       */
      bool loadFile();


      /**
       * Rechecks file on disk if recheckTime has passed since last check.
       *
       * @return True if all is ok, false if loadFile failed.
       */
      bool recheckFile();


      /**
       * Get the last news.
       */
      const byte* getLastNews() const;


      /**
       * Get the length of last news.
       */
      uint32 getLastNewsLength() const;


      /**
       * Get the file path.
       */
      const char* getFilePatch() const;


      /**
       * Get the crc.
       */
      uint32 getCRC() const;


      /**
       * Get the number of different days left values.
       */
      static uint32 getNbrDaysLeftValues();


      /**
       * Get the days left value at index.
       */
      static uint16 getDaysLeftValue( uint32 index );


      /**
       * Get the closest larger days left value for a value.
       */
      static uint16 getClosestDaysLeftValue( uint16 value );


   protected:
      /**
       * The last news file.
       */
      const byte* m_lastNews;


      /**
       * The file.
       */
      aFOB m_file;


      /**
       * The length of last news.
       */
      uint32 m_lastNewsLength;


      /**
       * The CRC for the latest new file.
       */
      uint32 m_crc;


      /**
       * The filepatch to last news file.
       */
      char* m_lastNewsFile;


      /**
       * The client type of the latest news image.
       */
      char* m_clientType;


      /**
       * The language of the latest news image.
       */
      LangTypes::language_t m_lang;


      /**
       * The Wayfinder subscription type of the lastest news.
       */
      WFSubscriptionConstants::subscriptionsTypes m_wfSubscriptionType;


      /**
       * The number of days left for subscription.
       */
      uint16 m_daysLeft;


      /**
       * The image extension.
       */
      char* m_imageExtension;


      /**
       * The time before rechecking.
       */
      uint32 m_recheckTime;


      /**
       * Time of last news load.
       */
      uint32 m_lastNewsLoadTime;

   private:
      /**
       * The number of days left values.
       */
      static const uint32 m_nbrDaysLeftValues;


      /**
       * The days left values.
       */
      static const uint16 m_daysLeftValues[];


      /**
       * NavLatestNewsLessComp may look at the variables.
       */
      friend class NavLatestNewsLessComp;


      /**
       * The group.
       */
      ParserThreadGroup* m_group;
};


/**
 * Subclass to NavLatestNewsData used for searching.
 *
 */
class SearchNavLatestNewsData : public NavLatestNewsData {
   public:
      /**
       * Constructor.
       */
      SearchNavLatestNewsData();


      /**
       * Set the data.
       * 
       * @param clientType The client type of the latest news image.
       * @param lang The language of the latest news image.
       * @param wfSubscriptionType The Wayfinder subscription type of the
       *                           lastest news.
       * @param daysLeft The number of days left for subscription.
       * @param imageExtension The image extension.
       */
      void setData( const char* clientType,
                    LangTypes::language_t lang,
                    WFSubscriptionConstants::subscriptionsTypes
                    wfSubscriptionType,
                    uint16 daysLeft,
                    const char* imageExtension );


      /**
       * Unsets the data, sets to NULL.
       */
      void unSetData();
};


// =======================================================================
//                                     Implementation of inlined methods =


inline uint32 
NavLatestNewsData::getCRC() const {
   return m_crc;
}


#endif // NAVLATESTNEWSDATA_H

