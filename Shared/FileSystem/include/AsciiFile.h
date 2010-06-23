/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ASCIIFILE_H
#define ASCIIFILE_H

#include "File.h"

/**
  *   Class that describes a file used to read text from a file.
  *   Inside a cache is used to speed up the reading.
  *
  */
class AsciiFile : public File {
   public:
      /**
        *   Create a new AsciiFile with a specified filename.
        *   @param   filename The full name of the file.
        */
      AsciiFile(const char* filename );

      /**
        *   Deletes this AsciiFile. Closes the file.
        */
      virtual ~AsciiFile();

      /**
        *   Read one line from the file. The data is read into
        *   a preallocted buffer, given as argument.
        *   @param   buf      The buffer where the data is copied.
        *   @param   maxLen   The maximum nunber of bytes copied 
        *                     into the buffer.
        *   @param   eofchars The allowed end of line characters.
        *                     This parameter is optional, default
        *                     value is "\r\n" (means that the line
        *                     ends at the first occurance of "\r"
        *                     or "\n").
        *   @param   readAllEol If true (default) this function will
        *                       remove all endofline characters in sequence.
        *                       If false it will stop after the firs occurance.
        *   @return  The number of bytes read from file.
        */
      int readLine(char* buf, size_t maxLen, 
                   const char* eofchars = "\r\n",
                   bool readAllEol = true);

      /**
        *   Write to file.
        *   @param   buf   The buffer which contains the data to be written.
        */
      void write(const char* buf);

   private:
      /**
        *   The buffer used as cache for the file.
        */
      char* m_fileCache;

      /**
        *   The current position in the cache.
        */
      int m_cachePos;

      /**
        *   The number of bytes that is used of the cashe.
        */
      int m_usedCacheSize;

      /**
        *   The total size of the cashe (in bytes).
        */
      int m_cacheSize;

};

// ==================================================================
//                                Implementation of inlined methods =

#endif



