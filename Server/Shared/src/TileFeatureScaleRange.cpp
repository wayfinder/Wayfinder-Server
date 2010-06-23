/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileFeatureScaleRange.h"

#include "TileFeatureStrings.h"
#include "XMLTool.h"
#include "XMLTreeFormatter.h"
#include "XPathExpression.h"

#include <fstream>

namespace TileFeatureScaleRange {

struct XMLDocumentRelease {
   explicit XMLDocumentRelease( DOMDocument* doc ):
      m_doc( doc ) {
   }
   ~XMLDocumentRelease() {
      m_doc->release();
   }
   DOMDocument* m_doc;
};

bool save( const MC2String& filename,
           const ScaleRangeByType& scaleRangeByType ) {


   DOMImplementation* impl =
      DOMImplementationRegistry::
      getDOMImplementation( X( "LS" ) );

   DOMDocument* reply = impl->
      createDocument( NULL,      // root element namespace URI.
                      X( "poi_scale_settings" ),  // root element name
                      NULL ); // document type object (DTD).

   XMLDocumentRelease replyRelease( reply );

   // Create the DOCTYPE
   reply->insertBefore( reply->createDocumentType( X( "poi_scale_settings" ) ),
                        reply->getDocumentElement() );

   DOMNode* firstNode = reply->getLastChild();

   using namespace XMLTool;

   for ( ScaleRangeByType::const_iterator
            it = scaleRangeByType.begin(), itEnd = scaleRangeByType.end();
         it != itEnd;
         ++it ) {
      DOMElement* poiNode = addNode( firstNode, "poi_scale" );
      addAttrib( poiNode, "type",
                 MC2String( TileFeatureStrings::feat2str( it->first ) ) );
      addAttrib( poiNode, "begin", it->second.first );
      addAttrib( poiNode, "end", it->second.second );
   }

   // Write an indented tree to file.
   ofstream outfile( filename.c_str() );
   if ( ! outfile ) {
      return false;
   }

   XMLTreeFormatter::printIndentedTree( reply, outfile );

   if ( ! outfile ) {
      return false;
   }

   return true;

}


bool load( const MC2String& filename,
           ScaleRangeByType& scaleRangeByType ) {
   XercesDOMParser parser;

   parser.parse( filename.c_str() );

   DOMDocument* doc = parser.getDocument();
   if ( doc == NULL || doc->getFirstChild() == NULL ) {
      mc2dbg << "[TFSR] Failed to parse document." << endl;
      return false;
   }

   using namespace XMLTool;

   using XMLTool::XPath::Expression;
   // Evaluate nodes and add settings

   Expression::result_type result =
      Expression( "poi_scale_settings/poi_scale*" ).
      evaluate( doc->getFirstChild() );

   ScaleRangeByType rangesOut;
   for ( Expression::result_type::const_iterator
            it = result.begin(), itEnd = result.end();
         it != itEnd;
         ++it ) {
      MC2String typeStr;
      getAttribValue( typeStr, "type", *it );
      ScaleRange range;
      getAttribValue( range.first, "begin", *it );
      getAttribValue( range.second, "end", *it );
      TilePrimitiveFeature::tileFeature_t type =
         TileFeatureStrings::str2feat( typeStr.c_str() );
      if ( type == TilePrimitiveFeature::nbr_tile_features ) {
         mc2log << warn << "[TFSR] Unknown feature type: " << typeStr << endl;
      } else {
         rangesOut[ type ] = range;
      }
   }

   rangesOut.swap( scaleRangeByType );

   return true;
}

}

