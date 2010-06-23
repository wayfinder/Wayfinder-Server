/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavKeyedDataHandler.h"
#include "NavPacket.h"
#include "NavMessage.h"
#include "Properties.h"
#include "File.h"
#include "MC2Exception.h"
#include "MC2CRC32.h"
#include <limits>

/// Help functions
namespace {

/**
 * Returns the file path for a key and language.
 */
MC2String pathForKey( const MC2String& key,
                      LangTypes::language_t language ) {
   return MC2String( Properties::getProperty( "KEYED_DATA_PATH",
                                              "./KeyedData" ) ) + "/" 
      + key + "." + LangTypes::getLanguageAsISO639( language, false );
}

/**
 * Fetches the data associated with a key and language.
 * @throw an MC2Exception if there is no data associated with the key.
 */
void getDataForKey( const MC2String& key,
                    LangTypes::language_t language,
                    vector<byte>& data ) {

   MC2String primaryPath( pathForKey( key, language ) );
   MC2String fallbackPath( pathForKey( key, LangTypes::english ) );

   if ( File::fileExist( primaryPath ) ) {
      File::readFile( primaryPath.c_str(), data );
   }
   else if ( File::fileExist( fallbackPath ) ) {
      mc2log << warn 
             << "There was no data for the requested key and language!"
             << endl;
      mc2log << warn << "Using english instead." << endl;

      File::readFile( fallbackPath.c_str(), data );
   }
   else {
      throw MC2Exception( "Couldn't find file for key " + key 
                          + " and language " 
                          +  LangTypes::getLanguageAsISO639( language, 
                                                             false ) );
   }
}

/**
 * Adds the data to the param block, splitting it up into several
 * parameters if it's too big for just one.
 */
void addDataAsParams( NParamBlock& rparams, const vector<byte>& data ) {
   const size_t MAX_PARAM_SIZE = std::numeric_limits<uint16>::max();

   const byte* pos = &data.front();
   size_t bytesLeft = data.size();

   while ( bytesLeft > 0 ) {
      int bytesToAdd = std::min( bytesLeft, MAX_PARAM_SIZE );

      rparams.addParam( NParam( 6102, pos, bytesToAdd ) );

      pos += bytesToAdd;
      bytesLeft -= bytesToAdd;
   }
}

/**
 * Just logs info about the request to mc2log.
 */
void logParameters( const MC2String& key,
                    LangTypes::language_t language,
                    uint32 crc,
                    bool sentCRC ) {
   mc2log << info << "Get Keyed Data parameters:" 
          << " key: " << key
          << " langugage: " << LangTypes::getLanguageAsString( language );

   if ( sentCRC ) {
      mc2log << " crc: " << crc;
   }
 
   mc2log << endl;
}

}

NavKeyedDataHandler::NavKeyedDataHandler( InterfaceParserThread* thread,
                                          NavParserThreadGroup* group ) 
      : NavHandler( thread, group ) {
   m_expectations.push_back( ParamExpectation( 6100, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 6101, NParam::Uint32 ) );
}

NavKeyedDataHandler::~NavKeyedDataHandler() {
}

bool NavKeyedDataHandler::
handleGetKeyedData( LangTypes::language_t language,
                    NavRequestPacket* pack, 
                    NavReplyPacket* reply ) 
try {
   if ( !checkExpectations( pack->getParamBlock(), reply ) ) {
      return false;
   }

   MC2String key;
   uint32 crc;
   bool sentCRC;

   getParameters( pack->getParamBlock(), key, crc, sentCRC );

   logParameters( key, language, crc, sentCRC);

   vector<byte> data;
   getDataForKey( key, language, data );

   uint32 newCrc = MC2CRC32::crc32( &data.front(), data.size() );

   if ( !sentCRC || crc != newCrc ) {
      NParamBlock& rparams = reply->getParamBlock();

      rparams.addParam( NParam( 6101, newCrc ) );
      addDataAsParams( rparams, data );
   }
   return true;
} catch ( const MC2Exception& e ) {
   mc2log << warn << "handleGetKeyedData: " << e.what() << endl;
   reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
   return false;
}

void NavKeyedDataHandler::getParameters( NParamBlock& params,
                                         MC2String& key,
                                         uint32& crc,
                                         bool& sentCRC ) {
   // get the key
   if ( !getParam( 6100, key, params ) ) {
      throw MC2Exception( "Client didn't send a valid key!" );

   }

   crc = 0;
   sentCRC = getParam( 6101, crc, params );
}
