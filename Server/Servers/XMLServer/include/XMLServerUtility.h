/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLSERVERUTILITY_H
#define XMLSERVERUTILITY_H

#include "config.h"

#include "UserConstants.h"
#include "ExpandedRouteItem.h"
#include "UserData.h"
#include "UserIDKey.h"

#include <iosfwd>

// Most functions moved from XMLParserThread
namespace XMLServerUtility {
/**
 * Reads a size_t xml string.
 *
 * @param str The MC2String with the size_t value.
 * @param value Set to the value of str.
 * @return True if str is a size_t string, false if not.
 */
bool readSizeValue( const MC2String& str, uint32& value );

/**
 * Prints a MC2String with line numbering before each line.
 *
 * @param out The ostream to print to.
 * @param str The MC2String to print.
 * @param preStr The MC2String to print first on line before line 
 *               number. Default "Line ".
 * @param nbrWidth The width of the line number field. -1 means that
 *                 the number of lines should be calculated and width
 *                 set accordingly. Default -1.
 */
void printWithLineNumbers( ostream& out, const char* str,
                           const char* preStr = "Line ", 
                           int nbrWidth = -1 );

/**
 * Turn a RouteTurnImageType into a string.
 *
 * @param turnImageType The RouteTurnImageType to return MC2String for.
 * @return The MC2String for the RouteTurnImageType.
 */
const char* 
routeTurnImageTypeToString( UserConstants::RouteTurnImageType type );

/**
 * Turns a ExpandedRouteItem::routeturn_t to string.
 *
 * @param turn The routeturn_t.
 * @return The MC2String for the routeturn_t.
 */
const char* routeturnTypeToString( ExpandedRouteItem::routeturn_t turn, 
                                   bool endOfRoad );

/**
 * Checks a MC2String for RouteTurnImageType.
 *
 * @param str The MC2String to check.
 * @param defaultType The type to return if str doesn't match any
 *        RouteTurnImageType, default 
 *        ROUTE_TURN_IMAGE_TYPE_MAP_IMAGE.
 */
UserConstants::RouteTurnImageType 
routeTurnImageTypeFromString( const char* str, 
                              UserConstants::RouteTurnImageType defaultType = 
                              UserConstants::ROUTE_TURN_IMAGE_TYPE_MAP_IMAGE );


/**
 * Checks a string for RouteImageType.
 *
 * @param str The MC2String to check.
 * @return The image type or ROUTEIMAGETYPE_NBR if no match.
 */
UserConstants::RouteImageType overviewImageTypeFromString( const char* str );

/**
 * Turn a RouteImageType into a string.
 *
 * @param imageType The RouteImageType to return string for.
 * @return The string for the RouteTurnImageType.
 */
const char* overviewImageTypeToString( UserConstants::RouteImageType type );

/**
 * Checks a string for transactionBased_t.
 *
 * @param str The string to check.
 * @return The transactionBased_t or NBR_TRANSACTION_T if no match.
 */
UserConstants::transactionBased_t 
transactionBasedTypeFromString( const char* str );


/**
 * Turn a transactionBased_t into a string.
 *
 * @param transType The transactionBased_t to return string for.
 * @return The string for the transactionBased_t.
 */
const char* 
transactionBasedTypeToString( UserConstants::
                              transactionBased_t transType );


/**
 * Converts a string to UserIDKey::idKey_t.
 */
UserIDKey::idKey_t idKeyTypeFromString( const char* str );


/**
 * Converts a UserIDKey::idKey_t to string.
 */
const char* idKeyTypeToString( UserIDKey::idKey_t type );

}

#endif // XMLSERVERUTILITY_H
