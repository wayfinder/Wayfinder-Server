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

#include "MapHttpServer.h"
#include "M3Creator.h"
#include "GenericMap.h"

#include <memory>
#include <stdexcept>

#include "TCPSocket.h"
#include "HttpHeader.h"
#include "DataBuffer.h"
#include "IPnPort.h"
#include "stat_inc.h"
#include "MonitorSet.h"
#include "File.h"
#include "Convert.h"
#include "STLStringUtility.h"
#include "PropertyHelper.h"

class MapHttpServerReqHandler : public ISABThread {
public:

   MapHttpServerReqHandler( TCPSocket* sock, MonitorSet<MC2String> &mapSet) : 
      m_sock( sock ),
      m_mapSet(mapSet)
   {}

   void run();
   
private:

   /// Return null if errro
   const MC2String* readline();

   /// Return null if error.
   HttpHeader* readHeader();

   /// Creates the out-header
   HttpHeader* createHeader( int status, size_t contentLength );

   /// Sends an HttpHeader
   bool sendHeader( const HttpHeader& header );
   
   /// Sends an MC2String back to client
   int sendString( const MC2String& str );

   /// Sends an error document
   void sendError( int status );
   void sendFile( const MC2String &fileName );


   auto_ptr<TCPSocket> m_sock;

   MC2String m_line;
  
   MonitorSet<MC2String> &m_mapSet;

};



int
MapHttpServerReqHandler::sendString( const MC2String& str )
{
   return m_sock->writeAll( reinterpret_cast<const uint8*>( str.c_str() ),
                            str.length() );
}

bool
MapHttpServerReqHandler::sendHeader( const HttpHeader& header )
{
   MC2String header_as_string;
   header.putHeader( &header_as_string );
   header_as_string.append( "\r\n" );
   return sendString( header_as_string ) == int(header_as_string.length());
}

const MC2String*
MapHttpServerReqHandler::readline()
{
   m_line.clear();
   char buf;
   while ( ( m_sock->read( reinterpret_cast<uint8*>(&buf), 1 ) == 1 ) ) {
      if ( buf == 13 ) {
         // Skip CR
         continue;
      }
      if ( buf == 10 ) {
         mc2dbg8 << "[MHAT]: readline: " << m_line << endl;
         return &m_line;
      }
      m_line += buf;
   }
   mc2dbg << "[MHAT]: Error reading line" << endl;
   return NULL;
}

HttpHeader*
MapHttpServerReqHandler::readHeader()
{
   HttpHeader* header = new HttpHeader;
   bool firstLine = true;
   for ( const MC2String* curLine = readline();
         curLine != NULL;
         curLine = readline() ) {
      if ( curLine->empty() ) {
         return header;
      }
      if ( firstLine ) {
         header->addRequestLine( curLine->c_str() );
         firstLine = false;
      } else {
         header->addHeaderLine( curLine->c_str() );
      }
   }
   mc2dbg << "[MHAT]: Error reading header " << endl;
   delete header;
   return NULL;
}

HttpHeader*
MapHttpServerReqHandler::createHeader( int status,
                                       size_t contentLength )
{
   HttpHeader* newHeader = new HttpHeader();

   char tmpChr[1024];
   sprintf( tmpChr, "HTTP/1.0 %d %s\r\n", status,
            ( status == 200 ) ? "OK" : "ERROR" );     
   newHeader->setStartLine( new MC2String( tmpChr ) );
   if ( contentLength ) {
      sprintf( tmpChr, "Content-Length: %lu", 
               static_cast<unsigned long>( contentLength ) );
      newHeader->addHeaderLine( tmpChr );
   }
   newHeader->addHeaderLine( "Server: MapHttpServerReqHandler" );
   return newHeader;
}

