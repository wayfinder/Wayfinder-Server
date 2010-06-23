/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLCOMMONENTITIES_H
#define XMLCOMMONENTITIES_H

#include "config.h"

#ifdef USE_XML
#include <dom/DOM.hpp>

#include <util/PlatformUtils.hpp>
#include <util/XMLString.hpp>
#include <util/XMLUniDefs.hpp>

#include "MC2String.h"
#include "XMLUtility.h"
#include "TopRegionMatch.h"


/**
 * Common Elements and Entities in the MC2 system. 
 *
 *
 */
class XMLCommonEntities {
   public:
      /**
       * The types of cooridnates.
       */
      enum coordinateType {
         /// MC2 "664732208"
         MC2 = 0,
         /// WGS84,  (N|S|E|W) D(D*)MMSS[.ddd] "N 554301.9561"
         WGS84,
         /// WGS84Rad, "0.97244876500992347956"
         WGS84Rad,
         /// WGS84Deg, "55.71721002777777777777"
         WGS84Deg,
         
         /// Number of coordinateTypes.
         NBR_COORDINATETYPES
      };


      /**
       * Convert a string to a top region type.
       *
       * @param str The string with the supposed top region type.
       * @return The topRegion_t mathing str, nbr_topregion_t of no match.
       */
      static TopRegionMatch::topRegion_t stringToTopRegionType( 
         const char* str );


      /**
       * Convert a top region type to a string.
       *
       * @param type The top region type to convert.
       * @return A string representing the top region type.
       */
      static const char* topRegionTypeToString( TopRegionMatch::topRegion_t
                                                type );


      /**
       * Retuns the type of coordianteFormat that is specified in the 
       * str string.
       *
       * @param str Pointer to the coordianteFormat string.
       * @param defaultFormat The default format to use if no match in 
       *        coordianteFormat string. Default MC2.
       * @return The type of coordianteFormat that is specified in the 
       *         str string.
       */
      static coordinateType coordinateFormatFromString(
         const char* const str, coordinateType defaultFormat = MC2 );

      
      /**
       * Returns the string representing the coordinateformat.
       * @param format The coordinateformat.
       * @return A constant string representing the coordinateformat.
       */
      static const char* coordinateFormatToString( 
         coordinateType format );

      
      /**
       * Prints a string into target representing the coordinateformat.
       *
       * @param target The string to print into.
       * @param ordinate The latitude or longitude.
       * @param format The type of coordinate to write.
       * @param latitude True if the ordinate is a latitude and false if
       *        the oridinate is a longitude.
       * @return target.
       */
      static char* coordinateToString( char* target, 
                                       int32 ordinate, 
                                       coordinateType format,
                                       bool latitude );

      
      /**
       * Parses a string and extracs the coordinate in it.
       * 
       * @param coordianteStr The string with the coordinate.
       * @param ordinate Set to the coordinates mc2-value.
       * @param format The type of position system used.
       * return True if coordianteStr contained a valid coordinate, 
       *        false if not.
       */
      static bool coordinateFromString( const char* coordianteStr,
                                        int32& ordinate,
                                        coordinateType format );


   private:
      /**
       * Private constructor to avoid usage.
       */
      XMLCommonEntities();
};


#endif // USE_XML

#endif // XMLCOMMONENTITIES_H

