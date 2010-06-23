/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavTrackHandler.h"
#include "NavRequestData.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "UserData.h"
#include "NavUserHelp.h"
#include "AddUserTrackRequest.h"
#include "isabBoxSession.h"
#include "STLStringUtility.h"
#include "GetUserTrackRequest.h"
#include "ParserPosPush.h"
#include "TimeUtility.h"

bool UserPIN_compareIDs( UserPIN& a, UserPIN& b) {
   return a.getID() < b.getID();
}


#define addpin( p, pin ) p.addUint32( pin->getID() ); \
    p.addString( pin->getPIN(), m_thread->clientUsesLatin1() ); \
    p.addString( pin->getComment(), m_thread->clientUsesLatin1() );


NavTrackHandler::NavTrackHandler( InterfaceParserThread* thread,
                                  NavParserThreadGroup* group,
                                  NavUserHelp* userHelp )
      : NavHandler( thread, group ),
        m_userHelp( userHelp )
{
   m_expectations.push_back( ParamExpectation( 
                                5200, NParam::Byte_array ) );
   m_expectations.push_back( ParamExpectation( 
                                5201, NParam::Uint32_array ) );
   m_expectations.push_back( ParamExpectation( 
                                5202, NParam::Uint32_array ) );
   m_expectations.push_back( ParamExpectation( 
                                5203, NParam::Byte_array ) );
   m_expectations.push_back( ParamExpectation( 
                                5204, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 
                                5205, NParam::String ) );
   m_expectations.push_back( ParamExpectation( 
                                5206, NParam::Uint32_array ) );
}


