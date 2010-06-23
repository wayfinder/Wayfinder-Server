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

// sfdmerger
#include "CommandlineOptionHandler.h"
#include "File.h"
#include "SFDSavableHeader.h"
#include "SFDLoadableHeader.h"
#include "SFDMultiBufferWriter.h"
#include "SFDMultiBufferReader.h"
#include "TileMapParams.h"
#include "ParamsNotice.h"
#include "TileMap.h"
#include "TileCollectionNotice.h"
#include "SFDFile.h"


int sfdmerger( int argc, char* argv[] );
int sfdmerger2( int argc, char* argv[] );

int main(int argc, char *argv[]) {
   int res = sfdmerger2( argc, argv );

   return res;
}


/**
 * Reads and merges sfd (wfd) files together.
 */
int
sfdmerger2( int argc, char* argv[] ) {
   // COH to get BT and handle arguments
   CommandlineOptionHandler coh( argc, argv, 2 );

   coh.setSummary( "Merges any number of sfd files together "
                   "into one. But the result must form a square." );
   coh.setTailHelp( "outSFDfile inSFDfile+" );

   int wfdVersion = 0; // Only version 0 supported by all sfd readers 20081121
   bool testRun = false;
   char* name;

   coh.addOption( "-w", "--wfd-version", CommandlineOptionHandler::integerVal,
                  1, &wfdVersion, "0", 
                  "Set the sfd version to save, default 0. Only version 0 "
                  "supported by any reader but this.");
   coh.addOption( "-t", "--test", CommandlineOptionHandler::presentVal,
                  1, &testRun, "F", 
                  "If to only test to load the sfd files not make any output, "
                  "default false" );
   coh.addOption( "-n", "--name",
                  CommandlineOptionHandler::stringVal,
                  1, &name, "\0",
                  "The name of the merged sfd, like 22_6_3 for a 3x3 tile "
                  "merge of x 66-68 and y 18-20.");

   if ( !coh.parse() ) {
      coh.printHelp( cout );
      return 1;
   }

   MC2String outFile( coh.getTail( 0 ) );
   SFDFile outSfd( name, false/*debug, overwritten by load*/ );
   uint32 totalBuffSize = 0;

   int res = 0;

   // For all the input files
   for ( int i = 1/*0 is out file name*/; 
         i < coh.getTailLength() && res == 0 ; ++i ) {
      // Read and merge
      MC2String inFile( coh.getTail( i ) );
      mc2dbg << "Attempting to read " << inFile << endl;
      vector<byte> buff;
      int fres = File::readFile( inFile.c_str(), buff );
      if ( fres < 0 ) {
         res = 2;
      } else {
         mc2dbg << "Attempting to load " << inFile << endl;
         auto_ptr<BitBuffer> bitBuff( 
            new BitBuffer( &buff.front(), buff.size() ) );
         SFDFile sfd( inFile.c_str(), false/*debug, overwritten by load*/ );
         sfd.load( *bitBuff );

         // Size calculation! (Add all input file sizes and use)
         totalBuffSize += buff.size();

         if ( false && !testRun ) {
            auto_ptr<BitBuffer> outBuff( new BitBuffer( buff.size() ) );
            sfd.save( *outBuff, wfdVersion );

            MC2String saveFile( inFile + ".test" );
            int fres = File::writeFile( saveFile.c_str(), 
                                        outBuff->getBufferAddress(), 
                                        outBuff->getCurrentOffset() );
            if ( fres < 0 ) {
               mc2dbg << "Failed to save saved copy of sfd " << fres << endl;
               res = 2;
            } else {
               mc2dbg << "Saved copy of sfd " << saveFile << " size " << fres 
                      << " bytes" << endl;
            }
         } // End if to save each input as .test 

         if ( i == 1 ) {
            // Set first as out
            outSfd.assign( sfd );
            // Set the name
            outSfd.setName( name );
         } else {
            if ( !outSfd.merge( sfd ) ) {
               mc2dbg << "Failed to merge " << inFile << " into all buffers" 
                      << endl;
               res = false;
            }
         }
      }

   } // End for all input files

   if ( res == 0 && !testRun ) {
      auto_ptr<BitBuffer> outBuff( 
         new BitBuffer( totalBuffSize * (wfdVersion > 0 ? 2 : 1 ) ) );
      outSfd.save( *outBuff, wfdVersion );
      int fres = File::writeFile( outFile.c_str(), 
                                  outBuff->getBufferAddress(), 
                                  outBuff->getCurrentOffset() );
      if ( fres < 0 ) {
         mc2dbg << "Failed to save merged sfd " << fres << endl;
         res = 2;
      } else {
         mc2dbg << "Saved merged sfd " << outFile << " size " << fres 
                << " bytes" << endl;
      }
   }

   coh.deleteTheStrings();

   return res;
}



