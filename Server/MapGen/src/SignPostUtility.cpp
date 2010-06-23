/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SignPostUtility.h"
#include "GMSSignPost.h"
#include "GDColor.h"
#include "GenericMap.h"
#include "OldGenericMap.h"
#include "LangTypes.h"
#include "GDColor.h"

#include <map>


using namespace GDUtils::Color;

namespace SignPostUtility {

typedef pair< StringTable::countryCode,
              GMSSignPostElm::ElementClass > CountryAndElement;

struct SignData {
   SignData( const SignColors& c, uint32 p,
             uint32 _shieldRouteType )
      : colors( c ), prio( p ),
        shieldRouteType( _shieldRouteType )
   {}

   SignColors colors;
   uint32 prio;
   uint32 shieldRouteType;
};

typedef map< CountryAndElement, SignData > CountryAndElementClassColorCont;

struct SignColor {
   GMSSignPostElm::ElementClass m_class;
   imageColor m_text;
   imageColor m_front;
   imageColor m_back;
} g_signColorData[] = {
   { 5, WHITE, WHITE, SEAGREEN },
   { 200, WHITE, WHITE, BLUE },
   { 205, WHITE, RED, RED },
   { 210, WHITE, SEAGREEN, SEAGREEN },
   { 300, YELLOW, WHITE, BLUE },
   { 305, YELLOW, WHITE, SEAGREEN },
   { 310, WHITE, BLUE, BLUE },
   { 400, YELLOW, SEAGREEN, SEAGREEN },
   { 405, BLACK, BLACK, WHITE },
   { 410, WHITE, BLUE, BLUE },
   { 415, WHITE, BLUE, BLUE },
   { 420, WHITE, BLUE, WHITE },
   { 425, YELLOW, YELLOW, SEAGREEN },
   { 430, YELLOW, YELLOW, SEAGREEN },
   { 435, YELLOW, SEAGREEN, SEAGREEN },
   { 440, YELLOW, SEAGREEN, SEAGREEN },
   { 445, YELLOW, SEAGREEN, SEAGREEN },
   { 450, YELLOW, SEAGREEN, SEAGREEN },
   { 455, WHITE, RED, RED },
   { 500, WHITE, WHITE, BLUE },
   { 505, WHITE, WHITE, BLUE },
   { 510, WHITE, BLUE, BLUE },
   { 600, BLACK, BLACK, WHITE },
   { 610, BLACK, WHITE, WHITE },
   { 615, WHITE, WHITE, BLUE },
   { 1000, WHITE, WHITE, SEAGREEN }, //not square
   { 1005, BLACK, SEAGREEN, WHITE },
   { 1055, BLACK, BLACK, WHITE },
   { 1700, WHITE, WHITE, RED },
   { 1705, WHITE, WHITE, BLUE },
   { 1800, WHITE, WHITE, RED },
   { 1805, BLACK, BLACK, YELLOW },
   { 1810, BLACK, BLACK, WHITE },
   { 1815, SEAGREEN, SEAGREEN, WHITE },
   { 1905, WHITE, WHITE, BLUE },
   { 1910, WHITE, WHITE, RED },
   { 2000, WHITE, WHITE, BLUE },
   { 2005, BLACK, BLACK, YELLOW },
   { 2010, BLACK, WHITE, WHITE },
   { 2100, BLACK, BLACK, YELLOW },
   { 2105, BLACK, BLACK, YELLOW },
   { 2110, BLACK, BLACK, WHITE },
   { 2200, WHITE, WHITE, RED },
   { 2205, BLACK, BLACK, ORANGE },
   { 2210, BLACK, BLACK, WHITE },
   { 2215, BLACK, BLACK, WHITE },
   { 2300, WHITE, WHITE, RED },
   { 2400, WHITE, WHITE, RED },
   { 2405, BLACK, BLACK, YELLOW },
   { 2410, BLACK, BLACK, WHITE },
   { 2415, WHITE, WHITE, BLUE },
   { 2500, WHITE, WHITE, RED },
   { 2505, WHITE, WHITE, RED },
   { 2510, BLACK, BLACK, ORANGE },
   { 2600, WHITE, WHITE, BLUE },
   { 2605, YELLOW, SEAGREEN, SEAGREEN },
   { 2610, BLACK, BLACK, WHITE },
   { 2700, WHITE, WHITE, SEAGREEN },
   { 2705, WHITE, WHITE, BLUE },
   { 2800, BLACK, BLACK, YELLOW },
   { 2900, WHITE, WHITE, BLUE },
   { 2905, BLACK, BLACK, YELLOW },
   { 2910, WHITE, WHITE, SEAGREEN },
   { 3000, WHITE, WHITE, BLUE },
   { 3005, WHITE, WHITE, SEAGREEN },
   { 3010, WHITE, WHITE, BLUE },
   { 3100, WHITE, WHITE, BLUE },
   { 3105, YELLOW, WHITE, SEAGREEN },
   { 3110, BLACK, WHITE, WHITE },
   { 3200, WHITE, WHITE, SEAGREEN },
   { 3205, WHITE, WHITE, BLUE },
   { 3210, WHITE, WHITE, BLUE },
   { 3215, WHITE, WHITE, BLUE },
   { 3220, BLACK, BLACK, WHITE },
   { 3225, WHITE, WHITE, SEAGREEN },
   { 3400, WHITE, BLACK, RED },
   { 3405, BLACK, BLACK, ORANGE },
   { 3500, WHITE, WHITE, BLUE },
   { 3505, WHITE, BLACK, ORANGE },
   { 3510, BLACK, BLACK, YELLOW },
   { 3515, WHITE, WHITE, BLUE },
   { 3600, WHITE, RED, RED },
   { 3605, WHITE, WHITE, BLUE },
   { 3800, BLACK, BLACK, WHITE },
   { 3805, BLACK, BLACK, WHITE },
   { 4000, WHITE, WHITE, RED },
   { 4005, BLACK, BLACK, YELLOW },
   { 4010, BLACK, BLUE, WHITE },
   { 4015, WHITE, RED, RED },
   { 4105, BLACK, BLACK, WHITE },
   { 4110, BLACK, BLACK, WHITE },
   { 4200, WHITE, WHITE, RED },
   { 4205, BLACK, BLACK, YELLOW },
   { 4300, WHITE, WHITE, BLUE },
   { 4305, WHITE, WHITE, RED },
   { 4310, BLACK, BLACK, WHITE },
   { 4315, BLACK, BLACK, WHITE },
   { 4320, BLACK, BLACK, WHITE },
   { 4325, BLACK, YELLOW, WHITE },
   { 4330, BLACK, BLACK, WHITE },
   { 4400, WHITE, BLUE, RED }, //not square 
   { 4500, WHITE, WHITE, SEAGREEN },
   { 4505, WHITE, WHITE, BLUE },
   { 4510, WHITE, WHITE, BLUE },
   { 4515, WHITE, WHITE, BLUE },
   { 4700, YELLOW, SEAGREEN, SEAGREEN }, //missing figure
   { 4755, BLACK, SEAGREEN, WHITE }, //not square
   { 4800, WHITE, WHITE, RED },
   { 4805, WHITE, WHITE, BLUE },
   { 4900, WHITE, WHITE, SEAGREEN },
   { 4905, WHITE, WHITE, BLUE },
   { 5000, WHITE, WHITE, BLUE },
   { 5005, WHITE, WHITE, BLUE },
   { 5105, WHITE, WHITE, BLUE },
   { 5110, BLACK, WHITE, WHITE },
   { 5200, WHITE, WHITE, ORANGE },
   { 5205, WHITE, WHITE, BLUE },
   { 5210, BLACK, BLACK, WHITE },
   { 5300, BLACK, SEAGREEN, WHITE }, //not square
   { 5310, WHITE, WHITE, BLUE }, //not square
   { 5315, BLACK, BLACK, WHITE },
   { 5320, BLACK, BLACK, WHITE }, //not square
   { 5400, WHITE, WHITE, BLUE }, //not square
   { 5450, BLACK, BLACK, WHITE },
};

// place holder for the above data
STLUtility::Array< const SignColor >
g_signColors( g_signColorData,
              sizeof ( g_signColorData ) /
              sizeof ( g_signColorData[ 0 ] ) );

struct CompareElementClass {
   bool operator () ( GMSSignPostElm::ElementClass classType,
                      const SignColor& color ) const {
      return classType < color.m_class;
   }

