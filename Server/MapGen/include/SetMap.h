/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SETMAP_H
#define SETMAP_H

#include "config.h"


/**
  * This class is a STL map of sets. Use it when you want something sorted
  * sorted by something else. Main benefit is that you don't have to create
  * the elements in the map. 
  *
  * KeyType The type used as key in the map.
  * DataType The type used for the elements of the set.
  */
template<class KeyType, class DataType>
class SetMap: public map<KeyType, set<DataType> > {
 public:
   /**
    * @param mapKey Identifies the set where to insert the data, a new set
    *               is created if a set with this identifier does not exist.
    * @param data   The data to insert into the set.
    *
    * @return True if a new element was created in the set, false otherwise.
    */
   bool insertToSet( KeyType mapKey, DataType data ){
      typename SetMap<KeyType, DataType>::iterator it = 
         this->find(mapKey);
      if ( it == this->end() ){
         pair<KeyType, set<DataType> > mapItem;
         mapItem.first = mapKey;
         it = this->insert(mapItem).first;
      }
      return it->second.insert(data).second;
   }

}; // SetMap








#endif // SETMAP
