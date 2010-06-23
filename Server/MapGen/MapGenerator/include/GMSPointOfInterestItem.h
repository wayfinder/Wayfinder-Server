/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSPOINTOFINTERESTITEM_H
#define GMSPOINTOFINTERESTITEM_H

#include "OldPointOfInterestItem.h"
#include "GMSItem.h"

/**
  *   GenerateMapServer company item. Company item
  *   with extra features needed when creating the map.
  *
  */
class GMSPointOfInterestItem : public OldPointOfInterestItem,
                               public GMSItem {
   public:
      /**
        *   Default constructor
        */
      GMSPointOfInterestItem();
   
      /**
        *   Constructor with id.
        */
      GMSPointOfInterestItem(uint32 id);

      /**
        *   Constructor with map type.
        */
      GMSPointOfInterestItem(GMSItem::map_t mapType);

      /**
        *   Creates a OldPointOfInterestItem with the specified parameters.
        */
      GMSPointOfInterestItem(uint32 streetSegmentID, 
                             uint16 offsetOnStreet);
      
      /**
        *   Destructor
        */
      virtual ~GMSPointOfInterestItem();
       

      /**
       * Set and get artificial temporary value.
       * Used for removing unwanted = aritificial city centres.
       */
      //@{
      inline bool isArtificial();
      inline void setArtificial(bool artificial);
      //@}
      
      
   private:
      
      /**
       *    Init member variables of this poi item.
       */
      void initMembers();

      /// Used for marking artificial city centres to later remove them.
      bool m_artificial;

};


bool
GMSPointOfInterestItem::isArtificial() {
   return m_artificial;
}

void
GMSPointOfInterestItem::setArtificial(bool artificial) {
   m_artificial = artificial;
}

#endif

