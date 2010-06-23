/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEROUTEABLEITEMINFOWIDGET_H
#define MEROUTEABLEITEMINFOWIDGET_H

#include "config.h"
#include "MEAbstractItemInfoWidget.h"
#include "OldRouteableItem.h"
#include <gtkmm.h>
#include <gtkmm/treeview.h>
#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/entry.h>
#include <gtkmm/combo.h>
#include <gtkmm/checkbutton.h>


class MENodeInfoBox;
class OldConnection;
class MEConnectionInfoBox;
class OldNode;
class METreeItem;

/**
 *    Widget that shows information that is common for all RouteableItems. 
 *    That is e.g. nodes and connections.
 *
 */
class MERouteableItemInfoWidget : public MEAbstractItemInfoWidget {
   public:
      MERouteableItemInfoWidget();
      virtual ~MERouteableItemInfoWidget();
      void activate(MEMapArea* mapArea, OldRouteableItem* item); 
      
#ifdef MAP_EDITABLE
      /**
       *    Save the changes that are done to m_item in this info widget.
       *    @param logCommentBox Where to get the comment, source etc.
       */
      virtual void saveChanges(MELogCommentInterfaceBox* logCommentBox);
#endif // MAP_EDITABLE
      
   private:

      /*
       * Function to handle when user clicks a routable node
       * or connection.
       */
      void on_routable_treeview_row_activated();

      Gtk::TreeView* m_nodeTree;

      MENodeInfoBox* m_nodeInfoBox; 
      MEConnectionInfoBox* m_connectionInfoBox;

      /*
       * Various TreeViewer structures for routable nodes and connections
       */

      // TreeViewer to visualize the extradata
      Gtk::TreeView m_TreeView;

      // Structure for handling TreeView selections.
      Glib::RefPtr<Gtk::TreeView::Selection> m_Selection;

      // Underlying data structure
      Glib::RefPtr<Gtk::TreeStore> m_TreeStore;

      // Internal class for extradata columns:
      class Columns : public Gtk::TreeModel::ColumnRecord
      {
      public:

         Columns()
         { add(m_info); add(m_treeItem);}

         Gtk::TreeModelColumn<MC2String> m_info;
         Gtk::TreeModelColumn<METreeItem *> m_treeItem;
      };

      // Extradata columns
      Columns m_Columns;
};

/**
 *    The information about the node.
 */
class MENodeInfoBox : public Gtk::VBox {
   public:
      /**
       *    Create a new Node-information box.
       */
      MENodeInfoBox();

      /**
       *    Delete the node-information box.
       */
      virtual ~MENodeInfoBox();

      /**
       *    Add data about one node to this box.
       */
      void addNodeData(OldNode* node);

      /**
       *    Clear all attribute-values.
       */
      void reset();

      /**
       *    Get the ID of the node that is currently displayed.
       *    @return The ID of the current node. MAX_UINT32 will be
       *            returned if no node is selected.
       */
      uint32 getNodeID();

      /**
       *    Get the seleced value of the entryrestrictions.
       *    @return An integer that is the value of the
       *            entryrestriciont_t-enumeration. Returned as an
       *            integer to be able to return error (a negative
       *            value).
       */
      int getEntryRestrictions();

      /**
       *    Get the speedlimit from the entry-box.
       *    @return The speedlimit (in km/h). A negative value is
       *            returned upon error.
       */
      int getSpeedLimit();

      /**
       *    Get the level from the entry-box.
       *    @return The level, relative ground-level. MAX_INT32 is
       *            returned upon error.
       */
      int getLevel();

#ifdef MAP_EDITABLE
      void saveChanges(MELogCommentInterfaceBox* logCommentBox,
                       OldGenericMap* curMap);
#endif

   private:
      /**
       *    The node that is represented in this box.
       */
      OldNode* m_node;

      /**
       *    The table where all the attributes are shown.
       */
      Gtk::Table* m_nodeTable;