void
MapHttpServerReqHandler::sendError( int status )
{
   mc2dbg << "[MHAT]: sendError(" << status << ")" << endl;
   char errorDocu[4096];

   sprintf( errorDocu, "<html><head><title>Error %d</title></head>"
            "<body><h1>Error %d has occurred</h1></body></html>",
            status, status );

   MC2String msg( errorDocu );
   
   auto_ptr<HttpHeader> header( createHeader(status, msg.length() ) );

   if ( sendHeader( *header ) ) {
      sendString( errorDocu );
   }
}



void
MapHttpServerReqHandler::sendFile( const MC2String &fileName ) {
   // Get the map and write it.
   DataBuffer buf;
   buf.memMapFile( fileName.c_str(), false );

   auto_ptr<HttpHeader> out_header( createHeader( 200, buf.getBufferSize() ) );
   
   if ( sendHeader( *out_header ) ) {
      // send header succesfull. Now send the file

      mc2dbg << "[MHAT]: Sending file " << fileName << endl;

      m_sock->writeAll( buf.getBufferAddress(),
                        buf.getBufferSize() );

      mc2dbg << "[MHAT]: " << buf.getBufferSize() << " bytes sent" << endl;
   } else {
      throw MC2String("Error sending header. ") + __FUNCTION__;
   }

}

void
MapHttpServerReqHandler::run() try {

   // Get the header
   auto_ptr<HttpHeader> in_header( readHeader() );
   if ( in_header.get() == NULL ) {
      throw MC2String("Header NULL");
   }

   if ( in_header->getPagename() == NULL ) {
      throw MC2String("PageName NULL in in_header!");
   }

   // determine map path

   MC2String urlPath = *in_header->getURLPath();

   MC2String sourceFilename, destFilename;

   // We do use the destination path
   MC2String mapPath;
   Convert::getMapPaths( 0, destFilename, mapPath );

   mc2dbg << "URL path is: " << urlPath << endl;
   mc2dbg << "destFilename is: " << destFilename << endl;

   // very simple find, assumes we dont have similar path in 
   // the src path....
   MC2String::size_type pos = urlPath.find( destFilename );
   if ( pos == MC2String::npos ) {
      throw MC2String("URL did not specify the correct destination path.");
   } else { 
      // if we dont have '/' at the end of the destFilename
      // then the offset will be wrong if we substract 1
      int offset = 1;
      if ( urlPath[pos + destFilename.size()] == '/' )
         offset = 0;

      sourceFilename = STLStringUtility::basename( 
         urlPath.erase( 0, pos + destFilename.size() - offset ) );
         
   }
   mc2dbg << "sourceFilename " << sourceFilename << endl;
   mc2dbg << "Using src path: " << STLStringUtility::dirname( sourceFilename ) << endl;

   //destFilename = *in_header->getURLPath();

   //
   // if the client only wants index.db then we dont check 
   // the .m3 format string and send it right away
   //
   if ( STLStringUtility::basename( sourceFilename ) != "index.db" ) {
      //
      // check .m3 format
      //
      int srcFileVersion = 
         GenericMapHeader::getMapVersionFromFilename( sourceFilename );
      if ( srcFileVersion == -1 ) {
         throw MC2String("Could not determine file version from file: ") +
            sourceFilename;
      }

      mc2dbg << "[MHAT]: source filename: " << sourceFilename << endl;
      mc2dbg << "[MHSRQ]: File version: " << srcFileVersion << endl;

      if ( static_cast<uint32>( srcFileVersion ) != 
           GenericMapHeader::getMapVersion() ) {
         throw MC2String("Wrong Map Version! File: ") + sourceFilename;
      }
      // strip -<num>.m3 from src filename
      sourceFilename.erase( sourceFilename.
                            find_last_not_of( "0123456789",
                                         sourceFilename.length() - 4 ) );

      sourceFilename += ".mcm";

      destFilename.erase( destFilename.
                            find_last_not_of( "0123456789",
                                         destFilename.length() -4 ) );
    
      char buff[ 16 ];
      sprintf( buff, "-%d.m3", GenericMapHeader::getMapVersion() );
      destFilename += buff;
   }

   mc2dbg << "[MHAT]: source filename: " << sourceFilename << endl;
   mc2dbg << "[MHAT]: dest filename: " << destFilename << endl;

   if ( ! File::fileExist( destFilename ) ) {
      // lock index.db if needed
      std::auto_ptr<LockHelper<MC2String> > indexLock;
      if ( STLStringUtility::basename( destFilename ) != "index.db" &&
           ! File::fileExist( STLStringUtility::dirname( destFilename ) + "/"
                              + "index.db" ) ) {
         indexLock.reset( new LockHelper<MC2String>( m_mapSet, 
                              STLStringUtility::dirname( destFilename ) + "/" 
                              + "index.db" ) );
      }

      LockHelper<MC2String> lock( m_mapSet, destFilename );

      Convert::convertFile( mapPath + sourceFilename, destFilename );

   } else {
      mc2dbg << "[MHAT]: file: " << destFilename 
             << " already exist, using it." << endl;
   }

   // file is now ready for sending
   sendFile( destFilename );
      
  
} catch ( PropertyException err ) {
   mc2dbg << "[MHAT]: " << err.what() << endl;
   sendError( 404 );
} catch ( MC2String err ) {
   mc2dbg << "[MHAT]: " << err << endl;
   sendError( 404 );      
} catch ( std::out_of_range err) {
   mc2dbg << "[MHAT]: " << err.what() << endl;
   sendError( 404 );
}

