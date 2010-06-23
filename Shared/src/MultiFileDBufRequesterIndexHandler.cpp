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
#include "MultiFileDBufRequesterIndexHandler.h"
#include "MultiFileDBufRequester.h"
#include "BitBuffer.h"

#define SET_STATE(s) setState(s, __LINE__)

MultiFileDBufRequesterIndexHandler::
MultiFileDBufRequesterIndexHandler(MultiFileDBufRequester* listener,
                                   vector <FileHandler*>& indexFiles,
                                   int nbrHash )
      : m_indexFiles(indexFiles),
        m_currentIndexFile(0),
        m_nbrHash( nbrHash )
{
   m_nbrFiles = indexFiles.size() / nbrHash;
   m_listener = listener;
   m_readBuffer = NULL;
   m_writeBuffer = NULL;
   m_indeces.resize( m_indexFiles.size() );
   // VC++ does not handle these being non-pointers.
   for ( uint32 i = 0; i < m_indeces.size(); ++i ) {
      m_indeces[i] = new indexMap_t;
   }
   m_state = IDLE;
   m_shuttingDown = false;
}

MultiFileDBufRequesterIndexHandler::
~MultiFileDBufRequesterIndexHandler()
{
   m_shuttingDown = true;
   delete m_writeBuffer;
   delete m_readBuffer;
   for ( uint32 i = 0; i < m_indeces.size(); ++i ) {
      delete m_indeces[i];
   }
}

void
MultiFileDBufRequesterIndexHandler::clearIndex(int nbr)
{
   for ( int i = 0; i < m_nbrHash; ++i ) {
      m_indeces[ nbr * m_nbrHash + i ]->clear();
   }
}

void
MultiFileDBufRequesterIndexHandler::shutDown()
{
   m_shuttingDown = true;
}

void
MultiFileDBufRequesterIndexHandler::setState(state_t newState,
                                             int line)
{
   mc2dbg2 << "[MFDBRIH]: " << __FILE__ << ":" << line
          << " State change "
          << m_state << " -> " << newState << endl;
   m_state = newState;
}

void
MultiFileDBufRequesterIndexHandler::find( const MC2SimpleString& desc,
                                          int lastWrittenFileNbr,
                                          int maxNbrFiles )
{
   MC2_ASSERT( m_state == IDLE );
   m_mode = MODE_FINDING;
   m_nbrFilesLeftToSearch = m_nbrFiles;
   if ( maxNbrFiles > 0 ) {
      m_nbrFilesLeftToSearch = maxNbrFiles;
   }
   m_currentIndexFile     = lastWrittenFileNbr;
   m_currentDescToFind    = desc;
   startFinding();
}

void
MultiFileDBufRequesterIndexHandler::
updateMapIndex( int fileIdx,
                const MC2SimpleString& desc,
                int startOffset,
                int size )
{
   MC2_ASSERT( m_state == IDLE );
   m_mode = MODE_WRITING;
   m_currentIndexFile  = fileIdx;
   m_currentDescToFind = desc;
   m_currentWritePosAndSize = make_pair(startOffset, size);
   
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );

   // Start working
   // Get the size of the index.
   if ( ! m_indeces[ idx ]->empty() ) {
      addCurrentMapToCurrentIndex();
      writeIndex();
      return;
   }
   m_indeces[ idx ]->clear();
   SET_STATE( WRITING_MAP_READING_INDEX_LENGTH );
   allocReadBuffer(4);
   m_indexFiles[ idx ]->setPos(0);
   m_indexFiles[ idx ]->read(m_readBuffer->getBufferAddress(),
                                          m_readBuffer->getBufferSize(),
                                          this);
}

void
MultiFileDBufRequesterIndexHandler::remove( const MC2SimpleString& desc )
{
   MC2_ASSERT( m_state == IDLE );
   m_mode = MODE_REMOVING;
   m_currentIndexFile = 0;
   m_currentDescToFind = desc;
   m_nbrFilesLeftToSearch = m_nbrFiles;
   startFinding();
}

