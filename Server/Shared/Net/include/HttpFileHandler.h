/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPFILEHANDLER_H
#define HTTPFILEHANDLER_H

// Includes
#include "Types.h"
#include "config.h"
#include "MC2String.h"
#include "NotCopyable.h"
#ifdef __linux
#include <sys/stat.h>
#endif
#ifdef WIN32
#include <stat.h>
// I think in WIN32
#endif


/**
 * HttpFileHandler - Handles files for the http-server.
 *
 * @version 1.0
 */
class HttpFileHandler: private NotCopyable {
   public:
      /**
       * Constructor. Uses the property HTML_ROOT.
       */
      HttpFileHandler();


      /**
       *  Removes allocated resources.
       */
      ~HttpFileHandler();
   

      /**
       * Delivers a file as a byte array. The user must delete the file.
       * @param fileString is the path and name of the file.
       * @param length is set to the length of the buffer.
       * @param status is set to the status of the file.
       * @return the file as a byte vector.
       */
      byte* getFile(const MC2String& fileString, 
                    int& length, 
                    struct stat* status);
      
   
      /**
       * Takes an extension and returns corresponding Content-Type.
       * Returns WAP 1.1 mime-types for WML and WMLS.
       * @param ext is the extension of the filetype requested.
       * @param file is a pointer to the file of the extension. 
       *        Only used for deferencing between wmlsc 1.0 and 1.1.
       * @return the type of file.
       */
      static const char* getFileType(const MC2String& ext, byte* file);
      

   private:
      /** 
       * The path to the html-root
       */
      MC2String* m_htmlRoot;
};
#endif // HTTPFILEHANDLER_H 
