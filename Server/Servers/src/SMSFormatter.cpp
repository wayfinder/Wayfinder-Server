/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SMSFormatter.h"

#include "StringTable.h"
#include "StringUtility.h"
#include "SMSStringUtility.h"
#include "ExpandStringItem.h"
#include "ExpandRoutePacket.h"
#include "ExpandItemID.h"
#include "SMSSendRequest.h"
#include "SearchMatch.h"
#include "UserData.h"
#include "Utility.h"

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <typeinfo>

#define FOOTER_SIZE 8

const uint32 SMSFormatter::maxNbrChoiceRows = 2;

#define oldD1(a) DEBUG1(a)
#undef DEBUG1
#define oldD2(a) DEBUG2(a)
#undef DEBUG2
#define oldD4(a) DEBUG4(a)
#undef DEBUG4
//#define oldD8(a) DEBUG8(a)
//#undef DEBUG8

#define   DEBUG1(a)   cerr << __FILE__ << ":" << __LINE__ << ":\t" ; a
#define   DEBUG2(a)   cerr << __FILE__ << ":" << __LINE__ << ":\t" ; a
#define   DEBUG4(a)   cerr << __FILE__ << ":" << __LINE__ << ":\t" ; a
//#define   DEBUG8(a)   cerr << __FILE__ << ":" << __LINE__ << ":\t" ; a

char *
SMSFormatter::getRouteSummary( ExpandRouteReplyPacket *ePack,
                               uint32 phoneWidth,
                               UserConstants::EOLType eol,
                               StringTable::languageCode language)
{
   DEBUG4( cerr << "SMSFormatter::getRouteSummary" << endl );
   char *result = NULL;
   if (ePack != NULL) {
      result = new char[512]; result[0] = '\0';
      char *extraData = new char[256];
      // Add total length to curData
      uint32 totDistance = ePack->getTotalDist();
      uint32 extraPos =
         sprintf(extraData, "%s ",  
                 StringTable::getString(StringTable::TOTAL_DISTANCE, 
                                        language));
      

      extraPos += StringTableUtility::
         printDistance( extraData + extraPos,
                        totDistance, language,
                        StringTableUtility::COMPACT,
                        StringTableUtility::METERS );
      extraPos += writeEol(extraData, extraPos, eol, phoneWidth-extraPos);
      extraData[extraPos] = '\0';
      strcat(result, extraData);

/*      
        // Total time
        char* timeStr = new char[32];
        uint32 totTime = ePack->getTotalTime();
        TimeUtility::splitSeconds(totTime, timeStr);
        sprintf(extraData,
        "%s ",  
        StringTable::getString(StringTable::TOTAL_TIME, 
        language) );
        strcat(extraData, timeStr);
        extraPos = strlen(extraData);
        extraPos += writeEol(extraData, extraPos, eol, phoneWidth-extraPos);
        extraData[extraPos] = '\0';
        strcat(result, extraData);
      
        // Total stand still time
        uint32 standStillTime = ePack->getStandStillTime();
        TimeUtility::splitSeconds(standStillTime, timeStr);
        sprintf(extraData, "%s ",  
        StringTable::getString(StringTable::TOTAL_STANDSTILL_TIME, 
        language) );
        strcat(extraData, timeStr);
        extraPos = strlen(extraData);
        extraPos += writeEol(extraData, extraPos, eol, phoneWidth-extraPos);
        extraData[extraPos] = '\0';
        delete timeStr;

        strcat(result, extraData);
*/
      delete [] extraData;
   }
   return result;
}

void
SMSFormatter::getCompanyStreetNumber(char *&res, VanillaMatch *match)
{
   res[0] = '\0';
   if ((match != NULL) &&
       (typeid(*match) == typeid(VanillaCompanyMatch)))
   {
      sprintf(res, " %u", static_cast<VanillaCompanyMatch *>
              (match)->getStreetNbr());
   }
}

