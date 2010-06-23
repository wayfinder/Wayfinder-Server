/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MULTIFILEDBUFREQUESTER_H
#define MULTIFILEDBUFREQUESTER_H

#include "config.h"
#include "DBufRequester.h"
#include "MC2SimpleString.h"
#include "FileHandler.h"

#include <list>
#include <vector>
#include <set>

class MemoryDBufRequester;
class MultiFileDBufRequesterIndexHandler;
class BitBuffer;

class MultiFileDBufRequester : public DBufRequester,
                               public FileHandlerListener {
public:
   /**
    *   Creates a new MultiFileDBufRequester that saves files
    *   in the specified path and uses the specified number of
    *   files.
    */
   MultiFileDBufRequester( DBufRequester* parent,
                           const char* basepath,
                           uint32 maxSize,
                           int nbrFiles = 5,
                           int nbrHash = 23 );

   /// Deletes the requester
   virtual ~MultiFileDBufRequester();
   
   /// Does the initialization.
   bool init();

   // -- DBufRequester
   
   void request( const MC2SimpleString& desc, DBufRequestListener* client );
   
   BitBuffer* requestCached( const MC2SimpleString& desc);

   void release( const MC2SimpleString& desc, BitBuffer* buffer );

   void cancelAll();
   
   // -- FileHandlerListener
   void readDone( int nbrRead );
   void writeDone( int nbrWritten );
   
   /// Sets maximum storage size.
   void setMaxSize(uint32 size);
   /// Clears cache
   void clearCache();
   
   /// Called by the MultiFileDBufRequesterIndexHandler when done searching
   void findComplete( const MC2SimpleString& desc,
                      int fileNo,
                      int startOffset, int size );

   /// Called by the MultiFileDBufRequesterIndexHandler when index is written.
   void indexWritten( const MC2SimpleString& desc );

   /// Called by the MultiFileDBufRequesterIndexHandler when it has removed
   void removeComplete( const MC2SimpleString& desc );
   
protected:
   /// Creates a filehandler siutable for the current platform.
   virtual FileHandler* createFileHandler(const char* fileName) = 0;
   
   /**
    *   Returns the path separator for the current platform.
    */
   virtual const char* getPathSeparator() const = 0;

   
private:
   /// Enum of states
   enum state_t {
      /// The MultiFileDBufRequester does not have anything to do.
      IDLE,
      /// We are looking for a map.
      LOOKING_FOR_MAP,
      /// We are reading a map from a file
      READING_MAP,
      /// We are checking where the map is
      WRITING_LOOKING_FOR_MAP,
      /// We are adding the map to a data file
      WRITING_MAP_DATA,
      /// We are adding the map to an index file.
      WRITING_INDEX,
      /// We are removing a map from the index.
      REMOVING_MAP,
   } m_state;

   void startFinding();
   void startWorking();
   void startWriting();
   void startRemoving();
   void setState( state_t newState );

   /// Decreases index. Wraps if necessary
   int decIndex(int oldIndex) const;
   
   /// Increases index. Wraps if necessary
   int incIndex(int oldIndex) const;

   /// Clears the oldest not empty file. @return The amount of deleted mem
   uint32 clearOldestNotEmptyFile();

   /// Returns the amount of avaliable space on the drive of the first index.
   uint32 getAvailableSpace() const;

   /// Returns the total cache size
   int32 getTotalCacheSize() const;

   /// Calculates max size of the cache
   uint32 calcMaxAllowedCacheSize( uint32 wantedSize ) const;
   
   /// Write remainding maps to a dump file.
   void writeRemainders();

   /// Read remainding maps from dump file.
   void readRemainders(FileHandler* dumpFile);

   /// Clears a file and its index.
   void clearFileAndIndex( int fileNbr );

   /// Internal remove call from DBufRequester
   void internalRemove( const MC2SimpleString& desc );

   /// Returns true if the desc is currently being removed.
   int isBeingRemoved( const MC2SimpleString& desc );
   
   inline uint32 getFileSize( int index );

   /// Maximum size of the files
   uint32 m_maxSize;
   /// Max size for one file.
   uint32 m_maxSizePerFile;
   /// Number of files to use.
   int m_nbrFiles;
   /// Nbr hashes for each index file.
   int m_nbrHash;
   /// Base path
   MC2SimpleString m_basePath;
   /// Map of fileHandlers for the map files.
   vector<FileHandler*> m_dataFiles;
   /// Map of fileHandlers for the index files.
   vector<FileHandler*> m_indexFiles;
   /// Set of maps to be removed from the index.
   set<MC2SimpleString> m_buffersToRemove;
   
   /// Maps that are waiting to be written do disk.
   MemoryDBufRequester* m_writeQueue;

   /// Currently removing descr
   MC2SimpleString m_currentlyRemoving;
   
   /// Buffer about to be written
   pair<MC2SimpleString, BitBuffer*> m_currentlyWriting;
   int m_startWritePos;

   typedef pair<MC2SimpleString, DBufRequestListener*> readJob_t;

   /// Queue of jobs to read.
   list<readJob_t> m_readQueue;
   
   /// Pair of map description and listener that we currently are reading.
   readJob_t m_currentlyReading;

   /// Last written fil
   int m_lastWrittenFileNbr;
   
   /// The index handler
   MultiFileDBufRequesterIndexHandler* m_indexHandler;

   /// The current read buffer.
   BitBuffer* m_readBuffer;
   
   /// BitBuffer used when dumping (10k constantly allocated)
   BitBuffer* m_dumpBuffer;
   
   /// Counts the number of writes to disk.
   uint32 m_nbrWrites;

   /// The dump file.
   FileHandler* m_dumpFile;

   /// Size of cache to set next time the handles is idle
   uint32 m_newSize;
   
   /// True if we are shutting down
   bool m_shuttingDown;

   /// Count if we are in startWriting
   int m_inStartWriting;
   /// Count if we are in startFinding.
   int m_inStartFinding;
   /// Count if we are in startRemoving.
   int m_inStartRemoving;
   
};

// --- Inlines ---

inline uint32
MultiFileDBufRequester::getFileSize( int index ) 
{
   uint32 size = m_dataFiles[ index ]->getFileSize();
   for ( int i = 0; i < m_nbrHash; ++i ) {
      size += m_indexFiles[ index * m_nbrHash + i ]->getFileSize();
   }
   return size;
}   

#endif
