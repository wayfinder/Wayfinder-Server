/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTINFOQUERY_H
#define EXTINFOQUERY_H

#include "MC2String.h"
#include "ExtServices.h"
#include "SearchReplyData.h"
#include "LangTypes.h"
#include "MC2Coordinate.h"

/**
 * Holds information about an info query.
 */
class ExtInfoQuery {
public:
   ExtInfoQuery( SearchReplyData& reply,
                 const ExtService& service,
                 const MC2String& externalId,
                 const LangType& lang,
                 int nbrRetries,
                 const MC2Coordinate& coord,
                 const MC2String& name ):
      m_reply( reply ),
      m_service( service ),
      m_externalId( externalId ),
      m_lang( lang ),
      m_nbrRetries( nbrRetries ),
      m_coord( coord ),
      m_name( name ) {
   }

   SearchReplyData& getReply() {
      return m_reply;
   }

   const ExtService& getServiceID() const {
      return m_service;
   }

   const MC2String& getExternalID() const {
      return m_externalId;
   }

   const LangType& getLang() const {
      return m_lang;
   }

   uint32 getNbrRetries() const {
      return m_nbrRetries;
   }

   const MC2String& getName() const {
      return m_name;
   }

   const MC2Coordinate& getCoordinate() const {
      return m_coord;
   }

private:
   SearchReplyData& m_reply;
   ExtService m_service;
   MC2String m_externalId;
   LangType m_lang;
   uint32 m_nbrRetries;
   MC2Coordinate m_coord;
   MC2String m_name;
};

#endif // EXTINFOQUERY_H
