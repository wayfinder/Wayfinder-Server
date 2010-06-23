/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHVIEWER_NAVSEARCHHANDLER_H
#define SEARCHVIEWER_NAVSEARCHHANDLER_H

#include "SearchHandler.h"
#include <memory>
#include <vector>

namespace NGPMaker {
struct Request;
}

class NParam;
class NParamBlock;

namespace SearchViewer {

/**
 * Handles Navigator search request
 */
class NavSearchHandler: public SearchHandler {
public:
   NavSearchHandler();

   /// @see SearchHandler
   void search( const IPnPort& address,
                const Auth& auth,
                const MC2String& descriptionCRC,
                const CompactSearch& search );
   void getReplyDebugData( MC2String& data, uint32 maxColumns ) {
      data = m_replyString;
   }
   /// @see SearchHandler
   const Headings& getMatches() const;
   

private:
   void parseCombinedSearch( NParamBlock& replyBlock );
   void parseSearchDesc( NParamBlock& reply );
   void addResult( Headings& headings, const NParam& reply );
   void sendRequest( const IPnPort& address,
                     std::vector<byte>& data,
                     MC2String& debugStr );
   void parseReply( vector<byte>& reply );
   Headings m_headings;
   std::auto_ptr<NGPMaker::Request> m_request;
   MC2String m_replyString;
};

}
#endif 
