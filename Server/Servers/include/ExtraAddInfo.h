/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTRAADDINFO_H
#define EXTRAADDINFO_H

#include "config.h"


/**
 * Class that holds information about extra rights to add when creating 
 * user.
 *
 */
class ExtraAddInfo {
   public:
      ExtraAddInfo() : year( 0 ), month( 0 ), day( 5 ), 
         explic( MAX_UINT32 ), isSet( false ), regionID( MAX_UINT32 ) {}

      ExtraAddInfo( uint32 y, uint32 m, uint32 d, uint32 e = MAX_UINT32,
                    uint32 r = MAX_UINT32, bool is = true ) 
         : year( y ), month( m ), day( d ), explic( e ), isSet( is ),
           regionID( r ) {}

      uint32 year;
      uint32 month;
      uint32 day;
      uint32 explic;
      bool isSet;
      uint32 regionID;
};


#endif // EXTRAADDINFO_H

