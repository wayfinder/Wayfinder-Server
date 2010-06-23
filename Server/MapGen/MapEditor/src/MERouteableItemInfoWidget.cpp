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
#include "MERouteableItemInfoWidget.h"
#include "OldRouteableItem.h"
#include "MEItemInfoDialog.h"
#include "METurnRestrictionsDialog.h" 
#include "MESignPostInfoDialog.h"
#include "MEMapArea.h"
#include "OldExternalConnections.h"
#include "OldExtraDataUtility.h"
#include <list>
#include <gtkmm/separator.h>
#include <gtkmm/label.h>
#include <gtkmm/main.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm.h>
#include "MEFeatureIdentification.h"
#include "MELogFilePrinter.h"
#include "OldNode.h"
#include <vector>

MERouteableItemInfoWidget::MERouteableItemInfoWidget()
    : MEAbstractItemInfoWidget("Routeable")
{   
   Gtk::Box* box = manage( new Gtk::VBox() ); 

   Gtk::ScrolledWindow* scrolledWin = manage(new Gtk::ScrolledWindow());
   scrolledWin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrolledWin->set_size_request(300, 75);

   m_TreeStore = Gtk::TreeStore::create(m_Columns);
   m_TreeView.set_model(m_TreeStore);
   m_TreeView.append_column("REMOVE ME", m_Columns.m_info);
   m_TreeView.set_headers_visible(false);

   // Enable selections.
   m_Selection = m_TreeView.get_selection();
   if(m_Selection)
   {
      m_Selection->signal_changed().connect(
        sigc::mem_fun(*this, 
              &MERouteableItemInfoWidget::on_routable_treeview_row_activated));
   } else {
      // If this doesn't work we're in trouble.
      mc2log << error << "No selection object created for corresponding "
             << "TreeView" << endl;
      MC2_ASSERT(false);
   }

   scrolledWin->add(m_TreeView); 

   box->pack_start(*scrolledWin);

   // Create the node and connection information boxes
   m_nodeInfoBox = manage(new MENodeInfoBox());
   box->pack_start(*m_nodeInfoBox, false, false);
   Gtk::HSeparator* sep = manage(new Gtk::HSeparator());
   box->pack_start(*sep, false, false);
   m_connectionInfoBox = manage(new MEConnectionInfoBox());
   box->pack_start(*m_connectionInfoBox, false, false);
   // Add "box" to this frame
   add(*box);
}

MERouteableItemInfoWidget::~MERouteableItemInfoWidget()
{

}

void
MERouteableItemInfoWidget::activate(
               MEMapArea* mapArea, OldRouteableItem* routeable)
{
   
   MEAbstractItemInfoWidget::activate(mapArea, routeable);
   // Create the nodes
   m_nodeInfoBox->reset();
   m_connectionInfoBox->reset();
   m_TreeStore->clear();

   char tmpstr[128];
   Gtk::TreeModel::Row row;
   Gtk::TreeModel::Row childrow;

   for (uint32 nodeNbr=0; nodeNbr<2; nodeNbr++) {
      OldNode* currentNode = routeable->getNode(nodeNbr);
      uint32 nodeID = currentNode->getNodeID();
      // Create the main entry (from node 0 or node 1)
      sprintf(tmpstr, "Node %d, id=%u", 
                      nodeNbr, nodeID);
      METreeItem* treeItemNode = new METreeItemNode(tmpstr,
                                                    currentNode,
                                                    m_nodeInfoBox,
                                                    m_mapArea);

  
      row = *(m_TreeStore->append());
      row[m_Columns.m_info] = tmpstr;
      row[m_Columns.m_treeItem] = treeItemNode; 

      // Connections to this node
      for (uint32 i=0; i<currentNode->getNbrConnections(); i++) {
         OldConnection* conn = currentNode->getEntryConnection(i);

         if ( conn->isMultiConnection() ) {
            list<uint32> nodeIDs;
            nodeIDs.push_back( currentNode->getNodeID() );
            nodeIDs.push_back( conn->getConnectFromNode() );
            m_mapArea->getMap()->expandNodeIDs( nodeIDs );
            mc2log << info << "Multi connection passing the following nodes:"
                   << endl;
            copy( nodeIDs.begin(), nodeIDs.end(), 
                  ostream_iterator<uint32> ( mc2log, " " ) );
         }

         // Get the turndirection-string and remove "then"
         const char* turnDir = 
            StringUtility::copyUpper(StringTable::getString(
                  ItemTypes::getTurndirectionSC(conn->getTurnDirection()),
                  MEItemInfoDialog::m_language)).c_str();
         #define REPLACE_STR "THEN "
         const char* thenStr = strstr(turnDir, REPLACE_STR);
         if (thenStr != NULL) {
            turnDir += strlen(REPLACE_STR);
         }
         // In co map this might be an incorrect name!!! (name of item id 0)
         const char* streetName = 
                 m_mapArea->getMap()->getFirstItemName(
                    conn->getFromNode() & MAX_INT32);
         sprintf(tmpstr, "- %s from %s",
                 turnDir,
                 streetName);
         METreeItem* treeItemConnection = new METreeItemConnection(
                                                tmpstr,
                                                currentNode,
                                                conn,
                                                m_nodeInfoBox,
                                                m_connectionInfoBox,
                                                m_mapArea,
                                                streetName,
                                                false); // Not external conn


         childrow = *(m_TreeStore->append(row.children()));         
         childrow[m_Columns.m_info] = tmpstr;
         childrow[m_Columns.m_treeItem] = treeItemConnection; 
      }
   
      // External connections?
      // Get the boundry segments.
      
      OldBoundrySegmentsVector* bvec =
         m_mapArea->getMap()->getBoundrySegments();
      if (bvec != NULL) {
         OldBoundrySegment* bseg =
            bvec->getBoundrySegment(
               currentNode->getNodeID() & MAX_INT32);
         if ( bseg != NULL ) {
            int nbrConnsToNode = bseg->getNbrConnectionsToNode(nodeNbr);
            for(int k=0; k < nbrConnsToNode; ++k) {
               OldConnection* conn = bseg->getConnectToNode(nodeNbr, k);
               uint32 fromMapID = bseg->getFromMapIDToNode(nodeNbr, k);
               sprintf(tmpstr, "- EXTERNAL connection from %u.%u, "
                               "restr 0x%x",
                       fromMapID,
                       conn->getFromNode(),
                       conn->getVehicleRestrictions());
               
                 METreeItem* treeItemConnection = new METreeItemConnection(
                                                         tmpstr,   
                                                         currentNode, 
                                                         conn,
                                                         m_nodeInfoBox,
                                                         m_connectionInfoBox,
                                                         m_mapArea, 
                                                         "", 
                                                         true); 
                                                     // Is external connection

                 childrow = *(m_TreeStore->append(row.children()));
                 childrow[m_Columns.m_info] = tmpstr;
                 childrow[m_Columns.m_treeItem] = 
                     treeItemConnection;
            }
         }
      }
   }
   show();
}

