/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "DataBuffer.h"

#include "GfxFeature.h"
#include "GfxPolygon.h"
#include "GfxRoadPolygon.h"
#include "MapUtility.h"
#include "MC2BoundingBox.h"
#include "StringUtility.h"

namespace {
/**
 * Save categories to a data buffer.
 * @param data Buffer to save to.
 * @param categories The categories to save to the buffer.
 */
void saveCategories( DataBuffer* data,
                     const GfxPOIFeature::Categories& categories ) {

   data->writeNextShort( categories.size() );
   GfxPOIFeature::Categories::const_iterator it = categories.begin();
   for (; it != categories.end(); ++it ) {
      data->writeNextShort( *it );
   }
}

/**
 * Load categories from data buffer.
 * @param data Buffer to load from.
 * @param categories The container to load to.
 */
void loadCategories( DataBuffer* data,
                     GfxPOIFeature::Categories& categories ) {
   categories.resize( data->readNextShort() );
   for ( uint32 i = 0; i < categories.size(); ++i ) {
      categories[ i ] = data->readNextShort();
   }
}


GfxFeature::SizeType
calcCategoriesSize( const GfxPOIFeature::Categories& categories ) {
   return 2 +  // nbr categories
      2 * sizeof ( categories.size() ); // data size
}

/**
 * Save event strings to a data buffer.
 * @param data Buffer to save to.
 * @param strings The strings to save in the data buffer.
 */
void saveStrings( DataBuffer* data,
                  const GfxEventFeature::Strings& strings ) {
   data->writeNextLong( strings.size() );
   GfxEventFeature::Strings::const_iterator it = strings.begin();
   for ( ; it != strings.end(); ++it ) {
      data->writeNextShort( (*it).getType() );
      data->writeNextString( (*it).getString().c_str() );
   }
}

/**
 * Load strings from data buffer.
 * @param data Buffer to load from.
 * @param strings The container to load to.
 */
void loadStrings( DataBuffer* data,
                  GfxEventFeature::Strings& strings ) {
   GfxEventFeature::Strings::size_type nbrStrings =
      data->readNextLong();
   while ( nbrStrings-- ) {
      GfxEventFeature::StringType::Type type = data->readNextShort();
      strings.push_back( GfxEventFeature::
                         StringType( type, data->readNextString() ) );
   }
}

GfxFeature::SizeType
calcStringsSize( const GfxEventFeature::Strings& strings ) {
   // nbr strings + nbr strings * sizeof type
   GfxFeature::SizeType size = 4 + strings.size() * 2;

   GfxEventFeature::Strings::const_iterator it = strings.begin();
   for ( ; it != strings.end(); ++it ) {
      size += it->getString().size() + 1; // strlen + terminating null
   }

   return size;
}

}

GfxFeature::GfxFeature( gfxFeatureType type, const char* name,
                        bool displayText, int fontSize,
                        int32 textLat, int32 textLon, 
                        int drawTextStart) 
      : m_type( type ),
        m_displayText( displayText ),
        m_fontSize( fontSize ),
        m_textLat( textLat ),
        m_textLon( textLon ),
        m_drawTextStart( drawTextStart ),
        m_countryCode( StringTable::NBR_COUNTRY_CODES )
{
   // IMPORTANT TO SET ALL MEMBER VARIABLES TO AVOID RANDOMNESS
   // WHEN CREATING TILEMAPS
   if (name != NULL)
      m_name = StringUtility::newStrDup( name );
   else
      m_name = StringUtility::newStrDup( "" );
   m_scaleLevel = CONTINENT_LEVEL;
}


GfxFeature::GfxFeature( gfxFeatureType type ) 
      : m_type( type ),
        m_displayText( false ),
        m_fontSize( 0 ),
        m_textLat( 0 ),
        m_textLon( 0 ),
        m_drawTextStart( 0 ),
        m_countryCode( StringTable::NBR_COUNTRY_CODES )
{
   // IMPORTANT TO SET ALL MEMBER VARIABLES TO AVOID RANDOMNESS
   // WHEN CREATING TILEMAPS
   m_name = StringUtility::newStrDup( "" );
   m_scaleLevel = CONTINENT_LEVEL;
}

