/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHREPLYPACKET_H
#define SEARCHREPLYPACKET_H

#include "config.h"
#include "SearchMatch.h"
#include "Packet.h"
#include "SearchTypes.h"
#include "SearchParams.h"

#include <vector>
#include <map>

class VanillaSearchReplyPacket; // forward
class SearchRequestPacket;      // forward (extern)
class MatchLink;
class VanillaRegionMatch;

#define SEARCH_REPLY_HEADER_SIZE             (REPLY_HEADER_SIZE+8)
#define OVERVIEW_SEARCH_REPLY_HEADER_SIZE    (SEARCH_REPLY_HEADER_SIZE+8)

/**
 *    Superclass to all search reply packets.
 *
 */
class SearchReplyPacket : public ReplyPacket {
  public:
   /**
    * Constructor
    * @param subType  The subtype of the reply (same as that of the request).
    * @param p        The (search)requestpacket that this reply is a reply to.
    * @param nbrPacks The number of other packets to merge into this packet.
    *                 (SearchHandler should not merge stuff into packets...).
    */
   SearchReplyPacket( uint16 subType,
                      const RequestPacket* p,
                      int nbrPacks = 1 );

};

/**
 *    This class of objects is sent in reply to most
 *    searchrequests.
 *
 */
class VanillaSearchReplyPacket : public SearchReplyPacket
{
  public:
 
   /**
    *   Constructor
    *   @param p the packet to be a reply to
    *   @param nbrPackets The number of packets to merge into the packet.
    *                     (Should not merge stuff in packets, really)
    */
   VanillaSearchReplyPacket( const RequestPacket* p,
                             int nbrPackets = 1 );

   /**
    *   Returns all matches in a vector.
    *   @param matches The vector to add the matches to.
    */
   void getAllMatches(vector<VanillaMatch*>& matches);

   /**
    *   Reads the header and returns the first object (if any, 
    *   otherwise NULL).
    *
    *   @param   position The value of position is set to point 
    *                     after the first object. This is a out
    *                     parameter (invalue is ignored).
    *   @param   nbrMatches  The number of matches in the
    *                     packet (value parameter).
    *   @return  The first match of the packet (or NULL).
    *            You should delete this object when you are done
    *            with it. After deleting all objects associated
    *            with a certain packet, the packet can be deleted.
    *            (the objects refer to strings in the packet directly).
    */
   VanillaMatch *getFirstMatch( int &position,
                                int &nbrMatches ) const;

   /**
    * @return the next object of the packet
    * (or NULL if packet end)
    */
   VanillaMatch *getNextMatch( int &position ) const;

   /**
    * @return the size of the databuffer (excluding header)
    */
   uint32 getSizeData() const {
      return readLong(REPLY_HEADER_SIZE+4);
   }

   /**
    * @return the number of matches in the packet
    */
   uint32 getNumberOfMatches() const {
      return readLong(REPLY_HEADER_SIZE);
   }

   /** sets the empty flag @param empty true if there were no objects
    *  at all within the selected locations
    */
   void setLocationEmptyHack( bool empty );
   bool getLocationEmptyHack() const;
  
   /**
    * Creates a linked list of matches. Can be used for mergesorting.
    * @return the linked list
    * @param nbrMatches return parameter containing the length of
    *                   the list.
    */
   MatchLink *getMatchesAsLinks( int &nbrMatches ) const
      {
         int position;
         VanillaMatch *vm = getFirstMatch( position,
                                           nbrMatches );
         MatchLink *firstLink = NULL;
         if (vm!=NULL) {
            firstLink = new MatchLink( vm );
            MatchLink *currentLink = firstLink;
            MatchLink *newLink = NULL;
            while (vm != NULL) {
               vm = getNextMatch( position );
               if (vm != NULL) {
                  newLink = new MatchLink( vm );
                  currentLink->setNext(newLink);
                  currentLink = newLink;
               } else {
                  currentLink->setNext( NULL );
               }
            }
         }
         return firstLink;
      }
    

