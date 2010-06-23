/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ParserPoiReviewHandler.h"
#include "PoiReviewItem.h"
#include <memory>
#include "PacketContainer.h"
#include "ParserThread.h"
#include "Packet.h"

ParserPoiReviewHandler::ParserPoiReviewHandler( 
   ParserThread* thread, ParserThreadGroup* group )
      : ParserHandler( thread, group )
{
}

int
ParserPoiReviewHandler::poiReview( uint32 uin,
                                   const MC2String& poiID,
                                   uint32 grade,
                                   LangTypes::language_t lang,
                                   MC2String& out_review_id,
                                   const MC2String& review_id,
                                   const MC2String& title, 
                                   const MC2String& text )
{
   int res = -1;

   PoiReviewItem p( poiID );
   p.addReview( PoiReviewDetail( uin, grade, review_id, title, text, lang ) );
   RequestPacket* r = new RequestPacket( 
      REQUEST_HEADER_SIZE + p.getSizeAsBytes(), 
      Packet::PACKETTYPE_POIREVIEW_ADD_REQUEST );
   int pos = REQUEST_HEADER_SIZE;
   p.save( r, pos );
   r->setLength( pos );

   auto_ptr<PacketContainer> replyCont( m_thread->putRequest(  
                                           r, MODULE_TYPE_USER ) );
   if ( replyCont.get() != NULL ) {
      ReplyPacket* rp = static_cast< ReplyPacket* > ( replyCont->getPacket() );
      if ( rp->getStatus() == StringTable::OK ) { 
         int pos = REPLY_HEADER_SIZE;
         out_review_id = rp->incReadString( pos );
         res = 0;
      } else if ( rp->getStatus() == StringTable::NOT_ALLOWED  ) {
         res = -4;
      } else if ( rp->getStatus() == StringTable::NOTFOUND  ) {
         res = -3;
      } else {
         res = -1;
      }
   } else {
      res = -2;
   }

   return res;
}

int 
ParserPoiReviewHandler::deleteReview( uint32 uin, const MC2String& review_id )
{
   int res = -1;

   RequestPacket* r = new RequestPacket( 
      REQUEST_HEADER_SIZE + 4 + review_id.size() +4, 
      Packet::PACKETTYPE_POIREVIEW_DELETE_REQUEST );
   int pos = REQUEST_HEADER_SIZE;
   r->incWriteLong( pos, uin );
   r->incWriteString( pos, review_id );
   r->setLength( pos );

   auto_ptr<PacketContainer> replyCont( m_thread->putRequest(  
                                           r, MODULE_TYPE_USER ) );
   if ( replyCont.get() != NULL ) {
      ReplyPacket* rp = static_cast< ReplyPacket* > ( replyCont->getPacket() );
      if ( rp->getStatus() == StringTable::OK ) { 
         res = 0;
      } else if ( rp->getStatus() == StringTable::NOT_ALLOWED  ) {
         res = -4;
      } else if ( rp->getStatus() == StringTable::NOTFOUND  ) {
         res = -3;
      } else {
         res = -1;
      }
   } else {
      res = -2;
   }
   return res;
}

int
ParserPoiReviewHandler::listReviews( uint32 ownerUIN,
                                     const MC2String& review_id,
                                     const MC2String& poiID,
                                     PoiReviewEnums::reviewDetails_t detail, 
                                     LangTypes::language_t lang,
                                     vector<PoiReviewItem*>& reviews )
{
   int res = -1;

   RequestPacket* r = new RequestPacket( 
      REQUEST_HEADER_SIZE + 3*4 + review_id.size() +1 + poiID.size() +1 +3, 
      Packet::PACKETTYPE_POIREVIEW_LIST_REQUEST );
   int pos = REQUEST_HEADER_SIZE;
   r->incWriteLong( pos, ownerUIN );
   r->incWriteString( pos, review_id );
   r->incWriteString( pos, poiID );
   r->incWriteLong( pos, detail );
   r->incWriteLong( pos, lang );
   r->setLength( pos );

   auto_ptr<PacketContainer> replyCont( m_thread->putRequest(  
                                           r, MODULE_TYPE_USER ) );
   if ( replyCont.get() != NULL ) {
      ReplyPacket* rp = static_cast< ReplyPacket* > ( replyCont->getPacket() );
      if ( rp->getStatus() == StringTable::OK ) { 
         res = 0;
         int rpos = REPLY_HEADER_SIZE;
         uint32 nbrPois = rp->incReadLong( rpos );
         for ( uint32  i = 0 ; i < nbrPois ; ++i ) {
            PoiReviewItem* p = new PoiReviewItem();
            p->load( rp, rpos );
            reviews.push_back( p );
         }
      } else if ( rp->getStatus() == StringTable::NOT_ALLOWED  ) {
         res = -4;
      } else if ( rp->getStatus() == StringTable::NOTFOUND  ) {
         res = -3;
      } else {
         res = -1;
      }
   } else {
      res = -2;
   }

   return res;
}