void
MERouteableItemInfoWidget::on_routable_treeview_row_activated()
{
   Gtk::TreeModel::iterator iter = m_Selection->get_selected();
   if(iter)
   {
      Gtk::TreeModel::Row row = *iter;
      METreeItem* tmpItem = row[m_Columns.m_treeItem]; 
      tmpItem->update_info();
   }
}


#ifdef MAP_EDITABLE
void
MERouteableItemInfoWidget::saveChanges(
      MELogCommentInterfaceBox* logCommentBox)
{
   if (m_nodeInfoBox != NULL) {
      m_nodeInfoBox->saveChanges(logCommentBox, m_mapArea->getMap());
   }
   if (m_connectionInfoBox != NULL) {
      m_connectionInfoBox->saveChanges(logCommentBox, m_mapArea->getMap());
   }
}
#endif // MAP_EDITABLE


// ========================================================================
//                                                T r e e I t e m N o d e =

METreeItemNode::METreeItemNode(const char* name,
                           OldNode* node,
                           MENodeInfoBox* nodeInfo,
                           MEMapArea* mapArea)
{
   m_node = node;
   m_nodeInfo = nodeInfo;
   m_mapArea = mapArea;
}

void
METreeItemNode::update_info ()
{
   mc2dbg8 << "SELECTED node " << endl;
   m_mapArea->highlightConnection(m_node->getNodeID(), MAX_UINT32);
   m_nodeInfo->addNodeData(m_node);
   mc2dbg << "node, nbr conns=" << m_node->getNbrConnections() << endl;

   // Lanes printing
   vector<GMSLane> lanes = m_mapArea->getLanes(*m_node);
   mc2log << info << "Printing lane info of:" << m_node->getNodeID()
          << " size:" << lanes.size() << endl;
   for (uint32 laneIdx=0; laneIdx<lanes.size(); laneIdx++){
      mc2log << info << "   " << laneIdx << ": " << lanes[laneIdx] << endl;
   }
}



// ========================================================================
//                                    T r e e I t e m C o n n e c t i o n =

METreeItemConnection::METreeItemConnection(
                           const char* name,
                           OldNode* toNode,
                           OldConnection* conn,
                           MENodeInfoBox* nodeInfo,
                           MEConnectionInfoBox* connInfo,
                           MEMapArea* mapArea,
                           const char* streetName,
                           bool externalConnection)
   :  m_streetName(streetName)
{
   m_toNode = toNode;
   m_connection = conn;
   m_nodeInfo = nodeInfo;
   m_connectionInfo = connInfo;
   m_mapArea = mapArea;
   m_externalConnection = externalConnection;
}

