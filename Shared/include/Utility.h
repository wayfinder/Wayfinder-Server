/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UTILITY_H
#define UTILITY_H

#include "config.h" 

#include <iosfwd>

#ifndef _MSC_VER
   // MAX and MIN macros. It's better to include them first.
   #undef MAX
   #undef MIN
   #include<sys/param.h>
#endif


#ifndef _MSC_VER
   #define HEXDUMP(a,b,c,d) Utility::hexDump(a,b,c,d)
#else
   #define HEXDUMP(a,b,c,d)
#endif


// Minimum macro
#ifndef MIN
   #define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif

// Maximum macro
#ifndef MAX
   #define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

class StringCode;

/**
  *   Contains static utility functions.
  *
  */
class Utility {
public:
      /**
        *   Checks a bool and returns "Y" for true and "N" for false.
        *   @param b  A bool.
        */
      static const char* convertBoolToString(bool b);

      /**
        *   Converts a boolStr ("Y" or "N" or "True" or "False") to a bool.
        *   @param boolStr  The string containing "Y" or "N".
        */
      static bool getBoolFromString(const char* boolStr);

      /**
       *    Hashing function described in
       *    ``Fast Hashing of Variable-Length Text Strings'',
       *    by Peter K Pearson, CACM, June 1990.
       *
       *    Note that the returned hashcode must be modded with
       *    the bucket cardinal of the hashtable. The bucket cardinal
       *    should be a prime for best performance.
       *
       *    @param   key   NULL terminated key to hash
       *    @return  Hashcode for the given key
       */
      static uint16 hash(const char *key);

      /**   Secure file read routine. The method reads count bytes
        *   from the file given by ifs and places the data in buf.
        *
        *   @param   ifs   input file stream
        *   @param   buf   result buffer
        *   @param   count requested number of bytes
        *
        *   @return  true on success, false on failure
        */
      static bool read(ifstream& ifs, void* buf, uint32 count);
      
      /**   
        *   Secure file read routine. The method reads count bytes
        *   from the file pointed to by fd and places the data in buf.
        *
        *   @deprecated
        *   @param   fd    filedescriptor
        *   @param   buf   result buffer
        *   @param   count requested number of bytes
        *
        *   @return  true on success, false on failure
        */
      static bool read( int fd, void *buf, uint32 count );

      /**
       *    Get the next token from a asci-file. Lines with @p #, @p ! 
       *    or @p % as the first non-space character are skiped.
       *    Whitespaces at the end of a token are also skiped. If a token
       *    are longer than maxLength it is truncated.
       *    @param   is          The file to read from.
       *    @param   str         The preallocated buffer where the token is 
       *                         stored.
       *    @param   maxLength   The maximum number of bytes to store in str.
       *    @param   delimiter   The character that delimits the tokens (except
       *                         from \n or \r).
       *    @return  True if the token is stored in str, false otherwise.
       */
      static bool getNextToken(istream& is, char* str, int maxLength, 
                               const char delimiter = '\t');

      /**
       *   Reads characters from a stream until endline
       *   occurs. The endChars are then removed from the string if
       *   the string ends with them.
       *
       *   @param file The stream to read from.
       *   @param outBuf The buffer to put the result into.
       *   @param bufLen The maximum number of characters to read.
       *   @param endChars A string of characters that should be removed
       *                   if they are at the end of the string.
       *   @return The number of bytes read. 
       */
      static int readLine(FILE* file, 
                          char* outBuf, 
                          int bufLen, 
                          const char* endChars);

      /**
        *   Get a string from a stringbuffer.
        *   
        *   @param inBuf The buffer to look in.
        *   @param startChar  The start delimiter. If == '\0' then the
        *                     returned string starts at the begining
        *                     of inBuf.
        *   @param endChar    The end delimiter. If == '\0' the end of 
        *                     string will be the end delimiter.
        *   @param outBuf     The buffer to put the result into.
        *   @param outBufSize The maximum number of chars to put in outBuf
        *   @return The length of the string put in outBuf.
        *   NULL if startChar || endChar wasn't found.
        */
      static char* getString( const char* inBuf, char startChar, 
                              char endChar, char* outBuf, int outBufSize);

      /**
        *   Get an integer from a string.
        *   @param   startPtr The string to parse.
        *   @param   dest     Outparameter that is set to the next 
        *                     position in the string.
        *   @param   val      Outparameter that is set to the value
        *                     that is read from the string.
        *   @return  True if val is set, false otherwise.
        */
      static bool getNumber(const char* startPtr, char* &dest, int &val);

