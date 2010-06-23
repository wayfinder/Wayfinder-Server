/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef KEY_ENCRYPTOR_DECRYPTOR_H
#define KEY_ENCRYPTOR_DECRYPTOR_H

#include "config.h"
#include "MC2String.h"
#include <memory.h>


/**
 * Abstract super for encryptors using a fix key.
 */
class KeyEncryptorDecryptor {
public:

   /**
    * Constructor.
    */
   KeyEncryptorDecryptor();

   /**
    * Destructor.
    */
   virtual ~KeyEncryptorDecryptor();

   /**
    * Encrypts a string.
    *
    * @param inID  The string to encrypt.
    * @param outID The encrypted string in base64 format.
    * @return True if encryption succeeded and outID is set, false if not.
    */
   bool encrypt( const MC2String& inID, MC2String& outID );

   /**
    * Decrypts a string.
    *
    * @param inID  The string to decrypt in base64 format.
    * @param outID The decrypted string.
    * @return True if decryption succeeded and outID is set, false if not.
    */
   bool decrypt( const MC2String& inID, MC2String& outID );

protected:

   /**
    * Get the key to use.
    * Subclasses must implement this and return the key to use.
    */
   virtual const unsigned char* getKey() = 0;

   /**
    * Get the initialization vector to use.
    * Subclasses must implement this and return the initialization vector 
    * to use.
    */
   virtual const unsigned char* getIV() = 0;

private:

   /// The hidden implementation stuff in this struct.
   struct Impl;

   /// Implementation hidden.
   auto_ptr<Impl> m_impl; 
};

#endif // KEY_ENCRYPTOR_DECRYPTOR_H