   /**
    * Creates a vector with copies of the matches in a new list.
    * @return a new list with copies of the matches, is getNumberOfMatches
    *         long
    */
   SearchMatchLink* getMatchesAsSearchLinks( int& nbrMatches ) const;
   
   /**
    *   Adds a match to the packet
    *   @param theMatch         The match. Could maybe be the only parameter
    *                           later. Coordinates are taken from this.
    *   @param increaseCount    True if the total number of matches should
    *                           be increased for the packet.
    */
   inline int addMatch( const SearchMatch* match,                      
                        bool increaseCount = true);

  private:
   
   /**
    *   Increases the number of matches counter
    */
   void incNumberOfMatches(){
      writeLong( REPLY_HEADER_SIZE, readLong(REPLY_HEADER_SIZE)+1);
   }

   /**
    *   Sets the length of the used part of the packet
    *   @param size the new size
    */
   void setSizeData( uint32 size ){
      writeLong( REPLY_HEADER_SIZE+4, size );
   }

   /**
    * Increases the length of the used part of the packet
    */
   void updateSize( uint32 index ){
      mc2dbg4 << "updateSize: " << index << endl;
      setSizeData( index - (SEARCH_REPLY_HEADER_SIZE + 4) );
      setLength( index );
   }

};

/**
 *    Superclass to OverviewSearchReplyPacket
 *    (and in the future of VanillaSearchReplyPacket).
 *
 */
class SearchMatchReplyPacket : public SearchReplyPacket {
  public:
   /**
    * @return the number of matches in the packet
    */
   uint32 getNumberOfMatches() const {
      mc2dbg4<< "getNumberOfMatches " << readLong(REPLY_HEADER_SIZE) 
             << endl;
      return readLong(REPLY_HEADER_SIZE);
   }

   /**
    *   Reads the header and returns the first object (if any, 
    *   otherwise NULL).
    *
    *   @param   position The value of position is set to point 
    *                     after the first object. This is an out
    *                     parameter (invalue is ignored).
    *   @param   nbrMatches  The number of matches in the
    *                     packet (value parameter).
    *   @return  The first match of the packet (or NULL).
    *            You should delete this object when you are done
    *            with it. After deleting all objects associated
    *            with a certain packet, the packet can be deleted.
    *            (the objects refer to strings in the packet directly).
    */
   SearchMatch *getFirstMatch(int &position,
                              int &nbrMatches ) const;
   
   /**
    * Implement all subclasses here.
    * @return the next object of the packet
    * (or NULL if packet end)
    */
   SearchMatch *getNextMatch( int &position ) const;

  protected:
   /** constructor
    *   After the normal header the reply packet contains:
    *   \begin{tabular}{lll}
    *      Pos                      & Size     & Description \\ \hline
    *      REPLY_HEADER_SIZE        & 4 bytes  & number of matches \\
    *      +4                       & 4 bytes  & length of data \\
    *      SEARCH_REPLY_HEADER_SIZE & x bytes  & the rest of the data
    *   \end{tabular}
    */

   /** constructor
    *  @param subType the type of the packet to return
    *  @param p the request packet (info on to whom to send the reply)
    */
   SearchMatchReplyPacket( uint16 subType,
                           const RequestPacket *p );
   
   /**
    * Increases the number of matches counter
    */
   void incNumberOfMatches() {
      writeLong( REPLY_HEADER_SIZE, readLong(REPLY_HEADER_SIZE) + 1);
   }
   
   /**
    * @return the size of the databuffer (excluding header)
    */
   uint32 getSizeData() const {
      return readLong(REPLY_HEADER_SIZE+4);
   }

   /**
    * Sets the length of the used part of the packet
    * @param size the new size
    */
   virtual void setSizeData( uint32 size ) {
      writeLong( REPLY_HEADER_SIZE+4, size );
   }

   /**
    * Increases the length of the used part of the packet
    */
   void updateSize( uint32 index ) {
      mc2dbg4 << "updateSize: " << index << endl;
      setSizeData( index );
      setLength( index );
   }

  private:

   /// common initialization for the constructors.
   ///  not for subclasses. private
   void init();
};

class OverviewSearchReplyPacket; // forward