      /**
        *   Get an unsigned integer from a string.
        *   @param   startPtr The string to parse.
        *   @param   dest     Outparameter that is set to the next 
        *                     position in the string.
        *   @param   val      Outparameter that is set to the value
        *                     that is read from the string.
        *   @return  True if val is set, false otherwise.
        */
      static bool getUint32(const char* startPtr, char* &dest, uint32 &val);

      /**
        *   Get a float from a string.
        *   @param   startPtr The string to parse.
        *   @param   dest     Outparameter that is set to the next 
        *                     position in the string.
        *   @param   val      Outparameter that is set to the value
        *                     that is read from the string.
        *   @return  True if val is set, false otherwise.
        */
      static bool getFloat64(const char* startPtr, 
                             char* &dest, 
                             float64 &val);


#ifndef _MSC_VER // We don't have Visual Studio
      /**
       *    Dump the buffer as hex.
       *    @param stream        The stream to print to (e.g. cerr or cout).
       *    @param data          The data to print as hex
       *    @param dataLength    The length of the data
       *    @param leadingString String to start each line of output with.
       */
      static void hexDump(ostream& stream,
                          byte* data,
                          unsigned int datalength,
                          const char* leadingString = NULL);
#endif

      /**
        *   @name Get commandline-parameters.
        *   Methods for getting the parameters on the command line.
        */
      //@{
         /**
           *   Check if a given parameter is present or not.   // -h (help)

           *
           *   @param   argc     The number of arguments in argv.
           *   @param   argv     Vector with the given parameters.
           *   @param   param    The parameter the caller is interested in.
           *   @param   startCheckingPos
           *                     Start checking this parameter (in order).
           *                     This parameter is optional, default value
           *                     is 0.
           *   @return  The index of the param, -1 is returned if not
           *            present.
           */
         static int parameterPresent(int argc, char* argv[], 
                                     const char* param, 
                                     int startCheckingPos = 0);
         
         /**
           *   Get the integer value of a specified parameter.
           *
           *   @param   argc     The number of arguments in argv.
           *   @param   argv     Vector with the given parameters.
           *   @param   param    The parameter the caller is interested in.
           *   @param   val      Outparameter that is set to the value of
           *                     param. If a value less than 0 is returned
           *                     then this value is not changed.
           *   @param   startCheckingPos
           *                     Start checking this parameter (in order).
           *                     This parameter is optional, default value
           *                     is 0.
           *   @return  The index of the param, -1 is returned of not
           *            present.
           */
         static int getParameterInt(int argc, char* argv[], 
                                    const char* param, int &val, 
                                    int startCheckingPos = 0);

         /**
           *   Get the string value of a specified parameter.
           *   If maxNbrCharachters > 1, then the result will always be 
           *   NULL-terminated.
           *
           *   @param   argc     The number of arguments in argv.
           *   @param   argv     Vector with the given parameters.
           *   @param   param    The parameter the caller is interested in.
           *   @param   result   Outparameter that is set to the value of
           *                     param. If a value less than 0 is returned
           *                     then this value is not changed.
           *   @param   maxNbrChars
           *                     The maximum number of characters that is
           *                     copied into the result. This parameter
           *                     isoptional, default value is MAX_UINT32.
           *   @param   startCheckingPos
           *                     Start checking this parameter (in order).
           *                     This parameter is optional, default value
           *                     is 0.
           *   @return  The index of the param, -1 is returned of not
           *            present.
           */
         static int getParameterStr(int argc, char* argv[], 
                                    const char* param, char* result, 
                                    int maxNbrChars = MAX_UINT32,
                                    int startCheckingPos = 0);

      //@}

      /**
        *   Get the compass (e.g North or East) of a given angle. The
        *   angle must be in degrees, calculated clockwise from the north
        *   direction.
        *   @param   degree   The direction in degrees.
        *   @param   res      The letter resolution from 1-3.
        *   @return  The compass degree direction in N, NW etc.
        */
      static StringCode getCompass(int degree, int res);

      /**
       *    @name Logical location.
       *    Methods to make it easier to deal with the logical location 
       *    in the items.
       */
      //@{
         /** 
           *   Get the municipal-part of a location. 
           *   @param   location   The location uint32.
           *   @return  The municipal part of the location. This is
           *            the same as the itemID for the municipal!
           */
         static uint8 getMunicipal( uint32 location ) {
            return uint8((location & 0xff000000) >> 24);
         }

