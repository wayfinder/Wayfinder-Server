/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSSIGNPOST_H
#define GMSSIGNPOST_H

#include "config.h"
#include "GMSSignPostSet.h"

class DataBuffer;
/**
 *   Class representing a single sing post, to be stored by connections.
 *
 */
class GMSSignPost {
   
   friend class SignPostTable;

 public:

   /**
    * Methods for saving and loading from and to DataBuffer.
    */
   //@{
   void save(DataBuffer& dataBuffer) const;
   void load(DataBuffer& dataBuffer);

   /**
    * Returns the maximum number of bytes this object will occupy when saved 
    * in  a data buffer.
    */
   uint32 sizeInDataBuffer() const;
   
   //@}

   /**
    * @return Return the sign post set with index. The index is zero based.
    */
   GMSSignPostSet& getOrAddSignPostSet(uint32 index);


   /**
    * @return Returns true if this sign post is equal to other.
    */ 
   bool operator==( const GMSSignPost& other ) const;

   /**
    * @return Returns false if this sign post is equal to other.
    */ 
   bool operator!=( const GMSSignPost& other ) const;

   /**
    * For debug printing to the log.
    */
   friend ostream& operator<< ( ostream& stream, 
                                const GMSSignPost& spElm );
   ostream& debugPrint( ostream& stream, 
                        const OldGenericMap* theMap );

   
   /**
    * Removes any contained sing post element with text matching signPostText.
    * @retrun Returns true if any sign post element is found.
    */
   bool removeSpElement(const OldGenericMap& theMap,
                        const MC2String& signPostText);

   /**
    * @return Returns true if this sing post does not include any sign post
    *         element.
    */
   bool isEmpty();

   typedef vector<GMSSignPostSet> GMSSignPostCont;
   typedef vector<GMSSignPostSet>::iterator iterator;
   typedef vector<GMSSignPostSet>::const_iterator const_iterator;

   const_iterator begin() const;
   const_iterator end() const;

   /**
    * @return Returns all sign post sets of this sing post.
    */
   const GMSSignPostCont& getSignPostSets() const;

   void convertSignPost( const OldGenericMap& map, GMSSignPostSet& inData );

 private:
   // Member methods //////////////////////////////////////////////////

   /**
    * @return Returns all sign post sets of this sing post in editable mode.
    */
   GMSSignPostCont& getNonConstSignPostSets();

   
   // Member variables ////////////////////////////////////////////////

   /** Container for all sing post sets. The index of the vector determines
    *  the order of the sign post sets on the sing post.
    */
   GMSSignPostCont m_signPostSets;


}; // GMSSignPost

#endif // GMSSIGNPOST_H


