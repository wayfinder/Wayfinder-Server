/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserThread.h"
#include "PoiReviewItem.h"
#include "XMLTool.h"
#include "ParserPoiReviewHandler.h"
#include "DeleteHelpers.h" // STLUtility
#include "STLStringUtility.h"
#include "XMLServerElements.h"

using namespace XMLTool;
using XMLServerUtility::appendStatusNodes;

bool XMLParserThread::xmlParsePoiReviewRequests( DOMNode* cur, 
                                                 DOMNode* out,
                                                 DOMDocument* reply,
                                                 bool indent )
{
   bool ok = true;
   int indentLevel = 1;

   DOMElement* review_reply = 
      reply->createElement( X( "poi_review_replies" ) );
   out->appendChild( review_reply );
   for ( DOMNode* child = cur->getFirstChild();
         child != NULL && ok; 
         child = child->getNextSibling() ) {
      if ( child->getNodeType() == DOMNode::ELEMENT_NODE ){
         if ( XMLString::equals( child->getNodeName(), 
                                 "poi_review_add_request" ) ) {
            ok = xmlParsePoiReviewAddRequest( child, review_reply, 
                                              reply, indent, indentLevel );
         } else if ( XMLString::equals( child->getNodeName(), 
                                        "poi_review_delete_request" ) ) {
            ok = xmlParsePoiReviewDeleteRequest( child, review_reply, 
                                                 reply, indent, indentLevel );
         } else if ( XMLString::equals( child->getNodeName(), 
                                        "poi_review_list_request" ) ) {
            ok = xmlParsePoiReviewListRequest( child, review_reply, 
                                               reply, indent, indentLevel );
         }
      } else {
         // Ignore nodes that are not ELEMENT_NODE
      }
   }
   if ( indent ) {
      XMLUtility::indentPiece( *review_reply, indentLevel );
   }
   return ok;
}


UserItem* 
XMLParserThread::findUser( uint32 uin, const MC2String& userID, 
                           const MC2String& userSessionID, 
                           const MC2String& userSessionKey )
{
   UserItem* userItem = NULL;
   bool ok = true;
   if ( !userID.empty() ) {
      // Get user by id
      ok = getUser( userID.c_str(), userItem, true );
   } else if ( !userSessionID.empty() ) {
      // Get user by session
      ok = getUserBySession( userSessionID.c_str(), 
                             userSessionKey.c_str(), userItem, true );
   } else if ( uin != 0 ) {
      ok = getUser( uin, userItem, true );
   } else {
      // Get auth user
      ok = getUser( m_user->getUIN(), userItem, true );
   }
   return ok ? userItem : NULL;
}

UserItem*
XMLParserThread::findUser( DOMNode* cur ) {
   UserItem* userItem = NULL;
   bool ok = true;
   MC2String val;
   MC2String val2;
   uint32 uin = 0;
   if ( getElementValue( uin, "uin", cur ) ) {
      // uin 
      ok = getUser( uin, userItem, true );
   } else if ( getElementValue( val, "user_id", cur ) ) {
      // user_id 
      ok = getUser( val.c_str(), userItem, true );
   } else if ( getElementValue( val, "user_session_id", cur ) &&
               getElementValue( val2, "user_session_key", cur ) ) {
      // user_session_id, user_session_key
      ok = getUserBySession( val.c_str(), val2.c_str(), userItem, true );
   }

   return userItem;
}
namespace {
/**
 * Append status nodes with res status converted for poi review requests.
 */
void
addPoiReviewStatusNodes( int res, 
                         DOMElement* replyNode, DOMDocument* reply, 
                         const MC2String& out_review_id = MC2String() ) {
   if ( res == 0 ) {
      // OK
      appendStatusNodes( replyNode, reply, 0, false, "0", "Ok." );
      if ( !out_review_id.empty() ) {
         replyNode->setAttribute( X( "review_id" ), X( out_review_id ) );
      }
   } else if ( res == -4 ) {
      appendStatusNodes( replyNode, reply, 0, false, "-601", "Not allowed." );
   } else if ( res == -3 ) {
      appendStatusNodes( replyNode, reply, 0, false, "-600", "Not found." );
   } else if ( res == -2 ) {
      appendStatusNodes( replyNode, reply, 0, false, "-3", "Timeout." );
   } else {
      appendStatusNodes( replyNode, reply, 0, false, "-1", "Error." );
   }
}

}