GfxFeature::GfxFeature( gfxFeatureType type,
                        const MC2BoundingBox& bbox,
                        const char* name )
      : m_type( type ),
        m_displayText( false ),
        m_fontSize( 0 ),
        m_textLat( 0 ),
        m_textLon( 0 ),
        m_drawTextStart( 0 ),
        m_countryCode( StringTable::NBR_COUNTRY_CODES )
                                             
{
   // IMPORTANT TO SET ALL MEMBER VARIABLES TO AVOID RANDOMNESS
   // WHEN CREATING TILEMAPS
   m_scaleLevel = CONTINENT_LEVEL;
   m_name = StringUtility::newStrDup( ( name != NULL ) ? name : "" );
   // Create polygon of the bbox.
   GfxPolygon* poly = new GfxPolygon( false );
   poly->addCoordinate( bbox.getMinLat(), bbox.getMinLon() );
   poly->addCoordinate( bbox.getMaxLat(), bbox.getMinLon() );
   poly->addCoordinate( bbox.getMaxLat(), bbox.getMaxLon() );
   poly->addCoordinate( bbox.getMinLat(), bbox.getMaxLon() );
   m_polys.push_back( poly );
}
      
GfxFeature*
GfxFeature::clone() const
{
   // Let's use the good old databuffer trick:
   DataBuffer tmpBuf( getSize() );
   save( &tmpBuf );
   tmpBuf.reset();
   return createNewFeature( &tmpBuf ); 
}

GfxFeature*
GfxFeature::clonePolygon( uint16 poly ) const
{
   if ( poly >= getNbrPolygons() ) {
      return NULL;
   }

   // Clone the whole feature.
   GfxFeature* newFeature = clone();

   // Grab the polygon we want and mark it as NULL in the vector.
   GfxPolygon* chosenPolygon = newFeature->m_polys[ poly ];
   newFeature->m_polys[ poly ] = NULL;

   // Delete all other polygons.
   for ( uint16 p = 0; p < getNbrPolygons(); ++p ) {
      delete newFeature->m_polys[ p ];
   }
   newFeature->m_polys.clear();

   // Add the polygon we want.
   newFeature->m_polys.push_back( chosenPolygon );
  
   return newFeature;
}

GfxFeature::~GfxFeature() {
   for ( uint32 i = 0 ; i < m_polys.size() ; i++ ) {
      delete m_polys[ i ];
   }
   delete [] m_name;
}

void 
GfxFeature::setName(const char* name)
{
   delete [] m_name;
   m_name = StringUtility::newStrDup(name);
}


GfxFeature* 
GfxFeature::createNewFeature( DataBuffer* data ) {
   // Read the type
   gfxFeatureType type = gfxFeatureType( data->readNextLong() );
   
   GfxFeature* feature = createNewFeature( type );
   feature->createFromDataBuffer( data );

   return feature;
}

GfxFeature*
GfxFeature::createNewFeature( GfxFeature::gfxFeatureType type,
                              const char* name ) {
   GfxFeature* feature = NULL;

   switch ( type ) { 
      // Roads
      case STREET_MAIN:
      case STREET_FIRST:
      case STREET_SECOND:
      case STREET_THIRD:
      case STREET_FOURTH:
      case WALKWAY:
         feature = new GfxRoadFeature( type, name );
         break;
      // POI
      case POI:
         feature = new GfxPOIFeature( name );
         break; 
      case SYMBOL:
         feature = new GfxSymbolFeature( type );
         break;
      case TRAFFIC_INFO:
         feature = new GfxTrafficInfoFeature( name );
         break;
      case EVENT:
         feature = new GfxEventFeature( name );
         break;
      // Other feature
      default:
         feature = new GfxFeature( type, name );
         break;
   }
   
   return (feature);
}


bool   
GfxFeature::save( DataBuffer* data ) const {
   writeHeader( data );
   writePolygons( data );

   return true;
}


