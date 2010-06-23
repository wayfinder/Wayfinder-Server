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
#pragma implementation
#endif
#include "CIMDUtil.h"
#include "SMSUtil.h"
#include "MC2String.h"
#include "URL.h"

#define SUBMIT       "03"
#define LOGIN        "01"
#define USER_ID      "010"
#define PASSWD       "011"
#define ADDRESS      "021"
#define ORIG_ADDR    "023"
#define USER_HEADER  "032"
#define USER_DATA    "033"
#define HEX_DATA     "034"
#define DATA_CODING_SCHEME  "030"
#define RESP_DELIVER "70"
#define ALIVE_REQ    "40"

#define ILLEGAL_COMBI -1

int CIMDUtil::makeSendRequest(char* outBuffer,
                              int pktNumber, char* destAddress,
                              byte* userData, int dataLength,
                              const char* hexData,
                              const char* header, int CIMDEncoding,
                              int smsNbr,
                              int nbrParts,
                              int partNbr)
{
   char* tempStr = new char[256];
   int pos = 0;
   // ADD STX
   outBuffer[pos++] = STX;
   // Add command:packetnumber<tab>
   sprintf(tempStr, "%s:%03d%c", SUBMIT, pktNumber, TAB);
   addString(outBuffer, pos, tempStr);
   // Add destination address <tab>
   sprintf(tempStr, "%s:+%s", ADDRESS, destAddress );
   addString(outBuffer, pos, tempStr);
   if( smsNbr != -1 ) {
      addString(outBuffer, pos, TAB_STRING);
      addString(outBuffer, pos, USER_HEADER );
      addString(outBuffer, pos, ":");
      SMSUtil::createConcatenatedHeader( outBuffer,
                                         pos,
                                         smsNbr,
                                         nbrParts,
                                         partNbr );
      outBuffer[pos++] = TAB;
   }
   if (header != NULL) {
      addString(outBuffer, pos, TAB_STRING);
      addString(outBuffer, pos, USER_HEADER);
      addString(outBuffer, pos, ":");
      addString(outBuffer, pos, header);
   }
   if (CIMDEncoding != -1) {
      char dataCodingScheme[4];
      sprintf(dataCodingScheme, "%d", CIMDEncoding);
      addString(outBuffer, pos, TAB_STRING);
      addString(outBuffer, pos, DATA_CODING_SCHEME);
      addString(outBuffer, pos, ":");
      addString(outBuffer, pos, dataCodingScheme);
   }
   if (hexData != NULL) {
      addString(outBuffer, pos, TAB_STRING);
      addString(outBuffer, pos, HEX_DATA);
      addString(outBuffer, pos, ":");
      addString(outBuffer, pos, hexData);
   } else {
#ifdef CIMD_USES_HEX
      // Add user data as HEX
      addString(outBuffer, pos, TAB_STRING);
      addString(outBuffer, pos, HEX_DATA);
      addString(outBuffer, pos, ":");

      // Allocate a buffer to put the 7-bitencoded data into.
      // It will be shorter than dataLength but it doesn't hurt
      // to avoid the division and multiplication required.
      // NB: This doesn't work when sending text messages due to
      // the absence of a length in the message protocol. That means that
      // if we send e.g. 49 bits and convert them into 8 bits they will become
      // 56 bits and the other side has no way to know if the last 7 bits
      // are there because we haven't filled the message or if we want to send
      // them. So it sends them.
      byte* dataShort = new byte[dataLength];
      int shortLength = SMSUtil::putEncoded120x8(dataShort, userData, dataLength);
      for(int i=0; i < shortLength; i++) {
         char hexString[5];
         sprintf(hexString, "%02X", dataShort[i]);
         addString(outBuffer, pos, hexString);
      }
      delete dataShort;
#else
      // Add user data as USERDATA
      addString(outBuffer, pos, TAB_STRING);
      addString(outBuffer, pos, USER_DATA);
      addString(outBuffer, pos, ":");
      // Allocate a buffer to put the combidata into. If all the characters
      // in data will be converted into combis then the length will be three
      // times as long as dataLength.
      char* dataLong = new char[dataLength*3 + 2];
      int longLength = CIMDUtil::convertToSpecialCombi(dataLong,
                                                       (char*)userData,
                                                       dataLength);
      dataLong[longLength] = '\0'; // Add terminating zero
      addString(outBuffer, pos, dataLong);
      delete [] dataLong;
#endif
   }

   // Add <tab>
   addString(outBuffer, pos, TAB_STRING);
   // Add space for checksum and ETX
   addString(outBuffer, pos, "  ");
   addString(outBuffer, pos, ETX_STRING);
   // Calculate checksum and add it.
   int checkSum = calcCS(outBuffer, pos);
   sprintf(tempStr, "%02X", checkSum);
   outBuffer[pos-3] = tempStr[0];
   outBuffer[pos-2] = tempStr[1];
   // Clean up and finish
   delete [] tempStr;
   return pos;
}

