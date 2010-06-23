/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CITYPARTITEM_H
#define CITYPARTITEM_H

#include "config.h"
#include "Item.h"
#include "DataBuffer.h"

/**
  *   Describes one citypart on the map.
  *
  */
class CityPartItem : public Item {
public:
   /**
    *    Default construcor, implemented to be as fast as possible.
    */
   inline CityPartItem() { } ;

   /**
    *   Creates an item containing information about a Citypart. 
    *   This constructor extracts (parses) the information from the 
    *   dataBuffer.
    *
    *   @param   id The localID of this item
    */
   explicit CityPartItem(uint32 id);

   /**
    *   Destroys the CityPartItem.
    */
   ~CityPartItem();

   /**
    *   Writes the item into the dataBuffer.
    *   @param   dataBuffer Where to print the information.
    *   @return  True if no errors occurred, false if otherwise.
    */
   void save( DataBuffer& dataBuffer, const GenericMap& map ) const;
   /**
    *    Fill this item with information from the databuffer.
    *    @param   dataBuffer  The buffer with the data.
    *    @return  True if the data of the item is set, false
    *             otherwise.
    */
   void load( DataBuffer& dataBuffer, GenericMap& theMap );


};

#endif

