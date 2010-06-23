/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "config.h"

class FileHandlerListener;

class FileHandler {
public:

   /**
    *   Destroys the file handler and closes the file.
    */
   virtual ~FileHandler() {}

   /**
    *   Clears the file to zero length.
    */
   virtual void clearFile() = 0;
   
   /**
    *   Reads maxLength bytes from the file.
    *   Calls the listener when done. If listener is NULL the
    *   call will by synchronous and the number of bytes written
    *   will be returned.
    */
   virtual int read( uint8* bytes,
                     int maxLength,
                     FileHandlerListener* listener ) = 0;

   /**
    *   Sets the read and write positions of the stream to the sent-in value.
    *   -1 should set the position to end of file.
    */
   virtual void setPos( int pos ) = 0;

   /**
    *   Returns the read/write position in the stream.
    */
   virtual int tell() = 0;
   
   /**
    *   Writes bytes to the file of the FileHandler.
    *   Calls the listener when it is done.
    *   If listener is NULL the
    *   call will by synchronous and the number of bytes written
    *   will be returned.
    */
   virtual int write( const uint8* bytes,
                      int length,
                      FileHandlerListener* listener ) = 0;

   /**
    *   Returns the modification date of the file.
    *   Currently it is enough if the date is less for
    *   old files and more for new files.
    */
   virtual uint32 getModificationDate() const = 0;

   /**
    *   Returns the amount of available space on the drive
    *   where the file is located.
    */
   virtual uint32 getAvailableSpace() const = 0;

   /**
    *   Cancels outstanding reads/writes.
    */
   virtual void cancel() = 0;
   
   /**
    *   Returns the size of the file.
    */
   virtual int getFileSize() {
      // Default implementation
      int oldPos = tell();
      setPos(-1); // Move to the end of the file      
      int retVal = tell();
      setPos( oldPos );
      return retVal;
   }

};

class FileHandlerListener {
public:
   /**
    *   Called when the read is done.
    *   @param nbrRead The number of bytes read. Negative value for failure.
    */
   virtual void readDone( int nbrRead ) = 0;

   /**
    *   Called when the write is done.
    *   @param nbrWritten The number of bytes written. Negative for failure.
    */
   virtual void writeDone( int nbrWritten ) = 0;
   
   
};

#endif
