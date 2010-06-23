/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SIGNPOSTSTABLE_H
#define SIGNPOSTSTABLE_H

#include "config.h"
#include <vector>
#include <map>
#include "GMSSignPost.h"

// Forward declarations.
class DataBuffer;
class OldGenericMap;

/**
 *   Used for storing signposts of connections.
 *
 */
class SignPostTable {
 public:

   /// Destructor.
   ~SignPostTable();
   
   /**
    * @return Returns the number of connections having signposts stored in this
    *         table.
    */
   uint32 size() const;

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


   /** Adds a sign post part valid for the connection from fromNodeID to
    *  toNodeID. 
    * 
    *  @param fromNodeID The node from which the connection of this sign
    *                    post data is to be stored.
    *  @param toNodeID   The node to which the connection of this sing post 
    *                    data is to be stored.
    *  @param signPostIdx The index of this sign post. There may be many
    *                       sign posts stored for the same connection.
    *  @param singPostSetIdx The sign post set to add the sign post data to.
    *  @param signPostElmIdx The sign post part within the set to add the
    *                        signpost data to.
    *  @param singPostElm The sign post data to add.
    *
    *  @return Returns false if the sing post could not be added.
    */
   bool addSignPostElm( uint32 fromNodeID, uint32 toNodeID,
                        uint32 signPostIdx,
                        uint32 signPostSetIdx, uint32 signPostElmIdx,
                        const GMSSignPostElm& signPostElm );
   /** Creates a new sign post, if the text in signPostElm does not already
    *  exist, and adds it as its only element.
    *
    *  @param fromNodeID  The node from which the connection of this sign
    *                     post data is to be stored.
    *  @param toNodeID    The node to which the connection of this sing post 
    *                     data is to be stored.
    *  @param signPostElm The text to use for this sign post.
    *
    *  @return Returns false if the sing post could not be added.
    */
   bool addSingleElmSignPost( uint32 fromNodeID, uint32 toNodeID, 
                              const GMSSignPostElm& signPostElm );

   /** Copies the sign post stored in otherMap on (otherFromNodeID, 
    *  otherToNodeID),to the map theMap on (fromNodeID, toNodeID).
    *
    *  @return Nubmer of sing posts copied.
    */
   uint32 copySignPostsFromOtherMap(OldGenericMap& theMap,
                                  uint32 fromNodeID, 
                                  uint32 toNodeID,
                                  const OldGenericMap& otherMap,
                                  uint32 otherFromNodeID, 
                                  uint32 otherToNodeID);


   /**
    * @return Returns number of sing posts of the connection from fromNodeID
    *         to toNodeID.
    */
   uint32 getNbrSignPosts( uint32 fromNodeID, uint32 toNodeID ) const;

   /**
    * @return Returns all sing posts of the connection from fromNodeID to
    *         toNodeID.
    */
   const vector<GMSSignPost*>& getSignPosts( uint32 fromNodeID, 
                                             uint32 toNodeID ) const;



   /**
    * @return Returns true if a sing post composed by ony one sign post elememt
    *         which equals signPostElm exists for the connection from 
    *         fromNodeID to toNodeID.
    */
   bool signPostExists( uint32 fromNodeID, uint32 toNodeID, 
                        const GMSSignPostElm& signPostElm ) const;

   /**
    * Removes all sing posts of the conncetion from fromNodeID to toNodeID.
    *
    * @return Returns true if any sing posts have been removed.
    */
   bool removeSignPosts( uint32 fromNodeID, uint32 toNodeID );

   /** Removes the sing post element matching singPostText.
    *
    *  @param theMap The map this sign post is stored in.
    *  @param signPostText The sing post elemetns matching this text are
    *         removed.
    *  @return True if a sing post element was removed.
    */
   bool removeSignPostElm( const OldGenericMap& theMap,
                           uint32 fromNodeID, uint32 toNodeID, 
                           MC2String signPostText );

   /**
    *  This method is used for externally modifying the string codes because of
    *  character encoding.
    *
    *  @return Returns non-const pointers to all sign post element texts
    *          string condes. 
    */
   vector<uint32*> getEditableStrCodePointers();

   /**
    * Removes duplicated sing posts, and checks that no holes exists in the 
    * sing post vectors.
    */
   void signPostShapeUp();


 private:
   // Member types

   /**
    * Used as key in m_signPostTable
    */
   typedef pair<uint32, uint32> fromAndToNodeID_t;

   /**
    * Key:   from node ID, to node ID
    * Value: The signposts of the connection identified by the key.
    */
   typedef map< fromAndToNodeID_t, vector<GMSSignPost*> > signPostTable_t;
      

   // Member variables
   
   /**
    * The sign posts of connections, defned by from and to node ID.
    */
   signPostTable_t m_signPostTable;
   
   
}; // SignPostTable

#endif // SIGNPOSTSTABLE_H