      /**
       *    The ID of the displayed node.
       */
      Gtk::Entry* m_nodeIDVal;

      /**
       *    The entry restriction when entering the street at this node.
       */
      Gtk::Entry* m_restrictionsVal;

      /**
       *    All available entry-restrictions.
       */
      Gtk::Combo* m_restrictionsCombo;

      /**
       *    The speedlimit when entering the street at this node
       *    (km/h).
       */
      Gtk::Entry* m_speedlimitVal;

      /**
       *    The type of junction.
       */
      Gtk::Entry* m_junctionTypeVal;

      /**
       *    The level of this node, relative ground-level.
       */
      Gtk::Entry* m_levelVal;

      /**
       *    "True" if major road, "False" otherwise.
       */
      Gtk::Entry* m_majorRoadVal;

      /**
       *    The maximum weight, in tons.
       */
      Gtk::Entry* m_maxWeightVal;

      /**
       *    The maximum height, in decimeters.
       */

      Gtk::Entry* m_maxHeightVal;

      /**
       *    The number of lanes when traversing the street from this
       *    node to the other.
       */
      Gtk::Entry* m_nbrLanesVal;

      /**
       *    "True" if toll road, "False" otherwise when entring the street
       *    segment in this node.
       */
      Gtk::CheckButton* m_roadTollVal;
};


/**
 *    This class contains information about the selected connection.
 *    
 */
class MEConnectionInfoBox : public Gtk::VBox {
   public:
      /**
       *    Create a new MEConnectionInfoBox.
       */
      MEConnectionInfoBox();

      /**
       *    Delete this MEConnectionInfoBox.
       */
      virtual ~MEConnectionInfoBox();

      /**
       *    Clear all the attribute-fields in the box on the screen.
       *    No attributes in the connection are affected.
       */
      void reset();

      /**
       *    Get the connection that is currently present in the box.
       *    Null will be returned if no connection.
       *    @return The connection that is displayed in the box.
       */
      OldConnection* getConnection();

      /**
       *    Get the integer value for the turndescription that is
       *    displayed in the box. The reason for using an int is
       *    to be able to return error (a negative value).
       *    @param   If positive (or zero), the integer value of 
       *             the enumeration that is printed in this box. 
       *             If negative, no valid turn-direction in the box.
       */
      int getTurnDescription();


      /**
       *    Print the data about a given connection in this box.
       *    @param con      The connection to display information 
       *                    about.
       *    @param name     The name of the street where the connection
       *                    start.
       *    @param toNodeID ID of the node where the connection is 
       *                    stored.
       *    @param costA    The value of the A-cost.
       *    @param costB    The value of the B-cost.
       *    @param costC    The value of the C-cost.
       *    @param costD    The value of the D-cost.
       */
      void addConnectionData(OldConnection* con, const char* name,
                             uint32 toNodeID, uint32 costA, 
                             uint32 costB, uint32 costC, uint32 costD,
                             bool externalConnection,
                             MEMapArea* mapArea);

      void updateVehicleRestrictions(uint32 rest);

      uint32 getVehicleRestrictions();

#ifdef MAP_EDITABLE
      void saveChanges(MELogCommentInterfaceBox* logCommentBox, 
                       OldGenericMap* curMap);
#endif

   private:
      /// my map area
      MEMapArea* m_mapArea;
      
      /**
       *    The connection that is currently displayed. NULL if no
       *    connection is displayed.
       */
      OldConnection* m_curConnection;
      /**
       *    True if m_curConnection is an external connection, false
       *    otherwise.
       */
      bool m_curConnectionExternal;

      uint32 m_toNodeID;

      /**
       *    Where the labels and entry-boxes with attribute values
       *    are stored.
       */
      Gtk::Table* m_nodeTable;

      /**
       *    The ID of the node where the connection leads from.
       */
      Gtk::Entry* m_fromNodeIDVal;

