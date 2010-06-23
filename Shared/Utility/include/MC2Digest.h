/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2_DIGEST_H
#define MC2_DIGEST_H

#include "config.h"
#include "MC2String.h"
#include <memory.h>


/**
 * Class for making digest, hashes, using a specified digest method.
 */
class MC2Digest {
public:

   /**
    * Constructor.
    *
    * @param digestName The name of the digest to use, like "DSS1" or "dss1" or
    *                   "DSA-SHA1" or "dsaWithSHA1". Another example:
    *                   "MD5" or "md5".
    */
   MC2Digest( const MC2String& digestName );

   /**
    * Destructor.
    */
   virtual ~MC2Digest();

   /**
    * Encrypts a string.
    *
    * @param data The string to make a digest for.
    * @param digest The digest as .
    * @return True if made digest, false if not.
    */
   bool digest( const MC2String& data, MC2String& digest );

private:

   /// The hidden implementation stuff in this struct.
   struct Impl;

   /// Implementation hidden.
   auto_ptr<Impl> m_impl; 
};

#endif // MC2_DIGEST_H

