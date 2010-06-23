/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSERTHREADFILEDBUFREQUESTER_H
#define PARSERTHREADFILEDBUFREQUESTER_H

#include "config.h"

#include "FileDBufRequester.h"
#include "ServerTileMapFormatDesc.h"
#include "MC2SimpleString.h"

class BitBuffer;

/**
 * DBufRequester specially for Parserthread. It checks
 * which of the maps to save before saving.
 * Will not save traffic maps.
 * Moved from ParserThread.cpp
 */
class ParserThreadFileDBufRequester: public FileDBufRequester {
public:
   /**
    * @param path Path to disc cache.
    */
   ParserThreadFileDBufRequester( const char* path );

   /** 
    * Determine if the tile map param described by \c descr is allowed on disc
    * or not.
    * @return True if it is allowed on disc.
    */
   bool allowed( const MC2SimpleString& descr ) const;

   /** 
    * Determine if the buffer is valid enough to be saved on disc.
    * @return True if it is allowed on disc.
    */
   bool allowed( const BitBuffer* buffer ) const;

   /**
    * Determine if both tile map description \c desc and \c buffer is allowed
    * on disc.
    * @param desc The tile map description.
    * @param buffer The content of the tile map.
    * @return true if both are allowed on disc.
    */
   bool allowed( const MC2SimpleString& desc, const BitBuffer* buffer ) const;

   /// @copydoc FileDBufRequester::release(descr, obj)
   void release( const MC2SimpleString& descr, BitBuffer* buffer );

   /// @copydoc FileDBufRequester::requestCached(descr)
   BitBuffer* requestCached( const MC2SimpleString& desc );

private:
   ServerTileMapFormatDesc m_desc;
   
};

#endif // PARSERTHREADFILEDBUFREQUESTER_H
