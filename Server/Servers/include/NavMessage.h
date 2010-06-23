/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVMESSAGE_H
#define NAVMESSAGE_H

#include "config.h"
#include "StringTable.h"
#include "ItemTypes.h"

#include "NavAddress.h"

#include "RouteTypes.h"
#include <stdlib.h>
// NavMessages isn't really depending on NavSession. Funny.
class NavSession;

class NavComm; // Forward declaration

/**
 *   Class that contains an address and type of a navigatormessage.
 */
class NavMessage {

public:

   /**
    *   Message types.
    *   
    */
   enum MessageType {
      INVALID            = 0x00,
      PICK_ME_UP_REQ     = 0x01,
      PICK_ME_UP_REPLY   = 0x02,
      PICK_UP_REQ        = 0x03,
      PICK_UP_REPLY      = 0x04,
      ROUTE_REQ          = 0x05,
      ROUTE_REPLY        = 0x06,
      SEARCH_REQ         = 0x07,
      SEARCH_REPLY       = 0x08,
      DEST_REQ           = 0x09,
      DEST_REPLY         = 0x0a,

      GPS_ADDRESS_REQ    = 0x0b,
      GPS_ADDRESS_REPLY  = 0x0c,
      GPS_POS_REQ        = 0x0d,
      GPS_POS_REPLY      = 0x0e,
      // The missing 0x0f is an very old thing, req is even below
      POLL_SERVER_REQ    = 0x10,
      POLL_SERVER_REPLY  = 0x11,

      MAP_REQ            = 0x12,
      MAP_REPLY          = 0x13,
      FAV_REQ            = 0x14,
      FAV_REPLY          = 0x15,

      INFO_REQ           = 0x16,
      INFO_REPLY         = 0x17,

      MESSAGE_REQ        = 0x18,
      MESSAGE_REPLY      = 0x19,
      UPGRADE_REQ        = 0x1a,
      UPGRADE_REPLY      = 0x1b,

      VECTOR_MAP_REQ     = 0x1c,
      VECTOR_MAP_REPLY   = 0x1d,

      MULTI_VECTOR_MAP_REQ   = 0x1e,
      MULTI_VECTOR_MAP_REPLY = 0x1f,

      PARAMETER_REQ      = 0x20,
      PARAMETER_REPLY    = 0x21,
      CELL_REPORT        = 0x22,
      CELL_CONFIRM       = 0x23,
      GPS_INIT_REQ       = 0x30,
      GPS_INIT_REPLY     = 0x31,
      BINARY_UPLOAD_REQ  = 0x32,
      BINARY_UPLOAD_REPLY = 0x33,
      NAVALARM_REQ       = 0x40,
      NAVALARM_REPLY     = 0x41,
   };
   
   /**
    * Enum for general reply statuses.
    * NAV_STATUS_REQUEST_SPECIFIC_MASK is not a status but a mask
    * to see if a status code is request specific or not. 
    * Status codes with the high bit set, 0x80-0xff,
    * is request specific.
    */
   enum ReplyStatus {
      NAV_STATUS_OK                    = 0x00,
      NAV_STATUS_NOT_OK                = 0x01,
      NAV_STATUS_REQUEST_TIMEOUT       = 0x02,
      NAV_STATUS_PARAM_REQ_NOT_FIRST   = 0x03,
      NAV_STATUS_OUTSIDE_MAP           = 0x04,
      NAV_STATUS_PROTOVER_NOT_SUPPORTED= 0x05,
      NAV_STATUS_OUTSIDE_ALLOWED_AREA  = 0x06,
      NAV_STATUS_NO_TRANSACTIONS_LEFT  = 0x07,

      NAV_STATUS_REQUEST_SPECIFIC_MASK = 0x80
   };

   /**
    * The current protocol version, the maximum value of protover 
    * supported.
    */
   static uint8 MAX_PROTOVER;

   /**
    *   Constructs a new NavMessage.
    *   @param address   The sender address for incoming messages,
    *                    the recipient address for outgoing messages.
    *   @param messType  The type of message.
    *   @param isRequest True if the message is a request, false if it is
    *                    a reply.
    *   @param session   The associated session
    */
   NavMessage(const NavAddress& address,
              MessageType messType,
              bool isRequest,
              NavSession * session);
   
   /**
    *   Destructor.
    */
   virtual ~NavMessage();
   
   /**
    *   @return The type of this message.
    */
   MessageType getType() const;
   
   /**
    *   Set the type of the message.
    *   @param type The type of the message.
    */
   void setType(uint16 type);
   
   
   /**
    *   @return The address of the message.
    */
   const NavAddress* getAddress() const;
   
   /**
    *   @return True if the message is a request and 
    *           false if it is a reply.
    */
   virtual bool isRequest() const;

   /**
    *   @return The protocol version.
    */
   uint8 getProtoVer() const;
   
   /**
    *   Set the protocol version of the message.
    *   @param protoVer The protocol version.
    */
   void setProtoVer(uint8 protoVer);

   /**
    *   @return The length.
    */
   uint32 getLength() const;
   
   /**
    *   Set the length of the message.
    *   @param length The length.
    */
   void setLength(uint32 length);

   /**
    *   @return The request id.
    */
   uint8 getReqID() const;
   
   /**
    *   Set the request id of the message.
    *   @param id The request id.
    */
   void setReqID(uint8 id);

   /**
    *   Set the CRC for the message.
    */
   void setCRC(uint32 crc);

