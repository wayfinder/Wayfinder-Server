/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GenericMap.h"
#include "GfxData.h"
#include "GfxDataFull.h"
#include "GfxDataMultiplePoints.h"
#include "GfxDataSingleLine.h"
#include "GfxDataSinglePoint.h"
#include "GfxDataSingleSmallPoly.h"

#include "CommandlineOptionHandler.h"
#include "STLStringUtility.h"

#include <iostream>
#include <algorithm>
#include <memory>
#include <cstdlib>

using namespace std;

class GfxDataStat {
public:
   GfxDataStat():
      m_nbrPolygons( 0 ),
      m_nbrData( 0 ),
      m_nbrWithoutData( 0 )  {

      // clear arrays
      fill_n( m_nbrCoordinates, 5, 0);
      fill_n( m_nbrGfxTypes, 5, 0 );
      fill_n( m_memoryUsage, 5, 0 );
      fill_n( m_sizeInDataBuffer, 5, 0 );
   }

   /// count sample
   void count(const GfxData* data ) {
      if ( data == NULL ) {
         m_nbrWithoutData++;
         return;
      }

      GfxData::gfxdata_t type = data->getGfxDataType();
      m_nbrPolygons += data->getNbrPolygons();
      m_nbrCoordinates[ type ] += data->getTotalNbrCoordinates();
      m_nbrGfxTypes[ type ]++;
      m_memoryUsage[ type ] += data->getMemoryUsage();
      m_sizeInDataBuffer[ type ] +=  data->getSizeInDataBuffer();
      m_nbrData++;
   }

   uint32 getTotalNbrCoordinates() const {
      return accumulate(m_nbrCoordinates, m_nbrCoordinates + 5, 0 );
   }

   uint32 getNbrCoordinates( GfxData::gfxdata_t type ) const {
      return m_nbrCoordinates[ type ];
   }

   uint32 getNbrPolygons() const {
      return m_nbrPolygons;
   }

   uint32 getNbrData() const {
      return m_nbrData;
   }

   uint32 getNbrWithoutData() const {
      return m_nbrWithoutData;
   }

   float32 getAverageNbrPolygons() const {
      return (float64)(m_nbrPolygons) / (float64)(m_nbrData);
   }

   float32 getAverageNbrCoords() const {
      return (float64)(getTotalNbrCoordinates()) / (float64)(m_nbrData);
   }

   uint32 getNbrGfxType( GfxData::gfxdata_t type ) const {
      return m_nbrGfxTypes[ type ];
   }
   uint64 getEntireSizeOfType( GfxData::gfxdata_t type ) const {
      switch ( type ) {
      case GfxData::gfxDataFull:
         return sizeof( GfxDataFull ) * m_nbrGfxTypes[ type ];
         break;
      case GfxData::gfxDataSingleSmallPoly:
         return sizeof( GfxDataSingleSmallPoly ) * m_nbrGfxTypes[ type ];
         break;
      case GfxData::gfxDataSingleLine:
         return sizeof( GfxDataSingleLine ) * m_nbrGfxTypes[ type ];
         break;
      case GfxData::gfxDataSinglePoint:
         return sizeof( GfxDataSinglePoint ) * m_nbrGfxTypes[ type ];
         break;
      case GfxData::gfxDataMultiplePoints:
         return sizeof( GfxDataMultiplePoints ) * m_nbrGfxTypes[ type ];
         break;
      }
      return 0;
   }
   uint32 getSizeOfType( GfxData::gfxdata_t type ) const {
      switch ( type ) {
      case GfxData::gfxDataFull:
         return sizeof( GfxDataFull );
         break;
      case GfxData::gfxDataSingleSmallPoly:
         return sizeof( GfxDataSingleSmallPoly );
         break;
      case GfxData::gfxDataSingleLine:
         return sizeof( GfxDataSingleLine );
         break;
      case GfxData::gfxDataSinglePoint:
         return sizeof( GfxDataSinglePoint );
         break;
      case GfxData::gfxDataMultiplePoints:
         return sizeof( GfxDataMultiplePoints );
         break;
      }
      return 0;
   }

   uint64 getMemoryUsageForType( GfxData::gfxdata_t type ) const {
      return m_memoryUsage[ type ];
   }

   uint64 getTotalMemoryUsage() const {
      return accumulate( m_memoryUsage, m_memoryUsage + 5, 0 );
   }
   uint64 getTotalSizeInDataBuffer() const {
      return accumulate( m_sizeInDataBuffer, m_sizeInDataBuffer + 5, 0 );
   }

   uint64 getSizeInDataBufferForType( GfxData::gfxdata_t type ) const {
      return m_sizeInDataBuffer[ type ];
   }

   /// @return string from type
   string getTypeString( GfxData::gfxdata_t type ) const {
      switch ( type ) {
      case GfxData::gfxDataFull:
         return "Full";
         break;
      case GfxData::gfxDataSingleSmallPoly:
         return "SSPoly";
         break;
      case GfxData::gfxDataSingleLine:
         return "SLine";
         break;
      case GfxData::gfxDataSinglePoint:
         return "SPoint";
         break;
      case GfxData::gfxDataMultiplePoints:
         return "MPoints";
         break;
      }
      return "unknown";
   }
private:
   uint32 m_nbrPolygons;
   uint64 m_nbrCoordinates[5];
   uint64 m_nbrGfxTypes[5];
   uint64 m_memoryUsage[5];
   uint64 m_sizeInDataBuffer[5];
   uint32 m_nbrData;
   uint32 m_nbrWithoutData;
};