         /** 
           *   Set the municipal-part of a location. 
           *   @param   municipal   The built up area part of the location.
           *   @param   location    The location uint32.
           */
         static void setMunicipal(uint8 municipal,
                                  uint32 &location ) {
            location = (location & 0x00ffffff) | 
                       (((uint32) municipal << 24) & 0xff000000);
         }
         
         /** 
           *   Get the built up area-part of a location. 
           *   @param   location    The location uint32.
           *   @return  The built up area part of the location.
           */
         static uint16 getBuiltUpArea( uint32 location ) {
            return uint16(location & 0x0000ffff);
         }

         /** 
           *   Set the built up area-part of a location. 
           *   @param   builtUpArea The built up area part of the location.
           *   @param   location    The location uint32.
           */
         static void setBuiltUpArea(uint16 builtUpArea, 
                                    uint32 &location ) {
            location = (location & 0xffff0000) | 
                       ((uint32) builtUpArea & 0x0000ffff);
         }
         
         /** 
           *   Get the city part-part of a location. 
           *   @param   location    The location uint32.
           *   @return  The built up area part of the location.
           */
         static uint8 getCityPart( uint32 location ) {
            return uint8((location & 0x00ff0000) >> 16);
         }

         /** 
           *   Set the built up area-part of a location. 
           *   @param   builtUpArea The built up area part of the location.
           *   @param   location    The location uint32.
           */
         static void setCityPart(uint8 cityPart, 
                                 uint32 &location ) {
            location = (location & 0xff00ffff) | 
                       (((uint32) cityPart << 16) & 0x00ff0000);
         }

      //@}

         /**
          * General method to check whether two zipcodes are close.
          * NB This method is more general than the closeZipCodes(), 
          * since it does not require the zipCode strings to be of 
          * equal length and it allows any alphanumeric character 
          * (not only numerical).
          * @param zc1 zipcode 1.
          * @param zc2 zipcode 2.
          * @param zipCodeDiff The tolerated difference.
          * @return True if the difference is within the tolerance.
          */
         static bool genCloseZipCodes(const char* zc1,
                                      const char* zc2,
                                      uint32 zipCodeDiff);

         /**
          * Checks whether the zipcodes are close.
          * @param zc1 zipcode 1.
          * @param zc2 zipcode 2.
          * @param zipCodeDiff The tolerated difference.
          * @param high The highest single digit of a zipcode.
          * @param low The lowest single digit of a zipcode.
          * @return True if the difference is within the tolerance.
          */
         static bool closeZipCodes(const char* zc1,
                                   const char* zc2,
                                   uint32 zipCodeDiff,
                                   int32 high,
                                   int32 low);

         /**
          * Calculates the distance between two zipcode strings.
          * @param zc1 zipcode 1.
          * @param zc2 zipcode 2.
          * @param low The lowest character occuring in a zipcode.
          * @param high The highest character occuring in a zipcode.
          * @return The distance.
          */
         static float64 zipCodeDistance(const char* zc1,
                                        const char* zc2,
                                        char low,
                                        char high);
         
         /**
          * Returns the cost for walking at a certain speed
          * in s/cm normalized to 1 byte. 
          * Implements a table defined in the design document.
          *
          * @param speed is the speed in km/h or mph.
          * @param tells if the speed is in km/h or mph.
          * @return the cost.
          */
         static uint8 getWalkRoutingCost( uint8 speed, 
                                          uint32 measureType );

         /**
          * Returns the cost for walking at a certain speed
          * in s/cm normalized to 1 byte. 
          * Implements a table defined in the design document.
          *
          * @param dist is the dist cost.
          * @param time is the time cost.
          * @param percent is the percent that the dist should be
          *                used and (100-percent) 
          * of time should be used.
          */
         static void getRoutingCost( uint8& dist, uint8& time, uint8 percent );

      /**
       *    Puts the names of the files in path. Will also list dirs.
       *    @param path The path to look for files in.
       *    @param entries The names of the directory entries will be
       *                   put here. Must be deleted by the caller.
       *    @return Number of directory entries.
       */
      static int directory(const char* path,
                           char*** entries);

      /**
       *    Deletes the array of strings pointed to by
       *    <code>strs</code>. strs will also be deleted.
       *    @param strs The array of strings.
       *    @param nbr  The number of strings.
       */
      static void deleteStrArray(char** strs, int nbr);

};

#endif // UTILITY_H
