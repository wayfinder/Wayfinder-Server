/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SMSUTIL4711_H
#define SMSUTIL4711_H
#include "Types.h"

/**
 *    SMSUtil.
 *
 */
class SMSUtil {
   public:
      /**
       *    Converts the data in dataLong to 8 bit data in dataShort.
       *    @param dataShort   
       *    @param dataLong    
       *    @param payLoadLen  
       */
      static int putEncoded120x8(uint8* dataShort,
                                uint8* dataLong,
                                int payloadLen);

      /**
       *    Converts the data in dataShort to 7 bit data in dataLong.
       *    @param dataShort   
       *    @param dataLong    
       *    @param count       
       */
      static int getDecoded160x7(uint8* dataLong,
                                uint8* dataShort,
                                int count);

      /**
       *    Fills the outbuffer with the concatenation header.
       *    @param outBuffer   
       *    @param pos         
       *    @param smsNbr      
       *    @param nbrParts    
       *    @param partNbr     
       */
      static int createConcatenatedHeader( char* outBuffer,
                                          int& pos,
                                          int smsNbr,
                                          int nbrParts,
                                          int partNbr);
};

#endif