int main(int argc, char **argv) {

   CommandlineOptionHandler coh( argc, argv, 1 );
   coh.setTailHelp(".m3 file");
   coh.setSummary("The map to create gfx stats.");
   // Parse command-line
   if ( !coh.parse() ) {
      throw MC2String( argv[0] ) + ": Error on commandline! (-h for help)";
   }

   const char *filename = coh.getTail( 0 );

   auto_ptr<GenericMap> map( GenericMap::createMap( filename ) );
   if ( map.get() == 0 ) {
      cout << "Could not open file: " << filename << endl;
      return EXIT_FAILURE;
   }


   //
   // collect gfx data
   //

   GfxDataStat stat;
   stat.count( map->getGfxData() );

   // for all items in map make a gfx sample
   for ( uint32 zoom = 0; zoom < NUMBER_GFX_ZOOMLEVELS; ++zoom ) {
      for ( uint32 i = 0; i < map->getNbrItemsWithZoom( zoom ); ++i ) {
         Item *item = map->getItem( zoom, i );
         if ( item == NULL )
            continue;
         stat.count( item->getGfxData() );
      }
   }

   //
   // print gfx data stat to stdout
   //

   fill_n( ostream_iterator<char>(cout, ""), 80, '-');
   cout << endl << "Map: " << STLStringUtility::basename( filename ) << endl;
   fill_n( ostream_iterator<char>(cout, ""), 80, '-');
   cout << endl;
   
   size_t w = 16, w2 = 10;
   cout << setw(w) << " ";
   for ( uint32 i =0; i < 5; ++i ) {
      GfxData::gfxdata_t type = static_cast<GfxData::gfxdata_t>(i);
      cout << setw(w2) << stat.getTypeString( type ) ;
   }
   cout << endl;

   cout << setw(w) << "count:";
   for ( uint32 i = 0; i < 5; ++i ) {
      GfxData::gfxdata_t type = static_cast<GfxData::gfxdata_t>(i);
      cout << setw(w2) << stat.getNbrGfxType( type );
   }
   cout << endl;

   cout << setw(w) << "sizeof:";
   for ( uint32 i = 0; i < 5; ++i ) {
      GfxData::gfxdata_t type = static_cast<GfxData::gfxdata_t>(i);
      cout << setw(w2) << stat.getSizeOfType( type );
   }
   cout << endl;

   cout << setw(w) << "mem usage:";
   for ( uint32 i = 0; i < 5; ++i ) {
      GfxData::gfxdata_t type = static_cast<GfxData::gfxdata_t>(i);
      cout << setw(w2) << stat.getMemoryUsageForType( type );
   }
   cout << endl;

   cout << setw(w) << "size databuff:";
   for ( uint32 i = 0; i < 5; ++i ) {
      GfxData::gfxdata_t type = static_cast<GfxData::gfxdata_t>(i);
      cout << setw(w2) << stat.getSizeInDataBufferForType( type );
   }
   cout << endl;

   cout << setw(w) << "sizeof x count:";
   for ( uint32 i = 0; i < 5; ++i ) {
      GfxData::gfxdata_t type = static_cast<GfxData::gfxdata_t>(i);
      cout << setw(w2) << stat.getEntireSizeOfType( type );
   }
   cout << endl;

   cout << setw(w) << "nbr coordinates:";
   for ( uint32 i = 0; i < 5; ++i ) {
      GfxData::gfxdata_t type = static_cast<GfxData::gfxdata_t>(i);
      cout << setw(w2) << stat.getNbrCoordinates( type );
   }
   cout << endl;

   fill_n( ostream_iterator<char>(cout, ""), 80, '-');
   cout << endl;

   w = 28; w2 = 10;
   uint32 total = stat.getNbrData() + stat.getNbrWithoutData();
   float32 percent = 100 * (float)(stat.getNbrWithoutData()) / (float)(total);
   cout << setw(w) << "Nbr GfxData: " << setw(w2) 
        << stat.getNbrData() << endl
        << setw(w) << "Nbr without GfxData: " << setw(w2)
        << stat.getNbrWithoutData() << "("<< percent << "%)" << endl
        << setw(w) << "Nbr Polygons: " << setw(w2) 
        << stat.getNbrPolygons()  << endl
        << setw(w) << "Nbr Coordinates: " << setw(w2) 
        << stat.getTotalNbrCoordinates() << endl
      /*
        << setw(w) << "Average nbr polygons per data: " << setw(w2)
        << stat.getAverageNbrPolygons() << endl
        << setw(w) << "Average nbr coords per data: " << setw(w2)
        << stat.getAverageNbrCoords() << endl
      */
        << setw(w) << "Total size in data buffer: " << setw(w2)
        << stat.getTotalSizeInDataBuffer() << endl
        << setw(w) << "Total size in memory: " << setw(w2) 
        << stat.getTotalMemoryUsage() << endl;

   fill_n( ostream_iterator<char>(cout, ""), 80, '-');
   cout << endl;


}
