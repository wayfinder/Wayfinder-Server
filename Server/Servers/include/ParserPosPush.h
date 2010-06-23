/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSERPOSPUSH_H
#define PARSERPOSPUSH_H

#include "config.h"
#include "ParserHandler.h"
#include "ParserPosPushItem.h"
#include <vector>

// Forward declarations
class UserTrackElement;
class UserItem;
class ClientSetting;


/**
 * Class handling pushing positions.
 *
 */
class ParserPosPush : public ParserHandler {
   public:
      /**
       * Constructor.
       */
      ParserPosPush( ParserThread* thread,
                     ParserThreadGroup* group );

      /**
       * Destructor.
       */
      virtual ~ParserPosPush();

      /**
       * Checks if positions should be pushed and does that and
       * also migt say that positions doesn't have to be stored in db as 
       * they already has been pushed.
       *
       * @param p The positions.
       * @param user The user that the positions are from.
       * @param clientSetting The client the user is using, may be NULL.
       * @return 0 if ok, 1 if positions mustn't be stored in db
       *         -1 if error and -2 if timeout.
       */
      int positionsReceived( const vector< UserTrackElement* >& p,
                             const UserItem* user,
                             const ClientSetting* clientSetting );

   private:
      typedef vector<ParserPosPushItem*> pushItems;

      /**
       * The different position push handlers.
       */
      pushItems m_pushItems;
};

#endif // PARSERPOSPUSH_H