GfxFeature::SizeType
GfxFeature::getSize() const {
   // Type + scaleLevel + name + (textLat+textLon)*4 + displayText + 
   // fontSize + drawTextStart + nbrpolygons + possible alignment 
   uint32 size =  4 + 4 + 2*4 + 4 + 4 + 4 + 4 + 3;
   if (m_name != NULL) {
      size += strlen(m_name)+1;
   }
   size += m_basename.size() + 1;

   // Polygons
   for ( uint32 i = 0 ; i < getNbrPolygons() ; i ++ ) {
      size += m_polys[ i ]->getSize();
   }
   return size;
}


void
GfxFeature::addNewPolygon( bool coordinates16Bits, uint32 startSize ) {
   mc2dbg8 << "creating new GfxPolygon" << endl;
   m_polys.push_back( new GfxPolygon( coordinates16Bits, startSize ) );
}

void
GfxFeature::addNewPolygon( GfxPolygon* poly ) {
   m_polys.push_back( poly );
}

void
GfxFeature::dump(int verboseLevel ) const {
   if ( verboseLevel > 0 ) {
      cout << "GfxFeature " << endl
           << "   Header: " << endl
           << "      Type:          " << (int)m_type << endl
           << "      Name:          " << m_name << endl
           << "      textLat:       " << m_textLat << endl
           << "      textLon:       " << m_textLon << endl
           << "      displayText:   " << m_displayText << endl
           << "      fontSize:      " << m_fontSize << endl
           << "      drawTextStart: " << m_drawTextStart << endl
           << "      NbrPolygons:   " << getNbrPolygons() << endl;
      for ( uint32 i = 0 ; i < getNbrPolygons() ; i ++ ) {
         m_polys[ i ]->dump( verboseLevel );
      }
   }
}


bool
GfxFeature::equalsType( const GfxFeature* feature ) const 
{
   return (m_type == feature->m_type);
}


bool
GfxFeature::createFromDataBuffer( DataBuffer* data ) {
   uint32 nbrPolys = readHeader( data );
   readPolygons( data, nbrPolys );

   return true;
}


uint32 
GfxFeature::readHeader( DataBuffer* data ) {
   // Type is read by createNewFeature
   // ScaleLevel
   m_scaleLevel = data->readNextLong();
   // Names
   delete [] m_name;
   m_name = StringUtility::newStrDup( data->readNextString() );
   m_basename = data->readNextString();
   // Text coord
   m_textLat = data->readNextLong();
   m_textLon = data->readNextLong();
   // Font size
   data->readNextShort(); // Not used, always 0
   m_fontSize = data->readNextShort();
   // Text start
   m_drawTextStart = data->readNextLong();
   // Display text
   m_displayText = data->readNextShort();
   // Country code
   m_countryCode = (StringTable::countryCode) data->readNextShort();
    
   return data->readNextLong();   
}


void 
GfxFeature::readPolygons( DataBuffer* data, uint32 nbrPolys ) {
   for ( uint32 i = 0 ; i < nbrPolys ; i ++ ) {
      m_polys.push_back( readPolygon( data ) );
   }
}


void
GfxFeature::writeHeader( DataBuffer* data ) const {
   // Type
   data->writeNextLong( m_type );
   // ScaleLevel
   data->writeNextLong( m_scaleLevel );
   // Name
   data->writeNextString( m_name );
   // Basename
   data->writeNextString( m_basename.c_str() );
   // Text coord
   data->writeNextLong( getTextLat() );
   data->writeNextLong( getTextLon() );
   // Font size
   data->writeNextShort( 0 ); // Not used, always 0
   data->writeNextShort( getFontSize() );
   // Draw text start
   data->writeNextLong( getDrawTextStart() );
   // Display text
   data->writeNextShort( getDisplayText() );
   // Country code
   data->writeNextShort( m_countryCode );
   // Nbr polygons
   data->writeNextLong( m_polys.size() );
}


void
GfxFeature::writePolygons( DataBuffer* data ) const {
   for ( uint32 i = 0 ; i < m_polys.size() ; i++ ) {
      m_polys[ i ]->save( data );
   }
}