bool
NavTrackHandler::handleTracking( NavRequestData& rd ) {
   if ( !checkExpectations( rd.req->getParamBlock(), rd.reply ) ) {
      return false;
   }

   bool ok = true;

   // The user
   UserUser* user = rd.session->getUser()->getUser();

   // Start parameter printing
   mc2log << info << "handleTrack:";
   if ( rd.params.getParam( 5200 ) ) {
      vector< const NParam* > pf;
      rd.params.getAllParams( 5200, pf );
      uint32 size = 0;
      for ( uint32 i = 0 ; i < pf.size() ; ++i ) {
         size += pf[ i ]->getLength();
      }
      mc2log << " Got " << size << " tracking bytes";
   }
   if ( rd.params.getParam( 5201 ) ) {
   }

   if ( rd.params.getParam( 5205 ) ) {
      mc2log << " TrackReq: " << rd.params.getParam( 5205 )->getString( 
         m_thread->clientUsesLatin1() );
      if ( rd.params.getParam( 5206 ) ) {
         // Read settings
         const NParam* p = rd.params.getParam( 5206 );
         if ( p->getLength() >= 4 ) {
            mc2log << " maxNbrTracks " << p->getUint32( 0 );
         }
         if ( p->getLength() >= 8 ) {
            mc2log << " startTime " << p->getUint32( 4 );
         }
         if ( p->getLength() >= 12 ) {
            mc2log << " endTime " << p->getUint32( 8 );
         }
      }
   }

   mc2log << endl;


   if ( rd.params.getParam( 5200 ) ) {
      // Track points
      // lat, lon, dist, heading, speed, time, comment
      vector< const NParam* > pf;
      rd.params.getAllParams( 5200, pf );

      AddUserTrackRequest* req = new AddUserTrackRequest( 
         m_thread->getNextRequestID(), user->getUIN() );
      uint32 now = TimeUtility::getRealTime();
      vector< UserTrackElement* > posItems;

      for ( uint32 i = 0 ; i < pf.size() ; ++i ) {
         const NParam* p = pf[ i ];
         uint32 pos = 0;
         while ( pos + 21 <= p->getLength() ) {
            // lat    4
            int32 lat = p->getInt32( pos );
            pos += 4;
            // lon    4
            int32 lon = p->getInt32( pos );
            pos += 4;
            // dist   4
            uint32 dist = p->getUint32( pos );
            pos += 4;
            // speed  2  In m/s * 32
            uint16 speed = p->getUint16( pos );
            pos += 2;
            // heading 2 In 256 degrees
            uint16 heading = (p->getUint16( pos ))*360/256;
            pos += 2;
            // time   4  Unixtime
            uint32 time = p->getUint32( pos );
            if ( time < 100000000 ) { // Time relative now
               time = now - time;
            }
            pos += 4;
            // comment String
            MC2String comment = p->incGetString( 
               m_thread->clientUsesLatin1(), pos );

            MC2String source( "Track" );
            if ( !comment.empty() ) {
               source.append( ": " );
               source.append( comment );
            }

            MC2Coordinate coord( Nav2Coordinate( lat, lon ) );

            mc2dbg/*2*/ << "Tracking coord " << coord 
                        << " dist " << int32(dist) << " speed " 
                        << (float64(speed)/32) << " heading " << heading
                        << " time " << time << " comment \"" << comment
                        << "\"" << endl;
            posItems.push_back( 
               new UserTrackElement( coord.lat, coord.lon, dist, speed,
                                     heading, time, source.c_str() ) );
            // req owns the UserTrackElement
            req->addUserTrackElement( posItems.back() );
         } // End while more in param
      } // End for all 5200 params

      int pres = 0;

      if ( !posItems.empty() ) {
         pres = m_thread->getPosPush().positionsReceived(
            posItems, rd.session->getUser(), rd.clientSetting );
         posItems.clear(); // req owns the UserTrackElements
      }

      PacketContainer* answerCont = NULL;
      if ( pres != 1 ) {
         m_thread->putRequest( req );
         answerCont = req->getAnswer();

         if ( req->getStatus() != StringTable::OK ) {
            mc2log << "handleTrack: AddUserTrackRequest Failed" << endl;
            rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         }
      }
      
      delete answerCont;
      delete req;
   } // End if have 5200 param(s)


   if ( rd.params.getParam( 5201 ) ) {
      bool ok = true;
      // PIN(s)
      UserUser* cuser = new UserUser( *user );

      // Add new PIN(s)
      if ( rd.params.getParam( 5203 ) ) {
         const NParam* p = rd.params.getParam( 5203 );
         uint32 pos = 0;
         while ( pos < p->getLength() ) {
            /*uint32 id = p->getUint32( pos );*/
            pos += 4;
            MC2String pin = p->incGetString( 
               m_thread->clientUsesLatin1(), pos );
            MC2String comment = p->incGetString( 
               m_thread->clientUsesLatin1(), pos );

            cuser->addElement( new UserPIN( 0, pin.c_str(), 
                                            comment.c_str() ) );
            mc2dbg/*4*/ << "Adding pIN " << pin << endl;
         }
      }

      // Delete PINIDDel
      if ( rd.params.getParam( 5202 ) ) {
         const NParam* p = rd.params.getParam( 5202 );
         uint32 pos = 0;
         while ( pos < p->getLength() ) {
            uint32 id = p->getUint32( pos );
            pos += 4;
            UserElement* el = cuser->getElementOfTypeWithID( 
               UserConstants::TYPE_PIN, id );
            if ( el != NULL ) {
               el->remove();
            }
            mc2dbg/*4*/ << "Deleteing pIN id " << id << endl;
         }
      }

      // Sync with database
      if ( m_thread->changeUser( cuser, user/*changer*/ ) ) {
         // Ok 
      } else {
         ok = false;
         rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         mc2log << warn << "handleTrack: Failed to change PINs"
                << "for "
                << user->getLogonID() << "("
                << user->getUIN() << ")" << endl;
      }
      
      // Get new user with up to date ids
      UserItem* rUserItem = NULL;
      if ( m_thread->getUser( user->getUIN(), 
                              rUserItem, true ) ) 
      {
         if ( rUserItem != NULL ) {
            // DONE!
            rd.session->setUser( new UserItem( *rUserItem ) );
            user = rd.session->getUser()->getUser();
         } else {
            // We should never get here, we have valid UIN, 
            // but jic.
            ok = false;
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_NOT_OK );
            mc2log << warn << "handleTrack: not able to get "
                   << "updated user " 
                   << user->getLogonID() 
                   << "(" << user->getUIN() << ") in "
                   << "db but I just had it." << endl;
         }
      } else {
         rd.reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
         ok = false;
         mc2log << warn << "handleTrack: failed to get "
                << " updated user " 
                << user->getLogonID() 
                << "(" << user->getUIN() << ")" << endl;
      }

      if ( ok ) {
         UserUser* userUser = rUserItem->getUser();
         // Make new lists to send to client also make crc
         // 5302 PINIDDel  uint32 array 
         // 5303 PINAdd    byte array 
         // 5304 PINCRC     uint32 
         typedef list< UserPIN > uList;
         uList syncList;
         uList serverList;
      
         const NParam* p = rd.params.getParam( 5201 );
         uint32 pos = 0;
         while ( pos < p->getLength() ) {
            uint32 id = p->getUint32( pos );
            pos += 4;
            syncList.push_back( UserPIN( id ) );
            mc2dbg/*4*/ << "Sync id " << id << endl;
         }
         syncList.sort( UserPIN_compareIDs );

         for ( uint32 i = 0 ;
               i < userUser->getNbrOfType( UserConstants::TYPE_PIN ) ; 
               i++ )
         {
            UserPIN* t = static_cast< UserPIN* >( 
               userUser->getElementOfType( i, UserConstants::TYPE_PIN ) );
            serverList.push_back( *t );
            mc2dbg/*4*/ << "Server PIN " << (*t).getPIN() << " id " 
                    << (*t).getID() << endl;
         }
         serverList.sort( UserPIN_compareIDs );


         uList::iterator s = serverList.begin();
         uList::iterator c = syncList.begin();

         NParam& delList = rd.rparams.addParam( 5302 );
         NParam& addList = rd.rparams.addParam( 5303 );
         
         while ( !(s == serverList.end() || c == syncList.end()) ) {
            uint32 sID = s->getID();
            uint32 cID = c->getID();
            if ( sID == cID ) {
               // identical, nothing to send
               serverList.erase( s++ );
               syncList.erase(   c++ );
            } else if ( sID < cID ) {
               // s not in syncList (client list), add
               mc2dbg/*4*/ << "Add " << (*s).getPIN() << " id " << (*s).getID()
                       << endl;
               addpin( addList, s );
               serverList.erase( s++ );
            } else if ( cID < sID ) {
               // c not in serverList, delete
               mc2dbg/*4*/ << "Del ID " << cID << endl;
               delList.addUint32( cID );
               syncList.erase( c++ );
            }
         }
         // add any tails left in the lists
         while ( s != serverList.end() ) {
            mc2dbg/*4*/ << "Add tail " << (*s).getPIN() << " id "
                    << (*s).getID() << endl;
            addpin( addList, s );
            serverList.erase( s++ );
         }
         while ( c != syncList.end()) {
            mc2dbg/*4*/ << "Del ID " << c->getID() << endl;
            delList.addUint32( c->getID() );
            syncList.erase( c++ );
         }
      
         // Make and add crc
         rd.rparams.addParam( 
            NParam( 5304, m_userHelp->getUsersPINsCRC( rUserItem ) ) );
      }

      m_thread->releaseUserItem( rUserItem );
      delete cuser;
   }


   if ( rd.params.getParam( 5205 ) ) {
      uint32 startTime = 0;
      uint32 endTime = MAX_UINT32;
      uint32 maxNbrTracks = 1;

      if ( rd.params.getParam( 5206 ) ) {
         // Read settings
         const NParam* p = rd.params.getParam( 5206 );
         if ( p->getLength() >= 4 ) {
            maxNbrTracks = p->getUint32( 0 );
         }
         if ( p->getLength() >= 8 ) {
            startTime = p->getUint32( 4 );
         }
         if ( p->getLength() >= 12 ) {
            endTime = p->getUint32( 8 );
         }
      }
      
      // Parse string (usertobetrackedid:PIN:)
      MC2String trackReqStr = rd.params.getParam( 5205 )->getString( 
         m_thread->clientUsesLatin1() );
      vector<MC2String> r = STLStringUtility::explode( 
         ":", trackReqStr );
      if ( r.size() >= 2 ) {
         // Get UIN/UserID
         UserItem* tuserItem = NULL;
         uint32 UIN = m_userHelp->getUIN( r[ 0 ].c_str() );
         if ( UIN != 0 && UIN != MAX_UINT32 ) {
            uint32 startTime = TimeUtility::getCurrentTime();
            ok = m_thread->getUser( UIN, tuserItem, true );
            uint32 endTime = TimeUtility::getCurrentTime();
            if ( !ok ) {
               mc2log << warn << "handleTrack: Get track user " << r[ 0 ] 
                      << "(" << UIN << ") Error: ";
               if ( endTime - startTime > 3000 ) {
                  rd.reply->setStatusCode( 
                     NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
                  mc2log << "Timeout";
               } else {
                  rd.reply->setStatusCode( 
                     NavReplyPacket::NAV_STATUS_NOT_OK );
                  mc2log << "Error";
               }
               mc2log << endl;
            }
         } else {
            ok = false;
            mc2log << warn << "handleTrack: Failed to get user from "
                   << "TrackReq string \"" << trackReqStr << "\": ";
            if ( UIN == MAX_UINT32 ) {
               mc2log << "Timeout";
               rd.reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
            } else {
               // User not found
               mc2log << "Error";
               rd.reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_NOT_OK ); // TODO: New status code
            }
            mc2log << endl;
         }
         
         if ( ok ) {
            // Get PIN
            MC2String PIN( r[ 1 ] );

            // Check if PIN in tuserItem
            bool pinMatches = false;
            for ( uint32 i = 0 ;
                  i < tuserItem->getUser()->getNbrOfType( 
                     UserConstants::TYPE_PIN ) ; i++ )
            {
               UserPIN* t = static_cast< UserPIN* >( 
                  tuserItem->getUser()->getElementOfType( 
                     i, UserConstants::TYPE_PIN ) );
               if ( PIN == t->getPIN() ) {
                  pinMatches = true; 
                  break;
               }
            }

            if ( !pinMatches ) {
               mc2log << warn << "handleTrack: PIN \"" << PIN << "\"not "
                      << "found in user " 
                      << tuserItem->getUser()->getLogonID() << "(" 
                      << tuserItem->getUIN() << ")" << endl;
               rd.reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_NOT_OK ); // TODO: New status code
               ok = false;
            }

            if ( ok ) {
               // Get tracks
               GetUserTrackRequest* req = new GetUserTrackRequest( 
                  m_thread->getNextRequestID(), tuserItem->getUIN() );
               
               req->setTimeInterval( startTime, endTime );
               req->setMaxNbrHits( maxNbrTracks );
               m_thread->putRequest( req );
               if ( req->getStatus() == StringTable::OK ) {
                  // Append user track data
                  NParam& r = rd.rparams.addParam( NParam( 5305 ) );

                  for ( UserTrackElementsList::const_iterator it = 
                      req->getResult().begin() ;
                        it != req->getResult().end() ; ++it )
                  {
                     Nav2Coordinate navCoord( 
                        MC2Coordinate( (*it)->getLat(), (*it)->getLon() ));
                     // lat    4
                     r.addInt32( navCoord.nav2lat );
                     // lon    4
                     r.addInt32( navCoord.nav2lon );
                     // dist   4
                     r.addUint32( (*it)->getDist() );
                     // speed  2  In m/s * 32
                     r.addUint16( (*it)->getSpeed() );
                     // heading 2 In 256 degrees
                     r.addUint16( (*it)->getHeading() * 256/360 );
                     // time   4  Unixtime
                     r.addUint32( (*it)->getTime() );
                     // comment String
                     r.addString( (*it)->getSource(), 
                                  m_thread->clientUsesLatin1() );
                     mc2dbg/*2*/ << "Tracking item (" << (*it)->getLat()
                                 << "," << (*it)->getLon() << " dist "
                                 << int32((*it)->getDist()) << " speed "
                                 << (*it)->getSpeed() << " heading "
                                 << (*it)->getHeading() << endl;
                  }

                  mc2log << info << "handleTrack: TrackReq: Nbr items "
                         << req->getResult().size() << " max "
                         << maxNbrTracks << " start "
                         << startTime << " end " << endTime << " User "
                         << tuserItem->getUser()->getLogonID() << "("
                         << tuserItem->getUIN() << ")" << endl;
               } else {
                  mc2log << warn << "handleTrack: Faild to process "
                         << "UserTrackRequest Status: " 
                         << StringTable::getString( req->getStatus(), 
                                                    StringTable::ENGLISH )
                         << endl;
                  ok = false;
                  rd.reply->setStatusCode( 
                     NavReplyPacket::NAV_STATUS_NOT_OK );
                  if ( req->getStatus() == StringTable::TIMEOUT_ERROR ) {
                     rd.reply->setStatusCode( 
                        NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
                  }
               }
               delete req;
            }
         }

         m_thread->releaseUserItem( tuserItem );
      } else {
         mc2log << warn << "handleTrack: Bad TrackReq string \""
                << trackReqStr << "\"" << endl;
         ok = false;
         rd.reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_PARAMETER_INVALID );
      }
      
   } // End parameter 5205


   return ok;
}
