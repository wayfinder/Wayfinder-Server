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

#include "SearchMap2.h"
#include "SearchNotice.h"

/**
 *   Comparator to be used when sorting the table
 *   of indeces. Compares the entire strings.
 */
class MultiStringSortComp {
public:
   MultiStringSortComp(const SearchMap2* searchMap) {
      m_searchMap = searchMap;
   }

   bool operator()(const pair<uint32,uint32>& a, const char* b) const {
      return strcmp(m_searchMap->getName(a.first), b) < 0;
   }

   bool operator()(const pair<uint32,uint32>& a,
                   const pair<uint32,uint32>& b) const {
      return strcmp(m_searchMap->getName(a.first),
                    m_searchMap->getName(b.first)) < 0;
   }

   bool operator()(const MultiSearchNotice& a, const char* b) const {
      return strcmp(m_searchMap->getName(a.getStringIndex()), b) < 0;
   }

   bool operator()(const char* b, const MultiSearchNotice& a) const {
      return strcmp(b, m_searchMap->getName(a.getStringIndex())) < 0;
   }
   
   
private:
   /// Search map to use for string-lookups.
   const SearchMap2* m_searchMap;
   
};

/**
 *   Comparator for full matches.
 */
typedef MultiStringSortComp MultiStringSearchFullComp; 

/**
 *   Comparator to be used when searching in the table
 *   of indeces. Only examines the common part of the
 *   strings.
 */
class MultiStringSearchComp {
public:
   /**
    *   Creates a new SearchComp.
    *   @param searchMap    The map is needed to lookup the strings.
    *   @param searchString This is the string that we will be searching
    *                       for. The length is stored inside so that we
    *                       know how many characters to compare.
    */
   MultiStringSearchComp(const SearchMap2* searchMap,
                         const char* searchString) {
      m_searchMap = searchMap;
      m_stringLen = strlen(searchString);
   }
   
   bool operator()(const MultiSearchNotice& a, const char* b) const {
      return strncmp(m_searchMap->getName(a.getStringIndex()),
                     b, m_stringLen) < 0;
   }

   bool operator()(const char* b, const MultiSearchNotice& a) const {
      return strncmp(b, m_searchMap->getName(a.getStringIndex()),
                     m_stringLen) < 0;
   }
   
private:
   /// The map for string lookups.
   const SearchMap2* m_searchMap;
   /// The length to compare.
   int m_stringLen;
};