GfxPolygon* 
GfxFeature::readPolygon( DataBuffer* data ) const {
   GfxPolygon* res = new GfxPolygon();
   
   res->createFromDataBuffer( data );
   
   return res;
}


void 
GfxFeature::addCoordinateToLast( int32 lat, int32 lon ) {
   m_polys.back()->addCoordinate(lat, lon);
}


void 
GfxFeature::addCoordinateToLast( int32 lat, int32 lon, 
                                 int32 prevLat, int32 prevLon )
{
   m_polys.back()->addCoordinate(lat, lon, prevLat, prevLon);
}

void
GfxFeature::resizeNbrPolygons( uint32 size, 
                               bool coordinate16Bits,
                               uint32 startSize )
{
   // Delete any extra polygons existing in this feature.
   while ( getNbrPolygons() > size ) {
      // Delete the last polygon.
      delete m_polys.back();
      m_polys.pop_back();
   }
   
   // Add any extra needed.
   while ( getNbrPolygons() < size ) {
      addNewPolygon( coordinate16Bits, startSize );
   }

   MC2_ASSERT( size == getNbrPolygons() );
}

//**********************************************************************
// GfxRoadFeature
//**********************************************************************


GfxRoadFeature::GfxRoadFeature( gfxFeatureType type ) 
      : GfxFeature( type )
{
   // IMPORTANT TO SET ALL MEMBER VARIABLES TO AVOID RANDOMNESS
   // WHEN CREATING TILEMAPS
}


GfxRoadFeature::GfxRoadFeature( gfxFeatureType type,
                                const char* name )
      : GfxFeature( type, name )
{
   // IMPORTANT TO SET ALL MEMBER VARIABLES TO AVOID RANDOMNESS
   // WHEN CREATING TILEMAPS
}


GfxRoadFeature::~GfxRoadFeature() {
}


GfxFeature::SizeType
GfxRoadFeature::getSize() const {
   return GfxFeature::getSize();
}


void 
GfxRoadFeature::addNewPolygon( bool coordinates16Bits, uint32 startSize )
{
   mc2dbg8 << "creating new GfxRoadPolygon" << endl;
   m_polys.push_back( new GfxRoadPolygon( coordinates16Bits, startSize ) );
}

void
GfxRoadFeature::addNewPolygon( GfxRoadPolygon* poly ) {
   m_polys.push_back( poly );
}

void
GfxRoadFeature::dump(int verboseLevel) const {
   if ( verboseLevel > 0 ) {
      cout << "GfxRoadFeature " << endl;
   }
   GfxFeature::dump( verboseLevel );
}


GfxPolygon* 
GfxRoadFeature::readPolygon( DataBuffer* data ) const {
   GfxPolygon* res = new GfxRoadPolygon();
   
   res->createFromDataBuffer( data );
   
   return res;
}


//**********************************************************************
// GfxPOIFeature
//**********************************************************************

GfxPOIFeature::GfxPOIFeature(const char* name) 
   : GfxFeature( GfxFeature::POI, name ) 
{
   // IMPORTANT TO SET ALL MEMBER VARIABLES TO AVOID RANDOMNESS
   // WHEN CREATING TILEMAPS
   m_poiType   = ItemTypes::company;
   m_extraInfo = 0xff;
}
   

GfxPOIFeature::GfxPOIFeature( ItemTypes::pointOfInterest_t poiType )
   : GfxFeature( GfxFeature::POI )
{
   // IMPORTANT TO SET ALL MEMBER VARIABLES TO AVOID RANDOMNESS
   // WHEN CREATING TILEMAPS
   m_poiType   = poiType;
   m_extraInfo = 0xff;
}


GfxPOIFeature::GfxPOIFeature( ItemTypes::pointOfInterest_t poiType, 
                              const char* name )
   : GfxFeature( GfxFeature::POI, name )
{
   // IMPORTANT TO SET ALL MEMBER VARIABLES TO AVOID RANDOMNESS
   // WHEN CREATING TILEMAPS
   m_poiType   = poiType;
   m_extraInfo = 0xff;
}


