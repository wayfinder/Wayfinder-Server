/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __PACKET_H
#define __PACKET_H


#include "config.h"

#include "AlignUtility.h"
#include "BitUtility.h"
#include "TimeUtility.h"
#include "NotCopyable.h"

#include "MC2BoundingBox.h"
#include "MC2String.h"

#ifndef _MSC_VER
#  include <netinet/in.h> // htonl
#else
#  include <winsock.h>
#endif

#include <string.h>

class IPnPort;

#define DEFAULT_PACKET_PRIO    7
#define DEFAULT_PACKET_TAG     "MC2"
#define PACKET_OFFSET_TAG	        0
#define PACKET_OFFSET_PROTMAJOR    3
#define PACKET_OFFSET_PROTPRIO     4
#define PACKET_OFFSET_PACKETNBR    5
#define PACKET_OFFSET_SUBTYPE      6
#define PACKET_OFFSET_ORIGINIP     8
#define PACKET_OFFSET_ORIGINPORT  12
#define PACKET_OFFSET_REQUESTID   14
#define PACKET_OFFSET_TIMESTAMP   16
#define PACKET_OFFSET_ORIGINATOR  20
#define PACKET_OFFSET_RESOURCE    22
#define PACKET_OFFSET_DEBITINFO   24
#define PACKET_OFFSET_PACKETID    28
#define PACKET_OFFSET_NBRPACKETS  30
#define PACKET_OFFSET_RESENDNBR   31
#define PACKET_OFFSET_MAPSETID    32
#define PACKET_OFFSET_REQUEST_TAG 36
#define PACKET_OFFSET_TIMEOUT     40
#define PACKET_OFFSET_CPUTIME     41
// Packet header size (bytes)
#  define   HEADER_SIZE          48    
// RequestPacket header size (bytes)
#  define   REQUEST_HEADER_SIZE  (HEADER_SIZE + 4)
// ReplyPacket header size (bytes)
#  define   REPLY_HEADER_SIZE    (HEADER_SIZE + 4)
// Max packet size for receiving OK
#  define   MAX_PACKET_SIZE      65000 
// SubTypes highest bit
#  define   CTRL_PACKET_BIT      32768 

/**
  *   An encapsulation of a packet used in the MC2-system.
  *
  *   @remark It is only the superclass Packet that is allowed to 
  *           have attributes!!
  *
  *   Format of packet header (header size = 30):
  *   @packetdesc
  *      @row 0   @sep 3 bytes @sep Tag ("MC2")                @endrow
  *      @row 3   @sep 1 byte  @sep Protocol major version     @endrow
  *      @row 3   @sep 1 byte  @sep Protocol major version     @endrow
  *      @row 4   @sep 1 nibble @sep protocol minor version    @endrow
  *      @row 4.5 @sep 1 nibble @sep priority                  @endrow
  *      @row 5   @sep 1 byte  @sep packetNbr (index of nbrPackets)@endrow
  *      @row 6   @sep 2 bytes @sep subtype                    @endrow
  *      @row 8   @sep 4 bytes @sep origin IP                  @endrow
  *      @row 12  @sep 2 bytes @sep origin port                @endrow
  *      @row 14  @sep 2 bytes @sep request ID                 @endrow
  *      @row 16  @sep 4 bytes @sep request timestamp          @endrow
  *      @row 20  @sep 2 bytes @sep request originator         @endrow
  *      @row 22  @sep 2 bytes @sep RESERVED for future use    @endrow
  *      @row 24  @sep 4 bytes @sep debiting info              @endrow
  *      @row 28  @sep 2 bytes @sep packet ID                  @endrow
  *      @row 30  @sep 1 byte  @sep nbrPackets                 @endrow
  *      @row 31  @sep 1 byte  @sep resend nbr.                @endrow
  *      @row 32  @sep 4 bytes @sep mapSet ID                  @endrow
  *      @row 36  @sep 4 bytes @sep Request Tag                @endrow
  *   @endpacketdesc
  *
  */
class Packet: private NotCopyable
{
   friend class PacketPool;