void
METreeItemConnection::update_info ()
{
   if ( m_externalConnection ) {
      // if external: don't highlight the connection-fromNode
      m_mapArea->highlightConnection(m_toNode->getNodeID(), MAX_UINT32);
   } else {
      m_mapArea->highlightConnection(m_toNode->getNodeID(),
                                     m_connection->getFromNode());
   }
   m_nodeInfo->addNodeData(m_toNode);
   // Get the connection costs
   uint32 costA, costB, costC, costD;
   m_mapArea->getConnectionCosts(m_connection, m_toNode,
                                 costA, costB, costC, costD,
                                 m_externalConnection);
   m_connectionInfo->addConnectionData(m_connection,
                                       m_streetName,
                                       m_toNode->getNodeID(),
                                       costA, costB, costC, costD,
                                       m_externalConnection,
                                       m_mapArea);

   // Sign posts printing
   mc2log << info << "Printing sign posts:" << endl;
   m_mapArea->printSignPosts(*m_toNode, *m_connection);


   // Lanes printing
   uint32 connectingLanes =
      m_mapArea->getConnectingLanes(*m_toNode, *m_connection);
   if ( connectingLanes != MAX_UINT32 ) {
      mc2log << info << "Connecting lanes from "
             << m_connection->getConnectFromNode() << ": "
             << MapGenUtil::intToBitFieldStr(connectingLanes, 32) << endl;
   }
   // Create the sign post dialog for this connection with map area
   // to enable for signpost editing.
   // (dialog is showed with "Sign post info"-button in MEConnectionInfoBox)
#ifdef MAP_EDITABLE
   MESignPostInfoDialog::instance(
         m_connection, m_toNode->getNodeID(), m_mapArea);
#else
   // no map area - no editing
   MESignPostInfoDialog::instance(
      m_connection, m_toNode->getNodeID(), NULL);
#endif
}


// ========================================================================
//                                                  N o d e I n f o B o x =

MENodeInfoBox::MENodeInfoBox()
{
   Gtk::Frame* frame = new Gtk::Frame("Node information");
   pack_start(*frame, false, false);

   m_nodeTable = manage(new Gtk::Table(10, 4, false)); // 10 rows 4 cols
   frame->add(*m_nodeTable);

   Gtk::Label* label = NULL;
   uint32 i = 0; // rowNbr

   // Node ID
   label = new Gtk::Label("Node ID");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 0, 1, i, i+1, Gtk::FILL, Gtk::FILL);
   m_nodeIDVal = new Gtk::Entry();
   m_nodeTable->attach(*m_nodeIDVal, 1, 4, i, i+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);
   i++;

   // restrictions
   label = new Gtk::Label("Entry restrictions");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 0, 1, i, i+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::FILL);
   m_restrictionsCombo = new Gtk::Combo();
   m_restrictionsVal = m_restrictionsCombo->get_entry();
   m_nodeTable->attach(*m_restrictionsCombo, 1, 4, i, i+1,
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);
   i++;

   // Speedlimit
   label = new Gtk::Label("Speed limit");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 0, 1, i, i+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::FILL);
   m_speedlimitVal = new Gtk::Entry();
   m_speedlimitVal->set_size_request(60, 20);
   m_nodeTable->attach(*m_speedlimitVal, 1, 2, i, i+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);
   // Junction type
   label = new Gtk::Label(" Junction type ");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 2, 3, i, i+1, Gtk::FILL, Gtk::FILL);
   m_junctionTypeVal = new Gtk::Entry();
   m_junctionTypeVal->set_size_request(60, 20);
   m_nodeTable->attach(*m_junctionTypeVal, 3, 4, i, i+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);
   i++;

   // Level
   label = new Gtk::Label("Level");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 0, 1, i, i+1, Gtk::FILL, Gtk::FILL);
   m_levelVal = new Gtk::Entry();
   m_levelVal->set_size_request(60, 20);
   m_nodeTable->attach(*m_levelVal, 1, 2, i, i+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);
   // Major road
   label = new Gtk::Label(" Major road ");
   label->set_alignment(XALIGN, YALIGN);
   //m_nodeTable->attach(*label, 2, 3, i, i+1, FIXED_OPT, FIXED_OPT);
   m_nodeTable->attach(*label, 2, 3, i, i+1, Gtk::FILL, Gtk::FILL);
   m_majorRoadVal = new Gtk::Entry();
   m_majorRoadVal->set_size_request(60, 20);
   m_nodeTable->attach(*m_majorRoadVal, 3, 4, i, i+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);   
   i++;

   // Maximum weight
   label = new Gtk::Label("Max weight");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 0, 1, i, i+1, Gtk::FILL, Gtk::FILL);
   m_maxWeightVal = new Gtk::Entry();
   m_maxWeightVal->set_size_request(60, 20);
   m_nodeTable->attach(*m_maxWeightVal, 1, 2, i, i+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);   // Maximum height
   label = new Gtk::Label(" Max height ");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 2, 3, i, i+1, Gtk::FILL, Gtk::FILL);
   m_maxHeightVal = new Gtk::Entry();
   m_maxHeightVal->set_size_request(60, 20);
   m_nodeTable->attach(*m_maxHeightVal, 3, 4, i, i+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);   
   i++;

   // Number of lanes
   label = new Gtk::Label("Nbr lanes");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 0, 1, i, i+1, Gtk::FILL, Gtk::FILL);
   m_nbrLanesVal = new Gtk::Entry();
   m_nbrLanesVal->set_size_request(60, 20);
   m_nodeTable->attach(*m_nbrLanesVal, 1, 2, i, i+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);
   // Has road toll?
   m_roadTollVal = manage(new Gtk::CheckButton("Toll road"));
   label = dynamic_cast<Gtk::Label*>(m_roadTollVal->get_child());
   if(label != NULL)
      label->set_alignment(XALIGN, 0.5);
   //box->pack_start(*m_roundaboutVal);
   m_nodeTable->attach(*m_roadTollVal, 3, 4, i, i+1, Gtk::EXPAND, Gtk::EXPAND);

   m_node = NULL;
   m_roadTollVal->set_active(false);

   //m_nodeIDVal->set_state(GTK_STATE_INSENSITIVE);
   m_nodeIDVal->set_editable(false);
   //m_junctionTypeVal->set_state(GTK_STATE_INSENSITIVE);
   m_junctionTypeVal->set_state(Gtk::STATE_INSENSITIVE);
   //m_majorRoadVal->set_state(GTK_STATE_INSENSITIVE);
   m_majorRoadVal->set_state(Gtk::STATE_INSENSITIVE);
   //m_maxWeightVal->set_state(GTK_STATE_INSENSITIVE);
   m_maxWeightVal->set_state(Gtk::STATE_INSENSITIVE);
   //m_maxHeightVal->set_state(GTK_STATE_INSENSITIVE);
   m_maxHeightVal->set_state(Gtk::STATE_INSENSITIVE);
   //m_nbrLanesVal->set_state(GTK_STATE_INSENSITIVE);
   m_nbrLanesVal->set_state(Gtk::STATE_INSENSITIVE);

