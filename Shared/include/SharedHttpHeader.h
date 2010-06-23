/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SHARED_HTTP_HEADER_H
#define SHARED_HTTP_HEADER_H

#include "config.h"
#include <string>
#include <vector>
#include "MC2SimpleString.h"

class SharedHttpHeader {

   private:
      /* Converts multiple whitespaces to single */
      void trimSpaces();

   public:

      /* Constructor */
      SharedHttpHeader();
      /* Destructor */
      ~SharedHttpHeader();

      /**
       *   Resets the header to start state.
       */
      void reset();
      
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
      int addBytes(const unsigned char* bytes, int len);

      /**
      *   Returns true if the header is fully read.
      */
      bool complete() const;

      /**
      *   Returns the value of the headerline
      *   matching the key. (Apart from the first line).
      *   @param key String to look for among the headers, e.g.
      *              "Content-Length". Shall not contain the ":".
      *   @return Value or NULL if not present.
      */
      const char* getHeaderValue(const char* key) const;

      /**
      *   Parses the first line containing
      *   "HTTP/1.X YYY STR" line and returns YYY.
      *   @return The status code of the first line of http-data.
      */
      const int getStatusCode() const;

   /* Variables required for state information. */
   private:
      /* This string holds the header after it is completed */
      string headerBuffer;
      /* IS true, if header is completely done */
      bool isComplete;
      
      /// Holds the header values to avoid the previous leak in getHeaderValue
      vector<char*> m_stringsToFree;
   
};

#endif
