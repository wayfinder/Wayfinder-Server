/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ISABBOXNAVMESSAGE_H
#define ISABBOXNAVMESSAGE_H

#include "NavMessage.h"
#include "isabBoxSession.h"

#define ISABBOX_MESSAGE_HEADER_SIZE_VER_0 11
#define ISABBOX_MESSAGE_HEADER_SIZE_VER_1 19
#define ISABBOX_MESSAGE_HEADER_SIZE_VER_2 35
// Doesn't return the userName now
#define ISABBOX_MESSAGE_HEADER_SIZE_VER_2_REPLY 15
// Version 5 has status code in reply but not in request
#define ISABBOX_MESSAGE_HEADER_SIZE_VER_5 13
#define ISABBOX_MESSAGE_HEADER_SIZE_VER_5_REPLY 14

// Forward
class RouteElement;


/**
 *   Utility class for converting isabboxMessages to/from bytes
 *
  */
class isabBoxNavMessageUtil {

public:

   enum binaryTransfer_t {
      invalidData          =  0x00,
      crossingMapGfxData   =  0x10,
      syncFavorites        =  0x11,
      crossingMapGifData   =  0x12,
      topRegionListData    =  0x13,
      additionalRouteData  =  0x14,
      categoriesFile       =  0x15,
      latestNewsFile       =  0x16,

      logFileData          =  0x20,
   };

   static const int navigatorStructSize = 12;
   /**
    *   Takes the header information and converts it to
    *   bytes suitable for the isabbox packet format.
    *   @param mess    The message to get the info from.
    *   @param buf     The buffer to write to.
    *   @param bufSize The maximum size of the buffer.
    *   @return Nbr of bytes written.
    */
   static int convertHeaderToBytes(const NavMessage* mess,
                               byte* buf,
                               int bufSize);

   /**
    *   Takes a buffer and writes the header information into a
    *   message.
    *   @param mess    The message to write it to.
    *   @param buf     The buffer to read from.
    *   @param bufSize The size of the buffer.
    *   @return The nbr of bytes read.
    */
   static int convertHeaderFromBytes(NavMessage* mess,
                                     const byte* buf,
                                     int bufSize);

   /**
    * Writes the CRC into the correct header position depending on 
    * protoVer.
    *
    * @param buf The buffer to write to, must contain header bytes.
    * @param protoVer The protocol version.
    */
   static void writeCRC( byte* buf, uint8 protoVer, uint32 crc );

   /**
    * Reads the CRC from the correct header position depending on 
    * protoVer.
    *
    * @param buf The buffer to read from, must contain header bytes.
    * @param protoVer The protocol version.
    */
   static uint32 readCRC( byte* buf, uint8 protoVer );

   /**
    *   Reads a byte from buf and updates the position.
    *   isabbox packets should be big endian.
    *   @param buf The buffer to read from.
    *   @param pos The position in the buffer to read from. Will be
    *              increased.
    *   @return    The value of the byte at position.
    */
   static uint8 incReadByte(const byte* buf,
                            int& pos);

   /**
    *   Reads an uint16 from buf and updates the position.
    *   isabBox packets should be big endian.
    *   @param buf The buffer to read from.
    *   @param pos The position in the buffer to read from. Will be
    *              increased.
    *   @return    The value of the uint16 at position.
    */
   static uint16 incReadShort(const byte* buf,
                              int& pos);

   /**
    *   Reads a uint32 from buf and updates the position.
    *   isabBox packets should be big endian.
    *   @param buf The buffer to read from.
    *   @param pos The position in the buffer to read from. Will be
    *              increased.
    *   @return    The value of the uint32 at position.
    */
   static uint32 incReadLong(const byte* buf,
                             int& pos);

   /**
    *   Finds a NULL-terminated string from position pos in buf buf.
    */
   
   static char* incReadString(const byte* buf,
                           int& pos);
   
    /**
     *   Writes a byte to buf and updates the position.
    *   isabBox packets should be big endian ( bytes are not endian, though).
    *   @param buf The buffer to write to.
    *   @param pos The position to write to.
    *   @param val The value to write.
    *   @return The new pos.
    */
   static int incWriteByte(byte* buf, int& pos, byte val);