#ifdef MAP_EDITABLE
   //m_restrictionsVal->set_state(GTK_STATE_NORMAL);
   m_restrictionsVal->set_state(Gtk::STATE_NORMAL);
   //m_speedlimitVal->set_state(GTK_STATE_NORMAL);
   m_speedlimitVal->set_state(Gtk::STATE_NORMAL);
   //m_levelVal->set_state(GTK_STATE_NORMAL);
   m_levelVal->set_state(Gtk::STATE_NORMAL);
   //m_roadTollVal->set_state(GTK_STATE_NORMAL);
   m_roadTollVal->set_state(Gtk::STATE_NORMAL);
#else
   //m_restrictionsVal->set_state(GTK_STATE_INSENSITIVE);
   m_restrictionsVal->set_state(Gtk::STATE_INSENSITIVE);
   //m_speedlimitVal->set_state(GTK_STATE_INSENSITIVE);
   m_speedlimitVal->set_state(Gtk::STATE_INSENSITIVE);
   //m_levelVal->set_state(GTK_STATE_INSENSITIVE);
   m_levelVal->set_state(Gtk::STATE_INSENSITIVE);
   //m_roadTollVal->set_state(GTK_STATE_INSENSITIVE);
   m_roadTollVal->set_state(Gtk::STATE_INSENSITIVE);
#endif

   show_all();
}

MENodeInfoBox::~MENodeInfoBox()
{
   // Nohting to do in destructor
}

void
MENodeInfoBox::addNodeData(OldNode* node)
{
   m_node = node;
   char tmpStr[128];

   // ID
   sprintf(tmpStr, "%u = 0x%x", node->getNodeID(),
                              node->getNodeID());
   m_nodeIDVal->set_text(tmpStr);

   // restrictions
#ifdef MAP_EDITABLE
   int pos = ItemTypes::getFirstEntryRestrictionPosition();
   list<string> list;
   while (pos >= 0) {
      StringTable::stringCode er =
         ItemTypes::getIncEntryRestrictionSC(pos);
      const char* str =
      StringTable::getString(er, MEItemInfoDialog::m_language);
      list.push_back(str);
   }
   m_restrictionsCombo->set_popdown_strings(list);
#endif
   m_restrictionsVal->set_text(
      StringTable::getString(
         ItemTypes::getEntryRestrictionSC(
            node->getEntryRestrictions()),
            MEItemInfoDialog::m_language)
   );

   // Speed limit
   sprintf(tmpStr, "%u", node->getSpeedLimit());
   m_speedlimitVal->set_text(tmpStr);

   // Junction type
   switch ( node->getJunctionType() ) {
      case ItemTypes::normalCrossing :
         sprintf(tmpStr, "%s", "normal Xing");
         break;
      case ItemTypes::bifurcation :
         sprintf(tmpStr, "%s", "bifurc");
         break;
      case ItemTypes::railwayCrossing :
         sprintf(tmpStr, "%s", "railw Xing");
         break;
      case ItemTypes::borderCrossing :
         sprintf(tmpStr, "%s", "border Xing");
         break;
      default :
         sprintf(tmpStr, "%u %s", node->getJunctionType(), "unknown");
   }
   //sprintf(tmpStr, "%u", node->getJunctionType());
   m_junctionTypeVal->set_text(tmpStr);

   // Level
   sprintf(tmpStr, "%i", node->getLevel());
   m_levelVal->set_text(tmpStr);

   // Major road?
   if (node->isMajorRoad()) {
      m_majorRoadVal->set_text("true");
   } else {
      m_majorRoadVal->set_text("false");
   }

   // Max weight
   sprintf(tmpStr, "%u tons", node->getMaximumWeight());
   m_maxWeightVal->set_text(tmpStr);

   // Max height
   sprintf(tmpStr, "%u dm", node->getMaximumHeight());
   m_maxHeightVal->set_text(tmpStr);

   // Nbr lanes
   //sprintf(tmpStr, "%u", node->getNbrLanes());
   m_nbrLanesVal->set_text("Not implemented");

   // Toll road
   m_roadTollVal->set_active(node->hasRoadToll());
}

