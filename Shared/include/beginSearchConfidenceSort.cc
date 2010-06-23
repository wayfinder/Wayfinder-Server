/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "beginSearchAlphaSort.cc"
#undef COMP

/**
 *  Compares the elements a and b of the vector with the name v.
 *  @param a one elem
 *  @param b one more elem
 *  @return <0 if points(b)<points(a), >0 if points(b)>points(a),
 *          ALPHACOMP(A, B) if points(b)==points(a).
 */
#ifdef DEBUG_LEVEL_4
// special case when debugging:
// stabilize sort by sorting by location name as well.
// (not used (yet) in release mode since it is slower)
#define CONFCOMP(A, B) \
 (((tmp2 = ((SLELEM(B)->getPoints()) - \
            (SLELEM(A)->getPoints()))) == 0) \
  ? \
  ( ((tmp3 = (ALPHACOMP((A), (B)))) == 0) ? \
    (DEBUGCOMPARELOCATION((A), (B))) : tmp3 \
  ) : tmp2)
#else
// release version, sorts without regard to location
#define CONFCOMP(A, B) (((tmp2 = ((SLELEM(B)->getPoints().getPoints())) - \
                                  (SLELEM(A)->getPoints().getPoints())) == 0) \
                                  ? \
                                  ALPHACOMP((A), (B)) : tmp2)
#endif
#define COMP(A, B) (CONFCOMP(A, B))

