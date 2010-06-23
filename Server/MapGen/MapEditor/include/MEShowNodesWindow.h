/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MESHOWNODESWINDOW_H
#define MESHOWNODESWINDOW_H

#include "config.h"
#include <gtkmm/window.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/fileselection.h>
#include <gtkmm/frame.h>
#include <vector>
#include <list>
#include "MEMapArea.h"

/**
 *    Reads data from an infile and shows it on the map and, in some 
 *    cases, some data in the dialog. Currently handles the following
 *    "records", unknown lines are skiped:
 *    <ul>
 *       <li><tt>NODE: mapID.nodeID</tt></li>
 *       <li><tt>TURNDESCRIPTION: mapID.fromNodeID;mapID.toNodeID;
 *           Old turndesc;Old crossingkind</tt></li>
 *    </ul>
 *
 */
class MEShowNodesWindow : public Gtk::Window {
   public:
      /**
       *    Use this static method as constructor. Will return a pointer
       *    to a window. This method will make sure that we only have one
       *    instance of the ShowNodesWindow.
       */
      static MEShowNodesWindow* instance(MEMapArea* mapArea) {
         if (m_theTurnDesciptionsWindow == NULL) {
            // No existing window
            m_theTurnDesciptionsWindow = 
               manage(new MEShowNodesWindow(mapArea));         
         } else if (m_theTurnDesciptionsWindow->m_mapArea != mapArea) {
            delete m_theTurnDesciptionsWindow;
            m_theTurnDesciptionsWindow = 
               manage(new MEShowNodesWindow(mapArea));         
         }
         return m_theTurnDesciptionsWindow;
      }

      /**
       *    Delete this window and everything that is allocated here.
       */
      virtual ~MEShowNodesWindow();


   private:
      /**
       *    Create the window that should be used when simulating the
       *    car on a route. Declared private to make sure we only have
       *    one instance of this class.
       *    @see instance
       */
      MEShowNodesWindow(MEMapArea* mapArea);

      void on_clickShowAll();
      void on_selectFile();
      void on_fileSelOK();
      void on_fileSelCancel();
      void on_loadFile();
      void on_showNode();


      void on_clickPrev();
      void on_clickNext();

      uint32 processRow( Gtk::TreeModel::Row row,
                         bool locateAndRedraw = true );

      Gtk::TreeModel::iterator m_selectedRow;

      /**
       *    The file-selection dialog.
       */
      Gtk::FileSelection* m_fileSelector;
      Gtk::Entry* m_fileNameEntry;

      /**
       *
       */
      Gtk::Label* m_curTurnDesc;
      Gtk::Label* m_curCrossingKind;
      Gtk::Label* m_oldTurnDesc;
      Gtk::Label* m_oldCrossingKind;


      MEMapArea* m_mapArea;

      static MEShowNodesWindow* m_theTurnDesciptionsWindow;

       /*
       * Various TreeViewer structures
       */

      Gtk::TreeView m_treeView;
      Glib::RefPtr<Gtk::TreeView::Selection> m_selection;
      Glib::RefPtr<Gtk::ListStore> m_listStore;

      /// Internal class for data columns:
      class Columns : public Gtk::TreeModel::ColumnRecord
      {
      public:

         Columns()
         { add(m_from); add(m_to); 
           add(m_oldTurn); add(m_oldCK);}

         Gtk::TreeModelColumn<Glib::ustring> m_from;
         Gtk::TreeModelColumn<Glib::ustring> m_to;
         Gtk::TreeModelColumn<Glib::ustring> m_oldTurn;
         Gtk::TreeModelColumn<Glib::ustring> m_oldCK;
      };

      Columns m_columns;



};

#endif