void
MENodeInfoBox::reset()
{
   m_node = NULL;
   m_nodeIDVal->set_text("");
   m_restrictionsVal->set_text("");
   m_speedlimitVal->set_text("");
   m_junctionTypeVal->set_text("");
   m_levelVal->set_text("");
   m_majorRoadVal->set_text("");
   m_maxWeightVal->set_text("");
   m_maxHeightVal->set_text("");
   m_nbrLanesVal->set_text("");
   m_roadTollVal->set_active(false);
}


uint32
MENodeInfoBox::getNodeID()
{
   if (m_node != NULL)
      return (m_node->getNodeID());
   return (MAX_UINT32);
}

int
MENodeInfoBox::getEntryRestrictions()
{
   const char* str = m_restrictionsVal->get_text().c_str();
   return ItemTypes::getEntryRestriction(
               str, MEItemInfoDialog::m_language);
}

int
MENodeInfoBox::getSpeedLimit()
{
   const char* str = m_speedlimitVal->get_text().c_str();
   if ( (str != NULL) && (strlen(str) > 0)) {
      int x = strtol(str, NULL, 10);
      return (x);
   }
   return -1;
}

int
MENodeInfoBox::getLevel()
{
   const char* str = m_levelVal->get_text().c_str();
   if ( (str != NULL) && (strlen(str) > 0)) {
      int x = strtol(str, NULL, 10);
      mc2dbg4 << "   getLevel(): str=" << str << ", x=" << x << endl;
      return (x);
   }
   return MAX_INT32;
}

#ifdef MAP_EDITABLE
void
MENodeInfoBox::saveChanges(MELogCommentInterfaceBox* logCommentBox,
                           OldGenericMap* curMap)
{
   if ( m_node == NULL ) {
      // no node marked in Routeable nodeTree
      return;
   }

   // For log comment string
   char origValStr[64];

   // New value strings, holding sub strings for the last part of the extra
   // data record
   vector<MC2String> newValStrings;
   char tmpStr[256];

   // Get identification string
   MC2String identString;
   bool identOK = MEFeatureIdentification::getNodeIdentificationString(
                     identString, curMap, m_node);
   if ( !identOK ) {
      mc2log << warn << " Failed to get node identification string" << endl;
      return;
   }

   // Entry restrictions
   // First get the value of the restrictions entry
   int x = getEntryRestrictions();
   if (x >= 0) {
      ItemTypes::entryrestriction_t newRestr = ItemTypes::entryrestriction_t(x);      ItemTypes::entryrestriction_t curRestr = m_node->getEntryRestrictions();
      if (newRestr != curRestr) {
         // the entry restr has changed
         sprintf(origValStr, "%s", StringTable::getString(
                  ItemTypes::getEntryRestrictionSC(curRestr),
                  MEItemInfoDialog::m_language) );
         vector<MC2String> logStrings =
               logCommentBox->getLogComments(origValStr);

         m_node->setEntryRestrictions(newRestr);
         sprintf( tmpStr, "%s", StringTable::getString(
                     ItemTypes::getEntryRestrictionSC(newRestr),
                     MEItemInfoDialog::m_language) );
         newValStrings.clear();
         newValStrings.push_back( tmpStr );

         MELogFilePrinter::print(
               logStrings, "setEntryRestrictions",
               identString, newValStrings );
         mc2dbg << "Entry restriction set to: " << tmpStr << endl;

         // If this node is on a routeable item connected to a virtual item
         // transfer the correction also to the virtual.
         vector<uint32> changedVirtuals;
         OldExtraDataUtility::transferChangeToVirtualNode(
                  curMap, m_node->getNodeID(), x,
                  OldExtraDataUtility::SET_ENTRYRESTRICTIONS,
                  changedVirtuals);
      }
   } else {
      mc2log << error << "Invalid entryrestrictions: " << x << endl;
   }

   // Speed limit
   x = getSpeedLimit();
   if ( (x >= 0) && (x != m_node->getSpeedLimit())) {
      sprintf(origValStr, "%d", m_node->getSpeedLimit());
      vector<MC2String> logStrings = logCommentBox->getLogComments(origValStr);

      m_node->setSpeedLimit(x);
      sprintf( tmpStr, "%d", x );
      newValStrings.clear();
      newValStrings.push_back( tmpStr );

      MELogFilePrinter::print(
            logStrings, "setSpeedLimit", identString, newValStrings );
      mc2dbg << "Speedlimit set to: " << x << endl;

      // If this node is on a routeable item connected to a virtual item
      // transfer the correction also to the virtual.
      vector<uint32> changedVirtuals;
      OldExtraDataUtility::transferChangeToVirtualNode(
               curMap, m_node->getNodeID(), x,
               OldExtraDataUtility::SET_SPEEDLIMIT,
               changedVirtuals);

   } else if  (x < 0) {
      mc2dbg1 << "Speed limit must be >= 0, tried with " << x << endl;
   }

   // Node level
   x = getLevel();
   if ( (x != MAX_INT32) && (x != m_node->getLevel())) {
      sprintf(origValStr, "%d", m_node->getLevel());
      vector<MC2String> logStrings = logCommentBox->getLogComments(origValStr);

      m_node->setLevel(x);
      sprintf( tmpStr, "%d", x );
      newValStrings.clear();
      newValStrings.push_back( tmpStr );

      MELogFilePrinter::print(
            logStrings, "setLevel", identString, newValStrings );
      mc2dbg << "Level set to: " << x << endl;
   }

   // Road toll
   if (m_roadTollVal->get_active() != m_node->hasRoadToll()) {
      sprintf( origValStr, "%s",
               StringUtility::booleanAsString(m_node->hasRoadToll()) );
      vector<MC2String> logStrings =
         logCommentBox->getLogComments(origValStr);
      bool newVal = m_roadTollVal->get_active();
      m_node->setRoadToll(newVal);
      newValStrings.clear();
      newValStrings.push_back( StringUtility::booleanAsString(newVal) );

      MELogFilePrinter::print(
            logStrings, "setTollRoad", identString, newValStrings );
      mc2dbg << "Road toll set to: "
             << StringUtility::booleanAsString(newVal)
             << " for node " << m_node->getNodeID() << endl;

      // If this node is on a routeable item connected to a virtual item
      // transfer the correction also to the virtual.
      vector<uint32> changedVirtuals;
      OldExtraDataUtility::transferChangeToVirtualNode(
               curMap, m_node->getNodeID(), x,
               OldExtraDataUtility::SET_TOLLROAD,
               changedVirtuals);
   }
}
#endif

