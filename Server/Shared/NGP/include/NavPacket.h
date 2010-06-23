/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVPACKET_H
#define NAVPACKET_H

#include "config.h"
#include "NParamBlock.h"

#ifdef USE_MAGICBYTE
#define MAGICBYTE 0xed
#else
// Magic XOR bytes
#define MAGICBYTES NavPacket::magicBytes
#define MAGICLENGTH 256
#endif

/**
 * Class that represents a Navigator packet.
 * 
 */
class NavPacket {
   public:
      enum RequestType {
         NAV_INVALID               = 0x00,

         // 0x01 - 0x04 Reserved (Used in older protovers)

         NAV_ROUTE_REQ             = 0x05,        
         NAV_ROUTE_REPLY           = 0x06,

         NAV_SEARCH_REQ            = 0x07,         
         NAV_SEARCH_REPLY          = 0x08,

         // 0x09 - 0x0a Reserved (Used in older protovers)

         NAV_REV_GEOCODING_REQ       = 0x0b,
         NAV_REV_GEOCODING_REPLY     = 0x0c,

         // 0x0d - 0x0e Reserved (Used in older protovers)
         // 0x10 - 0x11 Reserved (Used in older protovers)

         NAV_MAP_REQ               = 0x12,
         NAV_MAP_REPLY             = 0x13,

         NAV_FAV_REQ               = 0x14,
         NAV_FAV_REPLY             = 0x15,

         NAV_INFO_REQ              = 0x16,
         NAV_INFO_REPLY            = 0x17,

         NAV_MESSAGE_REQ           = 0x18,
         NAV_MESSAGE_REPLY         = 0x19,

         NAV_UPGRADE_REQ           = 0x1a,
         NAV_UPGRADE_REPLY         = 0x1b,

         NAV_VECTOR_MAP_REQ        = 0x1c,
         NAV_VECTOR_MAP_REPLY      = 0x1d,

         NAV_MULTI_VECTOR_MAP_REQ  = 0x1e,
         NAV_MULTI_VECTOR_MAP_REPLY= 0x1f,

         // 0x20,0x21,0x30-0x33,0x40,0x41 Reserved (Used in older protovers)
         NAV_CELL_REPORT           = 0x22,
         NAV_CELL_CONFIRM          = 0x23,

         NAV_TOP_REGION_REQ        = 0x25,
         NAV_TOP_REGION_REPLY      = 0x26,
         NAV_LATEST_NEWS_REQ       = 0x27,
         NAV_LATEST_NEWS_REPLY     = 0x28,
         NAV_CATEGORIES_REQ        = 0x29,
         NAV_CATEGORIES_REPLY      = 0x2a,
         NAV_CALLCENTER_LIST_REQ   = 0x2b,
         NAV_CALLCENTER_LIST_REPLY = 0x2c,
         NAV_SERVER_LIST_REQ       = 0x2d,
         NAV_SERVER_LIST_REPLY     = 0x2e,
         NAV_NEW_PASSWORD_REQ      = 0x2f,
         NAV_NEW_PASSWORD_REPLY    = 0x30,
         NAV_SERVER_INFO_REQ       = 0x31,
         NAV_SERVER_INFO_REPLY     = 0x32,
         NAV_WHOAMI_REQ            = 0x33,
         NAV_WHOAMI_REPLY          = 0x34,

         NAV_BINARY_TRANSFER_REQ   = 0x35,
         NAV_BINARY_TRANSFER_REPLY = 0x36,
         NAV_NOP_REQ               = 0x37,
         NAV_NOP_REPLY             = 0x38,
         NAV_CHANGED_LICENCE_REQ   = 0x39,
         NAV_CHANGED_LICENCE_REPLY = 0x3a,
         NAV_SERVER_AUTH_BOB_REQ   = 0x3b,
         NAV_SERVER_AUTH_BOB_REPLY = 0x3c,

         NAV_TRACK_REQ             = 0x3d,
         NAV_TRACK_REPLY           = 0x3e,

         NAV_TUNNEL_DATA_REQ       = 0x42,
         NAV_TUNNEL_DATA_REPLY     = 0x43,

         NAV_COMBINED_SEARCH_REQ   = 0x44,
         NAV_COMBINED_SEARCH_REPLY = 0x45,

         NAV_SEARCH_DESC_REQ       = 0x46,
         NAV_SEARCH_DESC_REPLY     = 0x47,

         NAV_CELLID_LOOKUP_REQ     = 0x48,
         NAV_CELLID_LOOKUP_REPLY   = 0x49,

         NAV_GET_KEYED_DATA_REQ    = 0x4a,
         NAV_GET_KEYED_DATA_REPLY  = 0x4b,

         NAV_ECHO_REQ              = 0x4c,
         NAV_ECHO_REPLY            = 0x4d,

         NAV_VERIFY_THIRD_PARTY_TRANSACTION_REQ   = 0x4e,
         NAV_VERIFY_THIRD_PARTY_TRANSACTION_REPLY = 0x4f,

