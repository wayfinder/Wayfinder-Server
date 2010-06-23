/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Utility.h"

#include "StringUtility.h"
#include "UserConstants.h"
#include "StringTable.h"
#include "ArrayTools.h"

#include <string.h>
#include <fstream>

#include <stdlib.h>

#include <sys/types.h>
#ifndef _MSC_VER
#include <dirent.h>
#endif


#ifdef _MSC_VER
   #include <limits.h>
#else
   #include <sys/resource.h>
   #include <sys/time.h>
#endif

#include <math.h>

namespace {
unsigned char randomNumbers[] =
{
    30, 89, 93,235,243,104,139,156,241,226,238,146, 12,189,124, 58,
   168, 81,  3,201,245, 61,171,202, 17,192,112, 74, 53, 16,111, 83,
   104,203, 61, 91, 51,199,247, 35,169,228,180,181,161, 47,238, 73,
   127,241, 17,116, 46,188, 61, 62,123,173,136,176,188,246,  2, 36,
   193, 63,126,243,  5,116, 22,174, 88,201, 98,248,248, 80, 65,119,
    64, 81,234,109, 13, 39,171,136,212, 51, 55,144, 40, 57,179,232,
   119, 48,219,124,164,240, 42,252,185,139,244,176,219, 52, 39, 26,
   133, 17,135,145, 56, 50, 24, 11,100, 79,154,140,136, 76,116,254,
   124, 78,122, 32, 62,164, 27,246, 47, 15,166,  9, 66,204, 35,199,
   221,169, 87, 20,219,111, 30, 63,189,184,202, 68,  3, 61, 66,127,
   139,188,158,200, 95,185,190,141,199, 99,149,  9, 47,183,207, 11,
    96, 38, 31, 59,148, 60,121, 80,243, 67,148,246,128,213,116, 11,
   145, 18,210,239,202,144,124,145,242, 16,153, 33,199,104, 43, 39,
   141, 73, 97, 32,133,218,112,120, 28,  3,109,156,216,225,166,104,
   242,120, 87,188,  7,210, 76,248,226,229, 25,168, 77, 67,206,217,
   140, 46,248, 17,  7,104,137, 35,106,246,190, 66,214,100,170, 37,
};
}
uint16
Utility::hash(const char *inkey)
{
   const uint8* key = reinterpret_cast<const uint8*>(inkey);
   uint16 hash1 = 0, hash2 = 0;
   while (*key != 0) {
      hash1 ^= ::randomNumbers[*key++];
      if (*key != 0)
         hash2 ^= ::randomNumbers[*key++];
   }
   return (hash1 << 8) | hash2;
}

bool
Utility::read(ifstream& ifs, void* buf, uint32 count)
{
   char* buffer = (char*)buf;
#ifdef _WIN32 
   int32 nbrBytes;
#else
   streamsize nbrBytes;
#endif

   do {
      ifs.read(buffer, count);
      nbrBytes = ifs.gcount();
      count -= nbrBytes;
      buffer += nbrBytes;
   } while((ifs.good()) && (count > 0));

   if(nbrBytes <= 0) {
      if(nbrBytes < 0) {
         DEBUG1(cerr << "Read error: " << strerror(errno) << endl);
         return(false);
      }
      else {
         DEBUG1(cerr << "Read error (tryes to read past end of file): " 
                     << strerror(errno) << endl);
         return(false);
      }
   }

   return(count == 0);
}

bool
Utility::read(int fd, void *buf, uint32 count)
{
#ifndef _MSC_VER
   // Create and initiate the necessery variables
   byte *buffer = (byte *)buf;
   // Save requested size to be able to return the correct value
   ssize_t nbrBytes;

   // Try to read from disk until all bytes are read or error!
   do {
      nbrBytes = ::read(fd, buffer, count);
      count -= nbrBytes;
      buffer += nbrBytes;
   } while ( (nbrBytes > 0) && (count > 0) );

   // Check returnvalue
   if (nbrBytes <= 0) {
      if (nbrBytes < 0) {
         DEBUG1(cerr << "Read error: " << strerror(errno) << endl);
         return false;
      } else {
         DEBUG1(cerr << "Read error (tryes to read past end of file): " 
                     << strerror(errno) << endl);
         return false;
      }
   }

   return (count == 0);
#else
   return FALSE;
#endif
}

