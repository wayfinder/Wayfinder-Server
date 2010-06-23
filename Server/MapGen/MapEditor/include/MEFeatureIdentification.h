/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEFEATUREIDENTIFICATION_H
#define MEFEATUREIDENTIFICATION_H

#include "config.h"
#include "MC2String.h"
#include "OldGenericMap.h"
#include "OldExtraDataUtility.h"
#include "OldNode.h"

class OldGenericMap;
class OldItem;
class OldNode;
class OldConnection;

/**
 *    Class for creation of feature identification strings. The strings are
 *    written in extra data format (using the extra data record separator).
 *    Features implemented: item, node, connection.
 */
class MEFeatureIdentification {
   public:

      /**
       *    Get identification string for an item. Used by e.g. EditNameDialog.
       *    "itemType<¡>mc2<¡>(lat, lon)<¡>itemName<¡>nameType<¡>nameLang<¡>"
       *
       *    @param   identString The identification string, outparam.
       *    @param   theMap   The item is located in this map.
       *    @param   item     The item to get identification string for.
       *    @param   selectedNameOffset  If editing name: index of the name
       *                      selected in the item dialog Names list. 
       *                      Default -1.
       *    @param   includeNameTypeAndLang  If the extra data record 
       *                      identification should include nameType and 
       *                      language. Default is true, false applies for
       *                      removeItem records.
       *
       *    @return  True if the identification string was created ok, false
       *             if e.g. the item could not be unique identified.
       */
      static bool getItemIdentificationString(
			MC2String& identString,
                        OldGenericMap* theMap, OldItem* item,
                        int selectedNameOffset = -1,
                        bool includeNameTypeAndLang = true);

      /**
       *    Get identification string for a connection.
       *    "(fromLat1, fromLon1);(fromLat2, fromLon2);(toLat1, toLon1);
       *     (toLat2, toLon2);"
       *
       *    @param   identString The identification string, outparam.
       *    @param   theMap   The connection is located in this map.
       *    @param   conn     The connection.
       *    @param   toNode   The node to which the connection leads.
       *
       *    @return  True if the identification string was created ok,
       *             false if not.
       */
      static bool getConnectionIdentificationString(
                     MC2String& identString,
                     OldGenericMap* theMap, OldConnection* conn,
                     OldNode* toNode);

      /**
       *    Get identification string for a node.
       *    "(lat1, lon1);(lat2, lon2);"
       *
       *    @param   identString The identification string, outparam.
       *    @param   theMap      The node is located in this map.
       *    @param   node        The node.
       *
       *    @return  True if the identification string was created ok,
       *             false if not.
       */
      static bool getNodeIdentificationString(
                     MC2String& identString,
                     OldGenericMap* theMap, OldNode* node);

};

#endif

