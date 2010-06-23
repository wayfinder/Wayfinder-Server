/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WRITEABLE_H
#define WRITEABLE_H

#include "config.h"


/**
 *   Abstract class that can be used to write to e.g. file or
 *   TCPSocket without knowing which it is.
 *
 */
class Writeable {
   public:
      /**
       *   Virtual destructor.
       *   Should close and delete the containing resource.
       */
      virtual ~Writeable() {};
   

      /**
       *   Tries to copy <code>size</code> bytes from <code>buffer</code>.
       *
       *   @param buf The buffer to copy from.
       *   @param size The number of bytes to write.
       *   @return The number of bytes written or a negative number.
       */
      virtual ssize_t write(const byte* buffer, size_t size ) = 0;


      /**
       *   Tries to copy <code>size</code> bytes from <code>buffer</code>.
       *
       *   @param buf The buffer to copy from.
       *   @param size The number of bytes to write.
       *   @param timeout The timeout. May or may not be implemented.
       *   @return The number of bytes written or a negative number if
       *           failure or timeout.
       */
      virtual ssize_t write( const byte* buffer, size_t size, 
                             uint32 timeout ) = 0;
};

#endif // WRITEABLE_H