SMSSendRequest*
SMSFormatter::getRouteSMSes(ExpandRouteReplyPacket* ePack,
                            const char* const senderPhone,
                            const char* const receiverPhone,
                            bool bConcatenatedSMS,
                            uint16 requestID,
                            VanillaMatch *origin,
                            OverviewMatch *origLocation,
                            VanillaMatch *destination,
                            OverviewMatch *destLocation,
                            SMSUserSettings *smsUserFormat)
{
   DEBUG4( cerr << "SMSFormatter::getRouteSMSes" << endl );
   
   // ***** init part
   
   // get formatting info
   int phoneWidth = smsUserFormat->getSMSLineLength();
   UserConstants::EOLType eol = smsUserFormat->getEOLType();
   
   if (ePack == NULL) return (NULL);
   // ePack is not null after this line.
   
   SMSSendRequest* retRequest = new SMSSendRequest(requestID);

   // Initiate the variables with data from the packet
   ExpandStringItem** strItemVector = ePack->getStringDataItem();
   uint32 nbrItems = ePack->getNumStringData();

   char* curData = new char[512];
   curData[0] = '\0';
   
   char* extraData = new char[256];

   // The count of the current SMS
   int32 smsNbr = 0;

   StringTable::languageCode language = smsUserFormat->getSMSLanguage();
   char *summary = getRouteSummary( ePack, phoneWidth, eol, language );
   // Approximate the total number of SMSes
   uint32 totalNbrOfSMSes = 0;
   
   // Get the first packet!
   curData = createNewPacket( smsNbr,
                              totalNbrOfSMSes,
                              (totalNbrOfSMSes <= 1),
                              false,
                              bConcatenatedSMS,
                              senderPhone,
                              receiverPhone, 
                              curData,
                              retRequest,
                              smsUserFormat );

   // print the (cut off) full name of the destination.
   uint32 extraPos = sprintf( extraData, StringTable::getString( 
      StringTable::DESTINATION,
      language ) ); // To
   extraData[ extraPos++ ] = ' ';
   extraData[ extraPos ] = '\0';

   char *destName = getNewMatchStringWithLocation( destination );
   /*
   if (destination != NULL) {
      VanillaStreetMatch *vsm =
         dynamic_cast<VanillaStreetMatch *>( destination );
      if (vsm != NULL) {
         DEBUG4( cerr << "dest offset: " << vsm->getOffset() << endl );
      } else {
         DEBUG4( cerr << "dest not a street (no offset)" << endl );
      }
   } else {
      cerr << "dest == NULL" << endl;
   }
   if (origin != NULL) {
      VanillaStreetMatch *vsm =
         dynamic_cast<VanillaStreetMatch *>( origin );
      if (vsm != NULL) {
         DEBUG4( cerr << "orig offset: " << vsm->getOffset() << endl );
      } else {
         DEBUG4( cerr << "orig not a street (no offset)" << endl );
      }
   } else {
      cerr << "orig == NULL" << endl;
   }
   */
   extraPos += printSMSString( extraData+strlen(extraData),
                               destName, language,
                               eol, phoneWidth,
                               extraPos, // destlinepos
                               5, // max number lines
                               true );
   strcat(curData, extraData);
   
   // then print the summary in the beginning
   strcat( curData, summary );
   delete [] summary; summary = NULL;
   
   // ***** loop part *****
   
   // Print all the items in the route
   ExpandStringItem* curItem = NULL;
   // do the start item (first one)
   extraPos = 0;
   char *turnDescription = new char[256];
   memset( turnDescription, 0, 256 );

   // 256 bytes ought to be enough for everyone
   if (nbrItems>0) {
      curItem = strItemVector[0];
      
      extraPos = sprintf( extraData + extraPos, "%s %s %s",
                          StringTable::getString(
                             StringTable::stringCode(
                                curItem->getStringCode()),
                             language),
                          StringTable::getString(
                             Utility::getCompass(curItem->getDist(), 2),
                             language),
                          StringTable::getString( StringTable::DIR_AT,
                                                  language)
                          );
      extraPos += writeEol( extraData, extraPos, eol,
                            phoneWidth - extraPos );
      char tmpStr[256];
      tmpStr[ 0 ] = 0;

//      char *origLocStr = getNewMatchString( origLocation );
      char *origLStr = getNewMatchStringLocation( origin );
      char *companyStreetNumber = new char[20];
      getCompanyStreetNumber(companyStreetNumber, origin);

      snprintf( tmpStr, 256, "%s:%s%s",
                origLStr, curItem->getText(),
                companyStreetNumber );
      delete [] companyStreetNumber;
      delete [] origLStr;
      extraPos += printSMSString( extraData + extraPos,
                                  tmpStr, language, eol,
                                  phoneWidth, 0, 4, true );
      extraData[extraPos] = '\0';
      strcat( curData, extraData );
   }
   uint32 footerSize;
   uint32 maxSMSSize;
   if(bConcatenatedSMS){
      footerSize = 0;
      maxSMSSize = MAX_CONCATENATED_SMS_SIZE;
   }
   else{
      footerSize = FOOTER_SIZE; // TODO: Add smslen( StringTable::ROUTE_CONT..)
      maxSMSSize = MAX_SMS_SIZE; 
   }
   
   for (uint32 i=1; i<nbrItems; i++) {
      // Initiate the current item
      curItem = strItemVector[i];

      // First write turndescription to the temporary data buffer
      uint32 ec = curItem->getExitCount();
      if (ec > 9) { ec = 0; } // don't print exitcount if > 9.
      StringTable::getRouteDescription(
         curItem->getStringCode(),
         language,
         ec,
         turnDescription, 
         255);
      extraPos = StringTableUtility::printDistance(extraData,
                                              curItem->getDist(),
                                              language,
                                              StringTableUtility::COMPACT,
                                              StringTableUtility::METERS);
      extraPos += sprintf( extraData + extraPos, " %s",
                           turnDescription );
      extraPos += writeEol( extraData, extraPos, eol,
                            phoneWidth - extraPos );
      if (curItem->hasName() != 0) {
         char *streetName = StringUtility::newStrDup( curItem->getText() );
         uint32 nbrRows = 1;
//         if (strlen(streetName) > ((uint32) phoneWidth + 4)) {
            // if there are more than 4 characters for the next row,
            // print a second row of the street name.
            nbrRows = 2;
//         }

         if ((i+1)==nbrItems) { // if last item:
            // add possible street number for companies
            delete [] streetName;
            streetName = new char[256];
            char *companyStreetNumber = new char[20];
            getCompanyStreetNumber( companyStreetNumber, destination );
            sprintf(streetName, "%s%s",
                    curItem->getText(), companyStreetNumber);
            delete [] companyStreetNumber;
            // and let nbrRows = 4 !!! so that we don't cut away the
            // number we just added...
            nbrRows = 4;
         }
         
         extraPos += printSMSString(extraData + extraPos,
                                    streetName, language, eol,
                                    phoneWidth, 0, nbrRows, true);
         delete [] streetName;
      } // else we don't write anything (for missing street names)
      extraData[extraPos] = '\0';
      
      // Check if the item (in extraData) will fit into the current packet
      if ( extraPos + strlen(curData) + footerSize > maxSMSSize ) {
         // Check if last SMS
         bool lastSMS = false;
         if ( i >= nbrItems ) {
            DEBUG4(cerr << "Last sms" << endl;);
            // All routeTurns printed check if 
            // write-full-name-of-destination fits too
            char* destNameSMS = NULL;
            uint32 destNameSMSLen = 0;
            newSMSString( destNameSMS, destNameSMSLen,
                          destName, smsUserFormat );
            DEBUG4( cerr << "Prestrlen(destName)" << strlen(destName)
                    << "strlen(curData)"  << strlen(curData)
                    << "maxSMSSize" << maxSMSSize
                    << "destNameSMSLen" << destNameSMSLen << endl );
            if ( (destNameSMSLen + strlen(curData)) <= maxSMSSize ) {
               lastSMS = true;
            }
            delete [] destNameSMS;
         } else {
            DEBUG4(cerr << "not last sms" << endl;);
         }

         // This part of the description will not fit into the message
         curData = createNewPacket(
            smsNbr,
            totalNbrOfSMSes,
            lastSMS, 
            true,
            bConcatenatedSMS,
            senderPhone, receiverPhone,
            curData, retRequest, smsUserFormat );
      }

      // Add the extraData to curData
      strcat(curData, extraData);

   }

   // ***** end part *****
   delete [] turnDescription;
   
   // begin write-full-name-of-destination
   char *destNameSMS = NULL;
   uint32 destNameSMSLen = 0;
   newSMSString( destNameSMS, destNameSMSLen,
                 destName, smsUserFormat );
   DEBUG4( cerr << "strlen(destName)" << strlen(destName)
           << "strlen(curData)"  << strlen(curData)
           << "maxSMSSize" << maxSMSSize
           << "destNameSMSLen" << destNameSMSLen << endl );
   
   if (/*( strncmp( destName,
         curItem->getText(),
         strlen(curItem->getText())) == 0 ) &&*/
      ( destNameSMSLen + strlen(curData)) > maxSMSSize )
   {
      curData = createNewPacket(
         smsNbr,
         totalNbrOfSMSes,
         false, // Not last because destName will be added in next!
         true,
         bConcatenatedSMS,
         senderPhone,
         receiverPhone,
         curData, retRequest, smsUserFormat );
   }
/*
  int startOffset = 0;

   if ((nbrItems>0) &&
       (strncmp( destName,
                 curItem->getText(),
                 MIN((int)
                     strlen(
                        curItem->getText()),
                     (int)
                     phoneWidth-1)) == 0))
   {
      // start of extraData is same as last item.
      startOffset = MIN( (int)
                         strlen(
                            curItem->getText()),
                         (int)
                         phoneWidth-1 );
   }
   printSMSString( extraData, destName+startOffset,
                   eol, phoneWidth, 0, 3, true );
*/
   DEBUG4( cerr << "destNameSMS: " << destNameSMS << endl );
   strcat( curData, destNameSMS );
   delete [] destName;
   delete [] destNameSMS;
//   strcat(curData, extraData);
   // end write-full-name-of-destination
   
   if (nbrItems > 0) { // This is true in 99.99999% of all expn. routes.
      // Create the last SMS packet
      curData = createNewPacket(
         smsNbr,
         totalNbrOfSMSes,
         true,
         true,
         bConcatenatedSMS,
         senderPhone,
         receiverPhone,
         curData,
         retRequest,
         smsUserFormat );
   }

   // Print the correct total number of SMSes y as in "Route x/y".
   // Unpack all of it, change the count, and repack it.
   if ( totalNbrOfSMSes < 100 ) {
      // replace xx by the correct number.
      char *totNbrString = new char[16];
      sprintf( totNbrString, "%2u", totalNbrOfSMSes );
      SMSSendRequest *tmpRequest = retRequest;
      retRequest = new SMSSendRequest(requestID);
      PacketContainer *pc = tmpRequest->getNextPacket();
      int partNbr = 1;
      while (pc != NULL) {
         SMSSendRequestPacket *srp = static_cast<SMSSendRequestPacket *>
            ( pc->getPacket() );
         byte *data = NULL; int32 dataLength = 0;
         char *tmps, *tmpr;
         srp->getPacketContents( CONVERSION_NO, tmps, tmpr,
                                 dataLength, data );
         if ( (dataLength > 10) && !bConcatenatedSMS ) {
            char *lp = strchr( (char *) data, '/' );
            if (lp != NULL) {
               strncpy( lp+1, totNbrString, 2 ); // '(xx)'->'( 1)'
            }
            if (totalNbrOfSMSes < 10) { // just one character, remove space
               strncpy( lp+1, lp+2, dataLength-(lp+1-(char*)data) );
               dataLength--;
            }
         }
         byte tempSmsNbr = (((requestID)<<1) & 0xfe) | 1;

         
         SMSSendRequestPacket *srp2 = new SMSSendRequestPacket();
         if( bConcatenatedSMS ){
            srp2->fillPacket( CONVERSION_NO,
                              senderPhone,
                              receiverPhone,
                              dataLength,
                              data,
                              SMSSendRequestPacket::SMSTYPE_TEXT,
                              tempSmsNbr,
                              totalNbrOfSMSes,
                              partNbr++);
         }
         else{
             srp2->fillPacket( CONVERSION_NO,
                              senderPhone,
                              receiverPhone,
                              dataLength,
                              data,
                              SMSSendRequestPacket::SMSTYPE_TEXT,
                              -1,
                              -1,
                              -1);
         }
         retRequest->addNewSMS(srp2);
         delete [] data;
         
         delete pc; pc = tmpRequest->getNextPacket();
      }
      delete [] totNbrString;
      delete tmpRequest;
   }

   // cleanup
   for (uint32 i = 0; i < nbrItems; i++) {
      delete strItemVector[i];
   }
   delete [] strItemVector;
   
   delete [] curData;
   delete [] extraData;
   return (retRequest);
}

