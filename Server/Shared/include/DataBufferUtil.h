/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DATABUFFERUTIL_H
#define DATABUFFERUTIL_H

#include "MC2String.h"
#include <memory>

class DataBufferObject;
class DataBuffer;
class BitBuffer;


namespace DataBufferUtil {

/**
 * Saves data buffer to file. Will throw exception if write to file fails.
 * @param buff the data buffer
 * @param fd the file descriptor
 */
void saveBuffer( const DataBuffer& buff, int fd ) throw (MC2String);
/**
 * saves a data buffer object. Will throw exception if write to file fails.
 * @param obj the data buffer object
 * @param fd the file descriptor
 */
void saveObject( const DataBufferObject& obj, int fd ) throw (MC2String);

/**
 * Converts a DataBuffer to a BitBuffer
 * If mem mapped the buffer is copied and inparamter is unchagned.
 * If not mem mapped the buffer is moved and the returned buffer
 * owns the buffer.
 *
 * @param buffer In buffer
 * @return A BitBuffer
 */
std::auto_ptr< BitBuffer> convertToBitBuffer( DataBuffer& buffer );

}

#endif // DATABUFFERUTIL_H