GfxPOIFeature::~GfxPOIFeature()
{

}

bool
GfxPOIFeature::createFromDataBuffer( DataBuffer* data ) {
   // Read common information
   bool retVal = GfxFeature::createFromDataBuffer( data );
   if (retVal) {
      byte poiType = data->readNextByte();

      // Check if the highest bit is set, indicating that
      // m_customImageName will be supplied.
      bool readImageName = poiType & 0x80;
      // Remove the highest bit.
      poiType &= 0x7f;
      m_poiType = ItemTypes::pointOfInterest_t( poiType );
      m_extraInfo = data->readNextByte();
       
      m_customImageName = "";

      if ( readImageName ) {
         m_customImageName = data->readNextString();
      } else {
         data->readNextString();
      }

      ::loadCategories( data, m_categories );

   }

   return (retVal);
}



bool   
GfxPOIFeature::save( DataBuffer* data ) const {
   // Save common information
   bool retVal = GfxFeature::save( data );

   if (retVal) {
      // Save poi-type
      
      byte poiType = m_poiType;

      poiType |= 0x80;
      
      data->writeNextByte( poiType );
      data->writeNextByte( m_extraInfo );

      data->writeNextString( m_customImageName.c_str() );

      ::saveCategories( data, m_categories );
   }
   
   return (retVal);
}


GfxFeature::SizeType
GfxPOIFeature::getSize() const
{
   // GfxFeature-size + poi-type + possible alignment
   return (GfxFeature::getSize() + 1 + 3) + 
      ::calcCategoriesSize( m_categories ); // num categories + all categories
}


void 
GfxPOIFeature::dump(int verboseLevel ) const
{
   if ( verboseLevel > 0 ) {
      cout << "GfxPOIFeature " << endl;
   }
   GfxFeature::dump( verboseLevel );

   if ( verboseLevel > 0 ) {
      cout << "   poi-type  = " << int(m_poiType) << endl;
      cout << "   extraInfo = " << int(m_extraInfo) << endl;
      cout << "   customImageName = " << m_customImageName << endl;
   }  
   
}


bool
GfxPOIFeature::equalsType( const GfxFeature* feature ) const 
{
   if (GfxFeature::equalsType( feature )) {
      return (m_poiType == 
              (static_cast<const GfxPOIFeature*> (feature))->m_poiType);
   } else {
      return (false);
   }
}


void
GfxPOIFeature::setCustomImageName( const MC2SimpleString& customName )
{
   m_customImageName = customName;
}
 
      
const MC2SimpleString& 
GfxPOIFeature::getCustomImageName() const
{
   return m_customImageName;
}


//**********************************************************************
// GfxSymbolFeature
//**********************************************************************


GfxSymbolFeature::GfxSymbolFeature( gfxFeatureType type ) 
      : GfxFeature( type )
{
   // IMPORTANT TO SET ALL MEMBER VARIABLES TO AVOID RANDOMNESS
   // WHEN CREATING TILEMAPS
   m_symbol = PIN;
   m_symbolImage =  StringUtility::newStrDup( "" );  
}


GfxSymbolFeature::GfxSymbolFeature( gfxFeatureType type,
                                    const char* name,
                                    symbols symbol,
                                    const char* symbolImage )
      : GfxFeature( type, name )
{
   // IMPORTANT TO SET ALL MEMBER VARIABLES TO AVOID RANDOMNESS
   // WHEN CREATING TILEMAPS
   m_symbol = symbol;
   m_symbolImage = StringUtility::newStrDup( symbolImage );
}


GfxSymbolFeature::~GfxSymbolFeature() {
   delete [] m_symbolImage;
}

void GfxSymbolFeature::setSymbolImage( const char* image ) {
   delete [] m_symbolImage;
   m_symbolImage = StringUtility::newStrDup( image );
}

GfxFeature::SizeType
GfxSymbolFeature::getSize() const {
   return GfxFeature::getSize() + strlen( m_symbolImage ) + 1 + 4;
}


