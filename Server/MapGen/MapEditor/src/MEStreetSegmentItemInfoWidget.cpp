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
#include "MEStreetSegmentItemInfoWidget.h"
#include "MEMapEditorWindow.h"
#include <gtkmm/combo.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/adjustment.h>
#include <list>
#include "MEFeatureIdentification.h"
#include "OldExtraDataUtility.h"
#include "MELogFilePrinter.h"

MEStreetSegmentItemInfoWidget::MEStreetSegmentItemInfoWidget()
   : MEAbstractItemInfoWidget("SSI")
{
   // Table with 3 rows and 2 columns
   Gtk::Table* table = manage(new Gtk::Table(3, 2));

   Gtk::Label* label = NULL;
   Gtk::Box* box = NULL;
   Gtk::Adjustment* adj = NULL;

   // Road class + road condition
   box = manage(new Gtk::HBox());
   label = manage(new Gtk::Label("Road class"));
   label->set_alignment(XALIGN, YALIGN);
   box->pack_start(*label, true, true);
   m_roadClassVal = manage(new Gtk::Entry());
   m_roadClassVal->set_size_request(100, 18);
   box->pack_start(*m_roadClassVal, true, true);
   label = manage(new Gtk::Label("Road cond"));
   label->set_alignment(XALIGN, YALIGN);
   box->pack_start(*label, true, true);
   m_roadConditionVal = manage(new Gtk::Entry());
   m_roadConditionVal->set_size_request(100, 18);
   box->pack_start(*m_roadConditionVal, true, true);
   table->attach(*box, 0, 2, 0, 1, 
                 Gtk::FILL | Gtk::EXPAND, Gtk::FILL);

   // House numbers
   label = manage(new Gtk::Label("Nbr L/R: type"));
   label->set_alignment(XALIGN, YALIGN);
   // Not sure if parameters correspond to old AttachOptions. 
   //table->attach(*label, 0, 1, 1, 2, FIXED_OPT, FIXED_OPT);
   table->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
   box = manage(new Gtk::HBox());

   adj = manage(new Gtk::Adjustment(0, 0, MAX_UINT16));
   m_houseNumberValLS = manage(new Gtk::SpinButton(*adj));
   m_houseNumberValLS->set_numeric(true);
   m_houseNumberValLS->set_size_request(60, 18);
   box->pack_start(*m_houseNumberValLS, true, true);

   label = manage(new Gtk::Label("-"));
   box->pack_start(*label, false, false);

   adj = manage(new Gtk::Adjustment(0, 0, MAX_UINT16));
   m_houseNumberValLE = manage(new Gtk::SpinButton(*adj));
   m_houseNumberValLE->set_numeric(true);
   m_houseNumberValLE->set_size_request(60, 18);
   box->pack_start(*m_houseNumberValLE);

   label = manage(new Gtk::Label(" / "));
   box->pack_start(*label, false, false);

   adj = manage(new Gtk::Adjustment(0, 0, MAX_UINT16));
   m_houseNumberValRS = manage(new Gtk::SpinButton(*adj));
   m_houseNumberValRS->set_numeric(true);
   m_houseNumberValRS->set_size_request(60, 18);
   box->pack_start(*m_houseNumberValRS);

   label = manage(new Gtk::Label("-"));
   box->pack_start(*label, false, false);

   adj = manage(new Gtk::Adjustment(0, 0, MAX_UINT16));
   m_houseNumberValRE = manage(new Gtk::SpinButton(*adj));
   m_houseNumberValRE->set_numeric(true);
   m_houseNumberValRE->set_size_request(60, 18);
   box->pack_start(*m_houseNumberValRE);

   label = manage(new Gtk::Label(":"));
   box->pack_start(*label, true, true);
   // Housenumber-type
   Gtk::Combo* tmpCombo = manage(new Gtk::Combo());
   m_houseNumberTypeVal = tmpCombo->get_entry();
   list<string> houseNbrTypeList;
   for (unsigned int i=0; i<5; i++) {  // Please change to < "invalidStreetNumberType"
      char tmpStr[8];
      sprintf(tmpStr, "%u", i);
      houseNbrTypeList.push_back(tmpStr);
   }
   tmpCombo->set_popdown_strings(houseNbrTypeList);
   tmpCombo->set_size_request(40,18);
   box->pack_start(*tmpCombo);
   
   table->attach(*box, 1, 2, 1, 2, Gtk::EXPAND, Gtk::FILL);

   // Street segment item attributes
   box = manage(new Gtk::HBox());
   // Ramp?
   m_rampVal = manage(new Gtk::CheckButton("ramp"));
   label = dynamic_cast<Gtk::Label*>(m_rampVal->get_child());
   if(label != NULL)
      label->set_alignment(XALIGN, 0.5);
   box->pack_start(*m_rampVal);
   // Roundabout
   m_roundaboutVal = manage(new Gtk::CheckButton("roundabout"));
   label = dynamic_cast<Gtk::Label*>(m_roundaboutVal->get_child());
   if(label != NULL)
      label->set_alignment(XALIGN, 0.5);
   box->pack_start(*m_roundaboutVal);
   // Roundaboutish
   m_roundaboutishVal = manage(new Gtk::CheckButton("rb_ish"));
   label = dynamic_cast<Gtk::Label*>(m_roundaboutishVal->get_child());
   if(label != NULL)
      label->set_alignment(XALIGN, 0.5);
   box->pack_start(*m_roundaboutishVal);
   // Multi dig
   m_multidigVal = manage(new Gtk::CheckButton("multi dig"));
   label = dynamic_cast<Gtk::Label*>(m_multidigVal->get_child());
   if(label != NULL)
      label->set_alignment(XALIGN, 0.5);
   box->pack_start(*m_multidigVal);
   // Controlled access
   m_controlledAccessVal = manage(new Gtk::CheckButton("ctrl acc"));
   label = dynamic_cast<Gtk::Label*>(m_controlledAccessVal->get_child());
   if(label != NULL)
      label->set_alignment(XALIGN, 0.5);
   box->pack_start(*m_controlledAccessVal);
   table->attach(*box, 0, 2, 2, 3, 
                 Gtk::FILL | Gtk::EXPAND, Gtk::FILL);

   // Set editable
   m_roadClassVal->set_state(Gtk::STATE_INSENSITIVE);
   m_roadConditionVal->set_state(Gtk::STATE_INSENSITIVE);
#ifdef MAP_EDITABLE
   m_houseNumberValLS->set_state(Gtk::STATE_NORMAL);
   m_houseNumberValLE->set_state(Gtk::STATE_NORMAL);
   m_houseNumberValRS->set_state(Gtk::STATE_NORMAL);
   m_houseNumberValRE->set_state(Gtk::STATE_NORMAL);
   m_houseNumberTypeVal->set_state(Gtk::STATE_NORMAL);
   m_rampVal->set_state(Gtk::STATE_NORMAL);
   m_roundaboutVal->set_state(Gtk::STATE_NORMAL);
   m_roundaboutishVal->set_state(Gtk::STATE_NORMAL);
   m_multidigVal->set_state(Gtk::STATE_NORMAL);
#else
   m_houseNumberValLS->set_state(Gtk::STATE_INSENSITIVE);
   m_houseNumberValLE->set_state(Gtk::STATE_INSENSITIVE);
   m_houseNumberValRS->set_state(Gtk::STATE_INSENSITIVE);
   m_houseNumberValRE->set_state(Gtk::STATE_INSENSITIVE);
   m_houseNumberTypeVal->set_state(Gtk::STATE_INSENSITIVE);
   m_rampVal->set_state(Gtk::STATE_INSENSITIVE);
   m_roundaboutVal->set_state(Gtk::STATE_INSENSITIVE);
   m_roundaboutishVal->set_state(Gtk::STATE_INSENSITIVE);
   m_multidigVal->set_state(Gtk::STATE_INSENSITIVE);
   m_controlledAccessVal->set_state(Gtk::STATE_INSENSITIVE);
#endif

   // Add the table to this frame
   add(*table);
}