bool
XMLParserThread::xmlParsePoiReviewAddRequest( DOMNode* cur, 
                                              DOMNode* out,
                                              DOMDocument* reply,
                                              int indentLevel,
                                              bool indent )
{
   bool ok = true;

   MC2String transaction_id;
   getAttrib( transaction_id, "transaction_id", cur );
   MC2String poi_id;
   getAttrib( poi_id, "poi_id", cur );
   MC2String review_id;
   bool hasreview_id = false;
   if ( getAttribValue( review_id, "review_id", cur ) ) {
      hasreview_id = true;
   }
   LangTypes::language_t lang = LangTypes::invalidLanguage;
   {
      MC2String lang_str;
      if ( getAttribValue( lang_str, "lang", cur ) ) {
         lang = getStringAsLanguage( lang_str.c_str() );
      }
   }
   int grade = 0;
   getAttrib( grade, "grade", cur, 0 );
   
   UserItem* userItem = findUser( cur );

   MC2String review_title;
   bool hasreview_title = false;
   if ( getElementValue( review_title, "poi_review_title", cur ) ) {
      hasreview_title = true;
   }

   MC2String review_text;
   bool hasreview_text = false;
   if ( getElementValue( review_text, "poi_review_text", cur ) ) {
      hasreview_text = true;
   }

   DOMElement* add_reply = reply->createElement( X( "poi_review_add_reply" ) );
   out->appendChild( add_reply );
   add_reply->setAttribute( X( "transaction_id" ), X( transaction_id ) );

   if ( userItem == NULL ) {
      appendStatusNodes( add_reply, reply, 0, false,
                         "-202", "Unknown user." );
   } else if ( ! m_user->getUser()->getEditUserRights() &&
               m_user->getUIN() != userItem->getUIN() ) {
      appendStatusNodes( add_reply, reply, 0, false,
                         "-201", "Access denied." );
   } else {
      MC2String out_review_id;
      int res = getPoiReviewHandler().poiReview( userItem->getUIN(),
                                                 poi_id, grade, lang, 
                                                 out_review_id, review_id, 
                                                 review_title, review_text );
      addPoiReviewStatusNodes( res, add_reply, reply, out_review_id );
   }

   if ( indent ) {
      XMLUtility::indentPiece( *add_reply, indentLevel );
   }

   releaseUserItem( userItem );
   return ok;
}

bool XMLParserThread::xmlParsePoiReviewDeleteRequest( DOMNode* cur, 
                                                      DOMNode* out,
                                                      DOMDocument* reply,
                                                      int indentLevel,
                                                      bool indent )
{
   bool ok = true;

   MC2String transaction_id;
   getAttrib( transaction_id, "transaction_id", cur );
   MC2String review_id;
   bool hasreview_id = false;
   if ( getAttribValue( review_id, "review_id", cur ) ) {
      hasreview_id = true;
   }

   DOMElement* delete_reply = reply->createElement( 
      X( "poi_review_delete_reply" ) );
   out->appendChild( delete_reply );

   UserItem* userItem = findUser( cur );
   if ( userItem == NULL ) {
      appendStatusNodes( delete_reply, reply, 0, false,
                         "-202", "Unknown user." );
   } else if ( ! m_user->getUser()->getEditUserRights() &&
               m_user->getUIN() != userItem->getUIN() ) {
      appendStatusNodes( delete_reply, reply, 0, false,
                         "-201", "Access denied." );
   } else {
      int res = getPoiReviewHandler().deleteReview( userItem->getUIN(), 
                                                    review_id );
      addPoiReviewStatusNodes( res, delete_reply, reply );
   }

   if ( indent ) {
      XMLUtility::indentPiece( *delete_reply, indentLevel );
   }

   releaseUserItem( userItem );
   return ok;
}