void
MultiFileDBufRequesterIndexHandler::removeComplete()
{
   SET_STATE( IDLE );
   m_listener->removeComplete( m_currentDescToFind );
}

void
MultiFileDBufRequesterIndexHandler::allocReadBuffer(int size)
{
   delete m_readBuffer;
   m_readBuffer = new BitBuffer( size );
}

void
MultiFileDBufRequesterIndexHandler::startFinding()
{
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
   if ( ! m_indeces[ idx ]->empty() ) {
      if ( m_mode == MODE_FINDING ) {
         indexMap_t::iterator findit = 
            m_indeces[ idx ]->find( m_currentDescToFind );
         if ( findit != m_indeces[ idx ]->end() ) {
            // Found the desc.
            findComplete( (*findit).second.first, (*findit).second.second );
         } else {
            // Move to next index.
            moveToNextIdxOrStop();
         }
      } else {
         MC2_ASSERT( m_mode == MODE_REMOVING );
         // Will do the moving to next file etc.
         removeFromIndex();
      }
      return;
   }
   m_indeces[ idx ]->clear();
   // Get the size of the index.
   if ( m_mode == MODE_FINDING ) {
      SET_STATE( FINDING_MAP_READING_INDEX_LENGTH );
   } else {
      SET_STATE( REMOVING_MAP_READING_INDEX_LENGTH );
   }
   allocReadBuffer(4);
   // Rewind the file
   m_indexFiles[ idx ]->setPos(0);
   // Request the read.
   m_indexFiles[ idx ]->read(m_readBuffer->getBufferAddress(),
                                          m_readBuffer->getBufferSize(),
                                          this);
}

void
MultiFileDBufRequesterIndexHandler::startReadingIndex()
{
   MC2_ASSERT( m_state == FINDING_MAP_READING_INDEX_LENGTH ||
               m_state == WRITING_MAP_READING_INDEX_LENGTH ||
               m_state == REMOVING_MAP_READING_INDEX_LENGTH );
   
   m_readBuffer->reset();
   int indexLength = m_readBuffer->readNextBALong();
   // Get the index
   if ( m_state == FINDING_MAP_READING_INDEX_LENGTH ) {
      SET_STATE( FINDING_MAP_READING_INDEX );
   } else if ( m_state == WRITING_MAP_READING_INDEX_LENGTH ) {
      SET_STATE ( WRITING_MAP_READING_INDEX );
   } else if ( m_state == REMOVING_MAP_READING_INDEX_LENGTH ) {
      SET_STATE( REMOVING_MAP_READING_INDEX );
   }
   
   allocReadBuffer( indexLength );
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
   m_indexFiles[ idx ]->read( m_readBuffer->getBufferAddress(),
                                             m_readBuffer->getBufferSize(),
                                             this );
}

void
MultiFileDBufRequesterIndexHandler::createIndex()
{
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
   if ( ! m_indeces[ idx ]->empty() ) {
      return;
   }
   BitBuffer& indexBuf = *m_readBuffer;
   indexBuf.reset();
   m_indeces[ idx ]->clear();
   int nbrEntries = indexBuf.readNextBALong();
   for ( int i = 0; i < nbrEntries; ++i ) {
      const char* descr = indexBuf.readNextString();
      int startOffset = indexBuf.readNextBALong();
      int size = indexBuf.readNextBALong();
      // Insert the read index entry last in the map since it is
      // already sorted.
      m_indeces[ idx ]->insert(
         m_indeces[ idx ]->end(),
         make_pair(descr, make_pair(startOffset,
                                    size ) ) );
   }   
}

