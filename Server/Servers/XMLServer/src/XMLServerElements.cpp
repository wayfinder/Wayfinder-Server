/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLServerElements.h"

#include "NamedServerList.h"
#include "NamedServerGroup.h"
#include "UserFavorites.h"
#include "XMLSearchUtility.h"
#include "XMLTool.h"

namespace XMLServerUtility {

void appendElement( DOMNode* cur, DOMDocument* reply,
                    const char* name, 
                    int indentLevel, bool indent ) {
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );   

   DOMElement* textElement = reply->createElement( X( name ) );

   // Add textElement to cur
   if ( indent ) {
      cur->appendChild( reply->createTextNode( X( indentStr.c_str() ) ) );
   }
   cur->appendChild( textElement );
}

void appendElementWithText( DOMNode* cur, DOMNode* reply,
                            const char* name, const char* text,
                            int indentLevel, bool indent,
                            attrVect* attribute ) {

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );   
   DOMDocument* doc = XMLTool::getOwner( reply );
   DOMElement* textElement = doc->createElement( X( name ) );
   if ( text != NULL ) {
      textElement->appendChild( doc->createTextNode( X( text ) ) );
   }

   if ( attribute != NULL ) {
      for ( uint32 i = 0 ; i < attribute->size() ; ++i ) {
         textElement->setAttribute( X( (*attribute)[ i ].first ), 
                                    X( (*attribute)[ i ].second ) );
      }
   }

   // Add textElement to cur
   if ( indent ) {
      cur->appendChild( doc->createTextNode( X( indentStr.c_str() ) ) );
   }
   cur->appendChild( textElement );
}


void 
appendStatusNodes( DOMNode* out, 
                   DOMDocument* reply,
                   int indentLevel,
                   bool indent,
                   const char* const code,
                   const char* const message,
                   const char* extendedCode,
                   const char* uri ) {

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   // Fill in status_code and status_message in out

   DOMElement* status_code = reply->createElement( X( "status_code" ) );
   status_code->appendChild( reply->createTextNode( X( code ) ) );
   if ( indent ) {
      // Newline
      out->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
   }
   out->appendChild( status_code );


   DOMElement* status_message = reply->
      createElement( X( "status_message" ) );

   status_message->appendChild( reply->createTextNode( X( message ) ) );
   if ( indent ) {
      // Newline
      out->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
   }
   out->appendChild( status_message );

   if ( extendedCode != NULL ) {
      DOMElement* status_code_extended = reply->createElement( 
         X( "status_code_extended" ) );
      status_code_extended->appendChild( 
         reply->createTextNode( X( extendedCode ) ) );
      if ( indent ) {
         // Newline
         out->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
      }
      out->appendChild( status_code_extended );
   }

   if ( uri != NULL && *uri ) {
      DOMElement* status_uri = reply->createElement( X( "status_uri" ) );
      status_uri->setAttribute( X( "href" ), X( uri ) );
      if ( indent ) {
         // Newline
         out->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
      }
      out->appendChild( status_uri );
   }
}

void
appendServerList( DOMNode* out, 
                  DOMDocument* reply,
                  int indentLevel,
                  bool indent, 
                  const NamedServerList* serverList,
                  const MC2String& fixedServerListName ) {
    
   if ( serverList == NULL ) {
      mc2log << warn << "[XPT] appendServerList: bad server list for"
             << " serverStr=\"" << fixedServerListName << "\" " << endl;
      return;
   }

   DOMElement* server_list = reply->createElement( X( "server_list" ) );
   server_list->setAttribute( X( "crc" ), 
                              XHex32( serverList->getServerListCRC() ) );
   const NamedServerList::group_vector_t& gs = serverList->getGroups();
   server_list->setAttribute( X( "numberitems" ), XUint32( gs.size() ) );

   // For all groups
   for ( NamedServerList::group_vector_t::const_iterator 
            it = gs.begin() ; it != gs.end() ; ++it ) {
      const NamedServerGroup::server_vector_t& ss = (*it).getServers();
      DOMElement* server_group = reply->createElement( X( "server_group" ) );
      server_list->appendChild( server_group );
      server_group->setAttribute( X( "type" ),
                                  X( NamedServerGroup::
                                     getTypeAsString( (*it).getType() ) ) );

      server_group->setAttribute( X( "numberitems" ), 
                                  XUint32( ss.size() ) );
      server_group->
         setAttribute( X( "switch_group_threshold"), 
                       XUint32( (*it).getSwitchGroupThreshold() ) );
      // For all servers
      for ( NamedServerGroup::server_vector_t::const_iterator 
               sit = ss.begin() ; sit != ss.end() ; ++sit ) {
         DOMElement* server = reply->createElement( X( "server" ) );
         server_group->appendChild( server );
         server->appendChild( reply->createTextNode( X( *sit ) ) );
      }
   }

   out->appendChild( server_list );
   if ( indent ) {
      XMLUtility::indentPiece( *server_list, indentLevel );
   }

}

}
