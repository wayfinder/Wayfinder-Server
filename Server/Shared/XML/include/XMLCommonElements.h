/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLCOMMONELEMENTS_H
#define XMLCOMMONELEMENTS_H

#include "config.h"

#ifdef USE_XML
#define XERCES_STD_NAMESPACE
#include <dom/DOM.hpp>

#include <util/PlatformUtils.hpp>
#include <util/XMLString.hpp>
#include <util/XMLUniDefs.hpp>

#include "MC2String.h"
#include "XMLUtility.h"
#include "XMLCommonEntities.h"


/**
 * Common Elements in the MC2 system. 
 *
 *
 */
class XMLCommonElements {
   public:
      /**
       * Extracs lat and lon from a position_item.
       *
       * @param positionItem The position_item to get data from.
       * @param lat Set to the latitude of the position_item in 
       *        mc2-coordinates.
       * @param lon Set to the longitude of the position_item in 
       *        mc2-coordinates.
       * @param angle Set to the angle of the positioniten, MAX_UINT16 if
       *              none.
       * @param errorCode If problem then this is set to error code.
       * @param errorMessage If problem then this is set to error message.
       * @param coordType will contain the coord type that was parsed. (optional)
       * @return True if positionItem really is a position_item node with
       *         valid lat and lon.
       */
      static bool getPositionItemData( const DOMNode* positionItem,
                                       int32& lat, int32& lon,
                                       uint16& angle,
                                       MC2String& errorCode, 
                                       MC2String& errorMessage,
                                       XMLCommonEntities::
                                       coordinateType* coordType = NULL );

   /// @see the other getPositionItemData
   static bool getPositionItemData( const DOMNode* positionItem,
                                    MC2Coordinate& coord,
                                    uint16& angle,
                                    MC2String& errorCode, 
                                    MC2String& errorMessage,
                                    XMLCommonEntities::
                                    coordinateType* coordType = NULL ) {
      return getPositionItemData( positionItem,
                                  coord.lat, coord.lon, angle,
                                  errorCode, errorMessage,
                                  coordType );
   }
      /**
       * Reads boundingbox node at curr.
       *
       * @param curr The boundingbox node.
       * @param bbox The MC2BoundingBox to fill the data into.
       * @param errorCode If problem then this is set to error code.
       * @param errorMessage If problem then this is set to error message.
       * @return True if curr really is a boundingbox node, false if not.
       */
      static bool readBoundingbox( const DOMNode* cur, MC2BoundingBox& bbox,
                                   MC2String& errorCode, 
                                   MC2String& errorMessage );


   private:
      /**
       * Private constructor to avoid usage.
       */
      XMLCommonElements();
};


#endif // USE_XML

#endif // XMLCOMMONELEMENTS_H

