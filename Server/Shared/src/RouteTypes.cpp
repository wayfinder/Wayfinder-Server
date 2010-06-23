/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RouteTypes.h"


void 
RouteTypes::routeCostTypeToCost( routeCostType cost, 
                                 byte& costA, byte& costB, 
                                 byte& costC, byte& costD )
{
   switch( cost ) {
      case DISTANCE:
         costA = 1;
         costB = costC = costD = 0;
         break;
      case TIME:
         costB = 1;
         costA = costC = costD = 0;
         break;
      case TIME_WITH_DISTURBANCES:
         costC = 1;
         costA = costB = costD = 0;
         break;
      case NBR_ROUTECOSTTYPE:
         costB = 1;
         costA = costC = costD = 0;
         break;
   }
}


RouteTypes::routeCostType 
RouteTypes::costToRouteCostType( byte costA, byte costB,  
                                 byte costC, byte costD )
{
   if ( costA != 0 ) {
      return DISTANCE;
   } else if ( costB != 0 ) {
      return TIME;
   } else if ( costC != 0 ) {
      return TIME_WITH_DISTURBANCES;
   } else {
      return TIME;
   }

}


RouteTypes::routeCostType 
RouteTypes::stringToRouteCostType( const char* str ) {
   if ( strcmp( str, "distance" ) == 0 ) {
      return DISTANCE;
   } else if ( strcmp( str, "time" ) == 0 ) {
      return TIME;
   } else if ( strcmp( str, "time_with_disturbances" ) == 0 ) {
      return TIME_WITH_DISTURBANCES;
   } else {
      return TIME;
   }
}


const char* 
RouteTypes::routeCostTypeToString( routeCostType cost ) {
   switch( cost ) {
      case DISTANCE:
         return "distance";
      case TIME:
         return "time";
      case TIME_WITH_DISTURBANCES:
         return "time_with_disturbances";
      case NBR_ROUTECOSTTYPE:
         return "time";
   }
   
   // Unreachable code
   return "time";
}
