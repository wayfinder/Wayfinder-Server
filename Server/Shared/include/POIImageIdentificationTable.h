/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POIIMAGETRANSLATIONTABLE_H
#define POIIMAGETRANSLATIONTABLE_H

#include "config.h"
#include "MC2String.h"

#include <vector>

class ServerTileMapFormatDesc;

/**
 * Encodes and decodes image filenames for the client.
 * It makes a compact code from a long image filename.
 *
 * The class needs a server tile map format descriptor at startup.
 * This descriptor must not be destroyed during the life time of this
 * object. This is because the table uses c-strings from the descriptor.
 * Also note that all functions must be reentrables, this is to make them all
 * easier to use with threads.
 *
 * Usage:
 * \code
 *  ServerTileMapFormatDesc desc( STMFDParams( LangTypes::english, false ) );
 *  desc.setData();
 *  POIImageIdentificationTable table( desc );
 *  if ( table.validateImageFilesOnDisc( imagePath ) ) {
 *     MC2String codeForGas = table.encode( ItemTypes::shop, "_gas" );
 *     // ...send code to client...
 *  } else {
 *     // failed to validate images on disc
 *  }
 *  // ...
 *  // ... got some code from client so lets decode it...
 *  MC2String fullFilename = table.decode( code );
 *  // .. load image and return it ...
 *  
 * \endcode
 * Note that full filename is returned when decoding the code, and NOT the
 * original code "_gas". This was mostly done because of convenience in other
 * parts of the code.
 * 
 *
 * The filename that the encode function takes must be coded with the following
 * code:
 * "uniquefile/_maprights_iconsubtype"
 * The "/" works as a separator between unique filename and common filename.
 * The "_" is a separator for maprights ( or in other words; icon subset ) and
 * icon subtype.
 * The "uniquefile" and "_maprights" are optional.
 * The "uniquefile" is a unique file that encode should try to find first
 * in the image table. If the unique file does not exist in the table the
 * filename is cut at the "/" ( resulting in a new filename
 * "_maprights_iconsubtype" ). The new filename is then matched against the
 * image table again and if its not found the filename will be cut at the last
 * "_" dash, resulting in "_maprights". Encode tries to find "_maprights" in
 * the table, and if found the a code will be returned.
 * If the encode will fail to find a matching code for the input filename the
 * fallback will be the default poitype from the search tile map format
 * descriptor.
 * For example:
 * \code
 * encode( ItemTypes::petrolStation, "gardstangamacken/_gas" ) 
 * \endcode
 * Here unique filename is "gardstangamacken" and the iconsubtype is "_gas".
 * ( Notice that there is no special mapright ( special icon subset ) here ).
 * So first it will search for "gardstangamacken" wich does not exist; then for
 * "_gas" which exist but the poi type does not match. So here it would 
 * fallback to server tile map format descriptor for the poi type 
 * ( petrolStation) and return a real image name instead of a code. The code 
 * would then be "tat_petrolstation".
 *
 * @see MapUtility
 * @see TileMapCreator
 * @see GfxFeatureMapUtility
 */
class POIImageIdentificationTable {
public:
   typedef MC2String ImageName; ///< image filename type
   typedef MC2String CodeType; ///< type of the code

   /// @param desc a default descriptor to fetch image names from.
   explicit
   POIImageIdentificationTable( const ServerTileMapFormatDesc& desc );

   /**
    * Decode a image code. Note that this will NOT return the original encoded
    * string but the full filename instead. 
    * 
    * @return The FULL image name to use for this code. If it's empty then the
    * code did not match anything in the table.
    */
   ImageName decode( const CodeType& code ) const;

   /**
    * Encode an image name to a code or to the fallback poi type image name.
    * @param poiType The poi type that should be associated with this image.
    * @param filename The image name without file ending and without 
    *                 poi prefix. This can be the "full" specially coded image
    *                 name.
    *                 For instance: "gardstangamacken/_petrol_gas"
    * @return code for this image name. If empty then the filename did not
    *         exist in the table and the poiType did not have an image name
    *         server tile map format descriptor.
    *         
    */
   CodeType encode( uint32 poiType, const ImageName& filename ) const;

