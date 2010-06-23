/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CRYPTO_BLOWFISH_H
#define CRYPTO_BLOWFISH_H

#include "config.h"
#include "MC2String.h"

#include <memory>

class DataBuffer;
/// contains crypt functions with blowfish crypt
namespace Blowfish {

/**
 * Encrypts a databuffer using blowfish crypt with cbc and puts a generated 
 * iv vector as the first 8 bytes in the final encrypted data.
 * @param key the password
 * @param data the data to be encrypted
 * @return pointer to encrypted databuffer
 */
std::auto_ptr<DataBuffer> encrypt( const char* key, const DataBuffer& data);

/**
 * Decrypts a databuffer using blowfish crypt with cbc and reads
 * iv vector as the first 8 bytes in the encrypted data.
 * @param key the password
 * @param data encrypted data that shall be decrypted
 * @return pointer to decrypted databuffer
 */
std::auto_ptr<DataBuffer> decrypt( const char* key, const DataBuffer& data );

/**
 * Encrypts a databuffer using blowfish crypt with cbc.
 * @param key a password
 * @param iv initialization vector
 * @param data the data to encrypt
 * @return pointer to encrypted databuffer
 */
std::auto_ptr<DataBuffer> encrypt( const char* key, const char* iv,
                                   const DataBuffer& data );

/**
 * Decrypts a databuffer using blowfish crypt with cbc.
 * @param key a password
 * @param iv initialization vector
 * @param data the data to encrypt
 * @return pointer to encrypted databuffer
 */
std::auto_ptr<DataBuffer> decrypt( const char* key, const char* iv, 
                                   const DataBuffer& data );

/**
 * Contains a "special" blowfish crypt for PagesJaunes.
 * These functions will not work with normal openssl programs.
 * 
 */
namespace PJ {

/**
 * @param key password
 * @param iv initialize vector
 * @param str the string to encrypt
 * @return encrypted string, base64 encoded.
 */
MC2String encrypt( const MC2String& key, const vector<char>& iv,
                   const MC2String& str );
}

}


#endif