void
GfxSymbolFeature::dump( int verboseLevel ) const {
   if ( verboseLevel > 0 ) {
      cout << "GfxSymbolFeature " << endl
           << "   Header: " << endl
           << "      symbol:        " << int( m_symbol ) << endl
           << "      symbolImage:   " << m_symbolImage << endl;
   }
   GfxFeature::dump( verboseLevel );
}


GfxSymbolFeature::symbols 
GfxSymbolFeature::getSymbol() const {
   return m_symbol;
}


const char* 
GfxSymbolFeature::getSymbolImage() const {
   return m_symbolImage;
}


uint32 
GfxSymbolFeature::toStringBuf( char* target ) const {
   return writeSymbolString( target, getSymbol(), getName(),
                             getPolygon( 0 )->getLat(0),
                             getPolygon( 0 )->getLon(0),
                             getSymbolImage() );
}


uint32 
GfxSymbolFeature::writeSymbolString( char* target,
                                     symbols symbol,
                                     const char* name,
                                     int32 lat,
                                     int32 lon,
                                     const char* symbolImage )
{
   char safeName[ strlen( name ) * 4 / 3 + 4 + 1 ];
   char safeSymbolImage[ strlen( symbolImage ) * 4 / 3 + 4 + 1 ];
   StringUtility::base64Encode( name, safeName );
   StringUtility::base64Encode( symbolImage, safeSymbolImage );
   char* urlName = new char[ 3*strlen( safeName ) + 1 ];
   char* urlSymbolImage = new char[ 3*strlen( safeSymbolImage ) + 1 ];

   StringUtility::URLEncode( urlName, safeName );
   StringUtility::URLEncode( urlSymbolImage, safeSymbolImage );

   uint32 length = sprintf( target, "%d_%d_%d_%s_%s", 
                            symbol, lat, lon, urlName, urlSymbolImage );
   delete [] urlName;
   delete [] urlSymbolImage;

   return length;
}


bool 
GfxSymbolFeature::readSymbolString( const char* data,
                                    symbols& symbol,
                                    char*& name,
                                    int32& lat,
                                    int32& lon,
                                    char*& symbolImage )
{
   char urlString[ 4096 ];
   int urlSymbol = 0;
   int32 urlLat = 0;
   int32 urlLon = 0;
   if ( sscanf( data, "%d_%d_%d_%4095s", 
                &urlSymbol, &urlLat, &urlLon, urlString ) == 4 )
   {
      urlString[ 4095 ] = '\0';
      char* findPos = strchr( urlString, '_' );

      if ( urlSymbol < NBR_SYMBOLS && urlSymbol >= 0 && findPos != NULL ) {
         symbol = symbols( urlSymbol );
         lat = urlLat;
         lon = urlLon;

         char safeTmpName[ 4096 ];
         char tmpName[ 4096 ];
         int safeTmpNameLength = 0;
         int tmpNameLength = 0;
         char safeTmpSymbolImage[ 4096 ];
         char tmpSymbolImage[ 4096 ];
         int safeTmpSymbolImageLength = 0;
         int tmpSymbolImageLength = 0;

         // End name string at '_'
         *findPos = '\0';
         findPos++;

         safeTmpNameLength = StringUtility::URLDecode( 
            (byte*)safeTmpName, urlString );
         safeTmpName[ safeTmpNameLength ] = '\0';

         tmpName[ 0 ] = '\0';
         tmpNameLength = StringUtility::base64Decode( 
            safeTmpName, (byte*)tmpName );
         name = StringUtility::newStrDup( tmpName );

         safeTmpSymbolImageLength = StringUtility::URLDecode( 
            (byte*)safeTmpSymbolImage, findPos );
         safeTmpSymbolImage[ safeTmpSymbolImageLength ] = '\0';
         tmpSymbolImage[ 0 ] = '\0';
         tmpSymbolImageLength = StringUtility::base64Decode( 
            safeTmpSymbolImage, (byte*)tmpSymbolImage );
         symbolImage = StringUtility::newStrDup( tmpSymbolImage );

         return true;
      } else {
         return false;
      }
   } else {
      return false;
   }
}