char* 
SMSFormatter::createNewPacket( int32 &smsNbr, uint32 &totalNbrOfSMSes,
                               bool lastSMS,
                               bool sendThisOne,
                               bool bConcatenatedSMS,
                               const char* const senderPhone,
                               const char* const receiverPhone, 
                               char* data, SMSSendRequest* theRequest, 
                               SMSUserSettings *format)
{
   DEBUG4( cerr << "SMSFormatter::createNewPacket" << endl );
   const uint32 phoneWidth = format->getSMSLineLength();
   UserConstants::EOLType eol = format->getEOLType();
   StringTable::languageCode language = format->getSMSLanguage();

   DEBUG2(cerr << "=======================" << endl);
   DEBUG2(cerr << data << endl);
   DEBUG2(cerr << "=======================" << endl);
   
   if (  (theRequest != NULL) && 
         (senderPhone != NULL) && 
         (receiverPhone != NULL) &&
         (sendThisOne) )
   {
      // Set the footer of this packet!
      if (!lastSMS) {
         DEBUG8(cerr << "Not last packet, printing (cont)" << endl;);
         // Print the "(cont)" string to the packet
         if(!bConcatenatedSMS ){
            char* dp = data + strlen(data);
            printSMSString(dp,
                           StringTable::getString(
                              StringTable::ROUTE_CONTINUES,
                              language),
                           language,
                           eol, phoneWidth, 0, 1, true);
         }
      } else {
         DEBUG8(cerr << "Last packet." << endl;);  
      }
      smsNbr++;

      // Create the new SMS packet
      SMSSendRequestPacket* curPack = new SMSSendRequestPacket();
      curPack->fillPacket(CONVERSION_TEXT, senderPhone, 
                          receiverPhone, strlen(data), 
                          (byte*) data);
      theRequest->addNewSMS(curPack);
      totalNbrOfSMSes++;
   } else {
      if (sendThisOne) {
         DEBUG1( cerr << "Error: "
                 << "SMSFormatter::createNewPacket something was NULL,"
                 << " giving up, and not sending any SMS." << endl );
      }
      smsNbr = 1;
   }

   // Header of the new message
   if(!bConcatenatedSMS){
      char* headerStr = new char[16];
      sprintf(headerStr, "%s%d/xx",
              StringTable::getString(
                 StringTable::ROUTE,
                 language),
              smsNbr/*, totalNbrOfSMSes*/);
      uint32 nbrBytesWritten = 
         printSMSString(data, headerStr,
                        language, eol, phoneWidth, 0, 1, true);
      data[nbrBytesWritten] = '\0';
      delete [] headerStr;
   }
   else
      data[0] = '\0';
   
   return (data);
}