   public:
      typedef uint16 PacketID;
      typedef uint16 RequestID;
      typedef uint32 MapID;
      /**
       *    The supported number of priorities in the packets. This
       *    is the maximum number of different priorities that could
       *    be used for the packets.
       */
      static const uint32 m_nbrPriorities;
      static const MapID MAX_MAP_ID;
      static const RequestID MAX_REQUEST_ID;
      /**
       *    The packettypes used internally in the server.
       */
      enum packetType {
         PACKETTYPE_ACKNOWLEDGE                             = 1,
         PACKETTYPE_MAPREPLY                                = 17,
         PACKETTYPE_ROUTEREPLY                              = 19,
         PACKETTYPE_COORDINATEREPLY                         = 22,
         PACKETTYPE_TOPREGIONREPLY                          = 23,
         PACKETTYPE_VANILLASEARCHREQUEST                    = 26,
         PACKETTYPE_VANILLASEARCHREPLY                      = 27,
         PACKETTYPE_USERSEARCHREQUEST                       = 28,
         PACKETTYPE_USERSEARCHREPLY                         = 29,
         PACKETTYPE_EXPANDROUTEREQUEST                      = 33,
         PACKETTYPE_EXPANDROUTEREPLY                        = 34,
         PACKETTYPE_LOADMAPREQUEST                          = 35,
         PACKETTYPE_SUBROUTE                                = 36,
         PACKETTYPE_TESTREQUEST                             = 41,
         PACKETTYPE_TESTREPLY                               = 42,
         PACKETTYPE_USERGETDATAREQUEST                      = 43,
         PACKETTYPE_USERGETDATAREPLY                        = 44,
         PACKETTYPE_USERVERIFYREQUEST                       = 45,
         PACKETTYPE_USERVERIFYREPLY                         = 46,
         PACKETTYPE_USERLOGOUTREQUEST                       = 47,
         PACKETTYPE_USERLOGOUTREPLY                         = 48,
         PACKETTYPE_USERSESSIONCLEANUPREQUEST               = 49,
         PACKETTYPE_USERSESSIONCLEANUPREPLY                 = 50,
         PACKETTYPE_USERCHECKPASSWORDREQUEST                = 51,
         PACKETTYPE_USERCHECKPASSWORDREPLY                  = 52,
         PACKETTYPE_USERFINDREQUEST                         = 53,
         PACKETTYPE_USERFINDREPLY                           = 54,
         PACKETTYPE_GETCELLULARPHONEMODELDATAREQUEST        = 55,
         PACKETTYPE_GETCELLULARPHONEMODELDATAREPLY          = 56,
         // Unused 57,58
         PACKETTYPE_DELETEMAPREQUEST                        = 63,
         // DELETEMAPREPLY moved to ctrl packet
         PACKETTYPE_PROXIMITYPOSITIONREQUESTPACKET          = 65,
         PACKETTYPE_PROXIMITYITEMSETREQUESTPACKET           = 66,
         PACKETTYPE_PROXIMITYREPLYPACKET                    = 67,
         PACKETTYPE_COVEREDIDSREPLYPACKET                   = 68,
         PACKETTYPE_PROXIMITYSEARCHREQUEST                  = 69,
         PACKETTYPE_PROXIMITYSEARCHREPLY                    = 70,
         /// For the SMSModule and friends
         PACKETTYPE_SMSREQUEST                              = 71,
         PACKETTYPE_SMSREPLY                                = 72,
         PACKETTYPE_SMSLISTENREPLY                          = 77,
         PACKETTYPE_SMSLISTENREQUEST                        = 80,
         PACKETTYPE_SUBROUTEREQUEST                         = 83,
         PACKETTYPE_DEBITREQUEST                            = 84,
         PACKETTYPE_DEBITREPLY                              = 85,
         PACKETTYPE_OVERVIEWSEARCHREQUESTPACKET             = 86,
         PACKETTYPE_OVERVIEWSEARCHREPLYPACKET               = 87,
         PACKETTYPE_COORDINATEONITEMREQUEST                 = 88,
         PACKETTYPE_COORDINATEONITEMREPLY                   = 89,
         PACKETTYPE_EXPANDITEMREQUEST                       = 90,
         PACKETTYPE_EXPANDITEMREPLY                         = 91,
         PACKETTYPE_SEARCHSELECTIONREPORT                   = 96,
         // Unused 99,100
         PACKETTYPE_SENDEMAILREQUEST                        = 101,
         PACKETTYPE_SENDEMAILREPLY                          = 102,
         PACKETTYPE_UPDATETRAFFICCOSTREPLY                  = 103,
         PACKETTYPE_ADDDISTURBANCEREQUEST                   = 104,
         PACKETTYPE_ADDDISTURBANCEREPLY                     = 105,
         PACKETTYPE_REMOVEDISTURBANCEREQUEST                = 106,
         PACKETTYPE_REMOVEDISTURBANCEREPLY                  = 107,
         PACKETTYPE_UPDATETRAFFICCOSTREQUEST                = 110,
         PACKETTYPE_DISTURBANCESUBSCRIPTIONREQUEST          = 113,
         PACKETTYPE_DISTURBANCESUBSCRIPTIONREPLY            = 114,
         PACKETTYPE_GETUSERNAVDESTINATIONREQUESTPACKET      = 124,
         PACKETTYPE_GETUSERNAVDESTINATIONREPLYPACKET        = 125,
         PACKETTYPE_SORTDISTREQUEST                         = 129,
         PACKETTYPE_SORTDISTREPLY                           = 130,
         PACKETTYPE_EXPANDCATEGORY2SEARCHREQUEST            = 131, 
         PACKETTYPE_USER_AUTH_REQUEST                       = 134,
         PACKETTYPE_USER_AUTH_REPLY                         = 135,
         PACKETTYPE_ROUTEEXPANDITEMREQUEST                  = 136,
         PACKETTYPE_ROUTEEXPANDITEMREPLY                    = 137,
         PACKETTYPE_GETTMCCOORREQUEST                       = 138,
         PACKETTYPE_GETTMCCOORREPLY                         = 139,
         PACKETTYPE_ROUTESTORAGE_GET_ROUTE_REQUEST          = 140,
         PACKETTYPE_ROUTESTORAGE_GET_ROUTE_REPLY            = 141,
         PACKETTYPE_ITEM_NAMES_REQUEST                      = 142,
         PACKETTYPE_ITEM_NAMES_REPLY                        = 143,
         PACKETTYPE_ADDMAPUPDATE_REQUEST                    = 144,
         PACKETTYPE_ADDMAPUPDATE_REPLY                      = 145,
         PACKETTYPE_ITEMINFO_REQUEST                        = 146,
         PACKETTYPE_ITEMINFO_REPLY                          = 147,
         PACKETTYPE_GFXFEATUREMAP_IMAGE_REQUEST             = 148,
         PACKETTYPE_GFXFEATUREMAP_IMAGE_REPLY               = 149,
         PACKETTYPE_EDGENODESREQUEST                        = 154,
         PACKETTYPE_EDGENODESREPLY                          = 155,
         PACKETTYPE_IDTRANSLATIONREQUEST                    = 156,
         PACKETTYPE_IDTRANSLATIONREPLY                      = 157,
         PACKETTYPE_USERFAVORITES_REQUEST                   = 158,
         PACKETTYPE_USERFAVORITES_REPLY                     = 159,
         PACKETTYPE_MAPSUBSCRIPTIONREQUEST                  = 160,
         PACKETTYPE_MAPSUBSCRIPTIONHEARTBEATREQUEST         = 161,
         PACKETTYPE_TEST_PUSH                               = 162,
         PACKETTYPE_UPDATEDISTURBANCEREQUEST                = 165,
         PACKETTYPE_UPDATEDISTURBANCEREPLY                  = 166,
         PACKETTYPE_SHUTDOWN                                = 167,
         PACKETTYPE_ADDUSERTRACK_REQUEST                     = 168,
         PACKETTYPE_ADDUSERTRACK_REPLY                       = 169,
         PACKETTYPE_GETUSERTRACK_REQUEST                     = 170,
         PACKETTYPE_GETUSERTRACK_REPLY                       = 171,
         PACKETTYPE_SEARCHEXPANDITEMREQUEST                  = 172,
         PACKETTYPE_SEARCHEXPANDITEMREPLY                    = 173,
         PACKETTYPE_MATCHINFOREQUEST                         = 174,
         PACKETTYPE_MATCHINFOREPLY                           = 175,
         PACKETTYPE_SEARCHREQUEST                            = 176,
         PACKETTYPE_WFACTIVATIONREQUEST                      = 177,
         PACKETTYPE_WFACTIVATIONREPLY                        = 178,
         PACKETTYPE_QUEUESTARTREQUEST                        = 179,
         PACKETTYPE_QUEUESTARTREPLY                          = 180,
         PACKETTYPE_ROUTETRAFFICCOSTREQUEST                  = 181,
         PACKETTYPE_ROUTETRAFFICCOSTREPLY                    = 182,
         PACKETTYPE_EXTERNALSEARCH_REQUEST                   = 183,
         PACKETTYPE_EXTERNALSEARCH_REPLY                     = 184,
         PACKETTYPE_EXTERNALITEMINFO_REQUEST                 = 185,
         PACKETTYPE_TILEMAP_REQUEST                          = 186,
         PACKETTYPE_TILEMAP_REPLY                            = 187,
         PACKETTYPE_EXPAND_REQUEST                           = 188,
         PACKETTYPE_EXPAND_REPLY                             = 189,
         PACKETTYPE_TRAFFICPOINTREQUESTPACKET                = 192,
         PACKETTYPE_TRAFFICPOINTREPLYPACKET                  = 193,
         PACKETTYPE_GFXFEATUREMAPREQUEST                     = 194,
         PACKETTYPE_GFXFEATUREMAPREPLY                       = 195,
         PACKETTYPE_INFOCOORDINATEREQUEST                    = 196,
         PACKETTYPE_INFOCOORDINATEREPLY                      = 197,
         // POI review
         PACKETTYPE_POIREVIEW_ADD_REQUEST                    = 198,
         PACKETTYPE_POIREVIEW_ADD_REPLY                      = 199,
         PACKETTYPE_POIREVIEW_DELETE_REQUEST                 = 200,
         PACKETTYPE_POIREVIEW_DELETE_REPLY                   = 201,
         PACKETTYPE_POIREVIEW_LIST_REQUEST                   = 202,
         PACKETTYPE_POIREVIEW_LIST_REPLY                     = 203,
         PACKETTYPE_PERIODIC_REQUEST                         = 204,
         PACKETTYPE_PERIODIC_REPLY                           = 205,
         PACKETTYPE_SEND_DATA_REQUEST                        = 206,
         PACKETTYPE_SEND_DATA_REPLY                          = 207,
         // Unused 208,209 here
         PACKETTYPE_TILEMAPPOI_REQUEST                       = 210,
         PACKETTYPE_TILEMAPPOI_REPLY                         = 211,
         // Stored user data packets
         PACKETTYPE_GET_STORED_USER_DATA_REQUEST             = 212,
         PACKETTYPE_GET_STORED_USER_DATA_REPLY               = 213,
         PACKETTYPE_SET_STORED_USER_DATA_REQUEST             = 214,
         PACKETTYPE_SET_STORED_USER_DATA_REPLY               = 215,

         PACKETTYPE_LICENCE_TO_REQUEST                       = 216,
         PACKETTYPE_LICENCE_TO_REPLY                         = 217,
         PACKETTYPE_LISTDEBITREQUESTPACKET                   = 218,
         PACKETTYPE_LISTDEBITREPLYPACKET                     = 219,         

         PACKETTYPE_DISTURBANCE_CHANGESET_REQUEST            = 220,
         PACKETTYPE_DISTURBANCE_CHANGESET_REPLY              = 221,

         PACKETTYPE_FETCH_ALL_DISTURBANCES_REQUEST           = 222,
         PACKETTYPE_FETCH_ALL_DISTURBANCES_REPLY             = 223,
         PACKETTYPE_EXTSERVICE_LIST_REQUEST                  = 224,
         PACKETTYPE_EXTSERVICE_LIST_REPLY                    = 225,

         /// Reply to this is PACKETTYPE_ITEMINFO_REPLY
         PACKETTYPE_GET_ITEM_INFO_REQUEST                    = 226,
         /// This is just a placeholder is not used.
         PACKETTYPE_GET_ITEM_INFO_REPLY                      = 227,

         /// Packet to move an idKey to a user.
         PACKETTYPE_ID_KEY_TO_REQUEST                        = 228,
         PACKETTYPE_ID_KEY_TO_REPLY                          = 229,


         // The ctrl-bit is set (32768..65536) --> packet handled by leader

         /// Indicate that the leader is alive.
         PACKETTYPE_HEARTBEAT                               = 32769, 
         /// Initiate new election of leader.
         PACKETTYPE_VOTE                                    = 32770,
         /// Module to be ignored (from leader).
         PACKETTYPE_IGNORE                                  = 32771, 
         /// Loadinfo to leader (reply to heartbeat).
         PACKETTYPE_STATISTICS                              = 32772, 
         
         // Top region request
         PACKETTYPE_TOPREGIONREQUEST                        = 32776,
         PACKETTYPE_ALLMAPREQUEST                           = 32777,
         PACKETTYPE_ALLMAPREPLY                             = 32778,
         /// Send from avail. to leader when map loaded successfully.
         PACKETTYPE_LOADMAPREPLY                            = 32779,   
         PACKETTYPE_COORDINATEREQUEST                       = 32780,
         /// IDs in a Area.
         PACKETTYPE_COVEREDIDSREQUESTPACKET                 = 32781,
         /// SMS Add listener request.
         PACKETTYPE_SMSSTATISTICS                           = 32782,
         PACKETTYPE_SUBROUTEREPLY                           = 32783,
         /// User writes.
         PACKETTYPE_USERADDREQUEST                          = 32784,
         PACKETTYPE_USERADDREPLY                            = 32785,
         PACKETTYPE_USERDELETEREQUEST                       = 32786,
         PACKETTYPE_USERDELETEREPLY                         = 32787,
         PACKETTYPE_USERCHANGEREQUEST                       = 32788,
         PACKETTYPE_USERCHANGEREPLY                         = 32789,
         PACKETTYPE_ADDCELLULARPHONEMODELREQUESTPACKET      = 32790,
         PACKETTYPE_ADDCELLULARPHONEMODELREPLYPACKET        = 32791,
         PACKETTYPE_CHANGEUSERPASSWORDREQUESTPACKET         = 32792,
         PACKETTYPE_CHANGEUSERPASSWORDREPLYPACKET           = 32793,
         PACKETTYPE_CHANGECELLULARPHONEMODELREQUESTPACKET   = 32794,
         PACKETTYPE_CHANGECELLULARPHONEMODELREPLYPACKET     = 32795,
         // Removed ctrl bit from LISTDEBIT packet 
         PACKETTYPE_ADDUSERNAVDESTINATIONREQUESTPACKET      = 32801,
         PACKETTYPE_ADDUSERNAVDESTINATIONREPLYPACKET        = 32802, 
         PACKETTYPE_DELETEUSERNAVDESTINATIONREQUESTPACKET   = 32808,
         PACKETTYPE_DELETEUSERNAVDESTINATIONREPLYPACKET     = 32803,
         PACKETTYPE_CHANGEUSERNAVDESTINATIONREQUESTPACKET   = 32804,
         PACKETTYPE_CHANGEUSERNAVDESTINATIONREPLYPACKET     = 32805,
         PACKETTYPE_ALLCOUNTRYREQUEST                       = 32806,
         PACKETTYPE_ALLCOUNTRYREPLY                         = 32807,
         PACKETTYPE_ROUTESTORAGE_ADD_ROUTE_REQUEST          = 32811,
         PACKETTYPE_ROUTESTORAGE_ADD_ROUTE_REPLY            = 32812,
         PACKETTYPE_USER_CREATE_SESSION_REQUEST             = 32813,
         PACKETTYPE_USER_CREATE_SESSION_REPLY               = 32814,
         PACKETTYPE_LEADERIP_REQUEST                        = 32819,
         PACKETTYPE_LEADERIP_REPLY                          = 32820,