// -- MapHttpAcceptorThread

class MapHttpAcceptorThread : public ISABThread {
public:

   MapHttpAcceptorThread( uint16 port, bool convertAll );
   
   void run();
   
private:

   /// Starts a handler
   void handleConnection( TCPSocket* sock );

   uint16 m_listenPort;

   MonitorSet<MC2String> m_mapSet;
   bool m_convertAll;
};

MapHttpAcceptorThread::MapHttpAcceptorThread( uint16 port, bool convertAll ):
   m_listenPort( port ),
   m_convertAll( convertAll )
{
}

void
MapHttpAcceptorThread::handleConnection( TCPSocket* sock )
{
   ISABThreadHandle newThread = new MapHttpServerReqHandler( sock, m_mapSet);
   newThread->start();
}

void
MapHttpAcceptorThread::run()
{
  
   if ( m_convertAll  ) {
      uint32 nbrMapSet = PropertyHelper::get<int>("MAP_SET_COUNT");

      mc2dbg << "[MHAT]: converting all " << nbrMapSet << 
         " map sets." << endl;
      vector<uint32> mapIDs;

      // convert from index and return
      for ( uint32 i = 0; i < nbrMapSet ; ++i ) try {
         Convert::convertFromIndex( i, mapIDs );
      } catch ( MC2String err ) {
         mc2dbg << "[MHAT]: " << err << endl;
      } catch ( PropertyException err ) {
         mc2dbg << "[MHAT]: " << err.what() << endl;
      }

      return;
   }

   TCPSocket socket;
   socket.open();
   socket.listen( m_listenPort );

   mc2dbg << "[MHAT]: Listening on port " << m_listenPort << endl;
   
   while ( ! terminated ) {
      TCPSocket* tmp = socket.accept();
      IPnPort addr( 0, 0 );
      tmp->getPeerName( addr.first, addr.second );
      mc2dbg << "[MHAT]: Accepted connection from "
             << addr << endl;
      handleConnection( tmp );
   }
   
}



// -- MapHttpServer

MapHttpServer::MapHttpServer( uint16 port, bool convertAll )
{
   m_acceptorThread = new MapHttpAcceptorThread( port, convertAll );
}

MapHttpServer::~MapHttpServer()
{
   terminate();   
}

void
MapHttpServer::start()
{
   m_acceptorThread->start();
}

void
MapHttpServer::run()
{
   m_acceptorThread->run();
}

void
MapHttpServer::terminate()
{
   m_acceptorThread->terminate();
}