   /**
    *   Set the Navigator id for the message.
    */
   void setNavID(uint32 navID);

   /**
    *   @return The Navigators ID.
    */
   uint32 getNavID() const;
   
   /**
    *   Set the User id for the message.
    */
   void setUserID(uint32 userID);
   
   /**
    *   @return The user ID.
    */
   uint32 getUserID() const;

   /**
    *   Set the userName for the message.
    */
   void setUserName(const byte* userName);
   
   /**
    *   @return The a pinter to the userName.
    */
   char* getUserName();
   
   
   /**
    *   @return The CRC.
    */
   uint32 getCRC() const;
   
   /**
    *   @return The cookie for the message. The cookie is
    *           used by the comms to store data.
    */
   void* getCookie() const;

   /**
    *   Set the cookie for this message. The cookie is used
    *   by the comms to store data while the request is processed.
    */
   void setCookie(void* cookie);

   /**
    *   Get the associated session
    *   @return The session this request came in during
    */
   NavSession * getSession() const;

   /**
    *   Returns true if the request has valid origin coordinates.
    */

   bool hasValidCoordinates() { return m_validCoordinates; }

   /**
    * Turns a request message type to reply for same message type.
    * 
    * @param reqType The request message type.
    * @return The reply message type for the request type.
    */
   static MessageType replyType( MessageType request );


protected:

   /**
    *   Creates a new message.
    *   The constructor should only be used by the subtypes.
    *   @param type The type of message.
    */
   NavMessage() : m_address(NavAddress::ADDRESS_TYPE_NULL, "")
      { cerr << "Warning NavMessage called() " << endl; exit(1);}
   
   /**
    *   Protocol version ( In the header of all messages )
    */
   uint8 m_protoVer;

   /**
    *   The type of message ( In the header of all messages )
    */
   MessageType m_type;

   /**
    *   Message length ( In the header of all messages )
    */
   uint32 m_length;

   /**
    *   Request ID ( In the header of all messages ).
    */
   uint8 m_requestID;

   /**
    *   CRC ( In the header of all messages ).
    */
   uint32 m_crc;

   /**
    *   The address of the message.
    */
   NavAddress m_address;

   /**
    *   True if the message is a request.
    */
   bool m_isRequest;

   /**
    *   Cookie used to store data used by the comms.
    */
   void* m_cookie;

   /**
    *   Navigator ID that is unique for each navigator.
    */
   uint32 m_navID;

   /**
    *   User ID that is unique for each user.
    */
   uint32 m_userID;

   /**
    *   User name that is unique for each user.
    */
   char m_userName[20];
   
   /**
    *   The associated session that holds state during a 
    *   conversation
    */
   NavSession * m_session;

   /**
    *   Whether the coordinates in this request was valid
    *   or not.
    */
   bool m_validCoordinates;
};

class OutGoingNavMessage : public NavMessage {

public:

   /**
    *   Constructor
    *   @param recipientAddress  The address which this message should
    *                            be sent to.
    *   @param type              The type of message.
    *   @param isRequest         True if the message is a request.
    *   @param session           The associated session
    */
   OutGoingNavMessage(const NavAddress& recipientAddress,
                      MessageType type,
                      bool isRequest,
                      NavSession * session);

   /**
    *   Destructor.
    */
   virtual ~OutGoingNavMessage();
   
   
   /**
    *   Convert the message to bytes.
    *   Override this one in the subclasses for different
    *   output formats.
    *   @param buf     The buffer to write to.
    *   @param bufSize The size of the buffer.
    *   @return True if the message could be converted.
    */
   virtual bool convertToBytes(byte* buf,
                               int bufSize) = 0;

   /**
    * Set the status of the reply.
    *
    * @param status The new status of the reply.
    */
   void setStatus( uint8 status );

   /**
    * Get the status of the reply.
    *
    * @return The status of the reply.
    */
   uint8 getStatus() const;


   /**
    * Returns the reply size in bytes, 0 if not known.
    */
   virtual uint32 getReplySize() const;


private:

   /**
    * The status of the reply
    */
   uint8 m_status;
};

class IncomingNavMessage : public NavMessage {

public:

   /**
    *   Constructor
    *   @param senderAddress The address which this message came from.
    *   @param type          The type of message.
    *   @param isRequest     True if the message is a request, false
    *                        if it is a reply.
    *   @param session       The associated session
    */
   IncomingNavMessage(const NavAddress& senderAddress,
                      MessageType type,
                      bool isRequest,
                      NavSession * session);

   /**
    *    Destructor.
    */
   virtual ~IncomingNavMessage();
   
   /**
    *   Convert this message from bytes.
    *   Override this method in the subclasses for different
    *   input formats.
    *   @param buf       The buffer to read from.
    *   @param bufSize   The size of the buffer.
    *   @return          True if the conversion could be made.
    */
   virtual bool convertFromBytes(const byte* buf,
                                 int bufSize);

   /**
    *   This function will probably disappear and be put
    *   into the requestcontainer.
    *   @return The comm that this message came from.
    */
   NavComm* getComm() const {
      return m_comm;
   }

   /**
    *   Set the comm which this message came from.
    */
   void setComm(NavComm* comm) {
      m_comm = comm;
   }

private:
   
   /**
    *  The comm that this message came from.
    */
   NavComm* m_comm;
   
};


/* Implementation of inlined methods */

#endif



