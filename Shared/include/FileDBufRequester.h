/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FILEDATABUFFERREQUESTER_H
#define FILEDATABUFFERREQUESTER_H

#include "config.h"
#include "DBufRequester.h"
#include "MC2SimpleString.h"

#include<map>

/**
 *   Class requesting databuffers from file and a parent
 *   requester.
 */
class FileDBufRequester : public DBufRequester {
public:
   /**
    *   Creates a new FileDBufRequester.
    *   @param parent Who to ask if the buffer isn't in file.
    *   @param path   The path to the directory 
    */
   FileDBufRequester(DBufRequester* parent,
                     const char* path);

   /**
    *   Deletes all stuff.
    */
   virtual ~FileDBufRequester();
   
   /**
    *   Makes it ok for the Requester to delete the BitBuffer
    *   or to put it in the cache. Requesters which use other
    *   requesters should hand the objects back to their parents
    *   and requesters which are top-requesters should delete them.
    *   It is important that the descr is the correct one.
    */ 
   void release(const MC2SimpleString& descr,
                BitBuffer* obj);

   /**
    *   Saves the buffer to disk without deleting it.
    */
   void saveToCacheNoDelete( const MC2SimpleString& descr,
                             BitBuffer* obj );

   /**
    * The same method as release but with the extra knowledge that the
    * buffer is from requestCached.
    */ 
   void releaseCached( const MC2SimpleString& descr,
                       BitBuffer* obj );

   /**
    *   If the DBufRequester already has the map in cache it
    *   should return it here or NULL. The BitBuffer should be
    *   returned in release as usual.
    *   @param descr Key for databuffer.
    *   @return Cached BitBuffer or NULL.
    */
   BitBuffer* requestCached(const MC2SimpleString& descr);

protected:
   /**
    *   Deletes all occurances of the buffer associated
    *   with the descr.
    */
   void internalRemove(const MC2SimpleString& descr );

private:

   /// Writes the buffer to file.
   bool writeToFile(const MC2SimpleString& descr,
                    BitBuffer* buffer);
   
   /// Returns a string with the filename for the descr.
   MC2SimpleString getFileName(const MC2SimpleString& descr);
   
   /// Read a buffer from file.
   BitBuffer* readFromFile(const MC2SimpleString& descr);
   
   /// The path to the dir of the files
   MC2SimpleString m_path;
   
};


#endif