         /// Moved from non ctrl packs.
         PACKETTYPE_DELETEMAPREPLY                          = 32821,
         PACKETTYPE_STREET_SEGMENT_ITEMREQUEST              = 32822,
         PACKETTYPE_STREET_SEGMENT_ITEMREPLY                = 32823,
         PACKETTYPE_DISTURBANCEREQUEST                      = 32824,
         PACKETTYPE_DISTURBANCEREPLY                        = 32825,
         PACKETTYPE_DISTURBANCEPUSH                         = 32826,
         // 
         PACKETTYPE_ALIVEREQUEST                            = 32827,
         PACKETTYPE_ALIVEREPLY                              = 32828,

         /// Control packet since the MapReader knows map version.
         PACKETTYPE_MAPREQUEST                              = 32829,
         
         PACKETTYPE_SMALLESTROUTINGCOSTREQUEST              = 32830,
         PACKETTYPE_SMALLESTROUTINGCOSTREPLY                = 32831,

         PACKETTYPE_ROUTINGINFOREQUEST                      = 32832,
         PACKETTYPE_ROUTINGINFOREPLY                        = 32833,

         PACKETTYPE_ROUTESTORAGE_CHANGE_ROUTE_REQUEST       = 32836,
         PACKETTYPE_ROUTESTORAGE_CHANGE_ROUTE_REPLY         = 32837,

         PACKETTYPE_TRANSACTION_REQUEST                     = 32838,
         PACKETTYPE_TRANSACTION_REPLY                       = 32839,

         PACKETTYPE_TRANSACTION_DAYS_REQUEST                = 32840,
         PACKETTYPE_TRANSACTION_DAYS_REPLY                  = 32841,
         PACKETTYPE_DISTURBANCEINFOREQUEST                  = 32842,
         PACKETTYPE_DISTURBANCEINFOREPLY                    = 32843,
         PACKETTYPE_BBOXREQUEST                             = 32844,
         PACKETTYPE_BBOXREPLY                               = 32845,

         PACKETTYPE_COPYRIGHTBOX_REQUEST                    = 32846,
         PACKETTYPE_COPYRIGHTBOX_REPLY                      = 32847,

      };

      /**
       * Max UDP packet Size.
       */
      static const uint32 MAX_UDP_PACKET_SIZE;

      /**
       *  Defines the limit size for sending the packets with TCP
       *  instead of UDP. If the packet is larger than this limit
       *  then it's sent with TCP.
       */
      static uint32 getTCPLimitSize();
  
      /**
       *   Packet id that tells the Requests that the id
       *   must be set.
       */
      static const uint16 NO_PACKET_ID = MAX_UINT16;
      
      /**
       *   Create an empty packet with specified buffer length.
       *   @param   bufLength The length of the buffer, created in 
       *                        the constructor.
       */
      Packet(uint32 bufLength);
      
      /**
       *   Create a packet and fill it with a given buffer.
       *   @param   buf         The preallocated buffer. This must be
       *                        alligned (created like 
       *                        buf = (byte*) new uint32[x]).
       *   @param   bufLength   The length of the byte-buffer.
       *   @param   nodeleteandinit If true, the buffer will not be inited
       *                            or deleted.
       */
      Packet(byte* buf, uint32 bufLength, bool nodeleteandinit = false );

      /** 
        *   Create a packet of given size, and with a lot of parameters
        *   in its head set.
        *   @param   bufLength   The length of the buffer inside the packet.
        *   @param   prio        The priority-filed of the packet.
        *   @param   subType     The type of the packet (see packetType).
        *   @param   ip          IP-adress of the originator.
        *   @param   port        Port of the originator.
        *   @param   packetId    The ID of this packet.
        *   @param   deb         The value in the debiting field.
        */
      Packet(uint32 bufLength, byte prio, uint16 subType, uint32 ip, 
	          uint16 port, PacketID packetId, uint32 deb);
      
      /** 
       *   Create a packet of given size, and with a lot of parameters
       *   in its head set. This constructor is mainly used by the 
       *   RequestPacket-subclass.
       *   @param   bufLength   The length of the buffer inside the packet.
       *   @param   prio        The priority-filed of the packet.
       *   @param   subType     The type of the packet (see packetType).
       *   @param   packetId    The ID of this packet.
       *   @param   requestID   The ID of the request that created this 
       *                        packet.
       */
      Packet( uint32 bufLength, byte prio, uint16 subType,
              PacketID packetId, RequestID requestID );      
      
      /** 
       *   Create a packet of given size, and with a lot of parameters
       *   in its head set.
       *   @param   bufLength   The length of the buffer inside the packet.
       *   @param   prio        The priority-filed of the packet.
       *   @param   subType     The type of the packet (see packetType).
       *   @param   ip          IP-adress of the originator.
       *   @param   port        Port of the originator.
       *   @param   packetId    The ID of this packet.
       *   @param   requestID   The ID of the request that created this 
       *                        packet.
       *   @param   deb         The value in the debiting field.
       */
      Packet(uint32 bufLength, byte prio, uint16 subType,
             uint32 ip, uint16 port, PacketID packetId,
             RequestID requestId, uint32 deb );
      
      /**
       * Create a packet from a buffer. Reads subtype and
       * creates right class if it is supported by this function
       * otherwise it makes a Packet.
       * Gives buff to a packet.
       * 
       * @param buff The packetbuffer with packet header.
       * @param buffLength The length of the buffer.
       * @param time the time out in seconds
       * @return A new packet with buff as buffer.
       */
      static Packet* makePacket( byte* buff, uint32 buffLength );
      
      /**
       *   The destructor. Returns the buffer to the OS;
       */
      virtual ~Packet();
      
      /**
       *   Get the length of the packet. 
       *   @remark It is not the length of the buffer that is returned!
       *   @return  The length of the packet, <B>not the buffer</B>.
       */
      inline uint32 getLength() const;
      
      /** 
       *   Sets the length of the packet, <B>not the buffer</B>.
       *   @param   l  The length of this packet.
       */ 
      inline void setLength(uint32 l);
      
      /**
       *   Get the maximum size of the buffer.
       *   @return  Maximum size of the buffer.
       */
      inline uint32 getBufSize() const;
	   
      /**
       *   Get a pointer to the start of the internal buffer in this 
       *   packet.
       *   @return  Pointer to the start of the packet buffer.
       */
      inline byte* getBuf() const;
      
      /**
       *   Get a pointer to the byte after the Packets header
       *   of the buffer, subclasses header sizes are NOT skipped so
       *   you get the first byte of the subclass header if it has any.
       *
       *   @return  Pointer to the byte at HEADER_SIZE in the buffer.
       */
      inline byte* getDataAddress() const;
      
      /**
       *   Get the protocolversion used in this packet.
       *   @return  The version of the protocol, the major version is in the
       *            high 8 bits, the minor in the lowest 4.
       */
      inline uint16 getProtocolVersion() const;
      
      /** 
       *   Set the protocol version field.
       *   @param   majorProt    Protocol major version used in this packet.
       *   @param   minorProt    Protocol minor version used in this packet.
       */
      inline void setProtocolVersion(byte majorProt, byte minorProt);
      
      /**
       *   Get the priority of this packet.
       *   @return  The priority of the packet.
       */
      inline byte getPriority() const;
      
      /**
       *   Set the priority of this packet.
       *   @param   pr    The new priority of this packet.
       */
      inline void setPriority(byte pr);
      
      /**
       *   Get the type of the packet.
       *   @see     packetType for details about the returned value.
       *   @return  The type of the packet.
       */
      inline packetType getSubType() const;

      
      /**
       *   Extracts subType from a buffer with a packet header.
       * @param buff The buffer to read subtype from.
       */
      static inline packetType getSubType( byte* buff );

      /**
       *   Set the tag of the packet
       *   @param   tag    Packet tag.
       */
      inline void setPacketTag( const char* tag );