/**
 * Reads and merges sfd (wfd) files together.
 */
int
sfdmerger( int argc, char* argv[] ) {
   if ( argc < 3 ) {
      mc2log << error << "Usage: sfdmerger outSFDfile inSFDfile+" << endl;
      return 1;
   }
   // COH to get BT
   CommandlineOptionHandler coh( argc, argv );

   SFDSavableHeader outHeader( argv[ 1 ], false/*debug*/ );
   SFDMultiBufferWriter out( 0/*startOffset*/ );
   int res = 0;

   // TODO: First go thou all files and read headers and merge them
   // TODO: Check for buffers with same crc and reuse the old if so

   for ( int i = 2/*0 is program name, 1 is out file name*/; 
         i < argc && res == 0 ; ++i ) {
      // Read and add to out
      mc2dbg << "Attempting to read " << argv[ i ] << endl;
      vector<byte> buff;
      int fres = File::readFile( argv[ i ], buff );
      if ( fres < 0 ) {
         res = 2;
      } else {
         mc2dbg << "Attempting to load " << argv[ i ] << endl;
         BitBuffer* bitBuff = new BitBuffer( &buff.front(), buff.size() );
         mc2dbg << "file size " << bitBuff->getBufferSize() << endl;
         // Header
         SFDLoadableHeader header( argv[ i ], false/*debug*/ );
         mc2dbg << "pos before header " << bitBuff->getCurrentOffset() << endl;
         header.load( *bitBuff );
         mc2dbg << "pos after header " << bitBuff->getCurrentOffset() << endl;

         mc2dbg << "header.m_stringsAreNullTerminated " << header.stringsAreNullTerminated() << endl;
         mc2dbg << "header.m_strIdxEntrySizeBits " << header.getStrIdxEntrySizeBits() << endl;
         mc2dbg << "header.m_strIdxStartOffset " <<  header.getStrIdxStartOffset() << endl;
         mc2dbg << "header.m_nbrStrings " << header.getNbrStrings() << endl;
         mc2dbg << "header.m_strDataStartOffset " << header.getStrDataStartOffset() << endl;
         mc2dbg << "header.m_bufferIdxStartOffset " << header.getBufferIdxStartOffset() << endl;
         mc2dbg << "header.m_bufferDataStartOffset " << header.getBufferDataStartOffset() << endl;
         mc2dbg << "header.m_maxStringSize " << header.maxStringSize() << endl;
         // str_offset_buf
         mc2dbg << "pos before str_offset_buf " << bitBuff->getCurrentOffset() << endl;
         for ( uint32 i = 0 ; i < header.getNbrStrings() + 1 ; ++i ) {
            bitBuff->readNextBALong();
         }
         // strBuf
         mc2dbg << "pos before strBuf " << bitBuff->getCurrentOffset() << endl;
         for ( uint32 i = 0 ; i < header.getNbrStrings() ; ++i ) {
            bitBuff->readNextString();
         }
         // buf_offset_buf
         mc2dbg << "pos before buf_offset_buf " << bitBuff->getCurrentOffset() << endl;
         uint32 buffOffset = 0;
         for ( uint32 i = 0 ; i < header.getNbrStrings() + 1 ; ++i ) {
            buffOffset = bitBuff->readNextBALong();
            mc2dbg << "buffOffset " << buffOffset << endl;
         }
         // mapBuf
         mc2dbg << "pos before mapBuff " << bitBuff->getCurrentOffset() << endl;
         for ( uint32 i = 0 ; i < header.getNbrStrings() ; ++i ) {
            // TileMap
         }
         // Skipp to buffOffset
         bitBuff->readPastBytes( buffOffset -
                                 bitBuff->getCurrentOffset() );
         mc2dbg << "pos after mapBuff " << bitBuff->getCurrentOffset() 
                << " buffOffset " << buffOffset << endl;
         // offsetBuf
         //  offset for each tile in the tile collection
         //  Get min, max from TileCollectionNotice...
         const vector<TileCollectionNotice>& tileCollection =
            header.getTileCollection();
         uint32 nbrOffsets = 0;
         int aLatIdx = MAX_INT32;
         int aLonIdx = MAX_INT32;
         for ( uint32 i = 0 ; i < tileCollection.size() ; ++i ) {
            const vector<TilesForAllDetailsNotice>& tileDetail =
               tileCollection[ i ].getTilesForAllDetails();
            for ( uint32 j = 0 ; j < tileDetail.size() ; ++j ) {
               const vector<TilesNotice>& tileNotices = 
                  tileDetail[ j ].getTilesNotices();
               for ( uint32 k = 0 ; k < tileNotices.size() ; ++k ) {
                  int startLatIdx = tileNotices[ k ].getStartLatIdx();
                  if ( aLatIdx == MAX_INT32 ) {
                     aLatIdx = startLatIdx;
                  }
                  int endLatIdx = tileNotices[ k ].getEndLatIdx();
                  int startLonIdx = tileNotices[ k ].getStartLonIdx();
                  if ( aLonIdx == MAX_INT32 ) {
                     aLonIdx = startLonIdx;
                  }
                  int endLonIdx = tileNotices[ k ].getEndLonIdx();
                  int nbrLayers = tileNotices[ k ].getNbrLayers();
                  nbrOffsets += 
                     (endLatIdx - startLatIdx + 1) * (endLonIdx-startLonIdx + 1);
                  mc2dbg << "startLatIdx " << startLatIdx << endl;
                  mc2dbg << "endLatIdx " << endLatIdx << endl;
                  mc2dbg << "startLonIdx " << startLonIdx << endl;
                  mc2dbg << "endLonIdx " << endLonIdx << endl;
                  mc2dbg << "nbrLayers " << nbrLayers << endl;
                  //mc2dbg << "nbrOffsets += " << ((endLatIdx - startLatIdx + 1) * (endLonIdx-startLonIdx + 1))  << endl;
               } // End for all TilesNotice

            } // End for all TilesForAllDetailsNotice

         } // End for all TileCollectionNotice

         mc2dbg << "Nbr offsets = " << (nbrOffsets+1) << endl;
         
         bitBuff->readPastBytes( (nbrOffsets+1) * 4 );

         uint32 nbrMultiBuffers = 0;
         for ( uint32 i = 0 ; i < tileCollection.size() ; ++i ) {
            const vector<TilesForAllDetailsNotice>& tileDetail =
               tileCollection[ i ].getTilesForAllDetails();
            for ( uint32 j = 0 ; j < tileDetail.size() ; ++j ) {
               const vector<TilesNotice>& tileNotices = 
                  tileDetail[ j ].getTilesNotices();
               for ( uint32 det = 0 ; det < tileNotices.size() ; ++det ) {

                  int startLatIdx = tileNotices[ det ].getStartLatIdx();
                  int endLatIdx = tileNotices[ det ].getEndLatIdx();
                  int startLonIdx = tileNotices[ det ].getStartLonIdx();
                  int endLonIdx = tileNotices[ det ].getEndLonIdx();
                  int nbrLayers = tileNotices[ det ].getNbrLayers();
                  mc2dbg << "startLatIdx " << startLatIdx << endl;
                  mc2dbg << "endLatIdx " << endLatIdx << endl;
                  mc2dbg << "startLonIdx " << startLonIdx << endl;
                  mc2dbg << "endLonIdx " << endLonIdx << endl;
                  mc2dbg << "nbrLayers " << nbrLayers << endl;

                        
                  for ( int32 lat = startLatIdx ; lat <= endLatIdx ; ++lat ) {
                     for ( int32 lon = startLonIdx ; lon <= endLonIdx ; ++lon ) {
                              
                        //for ( int imp = tileNotices[ k ].getFirstImportanceNbr( 0/*lay*/ ) ; imp <= tileNotices[ k ].getLastImportanceNbr( 0/*lay*/ ) ; ++imp ) {

                        mc2dbg << "multiBuff  " << nbrMultiBuffers++ << " lat " << lat << "/" << endLatIdx << " lon " << lon << "/" << endLonIdx << " det " << det << "/" << tileNotices.size() /*<< " imp " << imp << "/" << tileNotices[ k ].getLastImportanceNbr( 0*lay* )*/ /*<< " lay " << lay << "/" << nbrLayers*/ << " tileNotice " << det << "/" << tileNotices.size() << " tileDetails " << j << "/" << tileDetail.size() << " " << " tileCollection " << i << endl;
                        mc2dbg << "pos before multiBuff " << bitBuff->getCurrentOffset() << endl;
                        // multiBuf
                        const TileMapParams param( 
                           9/*serverPrefix*/, 1/*gzip*/, 0/*layer*/, 
                           TileMapTypes::tileMapData, 0/*imp*//*importanceNbr*/, LangTypes::english,
                           lat/*tileIndexLat*/, lon, det/*detailLevel*/ ); // FIXME: Less hardcoded values
                        auto_ptr<SFDMultiBufferReader> reader( new SFDMultiBufferReader( bitBuff, param, &header ) );
                        mc2dbg << "reader.hasNext() " << reader->hasNext() << endl;
                        while ( reader->hasNext() ) {
                           LangTypes::language_t lang = LangTypes::english; // FIXME:
                           // Data
                           SFDMultiBufferReader::bufPair_t data = reader->readNext( lang );
                           // TODO: Add more to outHeader
                           outHeader.updateMetaData( data.first.getAsString() );
                           out.addMap( data.first, data.second );
                        }
                        mc2dbg << "pos after multiBuff " << bitBuff->getCurrentOffset() 
                               << " buffer is " << bitBuff->getBufferSize() << endl;

                        //mc2dbg << "Done with importance " << imp << "/" << tileNotices[ k ].getLastImportanceNbr( 0/*lay*/ ) << endl;
                        //} // End for all importances

                     } // End for all lon
                  } // End for all lat
                              
                  mc2dbg << "TileNotice " << det << " done" << endl;
               } // End for all TilesNotice
               
               mc2dbg << "TileDetailNotice " << j << " done" << endl;
            } // End for all TilesForAllDetailsNotice
         } // End for all TileCollectionNotice

         mc2dbg << "pos after all multiBuff " << bitBuff->getCurrentOffset() 
                << " buffer is " << bitBuff->getBufferSize() << endl;
      } // End else could read input file
   } // For all input sfd-files

   if ( res == 0 ) {
      mc2dbg << "Read all sfd files now saving meta sfd" << endl;
      // SharedBuffer& buf;
      // outHeader
      out.writeAdded();
      const SharedBuffer* outData = out.getBuffer();
      mc2dbg << "Writing " << outData->getBufferSize() << " bytes "
             << "to " << outHeader.getName().c_str() << endl;
      int fres = File::writeFile( 
         outHeader.getName().c_str(), 
         outData->getBufferAddress(), outData->getBufferSize() );
      if ( fres <= 0 ) {
         mc2dbg << "Write failed res " << fres << endl;
         res = 2;
      }
   }

   return res;
}
