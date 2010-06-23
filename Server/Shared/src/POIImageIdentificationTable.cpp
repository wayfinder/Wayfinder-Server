/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "POIImageIdentificationTable.h"

#include "ServerTileMapFormatDesc.h"
#include "ItemTypes.h"
#include "File.h"
#include "TileMapParams.h"
#include "STLUtility.h"
#include "Array.h"

#include <algorithm>


/**
 * Counter using a set of code characters.
 * Can handle "unlimited" number. Kind of overkill here...but what da hey
 */
struct CodeCounter {
   CodeCounter():
      // using tilemap params code chars.
      m_codeChars( TILEMAP_CODE_CHARS, 
                   strlen( TILEMAP_CODE_CHARS ) ) { 
      // setup first value
      m_values.push_back( 0 );
   }

   /// increase the counter.
   MC2String operator()() {
      // compose return string
      MC2String retStr;
      for ( int32 i = m_values.size() - 1; i >= 0; --i ) {
         retStr += m_codeChars[ m_values[ i ] ];
      }
      // increase value
      incValue( 0 );

      return retStr;
   }

private:

   /**
    * increase the counter index.
    * @param index Index into m_values to be increased.
    */
   void incValue( uint32 index ) {
      if ( m_values[ index ] + 1 == m_codeChars.size() ) {
         // reset to 0 and increase next code character
         m_values[ index ] = 0;
         // need to add another code character?
         if ( index + 1 == m_values.size() ) {
            m_values.push_back( 0 );
         } else {
            // increase next code character
            incValue( index + 1 );
         }
      } else {
         // increase normal
         m_values[ index ]++;
      }
   }

   /// Current code values
   std::vector<uint32> m_values;
   /** 
    * Holds the code characters.
    * It's a simple array since we don't need to own
    * the buffer.
    */
   STLUtility::Array< const char > m_codeChars;
};


POIImageIdentificationTable::
POIImageIdentificationTable( const ServerTileMapFormatDesc& desc ) {

   // setup code tables
   // OBS! strict ordering!, 
   // Do NOT comment out any line.
   // If a line has to be removed an empty string can take its place
   // including the cc();
   // e.g { ItemTypes::nbr_pointOfInterest, "", cc(), false }
   CodeCounter cc;
   struct PreTableType {
      ItemTypes::pointOfInterest_t m_poiType;
      ImageName m_image;
      CodeType m_code;
      bool m_isUnique;
      bool m_showInSearches;
   } types[] = {
      { ItemTypes::postOffice, "sepostoffice", cc(), false, false },
      { ItemTypes::postOffice, "sepostofficebusinesscenter", cc(), false, false },
      { ItemTypes::petrolStation, "bestprice", cc(), false, false },
      // Add when fixed...
      { ItemTypes::touristAttraction, "toureiffel", cc(), true, false },
      { ItemTypes::postOffice, "stampseller", cc(), false, false },
      { ItemTypes::postOffice, "mailbox", cc(), false, false },
      { ItemTypes::subwayStation, "germanyberlinsubway", cc(), false, true },
      { ItemTypes::subwayStation, "uklondonsubway", cc(), false, true },
      { ItemTypes::subwayStation, "franceparissubway", cc(), false, true },
      { ItemTypes::subwayStation, "portugallisbonsubway", cc(), false, true },
      { ItemTypes::subwayStation, "portugalportosubway", cc(), false, true },
      { ItemTypes::subwayStation, "spainmadridsubway", cc(), false, true },
   };

   const uint32 typesSize = sizeof ( types ) / sizeof ( types[ 0 ] );

   // Build the table using the server tile map format desc.
   // The server tile map format desc is used to fetch the prefix for
   // images for each poi type.
   for ( uint32 i = 0; i < typesSize; ++i ) {
      if ( types[ i ].m_image.empty() ) {
         continue;
      }

      MC2String imageName;
      const char* imagePrefix = NULL;
      // first lets see if we need an image.
      // i.e if the image is a unique image we dont want prefix.
      if ( types[ i ].m_isUnique ) {
         imageName = types[ i ].m_image;
      } else {
         // setup image name and prefix
         imagePrefix = desc.getPOIImageName( types[ i ].m_poiType );
         imageName = "_" + types[ i ].m_image;
      }

      m_images.push_back( TableType( types[ i ].m_poiType,
                                     imageName, imagePrefix,
                                     types[ i ].m_code,
                                     types[ i ].m_showInSearches ) );
   }

   sort( m_images.begin(), m_images.end(), SortByImageNameAndPOI() );

   // fill upp codes with default data; 0, 1, 2, 3, 4 ... n
   // and then sort it according to code in TableType.
   m_codes.resize( m_images.size() );
   generate( m_codes.begin(), m_codes.end(), 
             STLUtility::Counter<uint32>( 0 ) );

   // this must be done after sorting of images.
   sort( m_codes.begin(), m_codes.end(), SortByCode( m_images ) );

   // setup the code index in the image table.
   for ( uint32 i = 0; i < m_codes.size(); ++i ) {
      m_images[ m_codes[ i ] ].setCodeIndex( i );
   }
}

POIImageIdentificationTable::ImageName 
POIImageIdentificationTable::decode( const CodeType& code ) const {
   // search for image name matching the code
   CodeToImageTable::const_iterator it = 
      std::lower_bound( m_codes.begin(), m_codes.end(),
                        code, CodeLess( m_images ) );
   if ( it == m_codes.end() ||
        m_images[ *it ].getCode() != code ) {
      // failed to find code
      return CodeType();      
   }

   return m_images[ *it ].getFullImageName();
}

