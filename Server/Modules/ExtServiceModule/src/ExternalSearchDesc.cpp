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

#include "ExternalSearchDesc.h"
#include "LangTypes.h"

#include "ExtServices.h"
#include "ExternalSearchDescEntry.h"
#include "ExternalSearchHelper.h"
#include "StringUtility.h"
#include "MC2CRC32.h"
#include "Packet.h"
#include "DataBuffer.h"
#include "Properties.h"
#include "STLStringUtility.h"
#include "PacketDataBuffer.h"
#include "StringTable.h"


void
ExternalSearchDesc::setService( uint32 serviceId )
{
   m_service = serviceId;
}

uint32
ExternalSearchDesc::getService() const
{
   return m_service;
}

LangType
ExternalSearchDesc::getLang() const
{
   return LangTypes::language_t( m_langType );
}

void
ExternalSearchDesc::setLang( const LangType& langType )
{
   m_langType = langType;
}

void
ExternalSearchDesc::setName( const MC2String& name )
{
   m_name = name;
}

void
ExternalSearchDesc::setColour( const uint32 colour )
{
   m_colour = colour;
}

uint32
ExternalSearchDesc::getColour() const
{
   return m_colour;
}

const MC2String&
ExternalSearchDesc::getName() const
{
   return m_name;
}

const ExternalSearchDesc::entryvector_t&
ExternalSearchDesc::getEntries() const
{
   return m_entries;
}

void
ExternalSearchDesc::swapEntries( entryvector_t& newEntries )
{
   m_entries.swap( newEntries );
}

bool
ExternalSearchDesc::empty() const
{
   return m_entries.empty();
}


bool
ExternalSearchDesc::checkOneParam( int inParamID,
                                   const MC2String& inParamVal,
                                   const ExternalSearchDescEntry& facit ) const
{
   switch ( facit.getType() ) {
      case ExternalSearchDescEntry::string_type:
         // Always ok         
         return true;
         break;
      case ExternalSearchDescEntry::number_type:
      case ExternalSearchDescEntry::choice_type:
      {
         // Try strtoul
         char* endPtr;
         unsigned long val = strtoul( inParamVal.c_str(), &endPtr, 0 );
         if ( *endPtr != '\0' ) {
            return false;
         } else {
            if ( facit.getType() == ExternalSearchDescEntry::number_type ) {
               return true;
            }
         }
         if ( facit.getType() == ExternalSearchDescEntry::choice_type ) {
            // Check that it is one of the valid choices.
            for( ExternalSearchDescEntry::choice_vect_t::const_iterator it =
                    facit.getChoices().begin();
                 it != facit.getChoices().end();
                 ++it ) {
               if ( it->first == int(val) ) {
                  return true;
               }
            }
            return false;
         }
      }      
      break;
   }
   return false;
}

bool
ExternalSearchDesc::checkParams( const valmap_t& value_map,
                                 uint32 service ) const
{
   if ( service != getService() ) {
      mc2log << warn << "[ExternalSearchDesc]: Comparing service "
             << service << " to " << m_service << endl;
      return false;
   }
   uint32 reqCheckVal = MAX_UINT32;
   
   for( entryvector_t::const_iterator it = m_entries.begin();
        it != m_entries.end();
        ++it ) {
      // Find in value_map. If not found, but required - return false.
      valmap_t::const_iterator findit = value_map.find( it->getID() );
      if ( findit != value_map.end() ) {
         if ( ! checkOneParam( findit->first, findit->second, *it ) ) {
            mc2dbg << "[ExternalSearchDesc]: param " << findit->first
                   << " = " << findit->second << " is bad" << endl;
            return false;
         }
      }
      // Check requirement
      if ( findit == value_map.end() ||
           StringUtility::trimStartEnd( findit->second ).empty() ) {
         // Not found or empty -> reverse bits.
         reqCheckVal &= ( uint32(MAX_UINT32) ^ it->getRequiredBits() );
      } else {
         // Found and ok. And with bits as is.
         if ( it->getRequiredBits() != 0 ) {
            reqCheckVal &= it->getRequiredBits();
         }
      }
   }
   mc2dbg << "[ExternalSearchDesc]: checkVal = "
          << MC2HEX( reqCheckVal ) << endl;
   return reqCheckVal != 0;
}


int
ExternalSearchDesc::save( Packet* packet, int& pos ) const
{
   int start_pos = pos;
   packet->incWriteLong( pos, m_service );
   packet->incWriteLong( pos, m_langType );
   packet->incWriteLong( pos, m_colour );
   packet->incWriteString( pos, m_name );
   packet->incAlignWriteLong( pos );
   for ( entryvector_t::const_iterator it = m_entries.begin();
         it != m_entries.end();
         ++it ) {
      it->save( packet, pos );
   }
   return pos - start_pos;
}


void
ExternalSearchDesc::calcCRC()
{
   DataBuffer buf( 65536*10 );
   PacketDataBuffer::saveAsInPacket( buf, *this );
   m_crc = MC2CRC32::crc32( buf.getBufferAddress(),
                            buf.getCurrentOffset() );
}

uint32
ExternalSearchDesc::getCRC() const
{
   return m_crc;
}


bool 
ExternalSearchDesc::setTopRegionField( valmap_t& map, uint32 topRegion ) const
{
   if ( topRegion != getTopRegionID() &&
        getTopRegionID() != MAX_UINT32 ) {
      return false;
   }

 
   StringTable::countryCode countryCode = StringTable::NBR_COUNTRY_CODES;
   countryCode = ExternalSearchHelper::topRegionToCountryCode( topRegion, getService() );

   // Country field
   map[ country ] = STLStringUtility::uint2str( countryCode );

   return true;
}

bool 
ExternalSearchDesc::setWhatWhereField( valmap_t& map, 
                                       const MC2String& what,
                                       const MC2String& where,
                                       const MC2String& categoryName ) const
{
   switch ( getService() ) { 
      // WP searches does not support categories, at least not such as Bank
      /*
         if ( !categoryName.empty() ) {
            return false;
         } else {
            map[ name_or_phone ] = what;
            map[ address_or_city ] = where;
            return true;
         }
         break;
      */
      // YP searches
      case ExtService::google_local_search:
      case ExtService::qype:
      case ExtService::adserver:
         if ( !categoryName.empty() ) {
            map[ company_or_search_word ] = categoryName;
            map[ category ] = categoryName;
         } else {
            map[ company_or_search_word ] = what;
         }
         map[ city_or_area ] = where;
         return true;
         break;
      case ExtService::not_external:
      case ExtService::nbr_services:
         break;
   }
   
   return false;
}

uint32 
ExternalSearchDesc::getTopRegionID() const {
   // there is currently no other way then to do fixed values here.
   // return values from region_ids.xml ( 2007-11-14 )
   switch ( getService() ) { 
      case ExtService::google_local_search:
      case ExtService::qype:
      case ExtService::adserver:
         return MAX_UINT32; // all top regions
         
      case ExtService::not_external:
      case ExtService::nbr_services:
         break;
   }

   return 0;
}
