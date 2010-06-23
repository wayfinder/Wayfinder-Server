/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "UserEnumNames.h"
#include "NameUtility.h"

#define SERVICE(x) UserEnums::URType( UserEnums::UR_NO_LEVEL, (UserEnums::x) )
#define LEVEL(x) UserEnums::URType( (UserEnums::x),\
                                     UserEnums::userRightService(0))

// These names are in utf-8
const UserEnumNames::name_entry_t
UserEnumNames::c_names[] =
{
   // These ones will not be displayed to the user currently.
   { LangTypes::english, LEVEL( UR_TRIAL),              "Trial"         },
   { LangTypes::english, LEVEL( UR_SILVER),             "Silver"        },
   { LangTypes::english, LEVEL( UR_GOLD),               "Gold"          },
   { LangTypes::swedish, LEVEL( UR_GOLD),               "Guld"          },
   { LangTypes::english, LEVEL( UR_DEMO),               "Demo"          },
   { LangTypes::english, SERVICE( UR_WF ),              "Wayfinder Navigator"},
   { LangTypes::english, SERVICE( UR_MYWAYFINDER),      "myWayfinder"   },
   { LangTypes::english, SERVICE( UR_MAPDL_PREGEN ),    "Map Download"  },
   { LangTypes::english, SERVICE( UR_MAPDL_CUSTOM ),    "Map Download"  },
   { LangTypes::english, SERVICE( UR_XML ),             "XML"           },
   { LangTypes::english, SERVICE( UR_TRAFFIC     ),     "Traffic info"  },
   { LangTypes::english, SERVICE( UR_SPEEDCAM    ),     "Speed cameras" },   
   { LangTypes::english, SERVICE( UR_FREE_TRAFFIC ),    "Free Traffic Info" },
   { LangTypes::swedish, SERVICE( UR_FREE_TRAFFIC ),    "Gratis trafikinfo" },

   // This poi type right is probably good to translate too. 
   { LangTypes::english, SERVICE( UR_POSITIONING ),     "Positioning"   },
   { LangTypes::english, SERVICE( UR_USE_GPS ),         "Use GPS"   },
   { LangTypes::english, SERVICE( UR_ROUTE ),           "Route"   },
   { LangTypes::english, SERVICE( UR_SEARCH ),          "SEARCH"   },
   { LangTypes::english, SERVICE( UR_POI ),             "POI"   },
   { LangTypes::english, SERVICE( UR_PRO_WEATHER ),     "Pro weather"   },
   { LangTypes::english, SERVICE( UR_ADD_NON_SUP_RIGHTS),"add non sup rights"},
   { LangTypes::english, SERVICE( UR_APP_STORE_NAVIGATION ), "App Store Navigation" },
};

MC2String
UserEnumNames::getName( const UserEnums::URType& type,
                        LangTypes::language_t lang )
{
   int nbrCandidates = 0;
   uint32 candidateStrIdx[ LangTypes::nbrLanguages ];
   const char* candidateStr[ LangTypes::nbrLanguages ];
   
   int nbr_entries = sizeof(c_names) / sizeof( c_names[0] );
   for ( int i = 0; i < nbr_entries; ++i ) {
      bool matches = false;
      if ( UserEnums::isBitRight( c_names[i].m_bit.service() ) ) {
         matches = type.levelAndServiceMatchMask( c_names[i].m_bit );
      } else {
         matches = type.levelAndServiceMatch( c_names[i].m_bit );
      }

      if ( matches ) {
         candidateStr[ nbrCandidates ]    = c_names[i].m_name;
         candidateStrIdx[ nbrCandidates ] =
            CREATE_NEW_NAME( c_names[i].m_lang,
                             ItemTypes::officialName,
                             nbrCandidates );
         ++nbrCandidates;
      }
   }
   mc2dbg4 << "[UEN]: Nbr candidates = " << nbrCandidates << endl;

   if ( nbrCandidates == 0 ) {
      return "";
   } else {
      int idx = NameUtility::getBestName( nbrCandidates, candidateStrIdx,
                                          lang );

      MC2_ASSERT( idx >= 0 );
      
      return candidateStr[idx];
   }
}
