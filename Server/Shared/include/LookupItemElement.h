/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LOOKUPITEMELEMENT_H
#define LOOKUPITEMELEMENT_H

#include "config.h"
#include "VectorElement.h"
#include "ReverseLookupItemElement.h"

/**
  *   Class that contains the items that is included in the 
  *   overview map from one true map.
  *   
  */
class LookupItemElement : public VectorElement {
   public:
      /**
        *   This constructor might only be for temporary objects,
        *   e.g. when doing a search.
        */
      inline LookupItemElement(uint32 localItemID);

      /**
       * 
       *   @param   reverse is a pointer to the object containing data 
       */
      inline LookupItemElement(ReverseLookupItemElement *reverse);

      /**
        *   @name The operators needed for sorting and searching.
        */
      //@{
         /// equal
         bool operator == (const VectorElement& elm) const {
            return ( m_reverse->m_overviewItemID == 
                     (static_cast<const LookupItemElement*> (&elm))
                        ->m_reverse->m_overviewItemID);
         };

         /// not equal
         bool operator != (const VectorElement& elm) const {
            return ( m_reverse->m_overviewItemID != 
                     (static_cast<const LookupItemElement*> (&elm) )
                        ->m_reverse->m_overviewItemID);
         };

         /// greater
         bool operator > (const VectorElement& elm) const {
            return ( m_reverse->m_overviewItemID > 
                     (static_cast<const LookupItemElement*> (&elm) )
                        ->m_reverse->m_overviewItemID);
         };

         /// less
         bool operator < (const VectorElement& elm) const {
            return ( m_reverse->m_overviewItemID < 
                     (static_cast<const LookupItemElement*> (&elm) )
                        ->m_reverse->m_overviewItemID);
         };
      //@}

      /**
        *   
        */
      inline uint32 getOverviewItemID();

      /**
        *
        */
      inline uint32 getTrueMapID();

      /**
        *
        */
      inline uint32 getTrueItemID();

      inline ReverseLookupItemElement* getReverseItem();
   private:
      /**
        *   
        */
      ReverseLookupItemElement* m_reverse;

};

// ==================================================================
//                                Implementation of inlined methods =

inline LookupItemElement::LookupItemElement(uint32 localItemID)
{
   m_reverse = new ReverseLookupItemElement( 0, localItemID );
}

inline LookupItemElement::LookupItemElement(ReverseLookupItemElement *reverse)
   : m_reverse(reverse)
{
   //m_reverse = reverse;
}

inline uint32
LookupItemElement::getOverviewItemID() 
{
   return (m_reverse->m_overviewItemID);
}

inline uint32 
LookupItemElement::getTrueMapID()
{
   return (uint32(m_reverse->m_mapAndItemID >> 32));
}

inline uint32
LookupItemElement::getTrueItemID()
{
   return (uint32(m_reverse->m_mapAndItemID & 0x00000000ffffffff));
}

inline ReverseLookupItemElement*
LookupItemElement::getReverseItem()
{
   return m_reverse;
}

#endif