      /**
       *   Sets the extra resource if maps aren't
       *   used for selecting modules.
       */
      inline void setResourceID( uint16 res );
      
      /**
       *   Gets the extra resource if maps aren't
       *   used for selecting modules.
       */
      inline uint16 getResourceID() const;
      
      /**
       *   Set the type of the packet.
       *   @param   st    Packet type.
       */
      inline void setSubType(uint16 st);
		
      /**
        *   Get the type of this packet as a nullterminated string.
        *   @return  Pointer to a nullterminated string containing 
        *            (sub)Type of the packet.
        */
      const char *getSubTypeAsString() const;

      /** 
        *   The IP of the sender of this packet (the one who should be sent
        *   the reply).
        *   @return  The IP of the sender.
        */
      inline uint32 getOriginIP() const;

      /** 
        *   Set the server IP 
        *   @param   ip The if of the origin.
        */
      inline void setOriginIP(uint32 ip);

      /**
        *   The port of the sender of this packet (the one who should 
        *   be sent the reply).
        *   @return The port of the sender. 
        */
      inline uint16 getOriginPort() const;
      
      /** 
        *   Set the port of the sender. 
        *   @param   pn The new portnumber of the originator.
        */
      inline void setOriginPort(uint16 pn);

      /**
       *    Returns the origin address, i.e. ip and port.
       */
      IPnPort getOriginAddr() const;

      /**
       *    Sets the origin address, i.e. ip and port.
       */
      void setOriginAddr( const IPnPort& addr );
      
      /**
        *   Get the ID of the request where this packet where created.
        *   @return The ID of the request. The modules don't change this.
        */
      inline RequestID getRequestID() const;

      /**
        *   Set the request ID.
        *   @param   rid   The new request ID of this packet.
        */
      inline void setRequestID(RequestID rid);
      
      /**
        *   Get the timestamp.
        *   @return The ID of the request. The modules don't change this.
        */
      inline uint32 getRequestTimestamp() const;

      /**
        *   Set the timestamp.
        *   Old, now deprecated version, sets a subset of the new
        *   larger ID.
        *   @param   time   The new timestamp
        */
      inline void setRequestTimestamp(uint32 timestamp);
      
      /**
        *   Get the originator of the request, lowest 8 bits = type
        *   (servertype_t), highest 8 bits currently reserved.
        *   @return The ID of the request. The modules don't change this.
        */
      inline uint16 getRequestOriginator() const;

      /**
        *   Set the originator of the request, lowest 8 bits = type
        *   (servertype_t), highest 8 bits currently reserved.
        *   @param   origin   The new originator
        */
      inline void setRequestOriginator(uint16 origin);

      /**
       *    Copies the originator, timestamp and request id
       *    from the supplied packet.
       *    If the <code>other</code> is NULL, nothing will
       *    happen.
       */
      void copyRequestInfoFrom( const Packet* other );
      
      /**
        *   Get the ID of this packet.
        *   @return Packet ID.
        */
      inline PacketID getPacketID() const;

      /**
        *   Set packet ID.
        *   @param   The id of the packet.
        */
      inline void setPacketID(PacketID pi);

      /**
        *   Get the debiting information for this packet.
        *   @return Debiting information.
        */
      inline uint32 getDebInfo() const;

      /**
        *   Set debiting information.
        *   @param   de The new debiting information.
        */
      inline void setDebInfo(uint32 de);

   /**
    * Sets cpu time
    * @param time in milli seconds
    */
   inline void setCPUTime( uint32 time );
   /// @return cpu time
   inline uint32 getCPUTime() const;
      /**
       *    Get packetNbr.
       *    @return the part number of the packet.
       */
      inline byte getPacketNbr() const;

      /**
       *    Set packetNbr.
       *    @param nbr the part number of the packet.  
       */
      inline void setPacketNbr( byte nbr );

      /**
       *    Get nbrPackets.
       *    @return the number of parts the packet consists of.
       */
      inline byte getNbrPackets() const;

      /**
       *    Set nbrPakets.
       *    @param nbr the number of parts the packet consists of.
       */
      inline void setNbrPackets( byte nbr );
 
      /**
       *    Get resendNbr.
       *    @return The resend number of the packet.
       */
      inline byte getResendNbr() const;

      /**
       *    Set ResendNbr.
       *    @param nbr The resend number of the packet.  
       */
      inline void setResendNbr( byte nbr );

      /**
       *   Get the map set id of this packet
       *   @return  The map set id, MAX_UINT32 if not used
       */
      inline uint32 getMapSet() const;
      
      /**
       *   Set the map set id of this packet
       *   @param  mapSet The map set id, MAX_UINT32 if not used
       */
      inline void setMapSet(uint32 mapSet);

      /**
       *   Get the request tag for this packet
       *   @return  The request tag
       */
      inline uint32 getRequestTag() const;
      
      /**
       *   Set the request tag for this packet
       *   @param  requestTag The request tag
       */
      inline void setRequestTag(uint32 requestTag);

      /** 
        *   Write an aligned 32 bit big endian integer to the packet 
        *   buffer and increase the position. The written data is 
        *   always aligned
        *   
        *   @param   position The position in the buffer, updated after
        *                     writing.
        *   @param   value    The value to be written.
        *   @return  Number of bytes written
        */
      inline int incWriteLong(int &position, uint32 value);

      /**
       * Write an aligned 64 bit bgin endian integer to the packet
       * buffer and increase the position. The written data is always
       * aligned.

       * @param position The position in the buffer, updated after
       *                 writing.
       * @param value The vlaue to be written.
       * @return Number of bytes written. 
       */
      inline int incWriteLongLong(int& position, uint64 value);

      /**
       *    Writes a 32-bit float. Not tested on anything else
       *    than i386.
       */
      inline int incWriteFloat32( int& position, float32 value );
      
      /**
       *    Write zeroes and align to next long.
       *    @param position Position in packet to be aligned.
       *    @return Nbr of zero bytes written.
       */
      inline int incAlignWriteLong(int& position);
      
      /**
       *    Skip and align to next long.
       *    @param position Position in packet to be aligned.
       */
      inline void incAlignReadLong(int& position) const;
      
      /** 
        *   Write an aligned 16 bit big endian integer to the packet 
        *   buffer and increase the position.
        *
        *   @param   position The position in the buffer, updated after
        *                     writing.
        *   @param   value    The value to be written.
        *   @return  Number of bytes written.
        */
      inline int incWriteShort(int &position, uint16 value);

      /**
       *    Align to next short. Fill the gap with zeroes if write is true.
       *    @param position The position to be aligned.
       *    @return The number of zero bytes written.
       */
      inline int incAlignWriteShort(int& position);
      
      /**
       *    Align to next short. Skip bytes.
       *    @param position The position to be aligned.
       */
      inline void incAlignReadShort(int& position) const;
      
      /** 
        *   Write an byte (8 bit integer) to the packet buffer and 
        *   increase the position.
        *
        *   @param   position The position in the buffer, updated after
        *                     writing.
        *   @param   value    The value to be written.
        *   @return  Number of bytes written.
        */
      inline int incWriteByte(int &position, byte value);

      /** 
        *   Read an aligned 32 bit big endian integer from the packet 
        *   buffer and increase the position.
        *
        *   @param   position    The position in the buffer, will be 
        *                        increased.
        *   @return  The uint32 that was read.
        */
      inline uint32 incReadLong(int &position) const;
   
      /**
       * Read an aligned 64 bit big endian integer from the packet
       * buffer and increase the position.
       * @param poition The poition in the buffer, will be increased.
       * @return The uint64 that was read. 
       */
      inline uint64 incReadLongLong(int & position) const;

      /** 
        *   Read an aligned 32 bit float from the packet 
        *   buffer and increase the position. Only tested on i386.
        *
        *   @param   position    The position in the buffer, will be 
        *                        increased.
        *   @return  The uint32 that was read.
        */
      inline float32 incReadFloat32(int &position) const;

      /**
       *   Read an aligned 32 bit big endian integer from the packet 
       *   buffer and increase the position.
       *   <br />
       *   Of course this function assumes that you know what you are
       *   doing.
       * 
       *   @param   position    The position in the buffer, will be 
       *                        increased.
       *   @param   value       The value is put here.
       */
      template<class TYPE> void incReadLong(int& position, TYPE& value) const;
      
      /** 
        *   Read an aligned 16 bit big endian integer from the packet 
        *   buffer and update the position.
        *
        *   @param   position The position in the buffer, will be 
        *                     increased.
        *   @return The uint16 that was read.
        */
      inline uint16 incReadShort(int &position) const;

      /**
       *   Read an aligned 16 bit big endian integer from the packet 
       *   buffer and increase the position.
       *   <br />
       *   Of course this function assumes that you know what you are
       *   doing.
       * 
       *   @param   position    The position in the buffer, will be 
       *                        increased.
       *   @param   value       The value is put here.
       */
      template<class TYPE> void incReadShort(int& position, TYPE& value) const;

      /**
        *   Read a byte from the packet buffer and update the position.
        *   @param   position The position in the buffer, will be 
        *                     increased.
        *   @return  The byte that was read.
        */
      inline byte incReadByte(int &position) const;
      
       /** 
        *   Make str point to the null-terminated string starting at 
        *   position. Increase position so that it is after the string. 
        *
        *   NB: No copy is made. The pointer points into the packet.
        *                        Deleting the packet deletes the string.
        *
        *   @param   position    Position in the buffer to read from. 
        *                        Will be increased.
        *   @param   str         Points to the null-terminated string 
        *                        inside the packet.
        *   @return The length of the string.
        */
      inline int incReadString(int &position, char* &str) const;