uint32
SMSFormatter::writeSMSChoice( uint32 i,
                              SearchMatch *vm,
                              const char* locationName,
                              char* d,
                              SMSUserSettings *smsUserFormat)
{
   int phoneWidth = smsUserFormat->getSMSLineLength();
   UserConstants::EOLType eol = smsUserFormat->getEOLType();
   StringTable::languageCode language = smsUserFormat->getSMSLanguage();
   const char* matchLocationName = NULL;
   bool addLocationName = false;

   uint32 totL, lineL;
   char* data = d;
   // print the Choice index
   totL = lineL = sprintf(data, "%d ", i);
   data += totL;

   const char *theName = NULL;
   if (dynamic_cast<VanillaMatch *>( vm ) != NULL)
   {
      theName = static_cast<VanillaMatch *>( vm )->getName();
      matchLocationName = static_cast<VanillaMatch *>( vm )
         ->getLocationName();
   } else if (dynamic_cast<OverviewMatch *>( vm ) != NULL)
   {
      theName = static_cast<OverviewMatch *>( vm )->getName0( );
      matchLocationName = "";
   } else {
      DEBUG1( cerr << "Error: unexpected object in writeSMSChoice"
              << endl );
   }

   StringTable::languageCode lang = smsUserFormat->getSMSLanguage();
   if ( theName == NULL ) {
      theName = StringTable::getString(
         StringTable::UNKNOWN, lang );
   }

   char formattedName[1024];
   if (dynamic_cast<VanillaCategoryMatch*>(vm) != NULL) {
      // add "..." at end of category names.
      theName = static_cast<VanillaMatch*>(vm)->getName();
      sprintf(formattedName, "%s...", theName);
   } else {
      sprintf(formattedName, "%s", theName);

      // add name of location if match-location is different
      // from requested location, and object is not a category.
      addLocationName = (StringUtility::strcasecmp(
         matchLocationName, locationName ) != 0);
   }
   
   lineL = printSMSString( data, formattedName, language, eol, phoneWidth,
                           totL, maxNbrChoiceRows, !addLocationName );
   totL += lineL;
   data += lineL;

   if ( addLocationName )
   {
      lineL = printSMSString( data, ", ", language, eol, phoneWidth,
                              totL, maxNbrChoiceRows, false );
      totL += lineL;
      data += lineL;
      lineL = printSMSString( data, matchLocationName,
                              language, eol, phoneWidth,
                              totL, maxNbrChoiceRows, true );
      totL += lineL;
      data += lineL;
   }

   return (totL);
}


