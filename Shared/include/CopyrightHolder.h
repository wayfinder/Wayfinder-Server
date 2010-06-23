/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COPYRIGHT_HOLDER 
#define COPYRIGHT_HOLDER

#include "config.h"

#include "BitBuffer.h"
#include "MC2SimpleString.h"
#include "MC2BoundingBox.h"

#include <map>
#include <vector>

/**
 *   Copyright notice. 
 *   Describes one box containing copyright information.
 */
class CopyrightNotice {
public:
   typedef int CopyrightID;

   static const CopyrightID OVERLAP_ID = MAX_INT32;

   friend class CopyrightHolder;
   friend class CopyrightHandler;
   
   /// Load from buffer.
   bool load( BitBuffer& buf ); 
   
   /// Save to buffer.
   bool save( BitBuffer& buf ) const;
   
   /// Get the copyright id. OVERLAP_ID if overlap box.
   CopyrightID getCopyrightID() const;

   /**
    *  If the box is an overlap box, i.e. only a boundingbox
    *  for its child boxes. Contains no logical copyright information.
    */
   bool isOverlappingBox() const; 
   
   /// Get the box for this notice.
   const MC2BoundingBox& getBox() const;

   /// Dump data to stdout.
   void dump() const;
private:

   /// The box.
   MC2BoundingBox m_box;

   /// The copyright id. OVERLAP_ID if overlap box. 
   /// Also index into string table.
   CopyrightID m_copyrightID;

};

/**
 *   Copyright holder. 
 *   Contains all copyright information for the world.
 */
class CopyrightHolder {
public:

   friend class CopyrightHandler;
   
   /// Load from buffer.
   /// @return True if load was successful.
   bool load( BitBuffer& buf ); 
   
   /// Save to buffer.
   /// @param buf The buffer to save to.
   /// @param version 0-1 = do not include supplier ids, version > 2
   ///                      include supplier ids.
   /// @return True if save was successfull.
   bool save( BitBuffer& buf, uint32 version ) const;

   /// @return size in bytes in buffer.
   uint32 getSizeInBuffer() const;

   /// Pair type.
   typedef std::pair<int,int> pair_t;

   /// Dump info to stdout.
   void dump() const;

private:

   /// Vector of pairs, kind of like a std:map
   typedef std::vector<pair_t> map_t;
   
   /// Iterator to map.
   typedef map_t::const_iterator mapIt_t;

   /// Range of iterators.
   typedef std::pair<mapIt_t,mapIt_t> range_t;

   typedef uint32 SupplierID;
   typedef CopyrightNotice::CopyrightID StringIndex;

   /// Map string index to supplier ID
   typedef std::map<StringIndex, SupplierID> StringIndexToSupplier;

   /**
    *   Table describing the parent / child relationship for the 
    *   CopyrightNotices. pair_t::first is the parent id, and
    *   pair_t::second refers to the child id. The id:s are
    *   the position in the m_boxes vector.
    *
    *   Parent id MAX_INT32 means that there is no parent,
    *   i.e. the box is at the top / root level.
    */
   map_t m_boxesByParent;
   
   /**
    *   The copyright boxes. The position in the vector refers to
    *   their respective box id in m_boxesByParent.
    */
   std::vector<CopyrightNotice> m_boxes;

   /**
    *   The copyright strings. The copyright id in the CopyrightNotice
    *   refers to the position in this vector for the copyright string.
    */
   std::vector<MC2SimpleString> m_copyrightStrings;

   /// The copyright head.
   MC2SimpleString m_copyrightHead;
   
   /// The minimum coverage in percent for a copyright ID to be included.
   int m_minCovPercent;

   /// map for string index to supplier identifier
   StringIndexToSupplier m_strIdxToSupplier;

};

// -- Inlined functions

#endif // COPYRIGHT_HOLDER

