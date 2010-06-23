/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MODEMPARSER_4711_H
#define MODEMPARSER_4711_H

/**
 *    ModemParserResult.
 *
 */
class ModemParserResult {
   public:
      /**
       */
      enum result_t {
         OK = 0, ERROR, RING, NO_CARRIER, NO_DIALTONE, NO_ANSWER,
         CMTI, CMT, CMS_ERROR, NO_MATCH, CMGS, CMGR, PROMPT, CNMI,
         PDU
      };

      /**
       *    @param   resultCode  
       *    @param   firstParam  
       *    @param   secondParam 
       *    @param   stringParam 
       */
      ModemParserResult(result_t resultCode,
                        int firstParam,
                        int secondParam,
                        const char* stringParam = "");

      /**
       *    @param mpr
       */
      ModemParserResult(const ModemParserResult& mpr);

      /**
       */
      virtual ~ModemParserResult();

      /**
       */
      result_t m_resultCode;

      /**
       */
      int      m_firstParam;

      /**
       */
      int      m_secondParam;

      /**
       */
      char*    m_stringParam;
      
      /**
       */
      struct resultmap_t {      
         const char* resultString;
         result_t code;
      };

      /**
       */
      static const resultmap_t resultMap[];     
};


/**
 *    ModemParser.
 *
 */
class ModemParser {
   public:
      /**
       */
      ModemParser();

      /**
       */
      virtual ~ModemParser();

      /**
       *    @param   result  
       */
      ModemParserResult parse(char* result);
   
};

#endif

