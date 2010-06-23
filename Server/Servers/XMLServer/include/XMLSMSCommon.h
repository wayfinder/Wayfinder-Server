/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLSMSCOMMON_H
#define XMLSMSCOMMON_H

#include "MC2String.h"
#include "XMLUtility.h"
#include "StringTable.h"
#include "SearchMatch.h"
#include "VectorIncl.h"
#include "LangTypes.h"

#include <memory>

class UserItem;
class ParserThread;

/// @namespace common parsing for sms xml nodes
namespace XMLSMSCommon {

struct InviteData {
   MC2String m_type;
   MC2String m_name;
};

struct PlaceSMSData {
   MC2String m_type;
   MC2Coordinate m_coord;
   auto_ptr<VanillaMatch> m_match;
   LangTypes::language_t m_language;
};

/**
 * Appends a sms_list of smsmessages to cur of the strings in
 * smsVector.
 *
 * @param cur Where to put the sms_list node as last child.
 * @param reply The document to make the nodes in.
 * @param smsVector The smses.
 * @param indentLevel The indent to use.
 * @param indent If indent.
 */
bool appendSMSList( DOMNode* cur, DOMDocument* reply,
                    StringVector* smsVector,
                    int indentLevel, bool indent );

void composeInviteSMS( DOMNode* cur, DOMDocument* reply,
                       int indentLevel, bool indent,
                       const UserItem* user,
                       const InviteData& data );

void composePlaceSMS( ParserThread& thread,
                      DOMNode* cur, DOMDocument* reply,
                      int indentLevel, bool indent,
                      const UserItem* user,
                      const PlaceSMSData& data );


/**
 * Parse a wayfinder_destination_sms or wayfinder_route_sms
 */
bool xmlParseWayfinderSMS( DOMNode* cur,
                           char*& signature,
                           int32& originLat, int32& originLon,
                           MC2String& originDescription,
                           int32& destLat, int32& destLon,
                           MC2String& destDescription,
                           MC2String& errorCode, 
                           MC2String& errorMessage );
/**
 *   Parse a route_message_data
 */
bool xmlParseSendSMSRequestRouteMessageData( DOMNode* cur,
                                             uint32& routeID, 
                                             uint32& routeCreateTime,
                                             StringTable::languageCode& language,
                                             char*& signature,
                                             char*& originString, 
                                             char*& originLocationString,
                                             char*& destinationString, 
                                             char*& destinationLocationString );


/**
 *  Parse a wayfinder_favourite_sms.
 */
bool xmlParseWayfinderFavouriteSMS( DOMNode* cur,
                                    char*& signature,
                                    int32& lat, int32& lon,
                                    MC2String& name,
                                    MC2String& shortName,
                                    MC2String& description,
                                    MC2String& category,
                                    MC2String& mapIconName,
                                    MC2String& errorCode,
                                    MC2String& errorMessage );
}

#endif
