/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLSERVERELEMENTS_H
#define XMLSERVERELEMENTS_H

#include "config.h"
#include "XMLUtility.h"
#include "MC2String.h"

class NamedServerList;

// most function moved from XMLParserThread
namespace XMLServerUtility {

/**
 * Appends an empty element as last child in cur.
 * @param cur Where to put the node as last child.
 * @param reply The document to make the nodes in.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 */
void appendElement( DOMNode* cur, DOMDocument* reply,
                    const char* name, 
                    int indentLevel, bool indent );

struct stringStringStruct { 
   stringStringStruct( const char* a, const char* b ) {
      first = a;
      second = b;
   }

   MC2String first; MC2String second; 
};
typedef vector< stringStringStruct > attrVect;


/**
 * Appends a element with a text child node as last child to cur.
 *
 * @param cur Where to put the sms_list node as last child.
 * @param reply The document to make the nodes in.
 * @param name The name of the node.
 * @param text The text to put as a text child node to the
 *             elementnode.
 * @param indentLevel The indent to use.
 * @param indent If indent.
 * @param attribute A vector of attributes name and value,
 *                  default Null.
 */
void appendElementWithText( DOMNode* cur, DOMNode* reply,
                            const char* name,
                            const char* text,
                            int indentLevel, bool indent,
                            attrVect* attribute = NULL );

/**
 * Makes a status_code and status_message node from indata as
 * children to out.
 * @param out Where to put the nodes.
 * @param reply The document to make the nodes in.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 * @param code String with the code of the status.
 * @param message String with the message of the status.
 * @param extendedCode String with the extended error code, default
 *                     NULL and not appended if so.
 * @param uri String with the status URI, default NULL and not 
 *            appended if so.
 */
void appendStatusNodes( DOMNode* out, 
                        DOMDocument* reply,
                        int indentLevel,
                        bool indent,
                        const char* const code,
                        const char* const message,
                        const char* extendedCode = NULL,
                        const char* uri = NULL );

/**
 * Appends a server list to out.
 *
 * @param fixedServerListName The server list to use, using 
 *        clientSetting if empty.
 */
void
appendServerList( DOMNode* out, 
                  DOMDocument* reply,
                  int indentLevel,
                  bool indent, 
                  const NamedServerList* serverList,
                  const MC2String& fixedServerListName = "" );

}

#endif // XMLSERVERELEMENTS_H
