/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2STRINGSTREAM_H
#define MC2STRINGSTREAM_H

#include "config.h"

#if defined (__GNUC__) && __GNUC__ > 2
# define USE_STRING_STREAM
#else
# undef  USE_STRING_STREAM
#endif

#ifdef USE_STRING_STREAM
#include <sstream>
#else
#include <strstream>
#endif

/**
 * Systemindependant string streamer.
 *
 */
class mc2stringstream : public
#ifdef USE_STRING_STREAM
stringstream
#else
strstream
#endif
{
   public:
      /**
       * Constructor.
       */
      mc2stringstream();

      /**
       * De-constructor.
       */
      ~mc2stringstream();
   private:
};

// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -

inline mc2stringstream::mc2stringstream() {
#ifdef USE_STRING_STREAM
//stringstream
#else
//strstream
#endif
}

inline mc2stringstream::~mc2stringstream() {
#ifdef USE_STRING_STREAM
#else
   freeze(0);
#endif
}

#endif // MC2STRINGSTREAM_H

