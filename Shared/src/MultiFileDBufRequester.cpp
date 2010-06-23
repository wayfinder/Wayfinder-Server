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

#include "MultiFileDBufRequester.h"
#include "MultiFileDBufRequesterIndexHandler.h"
#include "MemoryDBufRequester.h"
#include "BitBuffer.h"
#include <stdio.h>
#include <algorithm>

MultiFileDBufRequester::MultiFileDBufRequester( DBufRequester* parent,
                                                const char* basepath,
                                                uint32 maxSize,
                                                int nbrFiles, 
                                                int nbrHash )
      : DBufRequester(parent)
{
   // Alloc the dumpbuffer already so that we know that we have
   // it when shutting down.
   m_dumpBuffer = new BitBuffer(10*1024);
   m_basePath = basepath;
   m_nbrFiles = MAX( nbrFiles, 1 );
   m_nbrHash = MAX( nbrHash, 1 );
   // Don't call setMaxSize until the filehandlers are created.
   m_maxSize = maxSize;   
   m_state = IDLE;
   m_lastWrittenFileNbr = 0;
   m_indexHandler = NULL;
   m_writeQueue = new MemoryDBufRequester(NULL, 256*1024);
   //m_writeQueue = new MemoryDBufRequester(NULL, 50*1024*1024);
   m_readBuffer = NULL;
   m_nbrWrites = 0;
   m_shuttingDown = false;
   m_inStartWriting  = 0;
   m_inStartFinding  = 0;
   m_inStartRemoving = 0;
}

MultiFileDBufRequester::~MultiFileDBufRequester()
{
   m_shuttingDown = true;
   if ( m_indexHandler ) {
      m_indexHandler->shutDown();
   }
   writeRemainders();

   for ( uint32 i = 0; i < m_dataFiles.size(); ++i ) {
      m_dataFiles[i]->cancel();
      delete m_dataFiles[i];
   }
   {for ( uint32 i = 0; i < m_indexFiles.size(); ++i ) {
      m_indexFiles[i]->cancel();
      delete m_indexFiles[i];
   }}
   delete m_dumpFile;
   delete m_readBuffer;
   delete m_indexHandler;
   delete m_writeQueue;
   delete m_dumpBuffer;
}

void
MultiFileDBufRequester::clearFileAndIndex(int nbr)
{
   for ( int i = 0; i < m_nbrHash; ++i ) {
      m_indexFiles[nbr*m_nbrHash + i]->clearFile();
   }
   m_dataFiles[nbr]->clearFile();
   m_indexHandler->clearIndex(nbr);
}

uint32
MultiFileDBufRequester::getAvailableSpace() const
{
   uint32 tmpVal = m_indexFiles[0]->getAvailableSpace();
   return tmpVal;
}

int32
MultiFileDBufRequester::getTotalCacheSize() const
{
   uint32 sum = 0;
   for ( uint32 i = 0; i < m_dataFiles.size(); ++i ) {
      sum += m_dataFiles[i]->getFileSize();
   }
   {for ( uint32 i = 0; i < m_indexFiles.size(); ++i ) {
      sum += m_indexFiles[i]->getFileSize();
   }}
   return sum;
}

int
MultiFileDBufRequester::decIndex(int oldIndex) const
{
   int fileIndex = oldIndex - 1;
   if ( fileIndex < 0 ) {
      fileIndex = m_nbrFiles - 1;
   }
   return fileIndex;
}

int
MultiFileDBufRequester::incIndex(int oldIndex) const
{
   int fileIndex = oldIndex + 1;
   if ( fileIndex >= m_nbrFiles ) {
      fileIndex = 0;
   }
   return fileIndex;
}

int
MultiFileDBufRequester::isBeingRemoved( const MC2SimpleString& desc )
{
   return ( desc == m_currentlyRemoving ) ||
      (m_buffersToRemove.find( desc ) != m_buffersToRemove.end() );
}


uint32
MultiFileDBufRequester::clearOldestNotEmptyFile()
{   
   int fileIndex = incIndex(m_lastWrittenFileNbr);
   int maxNbr = m_nbrFiles;
   
   while ( maxNbr-- && ( getFileSize( fileIndex ) == 0  ) ) {
      fileIndex = incIndex(fileIndex);
   }
   
   uint32 deletedAmount = getFileSize( fileIndex );
  
   clearFileAndIndex(fileIndex); 
   return deletedAmount;
}