uint32
GfxSymbolFeature::readHeader( DataBuffer* data )
{
   uint32 nbrPolys = GfxFeature::readHeader( data );
   m_symbol = symbols( data->readNextLong() );
   delete [] m_symbolImage;
   m_symbolImage = StringUtility::newStrDup( data->readNextString() );

   return nbrPolys;
}


void
GfxSymbolFeature::writeHeader( DataBuffer* data ) const {
   GfxFeature::writeHeader( data );

   data->writeNextLong( m_symbol );
   data->writeNextString( m_symbolImage );
}

//**********************************************************************
// GfxTrafficInfoFeature
//**********************************************************************


GfxTrafficInfoFeature::GfxTrafficInfoFeature(const char* name) 
      : GfxFeature( GfxFeature::TRAFFIC_INFO, name )
{
   // IMPORTANT TO SET ALL MEMBER VARIABLES TO AVOID RANDOMNESS
   // WHEN CREATING TILEMAPS
   m_trafficInfoType = TrafficDataTypes::NoType;
   m_angle = 0;
   m_validInBothDirections = false;
   m_startTime = MAX_UINT32;
   m_endTime = MAX_UINT32;
}


GfxTrafficInfoFeature::GfxTrafficInfoFeature( 
                        const char* name, 
                        TrafficDataTypes::disturbanceType trafficInfoType,
                        uint16 angle,
                        bool bothDirections,
                        uint32 startTime,
                        uint32 endTime )
      : GfxFeature( GfxFeature::TRAFFIC_INFO, name )
{
   // IMPORTANT TO SET ALL MEMBER VARIABLES TO AVOID RANDOMNESS
   // WHEN CREATING TILEMAPS
   m_trafficInfoType = trafficInfoType;
   setAngle(angle);
   m_validInBothDirections = bothDirections;
   m_startTime = startTime;
   m_endTime = endTime;
}


GfxTrafficInfoFeature::~GfxTrafficInfoFeature() 
{

}


GfxFeature::SizeType
GfxTrafficInfoFeature::getSize() const {
   return GfxFeature::getSize() + 1 + 1 + 1 + 4 + 4 + 1;
}


void
GfxTrafficInfoFeature::dump( int verboseLevel ) const {
   if ( verboseLevel > 0 ) {
      cout << "GfxTrafficInfoFeature " << endl
           << "   Header: " << endl
           << "      trafficInfoType:  " << int( m_trafficInfoType )
           << endl
           << "      angle:            " << getAngle() << endl
           << "      validInBothDirs   " << m_validInBothDirections << endl
           << "      startTime         " << m_startTime << endl
           << "      endTime           " << m_endTime << endl;
   }
   GfxFeature::dump( verboseLevel );
}


uint32
GfxTrafficInfoFeature::readHeader( DataBuffer* data ) {
   uint32 nbrPolys = GfxFeature::readHeader( data );
   m_startTime = data->readNextLong();
   m_endTime = data->readNextLong();
   m_trafficInfoType = TrafficDataTypes::disturbanceType(data->readNextByte());
   m_angle = data->readNextByte();
   m_validInBothDirections = data->readNextBool();

   return nbrPolys;
}


void
GfxTrafficInfoFeature::writeHeader( DataBuffer* data ) const {
   GfxFeature::writeHeader( data );

   data->writeNextLong( m_startTime );
   data->writeNextLong( m_endTime );
   data->writeNextByte( byte(m_trafficInfoType) );
   data->writeNextByte( m_angle );
   data->writeNextBool( m_validInBothDirections );         
}


