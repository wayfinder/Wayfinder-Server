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
#include "XMLZoomSettings.h"
#include "XMLTool.h"

#include <xercesc/dom/DOM.hpp>

bool XMLParserThread::
xmlParseZoomSettingsRequest( DOMNode* cur, DOMNode* out,
                             DOMDocument* reply, bool indent ) {

   DOMElement* root = XMLUtility::createStandardReply( *reply, *cur, 
                                                       "zoom_settings_reply" );
   out->appendChild( root );
   using XMLTool::getAttrib;

   uint32 pixelSize = CylindricalProjection::NBR_PIXELS;
   getAttrib( pixelSize, "pixel_size", cur, pixelSize );

   if ( ! CylindricalProjection::isValidPixelSize( pixelSize ) ){
      pixelSize = CylindricalProjection::NBR_PIXELS;
   }

   XMLZoomSettings::createReplyBody( *root, pixelSize, getClientSetting() );

   // now we have created a zoom_settings_reply body, we need
   // to check crc against the zoom_settings_request
   //
   // returns crc_ok if they match instead of entire zoom 
   // settings body

   MC2String crcReceived;
   getAttrib( crcReceived, "crc", cur, MC2String() );

   MC2String crcCreated; 
   try {
      getAttrib( crcCreated, "crc", XMLTool::findNode( 
                    root, "zoom_levels" ) );
   } catch ( const XMLTool::Exception& e ) {
      mc2log << fatal 
             << "xmlParseZoomSettingsRequest: No crc in created xml!" << endl;
      MC2_ASSERT( false );
   }
   
   if ( crcCreated == crcReceived ) { 
      // first child must be zoom_levels
      DOMNode* zoom_levels = XMLTool::findNode( 
         root, "zoom_levels" );
      if ( ! XMLString::equals( zoom_levels->getNodeName(), "zoom_levels" ) ) {
         mc2log << fatal << "[ZoomRequest] first child is not zoom levels!" << endl;
         MC2_ASSERT( false );
      }
      // replace zoom_levels with crc_ok
      root->replaceChild( reply->createElement( X( "crc_ok" ) ),
                          zoom_levels );
   }

   if ( indent ) {
      XMLUtility::indentPiece( *root, 1 );
   }
   return true;
}