      /**
       *    The turndirection that is used when traversing this
       *    connection.
       */
      Gtk::Entry* m_turnDescVal;

      /**
       *    All possible turn-directions.
       */
      Gtk::Combo* m_turnDescCombo;

      /**
       *    The vehicles that are allowed to traverse this connection.
       */
      Gtk::Entry* m_restrictionVal;

      /**
       *    The name of the street where this connection start.
       */
      Gtk::Entry* m_nameVal;

      /**
       *    The type pf crossing.
       */
      Gtk::Entry* m_crossingkindVal;

      /**
       *    The value of the four costs in the connection.
       */
      Gtk::Entry* m_costVal;

      /**
       *    Method called when the "Edit vehicle restr."-button is pressed.
       */
      void editVehicleRestrictionsPressed();
      
      /**
       *    Method called when the "Sign post info"-button is pressed.
       */
      void signPostInfoPressed();
      /**
       *    Method called when the "Select from node id"-button is pressed.
       */
      void selectFromNodeIDClicked();
};

/**
 *    Abstract class for one tree item (node or connection) in the 
 *    tree that are displayed in the StreetSegmentItemInfoDialog. 
 *
 */
class METreeItem {
   public:
      virtual void update_info () = 0;
};


/**
 *    Represent one mc2-node in the tree that are displayed in the
 *    StreetSegmentItemInfoDialog. Contains information about the
 *    node and will, if clicked, highlight the node on the map.
 *
 */
class METreeItemNode : public METreeItem {
   public:
      /**
       *    Create a new tree-node that represent one mc2-node.
       *    @param name The name of the street where the node is
       *                located.
       *    @param node The node to display.
       *    @param myDialog The dialog where the tree-node will be
       *                    displayed. To be able to e.g. call
       *                    getMapArea().
       */
      METreeItemNode(const char* name,
                   OldNode* node,
                   MENodeInfoBox* nodeInfo,
                   MEMapArea* mapArea);

      virtual void update_info ();

   protected:
      /**
       *    The node that is displayed in this tree-node.
       */
      OldNode* m_node;

      /**
       *    The dialog where the this tree-node is displayed.
       */
      MENodeInfoBox* m_nodeInfo;

      MEMapArea* m_mapArea;

};

/**
 *    Represent one connection in the tree that are displayed in the
 *    StreetSegmentItemInfoDialog. Contains information about the
 *    connection and will, if clicked, highlight the connection on
 *    the map.
 *
 */

class METreeItemConnection : public METreeItem {
   public:
      /**
       *    Create a new node for a connection in the tree.
       *    @param name     The text-representation of the connection.
       *    @param toNode   The node where the connection is stored.
       *    @param conn     The connection.
       *    @param myDialog The dialog where the tree-node will be
       *                    displayed. To be able to e.g. call
       *                    getMapArea().
       *    @param streetName The name of the street where the connection
       *                    starts.
       *    @param externalConnection
       *                    True if conn is an external connection,
       *                    false otherwise.
       */
      METreeItemConnection(const char* name,
                   OldNode* toNode,
                   OldConnection* conn,
                   MENodeInfoBox* nodeInfo,
                   MEConnectionInfoBox* connInfo,
                   MEMapArea* mapArea,
                   const char* streetName,
                   bool externalConnection);

       virtual void update_info ();

   protected:
      /**
       *    The connection that is represented in this tree-node.
       */
      OldConnection* m_connection;

      /**
       *    The node where the connection is stored.
       */
      OldNode* m_toNode;

      /**
       *    The name of the street where the connection starts.
       */
      const char* m_streetName;

      /**
       *    The dialog where the this tree-node is displayed.
       */
      MENodeInfoBox* m_nodeInfo;
      MEConnectionInfoBox* m_connectionInfo;
      MEMapArea* m_mapArea;


      /**
       *    True if m_connection is an external connection, false
       *    otherwise.
       */
      bool m_externalConnection;

};

#endif
