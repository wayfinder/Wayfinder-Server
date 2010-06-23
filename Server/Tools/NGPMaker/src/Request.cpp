/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "../include/Request.h"

#include "NParamBlock.h"
#include "StringConvert.h"
#include "NavPacket.h"
#include "isabBoxNavMessage.h"
#include "STLStringUtility.h"

#include <fstream>

namespace NGPMaker {

struct ValueTranslation {
   const char* m_name;
   NParam::NParam_t m_type;
} s_valueTranslation[] = {
   { "Bool", NParam::Bool },
   { "Byte", NParam::Byte },
   { "Uint16", NParam::Uint16 },
   { "Uint32", NParam::Uint32 },
   { "Int32", NParam::Int32 },
   { "String", NParam::String },
   { "Byte_array", NParam::Byte_array },
   { "Uint16_array", NParam::Uint16_array },
   { "Uint32_array", NParam::Uint32_array },
   { "Int32_array", NParam::Int32_array }
};



MC2String getTypeAsString( NParam::NParam_t type ) {
   // find the type
   for ( uint32 i = 0; 
         i < sizeof ( s_valueTranslation ) / sizeof ( s_valueTranslation[ 0 ] ); 
         ++i ) {
      if ( s_valueTranslation[ i ].m_type == type ) {
         return s_valueTranslation[ i ].m_name;
      }
   }

   return "";
}

bool getTypeFromString( const MC2String& strval,
                        NParam::NParam_t& type ) {
   // find the type
   for ( uint32 i = 0; 
         i < sizeof ( s_valueTranslation ) / sizeof ( s_valueTranslation[ 0 ] ); 
         ++i ) {
      if ( strcasecmp( strval.c_str(), s_valueTranslation[ i ].m_name ) == 0 ) { 
         type = s_valueTranslation[ i ].m_type;
         return true;

      }
   }
   return false;
}

void addArrayType( NParam& param, NParam::NParam_t type,
                   const MC2String& values );
/** 
 * Adds a type and convert a string value to that type 
 * @param param the parameter to add type to
 * @param type the parameter specific type
 * @param strVal the value of the type as a string
 */
void addType( NParam& param, NParam::NParam_t type, const MC2String& strVal ) {
   using namespace StringConvert;

   switch ( type ) {
   case NParam::Byte:
      param.addByte( convert<uint32>( strVal ) );
      break;
   case NParam::Bool:
      param.addByte( convert<bool>( strVal ) );
      break;
   case NParam::Uint16:
      param.addUint16( convert<uint16>( strVal ) );
      break;
   case NParam::Uint32:
      param.addUint32( convert<uint32>( strVal ) );
      break;
   case NParam::Int32:
      param.addInt32( convert<int32>( strVal ) );
      break;
   case NParam::String:
      param.addString( strVal, false );
      break;
      // for the array types we need to split the strings
      // with sepearators and convert each item
   case NParam::Byte_array:
      mc2dbg << "Adding byte array." << endl;
      addArrayType( param, NParam::Byte, strVal );
      break;
   case NParam::Uint16_array:
      addArrayType( param, NParam::Uint16, strVal );
      break;
   case NParam::Uint32_array:
      addArrayType( param, NParam::Uint32, strVal );
      break;
   case NParam::Int32_array:
      addArrayType( param, NParam::Int32, strVal );
      break;
   }
}
void addArrayType( NParam& param, NParam::NParam_t type,
                   const MC2String& strVal ) {
   vector<MC2String> values = STLStringUtility::explode( ",", strVal );
   for ( uint32 i = 0; i < values.size(); ++i ) {
      mc2dbg << "Adding: " << values[ i ] << endl;
      addType( param, type, values[ i ] );
   }
}
/**
 * Adds parameter types to parameter block
 * @param block parameter block
 * @param params parameters to add
 */
void addTypes( NParamBlock& block,
               const vector<Param>& params ) {
   
   for ( uint32 i = 0; i < params.size(); ++i ) {
      const Param& reqParam = params[ i ];
      NParam& param = block.addParam( NParam( reqParam.m_id ) );
      addType( param, reqParam.m_type, reqParam.m_value );
   }
}

/**
 * Encrypt data using MAGICBYTES from isabBoxNavMessage.h
 * @param data the data to encrypt
 */
void encrypt( vector<byte>& data ) {
   // skip the first 10 bytes ( header )
   uint32 magicPos = 0;
   for ( uint32 i = 10 ; i < data.size() ; i++ ) {
      data[ i ] = data[ i ]  ^ MAGICBYTES[ magicPos ];
      magicPos++;
      if ( magicPos >= MAGICLENGTH ) {
         magicPos = 0;
      }
   }
}

/**
 * Writes request from block to outdata buffer
 * @param outdata buffer to write to
 * @param params the parameter block to write
 * @param req the request to write
 */
void writeParams( vector<byte>& outdata, const NParamBlock& params,
                  const Request& req  ) {

   // write parameter ids to block
   params.writeParams( outdata, req.m_protocolVersion, req.m_useGzip );
   
   // utilize nav request packet to write binary block
   // to buffer
   NavRequestPacket packet( req.m_protocolVersion, 
                            req.m_type, req.m_id, req.m_version,
                            &outdata.front(), outdata.size() );
   outdata.clear();
   packet.writeTo( outdata, req.m_useGzip );

   encrypt( outdata );
}

/**
 * Writes request with parameters to file
 * @param filename the name of the file
 * @param params parameter block to write
 * @param req request to write
 */
bool writeParams( const MC2String& filename, NParamBlock& params, 
                  const Request& req ) {

   vector<byte> outdata;
   writeParams( outdata, params, req );
   // write to file
   ofstream ngpfile( filename.c_str(), ios::binary );
   if ( ! ngpfile ) {
      return false;
   }

   ngpfile.write( (const char*)&outdata.front(), outdata.size() );

   return true;
}


bool saveNGP( const MC2String& filename,
              const NGPMaker::Request& req ) 
try {
   // write parameters to block
   NParamBlock paramBlock;
   addTypes( paramBlock, req.m_params );
   // write block to file
   return writeParams( filename, paramBlock, req );

} catch ( const StringConvert::ConvertException& e ) {
   mc2log << warn << e.what() << endl;
   return false;
}

bool saveNGPToBuffer( vector<byte>& buffer,
                      const NGPMaker::Request& req ) 
try {
   // write parameters to block
   NParamBlock paramBlock;
   addTypes( paramBlock, req.m_params );
   // write block to buffer
   writeParams( buffer, paramBlock, req );

   return true;

} catch ( const StringConvert::ConvertException& e ) {
   mc2log << warn << e.what() << endl;
   return false;
}

}
