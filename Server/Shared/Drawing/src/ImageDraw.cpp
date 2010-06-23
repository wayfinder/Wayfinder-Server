/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ImageDraw.h"

#include "Properties.h"
#include "ImageTable.h"

// ImageMagic
#include <magick/api.h>
#include <magick/blob.h>

namespace {

/// Converts an ImageCode to a png filename string
MC2String toPNGFilename( ImageTable::ImageCode image,
                         ImageTable::ImageSet imageSet ) {
   return MC2String( ImageTable::getImage( image, imageSet ) ) + ".png";
}

}

ImageDraw::ImageDraw( uint32 xSize, uint32 ySize ):
   m_width( xSize ),
   m_height( ySize )
{
}


ImageDraw::~ImageDraw() {
}

void 
ImageDraw::transferPixels( void* image ) {
   Image* imImage = static_cast< Image* >( image );

   PixelPacket* q = NULL;
   int quantumFactor = imImage->depth -8;
   for ( int y = 0 ; y < (int)m_height ; y++ ) {
      q = GetImagePixels( imImage, 0, y, m_width, 1 );
      if ( q == NULL ) {
         break;
      }
      unsigned char red = 0;
      unsigned char green = 0;
      unsigned char blue = 0;
      for ( int x = 0 ; x < (int)m_width ; x++ ) {
         getPixel( x, y, red, green, blue );
         PixelPacket col;
         col.red = int(red) << quantumFactor;
         col.green = int(green) << quantumFactor;
         col.blue = int(blue) << quantumFactor;
//         col.opacity = x;
         *q=col;
         q++;
      }
      if ( ! SyncImagePixels( imImage ) ) {
         break;
      }
   }
}


byte* 
ImageDraw::convertImageToBuffer( uint32& size, 
                                 ImageDrawConfig::imageFormat format )
{
   byte* out = NULL;

   Image* imImage = NULL;
   ImageInfo* imImageInfo = NULL;
   uint32 width = 0;
   uint32 height = 0;

   width = m_width;
   height = m_height;

   DEBUG8(char cStr[512]; clock_t m_startTime = clock(); );
   DEBUG8(m_startTime = clock(); );
   mc2dbg8 << "width " << width << " height " << height << endl;

   imImageInfo = CloneImageInfo( NULL ); // Create new
   GetImageInfo( imImageInfo ); // Initialize it

   // Set image width and height
   char* imImageSize = static_cast< char* > (
      AcquireMemory( 22 ) ); // 10 + 1 + 10 + 1
   sprintf( imImageSize, "%ux%u", width, height );
   imImageInfo->size = imImageSize;

   // Set imageformat
   strcpy( imImageInfo->magick, 
           ImageDrawConfig::imageFormatMagick[ format ] );

   imImageInfo->colorspace = RGBColorspace;

   mc2dbg8 << "magick " << imImageInfo->magick << endl;
   

   imImage = AllocateImage( imImageInfo );
   DEBUG8(sprintf(cStr, "Init Total CPU time: %fs.\n", 
                  ((float64)(clock() - m_startTime ))/CLOCKS_PER_SEC );
          mc2dbg << cStr << endl;);

   // Transfer pixels
   DEBUG8(m_startTime = clock(); );
   transferPixels( imImage );

   DEBUG8(sprintf(cStr, "Transfer pixels Total CPU time: %fs.\n", 
                  ((float64)(clock() - m_startTime ))/CLOCKS_PER_SEC );
          mc2dbg << cStr << endl;);
/*
   // Quantize to get used colors to reduce number colors
   DEBUG8(m_startTime = clock(););
   QuantizeInfo quantize_info;
   GetQuantizeInfo( &quantize_info );
   // Don't require less colors just remove unused
   quantize_info.number_colors = imImage->colors; 
   quantize_info.tree_depth = 8;
   QuantizeImage( &quantize_info, imImage );
   DEBUG8(sprintf(cStr, "Quantize Total CPU time: %fs.\n", 
                  ((float64)(clock() - m_startTime ))/CLOCKS_PER_SEC );
          mc2dbg << cStr << endl;);
*/
   DEBUG8( mc2dbg << "GetNumberColors " << GetNumberColors( imImage, NULL ) 
                  << " ~= " << imImage->colors << endl << endl;
   
          for ( uint32 i = 0 ; i < imImage->colors ; i++ ) {
             testpixel = imImage->colormap[ i ];
             mc2dbg << "imImage col: " << i << " RGB "
                    << (int)testpixel.red << "," 
                    << (int)testpixel.green << "," << (int)testpixel.blue 
                    << endl;
          } );

   // Create byte vector with imagedata of right format
   DEBUG8(m_startTime = clock(););
   ExceptionInfo err;
   size = 0;
   size_t imageSize = 0;
   void* res = ImageToBlob( imImageInfo, imImage, &imageSize, &err );
   size = static_cast<uint32>( imageSize );

   if ( res == NULL ) {
      mc2log << error << "ImageToBlob error: " << endl << err.description 
             << " why: " << err.reason << endl; 
   }
   out = (byte*)memcpy( new byte[ size ], res, size );
   LiberateMemory( &res );
   DestroyImage( static_cast<Image*>( imImage ) );
   DestroyImageInfo( static_cast< ImageInfo* > ( imImageInfo ) );
   DEBUG8(sprintf(cStr, "ImageToBlob Total CPU time: %fs.\n", 
                  ((float64)(clock() - m_startTime ))/CLOCKS_PER_SEC );
          mc2dbg << cStr << endl;);

   return out;
}