uint32
SMSFormatter::calcSMSChoiceLength( uint32 i,
                                   SearchMatch *vm,
                                   SMSUserSettings *smsUserFormat,
                                   const char* locationName)
{
   // Make sure (?) not to seg.fault ...
   if (vm == NULL) {
      return (0);
   }

   char *temp = new char[512];
   temp[ 0 ] = 0;

   uint32 result = writeSMSChoice( i, vm, locationName,
                                   temp, smsUserFormat);
   if (result != strlen(temp)) {
      DEBUG1( cerr
         << "Error?: result != strlen(temp) in calcSMSChoiceLength: "
         << result << " != " << strlen(temp) << endl );
   }
   delete [] temp;
   return result;
}

char *
SMSFormatter::getNewMatchStringLocation( VanillaMatch *match )
{
   DEBUG4( cerr << "SMSFormatter::getNewMatchStringLocation( "
           << "VanillaMatch="
           << match << " )" << endl );
   char *result = StringUtility::newStrDup("<>");
   if (match != NULL) {
      delete [] result;
      char tmpStr[512];
      tmpStr[ 0 ] = 0;
      snprintf(tmpStr, 512, "%s", match->getLocationName());
      result = StringUtility::newStrDup( tmpStr );
   }
   return result;
}

char *
SMSFormatter::getNewMatchStringWithLocation( VanillaMatch *match )
{
   DEBUG4( cerr << "SMSFormatter::getNewMatchStringWithLocation( "
           << "VanillaMatch="
           << match << " )" << endl );
   char *result = StringUtility::newStrDup("<>");
   if (match != NULL) {
      delete [] result;
      char tmpStr[512];
      tmpStr[ 0 ] = 0;
      VanillaStreetMatch *vsm =
         dynamic_cast<VanillaStreetMatch *>( match );
      VanillaCompanyMatch *vcm = NULL;
      if (vsm == NULL) {
         vcm = dynamic_cast<VanillaCompanyMatch *>( match );
      }
      if ((vsm != NULL) &&
          (vsm->getStreetNbr() != 0))
      {
         snprintf( tmpStr, 512, "%s:%s %u",
                   match->getLocationName(),
                   match->getName(),
                   vsm->getStreetNbr());
      } else {
         snprintf( tmpStr, 512, "%s:%s",
                   match->getLocationName(),
                   match->getName() );
      }
      result = StringUtility::newStrDup( tmpStr );
   }
   return result;
}

