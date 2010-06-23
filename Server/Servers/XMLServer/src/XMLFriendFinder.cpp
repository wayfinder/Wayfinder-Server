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


#ifdef USE_XML
#include "UserData.h"
#include "XMLTool.h"
#include "XMLServerElements.h"
#include "LocaleUtility.h"
#include "XMLAuthData.h"
#include <sstream>
#include "SearchMatch.h"

using namespace XMLTool;

// Nice macros to have when sending arrays around.
#define BEGIN_ARRAY(x) (x)
#define END_ARRAY(x) ( (x) + (sizeof(x)/sizeof((x)[0]) ) )

/// If this goes beyond a prototype move things up.
/// Keeping it here so it can be removed and changed easily

/// The hardcoded friends list with the hardcoded values.

/**
 * Class for a hardcoded friend with the hardcoded values.
 */
class HardCodedFriendData {
public:
/*
   HardCodedFriendData( uint32 u,
           const MC2String& s,
           const MC2String& d,
           const MC2String& a,
           const MC2String& t ) 
         : uin( u ), status( s ), description( d ), 
           avatarImageName( a ), thumbIconName( t ) 
   {}
*/
   uint32 uin;
   MC2String status;
   MC2String description;
   MC2String avatarImageName;
   MC2String thumbIconName;
};

class FriendLocation {
public:
   FriendLocation( MC2Coordinate p,
                   const MC2String& l,
                   time_t t )
         : pos( p ), locationStr( l ), timeStamp( t )
   {}

   FriendLocation()
   {}

   FriendLocation( const MC2String& s ) {
      // Parse string
      fromString( s );
   }

   MC2String toString() const {
      stringstream str;

      str << pos << ":";
      str << timeStamp << ":";
      str << locationStr;

      return str.str();
   }

   bool fromString( const MC2String& s ) {
      char str[ s.size() ];
      str[ 0 ] = '\0';
      if ( sscanf( s.c_str(), "(%d,%d):%ld:%[^\n]", 
                   &pos.lat, &pos.lon, &timeStamp, str ) >= 3 ) {
         locationStr = str;
         return true;
      } else {
         mc2log << error << "[FL] Invalid position string \"" 
                << s << "\"" << endl;
         return false;
      }
   }

   MC2Coordinate pos;
   MC2String locationStr;
   time_t timeStamp;
};

/**
 * Holder of the complete friend data.
 */
class FriendData {
public:
   FriendData( const HardCodedFriendData& h )
         : uin( h.uin ), status( h.status ), description( h.description ), 
           avatarImageName( h.avatarImageName ), 
           thumbIconName( h.thumbIconName )
   {}

   uint32 uin;
   MC2String status;
   MC2String description;
   MC2String avatarImageName;
   MC2String thumbIconName;
   FriendLocation loc;
   MC2String name;
   MC2String phoneNumber;
   MC2String email;
};

// Store the friend elements so they can be sorted
typedef vector<FriendData> FriendDataCont;
/// Sorts a FriendDataCont using name
class friendDataAlphaSort {
public:
   friendDataAlphaSort( LangTypes::language_t lang )
         : locu( lang )
      {}

   bool operator () ( const FriendData& a, const FriendData& b ) const {
      return locu( LocaleStringRef( a.name ), 
                   LocaleStringRef( b.name ) );
   }

private:
   LocaleCompareLess locu;
};


static HardCodedFriendData friendsArray[] = { 
/*   { 1668151010, "Testing Friend Finder", 
     "Hi I'm a test user for Friend Finder.\r\n"
     "I like to help out with the testing of new features.\r\n"
     "This is my purpose in life.\r\n"
     "Call me for more information.",
     "blue_bird_image", "blue_bird_icon" },*/
};

typedef vector< HardCodedFriendData > FriendsCont;
static FriendsCont friends ( BEGIN_ARRAY( friendsArray),
                             END_ARRAY( friendsArray ) );

class FriendsContUINFinder {
public:
   FriendsContUINFinder( uint32 u ) : m_uin( u ) {}

   bool operator () ( const HardCodedFriendData& o ) const {
      return o.uin == m_uin;
   }

private:
   uint32 m_uin;
};


inline void
randString( MC2String& s, StringTable::languageCode randLang ) {
   s.append( 
      StringTable::getString( 
         StringTable::stringCode( 
            float64(StringTable::NBR_STRINGS)*rand()/(RAND_MAX+1.0) ),
         randLang ) );
}