int
Utility::directory( const char* path,
                    char*** entries )
{
#ifndef _MSC_VER
   DIR* dir = opendir(path);
   if ( dir == NULL )
      return -1;
   
   errno = 0; // Must be zeroed
   uint32 nbrEntries = 0;
   uint32 allocatedSize = 0;
   *entries = NULL;

   struct dirent* dirent = NULL;
   do {
      dirent = readdir( dir );
      if ( dirent ) {
         *entries = ArrayTool::addElement( *entries,
                                           StringUtility::newStrDup(dirent->d_name),
                                           nbrEntries,
                                           allocatedSize );                 
      } else if ( errno != 0 ) {
         // Delete the entries so that there will be no trouble.
         deleteStrArray(*entries, nbrEntries);
         *entries = NULL;
         closedir(dir);
         return -1;
      }
   } while ( dirent );
   
   closedir(dir);
   return nbrEntries;
#else
   return -1;
#endif
}

bool
Utility::getNextToken(istream& is, 
                      char* str, 
                      int maxLength,
                      const char delimiter)
{
   char c;
   int strPos = 0;

   // Check the first non-space character to see if comment
   while (is >> c) {       // read first non-space character
      // Check if comment
      if (c == '#' || c == '!' || c == '%') {
         is.ignore(1024,'\n');
         if (!is)
            return false;
         continue;
      }
      else if (c == delimiter)
         return true;
      else
         break;
   }

   // Get the rest of the characters
   str[strPos++] = c;
   while (is.get(c)) {
      if (c == delimiter || c == '\r' || c == '\n') {
         break;
      }
      // Add the character to the str, if room left
      if (strPos<maxLength-2) {
         str[strPos++] = c;
      }
   }
   // Add end of string...
   str[strPos] = '\0';

   // If reached the end of the stream something is wrong...
   if (!is) {
      return false;
   }

   // Remove the spaces at the end of the string
   StringUtility::trimEnd(str);

   return true;
}

int Utility::readLine(FILE* file, 
                      char* outBuf, 
                      int bufLen, 
                      const char* endChars)
{
   char * s = NULL;
   if ( ( s = fgets(outBuf, bufLen, file ) ) == NULL ) {
      return -1; // Read failed
   } else {
      int length = strlen(s);
      // Sometimes fgets put the newline into the string. Remove it then.
      while ( length >= 1 && strchr(endChars, s[length-1] ) != NULL )
         s[--length] = '\0';
      return length;
   }
}


char* 
Utility::getString(  const char* inBuf, char startChar, char endChar,
                     char* outBuf, int outBufSize)
{
   char* stringStart = NULL;
   if (startChar == '\0')
      stringStart = (char*) inBuf;
   else {
      stringStart = const_cast<char*>(strchr(inBuf, startChar));
      if (stringStart != NULL)
         stringStart++;
   }
   
   char* stringEnd = NULL;
   if ( endChar == '\0' )
      stringEnd = (char*)(&(inBuf[ strlen(inBuf) ]));
   else if (stringStart != NULL) {
      stringEnd = strchr(stringStart, endChar);
   }
   
   if ( stringStart != NULL && stringEnd != NULL) {
      // Both were found
      intptr_t length = MIN ( (intptr_t)stringEnd - (intptr_t)stringStart, outBufSize-1);
      if (length > 0) {
         strncpy(outBuf, stringStart, length);
      } else {
         length = 0;
      }
      outBuf[length] = '\0';
      if (*stringEnd == '\0')
         return (stringEnd);
      else
         return (stringEnd+1);
   } else {
      // Both were not found.
      return NULL;
   }
}

bool Utility::getNumber(const char* startPtr, char* &dest, int &val)
{
   val = strtol(startPtr, &dest, 10);
   return (dest != startPtr);
}

bool Utility::getUint32(const char* startPtr, char* &dest, uint32 &val)
{
   val = strtoul(startPtr, &dest, 10);
   return (dest != startPtr);
}

bool Utility::getFloat64(const char* startPtr, char* &dest, float64 &val)
{
   val = strtod(startPtr, &dest);
   return (dest != startPtr);
}

int
Utility::parameterPresent(int argc, char* argv[], const char* param,
                          int startCheckingPos)
{
   int curIndex = startCheckingPos;
   while (curIndex < argc) {
      if (strcmp(argv[curIndex], param) == 0)
         return (curIndex);
      curIndex++;
   }
   return (-1);
}

int
Utility::getParameterInt(int argc, char* argv[], const char* param,
                         int &val, int startCheckingPos)
{
   int pos = parameterPresent(argc, argv, param, startCheckingPos);
   if ((pos >= 0) && (pos < argc-1)) {
      long int tmpVal = strtol(argv[pos+1], NULL, 10);
      if ((tmpVal != LONG_MIN) && (tmpVal != LONG_MAX)) {
         val = (int) tmpVal;
         return (pos);
      }
   }

   // defalut return value, val unchanged
   return (-1);
}

