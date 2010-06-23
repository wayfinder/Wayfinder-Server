/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SFDHEADERLOADABLE_H
#define SFDHEADERLOADABLE_H 

#include "config.h"
#include "SFDSavableHeader.h"
#include "FileHandler.h"

class SharedBuffer;

/**
 *    SFDHeader that can be loaded from buffer.
 * 
 * Copied and modiefied from Nav2 on 20081111.
 */
class SFDLoadableHeader : public SFDSavableHeader {
public:
   /**
    *    Constructor.
    *
    *    @param name The name of the buffer.
    *    @param debug Initial value of debug, is overwritten in load.
    */ 
   SFDLoadableHeader( const MC2SimpleString& name, bool debug );
   
   /**
    *    Destructor.
    */
   virtual ~SFDLoadableHeader();

   /**
    *    Loads the SFDLoadableHeader.
    *    @param   Buff   The holder of the SFD data.
    */
   bool load( SharedBuffer& buf );

private:
   /// Encryption type.
   enum encryption_t {
      no_encryption        = 0,
      uid_encryption       = 1,
      warez_encryption     = 2,
   } m_encryptionType;

   /**
    *    Load the initial part of the header.
    */
   bool loadInitialHeader( SharedBuffer& buf );
   
   /**
    *    Load the remaining part of the header.
    */
   bool loadRemainingHeader( SharedBuffer& buf );
};


// --- Inlines ---

#endif
