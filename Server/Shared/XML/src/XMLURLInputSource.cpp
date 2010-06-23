/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLURLInputSource.h"

#include "XMLDataBufferInputStream.h"
#include "URLFetcher.h"
#include "DataBuffer.h"
#include "FileException.h"

#include <memory>

namespace XMLTool {
URLInputSource::URLInputSource( const URL& url ):
   m_url( url ) {

}


URLInputSource::~URLInputSource() {
}

BinInputStream* URLInputSource::makeStream() const {
   // if file protocol then open local file
   if ( m_url.isValid() &&
        strcasecmp( m_url.getProto(), "file" ) == 0 ) {
      auto_ptr<DataBuffer> buf( new DataBuffer() );
      if ( ! buf->memMapFile( m_url.getPath(), 
                              false,  // no readWrite
                              false ) ) { // dont exit on failure
         throw FileUtils::
            FileException( MC2String("[URLInputSource] Failed to open file: ") +
                           m_url.getPath() );
      }
      return new DataBufferInputStream( buf.release() );
   }

   // else we must fetch it using http or https
   URLFetcher::dbPair_t ret = URLFetcher().get( m_url );
   if ( ret.first < 0 ) {
      return NULL;
   }

   return new DataBufferInputStream( ret.second );
}

}
