/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2BOUNDINGBOX_BUFFER_H
#define MC2BOUNDINGBOX_BUFFER_H

class DataBuffer;
class MC2BoundingBox;

namespace MC2BoundingBoxBuffer {
      
/**
 * Reads an MC2BoundingBox from the databuffer. (4*4 bytes).
 * Doesn't update the cosLat factor.
 * @param buffer
 * @param bbox The bouding box to put the result into.
 * @return The number of bytes read.
 */
int readNextBBox( DataBuffer& buffer,
                  MC2BoundingBox& bbox );

/**
 * Writes a bounding box to the data buffer (4*4 bytes).
 * @param buffer
 * @param bbox The bounding box to write.
 * @return The number of bytes written.
 */
int writeNextBBox( DataBuffer& buffer,
                   const MC2BoundingBox& bbox );

} // MC2BoundingBoxBuffer

#endif // MC2BOUNDINGBOX_BUFFER_H