int CIMDUtil::makeSendRequestText(char* outBuffer,
                              int pktNumber, char* destAddress,
                              byte* data, int dataLength,
                              int smsNbr,
                              int nbrParts,
                              int partNbr,
                              SMSSendRequestPacket::smsType_t smsType)
{
   return makeSendRequest(outBuffer, pktNumber, destAddress, data, dataLength, 
                          NULL, NULL, -1, smsNbr, nbrParts, partNbr);
}

int CIMDUtil::makeSendRequestWAPPushSI(char* outBuffer,
                              int pktNumber, char* destAddress,
                              const char* href,
                              const char* message,
                              int smsNbr,
                              int nbrParts,
                              int partNbr)
{
   char udh[20];
   char body[1024];
   URL url(href);

   if (!url.isValid())
      return -1;
   // WDP UDH (Wireless Data Protocol, User Data Header):
   // 06    Length of header
   // 05    WDP application port
   // 04    Length of dest + source port
   // 0B84  Destination port, WAP browser
   // 23F0  Source port
   strcpy(udh, "0605040B8423F0");
   mc2dbg2 << "CIMDUtil::makeSendRequestWAPPushSI: udh: " << udh << endl;
   // WSP (Wireless Session Protocol)
   strcpy(body, "1B0601AE");
   // WAP Push Service Indication (SI):
   // 02    WBXML v1.2
   // 05    DTD Service Indication
   // 6A    UTF-8
   // 00    String table length: NULL 
   // 45    <si>, with content
   // C6    <indication> with content and attributes
   // 0C    "http://"
   // 03    'inline string follows'
   strcat(body, "02056A0045C6");

   const char* host = url.getHost();
   const char* proto = url.getProto();
   const char* path = url.getPath();
   mc2dbg4 << "CIMDUtil::makeSendRequestWAPPushSI: proto: " << proto 
           << ", host: " << host << ", path: " << path << endl;
   if (strcmp(proto, "http") == 0) {
      if (strncmp(host, "www.", 4) == 0) {
         strcat(body, "0D");  // "http://www."
         host = host + 4;     // skip "www."
      } else {
         strcat(body, "0C");  // "http://"
      }
   } else if (strcmp(proto, "https") == 0) {
      if (strncmp(host, "www.", 4) == 0) {
         strcat(body, "0F");  // "https://www."
         host = host + 4;     // skip "www."
      } else {
         strcat(body, "0E");  // "https://"
      }
   }
   // 03    inline string follows
   strcat(body, "03");
   int pos = strlen(body);
   for(unsigned int i=0; i < strlen(host); i++) {
      pos += sprintf(body + pos, "%02X", host[i]);
   }
   // ".com/", ".edu/", ".net/" and ".org/" can also be encoded as tokens
   // 85, 86, 87, 88 respectively
   for(unsigned int i=0; i < strlen(path); i++) {
      pos += sprintf(body + pos, "%02X", path[i]);
   }
   // 00    String ends
   // 01    end [<indication> attribute list]
   // 03    inline string follows
   sprintf(body + pos, "000103");
   pos = strlen(body);
   for(unsigned int i=0; i < strlen(message); i++) {
      pos += sprintf(body + pos, "%02X", message[i]);
   }
   // 00    String ends
   // 01    end [<indication>]
   // 01    end [<si>]
   strcat(body, "000101");
   mc2dbg2 << "CIMDUtil::makeSendRequestWAPPushSI: body: " << body << endl;
   mc2dbg << "CIMDUtil::makeSendRequestWAPPushSI: udh length: " << strlen(udh) << ", body length: " << strlen(body)  << endl;

   if (strlen(body) > 266)
      return -2;  // message is too long, would be rejected by the SMSC

   // CIMDEncoding is set 244 according to GSM 03.38, section 4, page 9
   // in version 5.3.0 July 1996 of the spec
   // The binary string is:
   // 1111  Indicates "Data coding/message class"
   // 0     Bit 3 reserved
   // 1     8-bit data
   // 00    Message class 0
   return makeSendRequest(outBuffer, pktNumber, destAddress, NULL, 0, 
                          body, udh, 244, smsNbr, nbrParts, partNbr);
}


