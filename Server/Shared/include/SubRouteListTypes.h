/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SUBROUTELISTTYPES_H
#define SUBROUTELISTTYPES_H

/**
 *   Class containing the different types of SubRouteLists.
 *   For compatibility with old RouteModule and new server.
 *
 */
class SubRouteListTypes {
public:
   /**
    * These are the list types used in the SubRouteList.
    */
   enum list_types {
      NOT_VALID,  /**< No route could be made (only from availables */
      HIGHER_LEVEL, /**< Route on higher level */
      HIGHER_LEVEL_FORWARD, /**< Route to higher level from origin */
      HIGHER_LEVEL_BACKWARD, /**< Route to higher level from destination */
      LOWER_LEVEL,           /**< Route on lower level */
      LOWER_LEVEL_WALK,      /**< Route on lower level for pedestrian */
      PROXIMITY_REQUEST,     /**< Proximity. Shouldn't be in the RM */
      PUBLIC_TRANSPORTATION, /**< Not implemented */
   };
};

#endif
