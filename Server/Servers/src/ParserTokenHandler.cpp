/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ParserTokenHandler.h"
#include "UserData.h"
#include "ParserThread.h"
#include "ClientSettings.h"
#include "TimeUtility.h"

#define TOKEN_VALID_TIME 5*24*60*60
#define TOKEN_STILL_HOT_TIME 5*60

ParserTokenHandler::ParserTokenHandler( ParserThread* thread, 
                                        ParserThreadGroup* group )
      : ParserHandler( thread, group )
{
}


const char* 
ParserTokenHandler::tokenGroup( const UserUser* user,
                                const ClientSetting* clientSetting ) const
{
   if ( clientSetting != NULL && 
        m_thread->checkIfIronClient( clientSetting ) ) 
   {
      mc2dbg4 << "tokenGroup IRON" << endl;
      return "IRON";
   } else {
      mc2dbg4 << "tokenGroup \"\"" << endl;
      return "";
   }
}


MC2String
ParserTokenHandler::makeTokenStr() {
   MC2String res;
   const char randChars[80] = 
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
   for ( uint32 i = 0 ; i < 12 ; ++i ) {
      res += randChars[(int)((strlen(randChars)+0.0)*rand()/
                             (RAND_MAX+1.0))];
   }

   return res;
}


int
ParserTokenHandler::setToken( UserUser* user, const MC2String& tokenStr,
                              const ClientSetting* clientSetting,
                              bool increaseAge, bool sendChange)
{
   int res = 0;
   // Remove all current
   byte age = 0;
   uint32 now = TimeUtility::getRealTime();
   const char* groupStr = tokenGroup( user, clientSetting );
   for ( uint32 i = 0 ;
         i < user->getNbrOfType( UserConstants::TYPE_TOKEN ) ; i++ )
   {
      UserToken* t = static_cast< UserToken* >( 
         user->getElementOfType( i, UserConstants::TYPE_TOKEN ) );
      if ( t->getGroup() == groupStr ) {
         if ( t->getAge() > age ) {
            age = t->getAge();
         }
         if ( int32(now) - int32(t->getCreateTime()) > 
              TOKEN_STILL_HOT_TIME ) 
         {
            mc2dbg4 << "Removing Token: ID " << t->getID() 
                    << " CreateTime " 
                    << t->getCreateTime() << " Age " << int(t->getAge())
                    << " Token " << t->getToken() << endl;
            t->remove();
         }
      }
   }


   // Add new
   UserToken* newT = new UserToken( 0, TimeUtility::getRealTime(), 
                                    increaseAge ? age + 1 : age, 
                                    tokenStr.c_str(), groupStr );
   user->addElement( newT );

   uint32 startTime = TimeUtility::getCurrentTime();
   if ( sendChange && !m_thread->changeUser( user, NULL/*changer*/ ) ) {
      res = -1;
      uint32 endTime = TimeUtility::getCurrentTime();
      if ( (endTime-startTime) > 2000 ) {
         res = -2;
      }
   } // Else changed ok

   return res;
}


UserToken*
ParserTokenHandler::getToken( UserUser* user, const MC2String& tokenStr,
                              const ClientSetting* clientSetting )
{
   const char* groupStr = tokenGroup( user, clientSetting );
   for ( uint32 i = 0 ;
         i < user->getNbrOfType( UserConstants::TYPE_TOKEN ) ; i++ )
   {
      UserToken* t = static_cast< UserToken* >( 
         user->getElementOfType( i, UserConstants::TYPE_TOKEN ) );
      if ( tokenStr.compare( t->getToken() ) == 0 &&
           (clientSetting == NULL || t->getGroup() == groupStr) )
      {
         return t;
      }
   }

   return NULL;
}


const UserToken*
ParserTokenHandler::getToken( const UserUser* user, 
                              const MC2String& tokenStr,
                              const ClientSetting* clientSetting ) const
{
   return const_cast< ParserTokenHandler* > ( this )->getToken( 
      const_cast< UserUser* > ( user ), tokenStr, clientSetting );
}


