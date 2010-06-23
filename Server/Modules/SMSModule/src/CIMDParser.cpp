/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CIMDParser.h"
#include "CIMDUtil.h"
#include <stdlib.h>
#include "Utility.h"
#include "MC2String.h"
#include "SMSConvUtil.h"

#define DESTINATION_ADDRESS "021:"
#define ORIGINATOR_ADDRESS  "023:"
#define USER_DATA           "033:"
#define USER_BINARY         "034:"
#define USER_HEADER         "032:"

const CIMDParserResult::resultmap_t
CIMDParserResult::resultMap[] = {
   { "51",    RESP_LOGIN},
   { "53",    RESP_SUBMIT},
   { "55",    RESP_DELIVERY},
   { "20",    MESSAGE_DELIVERY},
   { "90",    RESP_ALIVE_ACK},
   { "99",    RESP_NACK},
   { "900",   RESP_ERROR},
   { "901",   RESP_ERROR_TXT},
   { "",      NO_MATCH } // Vaktpost
};

CIMDParserResult::CIMDParserResult(result_t resultCode,
                                   int packetNumber,
                                   int firstParam,
                                   int secondParam,
                                   int errorParam,
                                   int smsNumber,
                                   int maxParts,
                                   int partNumber,
                                   char* stringParam,
                                   char* recipientPhone,
                                   char* senderPhone)
{
   fillinparameters(resultCode,
                    packetNumber,
                    firstParam,
                    secondParam,
                    errorParam,
                    smsNumber,
                    maxParts,
                    partNumber,
                    stringParam,
                    recipientPhone,
                    senderPhone);
}


CIMDParserResult::CIMDParserResult(const CIMDParserResult& mpr)
{
   fillinparameters(mpr.m_resultCode,
                    mpr.m_packetNumber,
                    mpr.m_firstParam,
                    mpr.m_secondParam,
                    mpr.m_error,
                    mpr.m_smsNumber,
                    mpr.m_maxParts,
                    mpr.m_partNumber,
                    mpr.m_stringParam,
                    mpr.m_recipientPhone,
                    mpr.m_senderPhone);
}


inline void
CIMDParserResult::fillinparameters(result_t resultCode,
                                   int packetNumber,
                                   int firstParam,
                                   int secondParam,
                                   int errorParam,
                                   int smsNumber,
                                   int maxParts,
                                   int partNumber,
                                   char* stringParam,
                                   char* recipientPhone,
                                   char* senderPhone)
{
   m_resultCode   = resultCode;
   m_packetNumber = packetNumber;
   m_firstParam  = firstParam;
   m_secondParam = secondParam;
   m_error       = errorParam;
   m_smsNumber   = smsNumber;
   m_maxParts    = maxParts;
   m_partNumber  = partNumber;
   m_stringParam = new char[strlen(stringParam) + 1];
   strcpy(m_stringParam, stringParam);
   m_recipientPhone = new char[strlen(recipientPhone) + 1];
   strcpy(m_recipientPhone, recipientPhone);
   m_senderPhone = new char[strlen(senderPhone) + 1 ];
   strcpy(m_senderPhone, senderPhone);
}

CIMDParserResult::~CIMDParserResult()
{
   delete m_stringParam;
   delete m_recipientPhone;
   delete m_senderPhone;
}


CIMDParser::CIMDParser()
{
   // Find the error code in the map
   int i = 0;
   const char* errorpointer = NULL;
   do {
      if ( CIMDParserResult::resultMap[i].code ==
           CIMDParserResult::RESP_ERROR )
         errorpointer = CIMDParserResult::resultMap[i].resultString;
   } while ( CIMDParserResult::resultMap[++i].code !=
                CIMDParserResult::NO_MATCH );

   if ( errorpointer != NULL ) {
      m_errorString = new char[strlen(errorpointer) + 2];
      strcpy(m_errorString, errorpointer);
      strcat(m_errorString, ":");
   }
}


CIMDParser::~CIMDParser()
{
   delete [] m_errorString;
}


