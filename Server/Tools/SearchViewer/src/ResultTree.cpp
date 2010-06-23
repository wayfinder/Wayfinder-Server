/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ResultTree.h"

ResultTree::ResultTree() {
   m_treeStore = gtk_tree_store_new( 2, // two rows
                                     GDK_TYPE_PIXBUF, // first with image
                                     G_TYPE_STRING ); // second with string

   m_scrolledWindow = gtk_scrolled_window_new( NULL, NULL );
   // create and setup view
   GtkWidget* treeView = gtk_tree_view_new_with_model( GTK_TREE_MODEL( m_treeStore ) );

   // create the first column
   GtkCellRenderer* cellRenderer = gtk_cell_renderer_pixbuf_new();
   GtkTreeViewColumn *column = 
      gtk_tree_view_column_new_with_attributes( "",
                                                cellRenderer,
                                                "pixbuf", 0, // first column
                                                NULL );
   // append first column to the view
   gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column );

   // create the second column
   cellRenderer = gtk_cell_renderer_text_new();
   column = 
      gtk_tree_view_column_new_with_attributes( "",
                                                cellRenderer,
                                                "text", 1, // second column
                                                NULL );

   gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column );

   //   gtk_tree_view_set_fixed_height_mode( GTK_TREE_VIEW(treeView), true );
   gtk_tree_view_set_headers_visible( GTK_TREE_VIEW(treeView), false );

   gtk_container_add( GTK_CONTAINER( m_scrolledWindow ), treeView );
   gtk_widget_set_size_request( m_scrolledWindow, -1, 200 );

   m_treeView = treeView;

}

ResultTree::~ResultTree() {
   gtk_widget_destroy( m_scrolledWindow );
}

void ResultTree::addHeading( const MC2String& name,
                             const MC2String& imagename ) {
   GdkPixbuf* imagePixbuf = gdk_pixbuf_new_from_file( imagename.c_str(), NULL );
   // create a new line
   gtk_tree_store_append( m_treeStore, &m_curlineIt, NULL );
   gtk_tree_store_set( m_treeStore, &m_curlineIt,
                       0, imagePixbuf, // first column empty
                       1, name.c_str(), // second column text
                       -1 );
   
}

void ResultTree::addResult( const MC2String& name,
                            const MC2String& imagename ) {
   GdkPixbuf* imagePixbuf = gdk_pixbuf_new_from_file( imagename.c_str(), NULL );
   GtkTreeIter col2It;
   gtk_tree_store_append( m_treeStore, &col2It, &m_curlineIt );
   gtk_tree_store_set( m_treeStore, &col2It,
                       0, imagePixbuf,
                       1, name.c_str(),
                       -1 );
}
