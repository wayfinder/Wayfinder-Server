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
#include "ParserMapHandler.h"
#include "SimpleArray.h"
#include "MC2CRC32.h"
#include "XMLTool.h"
#include "XMLServerElements.h"
#include "ClientSettings.h"

using XMLServerUtility::appendStatusNodes;
   
bool XMLParserThread::
xmlParseSimplePOIDescRequest( DOMNode* cur, DOMNode* out,
                              DOMDocument* reply,
                              bool indent ) { 
try {

   DOMElement* root = reply->createElement( X( "simple_poi_desc_reply" ) );
   out->appendChild( root );
   const XMLCh* transaction_id = X( "transaction_id" );
   root->setAttribute(  transaction_id,
                        cur->getAttributes()->getNamedItem( transaction_id )
                        ->getNodeValue() );

   // get map handler and match crc

   ParserMapHandler& mapHandler = getMapHandler();

   const ClientSetting* clientSetting = getClientSetting();
   ImageTable::ImageSet imageSet = ImageTable::DEFAULT;
   if ( clientSetting != NULL ) {
      imageSet = clientSetting->getImageSet();
   }

   ParserMapHandler::bufPair_t bufPair = 
      mapHandler.getSimplePoiDesc( SimplePoiDescParams(), imageSet );

   auto_ptr<SharedBuffer> buf ( bufPair.second );

   if ( bufPair.first != ParserMapHandler::OK || buf.get() == NULL ) {
      appendStatusNodes( root, reply, 1, false,
                         "-1", "Could not get poi description" );
      return false;
   }
   // create text node with base64 encoded data

   MC2String base = StringUtility::
      base64Encode( MC2String(reinterpret_cast<char*>(buf->getBufferAddress()),
                              buf->getBufferSize() ));
   
   uint32 crc = MC2CRC32::crc32( (byte*)base.data(),
                                 base.length() );
   // check crc against the request
   // if crc matches then just send crc_ok 
   root->setAttribute( X( "crc" ), XHex32( crc ) );
   // no checks for pointers here, dtd checked.
   MC2String crcReceived;
   XMLTool::getAttrib( crcReceived, "crc", cur );
   
   if ( XMLString::equals( XHex32( crc ), crcReceived.c_str() ) ) {
      root->appendChild( reply->createElement( X("crc_ok" ) ) );
   } else {
      // set
      DOMElement* dataNode = 
         reply->createElement( X("simple_poi_desc_data" ) );
      XMLTool::addAttrib( dataNode, "te", MC2String("base64") );
      DOMText* dataValue = reply->createTextNode( X( base ) );
      dataNode->appendChild( dataValue );
      root->appendChild( dataNode );
   }

   if ( indent ) {
      XMLUtility::indentPiece( *root, 1 );
   }

   return true;

} catch ( const XMLTool::Exception& e ) {
   appendStatusNodes( out->getFirstChild(), reply, 1, false,
                      "-1", e.what() );
   return false;
}
}
