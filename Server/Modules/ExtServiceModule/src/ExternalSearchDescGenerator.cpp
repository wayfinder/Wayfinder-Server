/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "ExternalSearchDescGenerator.h"

#include "ExternalSearchHelper.h"
#include "ExternalSearchDesc.h"
#include "ExternalSearchDescEntry.h"

#include "StringTable.h"
#include "StringUtility.h"

#include "ExtServices.h"
#include "LangTypes.h"

#include "DataBuffer.h"
#include "MC2CRC32.h"

namespace {

   class StringHelper {
   public:
      StringHelper( LangTypes::language_t lang ) : m_lang ( lang ) {}
      
      MC2String getCapStr( StringCode code ) const {
         return StringUtility::makeFirstCapital(StringTable::getString(
                                                   code, m_lang ) );
      }
      
      MC2String getStr( StringCode code ) const {
         return StringUtility::makeFirstCapital(StringTable::getString(
                                                   code, m_lang ) );
      }
      
      MC2String getCountryName( StringTable::countryCode cc ) const {
         return StringUtility::makeFirstCapital(
            StringTable::getString( StringTable::getCountryStringCode( cc ),
                                    m_lang ) );
      }
         
      
   private:
      /// The language to be used.
      LangTypes::language_t m_lang;
   };
}

ExternalSearchDescGenerator::ExternalSearchDescGenerator()
{
}

ExternalSearchDescGenerator::~ExternalSearchDescGenerator()
{
   for ( serviceMap_t::iterator it = m_serviceMap.begin();
         it != m_serviceMap.end();
         ++it ) {
      delete it->second;
   }
   m_serviceMap.clear();
}

static MC2String getName( const ExtService& service,
                          const LangType& langType )
{
   //
   // !OBS!
   //
   // These names are NOT used by the compact/combined search description!
   // If you change name here please update SearchParserHandler.cpp !
   // compact/combined search does not include the country name
   //
   StringHelper st( langType );
   switch ( service ) {
      case ExtService::google_local_search:
         return "Google Local Search";
      case ExtService::qype:
         return "Qype";
      case ExtService::adserver:
         return "AdServer";
      case ExtService::nbr_services:
      case ExtService::not_external:
         
      mc2log << warn 
             << "[ExternalSearchDescGenerator] Unknown service: " 
             << service << endl;
   }

   return "Unknown";
}

int
ExternalSearchDescGenerator::makeDesc( const ExtService& service,
                                       const LangType& langType )
{
   StringHelper st( langType );

   // Think it is safer to have pointers. Don't know if entries
   // can be moved around by a map.
   pair<int,int> findPair( service, langType );
   serviceMap_t::iterator findit = m_serviceMap.find( findPair );
   if ( findit == m_serviceMap.end() ) {
      m_serviceMap.insert( serviceMap_t::value_type( findPair, NULL ) );
      return makeDesc( service, langType );
   }

   ExternalSearchDesc* desc = findit->second;
   
   if ( desc == NULL ) {
      desc = findit->second = new ExternalSearchDesc;      
   }

   ExternalSearchDesc::entryvector_t res;
   desc->setName( getName( service, langType ) );
   desc->setService( service ); // This also updates the icons from prop
   desc->setLang( langType );
   
   StringTable::countryCode country;
   bool whitepages = true;
   switch ( service ) {
      case ExtService::google_local_search:
         whitepages = false;
         country = StringTable::USA_CC;
         break;
      case ExtService::qype:
         whitepages = false;
         country = StringTable::USA_CC;
         break;
      case ExtService::adserver:
         whitepages = false;
         country = StringTable::SWEDEN_CC;
         break;
      default:
         return -1;
         break;
   } 
   
   // first argument is always language
   res.push_back( ExternalSearchDescEntry( ExternalSearchDesc::country,
                                           ExternalSearchHelper::getFieldName( 
                                              ExternalSearchDesc::country, 
                                              langType ),
                                           ExternalSearchDescEntry::
                                           choice_type,
                                           0x1 ) );
   res.back().addChoiceParam( country, st.getCountryName( country ) );
   // setup default arguments for yellowpages and whitepages
   struct DescArg {
      ExternalSearchDesc::search_fields_t m_id;
      ExternalSearchDescEntry::val_type_t m_val_type;
      uint32 m_required;
   }; 
   // standard white pages arguments
   DescArg wp_args[] = {
      // Name/phone
      { ExternalSearchDesc::name_or_phone,
        ExternalSearchDescEntry::string_type,
        1 },
      // address/city
      { ExternalSearchDesc::address_or_city,
        ExternalSearchDescEntry::string_type,
        0 },
   };
   // standard yellow pages arguments
   DescArg yp_args[] = {
      // company or search word
      { ExternalSearchDesc::company_or_search_word,
        ExternalSearchDescEntry::string_type,
        0 },
      // city or area
      { ExternalSearchDesc::city_or_area,
        ExternalSearchDescEntry::string_type,
        0 },
      // category
      { ExternalSearchDesc::category,
        ExternalSearchDescEntry::string_type,
        0 },
   };
   
   // determine which arguments we should use, whitepages or yellowpages
   size_t arraySize = whitepages ? 
      sizeof( wp_args ) / sizeof( wp_args[ 0 ] ) :
      sizeof( yp_args ) / sizeof( yp_args[ 0 ] );
   DescArg *args = whitepages ? wp_args : yp_args;
   
   for ( uint32 i = 0; i < arraySize; ++i ) {
      MC2String name = ExternalSearchHelper::getFieldName( args[ i ].m_id, 
                                                           langType );
      res.push_back( ExternalSearchDescEntry( args[ i ].m_id,
                                              name,
                                              args[ i ].m_val_type,
                                              args[ i ].m_required ) );
   }

   // Put the new entries into it.
   desc->swapEntries( res );
   // Importante. Calculate the CRC.
   desc->calcCRC();
   return 0;
}

const ExternalSearchDesc*
ExternalSearchDescGenerator::getDescPtr( const ExtService& service,
                                         const LangType& lang ) const
{
   serviceMap_t::const_iterator findit =
      m_serviceMap.find( pair<int,int>( service, lang ) );
   if ( findit != m_serviceMap.end() ) {
      return findit->second;
   } else {
      const_cast<ExternalSearchDescGenerator*>(this)->makeDesc( service,
                                                                lang );
      return getDescPtr( service, lang );
   }   
}


const ExternalSearchDesc&
ExternalSearchDescGenerator::getDesc( const ExtService& service,
                                      const LangType& lang ) const
{
   return *getDescPtr( service, lang );
}

int
ExternalSearchDescGenerator::
getDescs( vector<const ExternalSearchDesc*>& descs,
          const LangType& lang ) const
{
   for ( int i = 1; i < ExtService::nbr_services; ++i ) {
      const ExternalSearchDesc* cur = getDescPtr( ExtService::service_t(i),
                                                  lang );
      if ( cur && ! cur->empty() ) {
         descs.push_back( cur );
      }
   }
   return descs.size();
}

uint32
ExternalSearchDescGenerator::getCRC( const LangType& lang ) const
{
   vector<const ExternalSearchDesc*> descs;
   getDescs( descs, lang );
   DataBuffer buf( 4 * descs.size() );

   for ( vector<const ExternalSearchDesc*>::const_iterator it = descs.begin();
         it != descs.end();
         ++it ) {
      buf.writeNextLong( (*it)->getCRC() );
   }
   
   uint32 crc = MC2CRC32::crc32( buf.getBufferAddress(),
                                 buf.getCurrentOffset() );
   
   return crc;
}
