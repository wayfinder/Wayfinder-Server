/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2_ITOA_H
#define MC2_ITOA_H

#include "config.h"
#include "MC2String.h"

/**
 * Functions for integers as string with bases up to and including 64.
 * Please note that bases 11-16 upper case, a-f, and that bases > 16 uses 
 * proprietary identifiers. 
 */
namespace MC2Itoa {

/**
 * Makes a string representation of the number in the selected base.
 *
 * @param nbr The number to make a string of.
 * @param base The base to use, valid values are 2-64.
 * @param padToSize Left pad with zeros if result is smaller than this size.
 * @return The string for the number.
 */
MC2String toStr( uint32 nbr, int base, uint32 padToSize = MAX_UINT32 );

/**
 * Make an integer from a number string.
 *
 * @param str The string to convert.
 * @param base The base to use, valid values are 2-64.
 * @return The number for the string.
 */
uint32 fromStr( const MC2String& str, int base );

} // End namespace MC2Itoa

#endif // MC2_ITOA_H