MultiFileDBufRequesterIndexHandler::offsetAndSize_t
MultiFileDBufRequesterIndexHandler::findMapInCurrentIndexBuffer()
{
#if 0
   BitBuffer& indexBuf = *m_readBuffer;   
   indexBuf.reset();

   int nbrEntries = indexBuf.readNextBALong();
   for ( int i = 0; i < nbrEntries; ++i ) {
      const char* descr = indexBuf.readNextString();
      if ( m_currentDescToFind < descr ) {
         return offsetAndSize_t(-1, -1);
      }
      int startOffset = indexBuf.readNextBALong();
      int size = indexBuf.readNextBALong();
      // Insert the read index entry last in the map since it is
      // already sorted.
      if ( m_currentDescToFind == descr ) {
         return offsetAndSize_t(startOffset, size);
      }
   }
   return offsetAndSize_t(-1, -1);
#else 
   createIndex();
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
   indexMap_t::iterator findit = 
      m_indeces[ idx ]->find( m_currentDescToFind );
   if ( findit != m_indeces[ idx ]->end() ) {
      return (*findit).second;
   } else {
      return offsetAndSize_t(-1, -1);
   }
#endif
}

void
MultiFileDBufRequesterIndexHandler::writeIndex()
{
   MC2_ASSERT( m_writeBuffer == NULL );
   SET_STATE( WRITING_INDEX );
   int stringSum = 0;
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
   int indexSize = m_indeces[ idx ]->size();
   {
      for ( indexMap_t::const_iterator it =
               m_indeces[ idx ]->begin();
            it != m_indeces[ idx ]->end();
            ++it ) {
         stringSum += it->first.length() + 1;
      }
   }
   m_writeBuffer = new BitBuffer( 4 + 4 + stringSum + ( indexSize * 8 ) );
   m_writeBuffer->writeNextBALong( m_writeBuffer->getBufferSize() - 4 );
   m_writeBuffer->writeNextBALong( indexSize );
   for ( indexMap_t::const_iterator it = m_indeces[ idx ]->begin();
         it != m_indeces[ idx ]->end();
         ++it ) {
      m_writeBuffer->writeNextString( it->first.c_str() );
      m_writeBuffer->writeNextBALong( it->second.first );
      m_writeBuffer->writeNextBALong( it->second.second );
   }
   // Check if to clear the index.
   if ( m_writeBuffer->getCurrentOffset() > 10*1024 ) {
      mc2dbg2 << "[MFDBRIH]: Clearing index file " << idx << endl;
      m_indeces[ idx ]->clear();
   }
   // Activate the writing.
   m_indexFiles[ idx ]->setPos(0);
   m_indexFiles[ idx ]->write( m_writeBuffer->getBufferAddress(),
                                            m_writeBuffer->getCurrentOffset(),
                                            this );
}

void
MultiFileDBufRequesterIndexHandler::findComplete(int startOffet, int size)
{
   SET_STATE ( IDLE );
   m_listener->findComplete(m_currentDescToFind,
                            m_currentIndexFile,
                            startOffet, size);
}

bool
MultiFileDBufRequesterIndexHandler::updateFindIndex()
{
   m_nbrFilesLeftToSearch--;
   if ( m_nbrFilesLeftToSearch == 0 ) {
      return false;
   } else {
      m_currentIndexFile--;
      if ( m_currentIndexFile < 0 ) {
         m_currentIndexFile = m_nbrFiles - 1;
      }
      return true;
   }
}

void
MultiFileDBufRequesterIndexHandler::moveToNextIdxOrStop()
{
   if ( updateFindIndex() ) {
      startFinding();
   } else {
      if ( m_mode == MODE_FINDING ) {
         findComplete(-1, -1);
      } else {
         MC2_ASSERT( m_mode == MODE_REMOVING );
         removeComplete();
      }
   }
}

void
MultiFileDBufRequesterIndexHandler::addCurrentMapToCurrentIndex()
{
   mc2dbg2 << "[MFDBRIH]: Adding map "
          << m_currentDescToFind << " to index "
          << m_currentIndexFile << endl;
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
   
   (*m_indeces[idx])[ m_currentDescToFind ] =
      m_currentWritePosAndSize;
}