uint32
MultiFileDBufRequester::calcMaxAllowedCacheSize( uint32 wantedSize ) const
{
   static const int extraRoom = 512*1024;
   int32 alreadyUsed = getTotalCacheSize();
   // Include the space that we have already used in the calculation.
   // Adjust so that it is never negative.
   int32 available   = MAX( int32(getAvailableSpace()) + 
                            alreadyUsed - extraRoom ,
                            0 );
   // Return the minimum of the two.
   return MIN( wantedSize, uint32(available) );
}

void
MultiFileDBufRequester::setMaxSize(uint32 size)
{
   
   m_newSize = size;
   // Wait with setting the size until next time we're IDLE.
   if ( m_state == IDLE ) {
      // FIXME: Not detecting if we delete so much that we
      // arrive under the limit for the available disk space.
      // Set the max size to the sent in size. We will only
      // use m_maxSizePerFile to check when to delete a file
      m_maxSize = size;

      // Adjust the size for available space
      int nbrLeftToClear = m_nbrFiles;
      do {
         size = calcMaxAllowedCacheSize( m_maxSize );
         m_maxSizePerFile = size / m_nbrFiles + 1;
         if ( size == 0 || int(size) < getTotalCacheSize() ) {
            clearOldestNotEmptyFile();
         }
      } while ( ( (size == 0 ) || ( int(size) < getTotalCacheSize() ) )
                && (--nbrLeftToClear > 0 ) );
   }
}

void
MultiFileDBufRequester::clearCache()
{
   cancelAll();
   for ( int i = 0; i < m_nbrFiles; ++i ) {
      clearFileAndIndex( i );
   }
}

bool
MultiFileDBufRequester::init()
{
   char* tmpPath = new char[ m_basePath.length() + 10 ];
   sprintf(tmpPath, "%s%s%s",
           m_basePath.c_str(), getPathSeparator(), "a" );
   // Will delete the tmpPath
   MC2SimpleStringNoCopy basePath(tmpPath);
   tmpPath = NULL;
   
   // Create the fileHandlers
   m_dataFiles.resize(m_nbrFiles);
   m_indexFiles.resize(m_nbrFiles*m_nbrHash);
   {
      for ( int i = 0; i < m_nbrFiles; ++i ) {
         char nbrBuf[16];
         sprintf(nbrBuf, "%d", i);
         int length = basePath.length() +
            strlen(getPathSeparator()) +
            strlen("fili.") +
            strlen(nbrBuf) + 50;
         char* curDataName = new char[ length ];
         char* curIndexName = new char[ length ];
         sprintf( curDataName, "%s%s%s%s", basePath.c_str(),
                  getPathSeparator(), "fil.", nbrBuf );
         m_dataFiles[i] = createFileHandler( curDataName );

         char hashNbr[16];
         for ( int j = 0; j < m_nbrHash; ++j ) {
            sprintf( hashNbr, "%d", j );
            sprintf( curIndexName, "%s%s%s%s%s%s%s%s",
                     basePath.c_str(),
                     getPathSeparator(),
                     nbrBuf,
                     getPathSeparator(),
                     "fili.",
                     nbrBuf,
                     "_",
                     hashNbr );
            m_indexFiles[i * m_nbrHash + j ] = 
               createFileHandler( curIndexName );
         }
         
         delete [] curIndexName;
         delete [] curDataName;
      }
   }

   // Create the dump file.
   {
      int length = basePath.length() +
         strlen(getPathSeparator()) +
         strlen("fil.dump") + 1;

      char* dumpFileName = new char[ length ];
      sprintf( dumpFileName, "%s%sfil.dump",
               basePath.c_str(),
               getPathSeparator() );
      m_dumpFile = createFileHandler( dumpFileName );
      delete [] dumpFileName;
   }
   // Create file for initial maps
   FileHandler* initialDumpFile = NULL;
   {
      int length = basePath.length() +
         strlen(getPathSeparator()) +
         strlen("initial.dump") + 1;

      char* dumpFileName = new char[ length ];
      sprintf( dumpFileName, "%s%sinitial.dump", basePath.c_str(),
               getPathSeparator() );
      initialDumpFile = createFileHandler( dumpFileName );
      delete [] dumpFileName;
   }

   // Create index handler
   m_indexHandler = new MultiFileDBufRequesterIndexHandler(this,
                                                           m_indexFiles, 
                                                           m_nbrHash );

   setMaxSize( m_maxSize );   
   // Find the newest file and keep adding to it
   uint32 newest = 0;
   uint32 modIdx = 0;
   for ( uint32 i = 0; i < m_indexFiles.size(); ++i ) {
      uint32 modTime = m_indexFiles[i]->getModificationDate();
      if ( modTime > newest ) {
         newest = modTime;
         modIdx = i;
      }
   }
   m_lastWrittenFileNbr = modIdx / m_nbrHash;

   readRemainders(initialDumpFile);
   readRemainders(m_dumpFile);
   delete initialDumpFile;
   
   return true;
}

