/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SignPostTable.h"

#include "DataBuffer.h"
#include "DeleteHelpers.h"
#include "OldGenericMap.h"

SignPostTable::~SignPostTable() {
   for ( signPostTable_t::iterator it = m_signPostTable.begin();
         it != m_signPostTable.end(); ++it) {
      // sign post vector.
      vector<GMSSignPost*>& signPosts = it->second;
      STLUtility::deleteValues( signPosts );
   }
}

void 
SignPostTable::save(DataBuffer& dataBuffer) const
{
   // Size
   dataBuffer.writeNextLong(m_signPostTable.size());

   // Table content
   for ( signPostTable_t::const_iterator it = m_signPostTable.begin();
         it != m_signPostTable.end(); ++it) {
      dataBuffer.writeNextLong(it->first.first); // from node
      dataBuffer.writeNextLong(it->first.second); // to node
      // sign post vector.
      const vector<GMSSignPost*>& signPosts = it->second;
      dataBuffer.writeNextLong(signPosts.size()); // size
      for (uint32 i=0; i<signPosts.size(); i++){
         signPosts[i]->save(dataBuffer);  // the sign posts.
      }
      }

} // save


void 
SignPostTable::load(DataBuffer& dataBuffer)
{
   // Size
   uint32 tableSize = dataBuffer.readNextLong();

   // Table content
   uint32 fromNodeID;
   uint32 toNodeID;
   uint32 signPostsSize;
   for ( uint32 i=0; i<tableSize; i++){
      // The connection
      fromNodeID = dataBuffer.readNextLong();
      toNodeID = dataBuffer.readNextLong();

      // The sign posts
      signPostsSize = dataBuffer.readNextLong();
      vector<GMSSignPost*> signPosts;
      for (uint32 i=0; i<signPostsSize; i++){
         
         // An individual signpost.
         GMSSignPost* signPost = new GMSSignPost();
         signPost->load(dataBuffer);
         signPosts.push_back(signPost);
      }
      m_signPostTable.insert(make_pair( make_pair(fromNodeID, toNodeID),
                                        signPosts ));
   }
} // load

uint32 
SignPostTable::sizeInDataBuffer() const
{
   uint32 size = 4; // size of table.
   for ( signPostTable_t::const_iterator it = m_signPostTable.begin();
         it != m_signPostTable.end(); ++it) {
      size += 4; // from node ID
      size += 4; // to node ID
      size += 4; // number of sign posts.
      // The sign posts
      const vector<GMSSignPost*>& signPosts = it->second;
      for (uint32 i=0; i<signPosts.size(); i++){
         size += signPosts[i]->sizeInDataBuffer();
      }
   }
   return size;

} // sizeInDataBuffer

uint32 
SignPostTable::size() const
{
   return m_signPostTable.size();

} // sizeInDataBuffer


bool
SignPostTable::addSignPostElm( uint32 fromNodeID, uint32 toNodeID,
                               uint32 signPostIdx,
                               uint32 signPostSetIdx, uint32 signPostElmIdx,
                               const GMSSignPostElm& signPostElm )
{
   mc2dbg8 << "addSignPostElm (" << fromNodeID << ", " << toNodeID 
          << "): " << signPostIdx << "," << signPostSetIdx << "," 
          << signPostElmIdx << ": " 
          << signPostElm << endl;
   vector<GMSSignPost*>& signPosts = 
      m_signPostTable[make_pair(fromNodeID, toNodeID)];
   if ( signPostIdx >= signPosts.size() ){
      // Resize and allocate all sing posts of the vector.
      uint32 oldSize = signPosts.size();
      signPosts.resize(signPostIdx + 1);
      for (uint32 i=oldSize; i<signPosts.size(); i++){
         signPosts[i] = new GMSSignPost();
      }
   }
   MC2_ASSERT(signPosts.size() > signPostIdx);
   GMSSignPostSet& spSet = 
      signPosts[signPostIdx]->getOrAddSignPostSet(signPostSetIdx);
   GMSSignPostElm& spElm = spSet.getOrAddSignPostElm(signPostElmIdx);
   if ( spElm.isSet() ){
      mc2log << error << "Trying to set the same sign post element once more"
             << endl;
      mc2dbg << "Sign post [" << signPostIdx << "]" <<endl;
      mc2dbg << (*signPosts[signPostIdx]) << endl;

      mc2dbg << "Element to set:" << endl;
      mc2dbg << "[" <<signPostIdx << "," << signPostSetIdx << "," 
             << signPostElmIdx << "]";
      mc2dbg << signPostElm << endl;
         
      //MC2_ASSERT(false);
      return false;
   }
   spElm = signPostElm;
   return true;
} // addSignPostElm

