/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AsciiFile.h"

#include <string.h>


AsciiFile::AsciiFile(const char* filename)
   : File(filename)
{
   m_cacheSize = 65536;
   m_fileCache = new char[m_cacheSize];
   m_cachePos = 0;
   m_usedCacheSize = 0;
}

AsciiFile::~AsciiFile()
{
   delete [] m_fileCache;
}

void
AsciiFile::write(const char* buf)
{
   fputs(buf, m_file);
}


int
AsciiFile::readLine(char* data,
                    size_t maxNbrBytes,
                    const char* eolChars,
                    bool readAllEol)
{
   // Make sure the file is open and that we should read any 
   // characters
   if ((m_file == NULL) || (maxNbrBytes < 1))
      return (FILE_ERROR);

   // Check if we have to read all lines in the cache
   if (m_cachePos >= m_usedCacheSize) {
      // Check if we reached end of file
      if (feof(m_file))
         return (END_OF_FILE);

      // Read new strings into the cache
      
      // Read one block into the cache
      if ( fgets(m_fileCache, m_cacheSize-128, m_file) != NULL) {
         // This seems a bit slow?
         m_usedCacheSize = strlen(m_fileCache);

         // Read until end of line
         int readChar = 0;
         while ( (readChar != EOF) && 
                 (strchr(eolChars, m_fileCache[m_usedCacheSize-1]) == NULL)) {
            readChar = fgetc(m_file);
            m_fileCache[m_usedCacheSize] = (char) readChar;
            m_usedCacheSize++;
         }

         if ( readAllEol ) {
            // Read past all the end of line characters
            do {
               readChar = fgetc(m_file);
            } while (strchr(eolChars, (char) readChar) != NULL);
            ungetc(readChar, m_file);
         }         
         m_cachePos = 0;
      } else {
         // We get here on error or when no character have been read when
         // reading EOF.
         return 0;
      }
   }

   // Read the line from the cache
   uint32 totNbrRead = 0;
   do {
      data[totNbrRead] = m_fileCache[m_cachePos++];
   } while ( (strchr(eolChars, data[totNbrRead++]) == NULL) &&
             (totNbrRead < maxNbrBytes));

   if ( readAllEol ) {
      // Read past all the end of line characters in the file cache
      while ( (strchr(eolChars, m_fileCache[m_cachePos]) != NULL) && 
              (m_cachePos < m_usedCacheSize)) {
         m_cachePos++;
      }
   }

   // Terminate the string that should be returned
   data[MIN(totNbrRead-1, maxNbrBytes-1)] = '\0';

   // Return the number of read bytes
   return (totNbrRead);
}

/* // Alternative implementation, without filecache
int
AsciiFile::readLine(char* data, size_t maxNbrBytes, char* eolChars)
{
   // Make sure the file is open and that we should read any 
   // characters
   if ((m_file == NULL) || (maxNbrBytes < 1))
      return (FILE_ERROR);

   // Read the line
   size_t totNbrRead = 0;
   int readChar;
   do {
      readChar = fgetc(m_file);
      data[totNbrRead] = (char) readChar;
      //totNbrRead++;
   } while (readChar != EOF && 
            (strchr(eolChars, data[totNbrRead++]) == NULL) &&
            (totNbrRead < maxNbrBytes));

   // Read past all the end of line characters
   do {
      readChar = fgetc(m_file);
   } while (strchr(eolChars, (char) readChar) != NULL);
   ungetc(readChar, m_file);

   // Terminate the string that should be returned
   data[MIN(totNbrRead-1, maxNbrBytes-1)] = '\0';

   // Return the number of read bytes
   return (totNbrRead);
}
*/