MEStreetSegmentItemInfoWidget::~MEStreetSegmentItemInfoWidget()
{

}

void
MEStreetSegmentItemInfoWidget::activate(MEMapArea* mapArea, 
                                      OldStreetSegmentItem* ssi)
{
   MEAbstractItemInfoWidget::activate(mapArea, ssi);

   char tmpstr[128];

   // Road class + cond
   sprintf(tmpstr, "%d", uint32(ssi->getRoadClass()));
   m_roadClassVal->set_text(tmpstr);
   sprintf(tmpstr, "%d", uint32(ssi->getRoadCondition()));
   m_roadConditionVal->set_text(tmpstr);

   // Housenumber
   m_houseNumberValLS->set_value(ssi->getLeftSideNbrStart());
   m_houseNumberValLE->set_value(ssi->getLeftSideNbrEnd());
   m_houseNumberValRS->set_value(ssi->getRightSideNbrStart());
   m_houseNumberValRE->set_value(ssi->getRightSideNbrEnd());
   sprintf(tmpstr, "%d", uint32(int(ssi->getStreetNumberType())));
   m_houseNumberTypeVal->set_text(tmpstr);

   // Ramp, roundabout(ish), multi.dig. and controlled access
   m_rampVal->set_active(ssi->isRamp());
   m_roundaboutVal->set_active(ssi->isRoundabout());
   m_roundaboutishVal->set_active(ssi->isRoundaboutish());
   m_multidigVal->set_active(ssi->isMultiDigitised());
   m_controlledAccessVal->set_active(ssi->isControlledAccess());

   show_all();
}

