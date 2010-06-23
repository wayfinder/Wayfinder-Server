/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

class MC2Coordinate;

class ConverterSystem34 {
public:
   /*
    *   Converts from System34 or System45 to MC2.
    */
   static MC2Coordinate convertSystem34ToMC2( double x, double y );

   /**
    *   Converts from System34 or System45 to MC2.
    *   Adapted for use in coordinate transformer.
    *   in3 is ignored and out3 is set to zero.
    */
   static void convertSystem34ToMC2( float64 in1,
                                     float64 in2,
                                     float64 in3,
                                     float64& out1,
                                     float64& out2,
                                     float64& out3);
   
   /**
    *   Converts from MC2 to System34 or System45
    */
   static void convertMC2ToSystem34( float64 in1,
                                     float64 in2,
                                     float64 in3,
                                     float64& out1,
                                     float64& out2,
                                     float64& out3); 
  
private:
   
};