// ========================================================================
//                                      C o n n e c t i o n I n f o B o x =

MEConnectionInfoBox::MEConnectionInfoBox() 
{
   Gtk::Frame* frame = new Gtk::Frame("Connection information");
   pack_start(*frame, false, false);
   
   m_nodeTable = manage(new Gtk::Table(2, 4, false)); // row,cols
   frame->add(*m_nodeTable);
   uint32 curRow = 0;

   Gtk::Label* label = NULL;

   // From node ID
   label = new Gtk::Label("From node ID");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 0, 2, curRow, curRow+1, Gtk::FILL, Gtk::FILL);
   m_fromNodeIDVal = new Gtk::Entry();
   m_nodeTable->attach(*m_fromNodeIDVal, 2, 3, curRow, curRow+1, 
		       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);
   Gtk::Button* tmpButton1=manage(new Gtk::Button("Select"));
   tmpButton1->signal_clicked().connect(
         sigc::mem_fun(*this, &MEConnectionInfoBox::selectFromNodeIDClicked));
   m_nodeTable->attach(*tmpButton1, 3, 4, curRow, curRow+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);
   curRow++;

   // Turn description
   label = new Gtk::Label("Turn description");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 0, 2, curRow, curRow+1, Gtk::FILL, Gtk::FILL);
   m_turnDescCombo = new Gtk::Combo();
   m_turnDescVal = m_turnDescCombo->get_entry();
   m_nodeTable->attach(*m_turnDescCombo, 2, 4, curRow, curRow+1, 
		       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);
   curRow++;

   // Vehicle restrictions
#ifdef MAP_EDITABLE
   // Add a button to be able to edit the vehicle restrictions of this conn
   Gtk::Button* tmpButton=manage(new Gtk::Button("Edit vehicle restr."));
   tmpButton->set_size_request(20,12);
   tmpButton->signal_clicked().connect(
         sigc::mem_fun(*this, &MEConnectionInfoBox::editVehicleRestrictionsPressed));
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*tmpButton, 0, 2, curRow, curRow+1, 
                       Gtk::FILL, Gtk::FILL);
#else
   label = new Gtk::Label("Vehicle restrictions");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 0, 2, curRow, curRow+1, 
                       Gtk::FILL, Gtk::FILL);
