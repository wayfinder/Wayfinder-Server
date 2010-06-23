/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REVERSELOOKUPITEMELEMENT_H
#define REVERSELOOKUPITEMELEMENT_H


#define MAKE_UINT64(a,b) uint64( ( uint64((uint32)a)<<32 ) | uint64(uint32(b)) )

#include "config.h"
#include "VectorElement.h"


/**
  *   Class that contains the items that is included in the 
  *   overview map from one true map.
  *
  */
class ReverseLookupItemElement : public VectorElement {
   friend class LookupItemElement;
   
   public:
      /**
        *   This constructor might only be for temporary objects,
        *   e.g. when doing a search.
        *   @param mapAndItemID  64 bits with the map and item ID in the 
        *                        lower level map.
        */
      inline ReverseLookupItemElement(uint64 mapAndItemID)
         : m_mapAndItemID(mapAndItemID), m_overviewItemID(MAX_UINT32) { };

      /**
        *   Creates an MapTranslationMap with a specified map id.
        *   @param mapAndItemID  64 bits with the map and item ID in the 
        *                        lower level map.
        *   @param localItemID   The ID of the same item in the overview map.
        */
      inline ReverseLookupItemElement(uint64 mapAndItemID, 
                                      uint32 overviewItemID)
         : m_mapAndItemID(mapAndItemID), m_overviewItemID(overviewItemID) { };

      /**
        *   Deletes this element and releases the used memory.
        */
      virtual ~ReverseLookupItemElement() {};

      /**
        *   @name The operators needed for sorting and searching.
        */
      //@{
         /// equal
         bool operator == (const VectorElement& elm) const {
            return ( m_mapAndItemID == 
                     (static_cast<const ReverseLookupItemElement*> (&elm))
                        ->m_mapAndItemID);
         };

         /// not equal
         bool operator != (const VectorElement& elm) const {
            return ( m_mapAndItemID != 
                     (static_cast<const ReverseLookupItemElement*> (&elm) )
                        ->m_mapAndItemID);
         };

         /// greater
         bool operator > (const VectorElement& elm) const {
            return ( m_mapAndItemID > 
                     (static_cast<const ReverseLookupItemElement*> (&elm) )
                        ->m_mapAndItemID);
         };

         /// less
         bool operator < (const VectorElement& elm) const {
            return ( m_mapAndItemID < 
                     (static_cast<const ReverseLookupItemElement*> (&elm) )
                        ->m_mapAndItemID);
         };
      //@}

      /**
       *    Get the ID of the described item in the overview map.
       */
      inline uint32 getOverviewItemID();

      /**
       *    Get the ID of the lower level map where the described item 
       *    is located.
       */
      inline uint32 getTrueMapID();

      /**
       *    Get the ID of the item in the lower level map.
       */
      inline uint32 getTrueItemID();

   private:
      /**
        *   The map and itemID of the described item in the lower level map.
        */
      uint64 m_mapAndItemID;

      /**
        *   The id of the described item in the overview map.
        */
      uint32 m_overviewItemID;
};

// ==================================================================
//                                Implementation of inlined methods =

/*
ReverseLookupItemElement::ReverseLookupItemElement(
                                    uint64 mapAndItemID)
   : m_mapAndItemID(mapAndItemID), m_overviewItemID(MAX_UINT32)
{
   //m_mapAndItemID = mapAndItemID;
   //m_overviewItemID = MAX_UINT32;
}

ReverseLookupItemElement::ReverseLookupItemElement(
                                    uint64 mapAndItemID,
                                    uint32 overviewItemID)
   : m_mapAndItemID(mapAndItemID), m_overviewItemID(overviewItemID)
{
   //m_mapAndItemID = mapAndItemID;
   //m_overviewItemID = overviewItemID;
}

ReverseLookupItemElement::~ReverseLookupItemElement()
{
   // Nothing to do
}
*/

inline uint32
ReverseLookupItemElement::getOverviewItemID() 
{
   return (m_overviewItemID);
}

inline uint32 
ReverseLookupItemElement::getTrueMapID()
{
   return (uint32(m_mapAndItemID >> 32));
}

inline uint32
ReverseLookupItemElement::getTrueItemID()
{
   return (uint32(m_mapAndItemID & 0x00000000ffffffff));
}


#endif

