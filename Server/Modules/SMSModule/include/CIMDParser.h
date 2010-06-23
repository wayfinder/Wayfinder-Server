/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CIMDPARSER_4711_H
#define CIMDPARSER_4711_H

/**
 *    CIMDParserResult.
 *
 */
class CIMDParserResult {
   public:
      /**
       */
      enum result_t {
         RESP_LOGIN,
         RESP_SUBMIT,
         RESP_DELIVERY,
         MESSAGE_DELIVERY,
         RESP_NACK,
         RESP_ALIVE_ACK,
         RESP_ERROR = 900, // Special case: RESP_ERROR can be embedded in 
                           // another type of message.
         RESP_ERROR_TXT = 901,
         NO_MATCH 
      };

      /**
       */
      CIMDParserResult(result_t resultCode,
                       int packetNumber,
                       int firstParam,
                       int secondParam,
                       int errorParam,
                       int smsNumber,
                       int maxParts,
                       int partNumber,
                       char* stringParam = "",
                       char* recipientPhone = "",
                       char* senderPhone = "");

      /**
       */
      CIMDParserResult(const CIMDParserResult& mpr);

      /**
       */
      inline void fillinparameters(result_t resultCode,
                                   int packetNumber,
                                   int firstParam,
                                   int secondParam,
                                   int errorParam,
                                   int smsNumber,
                                   int maxParts,
                                   int partNumber,
                                   char* stringParam,
                                   char* recipientPhone,
                                   char* senderPhone);
      
      /**
       */
      virtual ~CIMDParserResult();
      
      /**
       */
      result_t m_resultCode;
      
      /**
       */
      int      m_packetNumber;
      
      /**
       */
      int      m_firstParam;
      
      /**
       */
      int      m_secondParam;
      
      /**
       */
      int      m_error;
      
      /**
       */
      int      m_smsNumber;
      
      /**
       */
      int      m_maxParts;
      
      /**
       */
      int      m_partNumber;
      
      /**
       */
      char*    m_stringParam;
      
      /**
       */
      char*    m_recipientPhone;
      
      /**
       */
      char*    m_senderPhone;
   
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
 *    CIMDParser.
 *
 */
class CIMDParser {
   public:
      /**
       */
      CIMDParser();

      /**
       */
      virtual ~CIMDParser();

      /**
       */
      CIMDParserResult parse(char* result);

      /**
       */
      char* m_errorString;
   
};
#endif