#endif
   m_restrictionVal = new Gtk::Entry();
   m_nodeTable->attach(*m_restrictionVal, 2, 3, curRow, curRow+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);

   // Signposts
   tmpButton=manage(new Gtk::Button("Sign post info"));
   tmpButton->set_size_request(90,12);
   tmpButton->signal_clicked().connect(
         sigc::mem_fun(*this, &MEConnectionInfoBox::signPostInfoPressed));
   m_nodeTable->attach(*tmpButton, 3, 4, curRow, curRow+1, 
		       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::FILL | Gtk::EXPAND);
   curRow++;

   // Turn to name
   label = new Gtk::Label("From street name");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 0, 2, curRow, curRow+1, Gtk::FILL, Gtk::FILL);
   m_nameVal = new Gtk::Entry();
   m_nodeTable->attach(*m_nameVal, 2, 4, curRow, curRow+1, 
		       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);
   curRow++;

   // Crossingkind
   label = new Gtk::Label("Crossingkind (exit cnt)");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 0, 2, curRow, curRow+1, Gtk::FILL, Gtk::FILL);
   m_crossingkindVal = new Gtk::Entry();
   m_nodeTable->attach(*m_crossingkindVal, 2, 4, curRow, curRow+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);
   curRow++;
   // Connection cost
   label = new Gtk::Label("Cost (A:B:C:D)");
   label->set_alignment(XALIGN, YALIGN);
   m_nodeTable->attach(*label, 0, 2, curRow, curRow+1, Gtk::FILL, Gtk::FILL);
   m_costVal = new Gtk::Entry();
   m_nodeTable->attach(*m_costVal, 2, 4, curRow, curRow+1, 
                       Gtk::FILL | Gtk::EXPAND, 
                       Gtk::EXPAND);
   curRow++;

   //m_fromNodeIDVal->set_state(GTK_STATE_INSENSITIVE);
   
   m_fromNodeIDVal->set_editable(false);
   //m_restrictionVal->set_state(GTK_STATE_INSENSITIVE);
   m_restrictionVal->set_state(Gtk::STATE_INSENSITIVE);
   //m_nameVal->set_state(GTK_STATE_INSENSITIVE);
   m_nameVal->set_state(Gtk::STATE_INSENSITIVE);
   //m_crossingkindVal->set_state(GTK_STATE_INSENSITIVE);
   m_crossingkindVal->set_state(Gtk::STATE_INSENSITIVE);
   //m_costVal->set_state(GTK_STATE_INSENSITIVE);
   m_costVal->set_state(Gtk::STATE_INSENSITIVE);
#ifdef MAP_EDITABLE
   //m_turnDescVal->set_state(GTK_STATE_NORMAL);
   m_turnDescVal->set_state(Gtk::STATE_NORMAL);
#else
   //m_turnDescVal->set_state(GTK_STATE_INSENSITIVE);
   m_turnDescVal->set_state(Gtk::STATE_INSENSITIVE);
#endif
}



MEConnectionInfoBox::~MEConnectionInfoBox() 
{
   // Nothing to do
}

void
MEConnectionInfoBox::reset() 
{
   m_fromNodeIDVal->set_text("");
   m_turnDescVal->set_text("");
   m_restrictionVal->set_text("");
   m_nameVal->set_text("");
   m_crossingkindVal->set_text("");
   m_costVal->set_text("");
   m_curConnection = NULL;
   m_curConnectionExternal = false;
}

OldConnection*
MEConnectionInfoBox::getConnection() 
{
   return m_curConnection;
}

int
MEConnectionInfoBox::getTurnDescription() 
{
   const char* str = m_turnDescVal->get_text().c_str();
   return ItemTypes::getTurnDirection(
               str, MEItemInfoDialog::m_language);
}

void
MEConnectionInfoBox::addConnectionData(OldConnection* con, 
                                     const char* name,
                                     uint32 toNodeID, 
                                     uint32 costA, uint32 costB, 
                                     uint32 costC, uint32 costD,
                                     bool externalConnection,
                                     MEMapArea* mapArea)
{
   char tmpStr[128];

   m_mapArea = mapArea;

   // The connection (to be able to save it)
   m_curConnection = con;
   m_curConnectionExternal = externalConnection;

   // The node to be able to get coordinates for it
   m_toNodeID = toNodeID;

   // ID
   MC2String extConnStr = "";
   if (externalConnection) {
      extConnStr = "ext conn ";
   }
   sprintf(tmpStr, "%s%u = 0x%x", 
           extConnStr.c_str(),
           con->getConnectFromNode(), 
           con->getConnectFromNode());
   m_fromNodeIDVal->set_text(tmpStr);

   // restrictions
   sprintf(tmpStr, "0x%x", con->getVehicleRestrictions());
   m_restrictionVal->set_text(tmpStr);

   // Turn description
#ifdef MAP_EDITABLE
   int pos = ItemTypes::getFirstTurnDirectionPosition();
   list<string> list;
   while (pos >= 0) {
      StringTable::stringCode td = 
         ItemTypes::getIncTurnDirectionSC(pos);
      const char* str = 
      StringTable::getString(td, MEItemInfoDialog::m_language);
      list.push_back(str);
   }
   m_turnDescCombo->set_popdown_strings(list);
#endif
   m_turnDescVal->set_text(
      StringTable::getString(
         ItemTypes::getTurndirectionSC(
            con->getTurnDirection()),
            MEItemInfoDialog::m_language)
   );

   // Name
   m_nameVal->set_text(name);

   // Crossingkind
   sprintf(tmpStr, "%s (%u)", 
           StringTable::getString(
              ItemTypes::getCrossingKindSC(con->getCrossingKind()),
              MEItemInfoDialog::m_language),
           con->getExitCount());
   m_crossingkindVal->set_text(tmpStr);

   // Connection costs
   sprintf(tmpStr, "%u:%u:%u:%u", costA, costB, costC, costD);
   m_costVal->set_text(tmpStr);

}

void
MEConnectionInfoBox::updateVehicleRestrictions(uint32 rest)
{
   char tmpStr[32];
   sprintf(tmpStr, "0x%x", rest);
   m_restrictionVal->set_text(tmpStr);
}