   /**
    * Determines full image name from a short image name. This is a variant
    * of the encode function ( @see encode )
    * Example: 
    * \code
    * getFullImageName( ItemTypes::shop, "_gas" );
    * \endcode
    * might return "tat_petrolstation_gas".
    *
    * @param poiType the poi type of the image name.
    * @param shortImageName the short version of the image name.
    * @return full image name
    */
   ImageName getFullImageName( uint32 poiType,
                               const ImageName& shortImageName ) const;

   /**
    * Determines the full image name to use in search result lists.
    * If we shouldn't show this special icon in search results
    * (probably because we don't have a suitable version of the icon),
    * this function returns the empty string.
    *
    * The difference between icons in the map and in the search results
    * is typically that the icon for the search results is just square
    * without a pointy bit.
    *
    * Example:
    * \code
    * getSearchResultImageName( ItemTypes::shop, "_astore" );
    * \endcode
    * might return "cat_tat_shop_astore".
    *
    * @param poiType the poi type of the image name.
    * @param shortImageName the short version of the image name.
    * @return search result image name or empty string.
    */
   ImageName getSearchResultImageName( uint32 poiType,
                                       const ImageName& shortImageName ) const;

   /**
    * Prints the missing image filenames and returns false if it finds any 
    * missing filenames.
    * @param path the path to the images.
    * @return false if there are missing files, else true.
    */
   bool validateImageFilesOnDisc( const MC2String& path ) const;

   /**
    * Determine a code index from poi type and longImageName.
    * @param poiType the real poi type of the requested image
    * @param longImageName the special coded image name from map module. @see encode
    * @return code index in code table or MAX_UINT32 if it fails to find image.
    */
   uint32 getCodeIndex( uint32 poiType, const ImageName& longImageName ) const;

   /// @param codeTable returns a sorted vector of codes.
   void getCodeTable( vector<CodeType>& codeTable ) const;

private:
    /**
    * Groups an image name with its poi type, code 
    * and default poi image name ( image prefix ).
    */
   class TableType {
   public:
      /**
       * @param poiType the point of interest type that this image name
       *                represent.
       * @param name image name suffix. 
       * @param prefix the image prefix. ( from the poi type ).
       * @param code a unique code for this image name.
       */
      TableType( uint32 poiType,
                 const ImageName& name,
                 const char* imagePrefix,
                 const CodeType& code,
                 bool showInSearches ):
         m_poiType( poiType ),
         m_imageName( name ),
         m_imagePrefix( imagePrefix ),
         m_code( code ),
         m_codeIndex( 0 ),
         m_showInSearches( showInSearches ) { }
      /// @return poi type.
      uint32 getPOIType() const { return m_poiType; }
      /// @return unique code for this image
      const CodeType& getCode() const { return m_code; }
      /// @return image name without file ending
      const ImageName& getImageName() const { return m_imageName; }
      /**
       * A prefix for the full image name. This is the fallback image name
       * taken from ServerTileMapFormatDesc. @see ServerTileMapFormatDesc
       * @return prefix for the full image name. 
       */
      const char* getPrefix() const { return m_imagePrefix; }
      /// @return the full image name with suffix and prefix but 
      ///         without file ending.
      ImageName getFullImageName() const { 
         if ( m_imagePrefix ) {
            return MC2String( getPrefix() ) + getImageName();
         }
         return getImageName();
      }

      /// @return index into the code to image table
      uint32 getCodeIndex() const { return m_codeIndex; }

      /// Set the index into the code to image table.
      void setCodeIndex( uint32 codeIndex ) { 
         m_codeIndex = codeIndex;
      }
      
      /// @return whether the image should be shown in search results
      bool showInSearches() const { return m_showInSearches; }

   private:
      /// The POI type to be prefixed on the image name.
      uint32 m_poiType;
      ImageName m_imageName; ///< Image name on disc
      const char* m_imagePrefix; ///< Image name prefix
      CodeType m_code; ///< Code of the image name
      uint32 m_codeIndex; ///< index into the code to image table.
      bool m_showInSearches; ///< should this image be shown in search results?
   };

  

