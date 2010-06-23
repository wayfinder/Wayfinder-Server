/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MEStreetSegmentItemLayer.h"
#include "OldStreetSegmentItem.h"
#include "OldNode.h"

#include "config.h"

MEStreetSegmentItemLayer::MEStreetSegmentItemLayer(
      Glib::RefPtr<Gdk::Window>& window, MEGdkColors::color_t col, 
      MEGdkColors::color_t highlightCol, uint8 filterLevel)
   : MERouteableItemLayer(window, 
                          ItemTypes::streetSegmentItem,
                          col, 
                          highlightCol,
                          filterLevel)
{
   MEGdkColors* colors = MEGdkColors::getColors();
   
   // Create the roadClass-GC's
   // Possible Fixme: The new function set_line_attributes contains more
   // paremeters that affect the appearance of lines compared to set_line_width.
   // Had to guess parameters, could be wrong
   m_mainGC->set_line_attributes(4, 
				      Gdk::LINE_SOLID,
				      Gdk::CAP_NOT_LAST,
				      Gdk::JOIN_BEVEL);
   //m_mainGC.set_line_width(4);

   /*
   Gdk::Color gcColors[m_nbrGC];
   gcColors[0] = colors->m_darkSlateGrey;
   gcColors[1] = colors->m_slateGrey;
   gcColors[2] = colors->m_grey;
   gcColors[3] = colors->m_peru;
   gcColors[4] = colors->m_brown;
   gcColors[5] = colors->m_lightBlue;
   gcColors[6] = colors->m_red;
   gcColors[7] = colors->m_red;
   gcColors[8] = colors->m_red;

   uint32 width = 4;
   for (uint32 i=0; i<m_nbrGC; ++i) {
      m_gc[i].create(window); 
      m_gc[i].set_foreground(gcColors[i]);
      m_gc[i].set_line_width(width);
      if (width > 1)
         --width;
   }
   */

   m_roadClass1GC=Gdk::GC::create(window);
   m_roadClass1GC->set_foreground(colors->m_black);
   
   // Possible Fixme: The new function set_line_attributes contains more
   // paremeters that affect the appearance of lines compared to set_line_width.
   // Had to guess parameters, could be wrong
   m_roadClass1GC->set_line_attributes(3, 
				      Gdk::LINE_SOLID,
				      Gdk::CAP_NOT_LAST,
				      Gdk::JOIN_BEVEL);
   //m_roadClass1GC.set_line_width(3);

   m_roadClass2GC=Gdk::GC::create(window);
   m_roadClass2GC->set_foreground(colors->m_darkSlateGrey);
   m_roadClass2GC->set_line_attributes(2, 
				      Gdk::LINE_SOLID,
				      Gdk::CAP_NOT_LAST,
				      Gdk::JOIN_BEVEL);
   //m_roadClass2GC.set_line_width(2);

   m_roadClass3GC=Gdk::GC::create(window);
   m_roadClass3GC->set_foreground(colors->m_lightSlateGrey);
   m_roadClass3GC->set_line_attributes(1, 
				       Gdk::LINE_SOLID,
				       Gdk::CAP_NOT_LAST,
				       Gdk::JOIN_BEVEL);
   //m_roadClass3GC.set_line_width(1);

   m_roadClass4GC=Gdk::GC::create(window);
   m_roadClass4GC->set_foreground(colors->m_lightBlue);
   m_roadClass4GC->set_line_attributes(1, 
				       Gdk::LINE_SOLID,
				       Gdk::CAP_NOT_LAST,
				       Gdk::JOIN_BEVEL);
   //m_roadClass4GC.set_line_width(1);
}

void 
MEStreetSegmentItemLayer::drawItems() 
{
   if (!m_highlightedOnly) {
      mc2dbg4 << "MEStreetSegmentItemLayer::drawItems To draw " 
              << m_allItems.size() << " items" << endl;

      bool skipLevel4 = (m_allItems.size() > 7000);

      multimap<uint32, MEDrawableItem*>::iterator p = m_allItems.begin();
      while (p != m_allItems.end()) {
         mc2dbg8 << "Drawing item..." << endl;

         int roadClass = GET_ZOOMLEVEL(p->second->m_item->getID()) - 2;
         switch (roadClass) {
            case 0 :
               p->second->draw(m_mainGC, m_window);
               break;
            case 1 :
               p->second->draw(m_roadClass1GC, m_window);
               break;
            case 2 :
               p->second->draw(m_roadClass2GC, m_window);
               break;
            case 3 :
               p->second->draw(m_roadClass3GC, m_window);
               break;
            default :
               if (!skipLevel4)
                  p->second->draw(m_roadClass4GC, m_window);
         }
         p++;
      }
   }

   // Highlight
   drawAllHighlight();
}



bool 
MEStreetSegmentItemLayer::highlightItems(
      MEStreetSegmentItemLayer::highlight_t t)
{
   // Reset all highlights
   clearAllHighlight();

   multimap<uint32, MEDrawableItem*>::iterator p = m_allItems.begin();
   switch (t) {
      case (isRamp) :
         while (p != m_allItems.end()) {
            if (static_cast<OldStreetSegmentItem*>(
                        p->second->m_item)->isRamp()) {
               addHighlightID(p->second->m_item->getID());
            }
            p++;
         }
         break;

      case (isDivided) :
         while (p != m_allItems.end()) {
            if (static_cast<OldStreetSegmentItem*>(p->second->m_item)
                  ->isDivided()) {
               addHighlightID(p->second->m_item->getID());
            }
            p++;
         }
         break;

      case (isRoundabout) :
         while (p != m_allItems.end()) {
            if (static_cast<OldStreetSegmentItem*>(p->second->m_item)
                  ->isRoundabout()){
               addHighlightID(p->second->m_item->getID());
            }
            p++;
         }
         break;

      case (isRoundaboutish) :
         while (p != m_allItems.end()) {
            if (static_cast<OldStreetSegmentItem*>(p->second->m_item)
                  ->isRoundaboutish()){
               addHighlightID(p->second->m_item->getID());
            }
            p++;
         }
         break;

      case (isMultidigitalised) :
         while (p != m_allItems.end()) {
            if (static_cast<OldStreetSegmentItem*>(p->second->m_item)
                                          ->isMultiDigitised()) {
               addHighlightID(p->second->m_item->getID());
            }
            p++;
         }
         break;

      case (isNoThroughfare) :
         while (p != m_allItems.end()) {
            OldStreetSegmentItem* ssi = 
               static_cast<OldStreetSegmentItem*>(p->second->m_item);
            if ( (ssi->getNode(0)->getEntryRestrictions() == 
                  ItemTypes::noThroughfare) ||
                 (ssi->getNode(1)->getEntryRestrictions() == 
                  ItemTypes::noThroughfare)) {
               addHighlightID(ssi->getID());
            }
            p++;
         }
         break;

   }
   
   return (true);
}