      /** 
        *   Make str point to the null-terminated string starting at 
        *   position. Increase position so that it is after the string. 
        *
        *   NB: No copy is made. The pointer points into the packet.
        *                        Deleting the packet deletes the string.
        *
        *   @param   position    Position in the buffer to read from. 
        *                        Will be increased.
        *   @return A string.
        */
      inline const char* incReadString(int &position) const;

      /**
       *    Reads the next string and puts it into <code>string</code>.
       */
      inline int incReadString( int& pos, MC2String& string ) const;

       /** 
        *   Returns a pointer into the packet where the byte array
        *   starts and increases <code>position</code> past the array
        *   to read.
        *
        *   NB: No copy is made. The pointer points into the packet.
        *                        Deleting the packet deletes the array.
        * 
        *   @param   position    Position in the buffer to read from. 
        *                        Will be increased.
        *   @param   nbrBytes    The number of bytes to "read".
        *   @return  A pointer to the start of the bytearray.
        */
      inline const byte* incReadByteArray(int &position,
                                          int nbrBytes) const;
      

      /**
       *   Copies the data from the packet to the <code>dest</code>.
       *   @param position Position in the packet. Will be increased
       *                   to the position after the byte array.
       *   @param dest     The destination for the bytes.
       *   @param nbrBytes The size of the bytearray.
       *   @return <code>dest</code>.
       */
      inline byte* incReadByteArray(int &position,
                                    byte* dest,
                                    int nbrBytes) const;
           
      /**
       *    Reads an MC2BoundingBox from the databuffer. (4*4 bytes).
       *    Doesn't update the cosLat factor.
       *   @param position   Position in the buffer to write to. 
       *                     Will be increased.
       *    @param bbox The bouding box to put the result into.
       *    @return The number of bytes read.
       *
       */
      inline int incReadBBox(int& position, MC2BoundingBox& bbox) const;
      
      /** 
        *   Copy str to buffer at position.
        *
        *   @param position   Position in the buffer to write to. 
        *                     Will be increased.
        *   @param str        Buffer to read the string from.
        */
      inline void incWriteString(int &position, const char* str);
      
      /** 
        *   Copy str to buffer at position.
        *
        *   @param position   Position in the buffer to write to. 
        *                     Will be increased.
        *   @param str        String to write.
        */
      inline void incWriteString(int &position, const MC2String& str );

      /**
       *    Copy the bytearray pointed to by <code>bptr</code> into
       *    the packet.
       *    Note that this function must be used with caution. The
       *    byte order is kept, so you can't send endian-ordering
       *    dependant data using this method. Gif:s are OK.
       *    @param position   Position in the buffer to write to.
       *                      Will be increased by <code>nbrBytes</code>.
       *    @param bptr       Where to copy from.
       *    @param nbrBytes   Number of bytes to copy into the packet.
       */
      inline void incWriteByteArray(int &position,
                                   const byte* bptr,
                                   int nbrBytes);
      
      /**
       *    Writes a bounding box to the data buffer (4*4 bytes).
       *    @param position   Position in the buffer to write to.
       *                      Will be increased by <code>nbrBytes</code>.
       *    @param bbox The bounding box to write.
       *    @return The number of bytes written.
       */
      inline int incWriteBBox(int& position, const MC2BoundingBox& bbox);
      
      /** 
        *   Same as incWriteString except it does not check str for NULL.
        *   @param position   Position in the buffer to write to. 
        *                     Will be increased.
        *   @param str        Buffer to read the string from.
        */
      inline void quickIncWriteString( int &position,
                                       const char *str );
      
      /**
        *   Get a copy of this packet.
        *   @param fullBuffer If <code>fullBuffer</code> is <b>true</b>
        *                     the new packet will have a buffer as
        *                     large as the original. If <b>false</b>
        *                     the new buffer will be as large as the
        *                     length of the packet. Default is <b>true</b>
        *                     for backward compability.
        *   @return  A copy of the packet.
        */
      inline virtual Packet *getClone(bool fullBuffer = true) const;

      /**
       * Makes the packets bufferSize buffSize.
       * The buffer is never made shorter than the length of the packet.
       * The buffer is created long aligned.
       * @param newSize The size the packets buffer should be, is increased
       *                to getLength() if getLength() is more than newSize.
       */
      void resize( uint32 newSize );

      /** 
       *   Dumps the paket to stdout. The packet header is decoded but 
       *   the packet data is written in hex. 
       * 
       *   @param   headerOnly  If true dump only writes the packet  
       *            header 
       *   @param   do a dns lookup of the IP. Can be very time 
       *            consuming if the IP is invalid. 
       */ 
      virtual void dump(bool headerOnly = false,
                        bool lookupIP = false ) const; 


      /**
       *    Check if the packet has room for addSize more byte.
       *    If it doesn't then the size of the packet is incremented
       *    with the specified amount.
       *
       *    @param   addSize     The number of bytes to check if the packet
       *                         has room for.
       *    @param   resizeSize  The number of bytes that the size of 
       *                         the packet is incremented to.
       *    @return  True if the packet was resized, false otherwise.
       */
      inline bool updateSize(uint32 addSize, uint32 resizeSize);

      /**
       *    Sets the arrival time to now.
       */
      inline void setArrivalTime() {
         m_arrivalTime = TimeUtility::getCurrentTime();
      }

      /**
       *    Returns the arrival time.
       */
      inline uint32 getArrivalTime() const {
         return m_arrivalTime;
      }

      /**
       *    Returns time since arrival.
       */
      inline uint32 getTimeSinceArrival() const {
         return TimeUtility::getCurrentTime() - getArrivalTime();
      }

     /**
      *   @return timeout in seconds
      */
     inline uint32 getTimeout() const;

     /**
      *   @return true if there is a timeout
      */
     inline bool hasTimeout() const {
        return getTimeout() != 0;
     }

     /** 
      *   @return true if the package timed out or not
      */
     bool timedOut() const;

     /**
      *   Sets the timeout in seconds
      */
    inline void setTimeout( byte timeout );

   protected:
      /** 
        *   The data buffer of the packet 
        */
      byte *buffer;
      
      /** 
        *   Size of data buffer 
        */
      uint32 bufSize;

      /** 
        *   The length of the packet 
        */
      uint32 length;

      /// Arrival time
      uint32 m_arrivalTime;
      
      /// True if the buffer should be deleted
      bool m_deleteBuffer;

      /**
        *   Dumps the pakets data to stdout.
        *   @param startPos   Tells where the data starts used when
        *                     subheader is printed
        *
        */
      void dumpData( uint32 startPos ) const; 
      
      /** 
        *   Write an aligned 32 bit big endian integer to the packet 
        *   buffer.
        *
        *   @param   position The position in the buffer.
        *   @param   value    The value to be written.
        *   @return  Number of bytes read.
        */
      inline int writeLong(int position, uint32 value);

      /** 
        *   Almost the same as the incReadLong(..)-method, but buffer
        *   is assumed to be aligned. Read an aligned 32 bit integer 
        *   big endian from the packet 
        *   buffer and increase the position.
        *
        *   @param   position    The position in the buffer, will be 
        *                        increased.
        *   @return  The uint32 that was read.
        */
      inline uint32 quickIncReadLong(int &position) const;
      
      /** 
        *   Almost the same as the incReadShort(..)-method, but buffer
        *   is assumed to be aligned. Read an aligned 16 bit integer 
        *   big endian from the packet buffer and increase the position.
        *
        *   @param   position    The position in the buffer, will be 
        *                        increased.
        *   @return  The uint32 that was read.
        */
      inline uint16 quickIncReadShort(int &position) const;
      
      /**
        *   Similar to incWriteLong(...), but quicker(?). Position is 
        *   assumed to be pre-aligned, checking is currently performed,
        *   might be removed if too slow.
        *   @param   position The position in the buffer.
        *   @param   value    The value to be written.
        */
      inline void quickIncWriteLong( int &position, uint32 value );

      /**
        *   Similar to incWriteShort(...), but quicker(?). Position is 
        *   assumed to be pre-aligned, checking is currently performed,
        *   might be removed if too slow.
        *   @param   position The position in the buffer.
        *   @param   value    The value to be written.
        */
      inline void quickIncWriteShort(int &position, uint16 value);
      
      /** 
        *   Write an aligned 16 bit big endian integer to the packet 
        *   buffer.
        *   @param   position The position in the buffer.
        *   @param   value    The value to be written.
        *   @return Number of bytes read.
        */
      inline int writeShort(int position, uint16 value);
      
      /** 
        *   Write a byte to the packet buffer.
        *
        *   @param   position The position in the buffer.
        *   @param   value    The value to be written.
        *   @return  Number of bytes read (always 1).
        */
      inline int writeByte(int position, byte value);

      /**
        *   Read a byte from the packet buffer.
        *   @param   position The position in the buffer.
        *   @return  The byte that as read.
        */
      inline byte readByte(int position) const;

  public:
      /**
       *    Write a bit to the packet into byte <code>bytenumber</code>
       *    at bitposition <code>bitnumber</code>.
       *    @param bytepos The byte number in the packet to write the
       *                   bit to.
       *    @param bitnumber The position of the bit in the byte. 0 is LSB.
       *    @param value     The value to write.
       */
      inline void writeBit(int bytepos, int bitnumber, bool value);
      
