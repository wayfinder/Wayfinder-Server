/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ngpmaker.h"
#include "NParam.h"
#include "NParamBlock.h"
#include "ReadlineHelper.h"
#include "StringUtility.h"
#include "NavPacket.h"
#include "File.h"
#include "MC2CRC32.h"
#include "STLStringUtility.h"
#include "CategoryTreeUtils.h"

namespace {
uint16 incReadShort( const byte* buf, int& pos ) {
   uint16 result = (uint16( buf[ pos ] ) << 8) | buf[ pos + 1 ];
   pos += 2;
   return result;
}

uint32 incReadLong( const byte* buf, int& pos ) {
   uint32 result = 0;
   for ( int i = 0 ; i < 4 ; ++i ) {
      result <<= 8;
      result |= buf[ pos + i ];
   }
   pos += 4;
   return result;
}
}

ngpmaker::ngpmaker(int argc, char *argv[]) {
   r = new ReadlineHelper( "", true, "./.ngpmakerhist" );
}


ngpmaker::~ngpmaker() {
   delete r;
}


bool
ngpmaker::writeFile( FILE*& outFile, const MC2String& outFileName,
                     const NParamBlock& params, byte protoVer, 
                     uint16 type, byte reqID, byte reqVer, bool reply,
                     byte statusCode, const MC2String& statusMessage,
                     bool mayUseGzip )
{
   // Make packet and save in output file
   vector<byte> t;
   bool ok = false;
   params.writeParams( t, protoVer, mayUseGzip );
   if ( reply ) {
      NavReplyPacket r( protoVer, type, reqID, reqVer, 
                        statusCode, statusMessage.c_str(),
                        &t.front(), t.size() );
      t.clear();
      r.writeTo( t, mayUseGzip );
   } else {
      NavRequestPacket r( protoVer, type, reqID, reqVer, 
                          &t.front(), t.size() );
      t.clear();
      r.writeTo( t, mayUseGzip );
   }
   
   params.dump( cerr, true, true, MAX_UINT32, MAX_UINT32 );


   // Encrypt it
   uint32 magicPos = 0;
   for ( uint32 i = 10 ; i < t.size() ; i++ ) {
      t[ i ] = t[ i ]  ^ MAGICBYTES[ magicPos ];
      magicPos++;
      if ( magicPos >= MAGICLENGTH ) {
         magicPos = 0;
      }
   }

   if ( outFile != NULL ) {
      fclose( outFile );
      outFile = NULL;
   }
   outFile = fopen( outFileName.c_str(), "wb" );
   if ( outFile == NULL ) {
      cerr << "Failed to open outfile " << MC2CITE( outFileName ) << endl;
   } else {
      if ( fwrite( &t.front(), t.size(), 1, outFile ) == 1 ) {
         cerr << "Wrote " << t.size() << " bytes to " << outFileName 
              << endl;
         ok = true;
      } else {
         cerr << "Failed to write outfile " << MC2CITE( outFileName ) 
              << endl;
      }
   }
   fclose( outFile );
   outFile = NULL;

   return ok;
}


