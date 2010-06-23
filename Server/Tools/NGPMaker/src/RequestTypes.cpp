/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RequestTypes.h"

namespace {

NGPMaker::RequestType s_requestTypes[] = {
   {"NAV_INVALID", 0x00},

   // 0x01 - 0x04 Reserved (Used in older protovers)

   {"NAV_ROUTE_REQ", 0x05},
   {"NAV_ROUTE_REPLY", 0x06},

   {"NAV_SEARCH_REQ", 0x07},
   {"NAV_SEARCH_REPLY", 0x08},

   // 0x09 - 0x0a Reserved (Used in older protovers)

   {"NAV_GPS_ADDRESS_REQ", 0x0b},
   {"NAV_GPS_ADDRESS_REPLY", 0x0c},

   // 0x0d - 0x0e Reserved (Used in older protovers)
   // 0x10 - 0x11 Reserved (Used in older protovers)

   {"NAV_MAP_REQ", 0x12},
   {"NAV_MAP_REPLY", 0x13},

   {"NAV_FAV_REQ", 0x14},
   {"NAV_FAV_REPLY", 0x15},

   {"NAV_INFO_REQ", 0x16},
   {"NAV_INFO_REPLY", 0x17},

   {"NAV_MESSAGE_REQ", 0x18},
   {"NAV_MESSAGE_REPLY", 0x19},

   {"NAV_UPGRADE_REQ", 0x1a},
   {"NAV_UPGRADE_REPLY", 0x1b},

   {"NAV_VECTOR_MAP_REQ", 0x1c},
   {"NAV_VECTOR_MAP_REPLY", 0x1d},

   {"NAV_MULTI_VECTOR_MAP_REQ", 0x1e},
   {"NAV_MULTI_VECTOR_MAP_REPLY", 0x1f},

   // 0x20,0x21,0x30-0x33,0x40,0x41 Reserved (Used in older protovers)
   {"NAV_CELL_REPORT", 0x22},
   {"NAV_CELL_CONFIRM", 0x23},

   {"NAV_TOP_REGION_REQ", 0x25},
   {"NAV_TOP_REGION_REPLY", 0x26},
   {"NAV_LATEST_NEWS_REQ", 0x27},
   {"NAV_LATEST_NEWS_REPLY", 0x28},
   {"NAV_CATEGORIES_REQ", 0x29},
   {"NAV_CATEGORIES_REPLY", 0x2a},
   {"NAV_CALLCENTER_LIST_REQ", 0x2b},
   {"NAV_CALLCENTER_LIST_REPLY", 0x2c},
   {"NAV_SERVER_LIST_REQ", 0x2d},
   {"NAV_SERVER_LIST_REPLY", 0x2e},
   {"NAV_NEW_PASSWORD_REQ", 0x2f},
   {"NAV_NEW_PASSWORD_REPLY", 0x30},
   {"NAV_SERVER_INFO_REQ", 0x31},
   {"NAV_SERVER_INFO_REPLY", 0x32},
   {"NAV_WHOAMI_REQ", 0x33},
   {"NAV_WHOAMI_REPLY", 0x34},

   {"NAV_BINARY_TRANSFER_REQ", 0x35},
   {"NAV_BINARY_TRANSFER_REPLY", 0x36},
   {"NAV_NOP_REQ", 0x37},
   {"NAV_NOP_REPLY", 0x38},
   {"NAV_CHANGED_LICENSE_REQ", 0x39},
   {"NAV_CHANGED_LICENSE_REPLY", 0x3a},
   {"NAV_SERVER_AUTH_BOB_REQ", 0x3b},
   {"NAV_SERVER_AUTH_BOB_REPLY", 0x3c},

   {"NAV_TRACK_REQ", 0x3d},
   {"NAV_TRACK_REPLY", 0x3e},

   {"NAV_TUNNEL_DATA_REQ", 0x42},
   {"NAV_TUNNEL_DATA_REPLY", 0x43},

   {"NAV_COMBINED_SEARCH_REQ", 0x44},
   {"NAV_COMBINED_SEARCH_REPLY", 0x45},

   {"NAV_SEARCH_DESC_REQ", 0x46},
   {"NAV_SEARCH_DESC_REPLY", 0x47},
};


}

namespace NGPMaker {

RequestType* getRequestTypes( uint32& size ) {
   size = sizeof( s_requestTypes ) / sizeof( s_requestTypes[ 0 ] );
   return s_requestTypes;
}



MC2String getRequestName( uint32 ID ) {
   // search for matching ID
   uint32 size;
   RequestType* requests = getRequestTypes( size );
   for ( uint32 i = 0; i < size; ++i ) {
      if ( requests[ i ].m_id == ID ) {
         return requests[ i ].m_name;
      }
   }
   // did not find any name
   return "";
}

}
