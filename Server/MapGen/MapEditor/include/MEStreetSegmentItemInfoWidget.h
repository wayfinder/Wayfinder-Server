/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MESTREETSEGMENTITEMINFOWIDGET_H
#define MESTREETSEGMENTITEMINFOWIDGET_H

#include "config.h"
#include "MEAbstractItemInfoWidget.h"
#include "OldStreetSegmentItem.h"
#include <gtkmm/entry.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/spinbutton.h>


/**
 *    Widget that shows information that is valid for all street segments.
 *
 */
class MEStreetSegmentItemInfoWidget : public MEAbstractItemInfoWidget {
   public:
      MEStreetSegmentItemInfoWidget();
      virtual ~MEStreetSegmentItemInfoWidget();
      void activate(MEMapArea* mapArea, OldStreetSegmentItem* ssi);
      
#ifdef MAP_EDITABLE
      virtual void saveChanges(MELogCommentInterfaceBox* logCommentBox);
#endif // MAP_EDITABLE

   private:
      /// The roadclass of this street segment.
      Gtk::Entry* m_roadClassVal;
      Gtk::Entry* m_roadConditionVal; // paved, unpaved, poor condition

      /// The house-numbers on this street segment.
      Gtk::SpinButton* m_houseNumberValLS;
      Gtk::SpinButton* m_houseNumberValLE;
      Gtk::SpinButton* m_houseNumberValRS;
      Gtk::SpinButton* m_houseNumberValRE;
      Gtk::Entry* m_houseNumberTypeVal;

      Gtk::CheckButton* m_rampVal;
      Gtk::CheckButton* m_roundaboutVal;
      Gtk::CheckButton* m_roundaboutishVal;
      Gtk::CheckButton* m_multidigVal;
      Gtk::CheckButton* m_controlledAccessVal;

      int getStreetNumberType() {
         const char* str = m_houseNumberTypeVal->get_text().c_str();
         if ( (str != NULL) && (strlen(str) > 0)) {
            int x = strtol(str, NULL, 10);
            return (x);
         }
         return -1;
      }

};

#endif

