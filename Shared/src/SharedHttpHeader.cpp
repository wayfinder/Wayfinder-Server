/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "SharedHttpHeader.h"

/* Line-ending characters */
#define CHAR_LF  '\n'
#define CHAR_CR  '\r'

/* Constructor */
SharedHttpHeader::SharedHttpHeader()
{
   isComplete = false;
   return;
}

/* Destructor */
SharedHttpHeader::~SharedHttpHeader()
{
   reset();
   isComplete = false;
   return;
}

void
SharedHttpHeader::reset()
{
   isComplete = false;
   headerBuffer = "";
   for ( vector<char*>::const_iterator it = m_stringsToFree.begin();
         it != m_stringsToFree.end();
         ++it ) {
      free(*it);
   }
   m_stringsToFree.clear();
}

/**
 *   Show the SharedHttpHeader the bytes that have been
 *   read from a socket. The SharedHttpHeader will return
 *   the number of bytes consumed. If the header is complete
 *   (i.e. the double CR/LF pair has been received) the
 *   SharedHttpHeader will consume any more bytes, they are
 *   part of the body of the message.
 *   @param bytes Bytes to add to the header.
 *   @param len   Max number of bytes to read into the header.
 *   @return Number of bytes consumed or -1 if an error occurs.
 */
int SharedHttpHeader::addBytes(const unsigned char* bytes, int len)
{
   /* Currently, reads all bytes of the header including the empty line */

   
   /* check if header is already read and done, */
   /* return Error, if done and no more bytes are required. */
   if( isComplete ) {
      return 0;
   }

   /* loop thru the string and add the characters to the buffer */
   for(int idx = 0; idx < len; idx++) {
      mc2dbg8 << "[SharedHttpHeader]: Byte = " << MC2CITE(bytes[idx])
              << endl;
      /* add the character to the local header buffer */
      headerBuffer.push_back(bytes[idx]);

      /* if newline, then check previous two chars for CR or LF */
      if( CHAR_LF == bytes[idx] ) {
         // Go back and check for another LF
         for( int i = headerBuffer.length()-2; i >= 0; --i ) {
            if ( CHAR_LF == headerBuffer[i] ) {
               // Trim spaces?
               mc2dbg << "[SharedHttpHeader]: Complete at ["
                      << idx << "]" << endl;
               isComplete = true;
               mc2dbg << "[SharedHttpHeader]: Header is "
                      << headerBuffer << endl;
               return idx + 1;
            } else if ( headerBuffer[i] == CHAR_CR ) {
               // Ignore
            } else {
               // Other character - not complete.
               break;
            }
         }
      }
   }

   return len;
}

/**
 *   Returns true if the header is fully read.
 */
bool SharedHttpHeader::complete() const
{
   return(isComplete);
}

/**
 *   Returns the value of the headerline
 *   matching the key. (Apart from the first line).
 *   @param key String to look for among the headers, e.g.
 *              "Content-Length". Shall not contain the ":".
 *   @return Value or NULL if not present.
 */
const char* SharedHttpHeader::getHeaderValue(const char* key) const
{
   /* get the string part which contains the key */
   char* srchStr = strstr( const_cast<char*>( headerBuffer.c_str() ), key);

   /* if key not found, return NULL */
   if(srchStr == NULL) return(NULL);

   char* token = NULL;
   const char* sepStr = ":\r\n";

   /* make a copy of the search buffer as it gets modified by */
   /* strtok() and use that for searching. */

   /* find end of the line */
   char* endIdx = strchr(srchStr, '\n');
   /* set it to zero temporarily */
   endIdx[0] = 0;
   /* copy it to a new string */
   char* newSrchStr = strdup(srchStr);
   /* set it back to newline */
   endIdx[0] = '\n';

   char *ptrptr = newSrchStr;
   /* bypass the identifier token */
   token = strtok_r( newSrchStr, sepStr, &ptrptr );
   /* next token gotten should be the correct value */
   token = strtok_r( NULL, sepStr, &ptrptr );

   if ( token != NULL ) {
      // Insert the value in the map.
      // UGLY, but if you consider const to be a function that returns the
      // same value everytime you call it is almost ok.
      const_cast<SharedHttpHeader*>(this)->
         m_stringsToFree.push_back( strdup( token ) );
   } else {
      free( newSrchStr );
      return NULL;
   }

   /* free the temporary string */
   free( newSrchStr );

   return m_stringsToFree.back();
}

/**
 *   Parses the first line containing
 *   "HTTP/1.X YYY STR" line and returns YYY.
 *   @return The status code of the first line of http-data.
 */
const int SharedHttpHeader::getStatusCode() const
{
   int statusCode = 0;
   sscanf(headerBuffer.c_str(), "HTTP/%*i.%*i %i", &statusCode);
   return(statusCode);
}

/* Converts multiple whitespaces to single */
void SharedHttpHeader::trimSpaces()
{
   string tmpStr;

   for(int i = 0; i < int(headerBuffer.length()); i++) {

      if(headerBuffer[i] == ' ') {
         if(headerBuffer[i-1] != ' ') {
            tmpStr.push_back(headerBuffer[i]);
         }
      }
      else {
         tmpStr.push_back(headerBuffer[i]);
      }
   }

   headerBuffer = tmpStr;

   return;
}