char *
SMSFormatter::getNewMatchString( VanillaMatch *match )
{
   DEBUG4( cerr << "SMSFormatter::getNewMatchString( VanillaMatch="
           << match << " )" << endl );
   char *result = StringUtility::newStrDup( "<>" );
   if (match != NULL) {
      delete [] result;
      char tmpStr[256];
      tmpStr[ 0 ] = 0;
      VanillaStreetMatch *vsm =
         dynamic_cast<VanillaStreetMatch *>( match );
      VanillaCompanyMatch *vcm = NULL;
      if (vsm == NULL) {
         vcm = dynamic_cast<VanillaCompanyMatch *>( match );
      }
      if ((vsm != NULL) &&
          (vsm->getStreetNbr() != 0))
      {
         snprintf( tmpStr, 256, "%s %u", vsm->getName( /*0*/ ),
                  vsm->getStreetNbr());
/*      } else if ((vcm != NULL) &&
                 (vcm->getStreetNbr() != 0))
      {
         sprintf( tmpStr, "%s %u", vcm->getName( 0 ),
         vcm->getStreetNbr());*/
      } else {
         snprintf( tmpStr, 256, "%s", match->getName( /*0*/ ) );
      }
      result = StringUtility::newStrDup( tmpStr );
   }
   return result;
}

char *
SMSFormatter::getNewMatchString( OverviewMatch *location )
{
   DEBUG4( cerr << "SMSFormatter::getNewMatchString( OverviewMatch="
           << location << " )" << endl );
   char *result = StringUtility::newStrDup( "" );
   if (location != NULL) {
      delete [] result;
      char tmpStr[256];
      tmpStr[ 0 ] = 0;
      OverviewMatch *om =
         dynamic_cast<OverviewMatch *>( location );
      if ( om != NULL ) {
         snprintf( tmpStr, 256, "%s", om->getName0() );
      }
      result = StringUtility::newStrDup( tmpStr );
   }
   return result;
}

char *
SMSFormatter::getNewMatchString( OverviewMatch *location,
                                 VanillaMatch *match )
{
   DEBUG4( cerr << "SMSFormatter::getNewMatchString( OverviewMatch="
           << location << ", VanillaMatch=" << match << " )" << endl );
   char *locationStr = getNewMatchString( location );
   char *matchStr    = getNewMatchString( match    );
   char *result = new char[ strlen(locationStr) + strlen(matchStr) + 1 +1 ];
   sprintf( result, "%s:%s", locationStr, matchStr );
   delete [] locationStr;
   delete [] matchStr;
   return result;
}

