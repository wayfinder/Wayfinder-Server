/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IDTRANSLATIONPACKET_H
#define IDTRANSLATIONPACKET_H

#define IDTRANSLATION_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define IDTRANSLATION_REPLY_PRIO   DEFAULT_PACKET_PRIO

#define IDTRANSLATION_REQUEST_MAX_LENGTH 65536
#define IDTRANSLATION_REPLY_MAX_LENGTH   65536

#include "Types.h"
#include "Packet.h"
#include "OrigDestInfo.h"
#include "IDPairVector.h"

/**
 *    RequestPacket for translating the supplied map and nodeID:s
 *    from one level to another, (e.g. from lower level to higher).
 *    The packet takes a number of IDPairVector_t-pointers and the
 *    number of pointers as inparameter. The module should translate
 *    the nodes to their counterpart on the requested level and if
 *    it is impossible, the mapID and nodeID should be MAX_UINT32.
 *    The nodes should keep their position in the vector so that the
 *    receiver can keep track of which node is which.<br>
 *    The packet should probably be sent to a module with an overviewmap.<br>
 *    After the normal header the packet contains (X is REQUEST_HEADER_SIZE):
 *    @packetdesc
 *        @row X       @sep 4 bytes @sep User defined data. Should be copied
 *                                       to the reply @endrow
 *        @row X + 4   @sep 1 byte  @sep Flags. Bit zero is translateToLower.
 *                                                    @endrow
 *        @row X + 5   @sep 3 bytes @sep Unused.                 @endrow
 *        @row X + 8   @sep 4 bytes @sep Unused.                 @endrow
 *        @row X + 12  @sep 4 bytes @sep Nbr of nodevectors.     @endrow
 *        @row X + 16  @sep 4 bytes*nbr of nodevectors
 *                                  @sep Sizes of nodevectors start here
 *                                                               @endrow
 *    @endpacketdesc
 *    Each nodevector consists of:
 *    @packetdesc
 *       @row Y        @sep 8 bytes*nbrNodes @sep mapid, nodeid @endrow
 *    @endpacketdesc
 */
class IDTranslationRequestPacket : public RequestPacket {

public:

   /**  The position of the user definded data. */
   static const int USER_DEF_POS = REQUEST_HEADER_SIZE;   

   /**  The position of the flags. */
   static const int FLAGS_POS    = USER_DEF_POS + 4;
   
   /**  The unused longword. */
   static const int UNUSED_POS   = FLAGS_POS + 4;
   
   /** The position of the number of nodevectors. */
   static const int NBR_NODE_VECT_POS = UNUSED_POS + 4;

   /** The position of the start of the sizes of the nodevectors. */
   static const int NODE_VECT_SIZES_START_POS = NBR_NODE_VECT_POS + 4;

   /** The bitposition of the lowerlevel flag */
   static const int LOWER_LEVEL_BIT_POS = 0;
   
   /**
    *   Creates a IDTranslationRequestPacket for two nodevectors.
    *   @param packetID  @see RequestPacket.
    *   @param requestID @see RequestPacket.
    *   @param mapID     Overview map id,
    *   @param translateToLower True if the nodes should be translated
    *                           from higher to lower level. False if the
    *                           nodes should be translated from
    *                           lower to higher level.
    *   @param nodeList  The node vector.
    *   @param userDef   Userdefined data to be copied to the answer.
    */
   IDTranslationRequestPacket(uint32 packetID,
                              uint32 requestID,
                              uint32 mapID,
                              bool translateToLower,
                              const OrigDestInfoList& nodeList,
                              uint32 userDef = MAX_UINT32);
   
   /**
    *   Creates a IDTranslationRequestPacket.
    *   @param packetID  @see RequestPacket.
    *   @param requestID @see RequestPacket.
    *   @param mapID     Overview map id,
    *   @param translateToLower True if the nodes should be translated
    *                           from higher to lower level. False if the
    *                           nodes should be translated from
    *                           lower to higher level.
    *   @param nodesVectors
    *                Array of NoteVector_t*:s containing nodes to
    *                be translated.
    *   @param nbrNodeLists
    *                Number of vectors in <code>nodeVectors</code>.
    *   @param userDef   Userdefined data to be copied to the answer.
    */
   IDTranslationRequestPacket( uint32 packetID,
                               uint32 requestID,
                               uint32 mapID,
                               bool translateToLower,
                               IDPairVector_t** nodeVectors,
                               int nbrNodeVectors,
                               uint32 userDef = MAX_UINT32);
                                 
   IDTranslationRequestPacket(uint32 packetID,
                              uint32 requestID,
                              uint32 mapID,
                              bool translateToLower,
                              IDPairVector_t& nodeVector,
                              uint32 userDef = MAX_UINT32);
                                 

   /**
    *   Returns the user defined data of the packet.
    */
   uint32 getUserDefData() const;
   
   /**
    *   Return the number of nodevectors.
    */
   int getNbrNodeVectors() const;

   /**
    *   Returns the number of nodes for vector <code>i</code>.
    */
   int getNbrNodesForVector(int i) const;

