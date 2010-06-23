/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DataBufferCreator.h"

#include "DataBuffer.h"
#include "URLFetcherNoSSL.h"
#include "URL.h"
#include "STLStringUtility.h"
#include "Properties.h"
#include "GunzipUtil.h"

#include "config.h"

#include <memory>

#ifdef __unix__
#define USE_MADVISE
#endif

#ifdef USE_MADVISE
#include <sys/mman.h>
#endif

using std::auto_ptr;

class VectorDataBuffer : public DataBuffer {
public:
   VectorDataBuffer( vector<uint8>& str ) {
      m_str.swap( str );
      m_buf = &( m_str[0] );
      m_pos = m_buf;
      m_bufSize = m_str.size();
      m_selfAlloc = false;
   }
private:
   /// String that holds the data.
   vector<uint8> m_str;
};

// Advise the virtual memory system about how to optimise use.
static void memadvise( const DataBuffer* buf )
{
   if ( buf == NULL ) {
      return;
   }
#ifdef USE_MADVISE
   mc2dbg << "[DBC]: madvising" << endl;
   // We will need those pages almost sequentially
   int madvres = madvise( buf->getBufferAddress(),
                          buf->getBufferSize(),
                          MADV_SEQUENTIAL );
   if ( madvres != 0 ) {
      mc2dbg << "[DBC]: madvise failed" << endl;
   }
#endif
}

DataBuffer* 
DataBufferCreator::
getDataBufferFromProgram(const char* command)
{
   FILE* program = popen( command, "r" );
   if ( program == NULL ) {
      return NULL;
   }

   vector<uint8> byteBuffer;
   
   // 10 megabytes of temporary buffer
   const int tmpBufSize = 4 * 4096;
   uint8* tmpBuf = new uint8[tmpBufSize];

   while ( !feof(program) ) {
      int readSize = fread( tmpBuf, 1, tmpBufSize, program );
      byteBuffer.insert( byteBuffer.end(), tmpBuf, tmpBuf + readSize );
   }

   delete [] tmpBuf;
   
   int progRes = pclose(program);
   
   if ( progRes == 0 && byteBuffer.size() > 2 ) {
      DataBuffer* db = new VectorDataBuffer( byteBuffer );
      mc2dbg << "[DataBufferCreator]: Read " << db->getBufferSize()
             << " bytes from " << command << endl;               
      return db;
   } else {
      mc2log << error << "[DataBufferCreator]: "
             << command << " returned to few bytes"
             << endl;
      return NULL;
   }
}

static DataBuffer*
loadGZipped( const char *filename)
{
   std::auto_ptr<DataBuffer> origBuf ( new DataBuffer );
   if ( ! origBuf->memMapFile( filename ) ) {
      return NULL;
   }
   if ( !GunzipUtil::isGzip( origBuf->getBufferAddress() ) ) {
      return NULL;
   }

   memadvise( origBuf.get() );
   
   int origLength = GunzipUtil::origLength( origBuf->getBufferAddress(),
                                            origBuf->getBufferSize() );   
   std::auto_ptr<DataBuffer> resBuf( new DataBuffer( origLength ) );
   mc2dbg << "[DBC]: gunzipping " << filename << endl;
   int expectedSize = origBuf->getBufferSize();
   if ( GunzipUtil::gunzip( resBuf->getBufferAddress(),
                            resBuf->getBufferSize(),
                            origBuf->getBufferAddress(),
                            origBuf->getBufferSize() ) == expectedSize ) {
      return resBuf.release();
   } else {
      return NULL;
   }
}


DataBuffer* 
DataBufferCreator::
loadFromFile( const char* filename ) {
   mc2dbg << "[DataBufferCreator]: Loading from: " << filename << endl;

   // Create a memory-mapped DataBuffer
   std::auto_ptr<DataBuffer> db( new DataBuffer() );
   if ( db->memMapFile( filename ) ) {
      memadvise( db.get() );
      return db.release();
   }

   // if normal map loading failed
   // try .bz2 and .gz
         
   // Try bunzip
   // FIXME: Stat the file first!
   //        And some other error checks.
   //        Also try gzip         

   char* command = new char[ strlen( filename ) + 1 + 500 ];
   sprintf( command, "bunzip2 -c %s.bz2", filename );
   mc2dbg << "[DataBufferCreator]: Trying " << command << endl;

   db.reset( getDataBufferFromProgram( command ) );

   // no databuffer?
   if ( db.get() == NULL ) {
      // Try gunzip if load from .bz2 failed
      MC2String gzname = MC2String(filename) + ".gz";
      db.reset( loadGZipped( gzname.c_str() ) );
   }

   delete [] command;

   return db.release();
}


DataBuffer* 
DataBufferCreator::
loadFromURL( const char* filename ) 
{
   URL url( filename );

   URLFetcherNoSSL fetcher;
   mc2dbg << "[DataBufferCreator]: Will fetch " << url << endl;

   URLFetcherNoSSL::dbPair_t ret = fetcher.get( url );
   if ( ret.first == 200 ) {
      return ret.second;
   }

   delete ret.second;

   return NULL;
}

DataBuffer*
DataBufferCreator::loadFromMapServer( const MC2String& filename )
{
   MC2String url = Properties::getProperty( "MAP_PATH_URL",
                                            "http://server" );
   
   return loadFromURL(
      STLStringUtility::addPathComponent( url, filename ).c_str() );
}

DataBuffer*
DataBufferCreator::loadMapOrIndexDB( const MC2String& filename,
                                     uint32 mapSet ) throw( MC2String )
{
   MC2String fullName = getMapOrIdxPath(filename, mapSet);
   
   // Try loading from file
   auto_ptr<DataBuffer> res ( loadFromFile( fullName.c_str() ) );

   if ( res.get() != NULL ) {
      return res.release();
   }
   
   // Try loading from url
   return loadFromMapServer( fullName );
}

MC2String
DataBufferCreator::getMapOrIdxPath( const MC2String& filename, uint32 mapSet){
   if ( mapSet == MAX_UINT32 ) {
      mapSet = Properties::getMapSet();
   }

   MC2String propName = "MAP_PATH";
   if ( mapSet != MAX_UINT32 ) {
      char tmpStr[24];
      sprintf( tmpStr, "_%u", mapSet );
      propName += tmpStr;
   }

   const char* map_path = Properties::getProperty(propName);
   if ( map_path == NULL ) {
      throw MC2String("[DBC]: Cannot get property " + propName );
      mc2log << error << "[DBC]: Cannot get property " << propName << endl;
   }

   MC2String fullName = STLStringUtility::addPathComponent(map_path,
                                                           filename );
   return fullName;
}
