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
#include <algorithm>
#include <iostream>
#include "MC2String.h"
#include <stdlib.h>

#include "TrieStruct.h"


TrieLeafNode::TrieLeafNode(char *suffix) {
    m_leaf = true;
    m_word = new char[strlen(suffix)+1];
    
    if (m_word == NULL) {
        cerr << "Out of memory2.\n";
        exit(-1);
    }
    strcpy(m_word,suffix);
}

TrieNonLeafNode::TrieNonLeafNode(char ch) {
    m_ptrs = new TrieNonLeafNode*;
    m_word = new char[2];
    if (m_ptrs == NULL || m_word == NULL) {
        cout<< "Out of memory3.\n";
        exit(1);
    }
    m_word[0] = ch;
    m_word[1] = '\0';
    
    m_leaf = false;
    m_endOfWord = false;
    m_ptrs[0] = NULL;
}

Trie::Trie(char* word) : m_notFound(-1) {
    m_root = new TrieNonLeafNode(word[0]); // initialize the root
    createLeaf(word[0],word+1,m_root); // to avoid later tests;
}


bool Trie::exactWordFound(char *word) {
   TrieNonLeafNode *p = m_root;   
   TrieLeafNode *lf;
   int pos;
   
   while (true)
      if (p->m_leaf) {                        
         // node p is a leaf, compare the end of <b>word</b>
         // with the word in the leaf
         lf = static_cast<TrieLeafNode*>(p);  
         if (strcmp(word,lf->m_word) == 0)    
            return true;                     
         else 
            return false;
      }
      else if (*word == '\0') {
          
         if (p->m_endOfWord)
            return true;
         else
            return false;
      }
      // continue path, if possible
      else if ((pos = position(p,*word)) != m_notFound &&
               p->m_ptrs[pos] != NULL) {
         p = p->m_ptrs[pos];                   
         word++;
      }
      else
         return false;                  
      
   
}

vector<Trie::results> Trie::getSearchResults(const char* inputWord,
                                             TrieNonLeafNode* root,
                                             const uint16 maxDistance)
{
   vector<results> res;
   char prefix[80];
   *prefix = '\0';
   searchTargetWords(res,
                     0,
                     root,
                     prefix,
                     inputWord,
                     maxDistance);
   sort(res.begin(), res.end(), Trie::searchOrder());
   return (res);
}



   
void Trie::searchTargetWords(vector<results>& resultVec,
                             int depth,
                             TrieNonLeafNode *p,
                             char *prefix,
                             const char* testWord,
                             const uint16 maxDistance)
{
   if (maxDistance == 0) {
      mc2dbg<<"Trie::searchTargetWords: Exact searching for '"
            <<testWord<<"'"<<endl<<endl;
      char *temptestWord = new char[strlen(testWord)+1];
      strcpy(temptestWord,testWord);
      if (exactWordFound(temptestWord)){
         results newResult;
         newResult.name=testWord;
         newResult.distance=0;
         resultVec.push_back(newResult);
      }
      delete []temptestWord;
      return;
   }
   if (depth == 0){
      mc2dbg<<"Trie::searchTargetWords: Searching for '"<<testWord
            <<"' with at most "<<maxDistance<<" errors"<<endl;
   }
   int i;
   // assumption: the root is not a leaf
   // and it is not null
   if (p->m_leaf) {            
      TrieLeafNode *lf = static_cast<TrieLeafNode*>(p);
      uint16 x = strlen(prefix);
      uint16 y = strlen(lf->m_word);
      char* tempTarget = new char[x+y+1];
      const char* targetWord;
      int dist = 0;
      // Put the prefix and the leaf word after each other in a new string
      if (y>0){
         for (uint16 po = 0; po < x; po++)
            tempTarget[po] = prefix[po];
         for (uint16 po = 0; po < y; po++)
            tempTarget[x+po] = lf->m_word[po];
         tempTarget[x+y]='\0';
      }
      else
         strcpy(tempTarget,prefix);
      targetWord=tempTarget;
      
      dist=getLevDist(targetWord, testWord, maxDistance );
      if ((dist >= 0)&&(dist <= maxDistance)){
            results newResult;
            newResult.name=targetWord;
            newResult.distance=dist;
            resultVec.push_back(newResult);
      }
      else
         delete [] tempTarget;  
   }
   else {
      for (i = strlen(p->m_word)-1; i >= 0; i--)
      {
         if (p->m_ptrs[i] != 0) {
               // add the letter corresponding to position $i$ to prefix
               prefix[depth] = p->m_word[i];  
               prefix[depth+1] = '\0';
               // Check the distance, compare the prefix with the
               // first letters in the testword
               char* tempTarget2 = new char[MIN(strlen(prefix),strlen(testWord))+1];
               for (uint16 po = 0; po < MIN(strlen(prefix),strlen(testWord)); po++)
                  tempTarget2[po] = testWord[po];
               tempTarget2[strlen(prefix)]='\0';
               int dist = getLevDist(tempTarget2,prefix,maxDistance+1);
               if (dist >= 0)
                  searchTargetWords(resultVec,
                                    depth+1,
                                    p->m_ptrs[i],
                                    prefix,
                                    testWord,
                                    maxDistance);
               
               delete [] tempTarget2;
               
         }
         
      }
      if (p->m_endOfWord) {
         prefix[depth] = '\0';
         char* tempTarget2 = new char[strlen(prefix)+1];
         strcpy(tempTarget2,prefix);
         const char* constprefix = tempTarget2;
         // The prefix is here a stored string in the trie.
         int dist = getLevDist(prefix,testWord,maxDistance);
         if ((dist >= 0)&&(dist <= maxDistance)){
            results newResult;
            newResult.name=constprefix;
            newResult.distance=dist;
            resultVec.push_back(newResult);
         }
         else
            delete []tempTarget2;
      }
   }
}