int CIMDUtil::makeLoginRequest(char* outBuffer, int packetNumber,
                               char* userID, char* passwd)
{
   char* tempStr = new char[256];
   int pos = 0;
   // ADD STX
   outBuffer[pos++] = STX;
   // Add command:packetNumber and <tab>
   sprintf(tempStr, "%s:%03d%c", LOGIN, packetNumber, TAB);
   addString(outBuffer, pos, tempStr);
   // Add 010:login<tab>011:passwd<tab>
   sprintf(tempStr, "%s:%s%c%s:%s%c", USER_ID, userID, TAB, PASSWD, passwd,
           TAB);
   addString(outBuffer, pos, tempStr);
   // Add place for checksum
   sprintf(tempStr, "  %c", ETX);
   addString(outBuffer, pos, tempStr);
   int checkSum = calcCS(outBuffer, pos);
   // Add checksum
   sprintf(tempStr, "%02X", checkSum);
   outBuffer[pos-3] = tempStr[0];
   outBuffer[pos-2] = tempStr[1];
   delete [] tempStr;
   return pos;
}


int CIMDUtil::makeDeliverResponse(char* outBuffer, int packetNumber)
{
   char* tempStr = new char[256];
   int pos = 0;
   // ADD STX
   outBuffer[pos++] = STX;
   // Add command:packetNumber and <tab>
   sprintf(tempStr, "%s:%03d%c", RESP_DELIVER, packetNumber, TAB);
   addString(outBuffer, pos, tempStr);
   // Add place for checksum
   sprintf(tempStr, "  %c", ETX);
   addString(outBuffer, pos, tempStr);
   int checkSum = calcCS(outBuffer, pos);
   // Add checksum
   sprintf(tempStr, "%02X", checkSum);
   outBuffer[pos-3] = tempStr[0];
   outBuffer[pos-2] = tempStr[1];   
   delete [] tempStr;
   return pos;
}


int CIMDUtil::makeAliveRequest(char* outBuffer, int packetNumber)
{
   char* tempStr = new char[256];
   int pos = 0;
   // ADD STX
   outBuffer[pos++] = STX;
   // Add command:packetNumber and <tab>
   sprintf(tempStr, "%s:%03d%c", ALIVE_REQ, packetNumber, TAB);
   addString(outBuffer, pos, tempStr);
   // Add place for checksum
   sprintf(tempStr, "  %c", ETX);
   addString(outBuffer, pos, tempStr);
   int checkSum = calcCS(outBuffer, pos);
   // Add checksum
   sprintf(tempStr, "%02X", checkSum);
   outBuffer[pos-3] = tempStr[0];
   outBuffer[pos-2] = tempStr[1];   
   delete [] tempStr;
   return pos;
}

int CIMDUtil::addString(char* buffer, int &pos, const char* stringToAdd)
{
   int length = strlen(stringToAdd);
   for(int i=0; i < length; i++)
      buffer[pos++] = stringToAdd[i];
   return length;
}


int CIMDUtil::convertFromSpecialCombi(char* outBuffer,
                                      char* inBuffer,
                                      int inBufferLen)
{
   int pos = 0;
   char* inPointer  = inBuffer;
   char* outPointer = outBuffer;
   while ( pos < inBufferLen ) {
      int i = 0;
      bool found = false;
      do {
         if ( strncmp( inPointer, combiMap[i].combi, combiMap[i].len ) == 0 ) {
            *outPointer++ = combiMap[i].code;
            inPointer+= combiMap[i].len;
            pos += combiMap[i].len;
            found = true;
         }
      } while ( combiMap[++i].code != ILLEGAL_COMBI && found == false);
      // If we didn't find it in combiMap no conversion is necessary.
      if ( found == false ) {
         *outPointer++ = *inPointer++;
         pos++;
      }
   }
   return outPointer - outBuffer;
}


