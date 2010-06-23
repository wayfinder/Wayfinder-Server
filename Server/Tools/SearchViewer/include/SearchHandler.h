/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHVIEWER_SEARCHHANDLER_H
#define SEARCHVIEWER_SEARCHHANDLER_H

#include "MC2String.h"
#include "CompactSearch.h"

#include <vector>
#include <map>
#include <iosfwd>

class IPnPort;

namespace SearchViewer {

struct ItemMatch {
   MC2String m_type;
   MC2String m_image;
   MC2String m_name;
   MC2String m_itemID;
   MC2String m_locationName;
   uint32 m_lat, m_lon;
   bool m_advert;
};

struct SearchMatch {
   SearchMatch():
      m_heading( -1 ),
      m_nbrItems( 0 ),
      m_startIndex( 0 ),
      m_endIndex( 0 ),
      m_totalNbrItems( 0 ) 
   {}
   
   int32 m_heading;
   uint32 m_nbrItems;
   uint32 m_startIndex;
   uint32 m_endIndex;
   uint32 m_totalNbrItems;
   uint32 m_topHits;
   typedef std::vector<ItemMatch> ItemMatches;
   ItemMatches m_items;
};


struct DescMatch {
   MC2String m_name;
   MC2String m_image;
   uint32 m_headingNum;
   SearchMatch m_matches;
};

struct Headings {
   MC2String m_crc; ///< crc of search descriptor
   typedef std::vector<DescMatch> ResultHeadings;
   /// Contains the headings from result.
   ResultHeadings m_resultHeadings; 
   typedef std::map<uint32, DescMatch> HeadingMap;
   /// Contains all headings from search desc including result.
   HeadingMap m_headings;
};

class Auth;

/**
 * Takes care of search requests to the server.
 */
class SearchHandler {
public:
   typedef std::vector< SearchMatch > SearchMatches;
   typedef std::vector< ItemMatch > ItemMatches;
   /// @param name the name of this handler, i.e XML or Navigator.
   explicit SearchHandler( const MC2String& name ):
      m_name( name ) { 
   }

   virtual ~SearchHandler() {
   }

   /**
    * Sends a search request to the server. The results
    * can be fetch using getMatches. @see getMatches
    * @param address ip and port for server
    * @param auth authentication values
    * @param descriptionCRC crc value for search_desc
    * @param search parameters for search
    */
   virtual void search( const IPnPort& address,
                        const Auth& auth,
                        const MC2String& descriptionCRC,
                        const CompactSearch& search ) = 0;
   /**
    * Fetches matches after a search. @see search
    * @return the matches
    */
   virtual const Headings& getMatches() const = 0;

   /**
    * Fetches a string that can be viewed as debug output for the 
    * return data from the server i.e xml reply from xml server and hexdump
    * from Navigator server.
    * @param data returns data for debug
    * @param maxColumns maximum number of columns
    */
   virtual void getReplyDebugData( MC2String& data, uint32 maxColumns ) {
      data = "no reply debug data.";
   } 
   /// @return name of this handler
   const MC2String& getName() const { return m_name; }

private:
   MC2String m_name;
};


} // SearchViewer

ostream& operator << ( ostream& ostr, const CompactSearch& params );

#endif // SEARCHVIEWER_SEARCHHANDLER_H
