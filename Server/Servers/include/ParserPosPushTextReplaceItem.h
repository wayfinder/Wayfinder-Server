/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSERPOSPUSHTEXTREPLACEITEM_H
#define PARSERPOSPUSHTEXTREPLACEITEM_H

#include "config.h"
#include "ParserPosPushItem.h"
#include "GfxUtility.h"
#include "URL.h"
#include <vector>

// Forward declarations
class UserTrackElement;
class UserItem;
class UserUser;
class ClientSetting;


/**
 * Class handling a specific pushing positions party.
 *
 */
class ParserPosPushTextReplaceItem : public ParserPosPushItem {
   public:
      /**
       * Constructor.
       *
       * @param peer Where to send the request.
       * @param users The users' changed fields are checked against 
       *              position user. This class takes ownership of the
       *              users.
       * @param postitionRequestData [UIN], [USERID], [DIST], [SPEED], 
       *        [TIME], [LAT], [LON], [ANGLE] are replaced. 
       * @param coordSys The content of [LAT] and [LON].
       * @param expectedReplyExp The regular expression required in the 
       *                         reply from the peer.
       * @param keepPositions If to add the positions to db not just push
       *                      them.
       */
      ParserPosPushTextReplaceItem( 
         ParserThread* thread,
         ParserThreadGroup* group,
         const URL& peer,
         vector< UserUser* >& users,
         const char* postitionRequestData,
         GfxUtility::coordianteRepresentations coordSys,
         const char* expectedReplyExp,
         bool keepPositions );

      /**
       * Destructor.
       */
      virtual ~ParserPosPushTextReplaceItem();

      /**
       * Checks if positions should be pushed and does that and
       * also might say that positions doesn't have to be stored in db as 
       * they already has been pushed.
       *
       * @param p The positions.
       * @param user The user that the positions are from.
       * @param clientSetting The client the user is using, may be NULL.
       * @return 0 if ok, 1 if positions mustn't be stored in db
       *         -1 if error and -2 if timeout.
       */
      virtual int positionsReceived( 
         const vector< UserTrackElement* >& p,
         const UserItem* user,
         const ClientSetting* clientSetting );

   private:
      /**
       * The peer.
       */
      URL m_peer;

      /**
       * The user patterns.
       */
      vector< UserUser* > m_users;

      /**
       * The replace string.
       */
      MC2String m_postitionRequestData;

      /**
       * The coordSys.
       */
      GfxUtility::coordianteRepresentations m_coordSys;

      /**
       * The expectedReplyExp.
       */
      MC2String m_expectedReplyExp;

      /**
       * The keepPositions.
       */
      bool m_keepPositions;
};

#endif // PARSERPOSPUSHTEXTREPLACEITEM_H