MC2String ImageDraw::getImageFullPath( const MC2String& filename ) {
   const char* imagesPath = Properties::getProperty( "IMAGES_PATH" );
   if ( imagesPath == NULL ) {
      imagesPath = "./";
   }
   return MC2String( imagesPath ) + "/" + filename;
}

MC2String ImageDraw::getPOISymbolFilename( ItemTypes::pointOfInterest_t type,
                                           ImageTable::ImageSet imageSet,
                                           byte extraPOIInfo ) {
   ImageTable::ImageCode image = ImageTable::NOIMAGE;
   switch ( type ) {
   case ItemTypes::atm : 
      image = ImageTable::ATM; break;
   case ItemTypes::golfCourse : 
      image = ImageTable::GOLF_COURSE; break;
   case ItemTypes::vehicleRepairFacility : 
      image = ImageTable::CAR_REPAIR; break;
         
   case ItemTypes::bank :
      image = ImageTable::BANK; break;
   case ItemTypes::casino :
      image = ImageTable::CASINO; break;
   case ItemTypes::cityCentre :
      if( extraPOIInfo == 2 ) {
         image = ImageTable::CITY_CENTRE_SQUARE;
      } else if( (extraPOIInfo == 3) || (extraPOIInfo == 4) ) {
         image = ImageTable::CITY_CENTRE;
      } else if( (extraPOIInfo == 5) || (extraPOIInfo == 6) ) {
         image = ImageTable::CITY_CENTRE;
      } else if( extraPOIInfo == 7 ) {
         image = ImageTable::CITY_CENTRE_SMALL_RED;
      } else if( extraPOIInfo == 8 ) {
         image = ImageTable::CITY_CENTRE_SMALL_PINK;
      } else if( (extraPOIInfo == 9) || (extraPOIInfo == 10) ) {
         image = ImageTable::CITY_CENTRE_POINT;
      } else if( extraPOIInfo == 11 ) {
         image = ImageTable::CITY_CENTRE_POINT;
      } else if( extraPOIInfo == 12 ) {
         image = ImageTable::CITY_CENTRE_POINT_SMALL;
      }
      break;
   case ItemTypes::commuterRailStation:
      image = ImageTable::COMMUTER_TRAIN; break;
   case ItemTypes::historicalMonument :
      image = ImageTable::HISTORICAL_MONUMENT; break;
   case ItemTypes::nightlife :
      image = ImageTable::NIGHTLIFE; break;
   case ItemTypes::postOffice :
      image = ImageTable::POST_OFFICE; break;
   case ItemTypes::skiResort :
      image = ImageTable::SKI_RESORT; break;
   case ItemTypes::theatre :
      image = ImageTable::THEATRE; break;
   case ItemTypes::touristAttraction :
      image = ImageTable::TOURIST_ATTRACTION; break;
      
   case ItemTypes::parkingGarage :
      image = ImageTable::PARKING_GARAGE; break;
   case ItemTypes::parkAndRide :
      image = ImageTable::PARK_AND_RIDE; break;
   case ItemTypes::openParkingArea :
      image = ImageTable::OPEN_PARKING_AREA; break;
   case ItemTypes::amusementPark :
      image = ImageTable::AMUSEMENT_PARK; break;
   case ItemTypes::groceryStore :
      image = ImageTable::GROCERY_STORE; break;
   case ItemTypes::petrolStation :
      image = ImageTable::PETROL_STATION; break;
   case ItemTypes::tramStation :
      image = ImageTable::TRAMWAY; break;
   case ItemTypes::ferryTerminal :
      image = ImageTable::FERRY_TERMINAL; break;
   case ItemTypes::cinema :
      image = ImageTable::CINEMA; break;
   case ItemTypes::busStation :
      image = ImageTable::BUS_STATION; break;
   case ItemTypes::railwayStation :
      image = ImageTable::RAILWAY_STATION; break;
   case ItemTypes::airport :
      image = ImageTable::AIRPORT; break;
   case ItemTypes::restaurant :
      image = ImageTable::RESTAURANT; break;
   case ItemTypes::hotel :
      image = ImageTable::HOTEL; break;
   case ItemTypes::touristOffice :
      image = ImageTable::TOURIST_OFFICE; break;
   case ItemTypes::hospital : {
      if ( extraPOIInfo == 1 ) {
         // Turkish special.
         image = ImageTable::TURKISH_HOSPITAL;
      } else {
         // Normal hospital.
         image = ImageTable::HOSPITAL;
      }
         
      break;
   }
         
   case ItemTypes::rentACarFacility:
      image = ImageTable::RENTACAR_FACILITY; break;
   case ItemTypes::policeStation:
      image = ImageTable::POLICE_STATION; break;

   case ItemTypes::wlan :
      image = ImageTable::WIFI_HOTSPOT; break;

   case ItemTypes::museum :
   case ItemTypes::sportsActivity :
   case ItemTypes::recreationFacility :
   case ItemTypes::restArea :
   case ItemTypes::tollRoad :
   case ItemTypes::university :
   case ItemTypes::winery :
   case ItemTypes::cityHall :
   case ItemTypes::library :
   case ItemTypes::school :
   case ItemTypes::iceSkatingRink : 
   case ItemTypes::marina : 

   case ItemTypes::bowlingCentre : 
   case ItemTypes::courtHouse :
   default:
      break;
   }
   if ( image == ImageTable::NOIMAGE ) {
      return "";
   }
   else {
      return toPNGFilename( image, imageSet );
   }
}

