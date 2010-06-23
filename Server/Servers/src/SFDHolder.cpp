/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SFDHolder.h"
#include "BitBuffer.h"
#include "MC2SimpleString.h"
#include "SFDLoadableHeader.h"
#include <stdio.h>
#include "File.h"
#include "FileException.h"
#include "Directory.h" // FileUtils::Directory
#include "TileMapParams.h"
#include "TileMapParamTypes.h"
#include "StringUtility.h"
#include "TileCollectionNotice.h" // To make a SFDHeader
#include "TileMap.h"
#include "DeleteHelpers.h"
#include "SFDMultiBufferReader.h"
#include "TileMapBufferHolder.h"
#include "ServerTileMap.h"
#include "ServerTileMapFormatDesc.h"
#include "NewStrDup.h"
#include "STLStringUtility.h"

namespace {

class RegExpDirFilter: public FileUtils::Directory::Filter {
public:
   RegExpDirFilter( const char* regExp ):
      m_regExp( regExp ) {}
   bool shouldInclude( const char* filename ) const {
      return StringUtility::regexp( m_regExp, filename );
   }
private:
   const char* m_regExp;
};

}


class SFDFileHeader {
public:
   SFDFileHeader( const MC2SimpleString& fileName ) 
         : filePath( fileName.c_str() ), sfdHeader( fileName, false ),
           rawFile( filePath.c_str() )
   {
      mc2dbg2 << "SFDFileHeader " << fileName << " = " << filePath 
              << " = " << rawFile.getFileName() << endl;
   }

   void loadHeader() {
      mc2dbg2 << "load sfd " << rawFile.getFileName() << endl;
      MC2_ASSERT( rawFile.open() );
      vector<byte> headBuff;
      MC2_ASSERT( rawFile.read( headBuff, 50000 ) ); // TODO: Less hardcoded
      SharedBuffer loadBuff( &headBuff.front(), headBuff.size() );
      MC2_ASSERT( sfdHeader.load( loadBuff ) );
   }

   /// The file path
   MC2String filePath;
   /// The header
   SFDLoadableHeader sfdHeader;
   /// The File
   File rawFile;
};

SFDHolder::SFDHolder( const MC2SimpleString& path ) {
   // Make the SFDFileHeader for all the sfd files

   // Language of the SFD files!
   MC2String failedLangs;
   for ( uint32 l = LangTypes::english ; l < LangTypes::nbrLanguages ; ++l ) {
      MC2String dirPath( path.c_str() );
      STLStringUtility::replaceString( 
         dirPath, "[ISO639-3]", 
         LangTypes::getLanguageAsISO639AndDialect( 
            LangTypes::language_t( l ) ) );
      try {
         // load all files in directory
         using FileUtils::Directory;
         Directory dir( dirPath );

         Directory::Filenames files;
         dir.scan( files, RegExpDirFilter( ".*\\.wfd$" ) );

         pair< SFDCont::iterator, bool > cur = m_maps.insert( 
            make_pair( LangTypes::language_t( l ),
                       SFDFileHeaderCont() ) );
         
         // load all files
         for ( Directory::Filenames::const_iterator it = files.begin();
               it != files.end(); ++it ) {
            cur.first->second.push_back( new SFDFileHeader( (*it).c_str() ) );
            cur.first->second.back()->loadHeader();
         }
         mc2log << info << "[SFDH] loaded SFD directory: " << dirPath << endl;
      } catch ( const FileUtils::FileException& e ) {
         if ( !failedLangs.empty() ) {
            failedLangs += ", ";
         }
         failedLangs += LangTypes::getLanguageAsISO639AndDialect( 
            LangTypes::language_t( l ) );
      }
   } // For all languages
   if ( !failedLangs.empty() ) {
      mc2log << warn << "[SFDH] Failed to load the following languages: " 
             << failedLangs << endl;
   }
}

SFDHolder::~SFDHolder() {
   for ( SFDCont::iterator it = m_maps.begin() ; it != m_maps.end() ; ++it ) {
      STLUtility::deleteValues( it->second );
   }
}

namespace {
void addEmptyImportances( const TileMapParams& param, int startImp, int endImp,
                          const ServerTileMapFormatDesc& tmfd,
                          vector<TileMapBufferHolder>& resVect ) {
   int curImp = startImp;
   do {
      TileMapParams impParam( param );
      impParam.setImportanceNbr( curImp );
      WritableTileMap emptyTileMap( impParam,
                                    tmfd,
                                    0, // unused
                                    0 ); // nbr polys.
      
      BitBuffer* holderBuffer = new BitBuffer( 1024 );
      emptyTileMap.save( *holderBuffer );
      
      holderBuffer->setSizeToOffset();
      holderBuffer->reset();
      resVect.push_back( TileMapBufferHolder( 
                            NewStrDup::newStrDup( 
                               impParam.getAsString().c_str() ),
                            holderBuffer,
                            emptyTileMap.getCRC(), true ) );
      ++curImp;
   } while ( curImp < endImp );
}


}