inline DOMElement* 
makeFriendElement( DOMDocument* reply,
                   int indentLevel, bool indent,
                   const FriendData& f,
                   bool addDescription = false ) {
   DOMElement* friendNode = reply->createElement( X( "friend" ) );
   addAttrib( friendNode, "uin", f.uin );
   addAttrib( friendNode, "name", f.name );
   addAttrib( friendNode, "status", f.status );
   if ( addDescription ) {
      addAttrib( friendNode, "description", f.description );
   }
   MC2String phoneNumber( f.phoneNumber );
   if ( !phoneNumber.empty() ) {
      phoneNumber.insert( phoneNumber.begin(), '+' );
   }
   addAttrib( friendNode, "avatar_image_name", f.avatarImageName );
   addAttrib( friendNode, "thumb_icon_name", f.thumbIconName );
   addAttrib( friendNode, "phone_number", phoneNumber );
   addAttrib( friendNode, "email_address", f.email );
   addAttrib( friendNode, "lat", f.loc.pos.lat );
   addAttrib( friendNode, "lon", f.loc.pos.lon );
   addAttrib( friendNode, "location", f.loc.locationStr );
   addAttrib( friendNode, "time_stamp", f.loc.timeStamp );

   return friendNode;
}


static const MC2String ffStudKey = "FriendFLoc";

inline bool
getAndMakeFriend( ParserThread* thread, FriendData& f,
                  MC2String& errorCode, 
                  MC2String& errorMessage ) {
   bool ok = true;

   // Get friend
   UserItem* userItem = NULL;
   if ( ! thread->getUser( f.uin, userItem, true ) ) {
      ok = false;
      errorCode = "-1";
      errorMessage = "Failed to get friend.";
   }
   // Get friend position. With location string
   FriendLocation loc;
   if ( ok ) {
      MC2String locStr;
      int res = thread->getUserHandler()->getStoredUserData( 
         f.uin, ffStudKey, locStr );

      if ( res == 0 ) {
         loc.fromString( locStr );
      } else if ( res == 1 ) {
         // Use demo values as user has no position
         // FIXME: Not good for a real user application
         FriendLocation demo( MC2Coordinate( 664737481, 157365842 ), 
                              "Barav Lund Sweden",
                              1223899565 );
         loc = demo;
         loc.pos.lat += (uint32)(50000.0*rand()/(RAND_MAX + 1.0));
         loc.pos.lat -= (uint32)(50000.0*rand()/(RAND_MAX + 1.0));
         loc.pos.lon += (uint32)(100000.0*rand()/(RAND_MAX + 1.0));
         loc.pos.lon -= (uint32)(100000.0*rand()/(RAND_MAX + 1.0));
      } else {
         ok = false;
         errorCode = "-1";
         errorMessage = "Failed to get friend location.";
         if ( res == -2 ) {
            errorCode = "-3";
         }
      }
   } // End if ok

   if ( ok ) {
      const UserUser* user = userItem->getUser();
      StringTable::languageCode randLang = 
         StringTable::languageCode( float64(StringTable::getNbrLanguages())*
                                    rand()/(RAND_MAX + 1.0) );

      f.loc = loc;
      f.name = user->getFirstname();
      if ( ! f.name.empty() ) {
         f.name.append( " " );
      }
      f.name.append( user->getLastname() );
      if ( f.name.empty() ) {
         // Fill it
         randString( f.name, randLang );
      }
      if ( f.status.empty() ) {
         randString( f.status, randLang );
      }
      if ( f.description.empty() ) {
         randString( f.description, randLang );
      }
      if ( user->getNbrOfType( UserConstants::TYPE_CELLULAR ) > 0 ) {
         UserCellular* cellular = static_cast< UserCellular* > (
            user->getElementOfType( 0, UserConstants::TYPE_CELLULAR ) );
         f.phoneNumber = cellular->getPhoneNumber();
      }
      f.email = user->getEmailAddress();
   }
   thread->releaseUserItem( userItem );
   
   return ok;
}

