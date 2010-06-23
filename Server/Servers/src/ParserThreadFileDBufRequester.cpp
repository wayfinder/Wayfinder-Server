/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ParserThreadFileDBufRequester.h"

#include "BitBuffer.h"
#include "TileMapParamTypes.h"

ParserThreadFileDBufRequester::
ParserThreadFileDBufRequester( const char* path ) :
   FileDBufRequester( NULL, path ),
   m_desc( STMFDParams( LangTypes::english, false ) ) {
   m_desc.setData();
}

bool ParserThreadFileDBufRequester::allowed( const MC2SimpleString& descr ) const {
   if ( TileMapParamTypes::isBufferHolder( descr ) ) {
      // Skip first character and run again.
      return allowed( descr.c_str() + 1 );
   }
   if ( TileMapParamTypes::isMapFormatDesc( descr.c_str() ) ) {
      return false;
   }
   if ( TileMapParamTypes::isBitmap( descr.c_str() ) ) {
      return false;
   }
   if ( ! TileMapParamTypes::isMap( descr.c_str() ) ||
        m_desc.allowedOnDisk( descr ) ) {
      return true;
   } else {
      return false;
   }
}

bool ParserThreadFileDBufRequester::allowed( const BitBuffer* buffer ) const {
   if ( buffer == NULL ) {
      return false;
   }
   if ( buffer->getBufferSize() == 0 ) {
      mc2dbg << "[PTH]: Empty buffer not allowed to be saved" << endl;
      MC2_ASSERT( buffer->getBufferSize() != 0 );
      return false;
   }
   return true;
}
   
bool ParserThreadFileDBufRequester::allowed( const MC2SimpleString& desc,
                                             const BitBuffer* buffer ) const {
   return allowed( desc ) && allowed( buffer );
}

void ParserThreadFileDBufRequester::release( const MC2SimpleString& descr,
                                             BitBuffer* buffer ) {
   mc2dbg8 << "[PTH]: release" << endl;
   if ( allowed( descr, buffer ) ) {
      FileDBufRequester::release( descr, buffer );
   } else {
      DBufRequester::release( descr, buffer );
   }
}
   
BitBuffer* ParserThreadFileDBufRequester::
requestCached( const MC2SimpleString& desc ) {
   if ( ! allowed( desc ) ) {
      return NULL;
   }
   mc2dbg8 << "[PTH]: request cached" << endl;
   BitBuffer* buf = FileDBufRequester::requestCached( desc );
   if ( buf != NULL && buf->getBufferSize() == 0 ) {
      mc2dbg << "[PTH]: removing empty buffer from cache " << desc
             << endl;
      removeBuffer( desc );
      delete buf;
      return NULL;
   }
      
   return buf;
}
