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
#include "MapRights.h"

#include "UserRight.h"
#include "UserEnums.h"
#include "STLUtility.h"
#include "NonStdStl.h"
#include "TimeUtility.h"
#include "NameUtility.h"
#include "Packet.h"
#include "Array.h"

namespace {

   MapRights userLevelToMapRight(UserEnums::userRightLevel level)
   {
      typedef pair<UserEnums::userRightLevel, MapRights> rights_t;
      static const rights_t levelToRight[] = {
         rights_t( UserEnums::UR_TRIAL,       MapRights( MapRights::BASIC_MAP ) ),
         rights_t( UserEnums::UR_SILVER,      MapRights( MapRights::BASIC_MAP ) ),
         rights_t( UserEnums::UR_GOLD,        MapRights( MapRights::BASIC_MAP ) ),
         rights_t( UserEnums::UR_DEMO,        MapRights( MapRights::BASIC_MAP ) ),
         rights_t( UserEnums::UR_IRON,        MapRights( MapRights::BASIC_MAP ) ),
         rights_t( UserEnums::UR_LITHIUM,     MapRights( MapRights::BASIC_MAP ) ),
         rights_t( UserEnums::UR_WOLFRAM,     MapRights( MapRights::BASIC_MAP ) ),
         rights_t( UserEnums::ALL_LEVEL_MASK, MapRights() ),
         rights_t( UserEnums::TSG_MASK,       MapRights() ),
         rights_t( UserEnums::SG_MASK,        MapRights() ),
         rights_t( UserEnums::TS_MASK,        MapRights() ),
         rights_t( UserEnums::TG_MASK,        MapRights() ),
         rights_t( UserEnums::TL_MASK,        MapRights() ),
         rights_t( UserEnums::TW_MASK,        MapRights() ),
         rights_t( UserEnums::UR_NO_LEVEL,    MapRights() ), 
         rights_t( UserEnums::ALL_FLAGS,      MapRights() ),
         rights_t( UserEnums::ALL_FLAGS_INV,  MapRights() ),
      };
      static const size_t n_levels = sizeof(levelToRight)/sizeof(*levelToRight);
      static const rights_t* begin = levelToRight;
      static const rights_t* end = levelToRight + n_levels;

      const rights_t* p = find_if(begin, end,
                                  compose1(bind1st(equal_to<UserEnums::userRightLevel>(),
                                                   level),
                                           STLUtility::select1st<rights_t>()));
      if(p != end){
         return p->second;
      } else {
         return MapRights();
      }
   }

   MapRights userServiceToMapRight(UserEnums::userRightService service)
   {
      typedef pair<UserEnums::userRightService, MapRights> rights_t;
      static const rights_t serviceToRight[] = {
         rights_t( UserEnums::UR_WF,               MapRights( MapRights::BASIC_MAP )), //?
         rights_t( UserEnums::UR_MYWAYFINDER,      MapRights( MapRights::BASIC_MAP )), //?
         rights_t( UserEnums::UR_MAPDL_PREGEN,     MapRights( MapRights::BASIC_MAP )), //?
         rights_t( UserEnums::UR_MAPDL_CUSTOM,     MapRights( MapRights::BASIC_MAP )), //?
         rights_t( UserEnums::UR_XML,              MapRights( MapRights::BASIC_MAP )), //?
         rights_t( UserEnums::UR_RESERVED_BIT_6,   MapRights()),
         rights_t( UserEnums::UR_TRAFFIC,          MapRights( MapRights::TRAFFIC )),
         rights_t( UserEnums::UR_SPEEDCAM,         MapRights( MapRights::SPEEDCAM )),
         rights_t( UserEnums::UR_RESERVED_BIT_9,   MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_10,  MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_11,  MapRights()),
         rights_t( UserEnums::UR_FREE_TRAFFIC,     MapRights( MapRights::FREE_TRAFFIC )),
         rights_t( UserEnums::UR_POSITIONING,      MapRights()), //?
         rights_t( UserEnums::UR_RESERVED_BIT_14,  MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_15,  MapRights()),
         rights_t( UserEnums::UR_FLEET,            MapRights( MapRights::FLEET )), //?
         rights_t( UserEnums::UR_VERSION_LOCK,     MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_18,  MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_19,  MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_20,  MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_21,  MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_22,  MapRights()),
         rights_t( UserEnums::UR_WF_TRAFFIC_INFO_TEST,  MapRights( MapRights::WF_TRAFFIC_INFO_TEST )),
         rights_t( UserEnums::UR_RESERVED_BIT_24,  MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_25,  MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_26,  MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_27,  MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_28,  MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_29,  MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_30,  MapRights()),
         rights_t( UserEnums::UR_RESERVED_BIT_31,  MapRights()),
         rights_t( UserEnums::UR_USE_GPS,          MapRights()),
         rights_t( UserEnums::UR_ROUTE,            MapRights()),
         rights_t( UserEnums::UR_SEARCH,           MapRights()),
         rights_t( UserEnums::UR_POI,              MapRights()),
         rights_t( UserEnums::UR_SCDB_SPEEDCAM,    MapRights( MapRights::SCDB ) ),
         rights_t( UserEnums::UR_DISABLE_POI_LAYER,  MapRights(MapRights::DISABLE_POI_LAYER )),
         rights_t( UserEnums::UR_ALL_TRAFFIC_MASK, MapRights()),
      };
      static const size_t n_services = 
         sizeof(serviceToRight)/sizeof(*serviceToRight);
      static const rights_t* begin = serviceToRight;
      static const rights_t* end = serviceToRight + n_services;
   
      const rights_t* p = find_if(begin, end,
                                  compose1(bind1st(equal_to<UserEnums::userRightService>(),
                                                   service),
                                           STLUtility::select1st<rights_t>()));
      if(p != end){
         return p->second;
      } else {
         return MapRights();
      }
   }

} //namespace