inline void
MultiFileDBufRequester::setState( state_t newState )
{
   m_state = newState;
}

void
MultiFileDBufRequester::release( const MC2SimpleString& desc,
                                 BitBuffer* buffer )
{
   mc2dbg2 << "[MultiFileDBufRequester::release("
           << MC2CITE(desc) << ")]" << endl;
   if ( m_maxSizePerFile > 10000 ) {
      m_writeQueue->release( desc, buffer );
   } else {
      if ( ( m_nbrWrites++ & 127 ) == 127 ) {
         // Check if there is available space on the disk again.
         setMaxSize(m_maxSize);
      }
      DBufRequester::release( desc, buffer );
   }
   // Check if it is time to start working.
   startWorking();
}

BitBuffer*
MultiFileDBufRequester::requestCached(const MC2SimpleString& descr)
{
   return m_writeQueue->requestCached(descr);
}

void
MultiFileDBufRequester::internalRemove( const MC2SimpleString& descr )
{
   mc2dbg2 << "[MultiFileDBufRequester::internalRemove]" << endl;
   // Remove it from the write queue at once
   m_writeQueue->removeBuffer( descr );
   // Add it to the set of buffers that should be removed
   m_buffersToRemove.insert( descr );
   // Start working
   startWorking();
}

void
MultiFileDBufRequester::request(const MC2SimpleString& descr,
                                DBufRequestListener* caller)
{
   mc2dbg2 << "[MultiFileDBufRequester::request("
           << MC2CITE(descr) << ")]" << endl;
   BitBuffer* maybeCached = requestCached(descr);
   if ( maybeCached ) {
      caller->requestReceived( descr, maybeCached );
      return;
   }
   // Check if the buffer is about to be removed
   if ( ! isBeingRemoved(descr) ) {
      // Not in memory - check queue.
      pair<MC2SimpleString, DBufRequestListener*>
         thePair(descr, caller);
      
      if ( std::find( m_readQueue.begin(), m_readQueue.end(), thePair ) ==
           m_readQueue.end() ) {
         // Not in queue - push back
         m_readQueue.push_back( thePair );
      } else {
         mc2dbg2 << "[MFDBR]: " << descr << " already in queue" << endl;
      }
   }         
   // Start reading if not reading already.
   startWorking();
}

void
MultiFileDBufRequester::cancelAll()
{
   mc2dbg2 << "[MultiFileDBufRequester::cancelAll]" << endl;
   m_readQueue.clear();
   DBufRequester::cancelAll();
}

