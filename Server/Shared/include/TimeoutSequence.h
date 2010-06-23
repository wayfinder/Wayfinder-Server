/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TIMEOUT_SEQUENCE_H
#define TIMEOUT_SEQUENCE_H

/**
 * Represents a sequence of timeouts, to be used for instance when
 * sending packets to modules, the first timeout can be 1 second, the next
 * 5 seconds etc.
 *
 * Currently only supports either a fixed timeout, or a sequence where
 * the first timeout differs from the rest.
 */
class TimeoutSequence {
public:
   /// Constructs a sequence of fixed timeouts
   TimeoutSequence( int constant );
   /// Constructs a sequence where the first timeout differs from the others
   TimeoutSequence( int first, int others );

   /**
    * Gets the timeout.
    * @param index The position in the sequence for which to get the timeout,
    *              the sequence starts at index 0.
    * @return Timeout number 'index'
    */
   int getTimeout( int index ) const;

private:
   int m_first;   ///< The first timeout
   int m_others;  ///< The other timeouts
};

#endif // TIMEOUT_SEQUENCE_H