   /**
    *   Writes an uint16 to buf and updates the position.
    *   isabBox packets should be big endian.
    *   @param buf The buffer to write to.
    *   @param pos The position to write to.
    *   @param val The uint16 to write.
    *   @return The new pos.
    */
   static int incWriteShort(byte* buf, int& pos, uint16 val);

   /**
    *   Writes an uint32 to buf and updates the position.
    *   isabBox packets should be big endian.
    *   @param buf The buffer to write to.
    *   @param pos The position to write to.
    *   @param val The uint32 to write.
    *   @return The new pos.
    */
   static int incWriteLong(byte* buf, int& pos, uint32 val);

   /**
    *   Converts an uint32 to navrad (rad*1000000) and writes it
    *   to the buffer. (To save a lot of typing).
    *   isabBox packets should be big endian.
    *   @param buf The buffer to write to.
    *   @param pos The position to write to.
    *   @param val The int32 to write. (this one is signed)
    *   @return The new pos.
    */
   static int incWriteNavRad(byte* buf, int& pos, int32 val);

   /**
    *   Converts a StringTable::stringCode containing a turn
    *   description to isabBox formats and writes it to a message
    *   using one byte.
    *   @param buf  The buffer to write to.
    *   @param pos  The position to write to.
    *   @param re   The turn to write.
    *   @param protoVer The protocoll version to convert for.
    */
   static int incWriteTurn( byte* buf, int& pos, 
                            const RouteElement* re,
                            int protoVer );

   /**
    *    Converts the string to latin-1 and then writes the converted
    *    string to the buffer and updates the pos pointer.
    *    @param buf  The buffer to write to.
    *    @param pos  The position in the buffer to write to.
    *    @param str  The string to write.
    *    @return     The number of bytes written.
    */
   static int incWriteString(byte* buf, int& pos, const char* str);
   
   /**
    *    Writes a string as is to the buffer and updates the pos pointer.
    *    @param buf  The buffer to write to.
    *    @param pos  The position in the buffer to write to.
    *    @param str  The string to write.
    *    @return     The number of bytes written.
    */
   static int incWriteStringUTF8(byte* buf, int& pos, const char* str);
   
   /**
    *   Converts from navrad (rad*1000000) as used in isabBox packets to
    *   MC2 units used in MC2. ( It is assumed that the server
    *   has more CPU power than the isabBox.
    *   @param mmin The number of milliminutes.
    *   @return     The number of MC2 units.
    */
   static int32 navRadToMC2(int32 mmin);

   /**
    *   Converts from MC2 coordinates to navrad (rad*1000000).
    *   @param mc2 The angle in mc2 coordinates.
    *   @return    The angle in milliminutes.
    */
   static int32 MC2ToNavRad(int32 mc2);

   /**
    *   Converts a MC2 turndescription to the format used in
    *   the isabBox.
    *
    *   @param re The RouteElement of the turn.
    *   @param protoVer The protocoll version to convert for.
    */
   static uint16 turnCodeToIsabBox( const RouteElement* re,
                                    int protoVer );
   
   /**
    * Revese of turnCodeToIsabBox.
    */
   static StringTable::stringCode isabBoxToTurnCode( 
      uint16 turnCode, byte protoVer );

   /**
    *   Converts a MC2 ExpandItemID attribute to the flag format used in
    *   the isabBox.
    */
   static byte attributeToFlags(byte attributes);

   /**
    *   Check if the coordinates are valid.
    *   @param lat   Latitude in Navigator coordinates.
    *   @param lon   Longitude in Navigator coordinates.
    *   @return True if the coordinates can be used,
    *           false if the coordinates are invalid.
    */
   static bool validNavCoordinates(uint32 lat, uint32 lon);

   /**
    * Aligns the position to nearest short position.
    * 
    * @param pos If odd then buf at pos will be set to zero and pos will
    *            be increased by one.
    * @param buf The buffer to align in.
    */
   static inline void alignShort( byte* buf, int& pos );

private:
 