bool
XMLParserThread::xmlParseFriendFinder(  DOMNode* cur, 
                                        DOMNode* out,
                                        DOMDocument* reply,
                                        bool indent )
{
   bool ok = true;
   int indentLevel = 1;
   MC2String errorCode;      //errorcode if one should be sent in reply.
   MC2String errorMessage;   //errorMessage if one should be sent in reply

   DOMElement* ff_reply = 
      XMLUtility::createStandardReply( *reply, *cur, "friend_finder_reply" );
   out->appendChild( ff_reply );

   XMLCommonEntities::coordinateType positionSystem =
      XMLCommonEntities::MC2;   
   getAttribValue( positionSystem, "position_system", cur );

   // Get position if any
   MC2Coordinate pos;
   MC2String latStr;
   MC2String lonStr;
   bool hasLat = getAttribValue( latStr, "lat", cur );
   bool hasLon = getAttribValue( lonStr, "lon", cur );
   if ( hasLat && hasLon ) {
      if ( ! XMLCommonEntities::coordinateFromString( latStr.c_str(), pos.lat,
                                                      positionSystem ) ) {
         ok = false;
         errorCode = "-1";
         errorMessage = "Latitude not valid.";
      }
      if ( ! XMLCommonEntities::coordinateFromString( lonStr.c_str(), pos.lon,
                                                      positionSystem ) ) {
         ok = false;
         errorCode = "-1";
         errorMessage = "Longitude not valid.";
      }

      if ( ok ) {
         // Update m_user's position

         // Get location string
         auto_ptr<VanillaMatch> match(
            getVanillaMatchFromPosition( pos, MAX_UINT16/*angle*/,
                                         errorCode, errorMessage ) );
         MC2String locStr;
         if ( match.get() != NULL ) {
            locStr.append( match->getLocationName() );
            if ( ! locStr.empty() ) {
               locStr.append( ", " );
            }
            locStr.append( match->getName() );
         }
         FriendLocation loc( pos, locStr, TimeUtility::getRealTime() );
         // Save in a STUD with key FriendF_Position
         int res = getUserHandler()->setStoredUserData( 
            m_user->getUIN(), ffStudKey, loc.toString() );
         if ( res == 0 ) {
            // Ok
         } else {
            // Just warn about it and let client get is's data
            mc2log << warn << "[xmlParseFriendFinder] Failed to store "
                   << "users location " << MC2CITE( loc.toString() ) << endl;
         }
      }
   }

   
   
   // Make friends list
   DOMElement* friendList = reply->createElement( X( "friend_list" ) );
   FriendDataCont friendsCont;
   for ( FriendsCont::const_iterator it = friends.begin() ;
         it != friends.end() && ok ; ++it ) {
      if ( it->uin != m_user->getUIN() ) { // Don't add self
         friendsCont.push_back( FriendData( *it ) );
         ok = getAndMakeFriend( this, friendsCont.back(),
                                errorCode, errorMessage );
      }
   }
   addAttrib( friendList, "nbr", friendsCont.size() );


   if ( ok ) {
      // Sort friends
      std::sort( friendsCont.begin(), friendsCont.end(), 
                 friendDataAlphaSort( m_authData->clientLang ) );
      
      // Add friends to friendList
      for ( FriendDataCont::const_iterator it = friendsCont.begin() ;
            it != friendsCont.end() ; ++it ) {
         
         friendList->appendChild( makeFriendElement( 
                                     reply, indentLevel + 1, indent,
                                     *it, false/*addDescription*/ ) );
      }
   }

   if ( !ok ) {
      XMLServerUtility::
         appendStatusNodes( ff_reply, reply, 0, false,
                            errorCode.c_str(), errorMessage.c_str() );
      ok = true; // Error handled
   } else {
      ff_reply->appendChild( friendList );
   }

   if ( indent ) {
      XMLUtility::indentPiece( *ff_reply, indentLevel );
   }
   return ok;

}


bool
XMLParserThread::xmlParseFriendFinderInfo(  DOMNode* cur, 
                                            DOMNode* out,
                                            DOMDocument* reply,
                                            bool indent )
{
   bool ok = true;
   int indentLevel = 1;
   MC2String errorCode;      //errorcode if one should be sent in reply.
   MC2String errorMessage;   //errorMessage if one should be sent in reply

   DOMElement* fi_reply = 
      XMLUtility::createStandardReply( *reply, *cur,
                                       "friend_finder_info_reply" );
   out->appendChild( fi_reply );

   uint32 uin = 0;
   getAttribValue( uin, "uin", cur );
   uint32 friendUIN = 0;
   getAttribValue( friendUIN, "friend_uin", cur );

   XMLCommonEntities::coordinateType positionSystem =
      XMLCommonEntities::MC2;   

   getAttribValue( positionSystem, "position_system", cur );
   
   if ( uin == m_user->getUIN() ||
        m_user->getUser()->getEditUserRights() )
   {
      // Find friend among user's friends
      FriendsCont friendsCont;
      FriendsCont::const_iterator findIt = std::find_if( 
         friends.begin(), friends.end(), FriendsContUINFinder( friendUIN ) );
      if ( findIt != friends.end() ) {
         FriendData f( *findIt );
         ok = getAndMakeFriend( this, f,
                                errorCode, errorMessage );
         if ( ok ) {
            fi_reply->appendChild( 
               makeFriendElement( 
                  reply, indentLevel + 1, indent,
                  f, true/*addDescription*/ ) );
         } else {
            ok = false;
            errorCode = "-1";
            errorMessage = "Failed to get friend data.";
         }
      } else {
         ok = false;
         errorCode = "-1";
         errorMessage = "Not a friend of yours.";
      }
   } else {
      mc2log << warn << "[XMLFFI] User (" << uin << ") Has no access to "
             << "(" << friendUIN << ")" << endl;
      ok = false;
      errorCode = "-1";
      errorMessage = "No friend of yours.";
   }


   if ( !ok ) {
      XMLServerUtility::
         appendStatusNodes( fi_reply, reply, 0, false,
                            errorCode.c_str(), errorMessage.c_str() );
      ok = true; // Error handled
   }
   

   if ( indent ) {
      XMLUtility::indentPiece( *fi_reply, indentLevel );
   }
   return ok;

}


#endif // USE_XML