bool
ngpmaker::loadFile( const MC2String& inFileName,
                    NParamBlock& params, byte& protoVer, 
                    uint16& type, byte& reqID, byte& reqVer,
                    bool& reply,
                    byte& statusCode, MC2String& statusMessage,
                    bool& mayUseGzip )
{
   // Read file and load into params etc.
   bool loaded = false;
   
   MC2String fileName = StringUtility::trimStartEnd( inFileName );
   if ( !fileName.empty() ) {
      // Check and fixup if needed 
      if ( fileName[ 0 ] != '/' &&
           !(fileName.size() >= 2 && fileName[ 0 ] == '.' &&
             fileName[ 1 ] == '/') &&
           !(fileName.size() >= 3 && fileName[ 0 ] == '.' &&
             fileName[ 1 ] == '.' && fileName[ 2 ] == '/') ) 
      {
         // Add ./
         fileName.insert( 0, "./" );
      }
      vector<byte> buff;
      if ( File::readFile( fileName.c_str(), buff ) >= 0 ) {
         const uint32 navPacketheaderLength = 10;
         const uint32 protoverPos = 5;
         bool ok = true;
         if ( buff.size() < navPacketheaderLength + 4/*CRC*/ ) {
            cerr << "loadFile Packet to short "
                 << buff.size() << " bytes." << endl;
            ok = false;
         }
         if ( ok ) {
            if ( buff[ protoverPos ] < 0xa ) {
               cerr << "loadFile protoVer < 0xa." << endl;
               ok = false;
            }
         }
         if ( ok && buff[ 0 ] != 0x02 ) { // STX
            cerr << "loadFile not STX first." << endl;
            ok = false;
         }
         
         if ( ok ) {
            uint32 magicPos = 0;
            for ( uint32 i = 10 ; i < buff.size() ; i++ ) {
               buff[ i ] = buff[ i ] ^ MAGICBYTES[ magicPos ];
               magicPos++;
               if ( magicPos >= MAGICLENGTH ) {
                  magicPos = 0;
               }
            }
            int pos = buff.size() - 4;
            uint32 crc = incReadLong( &buff.front(), pos );
            
            // Calculate CRC
            uint32 realCrc = MC2CRC32::crc32( &buff.front(), 
                                              buff.size() - 4/*crc*/ );
            if ( realCrc != crc ) {
               cerr << "loadFile crc is incorrent. 0x" 
                    << hex << crc << " should be 0x" << realCrc 
                    << dec << endl;
               ok = false;
            }
         }
         
         if ( ok ) {
            int pos = 6;
            type = incReadShort( &buff.front(), pos );
            reqID  = buff[ 8 ];
            reqVer = buff[ 9 ];
            protoVer = buff[ protoverPos ];

            NavRequestPacket r( 
               protoVer, type, reqID, reqVer, 
               &buff.front() + navPacketheaderLength, 
               buff.size() - navPacketheaderLength - 4/*crc*/ );
            reply = false;
            if ( !r.getParamBlock().getValid() ) {
               // Status
               statusCode = buff[ navPacketheaderLength ];
               statusMessage = (const char*)
                  (&buff.front() + navPacketheaderLength + 1);
               uint32 pos = navPacketheaderLength + 1 + 
                  statusMessage.size() + 1;

               NavReplyPacket r( 
                  protoVer, type, reqID, reqVer, 
                  statusCode, statusMessage.c_str(),
                  &buff.front() + pos, 
                  buff.size() - pos - 4/*crc*/ );
               mayUseGzip = r.getParamBlock().mayUseGzip();
               if ( !r.getParamBlock().getValid() ) {
                  cerr << "loadFile ParamBlock invalid." << endl;
                  ok = false;
               } else {
                  cerr << "loadFile: this is a reply packet." << endl;
                  reply = true;
                  params = r.getParamBlock();
                  loaded = true;
               }
            } else {
               params = r.getParamBlock();
               loaded = true;
            }
         }
         
      }
   }
   
   return loaded;
}


bool
ngpmaker::loadParamFile( const MC2String& inFileName,
                         NParamBlock& params, byte& protoVer, 
                         uint16& type, byte& reqID, byte& reqVer,
                         bool& reply,
                         byte& statusCode, MC2String& statusMessage,
                         bool& mayUseGzip )
{
   bool loaded = false;
   MC2String fileName = StringUtility::trimStartEnd( inFileName );
   if ( !fileName.empty() ) {
      // Check and fixup if needed 
      if ( fileName[ 0 ] != '/' &&
           !(fileName.size() >= 2 && fileName[ 0 ] == '.' &&
             fileName[ 1 ] == '/') &&
           !(fileName.size() >= 3 && fileName[ 0 ] == '.' &&
             fileName[ 1 ] == '.' && fileName[ 2 ] == '/') ) 
      {
         // Add
         fileName.insert( 0, "./" );
      }
      vector<byte> buff;
      if ( File::readFile( fileName.c_str(), buff ) >= 0 ) {
         mc2dbg4 << "Read " << buff.size() << " bytes from " << fileName 
                 << endl;
         buff.push_back( '\0' );
         if ( buff.size() > 1 && params.readParams( 
                 reinterpret_cast< const char* >( &buff.front() ), 
                 params, protoVer, type, reqID, reqVer ) ) {
            loaded = true;
         } else {
            cerr << "loadParamFile Bad NavRequest file." << endl;
         }
      } else {
         cerr << "loadParamFile Failed to read file " << fileName << " (" 
              << inFileName << ")" << endl;
      }
   } else {
      cerr << "loadParamFile Bad file name " << inFileName << endl;
   }

   return loaded;
}


class NGPSearchRegion {
public:
   NGPSearchRegion( const NParam& p, uint32& pos, byte reqVer ) {
      if ( reqVer >= 1 ) {
         type = p.incGetUint32( pos );
      } else {
         type = p.incGetUint16( pos );
      }
      id = p.incGetString( true, pos );
      name = p.incGetString( true, pos );
   }

   ostream& print( ostream& os ) const {
      os << "     type " << int( type ) << endl
         // Not good for diff, maybe argument. << "     id " << id << endl
         << "     name " << name << endl
         ;
      return os;
   }

   uint32 type;
   MC2String id;
   MC2String name;
};

