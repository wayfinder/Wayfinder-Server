/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEMAP_QUERTY_H
#define TILEMAP_QUERTY_H

#include "config.h"

#include <vector>
#include <set>
#include <utility>

class MC2SimpleString;
class SharedBuffer;
class TileMapBufferHolder;

/**
 *   This is the view of a TileMapQuery from a ParserThread
 *   or similar.
 *   @see TileMapBufferQuery for a query to use when a buffer is wanted.
 */
class TileMapQuery {
public:
   /// Type of vector of buffers.
   typedef vector<pair<MC2SimpleString, SharedBuffer*> > bufVect_t;
   /// Type of vector of wanted buffers
   typedef vector<MC2SimpleString> paramVect_t;
   /// Set of wanted buffers
   typedef set<MC2SimpleString> paramSet_t;   
   /// Type of the vector of wanted buffers.
   typedef vector<const char*> internalParamVect_t;

   /// Constructor
   TileMapQuery( const paramVect_t& wanted);

   /// Other constructor
   TileMapQuery( const paramSet_t& wanted);

   /// Third constctructor. Does not copy the data.
   TileMapQuery( const SharedBuffer& dataBuf );
   
   /// Virtual destructor
   virtual ~TileMapQuery();

   /// Returns all originally wanted maps
   const internalParamVect_t& getWanted() const { return m_wanted; }

   /**
    *   Returns the number of originally wanted maps.
    */
   int getNbrWanted() const;

   /**
    *   Returns the number of maps added.
    */
   int getNbrAdded() const;

   /**
    *   Returns the number of failed maps.
    */
   int getNbrFailed() const;

   /**
    *   Puts back the failed params and makes
    *   the query ready to process them.
    */
   bool reputFailed();
   
   /**
    *   Adds the next parameters to be requested to the end of the
    *   vector.
    *   To be called until it returns zero.
    */
   virtual int getNextParams( paramVect_t& params, int maxNbr );
   
   /**
    *   Convenience function if only one buffer should be added.
    *   @param param The map param for the buffer.
    *   @param buf   The buffer. Must be copied if it should be kept.
    *   @return Some number.
    */     
   int addOneBuffer( const MC2SimpleString& param,
                     SharedBuffer* buf );
   
   /**
    *   Add a vector of buffers to the result.
    *   @param bufs Vector of buffers to try to add.
    *   @return Number of buffers added.
    */
   virtual int addBuffers( const bufVect_t& bufs );

   /**
    *   Add a vector of buffers to the result.
    *   @param bufs Vector of buffers to try to add.
    *   @return Number of buffers added.
    */
   int addBuffers( const vector<TileMapBufferHolder>& bufs );

   /**
    *   Returns the time limit for the query.
    */
   uint32 getTimeLimit() const { return m_timeLimit; }
   
protected:

   /**
    *   Returns the number of params done so far.
    *   Default implementation returns m_wanted.size() - m_stillWanted.size()
    */
   virtual uint32 getNbrDone() const;

   /**
    *   Returns the number of still wanted tiles.
    */
   virtual uint32 getNbrStillWanted() const;
   
   /**
    *   Adds the buffers to the internal storage.
    */
   virtual int internalAddBuffers( const bufVect_t& bufs ) = 0;

   /**
    *    Returns true if no more maps are wanted.
    */
   virtual bool isDone() const { return m_stillWanted.empty(); }
   
   /// Returns all originally wanted maps
   internalParamVect_t& getWanted() { return m_wanted; }

   void printStatus();

   /// Time limit in milliseconds
   uint32 m_timeLimit;

   /// Last time we printed something. For debug mainly.
   uint32 m_lastPrintTime;
   
   /// Print lots of status.
   bool m_statusPrints;

private:
   
   /// Sorts the params in fetching order.
   void sortParams();
   
   /// Update still wanted from m_wanted.
   void updateStillWanted();
   
   /// Creates parameters
   void realGetNextParams( vector<MC2SimpleString>& params, int maxNbr );
   
   /// Vector of wanted buffers.
   internalParamVect_t m_wanted;

   struct internalSetComp {
      inline bool operator()(const char* a,
                             const char* b ) const {
         return strcmp( a, b ) < 0;
      }
   };
   
   /// Type of internal set which points into the vector
   typedef set<const char*,internalSetComp> internalParamSet_t;
   /// Set of buffers that are still wanted.
   internalParamSet_t m_stillWanted;
   /// Iterator to the next wanted param
   internalParamVect_t::const_iterator m_nextWantedIt;
   /// Number of maps actually added
   int m_nbrAdded;

   /// Time when it starts
   uint32 m_startTime;

protected:
   /// Buffer containing the param strings
   const SharedBuffer* m_paramBuf;
private:
   /// Buffer to be deleted
   SharedBuffer* m_paramBufToDelete;
   /// Number of buffers that were NULL
   uint32 m_nbrFailed;
   
};

#endif