int CIMDUtil::convertFromHex(char* outBuffer,
                             char* inBuffer,
                             int inBufferLen)
{
   char hexChars[3];
   unsigned int temp;
   hexChars[2] = '\0';
   for(int i=0; i < inBufferLen;) {
      hexChars[0] = inBuffer[i++];
      hexChars[1] = inBuffer[i++];
      sscanf(hexChars, "%x", &temp);
      outBuffer[(i / 2)-1] = temp;
   }
   return inBufferLen / 2;
}


int
CIMDUtil::convertFromWordHex(uint16* outBuffer,
                             char* inBuffer,
                             int inBufferLen)
{
   // OPTIM: Change the copying of caracters into an inc of inBuffer
   char hexChars[5];
   hexChars[4] = '\0';
   uint32 temp;
   for(int i=0; i < inBufferLen; i+=4) {
      for(int j=0; j < 4; j++)
         hexChars[j] = inBuffer[i+j]; // Copy four hex chars
      sscanf(hexChars, "%x", &temp);
      outBuffer[i/4] = temp;
   }
   return inBufferLen / 4;
}


int
CIMDUtil::convertFromWords(char* outBuffer,
                           uint16* inBuffer,
                           int inBufferLen)
{
   int convertedLength = 0;
   for(int i=0; i < inBufferLen; i++) {
      if ( inBuffer[i] < 256 )
         outBuffer[convertedLength++] = inBuffer[i] & 0xff;
      else {
         // Look in the table and see if we can translate it anyways.
         bool found = false;
         for ( int j=0; j < ucs2MapSize && !found ; j++ ) {
            if ( ucs2Map[j].ucs2Code == inBuffer[i] ) {
               outBuffer[convertedLength++] = ucs2Map[j].isoChar;
               found = true;
            }
         }
      }
   }
   return convertedLength;
}

bool
CIMDUtil::replacePktNbr(byte* buffer, int bufLen, int packetNumber)
{
   char* colon  = NULL;
   char* tabPos = NULL;
   if ( ( colon = strchr((char*)buffer, ':') ) != NULL &&
        ( tabPos = strchr((char*)buffer, TAB) ) != NULL &&
        (tabPos - colon) == 4 ) {
      char* tmpS = new char[4];
      sprintf(tmpS, "%03d", packetNumber);
      for(int i=0; i < 3; i++)
         buffer[((byte*)colon-buffer)+i+1] = ((byte*)tmpS)[i];
      delete [] tmpS;
      return true;
   } else {
      cout << "Error in replacePacketNumber" << endl;
      return false;
   }
      
      

}


int
CIMDUtil::convertToSpecialCombi(char* outBuffer,
                                char* inBuffer,
                                int inBufferLen)
{
   int pos = 0;
   char* inPointer  = inBuffer;
   char* outPointer = outBuffer;
   
   while ( pos < inBufferLen ) {
      int i = 0;
      bool found = false;
      do {
         if ( combiMap[i].code == *inPointer ) {
            for(int k=0; k < combiMap[i].len; k++)
               *outPointer++ = combiMap[i].combi[k];
            pos++;
            inPointer++;
            found = true;
         }
      } while ( combiMap[++i].code != ILLEGAL_COMBI && found == false );
      // If we didn't find it in combimap - send it as it is.
      if ( found == false ) {
         *outPointer++ = *inPointer++;
         pos++;
      }
   }
   return outPointer - outBuffer;
}