bool
ParserTokenHandler::expiredToken( UserUser* user, 
                                  const MC2String& tokenStr,
                                  const ClientSetting* clientSetting )
{
   UserToken* t = getToken( user, tokenStr, clientSetting );
   if ( t == NULL ) {
      return false;
   }
   uint32 now = TimeUtility::getRealTime();

   // Uses int32 to support createTimes in the future (easier testing
   // of operator integrations)
   if ( int32(now) - int32(t->getCreateTime()) > TOKEN_VALID_TIME ) {
      return true;
   }

   return false;
}


int
ParserTokenHandler::addNewToken( UserUser* user, 
                                 const MC2String& tokenStr, 
                                 const ClientSetting* clientSetting,
                                 bool sendChange )
{
   int res = 0;
   byte age = 0;
   const char* groupStr = tokenGroup( user, clientSetting );
   for ( uint32 i = 0 ;
         i < user->getNbrOfType( UserConstants::TYPE_TOKEN ) ; i++ )
   {
      UserToken* t = static_cast< UserToken* >( 
         user->getElementOfType( i, UserConstants::TYPE_TOKEN ) );
      if ( t->getGroup() == groupStr && t->getAge() > age ) {
         age = t->getAge();
      }
   }

   // Add new
   UserToken* newT = new UserToken( 0, TimeUtility::getRealTime(), 
                                    age, tokenStr.c_str(), groupStr );
   user->addElement( newT );

   uint32 startTime = TimeUtility::getCurrentTime();
   if ( sendChange && !m_thread->changeUser( user, NULL/*changer*/ ) ) {
      res = -1;
      uint32 endTime = TimeUtility::getCurrentTime();
      if ( (endTime-startTime) > 2000 ) {
         res = -2;
      }
   } // Else changed ok

   return res;
}


int
ParserTokenHandler::removeAllButToken( UserUser* user,
                                       const MC2String& tokenStr,
                                       const ClientSetting* clientSetting,
                                       bool sendChange,
                                       bool allGroups,
                                       bool deleteHotTokens )
{
   bool changed = false;
   int res = 0;
   const char* groupStr = tokenGroup( user, clientSetting );
   uint32 now = TimeUtility::getRealTime();
   for ( uint32 i = 0 ;
         i < user->getNbrOfType( UserConstants::TYPE_TOKEN ) ; i++ )
   {
      UserToken* t = static_cast< UserToken* >( 
         user->getElementOfType( i, UserConstants::TYPE_TOKEN ) );
      if ( (t->getGroup() == groupStr || allGroups) &&
           tokenStr.compare( t->getToken() ) != 0 && 
           (int32(now) - int32(t->getCreateTime()) > TOKEN_STILL_HOT_TIME ||
            deleteHotTokens) )
      {
         t->remove();
         changed = true;
      }
   }

   if ( sendChange && changed ) {
      uint32 startTime = TimeUtility::getCurrentTime();
      if ( !m_thread->changeUser( user, NULL/*changer*/ ) ) {
         res = -1;
         uint32 endTime = TimeUtility::getCurrentTime();
         if ( (endTime-startTime) > 2000 ) {
            res = -2;
         }
      } // Else changed ok
   } // End if changed

   return res;   
}


uint32
ParserTokenHandler::getNbrTokens( 
   const UserUser* user, const ClientSetting* clientSetting,
   bool allGroups ) const 
{
   int nbr = 0;
   const char* groupStr = tokenGroup( user, clientSetting );
   for ( uint32 i = 0 ;
         i < user->getNbrOfType( UserConstants::TYPE_TOKEN ) ; i++ )
   {
      UserToken* t = static_cast< UserToken* >( 
         user->getElementOfType( i, UserConstants::TYPE_TOKEN ) );
      if ( t->getGroup() == groupStr || allGroups ) {
         nbr++;
      }
   }
   return nbr;
}
