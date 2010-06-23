/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ModemParser.h"

#include "Utility.h"
#include "MC2String.h"

#include <stdlib.h>

const ModemParserResult::resultmap_t
ModemParserResult::resultMap[] = {
   { "OK",           OK },
   { "ERROR" ,       ERROR },
   { "RING",         RING },
   { "NO CARRIER",   NO_CARRIER },
   { "NO DIALTONE" , NO_DIALTONE },
   { "NO ANSWER",    NO_ANSWER },
   { "+CMS ERROR",   CMS_ERROR }, // NB! +CMS ERROR contains ERROR and
                                // must be placed after ERROR
   { "+CMT",  CMT },
   { "+CMTI", CMTI },
   { "+CMGR", CMGR },
   { "+CMGS", CMGS },
   { "+CNMI", CNMI },
   { ">",     PROMPT },
   { "",      NO_MATCH } // Vaktpost
};

ModemParserResult::ModemParserResult(result_t resultCode,
                                     int firstParam,
                                     int secondParam,
                                     const char* stringParam)
{
   m_resultCode  = resultCode;
   m_firstParam  = firstParam;
   m_secondParam = secondParam;
   m_stringParam = new char[strlen(stringParam) + 1];
   strcpy(m_stringParam, stringParam);
}


ModemParserResult::ModemParserResult(const ModemParserResult& mpr)
{
   m_resultCode  = mpr.m_resultCode;
   m_firstParam  = mpr.m_firstParam;
   m_secondParam = mpr.m_secondParam;
   m_stringParam = new char[strlen(mpr.m_stringParam) + 1];
   strcpy(m_stringParam, mpr.m_stringParam);   
}


ModemParserResult::~ModemParserResult()
{
   delete [] m_stringParam;
}
ModemParser::ModemParser()
{

}


ModemParser::~ModemParser()
{

}


ModemParserResult
ModemParser::parse(char* result)
{
   ModemParserResult::result_t resultCode = ModemParserResult::NO_MATCH;
   const char* resultString;
   int firstParameter  = -1;
   int secondParameter = -1;
   const char* stringParameter = "";
   int i = 0;
   cout << "ModemParser RESULT : " << result << endl;
   do {
      if ( strstr ( result, ModemParserResult::resultMap[i].resultString )
           != NULL ) {
         cout << "Found : " << ModemParserResult::resultMap[i].resultString
              << endl;
         resultCode = ModemParserResult::resultMap[i].code;
         resultString = ModemParserResult::resultMap[i].resultString;
      }
   } while ( ModemParserResult::resultMap[++i].code !=
             ModemParserResult::NO_MATCH );

   const int tmpSize = 256;
   char* tmpS = new char[tmpSize];
   char* pP = NULL;
   switch ( resultCode ) {
      case ModemParserResult::CMTI:
         // +CMTI: <mem>,<index>
         pP = Utility::getString( (const char*)result, ':' , ',',
                             tmpS, tmpSize);
         if ( pP != NULL) {
            firstParameter = atoi(tmpS);
            secondParameter = atoi(pP);
         }
         break;
      case ModemParserResult::CMT:
         // +CMT: [<alpha>], <length><CR><LF><pdu>
         // Move pP past ""ME""
         pP = Utility::getString( (const char*)result, '"', '"',
                                  tmpS, tmpSize);
         if ( pP != NULL ) {
            // Read the first parameter
            pP = Utility::getString( pP, ',', ',',
                                     tmpS, tmpSize);
            if ( pP != NULL ) 
               firstParameter = atoi(tmpS);
         }
         break;
      case ModemParserResult::CMGR:
         // +CMGR: <stat>,["<alpha>"],<length><CR><LF><pdu>
         // Read the first parameter
         pP = Utility::getString( result, ':', ',', tmpS, tmpSize);
         if ( pP != NULL ) {
            char* tempP;
            firstParameter = atoi(tmpS);
            // Throw away <alpha> The commas should be there, at least.
            tempP = Utility::getString( pP, '\0', ',', tmpS, tmpSize);
            if ( tempP == NULL)
               tempP = pP;
            pP = Utility::getString ( tempP, '\0', '\0', tmpS, tmpSize);
            if ( pP != NULL )
               secondParameter = atoi(tmpS);
         }
         break;
      case ModemParserResult::CMGS:
         // +CMGS: <message reference>
         // Get the number after +CMGS:
         pP = Utility::getString( result, ':', '\0', tmpS, tmpSize);
         if ( pP != NULL ) 
            firstParameter = atoi(tmpS);
         break;
      case ModemParserResult::NO_MATCH:
         // Check if it is a PDU
         // XXX: This is a very simple test. Maybe it could be better.
         // Check if the two first letters are "04" or "11"
         if((strncmp(result, "04", 2) == 0) ||(strncmp(result, "11" ,2) == 0)){
            resultCode = ModemParserResult::PDU;
            stringParameter = result;
            DEBUG2( cerr << "Found PDU" << endl);
         }
         break;
      default:
         // No parameters for the other responses
         break;
   }
   delete [] tmpS;
   return ModemParserResult(resultCode, firstParameter, secondParameter,
                            stringParameter);
}