bool 
SignPostTable::addSingleElmSignPost( uint32 fromNodeID, uint32 toNodeID, 
                                     const GMSSignPostElm& signPostElm )
{
   // Check if this sign post exists, and add it if not.
   if ( ! signPostExists( fromNodeID, toNodeID, signPostElm ) ){
      
      uint32 nextSignPostIdx = getNbrSignPosts(fromNodeID, toNodeID);
      addSignPostElm( fromNodeID,
                      toNodeID,
                      nextSignPostIdx,
                      0, // sign post set index
                      0, // sign post element index.
                      signPostElm);
      return true;
   }
   else {
      return false;
   }
}



uint32 
SignPostTable::getNbrSignPosts( uint32 fromNodeID, uint32 toNodeID ) const
{
   uint32 result = 0;
   signPostTable_t::const_iterator it = 
      m_signPostTable.find(make_pair(fromNodeID, toNodeID));
   if ( it != m_signPostTable.end() ){
      result = it->second.size();
   }
   return result;
} // getNbrSignPosts

vector<GMSSignPost*> emptySignPostVector;

const vector<GMSSignPost*>& 
SignPostTable::getSignPosts( uint32 fromNodeID, 
                             uint32 toNodeID ) const
{
   signPostTable_t::const_iterator it = 
      m_signPostTable.find(make_pair(fromNodeID, toNodeID));
   if ( it != m_signPostTable.end() ){
      return it->second;
   }
   else {
      return emptySignPostVector;
   }
   
} // getSignPosts


bool
SignPostTable::signPostExists( uint32 fromNodeID, uint32 toNodeID, 
                               const GMSSignPostElm& signPostElm ) const
{
   // Create a sign post with only this sign post set.
   GMSSignPost tmpSignPost;
   GMSSignPostSet& tmpSpSet = 
      tmpSignPost.getOrAddSignPostSet(0);
   GMSSignPostElm& tmpSpElm = tmpSpSet.getOrAddSignPostElm(0);
   tmpSpElm = signPostElm;
   
   // Look for an equal sign post.
   signPostTable_t::const_iterator it = 
      m_signPostTable.find(make_pair(fromNodeID, toNodeID));
   if ( it != m_signPostTable.end() ){
      const vector<GMSSignPost*>& compareSignPostVec = it->second;
      for ( uint32 i=0; i<compareSignPostVec.size(); i++){
         if ( *compareSignPostVec[i] == tmpSignPost ){
            return true;
         }
      }
   }

   // No equal sign post found, return false.
   return false;

} // signPostExists


vector<uint32*> 
SignPostTable::getEditableStrCodePointers()
{  
   vector<uint32*> result;
   for ( signPostTable_t::iterator spIt = m_signPostTable.begin();
         spIt != m_signPostTable.end(); ++spIt ){
      vector<GMSSignPost*>& signPosts = spIt->second;
      for ( uint32 i=0; i<signPosts.size(); i++){
         GMSSignPost* signPost = signPosts[i];  
         vector<GMSSignPostSet>& signPostSets = 
            signPost->getNonConstSignPostSets();
         for ( uint32 j=0; j<signPostSets.size(); j++){
            vector<GMSSignPostElm>& spElements = 
               signPostSets[j].getNonConstSignPostElements();
            for (uint32 k=0; k< spElements.size(); k++){
               GMSSignPostElm& spElm = spElements[k];
               result.push_back(spElm.getTextStringCodePointer());
            }
         }
      }
   }
   return result;
} // getEditableStrIdxPointers


bool
SignPostTable::removeSignPosts ( uint32 fromNodeID, uint32 toNodeID ){
   
   signPostTable_t::iterator it = 
      m_signPostTable.find(make_pair(fromNodeID, toNodeID));
   if ( it != m_signPostTable.end() ){
      vector<GMSSignPost*>& signPosts = it->second;
      STLUtility::deleteValues( signPosts );
      signPosts.clear();
      m_signPostTable.erase(it);
      return true;
   }
   return false;
}