const char*
GfxFeature::getFeatureTypeAsString( uint32 type ) {
   switch ( type ) {
   case STREET_MAIN:
      return "STREET_MAIN";
   case STREET_FIRST:
      return "STREET_FIRST";
   case STREET_SECOND:
      return "STREET_SECOND";
   case STREET_THIRD:
      return "STREET_THIRD";
   case STREET_FOURTH:
      return "STREET_FOURTH";
   case BUILTUP_AREA:
      return "BUILTUP_AREA";
   case PARK:
      return "PARK";
   case FOREST:
      return "FOREST";
   case BUILDING:
      return "BUILDING";
   case WATER:
      return "WATER";
   case ISLAND:
      return "ISLAND";
   case PEDESTRIANAREA:
      return "PEDESTRIANAREA";
   case AIRCRAFTROAD:
      return "AIRCRAFTROAD";
   case LAND:
      return "LAND";
   case BUILTUP_AREA_SQUARE:
      return "BUILTUP_AREA_SQUARE";
   case BUILTUP_AREA_SMALL:
      return "BUILTUP_AREA_SMALL";
   case WATER_LINE:
      return "WATER_LINE";
   case FERRY:
      return "FERRY";
   case RAILWAY:
      return "RAILWAY";
   case INDIVIDUALBUILDING:
      return "INDIVIDUALBUILDING";
   case NATIONALPARK:
      return "NATIONALPARK";
   case OCEAN:
      return "OCEAN";
   case BORDER:
      return "BORDER";
   case AIRPORTGROUND:
      return "AIRPORTGROUND";
   case CARTOGRAPHIC_GREEN_AREA:
      return "CARTOGRAPHIC_GREEN_AREA";
   case CARTOGRAPHIC_GROUND:
      return "CARTOGRAPHIC_GROUND";
   case ROUTE:
      return "ROUTE";
   case ROUTE_CONTINUATION:
      return "ROUTE_CONTINUATION";
   case PARK_CAR:
      return "PARK_CAR";
   case EMPTY:
      return "EMPTY";
   case SYMBOL:
      return "SYMBOL";
   case TRAFFIC_INFO:
      return "TRAFFIC_INFO";
   case ROUTE_ORIGIN:
      return "ROUTE_ORIGIN";
   case ROUTE_DESTINATION:
      return "ROUTE_DESTINATION";
   case POI:
      return "POI";
   case EVENT:
      return "EVENT";
   case WALKWAY:
      return "WALKWAY";
   case WATER_IN_PARK:
      return "WATER_IN_PARK";
   case ISLAND_IN_BUA:
      return "ISLAND_IN_BUA";
   case ISLAND_IN_WATER_IN_PARK:
      return "ISLAND_IN_WATER_IN_PARK";
   case ISLAND_IN_WATER_IN_PARK_BUA:
      return "ISLAND_IN_WATER_IN_PARK_BUA";
   case ISLAND_IN_WATER_IN_PARK_ISLAND:
      return "ISLAND_IN_WATER_IN_PARK_ISLAND";
   case BUA_ON_ISLAND:
      return "BUA_ON_ISLAND";
   case NBR_GFXFEATURES:
      return "NBR_GFXFEATURES";
   }

   return "Unknown";
  
}

GfxEventFeature::GfxEventFeature( const char* name ):
   GfxFeature( GfxFeature::EVENT, name ),
   m_strings(),
   m_categories(),
   m_date( 0 ),
   m_duration( 0 ),
   m_id( INVALID_ID ) {
}

bool GfxEventFeature::save( DataBuffer* data ) const {
   bool retVal = GfxFeature::save( data );
   if ( retVal ) {
      ::saveStrings( data, m_strings );
      ::saveCategories( data, m_categories );
      data->writeNextLong( m_date );
      data->writeNextLong( m_duration );
      data->writeNextLong( m_id );
   }

   return retVal;
}


bool GfxEventFeature::createFromDataBuffer( DataBuffer* data ) {
   bool retVal = GfxFeature::createFromDataBuffer( data );
   if ( retVal ) {
      ::loadStrings( data, m_strings );
      ::loadCategories( data, m_categories );
      m_date = data->readNextLong();
      m_duration = data->readNextLong();
      m_id = data->readNextLong();
   }

   return retVal;
}

GfxFeature::SizeType GfxEventFeature::getSize() const {
   SizeType size = GfxFeature::getSize();
   size += ::calcStringsSize( m_strings );
   size += ::calcCategoriesSize( m_categories );
   size += 2 * sizeof ( Date ); // m_date + m_duration
   size += 4; // ID
   return size;
}
