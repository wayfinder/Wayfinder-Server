/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTSERVICETALKER_H
#define EXTSERVICETALKER_H

#include "config.h"
#include "MC2String.h"

class SearchReplyData;
class EventRequestData;
class ExtService;
class LangType;
class ExtInfoQuery;
class ExternalSearchRequestData;

/**
 * Interface for External Service talkers
 */
class ExtServiceTalker {
public:
   explicit ExtServiceTalker( const MC2String& name ):
      m_name( name ) { }

   virtual ~ExtServiceTalker() { 
   }

   /**
    * The function called to initiate a xml query and
    * to parse result.
    *
    * @param reply        Where we store the search result to return
    * @param searchData   The user input data to create a query from
    * @param nbrRetries   The number of times to retry the xml query
    */
   virtual int doQuery( SearchReplyData& reply,
                        const ExternalSearchRequestData& searchData,
                        int nbrRetries = 3 ) = 0;

   /** Obsolete! Use doQuery with InfoQuery instead.
    * Get more info about one item.
    *
    * @param reply           Where we store the search result to return
    * @param ExtServicethe   type of service
    * @param externalID      The id of the info
    * @param lang            language
    * @param nbrRetries      The number of times to retry the xml query
    */
   virtual int doQuery( SearchReplyData& reply,
                        const ExtService& service, 
                        const MC2String& externalId, 
                        const LangType& lang, 
                        int nbrRetries = 5 ) = 0;

   /**
    * Queries item info from provider.
    * @param info All parameters needed to query item info.
    */
   virtual int doInfoQuery( ExtInfoQuery& info );

   const MC2String& getServiceName() const { return m_name; }

private:
   MC2String m_name; ///< name of this service
};

#endif // EXTSERVICETALKER_H
