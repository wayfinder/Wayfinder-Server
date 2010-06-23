/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NGPMAKER_REQUEST_H
#define NGPMAKER_REQUEST_H

#include "NParam.h"

/// \namespace Holds NGP utilities for making ngp binaries
namespace NGPMaker {

/// Describes an NGP Parameter
struct Param {
   uint32 m_id; ///< identification number
   NParam::NParam_t m_type; ///< param type
   MC2String m_value; ///< string value to be converted
   MC2String m_desc; ///< simple description of this parameter
};

/// Contains a request with all parameters
struct Request {
   uint32 m_protocolVersion; ///< protocol version
   uint32 m_type; ///< Request type
   uint32 m_id; ///< identification number
   uint32 m_version; ///< Version.
   bool m_useGzip; ///< whether or not to use gzip 
   vector<Param> m_params; ///< all parameters
   MC2String m_desc; ///< simple description of this request
   MC2String m_name; ///< name of the request
};

/**
 * Determines string representation of param type
 * @return string representation of type
 */
MC2String getTypeAsString( NParam::NParam_t type );

/**
 * Returns type that matches string
 * @param value string representation of type
 * @param type returns type value, if found
 * @return true if type was found and set
 */
bool getTypeFromString( const MC2String& value, 
                        NParam::NParam_t& type );

/**
 * Saves ngp request to a file
 * 
 * @param filename name of the file
 * @param request the request to write
 * @return true on success
 */
bool saveNGP( const MC2String& filename, const Request& request );

/**
 * Saves ngp request to a buffer
 * @param buffer to be filled with ngp data
 * @param request to be save
 * @return true on success
 */
bool saveNGPToBuffer( vector<byte>& buffer, const Request& request  );

}

#endif