void
MultiFileDBufRequester::findComplete(const MC2SimpleString& desc,
                                     int fileNbr,
                                     int startOffset,
                                     int size )
{
   if ( m_shuttingDown ) {
      return;
   }
   MC2_ASSERT( m_state == LOOKING_FOR_MAP ||
               m_state == WRITING_LOOKING_FOR_MAP );
   
   if ( m_state == LOOKING_FOR_MAP ) {
      mc2dbg2 << "[MFDBR]: Find complete " << endl;
      MC2_ASSERT( desc == m_currentlyReading.first );
      mc2dbg2 << "[MFDBR]: m_maxSizePerFile = " << m_maxSizePerFile << endl;
      
      
      setState( IDLE ); 
      if ( startOffset < 0 || size < 0 ) {
         // Not found - request from parent
         if ( m_parentRequester ) {
            m_parentRequester->request( m_currentlyReading.first,
                                        m_currentlyReading.second );
         } else {
            mc2dbg2 << "[MFDBR]: No parent" << endl;
         }
         startWorking();
      } else {
         mc2dbg2 << "[MFDBR]: Map " << desc << "found in cache " << endl;
         // Continue by reading the stuff from the file.
         setState( READING_MAP );
         MC2_ASSERT( m_readBuffer == NULL );
         MC2_ASSERT( size != 0 );
         m_readBuffer = new BitBuffer( size );
         m_dataFiles[fileNbr]->setPos( startOffset );
         m_dataFiles[fileNbr]->read( m_readBuffer->getBufferAddress(),
                                     m_readBuffer->getBufferSize(),
                                     this );
      }
   } else {
      MC2_ASSERT( m_state == WRITING_LOOKING_FOR_MAP );
      if ( ( startOffset >= 0 ) && (size >= 0 ) ) {
         mc2dbg2 << "[MFDBR]: Not writing map - "
                << MC2CITE( m_currentlyWriting.first)
                << "it was in one of the last files " << endl;
         setState( WRITING_INDEX );
         // Delete the map.
         DBufRequester::release( m_currentlyWriting.first,
                                 m_currentlyWriting.second );
         m_currentlyWriting.second = NULL;
         indexWritten( m_currentlyWriting.first );
      } else {
         // The code that used to be in startWriting.
         mc2dbg2 << "[MFDBR]: Will start writing " << endl;
         setState( WRITING_MAP_DATA );         
         m_dataFiles[m_lastWrittenFileNbr]->setPos( -1 );
         m_startWritePos = m_dataFiles[m_lastWrittenFileNbr]->tell();
         m_dataFiles[m_lastWrittenFileNbr]->write(
            m_currentlyWriting.second->getBufferAddress(),
            m_currentlyWriting.second->getBufferSize(),
            this);
      }                                    
   }
}

void
MultiFileDBufRequester::readDone( int nbrRead )
{
   if ( m_shuttingDown ) {
      return;
   }
   MC2_ASSERT( m_state == READING_MAP );
   setState( IDLE );
   if ( nbrRead != (int)m_readBuffer->getBufferSize() ) {
      mc2dbg2 << "[MFDBR]: Failed to read map" << endl;
      delete m_readBuffer;
      m_readBuffer = NULL;
      if ( m_parentRequester ) {
         m_parentRequester->request( m_currentlyReading.first,
                                     m_currentlyReading.second );
      }
   } else {      
      // All ok - copy all data.
      BitBuffer* tmpBuffer = m_readBuffer;
      m_readBuffer = NULL;
      MC2SimpleString tmpStr( m_currentlyReading.first );
      DBufRequestListener* listener = m_currentlyReading.second;
      m_currentlyReading.first  = "";
      m_currentlyReading.second = NULL;
      // And then tell the listener about the wonderful new buffer.
      mc2dbg2 << "[MFDBR]: Sending " << MC2CITE( tmpStr ) << " to listener"
             << endl;
      listener->requestReceived( tmpStr,
                                 tmpBuffer );
   }
   startWorking();
}

void
MultiFileDBufRequester::writeDone( int nbrWritten )
{
   if ( m_shuttingDown ) {
      return;
   }
   MC2_ASSERT( m_state == WRITING_MAP_DATA );
   ++m_nbrWrites;
   mc2dbg2 << "[MFDBR]: writeDone " << endl;
   setState( IDLE );

   // Save values needed.
   int bufSize = m_currentlyWriting.second->getBufferSize();

   
   // Get rid of the buffer.
   DBufRequester::release( m_currentlyWriting.first,
                           m_currentlyWriting.second );
   m_currentlyWriting.second = NULL;
   
   if ( nbrWritten == bufSize ) {
      // Update the index too.
      setState( WRITING_INDEX );
      m_indexHandler->updateMapIndex( m_lastWrittenFileNbr,
                                      m_currentlyWriting.first,
                                      m_startWritePos,
                                      nbrWritten );
   } else {
      mc2dbg2 << "[MFDBR]: Write error" << endl;
      startWorking();
   }
}

void
MultiFileDBufRequester::indexWritten(const MC2SimpleString& desc)
{
   if ( m_shuttingDown ) {
      return;
   }
   // Some sanity checks
   MC2_ASSERT( m_state == WRITING_INDEX );
   MC2_ASSERT( desc == m_currentlyWriting.first );
   MC2_ASSERT( m_currentlyWriting.second == NULL );
   setState ( IDLE );
   
   uint32 sizeSum = getFileSize( m_lastWrittenFileNbr );
   
   if ( sizeSum >= m_maxSizePerFile ) {
      mc2dbg2 << "[MFDBR]: Moving to next file" << endl;
      m_lastWrittenFileNbr = (m_lastWrittenFileNbr + 1) % m_nbrFiles;
      clearFileAndIndex( m_lastWrittenFileNbr );
   }
   

   if ( (m_nbrWrites & 31) == 31 ) {
      // Check disk space
      setMaxSize( m_newSize );
   }
   mc2dbg2 << "[MFDBR]: Index written too " << endl;
   mc2dbg2 << "[MFDBR]: m_maxSizePerFile = " << m_maxSizePerFile << endl;
   startWorking();
}