int Trie::position(TrieNonLeafNode *p, char ch) {
    uint i;
    for (i = 0; i < strlen(p->m_word) && p->m_word[i] != ch; i++);
       if (i < strlen(p->m_word))
          return i;
       else return m_notFound;
}



void Trie::addCell(char ch, TrieNonLeafNode *p, int stop) {
    int i, len = strlen(p->m_word);
    char *s = p->m_word;
    TrieNonLeafNode **tmp = p->m_ptrs;
    p->m_word = new char[len+2];
    p->m_ptrs    = new TrieNonLeafNode*[len+1];
    if (p->m_word == NULL || p->m_ptrs ==  NULL) {
        mc2dbg<< "Out of memory1.\n";
        exit(1);

    }
    for (i = 0; i < len+1; i++)
        p->m_ptrs[i] = NULL;
    // if <b>ch</b> does not follow all letters in <b>p</b>,
    // copy from tmp letters 
    if (stop < len)                        
         for (i = len; i >= stop+1; i--) { 
             p->m_ptrs[i]    = tmp[i-1];
             p->m_word[i]  = s[i-1];
         }
    p->m_word[stop] = ch;
    
    for (i = stop-1; i >= 0; i--) {        
        p->m_ptrs[i]    = tmp[i];
        p->m_word[i] = s[i];
    }
    p->m_word[len+1] = '\0';
    delete [] s;
}

void Trie::createLeaf(char ch, char *suffix, TrieNonLeafNode *p) {
   int  pos;
   pos = position(p,ch);
    if (pos == m_notFound) {
        for (pos = 0; pos < int32(strlen(p->m_word)) &&
                      p->m_word[pos] < ch; pos++);
        addCell(ch,p,pos);
    }
    p->m_ptrs[pos] = static_cast<TrieNonLeafNode*>(new TrieLeafNode(suffix));
}

void Trie::insert(char *word) {
    TrieNonLeafNode *p = m_root;
    TrieLeafNode *lf;
    int offset, pos;
    char *hold = word;
    while (true) {
       // the end of word is reached
       if (*word == '\0') {            
             if (p->m_endOfWord)
                  mc2log<< "Duplicate entry1 " << hold << endl;
             else p->m_endOfWord = true;  
             return;
        }
       pos = position(p,*word);
       if (pos == m_notFound) {          
           createLeaf(*word,word+1,p);
           return;                    
       }                                
       else if (p->m_ptrs[pos] == NULL){
          createLeaf(*word,word+1,p);
          return;
       }
       else if (pos != m_notFound &&     
                p->m_ptrs[pos]->m_leaf) {
          // hold this leaf
          lf = static_cast<TrieLeafNode*>(p->m_ptrs[pos]);    
          if (strcmp(lf->m_word,word+1) == 0) {
             mc2log<< "Duplicate entry2 " << hold << endl;
             return;
          }
          offset = 0;
          // create as many non-leaves as the length of identical
          // prefix of word and the string in the leaf (for cell <b>'R'</b>,
          // leaf <b>'EP'</b>, and word <b>'REAR'</b>, two such nodes are created)
          do {
             pos = position(p,word[offset]);
             // word == <b>"ABC"</b>, leaf = <b>"ABCDEF"</b> => leaf = <b>"DEF"</b>
             if (int(strlen(word)) == offset+1) {
                p->m_ptrs[pos] = new TrieNonLeafNode(word[offset]);
                p->m_ptrs[pos]->m_endOfWord = true;
                createLeaf(lf->m_word[offset],
                           lf->m_word + offset+1,
                           p->m_ptrs[pos]);
                return;
             }
             // word == <b>"ABCDEF"</b>, leaf = <b>"ABC"</b> => leaf = <b>"DEF"</b>
             else if (int(strlen(lf->m_word)) == offset) {
                p->m_ptrs[pos] = new TrieNonLeafNode(word[offset+1]);
                p->m_ptrs[pos]->m_endOfWord = true;
                createLeaf(word[offset+1],word+offset+2,p->m_ptrs[pos]);
                return;
             }
             p->m_ptrs[pos] = new TrieNonLeafNode(word[offset+1]);
             p = p->m_ptrs[pos];
             offset++;
          }while (word[offset] == lf->m_word[offset-1]);
          offset--;
          // word = <b>"ABCDEF"</b>, leaf = <b>"ABCPQR"</b> =>                                  // leaf(<b>'D'</b>) = <b>"EF"</b>, leaf(<b>'P'</b>) = <b>"QR"</b>
          // check whether there is a suffix left:
          // word = <b>"ABCD"</b>, leaf = <b>"ABCPQR"</b> =>
          // leaf(<b>'D'</b>) = null, leaf(<b>'P'</b>) = <b>"QR"</b>
          char *s;
          if (int(strlen(word)) > offset+2)
             s = word+offset+2;
          else
             s = new char[1];
          createLeaf(word[offset+1],s,p);
          // check whether there is a suffix left:
          // word = <b>"ABCDEF"</b>, leaf = <b>"ABCP"</b> =>
          // leaf(<b>'D'</b>) = <b>"EF"</b>, leaf(<b>'P'</b>) = NULL
          if (int(strlen(lf->m_word)) > offset+1)
             s = lf->m_word+offset+1;
             else {
                s = new char[1];
                s[0]='\0';
             }
             createLeaf(lf->m_word[offset],s,p);
             delete [] lf->m_word;
             delete lf;
             return;
       } // if (pos != m_notFound && ..
       else {
          p = p->m_ptrs[pos];
          word++;
       }
    }
}