class NGPSearchMatch {
public:
   NGPSearchMatch( const NParam& p, uint32& pos, byte reqVer ) {
      type = p.incGetByte( pos );
      subType = p.incGetByte( pos );
      if ( reqVer >= 2 ) {
         adVert = p.incGetByte( pos );
      }
      id = p.incGetString( true, pos );
      name = p.incGetString( true, pos );
      if ( reqVer >= 1 ) {
         image = p.incGetString( true, pos );
      }
      lat = p.incGetInt32( pos );
      lon = p.incGetInt32( pos );
      byte nbrRegions = p.incGetByte( pos );
      for ( byte i = 0 ; i < nbrRegions ; ++i ) {
         regionIndexes.push_back( p.incGetUint16( pos ) ); 
      }
   }

   
   ostream& print( ostream& os, 
                   const vector< NGPSearchRegion >& regions ) const {
      os << "    type " << int( type ) << endl
         << "    subType " << int( subType ) << endl
         // Not good for diff, maybe argument. << "    id " << id << endl
         << "    name " << name << endl
         << "    image " << image << endl
         << "    lat " << lat << endl
         << "    lon " << lon << endl;
      for ( uint32 i = 0 ; i < regionIndexes.size() ; ++i ) {
         os << "    region " << i << endl;
         if ( regionIndexes[ i ] >= regions.size() ) {
            os << "     Region idx " << regionIndexes[ i ] 
               << " does not exist!" << endl;
         } else {
            regions[ regionIndexes[ i ] ].print( os );
         }
      }
      return os;
   }


   byte type;
   byte subType;
   byte adVert;
   MC2String id;
   MC2String name;
   MC2String image;
   int32 lat;
   int32 lon;
   vector< uint16 > regionIndexes;
};

class NGPSearchArea {
public:
   NGPSearchArea( const NParam& p, uint32& pos, byte reqVer ) {
      type = p.incGetUint16( pos );
      id = p.incGetString( true, pos );
      name = p.incGetString( true, pos );
      if ( reqVer >= 1 ) {
         image = p.incGetString( true, pos );
      }
      byte nbrRegions = p.incGetByte( pos );
      for ( byte i = 0 ; i < nbrRegions ; ++i ) {
         regionIndexes.push_back( p.incGetUint16( pos ) ); 
      }
   }

   ostream& print( ostream& os, 
                   const vector< NGPSearchRegion >& regions ) const {
      os << "    type " << int( type ) << endl
         // Not good for diff, maybe argument. << "    id " << id << endl
         << "    name " << name << endl
         << "    image " << image << endl
         ;
      for ( uint32 i = 0 ; i < regionIndexes.size() ; ++i ) {
         os << "    region " << i << endl;
         regions[ regionIndexes[ i ] ].print( os );
      }
      return os;
   }

   uint16 type;
   MC2String id;
   MC2String name;
   MC2String image;
   vector< uint16 > regionIndexes;
};

void
ngpmaker::printCombinedReply( const NParamBlock& params, byte reqVer ) {
   cout << "NAV_COMBINED_SEARCH_REPLY" << endl;
   for ( uint32 paramID = 5700 ; params.getParam( paramID ) != NULL ; 
         paramID += 2 ) {
      // Heading
      const NParam* p = params.getParam( paramID );
      // Region table
      const NParam* t = params.getParam( paramID + 1);
      
      // Regions ( 4 type ,string id, string name
      vector< NGPSearchRegion > regions;
      uint32 pos = 0;
      while ( t != NULL && pos < t->getLength() ) {
         regions.push_back( NGPSearchRegion( *t, pos, reqVer ) );
      }

      // Heading
      pos = 0;
      uint32 headingID = p->incGetUint32( pos );
      uint32 listType = p->incGetUint32( pos );
      uint32 startIndex = p->incGetUint32( pos );
      uint32 totalNbrMatches = p->incGetUint32( pos );
      cout << " Heading " << headingID << endl
           << "  type " << listType << endl
           << "  startIndex " << startIndex << endl
           << "  totalNbrMatches " << totalNbrMatches << endl;
      uint32 minTopHits = 0;
      if ( reqVer >= 2 ) {
         // min top hits
         minTopHits = p->incGetUint32( pos );
         // "sponsored links" and "all results" titles
         cout << "  minTopHits " << minTopHits << endl
              << "  sponsoredLinkTitle " << p->incGetString( true, pos )
              << endl
              << "  allResultsTitle " << p->incGetString( true, pos ) << endl;
      }

      // Matches
      if ( listType == 0 ) {
         while ( pos < p->getLength() ) {
            NGPSearchMatch m( *p, pos, reqVer );
            cout << "   Match " << endl;
            m.print( cout, regions );
         }
      } else if ( listType == 1 ) {
         while ( pos < p->getLength() ) {
            NGPSearchArea m( *p, pos, reqVer );
            cout << "   Area " << endl;
            m.print( cout, regions );
         }
      } else {
         cout << "  Unknown list type " << listType << endl;
      }
      
   }
}