void
MultiFileDBufRequester::startFinding()
{
   if ( m_inStartFinding ) {
      return;      
   }
   m_inStartFinding++;
   while ( ( m_state == IDLE ) && ( ! m_readQueue.empty() ) ) {
      // Only start the reading process if we are idle.
      BitBuffer* cached = NULL;
      m_currentlyReading = m_readQueue.front();
      m_readQueue.pop_front();
      // Check if it is in the write queue first.
      cached = requestCached( m_currentlyReading.first );
      if ( cached ) {         
         m_currentlyReading.second->requestReceived(
            m_currentlyReading.first,
            cached );
         continue;
      }
      setState ( LOOKING_FOR_MAP );
      // The following function may call startFinding via startWorking
      // from findComplete
      m_indexHandler->find( m_currentlyReading.first,
                            m_lastWrittenFileNbr );
   }
   m_inStartFinding--;
}

void
MultiFileDBufRequester::startWriting()
{
   // Avoid recursion - m_indexHandler->find can call findComplete
   // and then we will get here again.
   if ( m_inStartWriting ) {
      return;
   }
   m_inStartWriting++;  
   while ( ( m_state == IDLE ) && ( ! m_writeQueue->empty() ) ) {
      m_currentlyWriting = m_writeQueue->front();
      m_writeQueue->pop_front();
      setState ( WRITING_LOOKING_FOR_MAP );
      // Look for the map in the two newest files.
      m_indexHandler->find( m_currentlyWriting.first,
                            m_lastWrittenFileNbr,
                            MIN(2, m_nbrFiles) );
      // If the indexhandler has called findComplete then we are idle
      // again here.      
   }
   m_inStartWriting--;
}

void
MultiFileDBufRequester::removeComplete( const MC2SimpleString& desc)
{
   MC2_ASSERT( desc == m_currentlyRemoving );
   MC2_ASSERT( m_state == REMOVING_MAP );
   mc2dbg2 << "[MFDBR]: removeComplete" << endl;
   setState( IDLE );
   m_currentlyRemoving = "";
   startWorking();
}

void
MultiFileDBufRequester::startRemoving()
{
   if ( m_inStartRemoving ) {
      return;
   }
   ++m_inStartRemoving;
   while ( ( m_state == IDLE ) && ( ! m_buffersToRemove.empty() ) ) {
      MC2_ASSERT( m_currentlyRemoving == "" );
      m_currentlyRemoving = *(m_buffersToRemove.begin());
      m_buffersToRemove.erase( m_buffersToRemove.begin() );
      setState( REMOVING_MAP );
      m_indexHandler->remove( m_currentlyRemoving );
   }
   --m_inStartRemoving;
}

void
MultiFileDBufRequester::startWorking()
{
   if ( !m_shuttingDown ) {
      if ( m_state == IDLE ) {
         // Safe to do tricks with the files
         if ( m_maxSize != m_newSize ) {
            setMaxSize( m_newSize );
         }
      }
      
      if ( m_state == IDLE ) {
         startRemoving();
      }
      
      if ( m_state == IDLE ) {
         startFinding();
      } 
      // Check if we are still idle.
      if ( m_state == IDLE ) {
         // Start writing etc.
         startWriting();      
      }
   }
}