   /**
    *   Returns the position in the packet where the nodeID:s start.
    */
   int getNodesStartPos() const;
   
   /**
    *   Put the nodes in <code>nodeVectors</code>.
    *   @return The number of nodes.
    */
   int getAllNodes(IDPairVector_t& allNodes) const;

   /**
    *   Return true if the nodes should be translated from higher
    *   to lower level. False if the opposite should be done.
    */
   bool getTranslateToLower() const;
   
   /**
    *   Calculates the minimum size of a packet to allocate
    *   for the supplied <code>nodeVectors</code> and nodes.
    *   @return Minimum size of the packet buffer.
    */
   static int calcPacketSize(const IDPairVector_t** nodeVectors,
                             int nbrNodeVectors);

   /**
    *   Calculates the minimum size of a packet to allocate
    *   for the supplied nodes.
    *   @return Minimum size of the packet buffer.
    */
   static int calcPacketSize(const IDPairVector_t& nodeVect1,
                             const IDPairVector_t& nodeVect2);
private:

   /**
    *   Helper function for the constructors.
    *   @see @IDTranslationRequestPacket::IDTranslationRequestPacket
    */
   void init(bool translateToLower,
             IDPairVector_t** nodeVectors,
             int nbrNodeVectors,
             uint32 userDef);
   
};

/**
 *   ReplyPacket for the IDTranslationRequestPacket.
 *   The nodes in the packet are identified by their positions
 *   in the vectors.
 *   @packetdesc
 *        @row X       @sep 4 bytes @sep Map id             @ endrow
 *        @row X + 4   @sep 4 bytes @sep User defined data. Should be copied
 *                                       to the reply @endrow
 *        @row X + 8   @sep 1 byte  @sep Flags. Bit zero is translateToLower.
 *                                                    @endrow
 *        @row X + 9   @sep 3 bytes @sep Unused.                 @endrow
 *        @row X + 12  @sep 4 bytes @sep Unused.                 @endrow
 *        @row X + 16  @sep 4 bytes @sep Nbr of nodevectors.     @endrow
 *        @row X + 20  @sep 4 bytes*nbr of nodevectors
 *                                  @sep Sizes of nodevectors start here
 *                                                               @endrow
 *    @endpacketdesc
 *    Each nodevector consists of:
 *    @packetdesc
 *       @row Y        @sep 8 bytes*nbrNodes @sep mapid, nodeid @endrow
 *    @endpacketdesc
 */
class IDTranslationReplyPacket : public ReplyPacket {

public:

   /**  The position of the map id */
   static const int MAP_ID_POS   = REPLY_HEADER_SIZE;
   
   /**  The position of the user definded data. */
   static const int USER_DEF_POS = MAP_ID_POS + 4;   

   /**  The position of the flags. */
   static const int FLAGS_POS    = USER_DEF_POS + 4;
   
   /**  The unused longword. */
   static const int UNUSED_POS   = FLAGS_POS + 4;
   
   /** The position of the number of nodevectors. */
   static const int NBR_NODE_VECT_POS = UNUSED_POS + 4;

   /** The position of the start of the sizes of the nodevectors. */
   static const int NODE_VECT_SIZES_START_POS = NBR_NODE_VECT_POS + 4;

   /** The bitposition of the lowerlevel flag */
   static const int LOWER_LEVEL_BIT_POS = 0;
   
   
   /**
    *   Creates a reply to the supplied request.
    *   @param p The RequestPacket that this is a reply to.
    *   @param status The status of the reply.
    *   @param allTranslatedNodes All translated nodes.
    */
   IDTranslationReplyPacket( const IDTranslationRequestPacket* p,
                             uint32 status,
                             const IDPairVector_t& allTranslatedNodes);

   /**
    *   Creates a reply to the supplied request. Status is probably
    *   not OK.
    */
   IDTranslationReplyPacket( const IDTranslationRequestPacket* p,
                             uint32 status);

   /**
    *   Calculates the minimum size of the packet for the supplied
    *   data.
    *   @return The minimum size of the packet buffer.
    */
   static int calcPacketSize( const IDTranslationRequestPacket* p,
                              const IDPairVector_t& allTranslatedNodes );

   /**
    *   Returns the user defined data.
    */
   uint32 getUserDefinedData() const;

   /**
    *   Returns the nodes for the supplied list.
    *   @param index      The list number to use.
    *   @param outList    The translated lists will be put here.
    *   @param origList   The list containing the costs and lat/lons.
    */
   int getTranslatedNodes( int index,
                           OrigDestInfoList& outList,
                           const OrigDestInfoList& origList ) const;

   int getTranslatedNodes( int index,
                           IDPairVector_t& outVector);
   
   /**
    *   Returns the number of node vectors.
    */
   int getNbrNodeVectors() const;
   
   /**
    *   Returns the start position for the nodes.
    */
   int getNodesStartPos() const;
   
   /**
    *   Returns the number of nodes for the vector at pos <code>index</code>.
    */
   int getNbrNodesForVector(int index) const;

   /**
    *   Returns the start position of the vector at pos <code>index</code>.
    */
   int getNodesStartForVector(int index) const;
};

#endif