bool XMLParserThread::xmlParsePoiReviewListRequest( DOMNode* cur, 
                                                    DOMNode* out,
                                                    DOMDocument* reply,
                                                    int indentLevel,
                                                    bool indent )
{
   bool ok = true;

   MC2String transaction_id;
   getAttrib( transaction_id, "transaction_id", cur );
   LangTypes::language_t lang = LangTypes::invalidLanguage;
   {
      MC2String lang_str;
      if ( getAttribValue( lang_str, "lang", cur ) ) {
         lang = getStringAsLanguage( lang_str.c_str() );
      }
   }
   MC2String review_id;
   bool hasreview_id = false;
   MC2String poi_id;
   bool haspoi_id = false;
   MC2String detailsStr;
   getAttrib( detailsStr, "details", cur );
   PoiReviewEnums::reviewDetails_t details = 
      PoiReviewEnums::reviewDetailsFromString( detailsStr.c_str() );

   DOMNode* foundNode = findNode( cur, "poi_review_poi" );
   if ( foundNode != NULL ) {
      if ( getAttribValue( poi_id, "poi_id", foundNode ) ) {
         haspoi_id = true;
      }
   }
   foundNode = findNode( cur, "poi_review_id" );
   if ( foundNode != NULL ) {
      if ( getAttribValue( review_id, "review_id", foundNode ) ) {
         hasreview_id = true;
      }
   }
   
   DOMElement* list_reply = reply->createElement( 
      X( "poi_review_list_reply" ) );
   out->appendChild( list_reply );
   list_reply->setAttribute( X( "transaction_id" ), X( transaction_id ) );

   UserItem* userItem = findUser( cur );

   typedef STLUtility::AutoContainer< vector<PoiReviewItem*> > reviewC;
   reviewC reviews;
   int res = getPoiReviewHandler().listReviews( 
      userItem ? userItem->getUIN() : 0, review_id, poi_id, details, lang,
      reviews );

   if ( res == 0 ) {
      // OK, make list 
      // poi_review*
      for ( uint32 i = 0 ; i < reviews.size() ; ++i ) {
         DOMElement* poiEl = addNode( list_reply, "poi_review" );
         PoiReviewItem* poi = reviews[ i ];
         addAttrib( poiEl, "poi_id", poi->getPoiID() );
         if ( poi->getTotalVotes() != 0 ) {
            MC2String str;
            addAttrib( poiEl, "avg_grade", STLStringUtility::float2Str( 
                          float64(poi->getTotalGrades()) / 
                          poi->getTotalVotes(), str ) );
            str = "";
            addAttrib( poiEl, "grade_count", STLStringUtility::uint2str( 
                          poi->getTotalVotes(), str ) );
         }
         for ( uint32 j = 0 ; j < poi->getReviews().size() ; ++j ) {
            const PoiReviewDetail& d = poi->getReviews()[ j ];
            DOMElement* dEl = addNode( poiEl, "poi_review_detail" );
            if ( d.getOwnerUIN() != 0 ) {
               DOMElement* uinEl = addNode( dEl, "uin" );
               setNodeValue( uinEl, d.getOwnerUIN() );
            }
            if ( !d.getTitle().empty() ) {
               DOMElement* titleEl = addNode( dEl, "poi_review_title" );
               setNodeValue( titleEl, d.getTitle() );
            }
            if ( !d.getText().empty() ) {
               DOMElement* textEl = addNode( dEl, "poi_review_text" );
               setNodeValue( textEl, d.getText() );
            }
            addAttrib( dEl, "review_id", d.getReviewID() );
            if ( d.getGrade() != MAX_UINT32 ) {
               addAttrib( dEl, "grade", d.getGrade() );
            }
            if ( d.getTime() != 0 ) {
               addAttrib( dEl, "date", d.getTime() );
            }
            if ( !d.getLogonID().empty() ) {
               addAttrib( dEl, "logonID", d.getLogonID() );
            }
            if ( !d.getFirstname().empty() ) {
               addAttrib( dEl, "firstname", d.getFirstname() );
            }
            if ( !d.getLastname().empty() ) {
               addAttrib( dEl, "lastname", d.getLastname() );
            }
         }
      }

   } else if ( res == -3 ) {
      appendStatusNodes( list_reply, reply, 0, false, "-600", "Not found." );
   } else if ( res == -2 ) {
      appendStatusNodes( list_reply, reply, 0, false, "-3", "Timeout." );
   } else {
      appendStatusNodes( list_reply, reply, 0, false, "-1", "Error." );
   }

   if ( indent ) {
      XMLUtility::indentPiece( *list_reply, indentLevel );
   }

   releaseUserItem( userItem );
   return ok;
}