#ifdef MAP_EDITABLE
void
MEStreetSegmentItemInfoWidget::saveChanges(
         MELogCommentInterfaceBox* logCommentBox)
{
   // For log comment string
   char origValStr[64];
   
   // New value strings, holding sub strings for the last part of the extra 
   // data record
   vector<MC2String> newValStrings;
   char tmpStr[256];

   mc2dbg4 << here << "m_mapArea: " << m_mapArea << endl;
   OldGenericMap* map = m_mapArea->getMap();
   MC2_ASSERT(map != NULL);

   // Get identification string, represent the ssi with the node 0
   OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>(m_item);
   OldNode* node = ssi->getNode(0);
   MC2String identString;
   bool identOK = MEFeatureIdentification::getNodeIdentificationString(
                     identString, map, node);
   if ( !identOK ) {
      mc2log << warn << " Failed to get node identification string" << endl;
      return;
   }

   // Housenumbers
   bool houseNumberChanged = false;
   sprintf(origValStr, "%d,%d,%d,%d,%d", ssi->getLeftSideNbrStart(),
                    ssi->getLeftSideNbrEnd(), ssi->getRightSideNbrStart(),
                    ssi->getRightSideNbrEnd(), ssi->getStreetNumberType() );
   if (m_houseNumberValLS->get_value_as_int() != ssi->getLeftSideNbrStart()) {
      ssi->setLeftSideNumberStart(
         uint16(m_houseNumberValLS->get_value_as_int()));
      houseNumberChanged = true;
   }
   if (m_houseNumberValLE->get_value_as_int() != ssi->getLeftSideNbrEnd()) {
      ssi->setLeftSideNumberEnd(
         uint16(m_houseNumberValLE->get_value_as_int()));
      houseNumberChanged = true;
   }
   if (m_houseNumberValRS->get_value_as_int() != ssi->getRightSideNbrStart()) {
      ssi->setRightSideNumberStart(
         uint16(m_houseNumberValRS->get_value_as_int()));
      houseNumberChanged = true;
   }
   if (m_houseNumberValRE->get_value_as_int() != ssi->getRightSideNbrEnd()) {
      ssi->setRightSideNumberEnd(
         uint16(m_houseNumberValRE->get_value_as_int()));
      houseNumberChanged = true;
   }
   int houseType = getStreetNumberType();
   if ( houseNumberChanged || 
        ((houseType >= 0) && 
         (ItemTypes::streetNumberType(houseType) != 
          ssi->getStreetNumberType())) ) {
      //ssi->setStreetNumberType(ItemTypes::streetNumberType(houseType));
      // values have been set to ssi, calculate therefrom to get the
      // correct type (if forgotten to change type in the ssi info widget).
      ItemTypes::streetNumberType newType = 
         ItemTypes::getStreetNumberTypeFromHouseNumbering(
               ssi->getLeftSideNbrStart(), ssi->getLeftSideNbrEnd(),
               ssi->getRightSideNbrStart(), ssi->getRightSideNbrEnd());
      ssi->setStreetNumberType(newType);
      houseNumberChanged = true;
   }
   if (houseNumberChanged) {
      vector<MC2String> logStrings = 
            logCommentBox->getLogComments(origValStr);
      newValStrings.clear();
      sprintf(tmpStr, "%d", ssi->getLeftSideNbrStart());
      newValStrings.push_back( tmpStr );
      sprintf(tmpStr, "%d", ssi->getLeftSideNbrEnd());
      newValStrings.push_back( tmpStr );
      sprintf(tmpStr, "%d", ssi->getRightSideNbrStart());
      newValStrings.push_back( tmpStr );
      sprintf(tmpStr, "%d", ssi->getRightSideNbrEnd());
      newValStrings.push_back( tmpStr );
      sprintf(tmpStr, "%d", ssi->getStreetNumberType());
      newValStrings.push_back( tmpStr );
      MELogFilePrinter::print(
            logStrings, "setHousenumber", identString, newValStrings );
      mc2dbg << "Housenumbers set to: "
             << OldExtraDataUtility::createEDFileString(newValStrings)
             << endl;
   }

   // Ramp
   if (m_rampVal->get_active() != ssi->isRamp()) {
      sprintf( origValStr, "%s",
               StringUtility::booleanAsString(ssi->isRamp()) );
      vector<MC2String> logStrings = logCommentBox->getLogComments(origValStr);
      bool newVal = m_rampVal->get_active();
      ssi->setRampValue( newVal );
      newValStrings.clear();
      newValStrings.push_back( StringUtility::booleanAsString(newVal) );
      MELogFilePrinter::print(
            logStrings, "setRamp", identString, newValStrings );
      mc2dbg << "Ramp set to: "
             << StringUtility::booleanAsString(newVal) << endl;
   }

   // Roundabout
   if (m_roundaboutVal->get_active() != ssi->isRoundabout()) {
      sprintf( origValStr, "%s",
               StringUtility::booleanAsString(ssi->isRoundabout()) );
      vector<MC2String> logStrings = logCommentBox->getLogComments(origValStr);
      bool newVal = m_roundaboutVal->get_active();
      ssi->setRoundaboutValue( newVal);
      newValStrings.clear();
      newValStrings.push_back( StringUtility::booleanAsString(newVal) );
      MELogFilePrinter::print(
            logStrings, "setRoundabout", identString, newValStrings );
      mc2dbg << "Roundabout set to: "
             << StringUtility::booleanAsString(newVal) << endl;
   }
   
   // Roundaboutish
   if (m_roundaboutishVal->get_active() != ssi->isRoundaboutish()) {
      sprintf( origValStr, "%s",
               StringUtility::booleanAsString(ssi->isRoundaboutish()) );
      vector<MC2String> logStrings = logCommentBox->getLogComments(origValStr);
      bool newVal = m_roundaboutishVal->get_active();
      ssi->setRoundaboutishValue( newVal );
      newValStrings.clear();
      newValStrings.push_back( StringUtility::booleanAsString(newVal) );
      MELogFilePrinter::print(
            logStrings, "setRoundaboutish", identString, newValStrings );
      mc2dbg << "Roundaboutish set to: "
             << StringUtility::booleanAsString(newVal) << endl;
   }

   // Multi dig
   if (m_multidigVal->get_active() != ssi->isMultiDigitised()) {
      sprintf( origValStr, "%s",
               StringUtility::booleanAsString(ssi->isMultiDigitised()) );
      vector<MC2String> logStrings = logCommentBox->getLogComments(origValStr);
      bool newVal = m_multidigVal->get_active();
      ssi->setMultiDigitisedValue( newVal );
      newValStrings.clear();
      newValStrings.push_back( StringUtility::booleanAsString(newVal) );
      MELogFilePrinter::print(
            logStrings, "setMultiDigitised", identString, newValStrings );
      mc2dbg << "Multidig. set to: "
             << StringUtility::booleanAsString(newVal) << endl;
   }

   // Controlled access
   if (m_controlledAccessVal->get_active() != ssi->isControlledAccess()) {
      sprintf( origValStr, "%s",
               StringUtility::booleanAsString(ssi->isControlledAccess()) );
      vector<MC2String> logStrings = logCommentBox->getLogComments(origValStr);
      bool newVal = m_controlledAccessVal->get_active();
      ssi->setControlledAccessValue( newVal );
      newValStrings.clear();
      newValStrings.push_back( StringUtility::booleanAsString(newVal) );
      MELogFilePrinter::print(
            logStrings, "setControlledAccess", identString, newValStrings );
      mc2dbg << "Controlled access set to: "
             << StringUtility::booleanAsString(newVal) << endl;
   }

}
#endif // MAP_EDITABLE


