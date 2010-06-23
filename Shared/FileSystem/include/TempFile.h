/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TEMPFILE_H
#define TEMPFILE_H

#include "MC2String.h"
#include "NotCopyable.h"

/**
 * Creates a temporary file and unlinks or renames it on destruction.
 * Note: Should inherit from class File once that class is complete.
 */
class TempFile: private NotCopyable {
public:
   /**
    * Creates a temp file with specific path and destination file.
    * @param tempPrefixFilename the filename that will have XXXXXX appended.
    * @param tempPath the path name for tempPrefixFilename.
    * @param realFilename the real filename the tempfile will be renamed to.
    * @param realPath the path name for realFilename.
    */
   TempFile( const MC2String& tempPrefixFilename,
             const MC2String& tempPath,
             const MC2String& realFilename = MC2String(),
             const MC2String& realPath = MC2String() );
   /// unlinks or renames the tempfilename depeding on ok() and m_failed.
   ~TempFile();

   /// @return file descriptor to temp file.
   int getFD() const { return m_fd; }
   /// @return true if temp file was successfully created.
   bool ok() const { return getFD() != -1; }
   /// @return temp filename including path.
   const MC2String& getTempFilename() const { return m_tempFilename; }
   /// @return real filename including path.
   const MC2String& getRealFilename() const { return m_realFilename; }
   /// @return file size
   uint32 getSize() const;
   /// @return current offset
   uint32 getCurrentOffset() const;

   /**
    * Sets failed value, if failed = true the temp file will be unlinked
    * instead of renamed.
    * @param value the new failed value
    */
   void setFailed( bool value ) { m_failed = value; }

   /**
    * Removes the temp file from disk.
    * @return 0 on success else -1 on failure and errno set.
    */
   int unlink();

   /**
    * Write buffer to temp file.
    * @param buff the buffer to use
    * @param size the size of the buffer to write
    * @return size of buffer written. -1 on failure with errno set.
    */
   int write( const char* buff, uint32 size );

   /**
    *   @see write
    */
   int write( const unsigned char* buff, uint32 size ) {
      return write( reinterpret_cast<const char*>(buff), size );
   }

   /**
    * Read data to buffer from temp file.
    * @param buff the buffer to store values in
    * @param size the desired read size
    * @return size of buffer read, -1 on failure with errno set.
    */
   int read( char* buff, uint32 size );

   /**
    *   @see read
    */
   int read( unsigned char* buff, uint32 size ) {
      return read( reinterpret_cast<char*>(buff), size );
   }
   
   /**
    * Reposition file offset @see lseek(2)
    * @param offset offset in bytes from whence directive
    * @param whence One of the following SEEK_SET, SEEK_END, SEEK_CUR. 
    * @return offset in bytes from the begining of the file else -1 on
    * failure with errno set.
    */
   int seek( int offset, int whence );

   /// Syncs data. @see fsync(2)
   int sync();
   /// Closes the file descriptor. @see close(2)
   int close();

private:
   /**
    * Syncs data to disc and closes the file descriptor
    */
   void syncAndClose();

   TempFile();

   MC2String m_tempFilename; //< including the path
   MC2String m_realFilename; //< including the path
   int m_fd; //< file descriptor of temp filename
   bool m_failed; //< unlink if failed else rename in destructor
};

#endif // TEMPFILE_H