CIMDUtil::specialCombiMap_t
CIMDUtil::combiMap[] = {
   { "_Oa",    0},
   { "_L-",    1},
   { "$",      2},
   { "_XX",    2}, // Euro symbol will be dollar.
   { "_Y-",    3},
   { "_e`",    4},
   { "_e'",    5},
   { "_u`",    6},
   { "_i`",    7},
   { "_o`",    8},
   { "_C,",    9},
   { "_O/",   11},
   { "_o/",   12},
   { "_A*",   14}, // Å
   { "_a*",   15}, // å
   { "_gd",   16},
   { "_--",   17},
   { "_gf",   18},
   { "_gg",   19},
   { "_gl",   20},
   { "_go",   21},
   { "_gp",   22}, // PI
   { "_gi",   23},
   { "_gs",   24},
   { "_gt",   25},
   { "_gx",   26},
   { "_XX",   27}, // Reserved...
   { "_AE",   28},
   { "_ae",   29},
   { "_ss",   30}, // Double s
   { "_E'",   31},
   { "_qq",   34},
   { "_XXe",  36}, // euro symbol
   { "_!!",   64},
   { "_A\"",  91},
   { "_O\"",  92},
   { "_N~",   93},
   { "_U\"",  94},
   { "_so",   95},
   { "_??",   96},
   { "_a\"", 123},
   { "_o\"", 124},
   { "_n~",  125},
   { "_u\"", 126},
   { "_a`",  127},
   { "]"  ,   14},   // Å
   { "}"  ,   15},   // å
   { "", ILLEGAL_COMBI } // Vaktpost, must be here.
};


int
CIMDUtil::initCombiMap(CIMDUtil::specialCombiMap_t initMap[])
{
   int i = 0;
   do {
      initMap[i].len = strlen(initMap[i].combi);
   } while ( initMap[i++].code != ILLEGAL_COMBI );
   return i;
}

int
CIMDUtil::combiMapSize = initCombiMap(combiMap);

const
CIMDUtil::ucs2Map_t
CIMDUtil::ucs2Map[] = {
   { 'ß', 0x03B2 },
   { ' ', 0x0000 } // Vaktpost, must be in the last position
};

int CIMDUtil::initUCS2Map(const CIMDUtil::ucs2Map_t ucs2Map[]) {
   int i = 0;
   while ( ucs2Map[i].ucs2Code != 0x000 )
      i++;   
   return i;
};

int
CIMDUtil::ucs2MapSize = initUCS2Map(ucs2Map);

const char*
CIMDUtil::errorToText(int errorCode)
{
   switch ( errorCode ) {
      case 1:
         return "Unexpected operation";
      case 2:
         return "Syntax error";
      case 3:
         return "Unsupported parameter error";
      case 4:
         return "Connection to SMSC list";
      case 5:
         return "No response from SMSC";
      case 6:
         return "General system error, du!";
      case 7:
         return "Cannot find information";
      case 8:
         return "Parameter formatting error";
      case 9:
         return "Requested operation failed";
      case 100:
         return "Invalid login";
      case 101:
         return "Incorrect access type";
      case 102:
         return "Too many users with this login ID";
      case 103:
         return "Login refused by SMSC";
      case 300:
         return "Incorrect destination address";
      case 301:
         return "Incorrect number of destination addresses";
      case 302:
         return "Syntax error in user data parameter";
      case 303:
         return "Incorrect bin/head/normal user data parameter combination";
      case 304:
         return "Incorrect dcs parameter usage";
      case 305:
         return "Incorrect validity period parameters usage";
      case 306:
         return "Incorrect originator address usage";
      case 307:
         return "Incorrect pid parameter usage";
      case 308:
         return "Incorrect first delivery parameter usage";
      case 309:
         return "Incorrect reply path usage";
      case 310:
         return "Incorrect status report request parameter usage";
      case 311:
         return "Incorrect cancel enabled parameter usage";
      case 312:
         return "Incorrect priority parameter usage";
      case 313:
         return "Incorrect tariff class parameter usage";
      case 314:
         return "Incorrect service description parameter usage";
      case 400:
      case 601:
         return "Incorrect address parameter usage";
      case 401:
      case 500:
      case 600:
         return "Incorrect scts parameter usage";
      case 501:
      case 602:
         return "Incorrect mode parameter usage";
      case 502:
      case 603:
         return "Incorrect parameter combination";
      case 800:
         return "Changing password failed";
      case 801:
         return "Changing password not allowed";
      case 900:
         return "Unsupported item requested";
      default:
         return "Unknown error";
   }  
}
