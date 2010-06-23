/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MESTREETSEGMENTITEMLAYER_H
#define MESTREETSEGMENTITEMLAYER_H

#include "config.h"
#include "MERouteableItemLayer.h"

/**
 *    Describes the StreetSegmentItems (SSI:s) on the map. This is a 
 *    subclass to the MERouteableItemLayer since the StreetSegments should
 *    be drawn, using information about road class etc.
 *
 */
class MEStreetSegmentItemLayer : public MERouteableItemLayer {
   public:
      enum  highlight_t {
         isRamp,
         isDivided,
         isRoundabout,
         isRoundaboutish,
         isMultidigitalised,
         isNoThroughfare
      };
   
      /**
       *    Create a new layer with the street segment items in the map.
       *    @param
       */
      MEStreetSegmentItemLayer(Glib::RefPtr<Gdk::Window>& window, 
                             MEGdkColors::color_t col, 
                             MEGdkColors::color_t highlightCol,
                             uint8 filterLevel);

      virtual ~MEStreetSegmentItemLayer() {};

      /**
       *    The SSI:s are drawn a bit different from the other items. E.g.
       *    the thickness of the lines depends on the roadclass.
       */
      virtual void drawItems();


      bool highlightItems(highlight_t t);

   private:
      /*#define m_nbrGC 9
      Gdk::GC m_gc[m_nbrGC];*/

      Glib::RefPtr<Gdk::GC> m_roadClass1GC;
      Glib::RefPtr<Gdk::GC> m_roadClass2GC;
      Glib::RefPtr<Gdk::GC> m_roadClass3GC;
      Glib::RefPtr<Gdk::GC> m_roadClass4GC;
};


#endif


