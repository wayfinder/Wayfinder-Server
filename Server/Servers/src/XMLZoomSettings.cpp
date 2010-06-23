/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLZoomSettings.h"

#include "XMLUtility.h"
#include "DrawingProjection.h"
#include "MC2CRC32.h"
#include "DataBuffer.h"
#include "ClientSettings.h"

#include <memory>

namespace XMLZoomSettings {

DOMElement* createReplyBody( DOMElement& root,
                             uint32 nbrPixels,
                             const ClientSetting* setting ) {
   DOMDocument* doc = root.getOwnerDocument();
   DOMElement* elements = doc->createElement( X("zoom_levels") );
   root.appendChild( elements );

   struct Int32Value {
      const char* m_name;
      int32 m_value;
   };

   auto_ptr<MercatorProjection> 
      proj( new MercatorProjection( 0, 0, 1, nbrPixels ) );

   // for calculating crc, 1024 should be large enough.
   DataBuffer crcBuffer( 1024 + 1024*proj->getNbrZoomlevels() );

   int nbrZoomLevels = proj->getNbrZoomlevels(); 
   // zoom_levels attributes
   Int32Value mainValues[] = {
      { "nbr_zoom_levels", nbrZoomLevels },
      { "pixel_size", nbrPixels },
   };
   const uint32 nbrMainValues = sizeof( mainValues )/sizeof( mainValues[0]);


   vector< pair<int, int> > sizes = proj->getSquareSizes();
   for ( uint32 i = 0; i < nbrMainValues; ++i ) {
      elements->setAttribute( X( mainValues[ i ].m_name ),
                              XInt32( mainValues[ i ].m_value ) );
      crcBuffer.writeNextLong( mainValues[ i ].m_value );
   }
   
   for ( int i = 0; i < nbrZoomLevels; ++i ) {

      DOMElement* elem = doc->createElement( X( "zoom_level" ) );

      // add zoom level attributes
      Int32Value values[] = {
         { "zoom_level_nbr", i + 1 },
         { "min_x", -sizes[i].second / 2 * nbrPixels },
         { "min_y", -sizes[i].first / 2 * nbrPixels },
         { "max_x", sizes[i].second / 2 * nbrPixels - nbrPixels },
         { "max_y", sizes[i].first / 2 * nbrPixels - nbrPixels },
      };

      const uint32 nbrValues = sizeof( values ) / sizeof( values[ 0 ] );

      for ( uint32 valueIndex = 0; valueIndex < nbrValues; ++valueIndex ) {
         elem->setAttribute( X( values[ valueIndex ].m_name ),
                             XInt32( values[ valueIndex ].m_value ) );
         crcBuffer.writeNextLong( values[ valueIndex ].m_value );
      }

      // Should we add j2me zoom levels?
      if ( proj->isJ2meZoom( i+1 ) ||
           // If we have setting and it is wolfram then
           // add j2me zoom level for all levels
           ( setting != NULL &&
             setting->getCreateLevel() == 
             WFSubscriptionConstants::WOLFRAM  ) ) {
         elem->setAttribute( X("zoom_j2me"), X("true") );
         crcBuffer.writeNextLong(1);
      }
      elements->appendChild( elem );
   }
   // calculate crc and add "crc" attrib
   uint32 crc = MC2CRC32::crc32( crcBuffer.getBufferAddress(), 
                                 crcBuffer.getCurrentOffset() );
   elements->setAttribute( X("crc"), XHex32( crc ) );
   XMLUtility::indentPiece( *elements, 1 );

   return elements;
}

DOMDocument* createReply( uint32 pixelSize ) {
   DOMImplementation* impl =
      DOMImplementationRegistry::getDOMImplementation( X( "" ) );
   
   DOMDocument* reply =
      impl->createDocument( 0,
                            X( "zoom_settings_reply" ),
                            0 );
   DOMElement* root = reply->getDocumentElement();
   root->setAttribute(  X( "transaction_id" ),
                        X( "" ) );
   XMLZoomSettings::createReplyBody( *root, pixelSize );

   return reply;
}

}
