/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSFORESTITEM_H
#define GMSFORESTITEM_H

#include "OldForestItem.h"
#include "GMSItem.h"


/**
  *   GenerateMapServer forest item. Forest item
  *   with extra features needed when creating the map.
  *
  */
class GMSForestItem : public OldForestItem,
                      public GMSItem {
   public:
      /**
        *   Default constructor.
        */
      GMSForestItem();
   
      /**
        *   Constructor with id.
        */
      GMSForestItem(uint32 id);

      /**
        *   Constructor with map type.
        */
      GMSForestItem(GMSItem::map_t mapType);

      /**
        *   Destructor.
        */
      virtual ~GMSForestItem();

      /**
        *   Creates a gms forestItem from mid/mif files.
        *   @param midFile          The attribute file.
        *   @param readRestOfLine   If the rest of the mid line (eol) should 
        *                           be read or not, default true.
        */
      bool createFromMidMif(ifstream& midFile, bool readRestOfLine = true);

   private:
      
      /**
       *    Init member variables of this forest item.
       */
       void initMembers();
};

#endif