int
Utility::getParameterStr(int argc, char* argv[], const char* param, 
                      char* result, int maxNbrChars, int startCheckingPos) 
{
   // Make sure that the caller want any characters
   if ((maxNbrChars < 1) || (result == NULL))
      return (-1);

   int pos = parameterPresent(argc, argv, param, startCheckingPos);
   if ((pos >= 0) && (pos < argc-1)) {
      strncpy(result, argv[pos+1], maxNbrChars);
      if (maxNbrChars > 1)
         result[maxNbrChars-1] = '\0';
         return (pos);
   }

   // defalut return value, val unchanged
   return (-1);
}


#ifndef _MSC_VER
void
Utility::hexDump(ostream& stream,
                 byte* data,
                 unsigned int dataLength,
                 const char* leadingString)
{
   const unsigned int numberOfHex = 8;
   const unsigned int hexLength = numberOfHex * 3 + 7;
   char* hexPart = new char[100];
   char* charPart = new char[61];
   
   hexPart[0]  = '\0';
   charPart[0] = '\0';
   
   unsigned int col = 0;
   unsigned int pos = 0;
   while ( pos < dataLength ) {
      sprintf(hexPart, "0x%04X ", pos);
      while ( col < numberOfHex && pos < dataLength ) {
         char tempStr[40];
         sprintf(tempStr, "%02X ", data[pos]);
         strcat(hexPart, tempStr);
         // If the character is printable then print it.
         if ( (data[pos] > 32 && data[pos] <= 127) ||
              (data[pos] > 161) ) {
           char tempStr[2];
           tempStr[0] = data[pos];
           tempStr[1] = '\0';
           strcat(charPart, tempStr);
         }
         else // Print a dot, like in the C128 monitor
           strcat(charPart, ".");
         pos++;
         col++;
      }
      col = 0;
      while ( strlen(hexPart) <  hexLength )
        strcat(hexPart, " ");
      if ( leadingString == NULL )
        stream << hexPart << " " << charPart << endl;
      else
        stream << leadingString << hexPart << " " << charPart << endl;
      hexPart[0] = '\0';
      charPart[0] = '\0';
   }
   delete [] charPart;
   delete [] hexPart;

}

#endif


// Returns the compass degree direction in N, NW etc.
// res is the letter resolution from 1-3 

StringCode Utility::getCompass(int degree, int res)
{
   const int bounds1[] = {45, 135, 225, 315};
   const int bounds2[] = {23, 68, 113, 158, 203, 248, 293, 338};
   const int bounds3[] = {11, 34, 56, 79, 101, 124, 146, 169,
                          191, 214, 236, 259, 281, 304, 326, 349};
   bool found = false;  
   int i = 0;
   degree = degree % 360;
   
   StringTable::stringCode northCode = StringTable::DIR_NORTH;
   
   switch (res) {    
      case 1:
         while (!found) {
            if (i == 0)
               found = (degree <= bounds1[0] || degree > bounds1[3]);
            else
               found = (degree <= bounds1[i] && degree > bounds1[i-1]);
            i++;
         }       
         return (StringTable::stringCode(northCode + (i-1)*4));
         break;
      case 2:
         while (!found) {
            if (i == 0)
               found = (degree <= bounds2[0] || degree > bounds2[7]);
            else
               found = (degree <= bounds2[i] && degree > bounds2[i-1]);
            i++;
         }
         return (StringTable::stringCode(northCode + (i-1)*2));
         break;
      case 3:
         while (!found) {
            if (i == 0)
               found = (degree <= bounds3[0] || degree > bounds3[15]);
            else
               found = (degree <= bounds3[i] && degree > bounds3[i-1]);
            i++;
         }
         return (StringTable::stringCode(northCode + (i-1)));
         break;
      default:
         break;
   }
   DEBUG1( cerr << "Uncaught directions and/or case!" << endl );
   return northCode;// default.
}

