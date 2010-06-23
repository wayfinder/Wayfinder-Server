/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NPARAMBLOCK_H
#define NPARAMBLOCK_H

#include "config.h"
#include "NParam.h"
#include "NParamUtility.h"
#include <set>



/**
 * Class that holds a number of NParams. Can parse a byte buffer with
 * parameter data.
 * 
 */
class NParamBlock {
   public:
      /**
       * Creates an empty block.
       */
      NParamBlock();


      /**
       * Creates an block from a byte buffer.
       */
      NParamBlock( const byte* buff, uint32 len, byte protVer,
                   uint32* uncompressedSize = NULL );


      /**
       * Get the NParam for paramID.
       */
      const NParam* getParam( uint16 paramID ) const;


      /**
       * Get all NParams with paramID. Adds all params with paramID
       * to params.
       */
      void getAllParams( uint16 paramID, 
                         vector< const NParam* >& params ) const;


      /**
       * Writes params to byte array.
       */
      void writeParams( vector< byte >& buff, byte protVer,
                        bool mayUseGzip = false,
                        uint32* uncompressedSize = NULL ) const;


      /**
       * Add a NParam. Is copied and reference is returned.
       * To add a large parameter it is best to first call addParam then
       * addByteArray on the returned param. This avoids copying the big
       * buffer.
       */
      NParam& addParam( const NParam& param );


      /**
       * Update or add a param.
       *
       */
      NParam& updateParam( const NParam& param );


      /**
       * Removes the NParam for paramID.
       */
      void removeParam( uint16 paramID );


      /**
       * If parsing from byte buffer went ok.
       */
      bool getValid() const;


      /**
       * Dump params to out.
       */
      void dump( ostream& out, bool dumpValues = false, 
                 bool singleLine = false, uint32 maxLen = 35,
                 uint32 maxSame = 2 ) const;

      /**
       * If supports gzip flags is set.
       */
      bool mayUseGzip() const;

      /**
       * Reads NavRequest params from str.
       */
      bool readParams( const char* str, NParamBlock& params, byte& protoVer,
                       uint16& type, byte& reqID, byte& reqVer );

      /// Max size ever from client
      static const uint32 maxMsgSize;


   private:
      typedef multiset<NParam, NParamCmpLess> NParams;


      /// The params
      NParams m_params;


      /// The find param
      mutable NParam m_find;


      /// If parsing from byte buffer went ok.
      bool m_valid;


      /// The flags
      uint32 m_flags;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline bool 
NParamBlock::getValid() const {
   return m_valid;
}


#endif // NPARAMBLOCK_H