/**
 *    OverviewSearchReplyPacket. First the normal SearchMatchReplyPacket 
 *    header, then the reply packet contains:
 *    \begin{tabular}{lll}
 *       Pos                               & Size     & Description \\ \hline
 *       REPLY_HEADER_SIZE                 & 4 bytes  & number of matches \\
 *       +4                                & 4 bytes  & length of data \\
 *       SEARCH_REPLY_HEADER_SIZE          & 4 bytes  & the total number of masks \\
 *       +4                                & 4 bytes  & uniqueOrFull bitmask \\
 *       OVERVIEW_SEARCH_REPLY_HEADER_SIZE & x bytes  & the matches \\
 *    \end{tabular}
 *
 *    The matches look like:
 *    \begin{tabular}{lll}
 *       Pos                & Size     & Description \\ \hline
 *       REPLY_HEADER_SIZE  & 4 bytes  & number of matches \\
 *       +4                 & 4 bytes  & length of data \\
 *       S_R_H_S            & 4 bytes  & the total number of masks \\
 *       OVERVIEW_S_R_H_S   & 4 bytes  & mapID \\
 *       +4                 & 4 bytes  & mask \\
 *       +4                 & 4 bytes  & itemID \\
 *       +4                 & x bytes  & name (as char *) \\
 *    \end{tabular}
 *
 */
class OverviewSearchReplyPacket : public SearchMatchReplyPacket {
  public:
   /** 
    *    Constructor.
    *
    */
   OverviewSearchReplyPacket( RequestPacket *p )
      : SearchMatchReplyPacket(
         Packet::PACKETTYPE_OVERVIEWSEARCHREPLYPACKET, p)
      {
         setFull(false);
         setTotalNbrMasks( 0 );
         updateSize( OVERVIEW_SEARCH_REPLY_HEADER_SIZE );
      };

   /**
    *    Constructor. Status defaults to StringTable::OK since there
    *    is a vector of results.
    *    @param req The request packet.
    *    @param result Vector of OverviewMatches to add to the packet.
    */
   OverviewSearchReplyPacket( const RequestPacket* req,
                              const vector<OverviewMatch*>& result );
   
   /**
    * Gets the starting position to start using getNextMatch.
    * @return The starting position.
    */
   int getStartPosition() const {
      return OVERVIEW_SEARCH_REPLY_HEADER_SIZE;
   }
   
   /**
    * @return the next object of the packet
    * (or NULL if packet end)
    */
   SearchMatch *getNextMatch( int &position ) const;

   /** Adds a match to the packet
    *  @param om the overview match to add
    */
   void addOverviewMatch( OverviewMatch *om );

   /** @return the total number of masks in all of the overviewmatches */
   uint32 getTotalNbrMasks() const;

   /**
    *    Sets the total number of masks in this packet.
    *    @param totNbrMasks total number of masks in the packet.
    */
   void setTotalNbrMasks( uint32 totNbrMasks );

   /** @return true if the returned matches are either
    *          unique or fully matching the requested search string
    */
   uint32 getUniqueOrFull() const;

   /** @param uof unique | full bitmask
    */
   void   setUniqueOrFull( uint32 uof );

   /**
    *   Return true if the matches are full.
    */
   bool getFull() const;

   /**
    *   Sets the full flag which means that all the matches
    *   are full matches.
    */
   void setFull( bool full );
      
  private:
 
};

// inlines

int 
VanillaSearchReplyPacket::addMatch( const SearchMatch* theMatch,
                                    bool increaseCount)
{
   mc2dbg4 << "addMatch start" << endl;
   int position = readLong( REPLY_HEADER_SIZE+4 );
   position += SEARCH_REPLY_HEADER_SIZE +4; // +4 for the emptyHack

   if (position < PACKET_FULL_LIMIT || !increaseCount) {
      // If not increaseCount then match if part of other match -> add it

      if ( increaseCount ) {
         incNumberOfMatches();
      }
      theMatch->save(this, position);
      updateSize(position);
   } else {
      mc2dbg8 << "Packet full!!!" << endl;
      position = -1;
   }
   return (position);
}

#endif