         NAV_LOCAL_CATEGORY_TREE_REQ   = 0x50,
         NAV_LOCAL_CATEGORY_TREE_REPLY = 0x51,

         NAV_ONE_SEARCH_REQ            = 0x52,
         NAV_ONE_SEARCH_REPLY          = 0x53,
         
         NAV_DETAIL_REQ                = 0x54,
         NAV_DETAIL_REPLY              = 0x55
      };


      /**
       * Constructor.
       *
       * @param protoVer The protocol version.
       * @param type The type of packet.
       * @param reqID The ID of the packet.
       * @param reqVer The version of request.
       */
      NavPacket( byte protoVer, uint16 type, byte reqID, byte reqVer );


      /**
       * Constructor.
       *
       * @param protoVer The protocol version.
       * @param type The type of packet.
       * @param reqID The ID of the packet.
       * @param reqVer The version of request.
       * @param buff The parameter block buffer.
       * @param buffLen The length of the parameter block buffer.
       * @param uncompressedSize Set to the uncompressed size if not NULL.
       */
      NavPacket( byte protoVer, uint16 type, byte reqID, byte reqVer,
                 const byte* buff, uint32 buffLen,
                 uint32* uncompressedSize = NULL );


      /**
       * Get the protoVer.
       */
      byte getProtoVer() const;


      /**
       * Get the type.
       */
      uint16 getType() const;


      /**
       * Get the reqID.
       */
      byte getReqID() const;


      /**
       * Get the reqVer.
       */
      byte getReqVer() const;


      /**
       * Get the parameter block.
       */
      NParamBlock& getParamBlock();


      /**
       * Returns the request type as string.
       *
       * @param type The request type to get string for.
       * @return A string with the name of the request type.
       */
      static const char* requestTypeAsString( uint16 type );


      /**
       * Writes the header up to req_ver.
       */
      void writeHeader( vector< byte >& buff ) const;


      /**
       * Writes the length and the params.
       */
      void writeLengthAndParams( vector< byte >& buff,
                                 bool mayUseGzip = false,
                                 uint32* uncompressedSize = NULL  ) const;

      /**
       * Dump packet.
       */
      void dump( ostream& out, const char* prefix, bool dumpValues = false, 
                 bool singleLine = false, uint32 maxLen = 35,
                 uint32 maxSame = 2 ) const;

      /**
       * The current protocol version, the maximum value of protover 
       * supported.
       */
      static byte MAX_PROTOVER;

      /**
       * Magic XOR bytes.
       */
      static uint8 magicBytes[];

   protected:
       /// The protocol version.
       byte m_protoVer;


       /// The type of packet.
       uint16 m_type;


       /// The ID of the packet.
       byte m_reqID;


       /// The version of type.
       byte m_reqVer;


       /// The parameter block.
       NParamBlock m_params;
};


/**
 * Class that represents a Navigator request packet.
 * 
 */
class NavRequestPacket : public NavPacket {
   public:
      /**
       * Constructor.
       *
       * @param protoVer The protocol version.
       * @param type The type of packet.
       * @param reqID The ID of the packet.
       * @param reqVer The version of request.
       * @param buff The parameter block buffer.
       * @param buffLen The length of the parameter block buffer.
       * @param uncompressedSize Set to the uncompressed size if not NULL.
       */
      NavRequestPacket( byte protoVer, uint16 type, byte reqID, 
                        byte reqVer, const byte* buff, uint32 buffLen,
                        uint32* uncompressedSize = NULL  );


      /**
       * Writes the packet to a buffer.
       *
       * @param buff The buffer to write to.
       */
      void writeTo( vector< byte >& buff,
                    bool mayUseGzip = false,
                    uint32* uncompressedSize = NULL ) const;
};



/**
 * Class that represents a Navigator reply packet.
 * 
 */