void
ngpmaker::printServerInfoReply( const NParamBlock& params, byte reqVer ) {
   cout << "NAV_SERVER_INFO_REPLY" << endl;

   if ( reqVer < 4 ) {
      params.dump( cout, true, false, MAX_UINT32, MAX_UINT32 );
   } else {
      // Only print the New client version stuff...
      const NParam* pVersion = params.getParam( 23 );
      const NParam* pForce = params.getParam( 4308 );
      const NParam* pUpgradeId = params.getParam( 4309 );

      if ( pVersion != NULL ) {
         cout << "Latest version: " << pVersion->getString() << endl;
      }

      if ( pForce != NULL ) {
         cout << "Force upgrade: " << pForce->getBool() << endl;
      }
      
      if ( pUpgradeId != NULL ) {
         cout << "Latest version: " << pUpgradeId->getString() << endl;
      }
   }
   
}


void printArea( const byte* areaTbl, const byte* strTbl, 
                int32 areaOffset ) {
   cout << "     Area " << endl;
   NParam area( 0, areaTbl + areaOffset, (2+4+4) );
   uint32 pos = 0;
   cout << "      Type " << area.incGetInt16( pos ) << endl;
   cout << "      Name " << (strTbl + area.incGetInt32( pos )) << endl;
   //cout << "      ID " << (strTbl + area.incGetInt32( pos )) << endl;
}

void printInfo( const byte* infoTbl, const byte* strTbl, 
                int32 infoOffset ) {
   cout << "     Info " << endl;
   NParam info( 0, infoTbl + infoOffset, (2+1+4+4) );
   uint32 pos = 0;

   cout << "      Type " << int( info.incGetUint16( pos ) ) << endl;
   cout << "      Content Type " << int( info.incGetByte( pos ) ) << endl;
   cout << "      Key " << (strTbl + info.incGetInt32( pos )) << endl;
   cout << "      Val " << (strTbl + info.incGetInt32( pos )) << endl;
}

void 
ngpmaker::printOneSearchReply( const NParamBlock& params, byte reqVer ) {
   cout << "NAV_ONE_SEARCH_REPLY" << endl;

   // 6900 numberMatches
   if ( params.getParam( 6900 ) != NULL ) {
      cout << "  NbrMatches " << params.getParam( 6900 )->getUint32() << endl;
   }

   // 6901 TotalNbrMatches
   if ( params.getParam( 6901 ) != NULL ) {
      cout << "  TotalNbrMatches " << params.getParam( 6901 )->getUint32()
           << endl;
   }

   // 6902 SearchStringTable
   const byte* strTbl = NULL;
   if ( params.getParam( 6902 ) != NULL ) {
      strTbl = params.getParam( 6902 )->getBuff();
   }

   // 6903 AreaTable
   const byte* areaTbl = NULL;
   if ( params.getParam( 6903 ) != NULL ) {
      areaTbl = params.getParam( 6903 )->getBuff();
   }

   // 6904 infoItemTable
   const byte* infoTbl = NULL;
   if ( params.getParam( 6904 ) != NULL ) {
      infoTbl = params.getParam( 6904 )->getBuff();
   }

   // 6905 Matches
   if ( params.getParam( 6905 ) != NULL ) {
      const NParam* matches = params.getParam( 6905 );

      uint32 pos = 0;

      while ( pos < matches->getLength() ) {
         // Read next match
         cout << "   Match " << endl;

         // Name idx
         int32 nameIDx     = matches->incGetInt32( pos );
         // Id idx
         int32 idIDx       = matches->incGetInt32( pos );
         // location idx
         int32 locIDx      = matches->incGetInt32( pos );
         // Coord
         MC2Coordinate coord;
         coord.lat         = matches->incGetInt32( pos );
         coord.lon         = matches->incGetInt32( pos );
         // Type
         int8 type         = matches->incGetByte( pos );
         int16 subType     = matches->incGetInt16( pos );
         // Category Image Idx
         int32 catImgIdx   = matches->incGetInt32( pos );
         // Provider Image Idx
         int32 provImgIdx  = matches->incGetInt32( pos );
         // Brand Image Idx
         int32 brandImgIdx = matches->incGetInt32( pos );
         // flags
         byte flags        = matches->incGetByte( pos );
         // Nbr categories
         byte nbrCats      = matches->incGetByte( pos );
         // Nbr areas
         byte nbrAreas     = matches->incGetByte( pos );
         // Nbr infos
         byte nbrInfos     = matches->incGetByte( pos );

         cout << "    Name " << (strTbl + nameIDx) << endl;
         // cout << "    ID " << (strTbl + idIDx) << endl;
         idIDx = 0; // Compiler happy with this usage
         cout << "    Location " << (strTbl + locIDx) << endl;
         cout << "    Coord " << coord << endl;
         cout << "    Type " << int(type) << endl;
         cout << "    SubType " << subType << endl;
         cout << "    Category Image " << (strTbl + catImgIdx) << endl;
         cout << "    Provider Image " << (strTbl + provImgIdx) << endl;
         cout << "    Brand Image " << (strTbl + brandImgIdx) << endl;
         cout << "    Flags " << MC2HEX( int(flags) ) << endl;
         
         // Categories
         cout << "    nbrCategories " << int(nbrCats) << endl;
         cout << "    Categories";
         for ( byte i = 0 ; i < nbrCats ; ++i ) {
            cout << " " << matches->incGetInt32( pos );
         }
         cout << endl;

         // Area offsets
         cout << "    nbrAreas " << int(nbrAreas) << endl;
         for ( byte i = 0 ; i < nbrAreas ; ++i ) {
            printArea( areaTbl, strTbl, matches->incGetInt32( pos ) );
         }

         // Infos offsets
         cout << "    nbrItemInfos " << int(nbrInfos) << endl;
         for ( byte i = 0 ; i < nbrInfos ; ++i ) {
            printInfo( infoTbl, strTbl, matches->incGetInt32( pos ) );
         }
         
         
      }
   }
   
}



