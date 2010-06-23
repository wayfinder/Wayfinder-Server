/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TalkerUtility.h"
#include "ScopedArray.h"
#include "ItemInfoFilter.h"

namespace TalkerUtility {
/// adds information from a string
template <>
void AddInfo::add<MC2String>( const MC2String& value,
                              ItemInfoEnums::InfoType typeOfInfo ) {
   
   // Adds info to the SearchMatch.
   if ( value.empty() ) {
      return;
   }
   const char* keyStr = ItemInfoEnums::infoTypeToString( m_lang,
                                                         typeOfInfo,
                                                         NULL );
   if ( keyStr == NULL ) {
      return;
   }

   if ( ! ItemInfoFilter::includeItemInfo( typeOfInfo, m_filterLevel ) ) {
      // Don't include this
      m_additionalInfoExists = true;
      return;
   }
 
   m_infoVect.push_back( ItemInfoEntry( StringUtility::
                                        makeFirstInWordCapital( keyStr ),
                                        value, typeOfInfo ) );
}

template <>
void AddInfo::add< MC2String >( const MC2String& value, 
                                const MC2String& value2,
                                ItemInfoEnums::InfoType typeOfInfo ) {
   
   // Adds info to the SearchMatch.
   if ( value.empty() ) {
      return;
   }
   const char* keyStr = ItemInfoEnums::infoTypeToString( m_lang,
                                                         typeOfInfo,
                                                         NULL );
   if ( keyStr == NULL ) {
      return;
   }
   
   if ( ! ItemInfoFilter::includeItemInfo( typeOfInfo, m_filterLevel ) ) {
      // Don't include this
      m_additionalInfoExists = true;
      return;
   }
   
   char buffer[256];
   sprintf( buffer, keyStr, value2.c_str() );
   
   m_infoVect.push_back( ItemInfoEntry( StringUtility::
                                        makeFirstInWordCapital( buffer ),
                                        value, typeOfInfo ) );
}

/**
 * Adds information from a vector of strings
 */
template <>
void AddInfo::add< vector<MC2String> >( const vector<MC2String>& container,
                                        ItemInfoEnums::InfoType type  ) {
   vector<MC2String>::const_iterator it = container.begin();
   vector<MC2String>::const_iterator itEnd = container.end();
   for ( ; it != itEnd; ++it ) {
      add( *it, type );
   }
}

   auto_ptr<XercesDOMParser> parseXMLSource( const MC2String& url, 
                                          const MC2String& xmlSource,
                                          bool ignoreInitialXML ) {

   auto_ptr<XercesDOMParser> xmlParser( new XercesDOMParser() );
   xmlParser->setValidationScheme( XercesDOMParser::Val_Auto );
   xmlParser->setIncludeIgnorableWhitespace( false );

   const char* subStr = strstr( xmlSource.c_str(), "<?xml" );
   if ( subStr == NULL ) {
      mc2dbg << "[TalkerUtility] no <?xml tag found. " << endl;
      if ( ignoreInitialXML ) {
         subStr = xmlSource.c_str();
      } else {
         return auto_ptr<XercesDOMParser>();
      }
   }

   MC2String tmp_xml_string = subStr;

   XStr str( url.c_str() );
   MemBufInputSource 
      source( reinterpret_cast<const XMLByte*>( tmp_xml_string.data() ), 
              tmp_xml_string.length(),
              str.XMLStr() );

   xmlParser->parse( source );

   return xmlParser;
}


MC2String getIntPhoneNumber( const MC2String& number,
                             StringTable::countryCode country ) {
   ScopedArray<char> nondigits( StringUtility::
                                removeAllButDigits( number.c_str() ) );

   MC2String trimmedNumber = nondigits.get();
   // remove the first '0' before adding international number,
   // except for italy which does not need this according to:
   // http://en.wikipedia.org/wiki/Telephone_numbers_in_Italy
   if ( trimmedNumber[0] == '0' &&
        country != StringTable::ITALY_CC ) {
      trimmedNumber.erase( 0, 1 );
   }

   return "+" + MC2String( StringTable::getCountryPhoneCode( country ) ) + 
      trimmedNumber;
}

template <>
vector<MC2String> convertToIntPhoneNumber( const vector<MC2String>& numbers,
                                           StringTable::countryCode country ) {
   if ( numbers.empty() ) {
      return vector<MC2String>();
   }
   vector<MC2String> outnums( numbers.size() );
   for ( uint32 i = 0; i < numbers.size(); ++i ) {
      outnums[ i ] = getIntPhoneNumber( numbers[ i ], country );
   }
   return outnums;
}

template <>
MC2String convertToIntPhoneNumber( const MC2String& number,
                                   StringTable::countryCode country ) {
   if ( number.empty() ) {
      return MC2String();
   }
   return getIntPhoneNumber( number, country );
}


}