MapRights toMapRight(const UserEnums::URType& ur)
{
   const MapRights rights = ( userLevelToMapRight(   ur.level()   ) | 
                              userServiceToMapRight( ur.service() )   );
   //mc2dbg << "[MR] URType " << ur << " maprights " << rights << endl;
   return rights;
}

MapRights toMapRight(const UserRight& ur, uint32 now)
{
   //ignore deleted, expired, and pending rights
   if ( ur.isDeleted() || ! ur.checkAccessAt( now ) ) {
      return MapRights();
   }
   
   return toMapRight( ur.getUserRightType() );
}

MapRights::MapRights( const MC2String& hexString ) :
   m_rights(0)
{
   m_rights = strtoull(hexString.c_str(), NULL, 16);
}


bool MapRights::save( Packet* packet, int& pos ) const
{
   packet->incWriteLongLong(pos, m_rights);
   return true;
}

bool MapRights::load( const Packet* packet, int& pos )
{
   m_rights = packet->incReadLongLong( pos );
   return true;
}


// These names are in utf-8
MC2String
MapRights::getName( LangTypes::language_t lang ) const
{
   struct name_entry_t {
      const LangTypes::language_t m_lang;
      const MapRights m_right;
      const char* const m_name;      
   };
   static const name_entry_t names[] = {
      // These ones will not be displayed to the user currently.
      // XXX: But perhaps in POI Info and user has not access to POI
      { LangTypes::english, MapRights( BASIC_MAP ),       "Wayfinder Navigator"},
      { LangTypes::english, MapRights( TRAFFIC     ),     "Traffic info"  },
      { LangTypes::english, MapRights( SPEEDCAM    ),     "Speed cameras" },   
      { LangTypes::english, MapRights( FREE_TRAFFIC ),    "Free Traffic Info" },
      { LangTypes::swedish, MapRights( FREE_TRAFFIC ),    "Gratis trafikinfo" },
      { LangTypes::english, MapRights( WF_TRAFFIC_INFO_TEST ), "Wayfinder Traffic Info Test"},
      { LangTypes::english, MapRights( SCDB ),            "Speed Camera DataBase" },
   };
   const size_t nbr_entries = sizeof(names) / sizeof( *names );

   int nbrCandidates = 0;
   uint32 candidateStrIdx[ LangTypes::nbrLanguages ];
   const char* candidateStr[ LangTypes::nbrLanguages ];


   MapRights firstMatch;
   for ( size_t i = 0; i < nbr_entries; ++i ) {
      if ( names[i].m_right & *this ) {
         if( ! firstMatch ){
            firstMatch = names[i].m_right;
         }
         if(names[i].m_right == firstMatch){
            candidateStr[ nbrCandidates ]    = names[i].m_name;
            candidateStrIdx[ nbrCandidates ] =
               CREATE_NEW_NAME( names[i].m_lang,
                                ItemTypes::officialName,
                                nbrCandidates );
            ++nbrCandidates;
         }
      }
   }
   mc2dbg2 << "[MR]: Nbr candidates = " << nbrCandidates << endl;

   if ( nbrCandidates == 0 ) {
      return "";
   } else {
      int idx = NameUtility::getBestName( nbrCandidates, candidateStrIdx,
                                          lang );
      MC2_ASSERT( idx >= 0 );
      
      return candidateStr[idx];
   }
}

MapRights filterMapRights( const MapRights& unfiltered )
{
   if(unfiltered == ~MapRights()){
      return unfiltered;
   }

   // Sets of rights with content that fully covers the area and replaces
   // other rights.

   // Speedcamera rights, that exclude each other, in prio order
   static const MapRights::Rights camRights[] = {
      // First small local rights with small coverage

      // Then large area rights
      MapRights::SCDB,
      MapRights::SPEEDCAM
   };
   
   // Traffic rights, that exclude each other, in prio order
   static const MapRights::Rights trafficRights[] = {
      // First small local rights with small coverage

      // Then large area rights
      MapRights::TRAFFIC,
//JIC      MapRights::FREE_TRAFFIC
   };

   typedef STLUtility::Array< const MapRights::Rights > RightsArray;
   typedef vector< RightsArray > FilterSets;
   FilterSets filterSets;
   filterSets.push_back( RightsArray( camRights, NBR_ITEMS( camRights ) ) );
   filterSets.push_back( RightsArray( trafficRights, 
                                      NBR_ITEMS( trafficRights ) ) );
   
   // The result
   MapRights filtered = unfiltered;
   
   // For all set to filter
   for ( FilterSets::const_iterator it = filterSets.begin() ; 
         it != filterSets.end() ; ++it ) {
      bool found = false;
      for ( uint32 i = 0 ; i < it->size() ; ++i ) {
         if ( unfiltered & (*it)[ i ] ) {
            if ( found ) {
               // Has more prioritized, remove this
               filtered &= ~MapRights( (*it)[ i ] );
            } else {
               // Has this
               found = true;
            }
         }
      }
   }

   return filtered;
}

ostream& operator<<(ostream& o, const MapRights& r)
{
   const std::ios::fmtflags f = o.flags();  // get current format flags
   o << MC2HEX( r.m_rights );               // write in hex
   o.flags(f);                              // restore format flags
   return o;
}
