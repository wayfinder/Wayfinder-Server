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

#include <vector>

#include "StringSearchUtility.h"

class Trie;


class TrieLeafNode {
   
  public:
   TrieLeafNode( char*);
   
  protected:
   TrieLeafNode(){
   };
   bool  m_leaf;
   char* m_word;
   friend class Trie;
};

class TrieNonLeafNode : public TrieLeafNode {
   
  public:
   TrieNonLeafNode(char);
   
  private:
   bool m_endOfWord;
   TrieNonLeafNode** m_ptrs;
   friend class Trie;
};


class Trie {
   
  public:
   struct results {
      const char* name;
      int distance;
   };
   
   struct searchOrder:
      public binary_function<const results&, const results&, bool> {
      bool operator()(const results& x, const results& y) {
         return (x.distance < y.distance);
      }
   };
   
   Trie() : m_notFound(-1) {
   }
   Trie(char*);


   /**
    *  Searches for a specific word in the trie.
    *  If the word is found, the searching stops.
    *  This method is used only if no spelling errors are allowed.
    *
    *  @param inputWord     The word to search for
    * 
    *  @return              True if the inputword was in the trie,
    *                       false otherwise
    */
   bool exactWordFound(char *inputWord);


   /**
    *  Searches for all the words close enough to the input word 
    *
    *  @param inputWord     The word to search for
    *  @param root          The root of the trie 
    *  @param maxDistance   Maximum number of errors allowed for input word,
    *                       e.g. one forgotten letter, one extra letter
    *                       or one wrong letter means that the distance is 1
    *
    *  @param return        A vecor containing the names of all matches and
    *                       the corresponding distance. The vector is sorted
    *                       according to increasing distance.
    */
   vector<Trie::results> getSearchResults(const char* inputWord,
                                          TrieNonLeafNode* root,
                                          const uint16 maxDistance);
   
   
   
  /**
   *   Inserts a word in the trie
   *
   *   @param newWord        The word to be inserted
   */ 
   void insert(char* newWord);
   
  /**
   *   Gets the root of the trie, needed when you search in it
   */ 
   TrieNonLeafNode* getRoot(){
      return m_root;
   }
   
   
  private:
   TrieNonLeafNode* m_root;
   char m_prefix[80];
   const int m_notFound;
   
  /**
   *    Tries to find the position of a letter in a word
   *  
   *    @param p     pointer to a word in a non-leaf node
   *    @param ch    letter to find in the word p points at
   *    @return      The position of <b>ch</b> in the word <b>p</b> points at
   *                 If no position is found, -1 is returned
   */ 
   int  position(TrieNonLeafNode*p, char ch);


  public:
  /**
   *    Calculates the distance between two strings, if the distance
   *    is within a certain limit.
   *    Examples: $(a,b)$ = (Baravägen, Braavägen)  => distance = 2
   *                         
   *              $(a,b)$ = (Baravägen, Bravgen)    => distance = 2
   *                                                 
   *              $(a,b)$ = (Baravägen, Baravägenn) => distance = 1
   *                                                
   *    @param a,b       Words to calculate the distance between
   *    @param maxDist   Maximum allowed distance
   *
   *    @param return    The distance if it is less than or equal to maxDist,
   *                     -1 otherwise
   */
   static inline int getLevDist(const char* a, const char* b, uint16 maxDist);

   /**
    *   Same as the other getLevDist, but with precalculated lengths.
    *   @param alen The length of string a.
    *   @param blen The length of string b.
    */
   static inline int getLevDist(const char* a, int alen,
                                const char* b, int blen,
                                uint16 maxDist);
   
   void addCell(char ch, TrieNonLeafNode *p, int stop);
   
   /**
    *   Creates a leaf containing a given suffix at the position of
    *   a given character.
    *
    *   @param ch       Character from which to create a leaf from
    *   @param suffix   Word to be contained in the new leaf.
    *   @param p        Pointer to the word where ch is searched for
    */
   void createLeaf(char ch, char *suffix, TrieNonLeafNode *p);


   /**
    *  Searches for all the words close enough to the input word.
    *  Called in method getSearchresults(const char* ,
    *                                    TrieNonLeafNode*,
    *                                    const uint16) 
    *
    *  @param resultVec     In - and outparameter. A vecor containing the names
    *                       of all matches and the corresponding distance.
    *                       The vector is sorted according to increasing distance.
    *
    
    *  @param p             Pointer to an non-leaf node, initially the trie root
    *  @param prefix        The beginning of a word in the trie
    *  @param testWord      The word to search for
    *  @param maxDistance   Maximum number of errors allowed for input word
    *
    *  @param return        A vector containing the names of all matches and
    *                       the corresponding distance. The vector is sorted
    *                       according to increasing distance.
    */
   void searchTargetWords(vector<results>& resultVec,
                          int depth,
                          TrieNonLeafNode* p,
                          char* prefix,
                          const char* testWord,
                          const uint16 maxDistance);
   
};

inline int
Trie::getLevDist(const char* a, const char* b, uint16 maxDist)
{
   return StringSearchUtility::getLevDist(a, b, maxDist);
}

inline int
Trie::getLevDist(const char* a, int alen,
                 const char* b, int blen,
                 uint16 maxDist)
{
   return StringSearchUtility::getLevDist(a, alen, b, blen, maxDist);
}
