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

#include "ModuleTypes.h"

const
ModuleTypes::module_name_entry_t ModuleTypes::c_moduleNames[] = {

   { "INVALID"    , "INV"  },
   { "Map"        , "MM"   },
   { "Route"      , "RM"   },
   { "Search"     , "SM"   },
   { "User"       , "UM"   },
   { "SMS"        , "SMM"  },   
   { "Test"       , "TM"   },
   { "Email"      , "EM"   },
   { "Info"       , "IM"   },
   { "Gfx"        , "GM"   },
   { "ExtService" , "XM"   },
   { "Tile"       , "TIM"  },
   { "Comm"       , "CMM"  },
};

const char*
ModuleTypes::moduleTypeToString( moduletype_t moduleType )
{
   CHECK_ARRAY_SIZE( c_moduleNames, MODULE_TYPE_MAX_MODULE_NUMBER );

   if ( moduleType < MODULE_TYPE_MAX_MODULE_NUMBER ) {
      return c_moduleNames[ moduleType ].longName;
   } else {
      return "UNKNOWN";
   }
}

const char*
ModuleTypes::moduleTypeToShortString( moduletype_t moduleType )
{
   MC2_ASSERT( ( sizeof( c_moduleNames ) / sizeof( c_moduleNames[0] ) ==
                 MODULE_TYPE_MAX_MODULE_NUMBER ) );
   if ( moduleType < MODULE_TYPE_MAX_MODULE_NUMBER ) {
      return c_moduleNames[ moduleType ].shortName;
   } else {
      return "N/A";
   }
}
