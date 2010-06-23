/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserThread.h"

#ifdef USE_XML
#include "XMLTool.h"
#include "XMLUtility.h"
#include "STLStringUtility.h"
#include "XMLServerElements.h"
#include "ClientSettings.h"
#include "ClientVersion.h"

bool
XMLParserThread::xmlParseServerInfoRequest(  DOMNode* cur,
                                             DOMNode* out,
                                             DOMDocument* reply,
                                             bool indent )
   try { 
   // create root node server_info_reply
   DOMElement* root =
      XMLUtility::createStandardReply( *reply, *cur, "server_info_reply" );
   
   using namespace XMLTool;

   // get request attributes
   MC2String strClientType;
   MC2String strClientTypeOptions;
   MC2String strClientVersion;

   getAttrib( strClientType, "client_type", cur );
   getAttrib( strClientTypeOptions, "client_type_options", cur, MC2String("") );
   getAttrib( strClientVersion, "client_version", cur );

   ClientVersion clientVer( strClientVersion, true /* programVersionOnly */ );

   const ClientSetting* setting = m_group->getSetting( 
      strClientType.c_str(), strClientTypeOptions.c_str() );

   // Check if version info is available for this client type
   bool upgradeAvailable = false;
   MC2String versionStr( setting->getVersion() );
   if( !versionStr.empty() && clientVer < *( setting->getVersionObj() ) ) {
      upgradeAvailable = true;
   }

   
   DOMElement* clientTypeInfoNode = addNode( root, "client_type_info" );
   addAttrib( clientTypeInfoNode, "upgrade_available", upgradeAvailable );
   addAttrib( clientTypeInfoNode, "latest_version", 
              MC2String( setting->getVersionObj()->getProgramVersionString() ) );
   addAttrib( clientTypeInfoNode, "force_upgrade", 
              setting->getForceUpgrade() && upgradeAvailable );
   
   if( !setting->getUpgradeId().empty() ) {
      addAttrib( clientTypeInfoNode, "upgrade_id", setting->getUpgradeId() );
   }

   out->appendChild( root );
   
   if ( indent ) {
      XMLUtility::indentPiece( *root, 1 );
   }
   
   return true;
   
} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[XMLParserThread]  " << e.what() << endl;
   XMLServerUtility::appendStatusNodes( out, reply, 1, false, "-1", e.what() );
   return true;
} catch ( const MC2Exception& ex ) {
   mc2log << error << "[XMLParserThread]  " << ex.what() << endl;
   XMLServerUtility::appendStatusNodes( out, reply, 1, false, "-1", ex.what() );
   return true;
} 

#endif // USE_XML
