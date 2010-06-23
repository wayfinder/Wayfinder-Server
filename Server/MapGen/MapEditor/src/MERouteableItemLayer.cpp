/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "MERouteableItemLayer.h"

MERouteableItemLayer::MERouteableItemLayer(Glib::RefPtr<Gdk::Window>& window,
                                       ItemTypes::itemType type,
                                       MEGdkColors::color_t col, 
                                       MEGdkColors::color_t highlightCol,
                                       uint8 filterLevel)
   : MEItemLayer(window, type, col, highlightCol, filterLevel),
     m_highlightedMaxCostItems(),
     m_highlightedCostItems()
{
   MEGdkColors* colors = MEGdkColors::getColors();
   
   // Create the connection highlight GC
   m_connectionHighlightGC=Gdk::GC::create(window);
   m_connectionHighlightGC->set_line_attributes(3, 
				      Gdk::LINE_SOLID,
				      Gdk::CAP_NOT_LAST,
				      Gdk::JOIN_BEVEL);
   //m_connectionHighlightGC.set_line_width(3);
   m_connectionHighlightGC->set_foreground(colors->m_orange);

   // Create cost-GC's
    m_highlightCostGC = Gdk::GC::create(window);
   //m_highlightCostGC=m_highlightCostGC->create(window);
   m_highlightCostGC->set_foreground(colors->m_orange);
   //m_highlightCostGC.set_line_width(3);
    m_highlightCostGC->set_line_attributes(3, 
				      Gdk::LINE_SOLID,
				      Gdk::CAP_NOT_LAST,
				      Gdk::JOIN_BEVEL);
   m_highlightMaxCostGC=Gdk::GC::create(window);
   m_highlightMaxCostGC->set_foreground(colors->m_orange);

   // Reset node-ids
   m_highlightNodeID = MAX_UINT32;
   m_highlightConnectionNodeID = MAX_UINT32;
   
}

void 
MERouteableItemLayer::drawAllHighlight()
{
   mc2dbg4 << "MERouteableItemLayer::drawAllHighlight()" << endl;

   // Highlight m_highlightedItemIDs
   MEItemLayer::drawAllHighlight();

   // Highlight m_highlightedMaxCostItems
   for (uint32 i=0; i<m_highlightedMaxCostItems.getSize(); i++) {
      drawItemWithID(m_highlightedMaxCostItems[i], m_highlightMaxCostGC);
   }

   // Highlight m_highlightedCostItems
   for (uint32 i=0; i<m_highlightedCostItems.getSize(); i++) {
      drawItemWithID(m_highlightedCostItems[i], m_highlightCostGC);
   }

   // Highlight m_highlightNodeID ?   true == include the node
   drawItemWithID(m_highlightNodeID, m_highlightGC, true); 

   // Highlight m_highlightConnectionNodeID ?    true == include the node
   drawItemWithID(m_highlightConnectionNodeID, m_connectionHighlightGC, true);
}

void
MERouteableItemLayer::clearAllHighlight()
{
   // Clear in the superclass
   MEItemLayer::clearAllHighlight();

   // Clear nodes etc. added in subclass.
   m_highlightNodeID = MAX_UINT32;
   m_highlightConnectionNodeID = MAX_UINT32;
}


bool 
MERouteableItemLayer::highlightConnection(uint32 toNodeID, 
                                        uint32 fromNodeID)
{
   m_highlightNodeID = toNodeID;
   m_highlightConnectionNodeID = fromNodeID;
   return (true);
}


bool 
MERouteableItemLayer::addCostHighlight(uint32 nodeID, uint32 cost)
{
   m_highlightNodeID = nodeID;
   if (cost == 0) {
      addHighlightID(nodeID);
      return (true);
   } else if (cost == MAX_UINT32) {
      m_highlightedMaxCostItems.addLast(nodeID);
      return (true);
   } else {
      m_highlightedCostItems.addLast(nodeID);
      return (true);
   }
}

