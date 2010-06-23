/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileColorSettings.h"

#include "ServerTileMapFormatDesc.h"
#include "TileFeature.h"
#include "XMLTool.h"
#include "XMLTreeFormatter.h"
#include "XPathExpression.h"
#include "STLStringUtility.h"
#include "TileFeatureStrings.h"

#include <fstream>
#include <sstream>

namespace {

// All tile map name types
#define TCS_NAMETYPE \
   TF2STR(on_roundrect); \
   TF2STR(on_bitmap); \
   TF2STR(on_line); \
   TF2STR(above_line); \
   TF2STR(under_line); \
   TF2STR(horizontal); \
   TF2STR(inside_polygon)

/**
 * Convert a string to a name type.
 * @param str
 * @return name type.
 */
TileMapNameSettings::name_t str2nameType( const char* str ) {
#define TF2STR(x) if ( strcasecmp( str, #x ) == 0 ) return TileMapNameSettings::x;
   TCS_NAMETYPE;
#undef TF2STR

   // TODO: fix some indication of error.
   return TileMapNameSettings::on_line;
}

/**
 * Convert type to a string.
 * @param type The type to convert to a string.
 * @return type as string
 */
const char* nameType2str( TileMapNameSettings::name_t type ) {
#define TF2STR(x) case TileMapNameSettings::x: return #x
   switch ( type ) {
      TCS_NAMETYPE;
   default:
      break;
   }
#undef TF2STR
   return "";
}

/**
 * Convert color integer to a XML string.
 */
MC2String tohex( uint32 color ) {
   stringstream str;
   str << hex 
       << setw(2) << setfill('0') << ( ( color >> 16 ) & 0xFF );
   str << setw(2) << setfill('0') << ( ( color >> 8 ) & 0xFF );
   str << setw(2) << setfill('0') << ( color & 0xFF );

   return str.str();
}

/**
 * Read color from attribute.
 * @param node
 * @param name The attribute name to find.
 * @return true if the attrib value was found and read into color.
 */ 
bool readColor( const DOMNode* node, const char* name, uint32& color ) {
   MC2String colorStr;
   if ( XMLTool::getAttribValue( colorStr, name, node ) ) {
      color = STLStringUtility::strtol( colorStr, 16 );
      return true;
   }

   return false;
}

/**
 * Read scales from baseNode and add them to settings.
 * @param baseNode
 * @param settings
 */
void readScales( const DOMNode* baseNode, TileDrawSettings& settings ) {
   using XMLTool::XPath::Expression;

   TileDrawSetting setting;
   Expression::result_type result =
      Expression( "scale*" ).evaluate( baseNode );

   using XMLTool::getAttrib;
   using XMLTool::getAttribValue;

   for ( Expression::result_type::const_iterator it = result.begin(),
            itEnd = result.end();
         it != itEnd; ++it ) {
      int scale = 0;
      getAttrib( scale, "value", *it );
      uint32 color;
      if ( readColor( *it, "color", color ) ) {
         setting.m_color = color;
      }
      if ( readColor( *it, "border_color", color ) ) {
         setting.m_borderColor = color;
      }

      getAttribValue( setting.m_fontType, "font_type", *it );
      getAttribValue( setting.m_fontSize, "font_size", *it );
      getAttribValue( setting.m_width, "width", *it );
      getAttribValue( setting.m_widthMeters, "width_meters", *it );
      getAttribValue( setting.m_borderWidth, "border_width", *it );

      MC2String nameType;
      getAttribValue( nameType, "name_type", *it );
      if ( ! nameType.empty() ) {
         setting.m_nameType = str2nameType( nameType.c_str() );
      }

      getAttribValue( setting.m_level, "level", *it );
      getAttribValue( setting.m_level1, "level1", *it );

      settings[ scale ] = setting;
   }
}

/**
 * Create XML scale nodes.
 * @param baseNode The node to read scales from.
 * @param settings Create XML nodes from these settings.
 */
void addScales( DOMElement* baseNode, const TileDrawSettings& settings ) {

   // For each setting add attributes that differs from previous setting
   for ( TileDrawSettings::const_iterator it = settings.begin(),
            itEnd = settings.end(), itPrev = settings.begin();
         it != itEnd; itPrev = it, ++it ) {

// Whether the property x is the first property to be added or
// its not the first property and the property differs from previous value
#define NOT_SAME_PROP(x) \
      ( itPrev == it || (itPrev != it && it->second.x != itPrev->second.x) )


      DOMElement* scaleNode = XMLTool::addNode( baseNode, "scale" );
      XMLTool::addAttrib( scaleNode, "value", it->first );

      if ( NOT_SAME_PROP( m_color ) ) {
         XMLTool::addAttrib( scaleNode, "color", tohex( it->second.m_color ) );
      }

      if ( NOT_SAME_PROP( m_borderColor ) &&
           it->second.m_borderColor != MAX_UINT32 ) {
         XMLTool::addAttrib( scaleNode, "border_color",
                             tohex( it->second.m_borderColor ) );
      }

      if ( NOT_SAME_PROP( m_borderWidth ) &&
           it->second.m_borderWidth != MAX_UINT32 ) {
         XMLTool::addAttrib( scaleNode, "border_width",
                             it->second.m_borderWidth );
      }

      if ( NOT_SAME_PROP( m_fontType ) &&
           it->second.m_fontType != MAX_UINT32 ) {
         XMLTool::addAttrib( scaleNode, "font_type", it->second.m_fontType );
      }

      if ( NOT_SAME_PROP( m_fontSize ) &&
           it->second.m_fontSize != MAX_UINT32 ) {
         XMLTool::addAttrib( scaleNode, "font_size", it->second.m_fontSize );
      }

      if ( NOT_SAME_PROP( m_nameType ) ) {
         MC2String nameTypeStr = nameType2str( ( TileMapNameSettings:: name_t )
                                               it->second.m_nameType );
         XMLTool::addAttrib( scaleNode, "name_type", nameTypeStr );
      }

      if ( NOT_SAME_PROP( m_width ) &&
           it->second.m_width != MAX_UINT32 ) {
         XMLTool::addAttrib( scaleNode, "width", it->second.m_width );
      }

      if ( NOT_SAME_PROP( m_widthMeters ) &&
           it->second.m_widthMeters != MAX_UINT32 ) {
         XMLTool::addAttrib( scaleNode, "width_meters",
                             it->second.m_widthMeters );
      }

      if ( NOT_SAME_PROP( m_level ) &&
           it->second.m_level != MAX_UINT32 ) {
         XMLTool::addAttrib( scaleNode, "level", it->second.m_level );
      }

      if ( NOT_SAME_PROP( m_level1 ) &&
           it->second.m_level1 != MAX_UINT32 ) {
         XMLTool::addAttrib( scaleNode, "level1", it->second.m_level1 );
      }

#undef NOT_SAME_PROP
   }

}

void readColorFromAttrib( const DOMNode* topNode,
                          const char* nodeName, const char* attribName,
                          uint32& color ) {
   const DOMNode* node = XMLTool::findNodeConst( topNode,
                                                 nodeName );
   if ( node ) {
      readColor( node, attribName, color );
   } else {
      mc2dbg << "No node named \"" << nodeName << "\" in top node:"
             << "\"" << topNode->getNodeName() << "\"" << endl;
   }
}

} // end anonymous namespace


