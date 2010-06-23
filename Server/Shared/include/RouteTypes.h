/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTETYPES_H
#define ROUTETYPES_H

#include "config.h"

#include<map>
#include<vector>

// Holds allowed maps
class RouteAllowedMap: public map< uint32, vector< uint32 > > {};

/**
 *    Class for holding general routes types.
 *
 */
class RouteTypes {
   public:
      /**
       * The types of route costs.
       */
      enum routeCostType {
         /// DISTANCE, optimize length of route.
         DISTANCE,
         /// TIME, optimize time of route.
         TIME,
         /**
          * TIME_WITH_DISTURBANCES, optimize time of route with 
          * distsurbances.
          */
         TIME_WITH_DISTURBANCES,

         /// Number of routeCostType.
         NBR_ROUTECOSTTYPE
      };


      /**
       * routeCostType to route cost A,B,C,D.
       */
      static void routeCostTypeToCost( routeCostType cost, 
                                       byte& costA, byte& costB,  
                                       byte& costC, byte& costD );


      /**
       * Route cost A,B,C,D to routeCostType.
       * The algorith for convertion is simple: If costA != 0 then
       * it is DISTANCE, if costB != 0 then it is TIME, if costC != 0
       * then it is TIME_WITH_DISTURBANCES else it is TIME.
       *
       * @param costA The route cost A.
       * @param costB The route cost B.
       * @param costC The route cost C.
       * @param costD The route cost D.
       * @return The routeCostType for the route cost.
       */
      static routeCostType costToRouteCostType( byte costA, byte costB,  
                                                byte costC, byte costD );


      /**
       * String to routeCostType.
       *
       * @param str The string to match to route cost types.
       * @return The matching routeCostType to str, TIME if no match.
       */
      static routeCostType stringToRouteCostType( const char* str );


      /**
       * routeCostType to string.
       *
       * @param cost The routeCostType to convert to string.
       * @return The string matching const.
       */
      static const char* routeCostTypeToString( routeCostType cost );


  private:
      /**
       * Private constructor to avoid usage.
       */
      RouteTypes();
};


#endif // ROUTETYPES_H