   bool operator () ( const SignColor& color,
                      GMSSignPostElm::ElementClass classType ) const {
      return color.m_class < classType;
   }

   bool operator () ( const SignColor& lhs,
                      const SignColor& rhs ) const {
      return lhs.m_class < rhs.m_class;
   }
};

} // SignPostUtility

SignPostUtility::CountryAndElementClassColorCont initColors() {
   using namespace SignPostUtility;

#define I( shieldRouteType, c, e, p, t, f, b ) \
   if ( !i.insert( make_pair( CountryAndElement( c, e ), \
                              SignData( SignColors( t, f, b ), p, \
                                        shieldRouteType ) ) ).second ) { \
      mc2log << fatal << "SignPostUtility entry not unique! Country " << int(c) \
             << " elementClass " << e << endl; \
      MC2_ASSERT( false ); \
   }


   MC2_ASSERT( is_sorted( g_signColors.begin(), g_signColors.end(),
                          CompareElementClass() ) );

   SignPostUtility::CountryAndElementClassColorCont i;
   typedef StringTable ST;
   using namespace GDUtils::Color;

   // SEAGREEN
   // MEDIUMSEAGREEN
   

   // ShieldRouteType Country      Type Prio Text   Front  Back
   I( 5, ST::ALBANIA_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   I( 5, ST::ANDORRA_CC, 1, 3, WHITE, WHITE, SEAGREEN );
   I( 200, ST::ANDORRA_CC, 2, 1, WHITE, BLUE, BLUE );
   I( 205, ST::ANDORRA_CC, 3, 2, WHITE, RED, RED );
   I( 210, ST::ANDORRA_CC, 4, 4, WHITE, SEAGREEN, SEAGREEN );

   I( 300, ST::UAE_CC, 1, 1, YELLOW, WHITE, BLUE );
   I( 305, ST::UAE_CC, 2, 2, YELLOW, WHITE, SEAGREEN );
   I( 310, ST::UAE_CC, 3, 3, WHITE, BLUE, BLUE );

   I( 400, ST::AUSTRALIA_CC, 1, 1, YELLOW, SEAGREEN, SEAGREEN );
   I( 405, ST::AUSTRALIA_CC, 2, 1, BLACK, BLACK, WHITE );
   I( 415, ST::AUSTRALIA_CC, 3, 2, WHITE, BLUE, BLUE );
   I( 420, ST::AUSTRALIA_CC, 4, 3, WHITE, BLUE, WHITE );
   I( 410, ST::AUSTRALIA_CC, 5, 4, WHITE, BLUE, BLUE );
   I( 430, ST::AUSTRALIA_CC, 6, 5, YELLOW, YELLOW, SEAGREEN );
   I( 450, ST::AUSTRALIA_CC, 7, 6, YELLOW, SEAGREEN, SEAGREEN );
   I( 425, ST::AUSTRALIA_CC, 8, 7, YELLOW, YELLOW, SEAGREEN );
   I( 435, ST::AUSTRALIA_CC, 9, 8, YELLOW, SEAGREEN, SEAGREEN );
   I( 440, ST::AUSTRALIA_CC, 10, 9, YELLOW, SEAGREEN, SEAGREEN );
   I( 445, ST::AUSTRALIA_CC, 11, 10, YELLOW, SEAGREEN, SEAGREEN );
   I( 455, ST::AUSTRALIA_CC, 12, 11, WHITE, RED, RED );

   I( 5, ST::AUSTRIA_CC, 1, 4, WHITE, WHITE, SEAGREEN );
   I( 500, ST::AUSTRIA_CC, 2, 1, WHITE, WHITE, BLUE );
   I( 505, ST::AUSTRIA_CC, 3, 2, WHITE, WHITE, BLUE );
   I( 510, ST::AUSTRIA_CC, 4, 3, WHITE, BLUE, BLUE );

   I( 5, ST::BELGIUM_CC, 1, 1, WHITE, WHITE, SEAGREEN );
   I( 600, ST::BELGIUM_CC, 2, 3, BLACK, BLACK, WHITE );
   I( 615, ST::BELGIUM_CC, 3, 4, WHITE, WHITE, BLUE );
   I( 610, ST::BELGIUM_CC, 4, 2, BLACK, WHITE, WHITE );

   I( 5, ST::BULGARIA_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   I( 5, ST::BOSNIA_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   I( 5, ST::BELARUS_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   // this is probably not correct...way too many signs to
   // have a valid conversion between old maps and new.
   I( 1000, ST::CANADA_CC, 1, 1, WHITE, WHITE, SEAGREEN );
   I( 1005, ST::CANADA_CC, 2, 2, WHITE, RED, BLUE );
   I( 1055, ST::CANADA_CC, 3, 3, BLACK, BLACK, WHITE );

   I( 5, ST::SWITZERLAND_CC, 1, 3, WHITE, WHITE, SEAGREEN );
   I( 1700, ST::SWITZERLAND_CC, 2, 1, WHITE, WHITE, RED );
   I( 1705, ST::SWITZERLAND_CC, 3, 2, WHITE, WHITE, BLUE );

   I( 1800, ST::CHINA_CC, 1, 1, WHITE, WHITE, RED );
   I( 1805, ST::CHINA_CC, 2, 2, BLACK, BLACK, YELLOW );
   I( 1810, ST::CHINA_CC, 3, 3, BLACK, BLACK, WHITE );
   I( 1815, ST::CHINA_CC, 4, 4, SEAGREEN, SEAGREEN, WHITE );

   I( 5, ST::CZECH_REPUBLIC_CC, 1, 2, WHITE, WHITE, SEAGREEN );
   I( 1910, ST::CZECH_REPUBLIC_CC, 2, 1, WHITE, WHITE, RED );
   I( 1905, ST::CZECH_REPUBLIC_CC, 3, 3, WHITE, WHITE, BLUE );

   I( 5, ST::GERMANY_CC, 1, 3, WHITE, WHITE, SEAGREEN );
   I( 2000, ST::GERMANY_CC, 2, 1, WHITE, WHITE, BLUE );
   I( 2005, ST::GERMANY_CC, 3, 2, BLACK, BLACK, YELLOW );

   I( 5, ST::DENMARK_CC, 1, 1, WHITE, WHITE, SEAGREEN );
   I( 2100, ST::DENMARK_CC, 2, 2, BLACK, BLACK, YELLOW );
   I( 2110, ST::DENMARK_CC, 3, 3, BLACK, BLACK, WHITE );
   I( 2105, ST::DENMARK_CC, 4, 4, BLACK, BLACK, YELLOW );

   I( 5, ST::SPAIN_CC, 1, 3, WHITE, WHITE, SEAGREEN );
   I( 200, ST::SPAIN_CC, 2, 1, WHITE, WHITE, BLUE );
   I( 2200, ST::SPAIN_CC, 3, 2, WHITE, WHITE, RED );
   I( 2205, ST::SPAIN_CC, 4, 4, BLACK, BLACK, ORANGE );
   I( 2210, ST::SPAIN_CC, 5, 5, BLACK, BLACK, WHITE );
   I( 2215, ST::SPAIN_CC, 6, 6, BLACK, BLACK, WHITE );

   I( 5, ST::ESTONIA_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   I( 5, ST::FINLAND_CC, 1, 1, WHITE, WHITE, SEAGREEN );
   I( 2400, ST::FINLAND_CC, 2, 2, WHITE, WHITE, RED );
   I( 2405, ST::FINLAND_CC, 3, 3, BLACK, BLACK, YELLOW );
   I( 2410, ST::FINLAND_CC, 4, 4, BLACK, BLACK, WHITE );
   I( 2415, ST::FINLAND_CC, 5, 5, WHITE, WHITE, BLUE );

   I( 5, ST::FRANCE_CC, 1, 4, WHITE, WHITE, SEAGREEN );
   I( 2500, ST::FRANCE_CC, 2, 1, WHITE, WHITE, RED );
   I( 2505, ST::FRANCE_CC, 3, 2, WHITE, WHITE, RED );
   I( 2510, ST::FRANCE_CC, 4, 3, BLACK, BLACK, ORANGE );

   // United kingdom
   I( 5, ST::ENGLAND_CC, 1, 3, WHITE, WHITE, SEAGREEN );
   I( 2600, ST::ENGLAND_CC, 2, 1, WHITE, WHITE, BLUE );
   I( 2605, ST::ENGLAND_CC, 3, 2, YELLOW, SEAGREEN, SEAGREEN );
   I( 2610, ST::ENGLAND_CC, 4, 3, BLACK, BLACK, WHITE );

   I( 5, ST::GREECE_CC, 1, 1, WHITE, WHITE, SEAGREEN );
   I( 2705, ST::GREECE_CC, 2, 3, WHITE, WHITE, BLUE );

   I( 5, ST::CROATIA_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   I( 5, ST::HUNGARY_CC, 1, 1, WHITE, WHITE, SEAGREEN );
   I( 3000, ST::HUNGARY_CC, 2, 2, WHITE, WHITE, BLUE );
   I( 3005, ST::HUNGARY_CC, 3, 3, WHITE, BLACK, SEAGREEN );
   I( 3010, ST::HUNGARY_CC, 4, 4, WHITE, WHITE, BLUE );

   I( 5, ST::IRELAND_CC, 1, 4, WHITE, WHITE, SEAGREEN );
   I( 3100, ST::IRELAND_CC, 2, 1, WHITE, WHITE, BLUE );
   I( 3105, ST::IRELAND_CC, 3, 2, YELLOW, WHITE, SEAGREEN );
   I( 3110, ST::IRELAND_CC, 4, 3, BLACK, WHITE, WHITE );

   I( 5, ST::ITALY_CC, 1, 3, WHITE, WHITE, SEAGREEN );
   I( 3200, ST::ITALY_CC, 2, 1, WHITE, WHITE, SEAGREEN );
   I( 3205, ST::ITALY_CC, 3, 2, WHITE, WHITE, BLUE );
   I( 3210, ST::ITALY_CC, 4, 4, WHITE, WHITE, BLUE );
   I( 3215, ST::ITALY_CC, 5, 5, WHITE, WHITE, BLUE );
   I( 3220, ST::ITALY_CC, 6, 6, BLACK, BLACK, WHITE );
   I( 3225, ST::ITALY_CC, 7, 7, WHITE, WHITE, SEAGREEN );

   I( 5, ST::LIECHTENSTEIN_CC, 1, 3, WHITE, WHITE, SEAGREEN );
   I( 1700, ST::LIECHTENSTEIN_CC, 2, 1, WHITE, WHITE, RED );
   I( 1705, ST::LIECHTENSTEIN_CC, 3, 2, WHITE, WHITE, BLUE );

   I( 5, ST::LITHUANIA_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   I( 5, ST::LUXEMBOURG_CC, 1, 1, WHITE, WHITE, SEAGREEN );
   I( 3500, ST::LUXEMBOURG_CC, 2, 2, BLACK, BLACK, WHITE );
   I( 3510, ST::LUXEMBOURG_CC, 4, 5, BLACK, BLACK, YELLOW );

   I( 5, ST::LATVIA_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   I( 5, ST::MONACO_CC, 1, 4, WHITE, WHITE, SEAGREEN );
   I( 2500, ST::MONACO_CC, 2, 1, WHITE, WHITE, RED );
   I( 2505, ST::MONACO_CC, 3, 2, WHITE, WHITE, RED );
   I( 2510, ST::MONACO_CC, 4, 3, BLACK, BLACK, YELLOW );

   I( 5, ST::MOLDOVA_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   I( 5, ST::MACEDONIA_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   I( 5, ST::NETHERLANDS_CC, 1, 3, WHITE, WHITE, SEAGREEN );
   I( 4000, ST::NETHERLANDS_CC, 2, 1, WHITE, WHITE, RED );
   I( 4005, ST::NETHERLANDS_CC, 3, 2, BLACK, BLACK, YELLOW );
   I( 4015, ST::NETHERLANDS_CC, 5, 5, WHITE, RED, RED );

   I( 5, ST::NORWAY_CC, 1, 1, WHITE, WHITE, SEAGREEN );
   I( 4105, ST::NORWAY_CC, 2, 2, BLACK, BLACK, WHITE );
   I( 4110, ST::NORWAY_CC, 3, 3, BLACK, BLACK, WHITE );

   I( 5, ST::POLAND_CC, 1, 1, WHITE, WHITE, SEAGREEN );
   I( 4200, ST::POLAND_CC, 2, 2, WHITE, WHITE, RED );
   I( 4205, ST::POLAND_CC, 3, 3, BLACK, BLACK, YELLOW );

   I( 5, ST::PORTUGAL_CC, 1, 3, WHITE, WHITE, SEAGREEN );
   I( 4300, ST::PORTUGAL_CC, 2, 1, WHITE, WHITE, BLUE );
   I( 4305, ST::PORTUGAL_CC, 3, 2, WHITE, WHITE, RED );
   I( 4310, ST::PORTUGAL_CC, 4, 4, BLACK, BLACK, WHITE );
   I( 4315, ST::PORTUGAL_CC, 5, 5, BLACK, BLACK, WHITE );
   I( 4320, ST::PORTUGAL_CC, 6, 6, BLACK, BLACK, WHITE );
   I( 4325, ST::PORTUGAL_CC, 7, 7, BLACK, YELLOW, WHITE );
   I( 4330, ST::PORTUGAL_CC, 8, 8, BLACK, BLACK, WHITE );

   I( 5, ST::ROMANIA_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   I( 5, ST::RUSSIA_CC, 1, 1, WHITE, WHITE, SEAGREEN );
   I( 4500, ST::RUSSIA_CC, 5, 2, WHITE, WHITE, SEAGREEN );
   I( 4505, ST::RUSSIA_CC, 2, 3, WHITE, WHITE, BLUE );
   I( 4510, ST::RUSSIA_CC, 3, 4, WHITE, WHITE, BLUE );
   I( 4515, ST::RUSSIA_CC, 4, 5, WHITE, WHITE, BLUE );


   I( 5, ST::SERBIA_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   // SMR - San Marino does not exist in Wayfinder. Italian micro state

   // SGP Singapore No types in TA pdf

   I( 5, ST::SLOVAKIA_CC, 1, 1, WHITE, WHITE, SEAGREEN );
   I( 4800, ST::SLOVAKIA_CC, 2, 2, WHITE, WHITE, RED );
   I( 4805, ST::SLOVAKIA_CC, 3, 3, WHITE, WHITE, BLUE );

   I( 5, ST::SLOVENIA_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   I( 5, ST::SWEDEN_CC, 1, 1, WHITE, WHITE, SEAGREEN );
   I( 5000, ST::SWEDEN_CC, 2, 2, WHITE, WHITE, BLUE );
   I( 5005, ST::SWEDEN_CC, 3, 3, WHITE, WHITE, BLUE );

   I( 5, ST::TURKEY_CC, 1, 1, WHITE, WHITE, SEAGREEN );
   I( 5200, ST::TURKEY_CC, 2, 2, WHITE, WHITE, RED );
   I( 5205, ST::TURKEY_CC, 3, 3, WHITE, WHITE, BLUE );
   I( 5210, ST::TURKEY_CC, 4, 4, BLACK, BLACK, WHITE );

   I( 5, ST::UKRAINE_CC, 1, 1, WHITE, WHITE, SEAGREEN );

   for ( int j = 1 ; j <= 6 ; ++j ) {
      I( 5400, ST::USA_CC, j, 1, WHITE, WHITE, BLUE );
   }
   for ( int j = 11 ; j <= 20 ; ++j ) {
      I( 5450, ST::USA_CC, j, 2, BLACK, BLACK, WHITE );
   }
   
#undef I
   // VAT - Vatican does not exist in Wayfinder. Italian micro state
   // with lots of power.

   return i;
}

namespace SignPostUtility {

SignPostUtility::CountryAndElementClassColorCont m_colors = initColors();

void convertSignPost( const OldGenericMap& map,
                      GMSSignPostSet& signs ) {

   for ( GMSSignPostSet::iterator signIt = signs.begin();
         signIt != signs.end() ; ++signIt ) {
      if ( signIt->getElementClass() == MAX_BYTE ) {
         signIt->setClass( GMSSignPostElm::INVALID_CLASS );
         continue;
      }

      CountryAndElementClassColorCont::const_iterator findIt = m_colors
         .find( CountryAndElement( map.getCountryCode(),
                                   signIt->getElementClass() ) );
      // if found, then set the converted value
      if ( findIt != m_colors.end() ) {
         signIt->setClass( findIt->second.shieldRouteType );
      } else {
         signIt->setClass( GMSSignPostElm::INVALID_CLASS );
      }
   }

}

uint32
makeSignPost( SignPost& sign, MC2String& str,
              LangType& strLang,
              const GMSSignPostSet& inData,
              const GenericMap& map,
              const OldGenericMap& oldMap,
              bool newSigns )
{
   uint32 prio = 0;
   SignColors colors;
   bool exit = false;
   if ( newSigns ) {
      getColorForNew( inData, map.getCountryCode(), colors, prio, exit );
   } else {
      getColorFor( inData, map.getCountryCode(), colors, prio, exit );
   }
   sign.setColors( colors );
   sign.setExit( exit );

   strLang = LangType( LangTypes::english );
   for ( GMSSignPostSet::const_iterator it = inData.begin() ; 
         it != inData.end() ; ++it ) {
      // String
      if ( !str.empty() ) {
         str.append( " " );
      }
      LangTypes::language_t lang = it->getTextLang( oldMap );
      if ( lang != LangTypes::invalidLanguage && lang != LangTypes::english ) {
         strLang = lang;
      }
      str.append( it->getTextString( oldMap ) );

      // Other things for prio!!
      if ( it->getAmbigousInfo() ) {
         prio += 100;
      }
      // elementType_t
      if ( it->getType() == GMSSignPostElm::pictogram ||
           it->getType() == GMSSignPostElm::otherDestination ) {
         // Don't use this, we don't support pictograms
         prio = INVALID_PRIO;
         break;
      }

   }

   return prio;
}

bool 
getColorForNew( const GMSSignPostSet& signs,
                StringTable::countryCode country,
                SignColors& colors,
                uint32& prio,
                bool& exit ) {
   GMSSignPostElm::ElementClass elementClass = GMSSignPostElm::INVALID_CLASS;
   GMSSignPostElm::elementType_t elementType = 
      GMSSignPostElm::invalidElementType;
   bool colorFound = false;

   for ( GMSSignPostSet::const_iterator it = signs.begin() ; 
         it != signs.end() ; ++it ) {
      if ( it->getType() != GMSSignPostElm::invalidElementType ) {
         elementType = it->getType();
      }
      if ( it->getElementClass() != GMSSignPostElm::INVALID_CLASS ) {
         elementClass = it->getElementClass();
         break;
      }
   }

   if ( elementClass != GMSSignPostElm::INVALID_CLASS ) {
      STLUtility::Array< const SignColor >::const_iterator findIt =
         lower_bound( g_signColors.begin(), g_signColors.end(),
                      elementClass,
                      CompareElementClass() );

      if ( findIt != g_signColors.end() &&
           elementClass == findIt->m_class ) {
         colorFound = true;
         colors = SignColors( findIt->m_text,
                              findIt->m_front,
                              findIt->m_back );

         // find prio by using the old color table
         // default prio if we dont find any.

         prio = DEFAULT_PRIO;

         CountryAndElementClassColorCont::const_iterator prioIt =
            m_colors.
            lower_bound( CountryAndElement( country,
                                            0 ) ); // for lowest element
         if ( prioIt != m_colors.end() &&
              (*prioIt).first.first == country ) {
            // loop until the country changes or until
            // we find the right element class
            for (; prioIt != m_colors.end() &&
                    (*prioIt).first.first == country;
                 ++prioIt ) {
               if ( prioIt->second.shieldRouteType == findIt->m_class ) {
                  prio = prioIt->second.prio;
                  break;
               }
            }
         }

      }
   }

   exit = ( elementType == GMSSignPostElm::exitNumber ||
            elementType == GMSSignPostElm::exitName );

   using namespace GDUtils::Color;

   if ( !colorFound ) {
      colors = SignColors( BLACK, BLACK, WHITE );
      prio = DEFAULT_PRIO;
   }

   return colorFound;
}

bool
getColorFor( const GMSSignPostSet& signs,
             StringTable::countryCode country,
             SignColors& colors,
             uint32& prio,
             bool& exit ) {
   GMSSignPostElm::ElementClass elementClass = GMSSignPostElm::INVALID_CLASS;
   GMSSignPostElm::elementType_t elementType =
      GMSSignPostElm::invalidElementType;

   bool colorFound = false;

   for ( GMSSignPostSet::const_iterator it = signs.begin() ;
         it != signs.end() ; ++it ) {
      if ( it->getType() != GMSSignPostElm::invalidElementType ) {
         elementType = it->getType();
      }
      if ( it->getElementClass() != GMSSignPostElm::INVALID_CLASS ) {
         elementClass = it->getElementClass();
         break;
      }
   }

   mc2log << "elementClass: " << elementClass << endl;

   if ( elementClass != GMSSignPostElm::INVALID_CLASS ) {
      CountryAndElementClassColorCont::const_iterator findIt = m_colors
         .find( CountryAndElement( country, elementClass ) );
      if ( findIt != m_colors.end() ) {
         colorFound = true;
         colors = findIt->second.colors;
         prio = findIt->second.prio;
      } else {
         mc2log << "Did not find any colors" << endl;
      }
   } else {
      mc2log << "Missing element class." << endl;
   }

   exit = ( elementType == GMSSignPostElm::exitNumber || 
            elementType == GMSSignPostElm::exitName );

   using namespace GDUtils::Color;
   // default to black and white colors with the worst prio
   if ( !colorFound ) {
      colors = SignColors( BLACK, BLACK, WHITE );
      prio = DEFAULT_PRIO;
   }

   return colorFound;
}

} // SignPostUtility