      /**
       *    Read a bit from the packet buffer at position
       *    <code>bytepos</code>.
       *    @param bytepos The position in the packet.
       *    @param bitnumber The position of the bit inside the byte.
       *                     0 is LSB.
       *    @return true if the bit was set.
       */
      inline bool readBit(int bytepos, int bitnumber) const;

  protected:
      
      /** 
        *   Read an prealigned 32 bit big endian integer from the packet 
        *   buffer.
        *
        *   @param   position The prealigned position in the buffer.
        *   @return  The uint32 that was read.
        */
      inline uint32 quickReadLong(int position) const;
      
      /** 
        *   Read an aligned 32 bit big endian integer from the packet 
        *   buffer.
        *
        *   @param   position The position in the buffer.
        *   @return  The uint32 that was read.
        */
      inline uint32 readLong(int position) const;
      
      /** 
        *   Read an prealigned 16 bit big endian integer from the 
        *   packet buffer.
        *
        *   @param   position The prealigned position in the buffer.
        *   @return  The uint16 that was read 
        */
      inline uint16 quickReadShort(int position) const;
      
      /** 
        *   Read an aligned 16 bit big endian integer from the packet 
        *   buffer.
        *
        *   @param   position The position in the buffer.
        *   @param   align    If align is true then position will be 
        *                     aligned before reading.
        *   @return  The uint16 that was read 
        */
      inline uint16 readShort(int position) const;

  protected:
      bool shortAligned(int pos) const {
         uint8* ptr = buffer + pos;
         uintptr_t iPtr = (uintptr_t)(ptr);
         return ! ( iPtr & 1 );
      }

      bool longAligned(int pos) const {
         uint8* ptr = buffer + pos;
         uintptr_t iPtr = (uintptr_t)(ptr);
         return ! ( iPtr & 3 );
      }

      inline void assertInside( const char* func,
                                int pos, int max ) const {
         if ( pos >= max ) {
            mc2log << "[Packet]: " << func << " - outside buffer "
                   << pos << " >= " << max << endl;
            MC2_ASSERT( pos < max );
         }
      }
      
  private:
      /**
        *   Pointer used by the PacketPool to maintain the packets
        *   allocated there.
        */
      Packet* next;
};

#define ASSERT_INSIDE(pos) assertInside( __FUNCTION__, pos, getBufSize())

/**
  *   Subclass of packet that is superclass of all RequestPackets.
  *   The header in class Packet is extended with one field that
  *   contains the map id that this request is about. No request
  *   can involve more than one map. In some rare cases the map is 
  *   unknown, then MAX_UINT32 is used as map id.
  *
  *   Format of @p RequestPacket subheader:
  *   @packetdesc
  *      @row HEADER_SIZE @sep 4 bytes 
  *            @sep ID of the map this request conserns. @endrow
  *   @endpacketdesc
  *   
  */
class RequestPacket : public Packet
{
   public:
      /**
       * The mapIDs position in the ReplyPacket.
       */
      static const uint32 REQUEST_PACKET_MAPID_POS;

      /**
        *   Creates an empty RequestPacket. The internal buffer is 
        *   created with MAX_PACKET_SIZE bytes.
        */
      RequestPacket(); 
      
      /**
       *   Create a RequestPacket and fill it with a given buffer.
       *   @param   buf         The preallocated buffer. This must be
       *                        alligned (created like 
       *                        buf = (byte*) new uint32[x]).
       *   @param   bufLength   The length of the byte-buffer.
       */
      RequestPacket(byte* buf, uint32 bufLength);
      
      /**
        *   Creates a RequestPacket with a specified length.
        *   @param   bufLength The length of the buffer, created in 
        *                        the constructor.
        */
      RequestPacket(uint32 bufLength);

      /**
       * Creates a RequestPacket with a specified length and type.
       *
       * @param bufLength The length of the buffer, created in 
       *                  the constructor.
       * @param subType The type of packet, see packetType.
       */
      RequestPacket( uint32 bufLength, uint16 subType );

      /**
        *   Creates a RequestPacket with the needed parameters.
        *
        *   @param   bufLength   The length of the internal buffer.
        *   @param   prio        Priority of this packet.
        *   @param   subType     Type of the packet, from the list in Packet.
        *   @param   packetId    ID of the packet to send.
        *   @param   requestID   ID of the request where this packet belong.
        *   @param   mapID       ID of the map this request is concerning.
        */
      RequestPacket( uint32 bufLength,
                     byte prio,
                     uint16 subType,
                     PacketID packetId,
                     RequestID requestID,
                     MapID mapID );

       /**
        *   Converts an already created packet into a RequestPacket.
        *   Useful when working with a packet pool!
        *   
        *   @param   prio        Priority of this packet.
        *   @param   subType     Type of the packet, from the list in Packet.
        *   @param   packetId    ID of the packet to send.
        *   @param   requestID   ID of the request where this packet belong.
        *   @param   mapID       ID of the map this request is concerning.
        */
      void init(  byte prio,
                  uint16 subType,
                  PacketID packetId,
                  RequestID requestID,
                  MapID mapID );

      /**
        *   Get the map id of this request.
        *   @return  MapID of this request packet.
        */
      inline MapID getMapID() const;

      /**
        *   Set the map id of this request.
        *   @param   mapID The mapID of this request packet.
        */
      inline void setMapID(MapID mapID);
};


/**
  *   Subclass of packet that is superclass of all ReplyPackets.
  *   The header in class Packet is extended with one field that
  *   contains the status of the reply.
  *
  *   Format of @p ReplyPacket subheader:
  *   @packetdesc
  *      @row HEADER_SIZE @sep 4 bytes @sep Status of this reply. @endrow
  *   @endpacketdesc
  *   
  */
class ReplyPacket : public Packet
{
   public:
      /**
       * The status position in the ReplyPacket.
       */
      static const uint32 REPLY_PACKET_STATUS_POS;

      /**
        *   Creates an empty ReplyPacket. The internal buffer is 
        *   created with MAX_PACKET_SIZE bytes.
        *   @see     getStatus
        */
      ReplyPacket(); 
      
      /**
       *   Create a ReplyPacket and fill it with a given buffer.
       *   @param   buf         The preallocated buffer. This must be
       *                        alligned (created like 
       *                        buf = (byte*) new uint32[x]).
       *   @param   bufLength   The length of the byte-buffer.
       */
      ReplyPacket(byte* buf, uint32 bufLength);
      
      /**
        *   Creates a ReplyPacket with a specified length.
        *   @see     getStatus
        *   @param   bufLength The length of the buffer, created in 
        *                        the constructor.
        */
      ReplyPacket(uint32 bufLength);

      /**
        *   Creates a ReplyPacket with a specified length and type.
        *
        *   @see     getStatus
        *   @param   bufLength   The length of the internal buffer.
        *   @param   subType     Type of the packet, from the list in 
        *                        Packet.
        */
      ReplyPacket(uint32 bufLength, uint16 subType ) ;

      /**
        *   Creates a ReplyPacket with the needed parameters.
        *
        *   @see     getStatus
        *   @param   bufLength   The length of the internal buffer.
        *   @param   subType     Type of the packet, from the list in 
        *                        Packet.
        *   @param   req         The corresponding request of this reply.
        *                        Used to set the common fields in the 
        *                        header.
        *   @param   status      The status of this request. 
        */
      ReplyPacket(uint32 bufLength, uint16 subType,
                  const RequestPacket* req, uint32 status);

      /**
        *   Converts an already created packet into a ReplyPacket.
        *   Useful when working with a packet pool!
        *   
        *   @see     getStatus
        *   @param   subType     Type of the packet, from the list in 
        *                        Packet.
        *   @param   req         The corresponding request of this 
        *                        reply. Used to set the common fields 
        *                        in the header.
        *   @param   status      The status of this request. 
        */
      void init(uint16 subtype, const RequestPacket* req, uint32 status);


      /**
        *   Get the status of this reply. The status is in terms of 
        *   stringcodes in <TT>class StringTable</TT>. The ``OK-string''
        *   (StringTable::OK) is used to indicate that the reply is 
        *   calculated alright.
        *
        *   @return  Status of this reply.
        */
      inline uint32 getStatus() const;

      /**
       *    Returns a pointer to a string in the StringTable describing
       *    the error code. (Hopefully).
       */
      const char* getStatusAsString() const;

      /**
        *   Set the status of this reply.
        *   @param   Status of this reply.
        *   @see     getStatus
        */
      inline void setStatus(uint32 status);

      /**
       *    Checks the status of the packet and prints the error
       *    if != OK.
       *    @param sender A string describing the position or state
       *                  of the program. Used for printing to
       *                  the screen.
       *    @return true if status was OK. false otherwise.
       */
      bool checkStatus(const char* sender);
};


// ========================================================================
//                                  Implementation of the inlined methods =

inline uint32 
Packet::getLength() const 
{ 
   return length; 
}

inline void 
Packet::setLength(uint32 l) 
{   
  length = l; 
}

inline uint32 
Packet::getBufSize() const 
{ 
   return bufSize; 
}

inline byte* 
Packet::getBuf() const 
{ 
   return buffer; 
}

inline byte* 
Packet::getDataAddress() const 
{ 
   return &buffer[HEADER_SIZE]; 
}


