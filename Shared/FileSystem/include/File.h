/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FILE_H
#define FILE_H


#include "config.h"
#include "MC2String.h"
#include "NotCopyable.h"

#include <stdio.h>

/**
  *   File.
  *
  */
class File: private NotCopyable {
public:
   /**
    *   Create a new File with a specified filename.
    *   @param   filename The full name of the file.
    */
   explicit File( const char* filename );

   /**
    *   Deletes this File. Closes the file.
    */
   virtual ~File();

   /**
    *   @name The possible error codes of the read methods.
    *   All the errorcodes are less than zero, so that can
    *   be used as a test for success or failure.
    */
   //@{
   /// No more records in this relation
   static const int END_OF_FILE;

   /// Something is wrong with the file
   static const int FILE_ERROR;
   //@}

   /**
    *   @return  The name of the file for this relation.
    */
   inline const char* getFileName();

   /**
    *   Open the file in different modes
    *   @param   mode "r" for read mode \\
    *                 "w" for write mode \\
    *   @return  True upon succes, false otherwise.
    */
   bool open(const char* mode = "r");


   /**
    *   Close the file.
    *   @return  True upon succes, false otherwise.
    */
   bool close();

   /**
    * Set the current file position.
    */
   bool setPos( size_t pos );

   /**
    * Read a specified number of bytes from the file into buff.
    */
   bool read( vector<byte>& buff, size_t size );

   /// determines if the file is a regular file or a link
   /// @param filename the filename
   /// @return true if file exist and is either a regular file or a link
   static bool fileExist( const MC2String& fileName );

   static int mkdir_p( const MC2String& str );


   /**
    * Reads the contents of filename into buff.
    *
    * @param filename The file path to read.
    * @param buff The buffer to add read bytes to.
    * @return The number of bytes read or < 0 if error.
    */
   static int readFile( const char* filename, vector<byte>& buff );


   /**
    * Writes the contents of buff into filename.
    *
    * @param filename The file path to write.
    * @param buff The buffer to write to file.
    * @return The number of bytes written or < 0 if error.
    */
   static int writeFile( const char* filename, const vector<byte>& buff );

   /**
    * Writes the contents of buff into filename.
    *
    * @param filename The file path to write.
    * @param buff The buffer to write to file.
    * @param buffLen The size of buff.
    * @return The number of bytes written or < 0 if error.
    */
   static int writeFile( const char* filename, const byte* buff,
                         uint32 buffLen );

protected:
   /**
    *    Opens m_file to read from the program
    *    <code>program</code>.
    *    @param program Program to run.
    *    @param mode    Mode of program.
    *    @return True if the program could be opened.
    */
   bool openProgram(const MC2String& program, const char* mode);
      
   /**
    *   The name of the file for this realation.
    */
   char* m_filename;

   /**
    *   The file where to read the rawdata from, or write to.
    */
   FILE* m_file;
};

// ==================================================================
//                                Implementation of inlined methods =
const char*
File::getFileName()
{
   return (m_filename);
}

#endif