SMSSendRequest*
SMSFormatter::getReply(StringTable::stringCode code,
                       VanillaMatch *origin, VanillaMatch *destination,
                       char* senderPhone,
                       char* receiverPhone,
                       SMSUserSettings *smsUserFormat,
                       uint16 requestID) 
{
   // Send a reason for not being able to make a route,
   // or some other error from a module.
   char* data = new char[ MAX_SMS_SIZE+1 ];
   memset( data, 0, MAX_SMS_SIZE + 1 );

   char *tmpStr = new char[ 256 ];
   tmpStr[ 0 ] = 0;

   StringTable::languageCode language = smsUserFormat->getSMSLanguage();
   
   // write "From: " + originname (max 2 lines)
   //"From: "
   const char *from = StringTable::getString( StringTable::ORIGIN,
                                              language );

   uint32 len = strlen( from );
   strcpy( tmpStr, from );
   strcat( tmpStr, ": " );
   len += 2;
   
   char *nameStr = getNewMatchString( origin );
   printSMSString( tmpStr+len, nameStr,
                   language,
                   smsUserFormat->getEOLType(),
                   smsUserFormat->getSMSLineLength(),
                   len, 2, true );
   delete [] nameStr;
   strncpy( data, tmpStr, MAX_SMS_SIZE + 1 );

   // write "To: " + destname (max 2 lines)
   // "To: "
   const char *to = StringTable::getString( StringTable::DESTINATION,
                                            language );

   len = strlen( to );
   strcpy( tmpStr, to );
   strcat( tmpStr, ": " );
   len += 2;

   nameStr = getNewMatchString( destination );
   printSMSString( tmpStr+len, nameStr,
                   language,
                   smsUserFormat->getEOLType(),
                   smsUserFormat->getSMSLineLength(),
                   len, 2, true );
   delete [] nameStr;
   strcpy( data + strlen(data), tmpStr );
   
   // write the error from the route module
   sprintf(data +strlen(data), "%s",
           StringTable::getString(code, language));

   // Send the SMS message
   SMSSendRequestPacket* p = new SMSSendRequestPacket();
   p->fillPacket(CONVERSION_TEXT, senderPhone, receiverPhone, 
                 strlen(data)+1, (byte*) data);

   SMSSendRequest* retReq = new SMSSendRequest(requestID, p);
   delete [] data;
   delete [] tmpStr;
   return (retReq);
}

uint32
SMSFormatter::printSMSString(char* const dest, const char* const src,
                             StringTable::languageCode language,
                             UserConstants::EOLType eol, uint32 phoneWidth,
                             uint32 destLinePos,
                             uint32 maxNbrLines, bool newLineAfterString) 
{
   return SMSStringUtility::printSMSString( dest, src, language, eol, 
                                            phoneWidth, destLinePos, 
                                            maxNbrLines, newLineAfterString );
}

uint32
SMSFormatter::writeEol(char* dest,
                       uint32 pos,
                       UserConstants::EOLType eol,
                       uint32 leftOnLine) 
{
   uint32 nbrCharsWritten = 0; // Keep the compiler happy (-O2)
   switch (eol) {
      case (UserConstants::EOLTYPE_CR) :
      case (UserConstants::EOLTYPE_AlwaysCR) :
         dest[pos] = 13;
         nbrCharsWritten = 1;
         break;
      case (UserConstants::EOLTYPE_LF) :
      case (UserConstants::EOLTYPE_AlwaysLF) :
         dest[pos] = 10;
         nbrCharsWritten = 1;
         break;
      case (UserConstants::EOLTYPE_CRLF) :
         if (leftOnLine < 2) {
            // Write one space
            dest[pos] = 32;
            nbrCharsWritten = 1;
         } else {
            dest[pos] = 13;
            dest[pos+1] = 10;
            nbrCharsWritten = 2;
         }
         break;
      case (UserConstants::EOLTYPE_NBR) :
         DEBUG1( cerr
                 << "Error: Incorrect EOLTYPE. Reverting to default."
                 << endl );
      case (UserConstants::EOLTYPE_AlwaysCRLF) :
      case (UserConstants::EOLTYPE_NOT_DEFINED) :
         dest[pos] = 13;
         dest[pos+1] = 10;
         nbrCharsWritten = 2;
         break;
   } // switch
   return (nbrCharsWritten);
}