BitBuffer*
SFDHolder::requestCached( const MC2SimpleString& desc,
                          const ServerTileMapFormatDesc& tmfd ) {

   // TODO: Some sort of mapping from one tile to sfds, so not asking all
   // TODO: And some error checks below, not asserts.
   const TileMapParams param( desc );
   mc2dbg2 << "[SFDH] want " << desc << " lang " 
           << int(param.getLanguageType()) << endl;

   SFDCont::iterator curMap = m_maps.end();
   if ( param.getTileMapType() == TileMapTypes::tileMapData && 
        !m_maps.empty() ) {
      // All languages (should) have same graphics so use first best.
      curMap = m_maps.begin();
   } else {
      curMap = m_maps.find( TileMapParams( desc ).getLanguageType() );
   }

   if ( curMap != m_maps.end() ) {
      // For all sfd files ask their header if desc may be in it
      for ( SFDFileHeaderCont::iterator it = curMap->second.begin() ; 
            it != curMap->second.end() ; ++it ) {
         if ( (*it)->sfdHeader.maybeInCache( desc ) ) {
            //  Ask SFD file for buffer and return it if not NULL
            int offsetOffset = (*it)->sfdHeader.getMultiBufferOffsetOffset( 
               param );
            MC2_ASSERT( (*it)->rawFile.setPos( offsetOffset ) );
            vector<byte> offsetBuff;
            // This and next offset
            MC2_ASSERT( (*it)->rawFile.read( offsetBuff, 8 ) ); 
            SharedBuffer bufOffsetBuf( &offsetBuff.front(), 8 );
            uint32 bufOffset = bufOffsetBuf.readNextBALong();
            // Find out the offset of the next buffer and calculate length
            int bufLength = bufOffsetBuf.readNextBALong() - bufOffset;
            // Seek to the position
            MC2_ASSERT( (*it)->rawFile.setPos( bufOffset ) );
            // And read
            vector<byte> buff;
            MC2_ASSERT( (*it)->rawFile.read( buff, bufLength ) );
            typedef vector<TileMapBufferHolder> TMBHV;
            TMBHV resVect;
            BitBuffer readBuff( &buff.front(), buff.size() );
            // Find the right importance!? 
            SFDMultiBufferReader reader( &readBuff, param, &(*it)->sfdHeader );
            TileMapParams inImp0( param );
            inImp0.setImportanceNbr( 0 );
            int curImp = 0;
            while ( reader.hasNext() ) {
               SFDMultiBufferReader::bufPair_t data = reader.readNext( 
                  param.getLanguageType() );
               // All that is the same with only different importance
               TileMapParams imp0( data.first );
               imp0.setImportanceNbr( 0 );
               if ( imp0.getAsString() == inImp0.getAsString() &&
                    data.second != NULL ) {
                  mc2dbg2 << " imp " << int(data.first.getImportanceNbr()) 
                          << " param " << data.first << " lang "
                          << int(imp0.getLanguageType()) << endl;
                  if ( curImp != 
                       TileMapParams( data.first ).getImportanceNbr() ) {
                     // Add empty ones?
                     addEmptyImportances( 
                        data.first, curImp, 
                        TileMapParams( data.first ).getImportanceNbr(), 
                        tmfd, resVect );
                     curImp = TileMapParams( data.first ).getImportanceNbr();
                  }
                  resVect.push_back( TileMapBufferHolder( 
                                        NewStrDup::newStrDup( 
                                           data.first.getAsString().c_str() ),
                                        data.second,
                                        &tmfd ) );
                  ++curImp;
               }
            }
            if ( curImp != 9 && curImp != 0/*!resVect.empty()*/ ) {
               addEmptyImportances( param, curImp, 9, tmfd, resVect );
            }
            if ( resVect.size() != 9 ) { // If all current importances
               mc2dbg2 << "not all importances only ";
               for ( TMBHV::iterator it = resVect.begin() ; 
                     it != resVect.end() ; ++it ) {
                  mc2dbg2 << " " << TileMapParams( (*it).getDesc() ).
                             getImportanceNbr();
                  (*it).deleteBuffer();
               }
               mc2dbg2 << endl;
               resVect.clear();
            } else {
               mc2dbg2 << "All importances! ";
               for ( TMBHV::iterator it = resVect.begin() ; 
                     it != resVect.end() ; ++it ) {
                  mc2dbg2 << " " << TileMapParams( (*it).getDesc() ).
                             getImportanceNbr();
               }
               mc2dbg2 << endl;
            }

            // Add the Cache buffer format
            BitBuffer* b = NULL;
            if ( !resVect.empty() ) {
               uint32 bufferSize = 4; // 4 for number of buffers
               for ( TMBHV::iterator it = resVect.begin() ; 
                     it != resVect.end() ; ++it ) {
                  bufferSize += strlen( (*it).getDesc() ) + 1;
                  if ( (*it).getBuffer() != NULL ) {
                     bufferSize += (*it).getBuffer()->getBufferSize();
                  }
               }
               // Fix size of header for each
               bufferSize += 9 * resVect.size();
               // Make if so
               b = new BitBuffer( bufferSize );
               
               b->writeNextBALong( resVect.size() ); // nbr buffers
               for ( TMBHV::iterator it = resVect.begin() ; 
                     it != resVect.end() ; ++it ) {
                  if ( (*it).getBuffer() != NULL ) {
                     b->writeNextBALong( (*it).getBuffer()->getBufferSize() );
                     b->writeNextBALong( (*it).getCRC() );
                  } else {
                     b->writeNextBALong( 0 );
                     b->writeNextBALong( MAX_UINT32 );
                  }
                  b->writeNextString( (*it).getDesc() );
                  // empty buffer byte
                  b->writeNextBAByte( (*it).getBuffer() == NULL ); 
                  if ( (*it).getBuffer() != NULL ) {
                     b->writeNextByteArray( 
                        (*it).getBuffer()->getBufferAddress(),
                        (*it).getBuffer()->getBufferSize() );
                  }
               }

               // Reset the offset
               b->reset();

               mc2dbg << "[SFDH] Returning result for " << desc << endl;
            } // End if resVect is not empty

            // Call deleteBuffer on all in resVect
            for ( TMBHV::iterator it = resVect.begin() ; it != resVect.end() ; 
                  ++it ) {
               (*it).deleteBuffer();
            }

            return b;
         } // End if tile may be in cache
      } // For all SFD files in the language
   } // End if we have SFDs for requested language

   return NULL;
}

