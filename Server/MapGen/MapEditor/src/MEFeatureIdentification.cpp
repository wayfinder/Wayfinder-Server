/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MEFeatureIdentification.h"


bool
MEFeatureIdentification::getItemIdentificationString(
      MC2String& identString,
      OldGenericMap* theMap, OldItem* item,
      int selectedNameOffset,
      bool includeNameTypeAndLang )
{

   if ( theMap == NULL ) {
      return false;
   }
   if ( item == NULL ) {
      return false;
   }

   // Parameters that are set in the getCoordinateFromItem-method
   int32 lat = 0;
   int32 lon = 0;
   byte nameOffset = MAX_BYTE;
   
   if ( theMap->getCoordinateFromItem(item, lat, lon, nameOffset) ) {
      
      // Vector to collect all identification sub strings
      vector<MC2String> identStrings;
      
      // Start adding the identification values to ident strings vector
      identStrings.push_back( StringTable::getString(
            ItemTypes::getItemTypeSC(item->getItemType()), 
            StringTable::ENGLISH) );
      identStrings.push_back( "mc2" );
      char tmpStr[128];
      sprintf(tmpStr, "(%d, %d)", lat, lon);
      identStrings.push_back( tmpStr );

      // nameOffset is the outparam from getCoordinateFromItem-method,
      // but if there is one name selected in the Names list we must use that 
      // name offset (EditName: updateName or removeNameFromItem)
      mc2dbg4 << "getItemIdentification nameOffset=" << int(nameOffset)
              << " (selectedNameOffset=" << selectedNameOffset << ")" << endl;
      if ( (selectedNameOffset >= 0) &&
           (selectedNameOffset < item->getNbrNames()) ) {
         // We have the case updateName or removeNameFromItem and must use 
         // selectedNameOffset which is the selected name.
         if ( nameOffset != selectedNameOffset ) {
            // is the item found using selectedNameOffset
            uint32 strIdx = item->getStringIndex(selectedNameOffset);
            vector<OldItem*> closeItems;
            OldItem* testItem = theMap->getItemFromCoordinate(
                     item->getItemType(), lat, lon,
                     closeItems, theMap->getName(strIdx), 1);
            if ( testItem != item ) {
               mc2log << error << "getItemIdentification getItemFromCoordinate"
                      << " failed using selectedNameOffset" << endl;
               return false;
            }
         }
         nameOffset = selectedNameOffset;
         
      }
      // else we have e.g. the case EditName:addNameToItem. Then we must 
      // use nameOffset because selectedNameOffset is not a valid offset.

      // Add original name (if exists), include type and lang only if requested
      if ( nameOffset != MAX_BYTE ) {
         uint32 stringIndex = item->getStringIndex(nameOffset);
         identStrings.push_back( theMap->getName(stringIndex) );
         if ( includeNameTypeAndLang ) {
            identStrings.push_back(
               ItemTypes::getNameTypeAsString(item->getNameType(nameOffset)));
            identStrings.push_back(
               LangTypes::getLanguageAsString(
                  item->getNameLanguage(nameOffset)) );
         }
      
      } else {
         // No old name, push empty strings
         identStrings.push_back( "" ); // name
         if ( includeNameTypeAndLang ) {
            identStrings.push_back( "" ); // name type
            identStrings.push_back( "" ); // name lang
         }
      }

      identString = OldExtraDataUtility::createEDFileString(identStrings);
      return true;
   }

   mc2log << error << here << "FAILED to get unique item-identification for" 
          << item->getID() << endl;
   return false;
}

bool
MEFeatureIdentification::getConnectionIdentificationString(
         MC2String& identString,
         OldGenericMap* theMap, OldConnection* conn, OldNode* toNode)
{
   if ( theMap == NULL )
      return false;
   if ( conn == NULL )
      return false;
   if ( toNode == NULL )
      return false;

   // Get coordinates for the toNode
   int32 lat1, lon1, lat2, lon2;
   if ( !theMap->getCoordinatesFromNode( toNode, lat1, lon1, lat2, lon2) ) {
      mc2log << error << here << "Failed to get coords for toNode" << endl;
      return false;
   }

   // Get the coordinates of the fromNode
   OldNode* fromNode = theMap->nodeLookup(conn->getConnectFromNode());
   int32 fromLat1, fromLon1, fromLat2, fromLon2;
   if ( (fromNode == NULL) ||
        (!theMap->getCoordinatesFromNode(
          fromNode, fromLat1, fromLon1, fromLat2, fromLon2))) {
      mc2log << error << here << "Failed to lookup fromNode" << endl;
      return false;
   }

   vector<MC2String> identStrings;

   char tmpStr[128];
   sprintf(tmpStr, "(%d, %d)", fromLat1, fromLon1);
   identStrings.push_back( tmpStr );
   sprintf(tmpStr, "(%d, %d)", fromLat2, fromLon2);
   identStrings.push_back( tmpStr );
   sprintf(tmpStr, "(%d, %d)", lat1, lon1);
   identStrings.push_back( tmpStr );
   sprintf(tmpStr, "(%d, %d)", lat2, lon2);
   identStrings.push_back( tmpStr );
   
   identString = OldExtraDataUtility::createEDFileString(identStrings);

   return true;
}

bool
MEFeatureIdentification::getNodeIdentificationString(
               MC2String& identString, OldGenericMap* theMap, OldNode* node)
{
   if ( theMap == NULL ) {
      return false;
   }
   if ( node == NULL ) {
      return false;
   }
   
   int32 lat1, lon1, lat2, lon2;
   if ( ! theMap->getCoordinatesFromNode(node, lat1, lon1, lat2, lon2) ) {
      mc2log << error << here
             << "Failed to lookup node, node=" << node->getNodeID() << endl;
      return false;
   }
   
   vector<MC2String> identStrings;

   char tmpStr[128];
   sprintf(tmpStr, "(%d, %d)", lat1, lon1);
   identStrings.push_back( tmpStr );
   sprintf(tmpStr, "(%d, %d)", lat2, lon2);
   identStrings.push_back( tmpStr );
   
   identString = OldExtraDataUtility::createEDFileString(identStrings);
   
   return true;
}