POIImageIdentificationTable::ImageName 
POIImageIdentificationTable::
getFullImageName( uint32 poiType, const ImageName& shortImageName ) const {
   // search for a matching image name
   ImageToCodeTable::const_iterator it = 
      decomposeFindFirst( poiType, shortImageName );
   if ( it == m_images.end() ) {
      // no image name found; lets fallback to poi type default image
      return findPrefix( poiType );
   }

   return it->getFullImageName();
}

POIImageIdentificationTable::ImageName 
POIImageIdentificationTable::
getSearchResultImageName( uint32 poiType, 
                          const ImageName& shortImageName ) const {
   // search for a matching image name
   ImageToCodeTable::const_iterator it = 
      decomposeFindFirst( poiType, shortImageName );
   if ( it != m_images.end() && it->showInSearches() ) {
      return MC2String("cat_")+it->getFullImageName();
   }
   else {
      return "";
   }
}

POIImageIdentificationTable::CodeType 
POIImageIdentificationTable::
encode( uint32 poiType, const ImageName& filename ) const {
   // strip the filename into code bits and try to find it
   ImageToCodeTable::const_iterator imageIt = 
      decomposeFindFirst( poiType, filename );
   if ( imageIt == m_images.end() ) {
      // did not find it; lets fallback to poi type default image
      return findPrefix( poiType );
   }

   return imageIt->getCode();
}


bool 
POIImageIdentificationTable::
validateImageFilesOnDisc( const MC2String& path ) const {
   // validate all PNG files on disc
   bool failed = false;
   ImageToCodeTable::const_iterator it = m_images.begin();
   for ( ; it != m_images.end(); ++it ) {
      MC2String imageName = path + "/" + it->getFullImageName() + ".png";
      if ( ! File::fileExist( imageName ) ) {
         mc2log << error << "[POIImageIdentificationTable] missing image: "
                << imageName << endl;
         failed = true;
      }
   }
   return ! failed;
}

uint32
POIImageIdentificationTable::
getCodeIndex( uint32 poiType, const ImageName& longImageName ) const {
   ImageToCodeTable::const_iterator it = 
      decomposeFindFirst( poiType, longImageName );
   if ( it == m_images.end() ) {
      return MAX_UINT32;
   }

   return it->getCodeIndex();
}


void POIImageIdentificationTable::
getCodeTable( vector<CodeType>& codeTable ) const {
   codeTable.resize( m_codes.size() );
   for ( uint32 i = 0; i < m_codes.size(); ++i ) {
      codeTable[ i ] = m_images[ m_codes[ i ] ].getCode();
   }
}


POIImageIdentificationTable::ImageToCodeTable::const_iterator 
POIImageIdentificationTable::
findImage( uint32 poiType, const ImageName& shortImageName ) const {
   // search for code matching the image name and poi type
   ImageToCodeTable::const_iterator it = 
      std::lower_bound( m_images.begin(), m_images.end(),
                        shortImageName, ImageLess( poiType ) );
   if ( it == m_images.end() ||
        it->getImageName() != shortImageName ||
        it->getPOIType() != poiType ) {
      return m_images.end();
   }
   return it;
}

inline 
MC2String POIImageIdentificationTable::findPrefix( uint32 poiType ) const {
   ImageToCodeTable::const_iterator imageIt = 
      std::lower_bound( m_images.begin(), m_images.end(),
                        "", ImageLess( poiType ) );
   // found a first matching default icon
   if ( imageIt != m_images.end() &&
        imageIt->getPOIType() == poiType &&
        imageIt->getPrefix() != NULL ) {
      return imageIt->getPrefix();
   }

   return MC2String();
}

namespace {
/**
 * Strip everything after the last dash. Example: "_something_dude" => "_something" .
 * @return true if a dash was found and stripped 
 */
inline bool stripAfterLastDash( MC2String& str ) {
   // remove the last part of the "_*"
   size_t pos = str.find_last_of( '_' );
   if ( pos == MC2String::npos ) {
      // nothing more to do
      return false;
   }
   str = str.substr( 0, pos );
   return true;
}

}

inline  
POIImageIdentificationTable::ImageToCodeTable::const_iterator
POIImageIdentificationTable::
decomposeFind( uint32 poiType, MC2String& filename ) const {
   ImageToCodeTable::const_iterator it = findImage( poiType, filename );

   if ( it == m_images.end() ) {
      // strip the icon sub type
      if ( ! ::stripAfterLastDash( filename ) ) {
         // no dash to strip, we are all done,  but not succesfully done.
         return m_images.end();
      }
      // Lets see if the new stripped image exist
      return decomposeFind( poiType, filename );
   }

   return it;
}


inline 
POIImageIdentificationTable::ImageToCodeTable::const_iterator
POIImageIdentificationTable::
decomposeFindFirst( uint32 poiType, const MC2String& inFilename ) const {
   MC2String filename = inFilename;
   // lets see first if we have a unique image
   MC2String imageName;
   size_t pos = filename.find( "/" );
   if ( pos != MC2String::npos ) {
      ImageToCodeTable::const_iterator it = 
         findImage( poiType, filename.substr( 0, pos ) );
      if ( it == m_images.end() ) {
         // remove unique icon name from string and try the other 
         // side of the "/" as the icon name
         filename = filename.substr( pos + 1, filename.size() );
      } else {
         return it;
      }

   }
   // See if the other side of the "/" has a valid image code
   return decomposeFind( poiType, filename );
}