   /**
    *   Values used when converting string codes to isabBox string codes. 
    */
   /// from ISos/prod/navigate1/navp4c/nav_common.h
   typedef enum {
      nav_route_point_not_ok      = - 0x0001,

      nav_route_point_end         = 0x0000,
      nav_route_point_start       = 0x0001,
      nav_route_point_ahead       = 0x0002,
      nav_route_point_left        = 0x0003,
      nav_route_point_right       = 0x0004,
      nav_route_point_uturn       = 0x0005,
      nav_route_point_startat     = 0x0006,
      nav_route_point_finally     = 0x0007,
      nav_route_point_enter_rdbt  = 0x0008,
      nav_route_point_exit_rdbt   = 0x0009,
      nav_route_point_ahead_rdbt  = 0x000a,
      nav_route_point_left_rdbt   = 0x000b,
      nav_route_point_right_rdbt  = 0x000c,
      nav_route_point_exitat      = 0x000d,
      nav_route_point_on          = 0x000e,
      nav_route_point_park_car    = 0x000f,
      nav_route_point_keep_left   = 0x0010,
      nav_route_point_keep_right  = 0x0011,
      nav_route_point_start_with_uturn = 0x0012,
      nav_rotue_point_uturn_rdbt  = 0x0013,
      nav_route_point_follow_road = 0x0014,
      nav_route_point_enter_ferry = 0x0015,
      nav_route_point_exit_ferry  = 0x0016,
      nav_route_point_change_ferry= 0x0017,
      nav_route_point_endofroad_left=0x0018,
      nav_route_point_endofroad_right=0x0019,
      nav_route_point_off_ramp_left=0x001A,
      nav_route_point_off_ramp_right=0x001B,
      nav_route_point_exit_rdbt_8 = 0x001C,
      nav_route_point_exit_rdbt_16= 0x001D,

      NBR_PDA_ACTIONS,

      nav_route_point_delta     = 0x03fe,
      nav_route_point_max       = 0x03ff,
   } action_t;

  
   /**
    *   Table type used when converting the string codes to
    *   isabBox string codes,
    */
   struct turnCodeTable_t {
      StringTable::stringCode mc2code;
      action_t                isabboxcode;
   };

   /**
    *   The table used when converting the string codes to
    *   isabBox string codes. Supports versions up to and including 6.
    */
   static const struct turnCodeTable_t turnCodeTable[];


   /**
    * The table used when converting the string codes to
    * isabBox string codes. Supports versions 7+.
    */
   static const struct turnCodeTable_t turnCodeTable7p[];
};

class isabBoxTypeMessage : public IncomingNavMessage {
  public:
   /**
    *   Creates a message for type-testing only.
    */
   isabBoxTypeMessage(const NavAddress& senderAddress,
                      const byte* buf,
                      int bufLen,
		      NavSession * session);
   
   
   /**
    *   Convert this message from bytes.
    *   @param buf       The buffer to read from.
    *   @param bufSize   The size of the buffer.
    *   @return          True if the conversion could be made.
    */
   bool convertFromBytes(const byte* buf,
                         int bufSize);

   /**
    * Get the size of a isabBox type message header for a certain protocol
    * version.
    *
    * @param protoVer The protocol version.
    * @param reply If reply header or request header.
    * @return The header size.
    */
   static int getHeaderSize( int protoVer, bool reply = true );
};


/**
 * Class used for generic replies such as error reply.
 * 
 */
class isabBoxReplyMessage : public OutGoingNavMessage {
   public:
      /**
       * Creates a isabBox reply with a status.
       */
      isabBoxReplyMessage( const NavAddress& recipientAddress,
                           MessageType type,
                           uint8 status,
                           NavSession* session, 
                           uint8 protoVer );

      /**
       *   Convert the message to bytes.
       *   Override this one in the subclasses for different
       *   output formats.
       *   @param buf     The buffer to write to.
       *   @param bufSize The size of the buffer.
       *   @return True if the message could be converted.
       */
      virtual bool convertToBytes( byte* buf,
                                   int bufSize );
};


inline void 
isabBoxNavMessageUtil::alignShort( byte* buf, int& pos ) {
   if ( pos & 0x01 ) {
      buf [ pos++ ] = 0;
   }
}


#endif

