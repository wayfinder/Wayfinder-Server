/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LOGBUFFER_H__
#define LOGBUFFER_H__

#include "config.h"
#include <iostream>

/**
 *   An iostream streambuf that prefixes each new line with the
 *   date and time and handles other things related to MC2 logging.
 *
 */
class LogBuffer: public std::streambuf {
public:
   /**
    *   Initializes the LogBuffer.
    */
   LogBuffer();

   /**
    *   Cleans up
    */
   virtual ~LogBuffer();

   /**
    *   Set an extra logging prefix
    *   @param prefix Pointer to prefix string
    */
   void setPrefix(const char* prefix);

protected:
   /**
    *  Notifies any registered handlers.
    */
   void notify();

   /**
    *  Called when there's a new character to output.
    *  @return ???
    */
   int overflow(int c);

   /**
    *  Called to read a character from the input, doesn't advance to
    *  the next character.
    *  @return ???
    */
   int underflow();

   /**
    *  Called to read a character from the input, advances to the
    *  next character.
    *  @return ???
    */
   int uflow();

   /**
    *  Called to sync internal state with external representation
    *  @return ???
    */
   int sync();

private:

   static const int BUFFERSIZE = 32768;

   /**
    *  Outputs the first half of the (date, time)
    *  @return 0 if OK, EOF if error
    */
   int putTime();

   /**
    *  Outputs the second half of the prefix (the level)
    *  @return 0 if OK, EOF if error
    */
   int putLevel();

   /**
    * Flag that's set to true when encounter a newline
    */
   bool m_newline;

   /**
    * Flag that's set when we're doing magic (handling our special
    * manipulators (info, warn, err)
    */
   bool m_magic;

   /**
    * The log level we're using
    */
   int m_level;

   /**
    * Our character buffer, needed for the Notifiers.
    */
   char* m_buffer;

   enum Sizes {
      TIME_BUFF_SIZE = 128
   };

   /**
    * Last timestamp
    */
   char m_timeBuf[TIME_BUFF_SIZE];

   /**
    * Last level string
    */
   char m_levelBuf[32];

   /**
    * Current position in the buffer.
    */
   int m_bufPos;

   /**
    * The prefix string
    */
   const char* m_prefix;
};


#endif // LOGBUFFER_H__
