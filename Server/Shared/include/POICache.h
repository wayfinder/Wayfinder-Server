/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POI_CACHE
#define POI_CACHE

#include "config.h"
#include "MC2Point.h"
#include "MC2String.h"
#include "PixelBox.h"
#include "GDCropTransparent.h"
#include "Properties.h"

#include <map>

/**
 * Describes the data of a poi, where center is located,
 * height and width.
 */
class POIData {

public:

   /**
    * Constructor, sets everything to 0,
    */
   POIData( ) : m_width( 0 ), m_height( 0 ), m_offset( MC2Point( 0, 0 ) )    
   { }

   /**
    * Constructor.
    * @param width The width of the image
    * @param height The height of the image
    * @param offset The offset point, based on center of visible rect in
    *               relation to the whole rect of the image.
    */
   POIData( int32 width, int32 height, const MC2Point& offset ) :
      m_width( width ), m_height( height ), m_offset( offset ) 
   { }

   /**
    * Sets the members of an allready created POIData object.
    * @param width The width of the image
    * @param height The height of the image
    * @param offset The offset point, based on center of visible rect in
    *               relation to the whole rect of the image.
    */
    
   void set( int32 width, int32 height, const MC2Point& offset = MC2Point( 0, 0 ) )
   {
      m_width  = width;
      m_height = height;
      m_offset = offset;
   }

   /**
    * Print on ostream.
    *
    * @param stream The stream to print on.
    * @param poiData  The PoiData to print.
    * @return The stream.
    */
   inline friend ostream& operator<< ( ostream& stream, 
                                       const POIData& coord );

   /**
    * Width of the poi
    */
   int32 m_width;

   /**
    * Height of the poi
    */
   int32 m_height;

   /**
    * The offset point for the poi. Based on the center position
    * of the complete image in relation to the center position of
    * the visible rect.
    */
   MC2Point m_offset;
};


/**
 * Contains pixelboxes to be checked for overlap, prevents 
 * unnecessary loading from disk.
 *   
 */
class POICache {

   /// Type of storage
   typedef std::map<MC2String, POIData> cache_t;
   
public:

   /**
    *   Clear everything.
    */
   void clear() 
   {
      m_dataStore.clear();
   }

   /**
    * Searches for a poi in the cache and if found returning
    * a pixelPox and an offsetPoint for the poi.
    * @param name The name of the poi to search for.
    * @param box The pixelBox that will contain the size for the found poi.
    * @param offsetPoint The offset for the poi.
    * @return True if poi was found in the cache.
    *         False if poi was NOT found in the cace.
    */
   bool get( const MC2String& name, PixelBox& box, MC2Point& offsetPoint ) 
   {
      cache_t::const_iterator it = m_dataStore.find( name );
      if ( it == m_dataStore.end() ) {
         return false;
      }         

      // Hard coded starting point, should not matter since the point
      // it self is not being used, only the size of the pixelBox.
      MC2Point p1( 0, 0 );
      MC2Point p2( p1.getX() + it->second.m_width, 
                   p1.getY() + it->second.m_height );
      box.set( p1, p2 );
      offsetPoint = it->second.m_offset;
      return true;
   }

   /**
    * Returns the complete cache.
    * @param in_boxes Reference to a map that will contain the cache.
    */
   void getCache( cache_t& in_boxes ) 
   {
      in_boxes = m_dataStore;
   }

   /**
    * Searches for a poi in the cache.
    * @param name Name of the poi to look for.
    * @param data Will contain the found poi.
    * @return True if poi was found in the cache.
    *         False if poi was NOT found in the cache.
    */
   bool find( const MC2String& name, POIData& data )
   {
      cache_t::const_iterator it = m_dataStore.find( name );
      if ( it == m_dataStore.end() ) {
         return false;
      }
      data = it->second;
      return true;
   }

   /**
    * Inserts a poi in the cache.
    * @param name The name of the poi to be inserted, key for the POIData.
    * @param data The POIData to be inserted into the cache.
    */
   void insert( const MC2String& name, POIData& data )
   {
      m_dataStore.insert( make_pair( name, data ) ); 
   }

   /**
    * Creates a POIData object and inserts it into the cache.
    * @param name The name of the poi to be inserted, key for the POIData.
    * @param width The width of the POIData.
    * @param height The height of the POIData.
    * @param offset The offset point for the POIData.
    */
   void insert( const MC2String& name, 
                int32 width, 
                int32 height, 
                const MC2Point& offset = MC2Point( 0, 0 ) ) 
   {
      m_dataStore.insert( make_pair( name, POIData( width, height, offset ) ) );
   }

   /**
    * Creates a POIObject, calculates the visible size of the image and
    * calcs the offset point for the image rect.
    * @name The name of the poi to be inserted, key for the POIData.
    * @data POIData that will contain the inserted POIData when calculation
    *       and insertion is completed.
    * @retun True if the operation was successful.
    *        False if NOT.
    */
   bool createAndInsert( const MC2String& name, POIData& data )
   {
      MC2String imagesPath( Properties::getProperty( "IMAGES_PATH" ) );
      if ( imagesPath.empty() ) {
         imagesPath = "./";
      } else if ( imagesPath.find( "/", imagesPath.length() - 1 ) == 
                  MC2String::npos ) {
         // Ending / in path does not exist
         imagesPath += '/';
      } 

      imagesPath += name;
      MC2String tmp ( ".png" );
      imagesPath += ( tmp );

      MC2Point offset( 0, 0 );
      GDUtils::CutPoints cutPoints;
      if ( !GDUtils::cropTransparentOffset( imagesPath, offset, &cutPoints ) ) {
         return false;
      }
            
      data.set( cutPoints.getBottomRight().getX() - cutPoints.getTopLeft().getX(),
                cutPoints.getBottomRight().getY() - cutPoints.getTopLeft().getY(),
                offset );

      insert( name, data );      
      return true;
   }

private:

   /// The non-overlapping boxes are stored here.
   cache_t m_dataStore;      
};




inline ostream& operator<< ( ostream& stream, const POIData& coord )
{
   return stream << "( Width: " << coord.m_width << ", Height: " 
                 << coord.m_height << ", OffsetPosition: " 
                 << coord.m_offset << endl;
}

#endif