namespace TileColorSettings {

bool loadSettings( const MC2String& filename,
                   map< int, TileDrawSettings >& settingsByType,
                   BasicSetting& setting ) try {

   // Parse filename
   XercesDOMParser parser;
   parser.setValidationScheme( XercesDOMParser::Val_Auto );
   parser.setIncludeIgnorableWhitespace( false );

   parser.parse( filename.c_str() );

   DOMDocument* doc = parser.getDocument();
   if ( doc == NULL ) {
      mc2dbg << "[TCS] Failed to parse document." << endl;
      return false;
   }

   using XMLTool::XPath::Expression;

   const DOMNode* topNode = XMLTool::findNodeConst( doc,
                                                    "tile_color_settings" );
   if ( topNode == NULL ) {
      mc2log << "[TCS] No node named tile_color_settings in document."
             << endl;
      return false;
   }

   map< int, TileDrawSettings > settingsByTypeOut;

   // Evaluate nodes and add settings
   Expression::result_type result =
      Expression( "setting*" ).evaluate( topNode->getFirstChild() );

   if ( result.empty() ) {
      mc2dbg << warn << "[TCS] No setting node in document." << endl;
      return false;
   }

   BasicSetting settingsOut;
   readColorFromAttrib( topNode, "text", "color",
                        settingsOut.m_textColor );
   readColorFromAttrib( topNode, "top_horizon", "color",
                        settingsOut.m_topHorizonColor );
   readColorFromAttrib( topNode, "bottom_horizon", "color",
                        settingsOut.m_bottomHorizonColor );

   // for each setting node, read scale settings.
   using XMLTool::getAttrib;
   for ( Expression::result_type::const_iterator it = result.begin(),
            itEnd = result.end();
         it != itEnd; ++it ) {

      MC2String typeStr;
      getAttrib( typeStr, "type", *it );
      MC2String primitiveStr;
      getAttrib( primitiveStr, "primitive", *it );
      
      TileDrawSettings settings;
      int type = TileFeatureStrings::str2feat( typeStr.c_str() );
      settings.m_primitive =
         TileFeatureStrings::str2prim( primitiveStr.c_str() );

      ::readScales( (*it)->getFirstChild(), settings );

      settingsByTypeOut[ type ] = settings;
   }

   // We read the nodes successfully, now swap with original.
   settingsByTypeOut.swap( settingsByType );
   setting = settingsOut;

   return true;
} catch ( ... ) {
   return false;
}

bool saveSettings( const MC2String& filename,
                   const map< int, TileDrawSettings >& settingsByType,
                   const BasicSetting& setting ) try {
   DOMImplementation* impl = 
      DOMImplementationRegistry::
      getDOMImplementation( X( "LS" ) ); // Load Save

   DOMDocument* reply = impl->
      createDocument( NULL,      // root element namespace URI.
                      X( "tile_color_settings" ),  // root element name
                      NULL ); // document type object (DTD).

   // Create the DOCTYPE
   reply->insertBefore( reply->createDocumentType( X( "tile_color_settings" ) ),
                        reply->getDocumentElement() );

   DOMNode* firstNode = reply->getLastChild();

   XMLTool::addAttrib( XMLTool::addNode( firstNode, "text" ),
                       "color", tohex( setting.m_textColor ) );

   XMLTool::addAttrib( XMLTool::addNode( firstNode, "top_horizon" ),
                       "color", tohex( setting.m_topHorizonColor ) );

   XMLTool::addAttrib( XMLTool::addNode( firstNode, "bottom_horizon" ),
                       "color", tohex( setting.m_bottomHorizonColor ) );

   // add all types with all their scales
   map< int, TileDrawSettings >::const_iterator it = settingsByType.begin();
   map< int, TileDrawSettings >::const_iterator itEnd = settingsByType.end();
   for ( ; it != itEnd; ++it ) {
      DOMElement* settingNode = XMLTool::addNode( firstNode, "setting" );
      MC2String tmpStr = TileFeatureStrings::feat2str( it->first );
      XMLTool::addAttrib( settingNode, "type", tmpStr );
      tmpStr = TileFeatureStrings::prim2str( it->second.m_primitive );
      XMLTool::addAttrib( settingNode, "primitive", tmpStr );


      ::addScales( settingNode, it->second );
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
} catch ( const DOMException& error ) {
   MC2_ASSERT( false );
   return false;
}

} // TileColorSettings
