/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2_MATH_H
#define MC2_MATH_H

//
// PI / 6
//
#define M_PI_6 0.523598775598

// Makes the code easier to read.
#define SQUARE(x) ((x)*(x))

// Precalculate 2^32 to make sure not to call pow(2, 32)...
#define POW2_32 4294967296.0

#ifdef _MSC_VER
#define rint(a)   ((a>0)?((int)(a+0.5)):((int)(a-0.5)))
#endif

namespace Math {

/**
 *    @name Constans (speeds)
 *   Multiplicative constants that should be used when 
 *   converting between speeds in <I>m/s</I> and <I>km/h</I>.
 */
//@{
/**
 *    From <I>km/h</I> to @$m/s@$.
 */
const float64 KMH_TO_MS = 0.2777777777777777;

/**
 *    From $m/s$ to $km/h$.
 */
const float64 MS_TO_KMH = 3.6;

//@}

/**
 *    Find out if a given number is odd or even.
 *    @param nbr  The number to test for oddity.
 *    @return True if the parameters bit 0 is set.
 */
inline bool odd( uint32 nbr ) {
   return (nbr & 1);
}

} // Math

#endif // MC2_MATH_H