uint32
MEConnectionInfoBox::getVehicleRestrictions()
{
   const char* str = m_restrictionVal->get_text().c_str();
   if ( (str != NULL) && (strlen(str) > 0)) {
      int x = strtoul(str, NULL, 16);
      mc2dbg4 << "   getVehicleRestrictions(): str=" << str << ", x=0x" 
              << hex << x << dec << endl;
      return (x);
   }
   return MAX_UINT32;
}


void
MEConnectionInfoBox::editVehicleRestrictionsPressed()
{
   if (m_curConnection != NULL) {
      METurnRestrictionsDialog::instance()->show(
                     getVehicleRestrictions(), this);
   }
}

void
MEConnectionInfoBox::signPostInfoPressed() 
{
   if (m_curConnection != NULL) {
      // show the sign post dialog
      MESignPostInfoDialog::instance(m_curConnection, m_toNodeID)->show();
   }
}

void
MEConnectionInfoBox::selectFromNodeIDClicked()
{
   mc2dbg4 << "MEConnectionInfoBox::selectFromNodeIDClicked" << endl;
   if (m_curConnection == NULL) {
      cout << "selectFromNodeIDClicked: no connection selected" << endl;
   } else {
      mc2dbg8 << "from node = " << m_curConnection->getFromNode() << endl;
      if ( m_curConnectionExternal ) {
         cout << "selectFromNodeIDClicked: connection from "
              << m_curConnection->getFromNode()
              << " is external - no highlight!" << endl;
      }
      else {
         // do stuff

         bool clearFirst = true;
         uint32 itemID = REMOVE_UINT32_MSB( m_curConnection->getFromNode() );
         if (!m_mapArea->highlightItem(itemID, clearFirst)) {
            mc2log << warn << here << " FAILED, did not find item with ID = " 
                   << itemID << " among the drawn item types" << endl;
         }

         OldItem* item = m_mapArea->getMap()->itemLookup(itemID);
         if (item != NULL) {
            // Show item-info, SSI is a bit special...
            MEItemInfoDialog* dialog = MEItemInfoDialog::instance(m_mapArea);
            dialog->setItemToShow(item, m_mapArea->getMap());
         }
      }
   }
}

#ifdef MAP_EDITABLE
void
MEConnectionInfoBox::saveChanges(MELogCommentInterfaceBox* logCommentBox,
                                 OldGenericMap* curMap)
{
   if (m_curConnection == NULL) {
      // no connection marked in Routeable nodeTree
      return;
   }

   OldNode* toNode = curMap->nodeLookup(m_toNodeID);
   if ( toNode == NULL ) {
      mc2log << error << here << "Failed to lookup toNode" << endl;
      return;
   }
   
   // For log comment string
   char origValStr[64];

   // New value strings, holding sub strings for the last part of the extra 
   // data record
   vector<MC2String> newValStrings;
   char tmpStr[256];

   // Get identification string
   MC2String identString;
   bool identOK = MEFeatureIdentification::getConnectionIdentificationString(
                     identString, curMap, m_curConnection, toNode );
   if ( !identOK ) {
      mc2log << warn << " Failed to get conn identification string" << endl;
      return;
   }


   // Turndescription
   int newTurnDescVal = getTurnDescription();
   if (newTurnDescVal >= 0) {
      ItemTypes::turndirection_t newTurnDesc = 
         ItemTypes::turndirection_t(newTurnDescVal);
      ItemTypes::turndirection_t curTurnDesc = 
         m_curConnection->getTurnDirection();
      if ( curTurnDesc != newTurnDesc) {
         sprintf(origValStr, "%s", StringTable::getString(
                  ItemTypes::getTurndirectionSC(curTurnDesc),
                  MEItemInfoDialog::m_language) );
         vector<MC2String> logStrings = 
               logCommentBox->getLogComments(origValStr);
         
         m_curConnection->setTurnDirection(newTurnDesc);
         sprintf(tmpStr, "%s", StringTable::getString(
                  ItemTypes::getTurndirectionSC(newTurnDesc),
                  MEItemInfoDialog::m_language) );
         newValStrings.clear();
         newValStrings.push_back( tmpStr );
         
         MELogFilePrinter::print(
               logStrings, "setTurnDirection", identString, newValStrings );
         mc2dbg << "Turn-description set to: " << tmpStr << endl;
      }
   } else {
      mc2log << error << here << " Invalid turn-description: " 
             << newTurnDescVal << endl;
   }

   // Turn restrictions (vehicle restrictions)
   uint32 dialogRestr = getVehicleRestrictions();
   if ( dialogRestr != m_curConnection->getVehicleRestrictions()) {
      sprintf(origValStr, "0x%x", m_curConnection->getVehicleRestrictions());
      vector<MC2String> logStrings = logCommentBox->getLogComments(origValStr);
      
      m_curConnection->setVehicleRestrictions(dialogRestr);
      sprintf(tmpStr, "0x%x", dialogRestr);
      newValStrings.clear();
      newValStrings.push_back( tmpStr );
      
      MELogFilePrinter::print(
            logStrings, "setVehicleRestrictions", identString, newValStrings );
      mc2dbg << "Vehicle restrictions set to: " << tmpStr << endl;
   }
}

#endif 
