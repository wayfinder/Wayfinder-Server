/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserEntityResolver.h"
#include <stdlib.h>

#ifdef USE_XML
#include <xercesc/util/XMLString.hpp>
#include <framework/MemBufInputSource.hpp> 

#include <iostream>
#include <fstream>
#include <fcntl.h>

#ifdef __linux
#include <sys/stat.h>
#endif
#ifdef WIN32
#include <stat.h>
// I think in WIN32
#endif

#ifdef __SVR4
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <unistd.h>
#endif


XMLParserEntityResolver::XMLParserEntityResolver() {
   // Read "isab-mc2.dtd"
   const char* fileName = "isab-mc2.dtd";
   struct stat status;
   m_isabmc2dtd = NULL;
   stat( fileName , &status );
   uint32 fSize = status.st_size;
   FILE* f = fopen( fileName, "rb" );
   if ( f != NULL ) {
      m_isabmc2dtdLength = fSize;
      m_isabmc2dtd = new byte[ m_isabmc2dtdLength ];
      if ( fread( m_isabmc2dtd, 1, m_isabmc2dtdLength, f ) 
           != m_isabmc2dtdLength )
      {
         mc2log << fatal << "XMLParserEntityResolver failed to read "
                << "all of \""
                << fileName <<  "\"!" << endl;
         exit ( 1 );
      }
      fclose( f );
   } else {
      mc2log << fatal << "XMLParserEntityResolver failed to open \""
             << fileName <<  "\"!" << endl;
      exit ( 1 );
   }
}


XMLParserEntityResolver::~XMLParserEntityResolver() {
   delete [] m_isabmc2dtd;
}

InputSource*
XMLParserEntityResolver::resolveEntity( const XMLCh* const publicId,
                                        const XMLCh* const systemId )
{
   InputSource* res = NULL;
   char* sysId = XMLString::transcode( systemId );
   if ( XMLString::compareString( sysId, "isab-mc2" ) == 0 ) {
      res = new MemBufInputSource( 
         m_isabmc2dtd, m_isabmc2dtdLength, "isab-mc2.dtd" );
   }
   delete [] sysId;

   return res;
}


#endif // USE_XML

