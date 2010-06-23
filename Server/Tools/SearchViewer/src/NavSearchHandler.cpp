/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavSearchHandler.h"

#include "../include/Request.h"
#include "STLStringUtility.h"
#include "Auth.h"
#include "TCPSocket.h"
#include "IPnPort.h"
#include "DataBuffer.h"
#include "NParamBlock.h"
#include "isabBoxNavMessage.h"
#include "NavPacket.h"
#include "Utility.h"

#include <sstream>

namespace SearchViewer {

void decodeBuffer( byte* buff, uint32 len ) {
   uint32 magicPos = 0;
   for ( uint32 i = 0 ; i < len ; i++ ) {
      buff[ i ] = buff[ i ] ^ MAGICBYTES[ magicPos ];
      magicPos++;
      if ( magicPos >= MAGICLENGTH ) {
         magicPos = 0;
      }
   }
}

NavSearchHandler::NavSearchHandler():
   SearchHandler( "Navigator" ),
   m_request( new NGPMaker::Request() ) {
   
}

void addLanguage( NGPMaker::Request& req,
                  LangTypes::language_t lang ) {
  NGPMaker::Param param;
  param.m_id = 6;
  param.m_type = NParam::Uint16;
  param.m_value = STLStringUtility::uint2str( lang );
  req.m_params.push_back( param );
}
void addSearchDescParams( NGPMaker::Request& req,
                          const CompactSearch& search ) {
   req.m_protocolVersion = 0xc;
   req.m_type = 0x46; // NAV_SEARCH_DESC_REQ
   req.m_version = 1;
   req.m_useGzip = false;
   req.m_id = 0;
   addLanguage( req, search.m_language );
}

void addAuthParam( NGPMaker::Request& req, 
                   const Auth& auth ) {
   NGPMaker::Param authParam;
   // username
   authParam.m_id = 8;
   authParam.m_type = NParam::String;
   authParam.m_value = auth.getUsername();
   req.m_params.push_back( authParam );
   // password
   authParam.m_id = 2;
   authParam.m_type = NParam::String;
   authParam.m_value = auth.getPassword();
   req.m_params.push_back( authParam );
}

void addSearchParams( NGPMaker::Request& req,
                      const CompactSearch& search ) {
   // standard header
   req.m_protocolVersion = 0xc;
   req.m_type = 0x44; // Combined search
   req.m_version = 2;
   req.m_useGzip = false;
   req.m_id = 1;

   // language
   addLanguage( req, search.m_language );

   NGPMaker::Param param;
   // category
   param.m_id = 1204;
   param.m_type = NParam::String;
   param.m_value = search.m_categoryName;
   req.m_params.push_back( param );
   // search area
   param.m_id = 1201;
   param.m_type = NParam::String;
   param.m_value = search.m_where;
   req.m_params.push_back( param );
   // search item
   param.m_id = 1203;
   param.m_type = NParam::String;
   param.m_value = search.m_what;
   req.m_params.push_back( param );
   // coordinates
   if ( search.m_location.isValid() ) {
      param.m_id = 1000;
      param.m_type = NParam::Uint32_array;
      MC2String coordStr;
      coordStr += STLStringUtility::uint2str( search.m_location.m_coord.lat );
      coordStr += ", ";
      coordStr += STLStringUtility::uint2str( search.m_location.m_coord.lon );
      param.m_value = coordStr;
      req.m_params.push_back( param );
   }
   // top region
   param.m_id = 1205;
   param.m_type = NParam::Uint32;
   param.m_value = STLStringUtility::uint2str( search.m_topRegionID );
   req.m_params.push_back( param );
   // start idx
   param.m_id = 5600;
   param.m_type = NParam::Uint32;
   param.m_value = STLStringUtility::uint2str( search.m_startIndex );
   req.m_params.push_back( param );
   // round
   param.m_id = 5601;
   param.m_type = NParam::Uint32;
   param.m_value = STLStringUtility::uint2str( search.m_round );
   req.m_params.push_back( param );
   // heading
   param.m_id = 5602;
   param.m_type = NParam::Uint32;
   param.m_value = STLStringUtility::uint2str( search.m_heading );
   req.m_params.push_back( param );
   // max hits
   param.m_id = 5603;
   param.m_type = NParam::Uint32;
   param.m_value = STLStringUtility::uint2str( search.m_endIndex - 
                                               search.m_startIndex );
   req.m_params.push_back( param );

}
void NavSearchHandler::sendRequest( const IPnPort& address,
                                    vector<byte>& data,
                                    MC2String& debugStr ) {
   TCPSocket sock;
   if ( ! sock.open() ) {
      debugStr += "Failed to open socket!";
      return;
   }

   if ( sock.connect( address, 5000 ) ) {
      ssize_t size = sock.writeAll( &data[ 0 ], data.size() );
      stringstream str;
      str << "Sent: " << size << " bytes." << endl;
      debugStr += str.str();
      const uint32 BUFF_SIZE = 100000;
      byte buff[BUFF_SIZE];
      data.clear();
      // used to determine how many request to expect back
      uint32 rounds = 0;
      
      while ( 1 )  {
         size = sock.read( buff, BUFF_SIZE );
         if ( size == -3 ) {  // EAGAIN
            mc2dbg << "size: " << size << endl;
            continue;
         } else if ( size <= 0 ) {
            break;
         }
         data.insert( data.end(), buff, buff + size );

         if ( size != 1 && size < (int)BUFF_SIZE ) {
            mc2dbg << "received: " << size << endl;
            parseReply( data );
            data.clear();
            // got enough request back?
            ++rounds;
            mc2dbg << "rounds: " << rounds << endl;
            if ( rounds == 2 ) {
               break;
            }
         }

      }

   } else {
      stringstream str;
      str << "Failed to connect to: " << address.toString() << endl;
      debugStr += str.str();
   }
   stringstream str;
   str << "Received: " << data.size() << " bytes." << endl;
   debugStr += str.str();
   
}

void NavSearchHandler::search( const IPnPort& address,
                               const Auth& auth,
                               const MC2String& descriptionCRC,
                               const CompactSearch& search ) {
   
   m_replyString.clear();

   // description crc not used here

   m_headings.m_resultHeadings.clear();

   // search desc request
   NGPMaker::Request searchDesc;
   addAuthParam( searchDesc, auth );
   addSearchDescParams( searchDesc, search );
   vector<byte> outBuffer;
   if ( ! NGPMaker::saveNGPToBuffer( outBuffer, searchDesc ) ) {
      m_replyString = "Failed to save search desc to buffer!\n";
      return;
   }

   { // combined search request
      addAuthParam( *m_request, auth );
      addSearchParams( *m_request, search );
      vector<byte> outBuffer2;
      if ( ! NGPMaker::saveNGPToBuffer( outBuffer2, *m_request ) ) {
         m_replyString = "Failed to save params to buffer!\n";
         return;
      }
      outBuffer.insert( outBuffer.end(), 
                        outBuffer2.begin(), outBuffer2.end() );
   }

   // send request
   stringstream str;
   str << search << endl;
   m_replyString += str.str();
   sendRequest( address, outBuffer, m_replyString );

}

void addItems( SearchMatch& heading, DataBuffer& buff ) {
   while ( buff.getCurrentOffset() < buff.getBufferSize() ) {
      ItemMatch item;
      item.m_type = "unknown";
      buff.readNextByte(); // type
      buff.readNextByte(); // sub type
      item.m_advert = buff.readNextByte(); // advert
      item.m_itemID = buff.readNextString();
      item.m_name = buff.readNextString();
      mc2dbg << "Item name: " << item.m_name << endl;
      item.m_image = buff.readNextString();
      // coordinates
      item.m_lat = buff.readNextLong();
      item.m_lon = buff.readNextLong();
      // skip regions
      uint32 nbrRegions = buff.readNextByte();
      if ( buff.bufferFilled() ) {
         break;
      }
      for ( uint32 i = 0; i < nbrRegions; ++i ) {
         buff.readNextShort();
         if ( buff.bufferFilled() ) {
            break;
         }
      }
      heading.m_items.push_back( item );
   }
   mc2dbg << "nbr items: " << heading.m_items.size() << endl;
}

void NavSearchHandler::addResult( Headings& headings, const NParam& reply ) {

   DataBuffer buff( (byte*)reply.getBuff(), (uint32)reply.getLength() );
   uint32 headingNum = buff.readNextLong();
   if ( m_headings.m_headings.find( headingNum ) == 
        m_headings.m_headings.end() ) {
      m_replyString += "Could not find matching heading for id =";
      stringstream str;
      str << headingNum << endl;
      m_replyString += str.str();
      return;
   }
   DescMatch& heading = m_headings.m_headings[ headingNum ];
   buff.readNextLong(); // area/item matches
   heading.m_matches.m_heading = heading.m_headingNum;
   heading.m_matches.m_startIndex = buff.readNextLong();
   heading.m_matches.m_totalNbrItems = buff.readNextLong();
   heading.m_matches.m_topHits = buff.readNextLong();
   
   // setup items for heading
   addItems( heading.m_matches, buff );

   headings.m_resultHeadings.push_back( heading );
}

void NavSearchHandler::parseCombinedSearch( NParamBlock& replyBlock ) {
   for ( uint32 i = 0; i < 50; i += 2 ) { // +2 to skip regions
      const uint32 id = 5700 + i;
      const NParam* headingParam = replyBlock.getParam( id );
      if ( headingParam == NULL ) {
         mc2dbg << "Heading param: id = " << id << " is empty." << endl;
         break;
      }
      addResult( m_headings, *headingParam );
   }
}

void NavSearchHandler::parseSearchDesc( NParamBlock& reply ) {

   const NParam* param = reply.getParam( 5900  );
   if ( param == NULL ) {
      mc2dbg << "Could not find heading param." << endl;
      return;
   }

   // get heading from buffer
   
   DataBuffer buff( (byte*)param->getByteArray(), 
                    (uint32)param->getLength() );
   uint32 pos = 0;
   while ( buff.getCurrentOffset() < buff.getBufferSize() &&
           pos != param->getLength() ) {
      DescMatch heading;
      /*
      buff.readNextLong(); // round
      heading.m_headingNum = buff.readNextLong();
      heading.m_name = buff.readNextString();
      buff.readNextLong(); // top region
      heading.m_image = buff.readNextString();
      m_headings.m_headings[ heading.m_headingNum ] = heading;
      */
      param->incGetUint32( pos ); // round
      heading.m_headingNum = param->incGetUint32( pos );
      heading.m_name = param->incGetString( false, pos );
      param->incGetUint32( pos ); // top region
      heading.m_image = param->incGetString( false, pos );
      m_headings.m_headings[ heading.m_headingNum ] = heading;
      mc2dbg << "heading: " << heading.m_headingNum << endl;
   }
}

void NavSearchHandler::parseReply( vector<byte>& reply ) try {
   mc2dbg << "Parse reply." << endl;

   if ( reply.empty() || reply[0] != 0x02 ) {
      m_replyString += "Missing STX byte or empty reply.\n";
      return;
   }

   const uint32 HEADER_SIZE = 10;
   decodeBuffer( &reply[0] + HEADER_SIZE,
                 reply.size() - HEADER_SIZE );

   // ignoring crc for now

   // ok, lets make package
   byte protVer = 0;
   uint16 type = 0;
   uint32 crc = 0;
   { 
      //      type =
      DataBuffer buff( &reply[0], reply.size() );
      buff.readNextByte(); // stx
      buff.readNextShort(); // length
      protVer = buff.readNextByte(); // protover
      type = buff.readNextShort(); // type
      buff.reset();
      buff.readPastBytes( reply.size() - 4 );
      crc = buff.readNextLong();
   }
   NParamBlock replyBlock( &reply[0] + HEADER_SIZE + 2, 
                           reply.size() - HEADER_SIZE - 4 - 2, 
                           protVer);
   stringstream str;
   Utility::hexDump(str, &reply[0], reply.size() );
   m_replyString += str.str();
   if ( ! replyBlock.getValid() ) {
      mc2dbg << "invalid param block." << endl;
      return;
   }

   if ( type == NavPacket::NAV_SEARCH_DESC_REPLY ) {
      mc2dbg << "nav search desc reply. "<< endl;
      parseSearchDesc( replyBlock );
      m_headings.m_crc = STLStringUtility::uint2str( crc );
   } else if ( type == NavPacket::NAV_COMBINED_SEARCH_REPLY ) {
      mc2dbg << "nav combined search reply." << endl;
      parseCombinedSearch( replyBlock );
   }

} catch ( const std::exception& e ) {
   m_replyString = e.what();
}

const Headings& NavSearchHandler::getMatches() const {
   return m_headings;
}

} // SearchViewer