MC2String ImageDraw::getSymbolFilename( GfxFeature::gfxFeatureType type,
                                        ImageTable::ImageSet imageSet ) {
   switch ( type ) {
      // Special for ROUTE_PARK_CAR
   case GfxFeature::PARK_CAR :
      return toPNGFilename( ImageTable::PARK_AND_WALK, imageSet ); 
      break;
   default:
      break;
   }
   return "";
}

MC2String ImageDraw::getSymbolFilename( DrawSettings::symbol_t sym,
                                        const DrawSettings& settings,
                                        const GfxFeature* feature,
                                        ImageTable::ImageSet imageSet ) {
   MC2String filename;
   switch ( sym ) {
   case DrawSettings::ROUTE_ORIGIN_SYMBOL :
      mc2dbg << "ImageDraw::drawSymbol ROUTE_ORIGIN_SYMBOL" << endl;
      // Drive, Walk or bike and starting left or right and U-turn
      if ( settings.m_transportationType == ItemTypes::drive ) {
         if ( settings.m_startingAngle <= 127 ) {
            if ( settings.m_initialUTurn ) {
               filename = toPNGFilename( ImageTable::CAR_RIGHT_U, imageSet );
            } else {
               filename = toPNGFilename( ImageTable::CAR_RIGHT, imageSet );
            }
         } else { 
            if ( settings.m_initialUTurn ) {
               filename = toPNGFilename( ImageTable::CAR_LEFT_U, imageSet );
            } else {
               filename = toPNGFilename( ImageTable::CAR_LEFT, imageSet );
            } 
         }
      } else if ( settings.m_transportationType == ItemTypes::walk ) {
         filename = toPNGFilename( ImageTable::MAN, imageSet );
      } else if ( settings.m_transportationType == ItemTypes::bike ) {
         if ( settings.m_startingAngle <= 127 ) {
            filename = toPNGFilename( ImageTable::BIKE_RIGHT, imageSet );
         } else { 
            filename = toPNGFilename( ImageTable::BIKE_LEFT, imageSet );
         }
      } else { // Default
         filename = toPNGFilename( ImageTable::ROUTE_ORIG, imageSet );
      }
      break;
   case DrawSettings::ROUTE_DESTINATION_SYMBOL : {
      mc2dbg << "ImageDraw::drawSymbol ROUTE_DESTINATION_SYMBOL" 
              << endl;
      filename = toPNGFilename( ImageTable::ROUTE_DEST, imageSet );
   } break;
   case DrawSettings::ROUTE_PARK_SYMBOL:
      mc2dbg << "ImageDraw::drawSymbol ROUTE_PARK_SYMBOL" << endl;
      filename = ImageDraw::getSymbolFilename( settings.m_featureType, 
                                               imageSet );
      break;   
   case DrawSettings::SQUARE_3D_SYMBOL:
      return "";
   case DrawSettings::SMALL_CITY_SYMBOL :
      return "";
   case DrawSettings::POI_SYMBOL : {
      const GfxPOIFeature* poi = 
         static_cast<const GfxPOIFeature*>( feature );

      filename = getPOISymbolFilename( settings.m_poiType,
                                       imageSet,
                                       poi->getExtraInfo() );
      break;
   }
   case DrawSettings::PIN : {
      mc2dbg << "ImageDraw::drawSymbol PIN" << endl;
      filename = toPNGFilename( ImageTable::MAP_PIN_CENTERED, imageSet );
      break;
   }
   case DrawSettings::ROADWORK : {
      mc2dbg << "ImageDraw::drawSymbol ROADWORK" << endl;
      filename = toPNGFilename( ImageTable::ROAD_WORK, imageSet );
      break;
   }
   case DrawSettings::SPEED_CAMERA : {
      mc2dbg << "ImageDraw::drawSymbol SPEED_CAMERA" << endl;
      filename = toPNGFilename( ImageTable::SPEED_CAMERA, imageSet );
      break;
   }
   case DrawSettings::USER_DEFINED_SPEED_CAMERA : {
      mc2dbg << "ImageDraw::drawSymbol USER_DEFINED_SPEED_CAMERA"
              << endl;
      filename = toPNGFilename( ImageTable::USER_DEFINED_SPEED_CAMERA, 
                                imageSet );

      break;
   }
   case DrawSettings::DANGER : {
      mc2dbg << "ImageDraw::drawSymbol DANGER" << endl;
      filename = toPNGFilename( ImageTable::TRAFFIC_INFORMATION, imageSet );
      break;
   }
   case DrawSettings::USER_DEFINED :   
      mc2dbg << "ImageDraw::drawSymbol USER_DEFINED " 
              << settings.m_symbolImage << endl;
      filename = settings.m_symbolImage;
      break;
   } // end of switch

   if ( filename.empty() ) {
      return "";
   }

   return getImageFullPath( filename );
}