void readLocalCategoryTreeNode( const NParam* cats, uint32 pos, 
                                const byte* strTbl, uint32 level ) {
   // CategoryID
   CategoryTreeUtils::CategoryID catId = cats->incGetInt32( pos );
   int32 nameIdx = cats->incGetInt32( pos );
   int32 iconIdx = cats->incGetInt32( pos );
   int16 nbrChildren = cats->incGetInt16( pos );

   MC2String indent( level*3, ' ' );
   cout << indent << " Cat id " << catId << " Name " << (strTbl + nameIdx) 
        << " Icon " << (strTbl + iconIdx) << endl;

   for ( int16 j = 0; j < nbrChildren; ++j ) {
      int32 childIndex = cats->incGetInt32( pos );
      readLocalCategoryTreeNode( cats, childIndex, strTbl, level + 1 );
   }
}

void 
ngpmaker::printLocalCategoryTreeReply( const NParamBlock& params, byte reqVer )
{
   cout << "NAV_LOCAL_CATEGORY_TREE_REPLY" << endl;

   // 6504 string table
   const byte* strTbl = NULL;
   if ( params.getParam( 6504 ) != NULL ) {
      strTbl = params.getParam( 6504 )->getBuff();
   }

   // 6503 lookup table
   const byte* lookTbl = NULL;
   if ( params.getParam( 6503 ) != NULL ) {
      lookTbl = params.getParam( 6503 )->getBuff();
   }

   // 6502 category table
   const byte* catTbl = NULL;
   if ( params.getParam( 6502 ) != NULL ) {
      catTbl = params.getParam( 6502 )->getBuff();
      const NParam* cats = params.getParam( 6502 );

      uint32 pos = 0;

      // Top Level
      int32 nbrTopLevel = cats->incGetInt16( pos );

      vector<CategoryTreeUtils::CategoryID> topLevelList;
      topLevelList.resize( nbrTopLevel );
      for ( int32 i = 0 ; i < nbrTopLevel ; ++i ) {
         int32 categoryIndex = cats->incGetInt32( pos );
         int32 catID = *reinterpret_cast<const int32*>(
            lookTbl + categoryIndex );
         topLevelList[ i ] = catID;

         readLocalCategoryTreeNode( cats, categoryIndex, strTbl, 1 );
      }
   }
}

