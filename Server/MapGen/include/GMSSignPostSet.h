/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSSIGNPOSTSET_H
#define GMSSIGNPOSTSET_H

#include "config.h"
#include <vector>

#include "GMSSignPostElm.h"

class DataBuffer;

/**
 *   Class representing a sing post set in a sign post (GMSSingPost).
 *   A sign post set contains information on a sing post related to each other,
 *   by for instance describing the same destination.
 *
 */
class GMSSignPostSet {
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
    * @return Return the sign post element  index. The index is zero based.
    */
   GMSSignPostElm& getOrAddSignPostElm(uint32 index);

   /**
    * @return Returns true if this sign post set is equal to other.
    */ 
   bool operator==( const GMSSignPostSet& other ) const;

   /**
    * @return Returns false if this sign post set is equal to other.
    */ 
   bool operator!=( const GMSSignPostSet& other ) const;
   
   /**
    * For debug printing to the log.
    */
   friend ostream& operator<< ( ostream& stream, 
                                const GMSSignPostSet& spElm );
   ostream& debugPrint( ostream& stream, 
                        const OldGenericMap* theMap );

   /**
    * Removes any contained sing post element with text matching signPostText.
    * @retrun Returns true if any sign post element is found.
    */
   bool removeSpElement(const OldGenericMap& theMap,
                        const MC2String& signPostText);


   /**
    * @return Returns true if this sing post set does not include any sign post
    *         element.
    */
   bool isEmpty();

   typedef vector<GMSSignPostElm> GMSSignPostSetCont;
   typedef vector<GMSSignPostElm>::iterator iterator;
   typedef vector<GMSSignPostElm>::const_iterator const_iterator;

   const_iterator begin() const;
   const_iterator end() const;

   iterator begin();
   iterator end();

   const GMSSignPostSetCont& getSignPostElements() const;

 private:
   // Member methods

   GMSSignPostSetCont& getNonConstSignPostElements();

   // Member variables
   
   /** Container for all sing post elements. The index in the vector determines
    *  the order of the individual elements in the set.
    */
   GMSSignPostSetCont m_signPostElements;
   
}; // GMSSignPostSet

#endif // GMSSIGNPOSTSET_H