int
MultiFileDBufRequesterIndexHandler::removeCurrentMapFromCurrentIndex()
{
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
   
   int sizeBefore = m_indeces[ idx ]->size();
   m_indeces[ idx ]->erase( m_currentDescToFind );
   int sizeAfter = m_indeces[ idx ]->size();
   if ( sizeAfter != sizeBefore ) {
      mc2dbg2 << "[MFDBRIH]: Removed map " << m_currentDescToFind 
              << " from index " << m_currentIndexFile << endl;
   } else {
      mc2dbg2 << "[MFDBRIH]: Desc " << m_currentDescToFind
              << " not found in index " << m_currentIndexFile << endl;
   }
   return sizeBefore != sizeAfter;
}

void
MultiFileDBufRequesterIndexHandler::indexWritten()
{
   if ( m_mode == MODE_WRITING ) {
      SET_STATE( IDLE );
      m_listener->indexWritten(m_currentDescToFind);
   } else {
      moveToNextIdxOrStop();
   }
}

void
MultiFileDBufRequesterIndexHandler::removeFromIndex()
{
   if ( removeCurrentMapFromCurrentIndex() ) {
      writeIndex();
   } else {
      moveToNextIdxOrStop();
   }

}

void
MultiFileDBufRequesterIndexHandler::readDone(int nbrRead)
{
   if ( m_shuttingDown ) {
      return;
   }
   switch ( m_state ) {
      case FINDING_MAP_READING_INDEX_LENGTH:
         if ( nbrRead != (int)m_readBuffer->getBufferSize() ) {
            /// Read error
            mc2dbg2 << "[MFDBRIH]: Read returned " << nbrRead
                    << " for file number " << m_currentIndexFile << endl;
            moveToNextIdxOrStop();
         } else {
            startReadingIndex();
         }
         break;

      case FINDING_MAP_READING_INDEX:     
         if ( nbrRead != (int)m_readBuffer->getBufferSize() ) {
            // Read error
            mc2dbg << "[MFDBRIH]: Read returned " << nbrRead
                   << " for file number " << m_currentIndexFile << endl;
            moveToNextIdxOrStop();
         } else {
            offsetAndSize_t offsetAndSize = findMapInCurrentIndexBuffer();
            bool found = offsetAndSize.first != -1;
            if ( found ) {
               mc2dbg2 << "[MFDBRIH]: Found " << m_currentDescToFind
                       << " in index " << endl;
               findComplete(offsetAndSize.first, offsetAndSize.second);
            } else {
               // Try the next one or give up.
               moveToNextIdxOrStop();
            }
         }
         break;

      case WRITING_MAP_READING_INDEX_LENGTH:
      case REMOVING_MAP_READING_INDEX_LENGTH:
         if ( nbrRead != (int)m_readBuffer->getBufferSize() ) {
            // Probably empty.
            if ( nbrRead != 0 ) {
               mc2log << warn << "[MFDBRIH]: Could not read index "
                      << endl;
            }
            if ( m_mode == MODE_REMOVING ) {
               removeFromIndex();
            } else {
               addCurrentMapToCurrentIndex();
               writeIndex();
            }
         } else {
            // Index found and ok
            startReadingIndex();
         }
         break;

      case WRITING_MAP_READING_INDEX:
      case REMOVING_MAP_READING_INDEX:
         if ( nbrRead != (int)m_readBuffer->getBufferSize() ) {
            // Something went wrong
            indexWritten();
         } else {
            // Add the map to the index.
            createIndex();
            if ( m_mode == MODE_REMOVING ) {
               removeFromIndex();
            } else {
               addCurrentMapToCurrentIndex();
               writeIndex();
            }
         }
         break;

      case IDLE:
      case WRITING_INDEX:
         mc2log << error << "[MFDBRIH] [Should not be idle now]" << endl;
         break;
   }
}

void
MultiFileDBufRequesterIndexHandler::writeDone(int nbrWritten)
{
   if ( m_shuttingDown ) {
      return;
   }
   MC2_ASSERT( m_state == WRITING_INDEX );
   delete m_writeBuffer;
   m_writeBuffer = NULL;
   mc2dbg2 << "[MFDBRIH]: nbrWritten = " << nbrWritten << endl;
   if ( m_mode == MODE_WRITING ) {
      indexWritten();
   } else {
      // Means that we are removing from all indeces
      moveToNextIdxOrStop();
   }
}
