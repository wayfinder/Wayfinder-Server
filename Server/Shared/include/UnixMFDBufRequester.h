/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UNIXMFDBUFREQUESTER_H
#define UNIXMFDBUFREQUESTER_H

#include "config.h"
#include <stdio.h>
#include "MC2SimpleString.h"
#include "FileHandler.h"
#include "MultiFileDBufRequester.h"

// -- UnixFileHandler

class UnixFileHandler : public FileHandler {
public:
   /**
    *   Creates a filehandler that handles the supplied file.
    *   Should create the file if it is not there already.
    */
   UnixFileHandler( const char* fileName );

   /**
    *   Destroys the file handler and closes the file.
    */
   virtual ~UnixFileHandler();

   /**
    *   Clears the file to zero length.
    */
   void clearFile();
   
   /**
    *   Reads maxLength bytes from the file.
    *   Calls the listener when done.
    */
   virtual int read( uint8* bytes,
                     int maxLength,
                     FileHandlerListener* listener );

   /**
    *   Sets the read and write position of the stream to the sent-in value.
    */
   void setPos( int pos );

   /**
    *   Returns the position in the stream.
    */
   int tell();

   /**
    *   Returns the modification date of the file or MAX_UINT32 if error.
    */
   uint32 getModificationDate() const;
   
   /**
    *   Writes bytes to the file of the FileHandler.
    *   Calls the listener when it is done.
    */
   virtual int write( const uint8* bytes,
                      int length,
                      FileHandlerListener* listener );
   /**
    *   Returns the amount of available space on the drive
    *   where the file is located.
    *   Currently returns a lot.
    */
   uint32 getAvailableSpace() const;

   /**
    *   Cancels outstanding reads/writes.
    */
   virtual void cancel() {}
   
private:
   MC2SimpleString m_fileName;
   FILE* m_file;
   /// Variable to keep track so that no functions are called before done.
   int m_working;
};

// -- UnixMFDBufRequester

class UnixMFDBufRequester : public MultiFileDBufRequester {
public:
   UnixMFDBufRequester(DBufRequester* parent,
                       const char* path,
                       uint32 maxSize,
                       int nbrFiles = 5) : MultiFileDBufRequester(parent,
                                                                  path,
                                                                  maxSize,
                                                                  nbrFiles) {
      init();
   }

   FileHandler* createFileHandler(const char* fileName) {
      return new UnixFileHandler( fileName );
   }

   const char* getPathSeparator() const {
      return "/";
   }
   
};

#endif