   /** @return poi image prefix for table type, from server tile map format
    *          desc.
    */
   inline MC2String findPrefix( uint32 poiType ) const;

   /// Image filename to code table type.
   typedef std::vector<TableType> ImageToCodeTable; 

   /** Code to image filename table type, index points into ImageToCodeTable.
    * @see ImageToCodeTable
    */
   typedef std::vector<uint32> CodeToImageTable;

   /**
    * Finds iterator to image table type that matches the poi type and short
    * image name.
    * @param poiType the point of interest type for the return image
    * @param shortImageName short version of the full image name
    * @return iterator into image to code table.
    */
   ImageToCodeTable::const_iterator 
   findImage( uint32 poiType, const ImageName& shortImageName ) const;

   /**
    * Cut the inFilname at the last "_" point and try to find it in the table.
    * @param poiType The poi type
    * @param orgFilename original filename. eg. "_petrol_gas" without the
    *        unique filename. i.e no "gardstangamacken" @see findCode
    * @return iterator to table type containing the image information.
    */
   inline
   ImageToCodeTable::const_iterator
   decomposeFind( uint32 poiType, MC2String& inFilename ) const;
   
   /// Iterator or image value, if iterator is invalid then the string can be.
   typedef pair<ImageToCodeTable::const_iterator, MC2String> IteratorOrImage;

   /**
    * Cuts up the inFilename into "unique" and "_mapright_iconsubtype" pieces
    * and tries to find one of them.
    * @see decomposeFind
    * @param poiType the poi type of the coded image name.
    * @param inFilename the entire specially coded image name.
    * @return iterator (pair.first) to the image table type. or the unique
    *         image name (pair.second).
    */
   inline 
   ImageToCodeTable::const_iterator
   decomposeFindFirst( uint32 poiType, const MC2String& inFilename ) const;

   /// less than comparable functor for image filenames.
   struct ImageLess {
      ImageLess( uint32 poiType ):
         m_poiType( poiType ) {
      }
      bool operator () ( const TableType& type, const ImageName& name ) {
         if ( type.getPOIType() != m_poiType ) {
            return type.getPOIType() < m_poiType;
         }
         return type.getImageName() < name;
      }

      bool operator () ( const ImageName& name, const TableType& type ) {
         if ( m_poiType != type.getPOIType() ) {
            return m_poiType < type.getPOIType();
         }
         return name < type.getImageName();
      }

      uint32 m_poiType;
   };

   /** For sorting the ImageToCode table in image name order. 
    * Less than operation.
    */
   struct SortByImageNameAndPOI {
      bool operator () ( const TableType& a, const TableType& b ) {
         if ( a.getPOIType() != b.getPOIType() ) {
            return a.getPOIType() < b.getPOIType();
         }
         return a.getImageName() < b.getImageName();
      }
   };

   /// For sorting the CodeToImageTable in Code index order
   struct SortByCode {
      SortByCode( const ImageToCodeTable& table ):
         m_table( table ) {
      }
      /// Does a less than operator on code at index A and index B
      bool operator () ( uint32 indexA, uint32 indexB ) {
         return m_table[ indexA ].getCode() < m_table[ indexB ].getCode();
      }

      const ImageToCodeTable& m_table;
   };

   /// less than comparable functor for codes
   struct CodeLess {
      CodeLess( const ImageToCodeTable& table ):
         m_table( table ) { }

      bool operator () ( uint32 index, const CodeType& code ) {
         return m_table[ index ].getCode() < code;
      }

      bool operator () ( const CodeType& code, uint32 index ) {
         return code < m_table[ index ].getCode();
      }

      const ImageToCodeTable& m_table; ///< lookup table for indexes
   };

   /// image filename to code mapping
   ImageToCodeTable m_images;
   /// code to image filename mapping into m_images
   CodeToImageTable m_codes;
};

#endif // POIIMAGETRANSLATIONTABLE_H