void
ngpmaker::printDetailsReply( const NParamBlock& params, byte reqVer ) {
   cout << "NAV_DETAIL_REPLY" << endl;

   
   if ( params.getParam( 7100 ) != NULL ) {
      const NParam* info = params.getParam( 7100 );
      uint32 pos = 0;
      // ItemID
      info->incGetString( true, pos );
      uint16 nbrTuples = info->incGetInt16( pos );
      cout << " nbrTuples " << int(nbrTuples) << endl;
      for ( uint16 i = 0 ; i < nbrTuples ; ++i ) {
         // Type
         uint16 type = info->incGetUint16( pos );
         // Content type
         byte content_type = info->incGetByte( pos );
         // Key
         MC2String key = info->incGetString( true, pos );
         // Value
         MC2String value = info->incGetString( true, pos );
         cout << " Type " << int(type) << "Content type " << int(content_type)
              << " key " << key << " value " 
              << value << endl;
      }
   }

   if ( reqVer >= 2 ) {
      // String table
      const NParam* strings = params.getParam( 7103 );
      if ( strings != NULL ) {
         const byte* strTbl = strings->getBuff();

         // Images
         const NParam* images = params.getParam( 7101 );
         if ( images != NULL ) {
            uint32 pos = 0;
            // Provider name
            int32 providerNameIdx = images->incGetInt32( pos );
            // Provider Image
            int32 providerImageIdx = images->incGetInt32( pos );
            // Nbr of images
            int16 nbrImages = images->incGetInt16( pos );
            
            cout << endl << " --- Images ---" << endl;
            cout << "Provider name "<< strTbl + providerNameIdx << endl;
            cout << "Provider image name "<< strTbl + providerImageIdx << endl;
            cout << "Nbr of images " << nbrImages << endl;

            for ( int32 i=0; i < nbrImages; i++ ) {
               cout << "    " << strTbl + images->incGetInt32( pos ) << endl;
            }            
         }

         // Reviews
         const NParam* reviews = params.getParam( 7102 );
         if ( images != NULL ) {
            uint32 pos = 0;
            // Provider name
            int32 providerNameIdx = reviews->incGetInt32( pos );
            // Provider Image
            int32 providerImageIdx = reviews->incGetInt32( pos );
            // Nbr of images
            int16 nbrReviews = reviews->incGetInt16( pos );

            cout << endl << " --- Reviews ---" << endl;
            cout << "Provider name "<< strTbl + providerNameIdx << endl;
            cout << "Provider image name "<< strTbl + providerImageIdx << endl;
            cout << "Nbr of reviews " << nbrReviews << endl;

            for ( int32 i=0; i < nbrReviews; i++ ) {
               cout << "     Rating " << int16( reviews->incGetByte( pos ) ) << endl;
               cout << "     Date " << strTbl + reviews->incGetInt32( pos ) << endl;
               cout << "     Reviewer " << strTbl + reviews->incGetInt32( pos ) << endl;
               cout << "     Review text " << strTbl + reviews->incGetInt32( pos ) << endl;
               cout << endl;
            }            
         }
         
      }
   }
}

bool
ngpmaker::humanReadablePrint( const NParamBlock& params, byte protoVer, 
                              uint16 type, byte reqID, byte reqVer, bool reply,
                              byte statusCode, const MC2String& statusMessage,
                              bool mayUseGzip )
{
   bool printed = true;
   switch ( type ) {
      case NavPacket::NAV_COMBINED_SEARCH_REPLY:
         printCombinedReply( params, reqVer );
         break;
      case NavPacket::NAV_SERVER_INFO_REPLY:
         printServerInfoReply( params, reqVer );
         break;
      case NavPacket::NAV_ONE_SEARCH_REPLY:
         printOneSearchReply( params, reqVer );
         break;
      case NavPacket::NAV_LOCAL_CATEGORY_TREE_REPLY:
         printLocalCategoryTreeReply( params, reqVer );
         break;
      case NavPacket::NAV_DETAIL_REPLY:
         printDetailsReply( params, reqVer );
         break;

      default:
         // Dump it
         params.dump( cout, true, false, MAX_UINT32, MAX_UINT32 );
         break;
   }

   if ( statusCode != NavReplyPacket::NAV_STATUS_OK ) {
      // Print error
      cout << "Error, code: " << int(statusCode) << " Message: "
           << MC2CITE( statusMessage );
      if ( params.getParam( 26 ) != NULL ) {
         cout << " url " << params.getParam( 26 )->getString();
      }
      cout << endl;
   }

   return printed;
}