CIMDParserResult
CIMDParser::parse(char* result)
{
   CIMDParserResult::result_t resultCode = CIMDParserResult::NO_MATCH;
   const char* resultString;
   int firstParameter  = -1;
   int secondParameter = -1;
   int pktNumber       = -1;
   int smsNumber       = -1;
   int maxParts        = -1;
   int partNumber      = -1;
   int errorParam      = 0;
   char stringParameter[512];
   char recipientPhone[128];
   char senderPhone[128];
   bool hasHeader = false;
   
   strcpy(stringParameter, "");
   strcpy(recipientPhone, "");
   strcpy(senderPhone, "");
   
   int i = 0;
   mc2dbg2 << "[CIMDParser] result: " << result << endl;
#ifdef DEBUG_LEVEL_4
   mc2dbg4 << "CIMDParser, parse(), result hexdump:" << endl;
   HEXDUMP(mc2dbg2, (byte*)result, strlen(result), "\t       ");
#endif

   int tmpSize = 512;
   char* tmpS  = new char[tmpSize];
   char* pP = NULL; // pointer to next token   
   if ( result[0] == STX ) {
      do {
         if ((pP = Utility::getString(&result[1],'\0',':', tmpS, tmpSize))
             != NULL){
            if (strcmp(tmpS, CIMDParserResult::resultMap[i].resultString) 
              == 0 ) {
            mc2dbg2 << "[CIMDParser] Found : " << CIMDParserResult::resultMap[i].resultString
                 << endl;
            resultCode = CIMDParserResult::resultMap[i].code;
            resultString = CIMDParserResult::resultMap[i].resultString;
            }
         }
      } while ( CIMDParserResult::resultMap[++i].code !=
                CIMDParserResult::NO_MATCH );

      if ( m_errorString != NULL ) {
         char* position;
         if ( (position = strstr(result, m_errorString)) != NULL ) {
            // We have an error
            char* errorpP = NULL;
            errorpP = Utility::getString(position, ':', TAB, tmpS, tmpSize);
            if ( errorpP != NULL ) {
               errorParam = atoi(tmpS);
               mc2log << warn << "Error code is " << errorParam << endl;
            }
         }
      }
         
      switch ( resultCode ) {
         case CIMDParserResult::RESP_LOGIN:
         case CIMDParserResult::RESP_SUBMIT:
         case CIMDParserResult::RESP_NACK:
         case CIMDParserResult::RESP_ALIVE_ACK:
            if ( pP != NULL ) {
               // Get the packet number.
               pP = Utility::getString(pP, '\0', TAB, tmpS, tmpSize);
               if ( pP != NULL )
                  pktNumber = atoi(tmpS);
            }
            break;
         case CIMDParserResult::MESSAGE_DELIVERY:
            if ( pP != NULL ) {
               // Get the packet number.
               pP = Utility::getString(pP, '\0', TAB, tmpS, tmpSize);
               if ( pP != NULL )
                  pktNumber = atoi(tmpS);
               // Find the recipientPhone
               pP = strstr(result, DESTINATION_ADDRESS);
               if ( pP != NULL ) {
                  pP = Utility::getString(pP, ':', TAB, tmpS, tmpSize);
                  if ( pP != NULL )
                     strcpy(recipientPhone, tmpS);
               }
               // Find the senderPhone
               pP = strstr(result, ORIGINATOR_ADDRESS);
               if ( pP != NULL ) {
                  pP = Utility::getString(pP, ':', TAB, tmpS, tmpSize);
                  if ( pP != NULL ) {
                     strcpy(senderPhone, tmpS);                     
                  }
               }
               // Check the header
               if ( ( pP = strstr(result, USER_HEADER) ) != NULL ) {
                  // This is a user header. Probably a multimessage
                  pP = Utility::getString(pP, ':', TAB, tmpS, tmpSize);
                  if ( pP != NULL ) {
                     mc2dbg2 << "[CIMDParser] header found" << endl;
                     hasHeader = true;
                     // We only understand concatenated messages
                     int strLen = strlen(tmpS);
                     unsigned int userHeader[strLen];
                     char hexData[3];
                     // Convert the message to a vector of integers
                     for(int i=0; i < strLen; i+=2) {
                        hexData[0] = tmpS[i];
                        hexData[1] = tmpS[i+1];
                        hexData[2] = '\0';
                        sscanf(hexData, "%x", &userHeader[i/2]);               
                     }
                     unsigned int pos = 0;
                     const unsigned int byteLen = strLen / 2;
                     bool decodingError = false;
                     while ( pos < byteLen && decodingError == false ) {
                        if ( pos == 0 ) { // Headerlength
                           if ( userHeader[pos++] != byteLen - 1) {
                              mc2log << warn << "[CIMDParser] Headerlength wrong"
                                     << endl;
                              decodingError = true;
                           }
                        } else { // pos > 0
                           // Find out which command it is
                           unsigned int command = userHeader[pos++];
                           unsigned int length = 0; // Used below
                           switch ( command ) {
                              case 0: // Concatenated message
                                 if ( userHeader[pos++] != 3 ) {
                                    mc2dbg << "[CIMDParser] conlength != 3"
                                           << endl;
                                    decodingError = false;
                                 } else {                                    
                                    smsNumber  = userHeader[pos];
                                    maxParts   = userHeader[pos+1];
                                    partNumber = userHeader[pos+2];
                                    mc2dbg << "[CIMDParser] Conc - ID " 
                                           << smsNumber << " " << partNumber
                                           << "/" << maxParts << endl;
                                    pos +=3;
                                 }                                    
                                 break;
                              case 5: // Port command (Nokia ??)
                                 length = userHeader[pos++];
                                 if ( length != 4 ) {
                                    mc2dbg << "[CIMDParser] portlength != 4" 
                                           << endl;
                                 } else {
                                    DEBUG1(
                                    uint32 destPort = userHeader[pos] << 8|
                                       userHeader[pos+1];
                                    uint32 sourcePort = userHeader[pos+2] << 8|
                                        userHeader[pos+3];
                                    pos += 4;
                                    mc2dbg << "[CIMDParser] found dest "
                                           "port " << hex << destPort <<
                                           " and source port " << sourcePort
                                           << endl;)
                                 }
                                 break;
                              default: // Skip unknown information
                                 length = userHeader[pos++];
                                 pos += length;
                                 mc2log << warn << "[CIMDParser] unknown IE: "
                                        << command << endl;
                                 break;                                 
                           }
                        }
                        
                     }
                  }
               }
               // Find the textmessage
               pP = strstr(result, USER_DATA);
               if ( pP != NULL ) {
                  // Data is text.
                  pP = Utility::getString(pP, ':', TAB, tmpS, tmpSize);
                  if ( pP != NULL ) {
                     mc2dbg << "[CIMDParser] found USER_DATA" << endl;
                     firstParameter =
                        CIMDUtil::convertFromSpecialCombi(stringParameter,
                                                          tmpS,
                                                          strlen(tmpS));
                  }
               } else if ( ( pP = strstr(result, USER_BINARY) ) != NULL ) {
                  // Data is hex.
                  pP = Utility::getString(pP, ':', TAB, tmpS, tmpSize);
                  if ( pP != NULL ) {
                     mc2dbg << "[CIMDParser] found USER_BINARY" << endl;
                     // Convert the data from hex to bytes
                     if ( hasHeader ) {
                        firstParameter = CIMDUtil::convertFromHex(
                           stringParameter,
                           tmpS,
                           strlen(tmpS));
                     } else {
                        mc2dbg << "[CIMDParser] no header, assuming"
                               " 16-bit data" << endl;
                        uint16 wordData[1024];
                        // Convert the data from 16-bit hex to integers
                        firstParameter = CIMDUtil::convertFromWordHex(
                           wordData,
                           tmpS,
                           strlen(tmpS));
                        // Convert the integers to characters.
                        firstParameter = CIMDUtil::convertFromWords(
                           stringParameter,
                           wordData,
                           firstParameter);
                        // Convert the iso characters to GSM
                        char tmpStr[1024];
                        memcpy( tmpStr, stringParameter, firstParameter+1);
                        firstParameter = SMSConvUtil::isoToGsm(stringParameter,
                                                               tmpStr,
                                                               firstParameter);
                           
                     }
                  } else {
                     mc2log << error << "[CIMDParser] Unsupported encoding... "
                            << "Tell pi to fix it!" << endl;
                     DEBUG1(
                        mc2dbg << "[CIMDParser] Dump of server message follows:"
                               << endl;
                        HEXDUMP(mc2dbg, (byte*)result, strlen(result), "\t       ");
                     );
                     firstParameter = 0;
                  }
               }               
            }
            break;
         default:
            break;
      }
   }
   delete [] tmpS;
   return CIMDParserResult(resultCode, pktNumber,
                           firstParameter, secondParameter, errorParam,
                           smsNumber, maxParts, partNumber,
                           stringParameter, recipientPhone, senderPhone);
}