void 
MultiFileDBufRequester::writeRemainders()
{
   typedef MemoryDBufRequester::stringBufPair_t stringBufPair_t;

   m_dumpBuffer->reset();
   
   /* write the number of maps */
   m_dumpBuffer->writeNextBALong( 0 ); // Fill in later.
   m_dumpFile->write( m_dumpBuffer->getBufferAddress(), 
                      m_dumpBuffer->getCurrentOffset(), 
                      NULL ); // Synch
   m_dumpBuffer->reset();
   
   uint32 i = 0;
   while ( !m_writeQueue->empty() ) {
      //m_dumpBuffer->reset();
      
      int freeDisk = m_dumpFile->getAvailableSpace() - 1024*512;
      if ( freeDisk < 0 ) {
         // No more disk! Jump out.
         mc2dbg2 << "writeRemainders: Out of disk at map " << i << endl;
         break;
      }
      
      MemoryDBufRequester::stringBufPair_t& stringBuf = 
         m_writeQueue->front();

      const MC2SimpleString& curDesc = stringBuf.first;  
      BitBuffer* curBuf = stringBuf.second;

      if ( ! isBeingRemoved ( curDesc ) ) {
         m_dumpBuffer->writeNextBALong( curDesc.length() );
         m_dumpBuffer->writeNextByteArray( (const byte*)curDesc.c_str(),
                                           curDesc.length() );
         m_dumpBuffer->writeNextBALong( curBuf->getBufferSize() );
         // Check how much room is left in the m_dumpBuffer->
         uint32 room =
            m_dumpBuffer->getBufferSize() - m_dumpBuffer->getCurrentOffset();
         // Make sure that there is room for the next description and lengths
         // too.
         if ( room > ( curBuf->getBufferSize() + 80 ) ) {
            mc2dbg2 << "[MFDBR]: Adding to old buffer room = "
                   << room << " bufsize = " << curBuf->getBufferSize() << endl;
            // Write the buffer into the temp buffer and then all at once
            // to the file.
            m_dumpBuffer->writeNextByteArray( curBuf->getBufferAddress(),
                                              curBuf->getBufferSize() );
         } else {
            mc2dbg2 << "[MFDBR]: Writing buffer room = "
                   << room << " bufsize = " << curBuf->getBufferSize() << endl;
            // Write the buffers separately.
            m_dumpFile->write( m_dumpBuffer->getBufferAddress(),
                               m_dumpBuffer->getCurrentOffset(),
                               NULL ); // Synch
            m_dumpFile->write( curBuf->getBufferAddress(),
                               curBuf->getBufferSize(),
                               NULL ); // Synch
            m_dumpBuffer->reset();
         }
      } else {
         // Buffer should be removed so don't write it.
      }
      
      m_writeQueue->pop_front();
      delete curBuf;
      ++i;
   }
   
   m_dumpFile->write( m_dumpBuffer->getBufferAddress(),
                      m_dumpBuffer->getCurrentOffset(),
                      NULL ); // Sync
   m_dumpBuffer->reset();
   
   // Write nbr of maps.
   m_dumpBuffer->writeNextBALong( i );
   m_dumpFile->setPos( 0 );
   m_dumpFile->write( m_dumpBuffer->getBufferAddress(), 
                      m_dumpBuffer->getCurrentOffset(), 
                      NULL ); // Synch
}

void 
MultiFileDBufRequester::readRemainders(FileHandler* dumpFile)
{
   
   BitBuffer tempBuf( 1024 );

   // Read the number of maps.
   if ( dumpFile->read( tempBuf.getBufferAddress(), 4, NULL ) != 4 ) {
      // The dumpfile was empty.
      return;
   }
   uint32 nbrMaps = tempBuf.readNextBALong();

   for ( uint32 i = 0; i < nbrMaps; ++i ) {

      tempBuf.reset();
     
      // Length of descr.
      dumpFile->read( tempBuf.getBufferAddress(), 4, NULL ); // Synch
      uint32 descSize = tempBuf.readNextBALong();

      tempBuf.reset();

      // Descr.
      dumpFile->read( tempBuf.getBufferAddress(),
                        descSize, NULL ); // Synch
      
      char* desc = new char[ descSize + 1];
      tempBuf.readNextByteArray( (byte*) desc, descSize );
      desc[ descSize ] = '\0';

      MC2SimpleString curDesc = desc;
      delete[] desc;
      tempBuf.reset();
      
      // Buffer size. 
      dumpFile->read( tempBuf.getBufferAddress(), 4, NULL ); // Synch
      uint32 bufSize = tempBuf.readNextBALong();
     
      // The buffer.
      BitBuffer* curBuf = new BitBuffer( bufSize );
      dumpFile->read( curBuf->getBufferAddress(), 
                        bufSize, NULL ); // Synch

      m_writeQueue->release( curDesc, curBuf );   
   }

   // Clear the dumpfile once we are finished.
   dumpFile->clearFile();
}
