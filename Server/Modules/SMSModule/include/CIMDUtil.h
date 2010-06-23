/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef __GNUC__
#pragma interface
#endif
#ifndef CIMD_UTIL_H
#define CIMD_UTIL_H

#define NUL ((char)0x00)
#define STX ((char)0x02)
#define TAB ((char)0x09)
#define ETX ((char)0x03)
#define TAB_STRING "\011\0"
#define ETX_STRING "\003\0"

#include "Types.h"
#include <stdio.h>
#include "SMSPacket.h"

/**
 *    CIMDUtil.
 *
 */
class CIMDUtil {
   public:
      /**
       *    Calculate the CIMD checksum.
       *    @param message The message to be checksummed. Space (2 chars )for
       *                   the checksum
       *                   should be included before the last <etx>.
       *    @param length  The length of the message.
       */     
      static int calcCS(char* message, int length) {
         int checksum = 0;
         char* p = message;
         char* petx = message + length - 1;
         
         while ( petx - p > 2 ) {
            checksum += *p;
            checksum &= 0xff;
            p++;
         }
         return checksum;
      }

      
      /**
       */
      static int makeSendRequest(char* outBuffer,
                                 int pktNumber, char* destAddress,
                                 byte* userData, int dataLength,
                                 const char* hexData,
                                 const char* header, int CIMDEncoding,
                                 int smsNbr,
                                 int nbrParts,
                                 int partNbr);
      
      static int makeSendRequestText(char* outBuffer,
                              int pktNumber, char* destAddress,
                              byte* data, int dataLength,
                              int smsNbr,
                              int nbrParts,
                              int partNbr,
                              SMSSendRequestPacket::smsType_t smsType);

      static int makeSendRequestWAPPushSI(char* outBuffer,
                              int pktNumber, char* destAddress,
                              const char* href,
                              const char* message,
                              int smsNbr,
                              int nbrParts,
                              int partNbr);
         
      /**
       */
      static int makeLoginRequest(char* outBuffer, int packetNumber,
                                  char* userID, char* passwd);

      /**
       */
      static int makeDeliverResponse(char* outBuffer, int packetNumber);

      /**
       *   Make an AliveRequest and put it into the outBuffer.
       *   @param outBuffer    The buffer to put the result into.
       *   @param packetNumber The packet number to use.
       *   @return The length of the result.
       */
      static int makeAliveRequest(char* outBuffer, int packetNumber);
      
      /**
       */
      static int addString(char* buffer, int &pos, const char* stringToAdd);

      /**
       *   Replaces the packetNumber in the message buffer with the
       *   packetnumber in packetNumber.
       *   @return true if the substitution was successful.
       */
      static bool replacePktNbr(byte* buffer, int bufLen, int packetNumber);
      
      /**
       *    Converts the combi-coded message in inBuffer to a GSM-coded
       *    message in outBuffer. The length of the decoded message can
       *    be shorter than the encoded.
       *    @param outBuffer   The buffer to put the result into.
       *    @param inBuffer    The buffer to read from
       *    @param inBufferLen The length of inbuffer.
       *    @return The length of the data in outBuffer.
       */
      static int convertFromSpecialCombi(char* outBuffer,
                                         char* inBuffer,
                                         int inBufferLen);

      /**
       *    Converts a 7-bit encoded message to Nokia's special combis.
       */
      static int convertToSpecialCombi(char* outBuffer,
                                       char* inBuffer,
                                       int inBufferLen);

      /**
       */
      static int convertFromHex(char* outBuffer,
                                char* inBuffer,
                                int inBufferLen);

      /** 
       *   Converts the 16-bit hexdata to integers.
       */
      static int convertFromWordHex(uint16* outBuffer,
                                    char* inBuffer,
                                    int inBufferLen);

      /**
       *   Converts 16-bit words to iso-latin-1
       */
      static int convertFromWords(char* outBuffer,
                                  uint16* inBuffer,
                                  int inBufferLen);

      /**
       *    @return A pointer to a constant string containing a description
       *            of the error.
       */
      static const char* errorToText(int errorCode);

   private:
      /**
       */
      struct specialCombiMap_t {
         const char* combi;
         char code;
         int len;
      };

      /**
       */
      struct ucs2Map_t {
         char isoChar;
         uint16 ucs2Code;
      };
      
      /**
       *    Map used when converting combis to chars
       */
      static specialCombiMap_t combiMap[];

      /**
       *    Map used when converting UCS2 encoded 16-bit data to iso.
       */
      static const ucs2Map_t ucs2Map[];
      
      /**
       *    Size of the combimap. Set with initCombiMap in .cpp-file.
       *    It is to be considered as a dummy variable, because the main
       *    purpose of it is to make sure that initCombiMap is run.
       */
      static int combiMapSize;
      
      /**
       *    Size of the map containing the translation from UCS2 to ISO.
       *    Its value is calculated by initUCS2Map in the .cpp file.
       */
      static int ucs2MapSize;

      /**
       *    @return The size of ucs2Map.
       */ 
      static int initUCS2Map(const CIMDUtil::ucs2Map_t ucs2Map[]);
      
      /**
       *    Sets the lengths in combiMap so that we will save a lot of
       *    strlens later.
       */
      static int initCombiMap(specialCombiMap_t initMap[]);
};

#endif