bool
Utility::genCloseZipCodes(const char* zc1,
		          const char* zc2,
			  uint32 zipCodeDiff)
{
   bool close = false;
   
   int len1 = strlen(zc1);
   int len2 = strlen(zc2);
   int minlen, maxlen;
   
   if (len1 < len2) {
      minlen = len1;
      maxlen = len2;
   } else {
      minlen = len2;
      maxlen = len1;
   }
   
   if ( StringUtility::onlyDigitsInString(zc1) &&
        StringUtility::onlyDigitsInString(zc2) ) {
      //only digits
      if (minlen == maxlen) {
         close = closeZipCodes(zc1, zc2, zipCodeDiff, '9', '0');
      } else {
         float64 zcDist = zipCodeDistance( zc1, zc2, '0', '9');
	 if ( (fabs(zcDist) < (int) zipCodeDiff) &&
	      (minlen > 0.79 * maxlen) ) {
	    //accept a small difference in lenght if the common
	    //digits are within the tolerance
	    close = true;
	 } else {
	    close = false;
	 }
      }
   } else {
      //letters in the zipCodes
      bool result = true;
      int i = 0;
      while ( i < minlen ) {
         if ( zc1[i] != zc2[i] ) {
	    if ( (i+1) < 0.79 * maxlen ) {
	       result = false;
	    } else if ( abs(zc1[i] - zc2[i]) >= (int) zipCodeDiff ) {
	       // >= similar to closeZipCodes
	       result = false;
	    }
	    i++;   //while-loop started at 0
	    break;
	 }
	 i++;
      }//while
      
      if ( result && (i < (0.79 * maxlen)) ) {
         //not equal length, but all common chars equal
	 result = false;
      }
      close = result;
   }
   
   return close;
}

bool
Utility::closeZipCodes(const char* zc1,
                       const char* zc2,
                       uint32 zipCodeDiff,
                       int32 high,
                       int32 low)
{
   bool result = false;

   const char* p1 = zc1;
   const char* p2 = zc2;
   int diff = 0;
   uint8 c1 = *p1;
   uint8 c2 = *p2;
   
   while ( abs(diff) < (int) zipCodeDiff ) {
      diff = diff*(high-low+1) + (c1-c2);
      if (c1 != '\0') {
         p1++;
      }
      if (c2 != '\0') {
         p2++;
      }
      c1 = *p1;
      c2 = *p2;
      if ((c1 == '\0') &&
          (c2 == '\0'))
      {
         if ( abs(diff) < (int) zipCodeDiff ) {
            result = true;
         }
         break;
      }
   }
   return result;
}

float64
Utility::zipCodeDistance(const char* zc1,
                         const char* zc2,
                         char low,
                         char high)
{
   float64 result = 0;
   if ((zc1 == NULL) && (zc2 == NULL)) {
      result = 0;
   } else if ((zc1 == NULL) && (zc2 != NULL)) {
      result = -1e99;
   } else if ((zc1 != NULL) && (zc2 == NULL)) {
      result = 1e99;
   } else {
      uint32 i = 0;
      result = 0;
      while ((zc1[i] != 0) && (zc2[i] != 0)) {
         result = result * (int(high) - int(low) + 1) + zc1[i] - zc2[i];
         i++;
      } // while
   }
   return result;
}

void Utility::getRoutingCost( uint8& dist, uint8& time, uint8 percent )
{
   dist = (uint8)(percent*3.5); //2.70438);
   time = 10 - percent;
}

uint8 Utility::getWalkRoutingCost( uint8 speed, 
                                   uint32 measureType )
{
   UserConstants::MeasurementType type = UserConstants::MeasurementType(
      measureType );
   if ( type == UserConstants::MEASUREMENTTYPE_IMPERIAL ) { // mph
      switch( speed ){
         case 1:
            return 158;
         case 2:
            return 79;
         case 3:
            return 53;
         case 4:
            return 40;
         case 5:
            return 32;
         case 6:
            return 26;
         default:
            return 53; // 3 mph
      }
   }
   else{ // km/h
      switch( speed ){
         case 1:
            return 255;
         case 2:
            return 128;
         case 3:
            return 85;
         case 4:
            return 64;
         case 5:
            return 51;
         case 6:
            return 43;
         case 7:
            return 36;
         case 8:
            return 32;
         case 9:
            return 28;
         case 10:
            return 25;
         default:
            return 51; // 5 km/h
      }
   }
}

const char*
Utility::convertBoolToString(bool b)
{
   if (b)
      return("Y");
   else
      return("N");
}

bool
Utility::getBoolFromString(const char* boolStr)
{
   if ( (boolStr[0] == 'Y') || (boolStr[0] == 'y') )
      return true;
   else if ( strcasecmp(boolStr, "true") == 0 )
      return true;
   else
      return false;
}

void
Utility::deleteStrArray( char** str,
                         int nbr )
{
   if ( str == NULL )
      return;

   for ( int i = 0; i < nbr; ++i ) {
      delete [] str[i];
   }
   delete [] str;
}

