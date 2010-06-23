/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PIPE_H
#define PIPE_H

#include "config.h"
#include "FileException.h"
#include "NotCopyable.h"
#include <memory>

class Selectable;

/**
 * Constructs a pipe. Creates a pair of file descriptors,
 * one for reading and one for writing.
 */
class Pipe: private NotCopyable {
public:
   /// @throws FileException on error
   Pipe() throw (FileUtils::FileException);
   virtual ~Pipe();
   /**
    * writes to the pipe.
    * @param buff the buffer to write.
    * @param count number of bytes in buffer to write.
    * @return -1 on error, else number of bytes written.
    * @see man pages for write.
    */
   int write( const char* buff, size_t count );
   /**
    * reads from the pipe.
    * @param buff the buffer to store data in.
    * @param count number of bytes to read.
    * @return -1 on error, else number of bytes read.
    * @see man pages for read
    */
   int read( const char* buff, size_t count );
   /// @return the selectable to write file descriptor
   const Selectable& getWriteSelectable() const;
   /// @return the selectable to read file descriptor
   const Selectable& getReadSelectable() const;
private:
   auto_ptr<Selectable> m_fd[2]; //< selectables for read and write
};

#endif