class NavReplyPacket : public NavPacket {
   public:
      enum ReplyStatus {
         NAV_STATUS_OK                           = 0x00,
         NAV_STATUS_NOT_OK                       = 0x01,
         NAV_STATUS_REQUEST_TIMEOUT              = 0x02,
         NAV_STATUS_PARAM_REQ_NOT_FIRST          = 0x03,
         NAV_STATUS_OUTSIDE_MAP                  = 0x04,
         NAV_STATUS_PROTOVER_NOT_SUPPORTED       = 0x05,
         NAV_STATUS_OUTSIDE_ALLOWED_AREA         = 0x06,
         NAV_STATUS_NO_TRANSACTIONS_LEFT         = 0x07,
         NAV_STATUS_GENERIC_PERMANENT_ERROR      = 0x08,
         NAV_STATUS_GENERIC_TEMPORARY_ERROR      = 0x09,
         NAV_STATUS_SERVER_UPGRADE_IN_PROGRESS   = 0x0a,
         NAV_STATUS_SERVER_OVERLOADED            = 0x0b,
         NAV_STATUS_EXPIRED_USER                 = 0x0c,
         NAV_STATUS_UNAUTHORIZED_USER            = 0x0d,
         NAV_STATUS_REDIRECT                     = 0x0e,
         NAV_STATUS_UPDATE_NEEDED                = 0x0f,
         NAV_STATUS_WF_TYPE_TOO_HIGH_LOW         = 0x10,
         NAV_STATUS_REQ_VER_NOT_SUPPORTED        = 0x11,
         NAV_STATUS_CRC_ERROR                    = 0x12,
         NAV_STATUS_PARAMBLOCK_INVALID           = 0x13,
         NAV_STATUS_UNKNOWN_REQUEST              = 0x14,
         NAV_STATUS_PARAMETER_INVALID            = 0x15,
         NAV_STATUS_MISSING_PARAMETER            = 0x16,
         NAV_STATUS_UNAUTH_OTHER_HAS_LICENCE     = 0x17,
         NAV_STATUS_EXTENDED_ERROR               = 0x18,
   
         NAV_STATUS_REQUEST_SPECIFIC_MASK        = 0x80
      };


      /**
       * Some changed licence specific status codes.
       */
      enum ChangedLicenceStatus {
         CHANGED_LICENCE_REPLY_OLD_LICENCE_NOT_IN_ACCOUNT       = 0xF1,
         CHANGED_LICENCE_REPLY_MANY_USERS_WITH_OLD_LICENCE      = 0xF2,
         CHANGED_LICENCE_REPLY_MANY_USERS_WITH_NEW_LICENCE      = 0xF3,
         CHANGED_LICENCE_REPLY_OLD_LICENCE_IN_OTHER_ACCOUNT     = 0xF4,
      };

      /**
       * Some upgrade specific status codes.
       */
      enum UpgradeStatus {
         UPGRADE_MUST_CHOOSE_REGION      = 0xF1,
      };


      /**
       * Constructor.
       *
       * @param protoVer The protocol version.
       * @param type The type of packet.
       * @param reqID The ID of the packet.
       * @param reqVer The version of reply.
       * @param statusCode The status of the reply.
       * @param statusMessage The status code as text.
       */
      NavReplyPacket( byte protoVer, uint16 type, byte reqID, 
                      byte reqVer, byte statusCode, 
                      const char* statusMessage );


      /**
       * Constructor.
       *
       * @param protoVer The protocol version.
       * @param type The type of packet.
       * @param reqID The ID of the packet.
       * @param reqVer The version of request.
       * @param statusCode The status of the reply.
       * @param statusMessage The status code as text.
       * @param buff The parameter block buffer.
       * @param buffLen The length of the parameter block buffer.
       * @param uncompressedSize Set to the uncompressed size if not NULL.
       */
      NavReplyPacket( byte protoVer, uint16 type, byte reqID, byte reqVer,
                      byte statusCode, const char* statusMessage,
                      const byte* buff, uint32 buffLen,
                      uint32* uncompressedSize = NULL );


      /**
       * Get the statusCode.
       */
      byte getStatusCode() const;


      /**
       * Set the statusCode.
       */
      void setStatusCode( byte statusCode );


      /**
       * Get the statusMessage.
       */
      const char* getStatusMessage() const;


      /**
       * Set the statusMessage.
       */
      void setStatusMessage( const char* statusMessage );


      /**
       * Set the statusMessage.
       */
      void setStatusMessage( const MC2String& statusMessage );


      /**
       * Writes the packet to a buffer.
       *
       * @param buff The buffer to write to.
       */
      void writeTo( vector< byte >& buff,
                    bool mayUseGzip = false,
                    uint32* uncompressedSize = NULL ) const;


   private:
      /// The status of the reply.
      byte m_statusCode;


      /// The status code as text.
      MC2String m_statusMessage;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline byte
NavPacket::getProtoVer() const {
   return m_protoVer;
}


inline uint16 
NavPacket::getType() const {
   return m_type;
}


inline byte 
NavPacket::getReqID() const {
   return m_reqID;
}


inline byte 
NavPacket::getReqVer() const {
   return m_reqVer;
}


inline NParamBlock& 
NavPacket::getParamBlock() {
   return m_params;
}


inline byte 
NavReplyPacket::getStatusCode() const {
   return m_statusCode;
}


inline void 
NavReplyPacket::setStatusCode( byte statusCode ) {
   m_statusCode = statusCode;
}


inline const char* 
NavReplyPacket::getStatusMessage() const {
   return m_statusMessage.c_str();
}


inline void 
NavReplyPacket::setStatusMessage( const char* statusMessage ) {
   m_statusMessage = statusMessage;
}

inline void
NavReplyPacket::setStatusMessage( const MC2String& statusMessage ) {
   setStatusMessage( statusMessage.c_str() );
}


#endif // NAVPACKET_H

