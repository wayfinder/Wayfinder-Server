/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MELOGCOMMENTINTERFACEBOX_H
#define MELOGCOMMENTINTERFACEBOX_H

#include "config.h"
#include <gtkmm/entry.h>
#include <gtkmm/box.h>
#include <gtkmm/combo.h>
#include <vector>

#include "MC2String.h"

/**
 *    This box contains labels and fields where to write e.g. the
 *    source and comment when making a change to a map. Here is also
 *    a method to get this information (including time, username etc.)
 *    in a format understod by ExtraDataReader.
 * 
 */
class MELogCommentInterfaceBox : public Gtk::HBox {
   public:
      /**
       *    Create a new box with entries for source and comment.
       */
      MELogCommentInterfaceBox(const char* countryName = "country",
                             uint32 mapId = MAX_UINT32);

      /**
       *    Get all information for the extra data comment record. It
       *    collects information from the edit-boxes in this class and,
       *    if provided, original value. The format is understod as 
       *    a comment by the ExtraDataReader.
       *
       *    @param origValStr A string containing the original value of
       *                      the change to be written in the logString.
       *                      Default is an empty string.
       *    @return  A vector with all sub strings that is the 
       *             extra data comment record.
       */
      vector<MC2String> getLogComments( const char* origValStr = "" );

   private:

      /**
       *    The entry- and combo-boxes with the values for the log comment.
       */
      //@{
         /// An entry-box where to write the source of the change.
         Gtk::Entry* m_mapEditSource;

         /// An entry-box where to write any comment to the change.
         Gtk::Entry* m_mapEditComment;

         /// An entry-box where to write insert type of the ed record created.
         Gtk::Entry* m_mapEditEDInsertType;

         /// Combo-box with the possible values of extra data insert type.
         Gtk::Combo* m_mapEditEDInsertTypeCombo;

         /**
          *    An entry-box where to write the group of the
          *    extra data record, if it should be part of any.
          */
         Gtk::Entry* m_mapEditEDGroup;

         /// An entry-box where to write a reference ID for the change.
         Gtk::Entry* m_mapEditRefID;

         /// An empty entry-box (to make the logboxes symmetric).
         Gtk::Entry* m_mapEditEmpty;
      //@}

      /**
       *    The country name of the map for which this LCIB was created.
       */
      const char* m_countryName;

      /**
       *    The mapId of the map for which this LCIB was created.
       */
      uint32 m_mapId;
};

#endif