SMSSendRequest* 
SMSFormatter::makeTextSMSRequest( const UserCellular* cellular,
                                  uint32 requestID,
                                  const char* userPhone,
                                  const char* serverPhone,
                                  uint32 nbrStrings,
                                  bool lineBreak,
                                  ... )
{
   const char* inStr = NULL;
   char line[ MAX_SMS_SIZE + 1 ];
   byte sms[ MAX_SMS_SIZE + 1 ];
   uint32 size = 0;
   uint32 lineLength = 0;

   va_list ap;
   va_start( ap, lineBreak );
   
   for ( uint i = 0 ; i < nbrStrings; i++) {
      inStr = va_arg( ap, const char* );
      lineLength = printSMSString( 
         line, inStr, 
         StringTable::ENGLISH, // We have src and dest -> unused
         cellular->getCellularEOLType(),
         cellular->getCellularSMSLineLength(),
         0, MAX_UINT32, lineBreak );
      if ( size + lineLength > MAX_SMS_SIZE ) {
         break;
      }
      strcpy( reinterpret_cast< char* >( sms + size ), line );
      size += lineLength;
   }
   va_end(ap);

   // Make SMSSendRequest
   SMSSendRequestPacket* p = new SMSSendRequestPacket();
   
   p->fillPacket( CONVERSION_TEXT,
                  serverPhone,
                  userPhone,
                  size,
                  sms );
   
   SMSSendRequest* req = new SMSSendRequest( requestID, p );
   
   return req;
}


SMSSendRequest* 
SMSFormatter::makeTextSMSRequest( const CellularPhoneModel* cellular,
                                  uint32 requestID,
                                  const char* userPhone,
                                  const char* serverPhone,
                                  uint32 nbrStrings,
                                  bool lineBreak,
                                  ... )
{
   const char* inStr = NULL;
   char line[ MAX_SMS_SIZE + 1 ];
   byte sms[ MAX_SMS_SIZE + 1 ];
   uint32 size = 0;
   uint32 lineLength = 0;

   va_list ap;
   va_start( ap, lineBreak );
   
   for ( uint i = 0 ; i < nbrStrings; i++) {
      inStr = va_arg( ap, const char* );
      lineLength = printSMSString( 
         line, inStr, 
         StringTable::ENGLISH, // We have src and dest -> unused
         cellular->getEOL(),
         cellular->getChars(),
         0, MAX_UINT32, lineBreak );
      if ( size + lineLength > MAX_SMS_SIZE ) {
         break;
      }
      strcpy( reinterpret_cast< char* >( sms + size ), line );
      size += lineLength;
   }
   va_end(ap);

   // Make SMSSendRequest
   SMSSendRequestPacket* p = new SMSSendRequestPacket();
   
   p->fillPacket( CONVERSION_TEXT,
                  serverPhone,
                  userPhone,
                  size,
                  sms );
   
   SMSSendRequest* req = new SMSSendRequest( requestID, p );
   
   return req;
}


void 
SMSFormatter::setSMSUserSettingsFromCellularPhoneModel( 
   SMSUserSettings* smsUserFormat, CellularPhoneModel* model,
   StringTable::languageCode lang )
{
   smsUserFormat->setSMSLineLength( model->getChars() );
   smsUserFormat->setEOLType( model->getEOL() );
   smsUserFormat->setLanguage( lang );
}


void
SMSFormatter::newSMSString( char *&dest,
                            uint32 &strLen,
                            const char *src,
                            SMSUserSettings *smsUserFormat )
{
   StringTable::languageCode language = smsUserFormat->getSMSLanguage();
   dest = new char[ strlen(src)+1 +
                  smsUserFormat->getEOLLength()*3 ];
   printSMSString( dest, src,
                   language,
                   smsUserFormat->getEOLType(),
                   smsUserFormat->getSMSLineLength(), 0, 3, true );
   strLen = strlen( dest );
//   DEBUG4( cerr << "newSMSString: " << dest << endl );
}



// SMSUserSettings ////////////////

SMSUserSettings::SMSUserSettings()
{
   // Default params
   m_SMSLineLength = 10;
   m_SMSEOLType = UserConstants::EOLTYPE_AlwaysCRLF;

   m_SMSEOLLength = calcEOLLength();
   setLanguage(StringTable::SMSISH_ENG);
}

uint32
SMSUserSettings::calcEOLLength()
{
   // Calculate the length of the eol.
   char *tmp = new char[64];
   uint32 result = SMSFormatter::writeEol
      ( tmp, 0, m_SMSEOLType, m_SMSLineLength );
   delete [] tmp; // don't care about what the string is, just get the length.
   return result;
}

#undef DEBUG1
#undef DEBUG2
#undef DEBUG4
//#undef DEBUG8
#define DEBUG1(a) oldD1(a)
#define DEBUG2(a) oldD2(a)
#define DEBUG4(a) oldD4(a)
//#define DEBUG8(a) oldD8(a)