void 
ngpmaker::doIt() {

   // Get NavPacket header stuff
   // This is a request not reply

   cerr << "Welcome to ngpmaker! Here we make NextGenerationProtocol "
        << "request packets." << endl;

   // Outfile
   MC2String outFileName = r->getText( "Outfile", "ngp.bin" );
   FILE* outFile = fopen( outFileName.c_str(), "ab" );
   if ( outFile == NULL ) {
      cerr << "Failed to open outfile " << MC2CITE( outFileName ) << endl;
      return;
   } else {
      cerr << "File to write to: " << outFileName << endl;
   }

   NParamBlock params;
   byte protoVer = 0;
   uint16 type = 0;
   byte reqID = 0;
   byte reqVer = 0;
   bool reply = false;
   byte statusCode = 0;
   MC2String statusMessage;
   bool mayUseGzip = true;
   MC2String protoVerStr( "0x" );
   STLStringUtility::uint2strAsHex( NavPacket::MAX_PROTOVER, protoVerStr );

   // The file to load, and populate this with.
   MC2String inFileName = r->getText( "Loadfile", "" );
   while ( !inFileName.empty() && 
           !loadFile( inFileName, params, protoVer, type, reqID, reqVer, 
                      reply, statusCode, statusMessage, mayUseGzip ) )
   {
      inFileName = r->getText( "Loadfile", "" );
   }

   if ( inFileName.empty() ) {
      inFileName = r->getText( "LoadParamfile", "" );
      while ( !inFileName.empty() && 
              !loadParamFile( inFileName, params, protoVer, type, reqID,
                              reqVer, reply, statusCode, 
                              statusMessage, mayUseGzip ) )
      {
         inFileName = r->getText( "LoadParamfile", "" );
      }
   }

   if ( inFileName.empty() ) {
      // Protover
      protoVer = r->getByte( "Enter protoVer", protoVerStr.c_str() );
   
      // Type TODO: More user friendly!!
      type = r->getShort( "Enter requestType", "0x3d" );
   
      // Request ID
      reqID = r->getByte( "Enter requestID", "0" );

      // Request Ver
      reqVer = r->getByte( "Enter request ver", "1" );

      // MayUseGzip
      mayUseGzip = r->getByte( "MayUseGzip", "1" );
   } else {
      cerr << "Protover: 0x" << hex << int(protoVer) 
           << " RequestType: 0x" << int(type)
           << " RequestID: 0x" << int(reqID)
           << " RequestVer: 0x" << int(reqVer)
           << " Status code: 0x" << int(statusCode)
           << " Status message: " << statusMessage
           << " MayUseGzip: " << BP(mayUseGzip)
           << dec << endl;
      params.dump( cerr, true, true, MAX_UINT32, MAX_UINT32 );
   }


   // Get params until "done" is entered then continue
   bool done = false;
   bool saved = false;
   MC2String cmd;
   while ( !done ) {
      cmd = r->getText( "Command" );

      if ( cmd == "a" ) {
         saved = false;
         uint16 paramID = r->getShort( "Parameter id" );
         bool pdone = false;
         NParam& p = params.addParam( NParam( paramID ) );
         while ( !pdone) {
            MC2String line = r->getText( "Parameter type" );
            if ( line == "t" ) {
               p.addString( r->getText( "Enter string" ), 
                            protoVer >= 0xa );
               cerr << "Added "; p.dump( cerr, true, true ); cerr << endl;
            } else if ( line == "b" ) {
               p.addByte( r->getByte( "Enter byte" ) );
               cerr << "Added "; p.dump( cerr, true, true ); cerr << endl;
            } else if ( line == "s" ) {
               p.addUint16( r->getShort( "Enter short" ) );
               cerr << "Added "; p.dump( cerr, true, true ); cerr << endl;
            } else if ( line == "l" ) {
               p.addUint32( r->getLong( "Enter long" ) );
               cerr << "Added "; p.dump( cerr, true, true ); cerr << endl;
            } else if ( line == "r" ) {
               if ( p.getLength() > 0 ) {
                  const_cast<vector< byte >&>(p.getVector()).pop_back();
               }
               cerr << "Now   "; p.dump( cerr, true, true ); cerr << endl;
            } else if ( line == "a" ) {
               vector< byte > b = r->getByteArray( "Enter byte array" );
               p.addByteArray( &b.front(), b.size() );
               cerr << "Added "; p.dump( cerr, true, true ); cerr << endl;
            } else if ( line == "d" ) {
               pdone = true;
            } else {
               cerr << "Unknown type " << line << " Try again with "
                    << "t(ext)|b(byte)|s(hort)|l(ong)|a(rray)|"
                    << "r(emove last byte)|d(one)."
                    << endl;
            }
         } // End while not pdone

      } else if ( cmd == "d" ) {
         saved = false;
         uint16 paramID = r->getShort( "Parameter id to delete" );
         params.removeParam( paramID );
         cerr << "Removed all " << paramID << endl;
      } else if ( cmd == "l" ) {
         inFileName = r->getText( "Loadfile", "" );
         if ( loadFile( inFileName, params, protoVer, type, reqID, reqVer,
                        reply, statusCode, statusMessage, mayUseGzip ) )
         {
            params.dump( cerr, true, true, MAX_UINT32, MAX_UINT32 );
         }
      } else if ( cmd == "lp" ) {
         inFileName = r->getText( "LoadParamfile", "" );
         if ( loadParamFile( inFileName, params, protoVer, type, reqID, reqVer,
                             reply, statusCode, statusMessage, mayUseGzip ) )
         {
            cerr << "Protover: 0x" << hex << int(protoVer) 
                 << " RequestType: 0x" << int(type)
                 << " RequestID: 0x" << int(reqID)
                 << " RequestVer: 0x" << int(reqVer)
                 << " Status code: 0x" << int(statusCode)
                 << " Status message: " << statusMessage
                 << " MayUseGzip: " << BP(mayUseGzip)
                 << dec << endl;
            params.dump( cerr, true, true, MAX_UINT32, MAX_UINT32 );
         }
      } else if ( cmd == "s" ) {
         if ( writeFile( outFile, outFileName, params, protoVer, type, 
                         reqID, reqVer, reply, statusCode, statusMessage,
                         mayUseGzip ))
         {
            saved = true;
         }
      } else if ( cmd == "p" ) {
         cerr << "Protover: 0x" << hex << int(protoVer) 
              << " RequestType: 0x" << int(type)
              << " RequestID: 0x" << int(reqID)
              << " RequestVer: 0x" << int(reqVer)
              << " Status code: 0x" << int(statusCode)
              << " Status message: " << statusMessage
              << " MayUseGzip: " << BP(mayUseGzip)
              << " OutFile: " << outFileName
              << dec << endl;
         params.dump( cerr, true, true, MAX_UINT32, MAX_UINT32 );
      } else if ( cmd == "h" ) {
         bool hdone = false;
         saved = false;
         while ( !hdone ) {
            hdone = true;
            MC2String line = r->getText( "Header to change" );
            if ( line == "p" ) {
               protoVer = r->getByte( "Enter protoVer", "0xb" );
               cerr << "Protover: 0x" << hex << int(protoVer) << dec 
                    << endl;
            } else if ( line == "t" ) {
               type = r->getShort( "Enter requestType", "0x3d" );
               cerr << "RequestType: 0x" << hex << int(type) << dec 
                    << endl;
            } else if ( line == "o" ) {
               outFileName = r->getText( "Outfile", "ngp.bin" );
               fclose( outFile );
               outFile = NULL;
               outFile = fopen( outFileName.c_str(), "ab" );
               if ( outFile == NULL ) {
                  cerr << "Failed to open outfile " 
                       << MC2CITE( outFileName ) << endl;
               } else {
                  cerr << "OutFile: " << outFileName << endl;
               }
            } else if ( line == "i" ) {
               reqID = r->getByte( "Enter requestID", "0" );
               cerr << "RequestID: 0x" << hex << int(reqID) << dec 
                    << endl;
            } else if ( line == "v" ) {
               reqVer = r->getByte( "Enter request ver", "1" );
               cerr << "RequestVer: 0x" << hex << int(reqVer) << dec 
                    << endl;
            } else if ( line == "c" ) {
               statusCode = r->getByte( "Enter status code", "0" );
               cerr << "Status code: 0x" << hex << int(statusCode) << dec 
                    << endl;
            } else if ( line == "m" ) {
               statusMessage = r->getText( "Enter status message", "" );
               cerr << "Status message: " << statusMessage << endl;
            } else if ( line == "g" ) {
               mayUseGzip = r->getByte( "MayUseGzip", "1" );
               cerr << "MayUseGzip: " << BP(mayUseGzip) << endl;
            } else if ( line == "d" ) {
               // Done
            } else {
               cerr << "Unknown header " << line << " Try again with "
                    << "p(rotoVer)|t(requestType)|o(utfile)|i(requestID)|"
                    << "v(requestVer)|c[status code]|m[status message]|"
                    << "g(zip)|d(one)."
                    << endl;
               hdone = false;
            }
            saved = false;
         }
      } else if ( cmd == "r" ) {
         reply = !reply;
         cerr << "Is now " << (reply ? "Reply" : "Request") << endl;
         saved = false;
      } else if ( cmd == "done" || cmd == "q" ) {
         done = true;
      } else if ( cmd == "hp" ) {
         // Human printable
         // Suitable for text diff
         humanReadablePrint( params, protoVer, type, 
                             reqID, reqVer, reply, statusCode, statusMessage,
                             mayUseGzip );
      } else {
         cerr << "Unknown command " << cmd << " Try again with "
              << "a(dd)|d(elete)|l(oad)|h(header)|p(rint)|r(eply/request)"
              << "|s(ave)|lp(loadparam)|hp(humanReadablePrint)|done."
              << endl;
      }
   }


   if ( !saved && cmd != "q" ) {
      writeFile( outFile, outFileName, params, protoVer, type, 
                 reqID, reqVer, reply, statusCode, statusMessage,
                 mayUseGzip );
   }
   
}