inline Packet*
Packet::getClone(bool fullBuffer) const
{
   // Set the length. 
   int newBufSize = fullBuffer ? getBufSize() : getLength();
   byte* buff = reinterpret_cast< byte* > ( 
      new uint32[ ( newBufSize + 3) >> 2 ] );

   // Only copy getLength() bytes to the new packet.
   memcpy( buff , getBuf(), getLength() );
   Packet *result = makePacket( buff, newBufSize );
   result->bufSize = newBufSize;
   result->length = getLength();
   result->m_arrivalTime = m_arrivalTime;
   result->m_deleteBuffer = true;
   return result;
}

inline bool
Packet::updateSize(uint32 addSize, uint32 resizeSize)
{
   if (getLength() + addSize < getBufSize()) {
      return (false);
   } else {
      resize(getLength() + resizeSize);
      return (true);
   }
}

// ----------- Read Long  -------------

inline uint32 
Packet::quickReadLong(int position) const 
{
   // ====================================================== 
   // This might be removed if the statement consumes too much cpu...
   MC2_ASSERT( longAligned( position ) );
   ASSERT_INSIDE( position );
   // ====================================================== 

   return ntohl(*((uint32 *)&(buffer[position])));
}

inline uint32 
Packet::readLong(int position) const 
{
   ASSERT_INSIDE( position );
   AlignUtility::alignLong(position);

   return quickReadLong( position );
}

inline uint32 
Packet::quickIncReadLong(int &position) const 
{
   uint32 tempLong = quickReadLong(position);
   position += sizeof(uint32);
   return tempLong;
}

inline uint32
Packet::incReadLong(int &position) const {
   AlignUtility::alignLong(position);
   return quickIncReadLong( position );
}

inline uint64 
Packet::incReadLongLong(int & position) const {
   uint64 val = incReadLong( position );
   val <<= 32;
   val |= incReadLong( position );
   return val;
}


inline float32
Packet::incReadFloat32( int& pos ) const {
   uint32 tmpVal = incReadLong( pos );
   return *reinterpret_cast<float32*>(&tmpVal);
}

template<class TYPE>
inline
void Packet::incReadLong(int& position, TYPE& value) const
{
   value = static_cast<TYPE>(incReadLong(position));
}

// ----------- Read Short -------------

inline uint16 
Packet::quickReadShort(int position) const 
{
   // ====================================================== 
   // This might be removed if the if-statement consumes too much cpu...
   MC2_ASSERT( shortAligned( position ) );
   ASSERT_INSIDE( position );
   // ====================================================== 

   return ntohs(*((uint16 *)&(buffer[position])));
}

inline uint16 
Packet::readShort(int position) const 
{
   AlignUtility::alignShort(position);
   return quickReadShort( position );
}

inline uint16 
Packet::quickIncReadShort(int &position) const 
{
   uint16 tempShort = quickReadShort(position);
   position += sizeof(uint16);
   return tempShort;
}

uint16
Packet::incReadShort(int &position) const {
   AlignUtility::alignShort(position);
   return  quickIncReadShort(position);
}

template<class TYPE>
inline
void Packet::incReadShort(int& position, TYPE& value) const
{
   value = static_cast<TYPE>(incReadShort(position));
}


// ----------- Read Byte  -------------

inline byte 
Packet::readByte(int position) const 
{ 
   return buffer[position]; 
}

inline byte
Packet::incReadByte(int &position) const
{
   return readByte(position++);
}

// ----------- Read Bit   -------------

inline bool
Packet::readBit(int bytepos,
                int bitnumber) const
{
   byte b = readByte(bytepos);
   return BitUtility::getBit(b, bitnumber);
}

// ----------- Read string -----------

int Packet::incReadString(int &position, char* &str) const
{
   str = (char *)(&(buffer[position]));
   uint32 strLength = strlen(str);
   position += (strLength + 1);
   return strLength;
}

inline const char*
Packet::incReadString(int &position) const
{
   char* str;
   incReadString(position, str);
   return str;
}

inline int
Packet::incReadString( int& pos, MC2String& str ) const
{
   char* tmp;
   int len = incReadString( pos, tmp );
   str = tmp;
   return len;
}

// ------------ Read bytearray --------

inline const byte*
Packet::incReadByteArray(int &position,
                         int nbrBytes) const
{
   const byte* bptr = (&(buffer[position]));
   position += nbrBytes;
   return bptr;
}

inline byte*
Packet::incReadByteArray(int &position,
                         byte* dest,
                         int nbrBytes) const
{
   const byte* source = incReadByteArray(position,
                                         nbrBytes);
   memcpy(dest, source, nbrBytes);
   return dest;
}

// ------------ Read bbox --------

inline int
Packet::incReadBBox(int& position, MC2BoundingBox& bbox) const
{
   bbox.setMaxLat( incReadLong( position ) );
   bbox.setMinLon( incReadLong( position ) );
   bbox.setMinLat( incReadLong( position ) );
   bbox.setMaxLon( incReadLong( position ) );
   return 4*4;
}


// ----------- Write Long -------------

inline void 
Packet::quickIncWriteLong( int &position, uint32 value )
{
   // ====================================================== 
   // This might be removed if the if-statement consumes too much cpu...
   MC2_ASSERT( longAligned( position ) );
   ASSERT_INSIDE( position );
   // ====================================================== 

   *( (uint32 *) &buffer[position] ) = htonl(value);
   position += sizeof(uint32);
}

inline int 
Packet::writeLong(int position, uint32 value) 
{
//   DEBUG8(cerr << "writeLong at position " << position << endl);

   // ====================================================== 
   // This might be removed if the if-statement consumes too much cpu...
   ASSERT_INSIDE( position );
   // ====================================================== 

   int oldPos = position;
   AlignUtility::alignLong(position);

   // no need to check if we align it anyway
   // ====================================================== 
   // This might be removed if the if-statement consumes too much cpu...
/*   if( ( ((int)(&buffer[position])) & 3) != 0 ) {
      mc2log << fatal << "BUG: Packet::writeLong(): Protocol not aligned. "
                         "Packettype = " << getSubTypeAsString() << " = " 
             << uint32(getSubType()) << endl; 
      exit(1);
   } */
   // ====================================================== 
   *( (uint32 *) &buffer[position] ) = htonl(value);
   return position-oldPos+sizeof(uint32); 
}

inline int
Packet::incAlignWriteLong(int& position)
{
   int retVal = 0;
   int newPos = position;
   AlignUtility::alignLong(newPos);
   while ( position < newPos ) {
      buffer[position++] = 0; // Zero out the stuff.
      ++retVal;
   }
   return retVal;
}

inline void
Packet::incAlignReadLong(int& position) const
{
   AlignUtility::alignLong(position);
}

inline int
Packet::incWriteLong(int &position, uint32 value)
{
   uint32 temp = writeLong(position, value);
   position += temp;
   return temp; 
}

inline int 
Packet::incWriteLongLong(int& position, uint64 value)
{
   const uint32 high = (value >> 32);
   const uint32 low  = (value >>  0);
   incWriteLong(position, high);
   incWriteLong(position, low);
   return 8;
}


inline int
Packet::incWriteFloat32( int& position, float32 val)
{
   return incWriteLong( position, *reinterpret_cast<uint32*>(&val));
}

// ----------- Write Short -------------

inline int
Packet::incAlignWriteShort(int& position)
{
   int newPos = position;
   AlignUtility::alignShort(newPos);
   if ( newPos > position ) {
      buffer[position++] = 0; // Write zero
      return 1;
   } else {
      return 0; // Nothing to do.
   }
}

inline void
Packet::incAlignReadShort( int& pos ) const
{
   AlignUtility::alignShort( pos );
}

inline int 
Packet::writeShort(int position, uint16 value) 
{
//   DEBUG8(cerr << "writeShort at position " << position << endl);
   
   // ====================================================== 
   // This might be removed if the if-statement consumes too much cpu...
   if( position >= int(bufSize) ) {
      mc2log << fatal << "BUG: Packet::writeShort(): Write outside packet "
                         "buffer, Packettype = " << getSubTypeAsString()
             << endl;
      MC2_ASSERT( false );
   }
   // ====================================================== 

   int oldPos = position;
   AlignUtility::alignShort(position);

   // ====================================================== 
   // This might be removed if the if-statement consumes too much cpu...
   MC2_ASSERT( shortAligned( position ) );
   // ====================================================== 
   
   *((uint16 *)&buffer[position]) = htons(value);
   return position-oldPos+sizeof(uint16);  
}

inline int
Packet::incWriteShort(int &position, uint16 value) {
   uint32 temp = writeShort(position, value);
   position += temp;
   return temp;  
}

inline void 
Packet::quickIncWriteShort(int &position, uint16 value)
{
   // ====================================================== 
   // This might be removed if the if-statement consumes too much cpu...
   MC2_ASSERT( shortAligned( position ) );
   // ====================================================== 
   *((uint16 *)&buffer[position]) = htons(value);
   position += sizeof(uint16);
}

// ----------- Write Byte -------------

inline int 
Packet::writeByte(int position, byte value) 
{
//   DEBUG8(cerr << "writeByte at position " << position << endl);

   // ====================================================== 
   // This might be removed if the if-statement consumes too much cpu...
   if( position > (int) bufSize ) {
      mc2log << error << "BUG: Packet::writeByte(): Write outside packet "
                         "buffer, Packettype = " << getSubTypeAsString()
             << endl;
      MC2_ASSERT( false );
   }
   // ====================================================== 
   buffer[position] = value;
   return (1);
}