bool
SignPostTable::removeSignPostElm( const OldGenericMap& theMap,
                                  uint32 fromNodeID, uint32 toNodeID, 
                                  MC2String signPostText )
{
   bool result = false;
   
   // Look for an equal sign post.
   signPostTable_t::iterator it = 
      m_signPostTable.find(make_pair(fromNodeID, toNodeID));
   vector<GMSSignPost*> newSignPosts;
   if ( it != m_signPostTable.end() ){
      vector<GMSSignPost*>& compareSignPostVec = it->second;
      
     for ( vector<GMSSignPost*>::iterator spIt = compareSignPostVec.begin();
            spIt != compareSignPostVec.end(); ++spIt ){
        result = (*spIt)->removeSpElement(theMap, signPostText);
        if ( (*spIt)->isEmpty() ){
           delete (*spIt);
        }
        else {
           newSignPosts.push_back(*spIt);
        }
     }
     if (newSignPosts.size() > 0){
        it->second = newSignPosts;
     }
     else {
        m_signPostTable.erase(it);
     }
   }



   return result;

} // signPostExists



uint32
SignPostTable::copySignPostsFromOtherMap(OldGenericMap& theMap,
                                         uint32 fromNodeID, 
                                         uint32 toNodeID,
                                         const OldGenericMap& otherMap,
                                         uint32 otherFromNodeID, 
                                         uint32 otherToNodeID)
{
   const vector<GMSSignPost*> otherSignPosts = 
      otherMap.getSignPostTable().getSignPosts(otherFromNodeID,
                                               otherToNodeID);
   uint32 nbrCopied = 0;

   // For all sign posts of this connection.
   for (uint32 i=0; i<otherSignPosts.size(); i++){
      const GMSSignPost* otherSignPost = otherSignPosts[i];
      const vector<GMSSignPostSet>& otherSignPostSets = 
         otherSignPost->getSignPostSets();
      // For all sign post sets.
      for ( uint32 j=0; j<otherSignPostSets.size(); j++){
         const vector<GMSSignPostElm>& otherSignPostElements = 
            otherSignPostSets[j].getSignPostElements();
         // For all sign post elements.
         for (uint32 k=0; k< otherSignPostElements.size(); k++){
            const GMSSignPostElm& otherSignPostElm = otherSignPostElements[k];

            // Copy the other sign post element.
            GMSSignPostElm newSignPostElm = otherSignPostElm;
            
            // Get the text of the other sign post
            MC2String otherText = otherSignPostElm.getTextString(otherMap);
            if ( otherText != "" ){
               // Don't set empty text.
               LangTypes::language_t lang = 
                  otherSignPostElm.getTextLang(otherMap);
               ItemTypes::name_t nameType = 
               otherSignPostElm.getTextNameType(otherMap);
               
               // Add the name to the map.
               uint32 newStringCode = theMap.addName(otherText.c_str(),
                                                     lang, nameType);
               newSignPostElm.setTextStringCode(newStringCode);
            }

            // Add the copied sign post element to the map.
            addSignPostElm( fromNodeID, toNodeID,
                            i, // signPostIdx,
                            j, // signPostSetIdx
                            k, // signPostElmIdx,
                            newSignPostElm );
         }
      }
      nbrCopied++;
   }
   return nbrCopied;
} // copySignPostsFromOtherMap

void
SignPostTable::signPostShapeUp()
{
   // Remove duplicated signposts.
   for ( signPostTable_t::iterator connIt = m_signPostTable.begin(); 
         connIt != m_signPostTable.end(); ++connIt ){
      vector<GMSSignPost*>& signPosts = connIt->second;
      
      
      bool doubleFound = false;
      for (int32 i=signPosts.size()-1; i>=0; i--){
         bool isDouble = false;
         for (int32 j=i-1; j>=0; j--){
            if ( *(signPosts[i]) == *(signPosts[j]) ){
               isDouble = true;
            }
         }
         if ( isDouble ){
            mc2dbg << "Removing double sign post from" 
                   << connIt->first.first << "," 
                   << connIt->first.second << endl;
            mc2dbg << *signPosts[i];
            delete signPosts[i];
            signPosts[i] = NULL;
            doubleFound = true;
         }
      }
      // Compact the vector.
      if (doubleFound){
         vector<GMSSignPost*> tmpSignPosts;
         for (uint32 i=0; i<signPosts.size(); i++){
            if ( signPosts[i] != NULL ){
               tmpSignPosts.push_back(signPosts[i]);
            }
         }
         connIt->second = tmpSignPosts;
      }
      
   }

   // Todo: Check sign posts vectors sanity here.


} // signPostShapeUp

