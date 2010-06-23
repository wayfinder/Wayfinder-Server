/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCH_REPLY_DATA_H
#define SEARCH_REPLY_DATA_H

#include "config.h"

#include <vector>

class VanillaMatch;
class Packet;
class PacketContainer;
class OverviewMatch;

/**
 *   Generic search result class.
 */
class SearchReplyData {
public:

   /// Initializes
   SearchReplyData();
   
   /// Deletes the contents of the vector(s).
   void clear();
   
   /// Deletes the contents of the vector(s).
   virtual ~SearchReplyData();
   
   /// Return a reference to the vector of matches.
   vector<VanillaMatch*>& getMatchVector() {
      return m_matches;
   }
   vector<OverviewMatch*>& getOverviewMatches() {
      return m_overviewMatches;
   }
   /**
    *   Sets the start and end index of the hits
    *   and also the total number of hits that
    *   can be returned.
    *   @param startIdx Index of first hit sent back. (0 in vector)
    *   @param endIdx   Index of last hit sent back.
    *   @param total    The total number of hits including the sent ones.
    */
   void setNbrHits( int startIdx,
                    int endIdx,
                    int total );
   
   /// Return a const reference to the vector of matches.
   const vector<VanillaMatch*>& getMatchVector() const {
      return m_matches;
   }
   const vector<OverviewMatch*>& getOverviewMatches() const {
      return m_overviewMatches;
   }

   /// Saves the data into the packet.
   int save( Packet* p, int& pos ) const;
   /// Loads the data from the packet.
   int load( const Packet* p, int& pos );

   /// Adds a packetcontainer to be deleted later.
   void addPacket( PacketContainer* pack ) {
      m_packetContainers.push_back( pack );
   }

   /// Returns the total number of matches
   uint32 getTotalNbrMatches() const;
   /// Translates match index to an index in the match vector
   int translateMatchIdx( int wantedIdx ) const;

private:
   /// Vector of ordinary matches.
   vector<VanillaMatch*> m_matches;
   /// Vector of overview matches
   vector<OverviewMatch*> m_overviewMatches;

   /// Vector of packetcontainers which contain data used in matches
   vector<PacketContainer*> m_packetContainers;

   /// Start index of the hits in the vector
   int m_startIdx;
   /// End index of the hits in the vector (startidx + m_matches.size())
   int m_endIdx;
   /// Total number of hits including the ones sent.
   int m_totalNbrHits;
   
};

#endif
