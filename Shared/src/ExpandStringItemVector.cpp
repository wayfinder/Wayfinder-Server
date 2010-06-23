/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandStringItemVector.h"
#include "DeleteHelpers.h"

#define MAX_SMS_LENGTH 160
#define MAX_CONCATENATED_SMS_SIZE 152
#define MAX_SMS_ITEM_LENGTH 140
#define TMP_LENGTH 255


ExpandStringItemVector::ExpandStringItemVector(uint32 nbrItems)
{
   m_items.reserve( nbrItems );
}


ExpandStringItemVector::~ExpandStringItemVector()
{

}


bool 
ExpandStringItemVector::
getRouteAsSMS( vector<char*>& destVector, 
               uint32 totRouteLength, 
               uint32 totRouteTime, 
               uint32 totRouteStandStillTime, 
               StringTable::languageCode language,
               UserConstants::EOLType eol, 
               int maxLineLength,
               bool concatinatedSMS,
               const char* preText, 
               const char* postText) 
{
   vector<char*> allStrings;
   allStrings.reserve( m_items.size() );
   vector<size_t> allStringSize;
   allStringSize.reserve( m_items.size() );

   // Create footer
   char* footerText = new char[ 
      strlen( StringTable::getString( 
                 StringTable::ROUTE_CONTINUES, language ) )*3 + 1 ];
   SMSStringUtility::printSMSString(
      footerText, 
      StringTable::getString( StringTable::ROUTE_CONTINUES, language),
      language, eol , maxLineLength, 0, 1, true);
   const uint32 footerSize = strlen(footerText);

   // Create header
   const char* routeText = StringTable::getString(StringTable::ROUTE,
                                                  language);
   char* header = new char[ strlen( routeText ) + 14 + 2 + 1 ];
   int headerSize = sprintf(header, "%s %u/NN", routeText, 1);
   headerSize += SMSStringUtility::writeEol(header, headerSize, eol, 5);
   header[headerSize] = '\0';


   const uint32 maxInStringLength = (concatinatedSMS ? 
                                     MAX_CONCATENATED_SMS_SIZE : 
                                     (MAX_SMS_LENGTH - footerSize - 
                                      headerSize) );
   cerr << "maxInStringLength " << maxInStringLength << endl;

   // Add the preText
   if ( (preText != NULL) && (strlen(preText) > 0) ) {
      // FIXME: Replace all endlines with EOL in text, make sure < MAX_SIZE
      char* tmpStr = new char[MAX_SMS_LENGTH + 1];
      strncpy( tmpStr, preText, maxInStringLength );
      tmpStr[ maxInStringLength ] = '\0';
      allStrings.push_back( tmpStr );
      allStringSize.push_back( strlen( tmpStr ) );
   }

   // Add all stringItems
   uint32 i;
   for (i=0; i < m_items.size(); i++) {
      ExpandStringItem* curItem = m_items[ i ];
      if  (curItem != NULL) {
         char* tmpStr = new char[MAX_SMS_LENGTH + 1];
         int tmpLen = curItem->getRouteDescriptionSMS(language, 
                                                      eol, 
                                                      maxLineLength,
                                                      tmpStr, 
                                                      MAX_SMS_ITEM_LENGTH);
         if (tmpLen > 0) {
            allStrings.push_back(tmpStr);
            allStringSize.push_back(tmpLen);
         }
      }
   }

   // Add the postText
   if ( (postText != NULL) && (strlen(postText) > 0) ) {
      // FIXME: Replace all endlines with EOL in text, make sure < MAX_SIZE
      char* tmpStr = new char[MAX_SMS_LENGTH + 1];
      strncpy( tmpStr, postText, maxInStringLength );
      tmpStr[ maxInStringLength ] = '\0';
      allStrings.push_back( tmpStr );
      allStringSize.push_back( strlen( tmpStr ) );
   }


   // Split the strings in allString into some route-SMS:es

   // Fist calculate total length to se if only one SMS
   uint32 totLength = 0;
   for ( i=0; i < allStrings.size(); i++){
      totLength += allStringSize[ i ];
   }

   if (totLength < MAX_SMS_LENGTH) {
      // All text fits into one SMS, no header och footer necessary!
      char* curSMS = new char[totLength+1];
      curSMS[0] = '\0';
      for (uint32 i=0; i < m_items.size(); i++){
         strcat(curSMS, allStrings[ i ]);
      }
      destVector.push_back(curSMS);
      delete [] footerText;
      delete [] header;
      STLUtility::deleteArrayValues( allStrings );
      return (true);
   } 
   else if( concatinatedSMS ){
      // More than one SMS necessary, but no headers and footers...
      // Create the SMSes and insert them into the outparameter (destVector)
      uint32 i=0;
      while( i < allStrings.size() ){
         uint32 totLength = allStringSize[ i ];
         char* curSMS = new char[MAX_SMS_LENGTH + 1];
         curSMS[0] = '\0';
         while( totLength <= MAX_CONCATENATED_SMS_SIZE && 
                i < allStrings.size() )
         {
            strcat(curSMS, allStrings[i]);
            i++;
            if ( i < allStrings.size() ) {
               totLength += allStringSize[ i ];
            }
         }
         destVector.push_back(curSMS);
      }      
      delete [] footerText;
      delete [] header;
      STLUtility::deleteArrayValues( allStrings );
      return (true);
   }
   else {
      // More than one SMS necessary, create headers and footers...
      vector<uint32> headerPositions; 
      // Create the first header and insert into allStrings
      allStrings.insert(allStrings.begin(), header);
      allStringSize.insert(allStringSize.begin(), strlen(header));
      headerPositions.push_back(0);

      // Insert headers and footers without totNbr (NN)
      uint32 curSize = headerSize;
      uint32 curSMSNumber = 1;
      uint32 i=0;


      while (i < allStrings.size()) {
         
         if ( (curSize + allStringSize[ i ]) >
              (MAX_SMS_LENGTH-footerSize) ) 
         {
            // Add footer
            char* footer = StringUtility::newStrDup(footerText);
            allStrings.insert( allStrings.begin() + i, footer);
            allStringSize.insert( allStringSize.begin() + i, footerSize);
            i++;
            
            // Add header
            char* header = new char[ strlen( routeText ) + 14 + 2 + 1 ];
            int headerPos = sprintf(header, "%s %u/NN", 
                                    routeText, curSMSNumber+1 );
            headerPos += SMSStringUtility::writeEol( header, headerPos,
                                                     eol, 5 );
            header[headerPos] = '\0';

            allStrings.insert( allStrings.begin() + i, header);
            allStringSize.insert( allStringSize.begin() + i, strlen(header));
            headerPositions.push_back(i);

            // Update variables
            curSMSNumber++;
            curSize = headerSize;
         } else {
            curSize += allStringSize[ i ];
         }
         i++;
      }

      // Change all the "NN" to the total number of SMSes (curSMSNumber+1)
      for (i=0; i<headerPositions.size(); i++) {
         // Write the number to a temporary str
         char nbrStr[ 11 ];
         sprintf(nbrStr,"%u", curSMSNumber);
         char* header = allStrings[ headerPositions[i] ];
         char* nnPos = strchr(header, '/')+1;
         if (strlen(nbrStr) < 3) {
            unsigned int j;
            for (j=0; j<strlen(nbrStr); j++) {
               nnPos[j] = nbrStr[j];
            }
            for (j=strlen(nbrStr); j<3; j++) {
               nnPos[j] = ' ';
            }
         }

      }

      // Add the last string to headerPos...
      headerPositions.push_back(allStrings.size());

      // Create the SMSes and insert them into the outparameter (destVector)
      for (i=0; i<headerPositions.size()-1; i++) {
         char* curSMS = new char[MAX_SMS_LENGTH + 1];
         curSMS[0] = '\0';
         for (uint32 j=headerPositions[i]; j<headerPositions[i+1]; j++) {
            strcat(curSMS, allStrings[j]);
         }

         destVector.push_back(curSMS);
      }
      
      delete [] footerText;
      STLUtility::deleteArrayValues( allStrings );
      return (true);
   }

   delete [] footerText;
   STLUtility::deleteArrayValues( allStrings );
   // Not possible to get here...
   return (false);
}

void ExpandStringItemVector::deleteAllObjs() {
   STLUtility::deleteValues( m_items );
}

