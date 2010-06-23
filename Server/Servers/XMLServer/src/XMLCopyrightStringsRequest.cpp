/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserThread.h"

#include "XMLTool.h"
#include "StringConvert.h"

#include "LangTypes.h"
#include "ServerTileMapFormatDesc.h"
#include "MC2CRC32.h"
#include "XMLServerElements.h"
#include "ClientSettings.h"

bool
XMLParserThread::
xmlParseCopyrightStringsRequest( DOMNode* cur,
                                 DOMNode* out,
                                 DOMDocument* reply,
                                 bool indent ) 
try {
   // create standard reply
   DOMElement* root = 
      XMLUtility::createStandardReply( *reply, *cur, 
                                       "copyright_strings_reply" );
   out->appendChild( root );

   // fetch attributes
   LangTypes::language_t language;
   XMLTool::getAttrib( language, "language", cur );
   MC2String inCRC;
   XMLTool::getAttrib( inCRC, "crc", cur );

   // fetch tilemap format desc with the correct language
   uint32 drawVersion = 1;
   // must have a user and client settings for a new draw version
   if ( getCurrentUser() && 
        getClientSetting() ) {
      drawVersion = getClientSetting()->getDrawVersion();
   }

   ImageTable::ImageSet imageSet = ImageTable::DEFAULT;
   if ( getClientSetting() ) {
      imageSet = getClientSetting()->getImageSet();
   }

   STMFDParams param( language,  // language for copyrights
                      false, // night mode is not important
                      // layers not important here
                      STMFDParams::DEFAULT_LAYERS,
                      // version to determine if to include
                      // supplier IDs in copyright holder, or not.
                      drawVersion,
                      imageSet );

   const TileMapFormatDesc* desc = m_group->
      getTileMapFormatDesc( param, this );

   // fetch copyright holder for this language
   const CopyrightHolder& copyrights = desc->getCopyrightHolder();

   // save copyrights to buffer and base64 encode the buffer
   BitBuffer buff( copyrights.getSizeInBuffer() );
   copyrights.save( buff, drawVersion ); 


   // allocate enough space for base64
   uint32 base64Size = buff.getBufferSize() * 4 / 3 + 4;
   ScopedArray<char> base64Out( new char[ base64Size ] );
   memset( base64Out.get(), 0, base64Size );
   if ( ! StringUtility::base64Encode( buff.getBufferAddress(),
                                       buff.getBufferSize(),
                                       base64Out.get() ) ) {
      mc2log << warn << "[CopyrightStringsRequest] Failed to base64"
             << " encode the copyright strings." << endl;
      return false;
   }
   // generate crc and compare to the crc we got in
   uint32 crc = MC2CRC32::crc32( (byte*)base64Out.get(),
                                 strlen( base64Out.get() ) );
   root->setAttribute( X( "crc" ), XHex32( crc ) );

   if ( XMLString::equals( XHex32( crc ), inCRC.c_str() ) ) {
      XMLTool::addNode( root, "crc_ok" );
   } else {
      // crc did not match, append data
      DOMElement* dataNode = 
         XMLTool::addNode( root, "copyright_strings_data" );
      // now append data to the final xml
      XMLTool::setNodeValue( dataNode, MC2String( base64Out.get() ) );
   }

   if ( indent ) {
      XMLUtility::indentPiece( *root, 1 );
   }

   return true;

} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[CopyrightStringsRequest]  " << e.what() << endl;
   XMLServerUtility::
      appendStatusNodes( out->getFirstChild(), reply, 1, false,
                         "-1", e.what() );
   return false;

} 
