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
#include "ScopedArray.h"
#include "StringTable.h"
#include "SearchParserHandler.h"
#include "XMLUtility.h"
#include "XMLTool.h"
#include "DataBuffer.h"
#include "MC2CRC32.h"
#include "UserSwitch.h"

bool
XMLParserThread::xmlParseSearchDescRequest( DOMNode* cur, 
                                            DOMNode* out,
                                            DOMDocument* reply,
                                            bool indent  ) 
try {
   DOMElement* root = 
      XMLUtility::createStandardReply( *reply, *cur, "search_desc_reply" );
   // delay append child until we have checked crc
   //   out->appendChild( root );

   using namespace XMLTool;

   // get language type from "lang" attribute
   LangTypes::language_t language;
   getAttrib( language, "language", cur );

   // desc_version, must be used by new clients
   uint32 descVersion = 0;
   getAttribValue( descVersion, "desc_version", cur );

   auto_ptr<UserSwitch> userSwitch;
   uint32 uin = 0;
   getAttrib( uin, "uin", cur, (uint32)0 );
   if ( uin != 0 ) {
      UserItem* newUser = NULL;
      if ( getUser( uin, newUser, true ) ) {
         mc2dbg << "[SearchDescRequest] switching to user uin = " 
                << uin << endl;
         const ClientSetting* settings = NULL;
         UserElement* el = 
            newUser->getUser()->
            getElementOfType( 0, UserConstants::TYPE_LAST_CLIENT );
         if ( el ) {
            UserLastClient* last = static_cast< UserLastClient* >( el );
            settings = getGroup()->getSetting( last->getClientType().c_str(),
                                               "");
         }
         // user switch will release the new user item
         userSwitch.reset( new UserSwitch( *this, newUser, settings ) );
      } else {
         mc2dbg << "[SearchDescRequest] failed to get user for uin = "
                << uin << endl;
      }
   }

   SearchParserHandler& handler = getSearchHandler();
   CompactSearchHitTypeVector types( handler.getSearchHitTypes( language, descVersion ) );
   // calculate crc buffer size
   CompactSearchHitTypeVector::const_iterator it = types.begin();
   CompactSearchHitTypeVector::const_iterator itEnd = types.end();
   uint32 bufferSize = 0;
   for (; it != itEnd; ++it ) {
      // round + heading + string length with \0
      bufferSize +=  
         2 * sizeof( uint32 ) + 
         (*it).m_name.size() + 1 +
         (*it).m_imageName.size() + 1;
   }

   DataBuffer crcBuff( bufferSize*2 ); // *2, just for alignment
   crcBuff.fillWithZeros();

   // write xml

   for ( it = types.begin(); it != itEnd; ++it ) {
      DOMElement* hitType = addNode( root, "search_hit_type" );

      addAttrib( hitType, "round", (*it).m_round );
      crcBuff.writeNextLong( (*it).m_round );

      addAttrib( hitType, "heading", (*it).m_heading );
      crcBuff.writeNextLong( (*it).m_heading );

      addNode( hitType, "name", (*it).m_name );
      crcBuff.writeNextString( (*it).m_name.c_str() );

      if ( ! (*it).m_type.empty() ) {
         addNode( hitType, "type", (*it).m_type );
         crcBuff.writeNextString( (*it).m_type.c_str() );
      }

      // only add top_region_id tag if it has valid top region id
      if ( (*it).m_topRegionID != MAX_UINT32 ) {
         addNode( hitType, "top_region_id", (*it).m_topRegionID );
         crcBuff.writeNextLong( (*it).m_topRegionID );
      }

      if ( ! (*it).m_imageName.empty() ) {
         addNode( hitType, "image_name", (*it).m_imageName );
         crcBuff.writeNextString( (*it).m_imageName.c_str() );
      }
   }

   // check crc
   MC2String reqCrc;
   getAttribValue( reqCrc, "crc", cur );

   uint32 crc = MC2CRC32::crc32( (byte*)crcBuff.getBufferAddress(),
                                 crcBuff.getCurrentOffset() );

   if ( XMLString::equals( XHex32( crc ), 
                           reqCrc.c_str() ) ) {
      // crc match, create a new reply with crc_ok
      root = XMLUtility::createStandardReply( *reply, *cur, 
                                              "search_desc_reply" );
      addNode( root, "crc_ok" );
   }

   // now we can append child
   out->appendChild( root );

   addAttrib( root, "length", types.size() );
   root->setAttribute( X( "crc" ), XHex32( crc ) );

   if ( indent ) {
      XMLUtility::indentPiece( *root, 1 );
   }

   return true;

} catch ( const XMLTool::Exception& e ) {
   mc2log << warn << "search_desc_request: " << e.what() << endl;
   return false;
}
