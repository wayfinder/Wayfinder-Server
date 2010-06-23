/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHMAP_SEARCHUNIT_H
#define SEARCHMAP_SEARCHUNIT_H

#include "config.h"

class SearchMap2;
class DataBuffer;

class MultiStringSearch;

/**
 *   This is the superclass to the class that handles all the
 *   searching.
 *   @see SearchableSearchUnit.
 */
class SearchUnit {
public:

   /**
    *   Creates new, empty SearchUnit. Must be initialized
    *   using load.
    */
   SearchUnit();

   /**
    *   Virtual destructor.
    */
   virtual ~SearchUnit();
   
   /**
    *   Returns the size of the object in a DataBuffer.
    */   
   int getSizeInDataBuffer() const;

   /**
    *   Loads the object from a DataBuffer.
    */
   int load(DataBuffer& dataBuffer);

   /**
    *   Saves the object into a DataBuffer.
    */
   int save(DataBuffer& dataBuffer) const;
   
protected:
   
   /**
    *   Creates a new MultiStringSearch. Used in load and abstract
    *   so that it is possible to override in SearchModule.
    *   The searchmap should be created before calling this function.
    *   (In load).
    */
   virtual void createMultiStringSearch();
   
   /// The searchmap
   SearchMap2* m_searchMap;

   /// The SearchStructs
   MultiStringSearch* m_ownedMultiStringSearch;
   
};



#endif