inline int
Packet::incWriteByte(int &position, byte value) {
   uint32 temp = writeByte(position, value);
   position += temp;
   return temp;  
}

// ----------- Write Bit -------------

inline void
Packet::writeBit(int bytepos,
                 int bitnumber,
                 bool value)
{
#if defined(__i386__) && defined(__GNUC__) && defined (NOT_NOW_MAYBE_NOT_EVER)
   struct __dummy { unsigned long a[100]; };
#  define ADDR (*(struct __dummy *) addr)
   byte* addr = &buffer[bytepos];
   if ( value ) {
      __asm__ (
         "btsl %1,%0"
         :"=m" (ADDR)
         :"Ir" (bitnumber));
   } else {
      __asm__ ( 
         "btrl %1,%0"
         :"=m" (ADDR)
         :"Ir" (bitnumber));
   }
#  undef ADDR
#else  
   byte b = readByte(bytepos);
   b = BitUtility::setBit(b, bitnumber, value);
   writeByte(bytepos, b);
#endif

}

// ----------- Write String -------------

inline
void Packet::quickIncWriteString( int &position,
                                  const char *str )
{
   // a little quicker...
   char *tmp = (char *) (buffer + position);
   while ( (*tmp++ = *str++) != '\0') {}
   position = tmp - (char *) buffer; 
}

inline
void Packet::incWriteString(int &position, const char* str) {
   if ( str == NULL )
      str = "";
   quickIncWriteString( position, str );
   
   // ======================================================
   DEBUG1(
      if( position >  int(bufSize) ) {
         cerr << "BUG: incWriteString : Write outside packet buffer "
              << "Packettype = " << getSubTypeAsString() << endl;
         MC2_ASSERT( false );
      }
      );
   // ======================================================
}


inline void
Packet::incWriteString( int& pos, const MC2String& str )
{
   // NB! Byte writing function cannot be used since it would
   //     not be compatible with readString, thus c_str() must
   //     be used.
   incWriteString( pos, str.c_str() );
}

//  -------------- Write bytearray ---------------
inline void
Packet::incWriteByteArray(int &position,
                          const byte* bptr,
                          int nbrBytes)
{
   // ====================================================== 
   // This might be removed if the statement consumes too much cpu...
   MC2_ASSERT( position + nbrBytes <= int32(bufSize) );
   // ====================================================== 

   byte *packetPos = (&(buffer[position]));
   memcpy(packetPos, bptr, nbrBytes);
   position += nbrBytes;
}

//  -------------- Write bbox ---------------

inline int
Packet::incWriteBBox(int& position, const MC2BoundingBox& bbox)
{
   // ====================================================== 
   // This might be removed if the statement consumes too much cpu...
   MC2_ASSERT( position + 4*4 <= int32(bufSize) );
   // ====================================================== 

   incWriteLong( position, bbox.getMaxLat() );
   incWriteLong( position, bbox.getMinLon() );
   incWriteLong( position, bbox.getMinLat() );
   incWriteLong( position, bbox.getMaxLon() );
   return 4*4;
}


// -- Set and get functions using the above read and write functions.

inline void 
Packet::setPacketTag( const char* tag ) 
{ 
   writeByte( PACKET_OFFSET_TAG, tag[0] );
   writeByte( PACKET_OFFSET_TAG + 1, tag[1] );
   writeByte( PACKET_OFFSET_TAG + 2, tag[2] );
}

inline void
Packet::setResourceID( uint16 resnbr )
{
   writeShort( PACKET_OFFSET_RESOURCE, resnbr );
}

inline uint16
Packet::getResourceID() const
{
   return readShort( PACKET_OFFSET_RESOURCE );
}

inline uint16 
Packet::getProtocolVersion() const 
{
   return ( readByte(PACKET_OFFSET_PROTMAJOR) << 8 ) |
      ( (readByte(PACKET_OFFSET_PROTPRIO) >> 4) & 0x0F ); 
}

inline void 
Packet::setProtocolVersion(byte majorProt, byte minorProt) 
{
   writeByte(PACKET_OFFSET_PROTMAJOR, majorProt);
   writeByte(PACKET_OFFSET_PROTPRIO, (minorProt << 4) | getPriority() );
}

inline byte 
Packet::getPriority() const 
{
   return (readByte(PACKET_OFFSET_PROTPRIO) & 0x0F); 
}

inline void 
Packet::setPriority(byte pr) 
{
   writeByte(PACKET_OFFSET_PROTPRIO, (pr & 0x0F) |
             (readByte(PACKET_OFFSET_PROTPRIO) & 0xF0) ); 
}


inline Packet::packetType 
Packet::getSubType( byte* buff ) {
   // Do not use readShort here, since it calls getSubType
   return (Packet::packetType)
      ntohs(*((uint16 *)&(buff[PACKET_OFFSET_SUBTYPE])));
}

inline Packet::packetType
Packet::getSubType() const 
{ 
   return getSubType(getBuf());
}


inline void 
Packet::setSubType(uint16 st) 
{ 
   writeShort( PACKET_OFFSET_SUBTYPE, st );
}

uint32 
Packet::getOriginIP() const 
{ 
   return readLong( PACKET_OFFSET_ORIGINIP ); 
}

void 
Packet::setOriginIP(uint32 ip) 
{ 
   writeLong( PACKET_OFFSET_ORIGINIP, ip ); 
}

uint16 Packet::getOriginPort() const { 
   return readShort(PACKET_OFFSET_ORIGINPORT); 
}

void Packet::setOriginPort(uint16 pn) { 
   writeShort(PACKET_OFFSET_ORIGINPORT, pn); 
}

Packet::RequestID Packet::getRequestID() const { 
   return readShort(PACKET_OFFSET_REQUESTID); 
}

void Packet::setRequestID(RequestID rid) { 
   writeShort(PACKET_OFFSET_REQUESTID, rid); 
}
void Packet::setTimeout( byte timeout ) {
   writeByte( PACKET_OFFSET_TIMEOUT, timeout );
}

uint32
Packet::getTimeout() const
{
   return readByte( PACKET_OFFSET_TIMEOUT );
}

uint32
Packet::getRequestTimestamp() const {
   return readLong(PACKET_OFFSET_TIMESTAMP);
}

void 
Packet::setRequestTimestamp(uint32 timestamp) {
   writeLong( PACKET_OFFSET_TIMESTAMP, timestamp ); 
}
      
uint16
Packet::getRequestOriginator() const {
   return readShort(PACKET_OFFSET_ORIGINATOR);
}

void
Packet::setRequestOriginator(uint16 origin) {
   writeShort(PACKET_OFFSET_ORIGINATOR, origin); 
}

uint32 Packet::getDebInfo() const { 
   return readLong(PACKET_OFFSET_DEBITINFO); 
}

void Packet::setDebInfo(uint32 de) { 
   writeLong(PACKET_OFFSET_DEBITINFO, de); 
}

uint32 Packet::getCPUTime() const {
   return readLong( PACKET_OFFSET_CPUTIME );
}

void Packet::setCPUTime( uint32 timeMs ) {
   writeLong( PACKET_OFFSET_CPUTIME, timeMs );
}

Packet::PacketID Packet::getPacketID() const { 
   return readShort(PACKET_OFFSET_PACKETID); 
}

void Packet::setPacketID(PacketID pi) { 
   writeShort(PACKET_OFFSET_PACKETID, pi); 
}

byte Packet::getPacketNbr() const { 
   return readByte(PACKET_OFFSET_PACKETNBR); 
}

void Packet::setPacketNbr(byte nbr) { 
   writeByte(PACKET_OFFSET_PACKETNBR, nbr); 
}

byte Packet::getNbrPackets() const { 
   return readByte(PACKET_OFFSET_NBRPACKETS); 
}

void Packet::setNbrPackets(byte nbr) { 
   writeByte(PACKET_OFFSET_NBRPACKETS, nbr); 
}

byte Packet::getResendNbr() const { 
   return readByte(PACKET_OFFSET_RESENDNBR); 
}

void Packet::setResendNbr(byte nbr) { 
   writeByte(PACKET_OFFSET_RESENDNBR, nbr); 
}

void Packet::setMapSet(uint32 mapSet) { 
   writeLong(PACKET_OFFSET_MAPSETID, mapSet); 
}

uint32 Packet::getMapSet() const { 
   return readLong(PACKET_OFFSET_MAPSETID); 
}

void Packet::setRequestTag(uint32 requestTag) { 
   writeLong(PACKET_OFFSET_REQUEST_TAG, requestTag); 
}

uint32 Packet::getRequestTag() const { 
   return readLong(PACKET_OFFSET_REQUEST_TAG); 
}

// ======================================================= RequestPacket =
inline Packet::MapID 
RequestPacket::getMapID() const 
{
   return ( readLong( REQUEST_PACKET_MAPID_POS ) );
}

inline void 
RequestPacket::setMapID( uint32 mapID ) 
{
   writeLong( REQUEST_PACKET_MAPID_POS, mapID );
}

// ========================================================= ReplyPacket =
inline uint32 
ReplyPacket::getStatus() const 
{
   return ( readLong( REPLY_PACKET_STATUS_POS ) );
}

inline void 
ReplyPacket::setStatus( uint32 status ) 
{
   writeLong( REPLY_PACKET_STATUS_POS, status);
}

#endif // __PACKET_H


